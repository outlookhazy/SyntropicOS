

# File syn\_pubsub.c

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_pubsub.c**](syn__pubsub_8c.md)

[Go to the documentation of this file](syn__pubsub_8c.md)


```C++
#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_PUBSUB) || SYN_USE_PUBSUB

#include "syn_assert.h"
#include "syn_pubsub.h"

void syn_pubsub_init(SYN_PubSubBroker *broker, SYN_PubSubSub *sub_array, size_t capacity)
{
    SYN_ASSERT(broker != NULL);
    SYN_ASSERT(sub_array != NULL || capacity == 0);

    if (broker == NULL) {
        return; /* LCOV_EXCL_LINE: Defensive NULL check or invalid parameter fallback */
    }

    broker->subs = sub_array;
    broker->capacity = capacity;
    broker->count = 0;
}

bool syn_pubsub_subscribe(SYN_PubSubBroker *broker, uint16_t topic, SYN_PubSubHandler handler,
                          void *ctx)
{
    SYN_ASSERT(broker != NULL);
    SYN_ASSERT(handler != NULL);

    if (broker == NULL || broker->subs == NULL || handler == NULL) {
        return false;
    }

    if (broker->count >= broker->capacity) {
        return false;
    }

    // Avoid exact duplicates
    for (size_t i = 0; i < broker->count; i++) {
        if (broker->subs[i].topic == topic && broker->subs[i].handler == handler &&
            broker->subs[i].ctx == ctx) {
            return true; // Already subscribed
        }
    }

    broker->subs[broker->count].topic = topic;
    broker->subs[broker->count].handler = handler;
    broker->subs[broker->count].ctx = ctx;
    broker->count++;
    return true;
}

bool syn_pubsub_unsubscribe(SYN_PubSubBroker *broker, uint16_t topic, SYN_PubSubHandler handler)
{
    SYN_ASSERT(broker != NULL);
    SYN_ASSERT(handler != NULL);

    if (broker == NULL || broker->subs == NULL || handler == NULL) {
        return false;
    }

    for (size_t i = 0; i < broker->count; i++) {
        if (broker->subs[i].topic == topic && broker->subs[i].handler == handler) {
            // Swap with last element to remove (O(1) removal, unordered)
            broker->count--;
            if (i != broker->count) {
                broker->subs[i] = broker->subs[broker->count];
            }
            return true;
        }
    }
    return false;
}

void syn_pubsub_publish(SYN_PubSubBroker *broker, uint16_t topic, const void *payload, size_t len)
{
    if (broker == NULL || broker->subs == NULL) {
        return;
    }
    SYN_ASSERT(broker != NULL);

    size_t i = 0;
    while (i < broker->count) {
        /* Snapshot key fields before dispatch — handler may mutate the table */
        SYN_PubSubHandler h = broker->subs[i].handler;
        uint16_t top = broker->subs[i].topic;
        void *ctx = broker->subs[i].ctx;

        if (h != NULL && (top == topic || top == SYN_PUBSUB_TOPIC_ALL)) {
            h(topic, payload, len, ctx);
            /*
             * If the handler unsubscribed itself, the slot at [i] now holds
             * the element that was swapped in from the tail.  Do NOT advance i
             * so that the newly-placed element is evaluated on the next iteration.
             * We detect the swap by checking whether subs[i] changed.
             */
            if (broker->count > i && (broker->subs[i].handler != h ||
                                      broker->subs[i].topic != top || broker->subs[i].ctx != ctx)) {
                continue; /* re-evaluate position i without incrementing */
            }
        }
        i++;
    }
}

#endif /* SYN_USE_PUBSUB */
```


