#include "unity/unity.h"
#include "syntropic/util/syn_pubsub.h"
#include <string.h>

#define MAX_SUBS 4

static SYN_PubSubBroker broker;
static SYN_PubSubSub sub_array[MAX_SUBS];

static int callback1_calls = 0;
static int callback2_calls = 0;
static uint16_t last_topic = 0;
static char last_payload[32];

static void pubsub_setUp(void) {
    syn_pubsub_init(&broker, sub_array, MAX_SUBS);
    callback1_calls = 0;
    callback2_calls = 0;
    last_topic = 0;
    memset(last_payload, 0, sizeof(last_payload));
}

static void handler1(uint16_t topic, const void *payload, size_t len, void *ctx) {
    (void)ctx;
    callback1_calls++;
    last_topic = topic;
    if (payload && len > 0 && len < sizeof(last_payload)) {
        memcpy(last_payload, payload, len);
        last_payload[len] = '\0';
    }
}

static void handler2(uint16_t topic, const void *payload, size_t len, void *ctx) {
    (void)payload;
    (void)len;
    (void)ctx;
    callback2_calls++;
    last_topic = topic;
}

void test_pubsub_subscribe(void) {
    pubsub_setUp();
    TEST_ASSERT_TRUE(syn_pubsub_subscribe(&broker, 10, handler1, NULL));
    TEST_ASSERT_EQUAL(1, broker.count);
    
    // Duplicate sub should return true but not increase count
    TEST_ASSERT_TRUE(syn_pubsub_subscribe(&broker, 10, handler1, NULL));
    TEST_ASSERT_EQUAL(1, broker.count);
    
    // Max capacity test
    TEST_ASSERT_TRUE(syn_pubsub_subscribe(&broker, 11, handler2, NULL));
    TEST_ASSERT_TRUE(syn_pubsub_subscribe(&broker, 12, handler1, NULL));
    TEST_ASSERT_TRUE(syn_pubsub_subscribe(&broker, 13, handler2, NULL));
    TEST_ASSERT_EQUAL(MAX_SUBS, broker.count);
    
    // 5th should fail
    TEST_ASSERT_FALSE(syn_pubsub_subscribe(&broker, 14, handler1, NULL));
}

void test_pubsub_publish_single(void) {
    pubsub_setUp();
    syn_pubsub_subscribe(&broker, 42, handler1, NULL);
    
    const char *msg = "hello";
    syn_pubsub_publish(&broker, 42, msg, strlen(msg));
    
    TEST_ASSERT_EQUAL(1, callback1_calls);
    TEST_ASSERT_EQUAL(42, last_topic);
    TEST_ASSERT_EQUAL_STRING("hello", last_payload);
}

void test_pubsub_publish_multiple(void) {
    pubsub_setUp();
    syn_pubsub_subscribe(&broker, 100, handler1, NULL);
    syn_pubsub_subscribe(&broker, 100, handler2, NULL);
    syn_pubsub_subscribe(&broker, 200, handler2, NULL);
    
    syn_pubsub_publish(&broker, 100, NULL, 0);
    
    TEST_ASSERT_EQUAL(1, callback1_calls);
    TEST_ASSERT_EQUAL(1, callback2_calls);
    
    syn_pubsub_publish(&broker, 200, NULL, 0);
    
    TEST_ASSERT_EQUAL(1, callback1_calls);
    TEST_ASSERT_EQUAL(2, callback2_calls);
}

void test_pubsub_unsubscribe(void) {
    pubsub_setUp();
    syn_pubsub_subscribe(&broker, 50, handler1, NULL);
    syn_pubsub_subscribe(&broker, 50, handler2, NULL);
    
    TEST_ASSERT_EQUAL(2, broker.count);
    
    TEST_ASSERT_TRUE(syn_pubsub_unsubscribe(&broker, 50, handler1));
    TEST_ASSERT_EQUAL(1, broker.count);
    
    // Shouldn't be able to unsubscribe again
    TEST_ASSERT_FALSE(syn_pubsub_unsubscribe(&broker, 50, handler1));
    
    syn_pubsub_publish(&broker, 50, NULL, 0);
    
    TEST_ASSERT_EQUAL(0, callback1_calls);
    TEST_ASSERT_EQUAL(1, callback2_calls);
}

void test_pubsub_wildcard(void) {
    pubsub_setUp();
    syn_pubsub_subscribe(&broker, SYN_PUBSUB_TOPIC_ALL, handler1, NULL);
    syn_pubsub_subscribe(&broker, 77, handler2, NULL);
    
    syn_pubsub_publish(&broker, 88, NULL, 0); // handler1 gets it
    syn_pubsub_publish(&broker, 77, NULL, 0); // both get it
    
    TEST_ASSERT_EQUAL(2, callback1_calls);
    TEST_ASSERT_EQUAL(1, callback2_calls);
}

void run_pubsub_tests(void) {
    RUN_TEST(test_pubsub_subscribe);
    RUN_TEST(test_pubsub_publish_single);
    RUN_TEST(test_pubsub_publish_multiple);
    RUN_TEST(test_pubsub_unsubscribe);
    RUN_TEST(test_pubsub_wildcard);
}
