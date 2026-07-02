

# File syn\_pubsub.h

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_pubsub.h**](syn__pubsub_8h.md)

[Go to the documentation of this file](syn__pubsub_8h.md)


```C++

#ifndef SYN_PUBSUB_H
#define SYN_PUBSUB_H

#include "../common/syn_defs.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SYN_PUBSUB_TOPIC_ALL 0xFFFF

typedef void (*SYN_PubSubHandler)(uint16_t topic, const void *payload, size_t len, void *ctx);

typedef struct {
    uint16_t topic;              
    SYN_PubSubHandler handler;   
    void *ctx;                   
} SYN_PubSubSub;

typedef struct {
    SYN_PubSubSub *subs;     
    size_t capacity;          
    size_t count;             
} SYN_PubSubBroker;

void syn_pubsub_init(SYN_PubSubBroker *broker, SYN_PubSubSub *sub_array, size_t capacity);

bool syn_pubsub_subscribe(SYN_PubSubBroker *broker, uint16_t topic, SYN_PubSubHandler handler, void *ctx);

bool syn_pubsub_unsubscribe(SYN_PubSubBroker *broker, uint16_t topic, SYN_PubSubHandler handler);

void syn_pubsub_publish(SYN_PubSubBroker *broker, uint16_t topic, const void *payload, size_t len);

static inline size_t syn_pubsub_count(const SYN_PubSubBroker *broker) {
    return broker->count;
}

static inline void syn_pubsub_reset(SYN_PubSubBroker *broker) {
    broker->count = 0;
}

#ifdef __cplusplus
}
#endif
#endif // SYN_PUBSUB_H
```


