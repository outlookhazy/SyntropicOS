#include "mock_port.h"
#include "syntropic/net/syn_mqtt.h"
#include "unity/unity.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static bool message_received = false;
static char last_payload[128] = {0};

static bool integration_skip_allowed(void)
{
    const char *v = getenv("SYN_ALLOW_SKIP_INTEGRATION");
    return (v != NULL && strcmp(v, "1") == 0);
}

static void fail_or_skip(const char *msg)
{
    if (integration_skip_allowed()) {
        TEST_IGNORE_MESSAGE(msg);
    } else {
        TEST_FAIL_MESSAGE(msg);
    }
}

static void on_mqtt_msg(const char *topic, const uint8_t *payload, size_t len, void *ctx)
{
    (void)ctx;
    printf("[Integration Test] Received MQTT topic '%s' (%zu B)\n", topic, len);
    if (len < sizeof(last_payload)) {
        memcpy(last_payload, payload, len);
        last_payload[len] = '\0';
    }
    message_received = true;
}

void setUp(void)
{
    message_received = false;
    memset(last_payload, 0, sizeof(last_payload));
}

void tearDown(void)
{
}

void test_mqtt_mosquitto_e2e(void)
{
    const char *host = getenv("MQTT_HOST");
    if (host == NULL) {
        host = "127.0.0.1";
    }
    uint16_t port = 1883U;

    uint8_t rx_buf[256];
    uint8_t tx_buf[256];
    SYN_MqttClient client;

    printf("[Integration Test] Connecting to Mosquitto Broker at %s:%u...\n", host, port);
    SYN_Status status = syn_mqtt_init(&client, host, port, "syn_integration_test_client", NULL,
                                      NULL, 60U, rx_buf, sizeof(rx_buf), tx_buf, sizeof(tx_buf));
    TEST_ASSERT_EQUAL_INT(SYN_OK, status);

    client.on_message = on_mqtt_msg;

    SYN_PT pt;
    PT_INIT(&pt);

    SYN_Task task;
    memset(&task, 0, sizeof(task));
    task.user_data = &client;

    int iterations = 0;
    while (client.state != SYN_MQTT_CONNECTED && iterations < 200) {
        mock_tick_advance(10U);
        syn_mqtt_task(&pt, &task);
        usleep(10000);
        iterations++;
    }
    if (client.state != SYN_MQTT_CONNECTED) {
        fail_or_skip("Mosquitto MQTT broker not reachable");
        return;
    }
    printf("[Integration Test] Connected to Mosquitto Broker!\n");

    status = syn_mqtt_subscribe(&client, "syntropic/test", 0U);
    TEST_ASSERT_EQUAL_INT(SYN_OK, status);

    for (int i = 0; i < 20; i++) {
        mock_tick_advance(10U);
        syn_mqtt_task(&pt, &task);
        usleep(10000);
    }

    const char *test_msg = "Hello Mosquitto from SyntropicOS!";
    status = syn_mqtt_publish(&client, "syntropic/test", test_msg, strlen(test_msg), 0U, false);
    TEST_ASSERT_EQUAL_INT(SYN_OK, status);

    iterations = 0;
    while (!message_received && iterations < 100) {
        mock_tick_advance(50U);
        syn_mqtt_task(&pt, &task);
        usleep(10000);
        iterations++;
    }

    TEST_ASSERT_TRUE(message_received);
    TEST_ASSERT_EQUAL_STRING(test_msg, last_payload);
    syn_mqtt_disconnect(&client);

    printf("[Integration Test] End-to-End Mosquitto Integration PASS!\n");
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_mqtt_mosquitto_e2e);
    return UNITY_END();
}
