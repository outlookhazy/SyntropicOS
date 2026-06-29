#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_PUBSUB) || SYN_USE_PUBSUB

#include "syn_pubsub.h"
#include "syn_assert.h"

void syn_pubsub_init(SYN_PubSubBroker *broker, SYN_PubSubSub *sub_array, size_t capacity) {
    SYN_ASSERT(broker != NULL);
    SYN_ASSERT(sub_array != NULL || capacity == 0);
    broker->subs = sub_array;
    broker->capacity = capacity;
    broker->count = 0;
}

bool syn_pubsub_subscribe(SYN_PubSubBroker *broker, uint16_t topic, SYN_PubSubHandler handler, void *ctx) {
    SYN_ASSERT(broker != NULL);
    SYN_ASSERT(handler != NULL);

    if (broker->count >= broker->capacity) {
        return false;
    }
    
    // Avoid exact duplicates
    for (size_t i = 0; i < broker->count; i++) {
        if (broker->subs[i].topic == topic && 
            broker->subs[i].handler == handler && 
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

bool syn_pubsub_unsubscribe(SYN_PubSubBroker *broker, uint16_t topic, SYN_PubSubHandler handler) {
    SYN_ASSERT(broker != NULL);
    SYN_ASSERT(handler != NULL);

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

void syn_pubsub_publish(SYN_PubSubBroker *broker, uint16_t topic, const void *payload, size_t len) {
    SYN_ASSERT(broker != NULL);

    for (size_t i = 0; i < broker->count; i++) {
        if (broker->subs[i].topic == topic || broker->subs[i].topic == SYN_PUBSUB_TOPIC_ALL) {
            broker->subs[i].handler(topic, payload, len, broker->subs[i].ctx);
        }
    }
}

#endif /* SYN_USE_PUBSUB */
