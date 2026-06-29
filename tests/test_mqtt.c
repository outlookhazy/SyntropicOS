#include "unity/unity.h"
#include "mocks/mock_port.h"
#include "syntropic/net/syn_mqtt.h"
#include <string.h>

static int s_mqtt_msg_count = 0;
static char s_mqtt_last_topic[64];
static uint8_t s_mqtt_last_payload[128];
static size_t s_mqtt_last_len = 0;

static void on_mqtt_message(const char *topic, const uint8_t *payload, size_t len, void *ctx)
{
    (void)ctx;
    s_mqtt_msg_count++;
    strncpy(s_mqtt_last_topic, topic, sizeof(s_mqtt_last_topic) - 1);
    s_mqtt_last_topic[sizeof(s_mqtt_last_topic)-1] = '\0';
    s_mqtt_last_len = len < sizeof(s_mqtt_last_payload) ? len : sizeof(s_mqtt_last_payload);
    memcpy(s_mqtt_last_payload, payload, s_mqtt_last_len);
}

void test_mqtt_connect(void)
{
    mock_port_reset();

    SYN_MqttClient c;
    uint8_t rx[256];
    uint8_t tx[256];
    syn_mqtt_init(&c, "broker.hivemq.com", 1883, "myclient", NULL, NULL, 60, rx, sizeof(rx), tx, sizeof(tx));

    SYN_PT pt;
    PT_INIT(&pt);

    SYN_Task task;
    task.user_data = &c;

    /* 1. Run task first time to trigger connection */
    syn_mqtt_task(&pt, &task);

    TEST_ASSERT_EQUAL(SYN_MQTT_CONNECTING, c.state);
    TEST_ASSERT_TRUE(mock_sock_tx_len > 0);
    TEST_ASSERT_EQUAL_UINT8(0x10, mock_sock_tx_buf[0]); /* CONNECT type */

    /* 2. Mock broker response with CONNACK (0x20, 0x02, 0x00, 0x00) */
    uint8_t connack[] = { 0x20, 0x02, 0x00, 0x00 };
    mock_sock_set_response(connack, sizeof(connack));

    /* 3. Run task again to read CONNACK */
    syn_mqtt_task(&pt, &task);

    TEST_ASSERT_EQUAL(SYN_MQTT_CONNECTED, c.state);
}

void test_mqtt_subscribe(void)
{
    mock_port_reset();

    SYN_MqttClient c;
    uint8_t rx[256];
    uint8_t tx[256];
    syn_mqtt_init(&c, "broker.hivemq.com", 1883, "myclient", NULL, NULL, 60, rx, sizeof(rx), tx, sizeof(tx));
    c.state = SYN_MQTT_CONNECTED;
    c.sock = 11;
    mock_sock_connected = true;

    SYN_Status st = syn_mqtt_subscribe(&c, "sensors/temp", 1);
    TEST_ASSERT_EQUAL(SYN_OK, st);

    /* Verify sent SUBSCRIBE packet (0x82) */
    TEST_ASSERT_TRUE(mock_sock_tx_len > 0);
    TEST_ASSERT_EQUAL_UINT8(0x82, mock_sock_tx_buf[0]);
    
    /* Topic should be in payload starting at index 6 */
    TEST_ASSERT_EQUAL_STRING_LEN("sensors/temp", &mock_sock_tx_buf[6], 12);
}

void test_mqtt_publish_qos0(void)
{
    mock_port_reset();

    SYN_MqttClient c;
    uint8_t rx[256];
    uint8_t tx[256];
    syn_mqtt_init(&c, "broker.hivemq.com", 1883, "myclient", NULL, NULL, 60, rx, sizeof(rx), tx, sizeof(tx));
    c.state = SYN_MQTT_CONNECTED;
    c.sock = 11;
    mock_sock_connected = true;

    const char *payload = "23.5";
    SYN_Status st = syn_mqtt_publish(&c, "sensors/temp", payload, strlen(payload), 0, false);
    TEST_ASSERT_EQUAL(SYN_OK, st);

    /* Verify sent PUBLISH packet (0x30) */
    TEST_ASSERT_TRUE(mock_sock_tx_len > 0);
    TEST_ASSERT_EQUAL_UINT8(0x30, mock_sock_tx_buf[0]);
    TEST_ASSERT_EQUAL_STRING_LEN("23.5", &mock_sock_tx_buf[mock_sock_tx_len - 4], 4);
}

void test_mqtt_publish_qos1_retry(void)
{
    mock_port_reset();

    SYN_MqttClient c;
    uint8_t rx[256];
    uint8_t tx[256];
    syn_mqtt_init(&c, "broker.hivemq.com", 1883, "myclient", NULL, NULL, 60, rx, sizeof(rx), tx, sizeof(tx));
    c.state = SYN_MQTT_CONNECTED;
    c.sock = 11;
    mock_sock_connected = true;

    SYN_PT pt;
    PT_INIT(&pt);
    SYN_Task task;
    task.user_data = &c;

    /* 1. Publish QoS 1 */
    const char *payload = "23.5";
    syn_mqtt_publish(&c, "sensors/temp", payload, strlen(payload), 1, false);

    TEST_ASSERT_NOT_EQUAL(0, c.pending_puback_id);
    size_t first_tx_len = mock_sock_tx_len;

    /* 2. Run task within timeout, shouldn't retransmit */
    mock_tick_advance(1000);
    syn_mqtt_task(&pt, &task);
    TEST_ASSERT_EQUAL(first_tx_len, mock_sock_tx_len);

    /* 3. Run task after 5000 ms timeout, should retransmit (DUP flag 0x08 set on header -> 0x30|0x08|0x02 = 0x3A) */
    mock_tick_advance(5000);
    syn_mqtt_task(&pt, &task);

    TEST_ASSERT_TRUE(mock_sock_tx_len > first_tx_len);
    TEST_ASSERT_EQUAL_UINT8(0x3A, mock_sock_tx_buf[first_tx_len]);

    /* 4. Mock PUBACK response from broker (0x40, 0x02, PID_MSB, PID_LSB) */
    uint8_t puback[] = { 0x40, 0x02, (uint8_t)(c.pending_puback_id >> 8), (uint8_t)(c.pending_puback_id & 255) };
    mock_sock_set_response(puback, sizeof(puback));

    syn_mqtt_task(&pt, &task);

    /* Verify pending_puback_id cleared */
    TEST_ASSERT_EQUAL(0, c.pending_puback_id);
}

void test_mqtt_rx_publish(void)
{
    mock_port_reset();
    s_mqtt_msg_count = 0;

    SYN_MqttClient c;
    uint8_t rx[256];
    uint8_t tx[256];
    syn_mqtt_init(&c, "broker.hivemq.com", 1883, "myclient", NULL, NULL, 60, rx, sizeof(rx), tx, sizeof(tx));
    c.state = SYN_MQTT_CONNECTED;
    c.sock = 11;
    c.on_message = on_mqtt_message;
    mock_sock_connected = true;

    SYN_PT pt;
    PT_INIT(&pt);
    SYN_Task task;
    task.user_data = &c;

    /* Mock incoming PUBLISH frame from broker on topic "cmd/led" with payload "ON" */
    /* Topic len: 7, Payload: "ON" */
    uint8_t frame[] = {
        0x30,                   /* PUBLISH QoS 0 */
        11,                     /* Rem Len = 2 + 7 + 2 (no pkt id in QoS 0) = 11? wait. topic len is 2 bytes (7) + topic (7) + payload (2) = 11 bytes */
        0x00, 0x07,             /* Topic Len */
        'c', 'm', 'd', '/', 'l', 'e', 'd',
        'O', 'N'
    };
    mock_sock_set_response(frame, sizeof(frame));

    syn_mqtt_task(&pt, &task);

    TEST_ASSERT_EQUAL(1, s_mqtt_msg_count);
    TEST_ASSERT_EQUAL_STRING("cmd/led", s_mqtt_last_topic);
    TEST_ASSERT_EQUAL_UINT32(2, s_mqtt_last_len);
    TEST_ASSERT_EQUAL_STRING_LEN("ON", s_mqtt_last_payload, 2);
}

void run_mqtt_tests(void)
{
    RUN_TEST(test_mqtt_connect);
    RUN_TEST(test_mqtt_subscribe);
    RUN_TEST(test_mqtt_publish_qos0);
    RUN_TEST(test_mqtt_publish_qos1_retry);
    RUN_TEST(test_mqtt_rx_publish);
}
