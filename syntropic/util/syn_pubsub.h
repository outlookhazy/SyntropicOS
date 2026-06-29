/**
 * @file syn_pubsub.h
 * @brief Synchronous publish/subscribe event broker.
 *
 * Provides a decoupled event system. Modules can subscribe to specific
 * topics and receive callbacks when events are published.
 * 
 * Uses a static caller-provided array for storing subscriptions (zero allocation).
 * @ingroup syn_sched
 */

#ifndef SYN_PUBSUB_H
#define SYN_PUBSUB_H

#include "../common/syn_defs.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Special topic ID used to subscribe to ALL events. */
#define SYN_PUBSUB_TOPIC_ALL 0xFFFF

/** 
 * @brief Event callback function signature.
 * 
 * @param topic   The topic ID that triggered this callback.
 * @param payload Pointer to the event data (can be NULL).
 * @param len     Size of the event data in bytes (can be 0).
 * @param ctx     User context pointer provided at subscription.
 */
typedef void (*SYN_PubSubHandler)(uint16_t topic, const void *payload, size_t len, void *ctx);

/** 
 * @brief A single subscription record.
 * @note Treated as opaque by the user. 
 */
typedef struct {
    uint16_t topic;              /**< Topic this subscription listens for */
    SYN_PubSubHandler handler;   /**< Callback function                  */
    void *ctx;                   /**< User context                       */
} SYN_PubSubSub;

/** @brief PubSub broker instance — subscription array + count. */
typedef struct {
    SYN_PubSubSub *subs;     /**< Subscription array                     */
    size_t capacity;          /**< Maximum subscriptions                  */
    size_t count;             /**< Active subscription count              */
} SYN_PubSubBroker;

/**
 * @brief Initialize a pubsub broker.
 * @param broker    Pointer to broker instance.
 * @param sub_array Pointer to an array of SYN_PubSubSub structures.
 * @param capacity  Number of elements in sub_array.
 */
void syn_pubsub_init(SYN_PubSubBroker *broker, SYN_PubSubSub *sub_array, size_t capacity);

/**
 * @brief Subscribe to a topic.
 * @param broker  Pointer to broker instance.
 * @param topic   Topic ID to listen for, or SYN_PUBSUB_TOPIC_ALL.
 * @param handler Callback function to invoke.
 * @param ctx     Optional user context pointer.
 * @return true if subscribed successfully, false if broker is full.
 */
bool syn_pubsub_subscribe(SYN_PubSubBroker *broker, uint16_t topic, SYN_PubSubHandler handler, void *ctx);

/**
 * @brief Unsubscribe from a topic.
 * @param broker  Pointer to broker instance.
 * @param topic   Topic ID to unsubscribe from.
 * @param handler The specific callback function to remove.
 * @return true if removed, false if not found.
 */
bool syn_pubsub_unsubscribe(SYN_PubSubBroker *broker, uint16_t topic, SYN_PubSubHandler handler);

/**
 * @brief Publish an event to all subscribers of a topic.
 * @param broker  Pointer to broker instance.
 * @param topic   Topic ID of the event.
 * @param payload Pointer to event data.
 * @param len     Size of event data in bytes.
 */
void syn_pubsub_publish(SYN_PubSubBroker *broker, uint16_t topic, const void *payload, size_t len);

/**
 * @brief Get current number of active subscriptions.
 * @param broker  Broker.
 * @return Subscription count.
 */
static inline size_t syn_pubsub_count(const SYN_PubSubBroker *broker) {
    return broker->count;
}

/**
 * @brief Reset the broker, removing all subscriptions.
 * @param broker  Broker.
 */
static inline void syn_pubsub_reset(SYN_PubSubBroker *broker) {
    broker->count = 0;
}

#ifdef __cplusplus
}
#endif
#endif // SYN_PUBSUB_H
