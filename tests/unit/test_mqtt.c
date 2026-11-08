#include "mocks/mock_port.h"
#include "syntropic/net/syn_mqtt.h"
#include "syntropic/sched/syn_sched.h"
#include "unity/unity.h"

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
    s_mqtt_last_topic[sizeof(s_mqtt_last_topic) - 1] = '\0';
    s_mqtt_last_len = len < sizeof(s_mqtt_last_payload) ? len : sizeof(s_mqtt_last_payload);
    memcpy(s_mqtt_last_payload, payload, s_mqtt_last_len);
}

void test_mqtt_connect(void)
{
    mock_port_reset();

    SYN_MqttClient c;
    uint8_t rx[256];
    uint8_t tx[256];
    syn_mqtt_init(&c, "broker.hivemq.com", 1883, "myclient", NULL, NULL, 60, rx, sizeof(rx), tx,
                  sizeof(tx));

    SYN_PT pt;
    PT_INIT(&pt);

    SYN_Task task = {0};
    task.user_data = &c;

    /* 1. Run task first time to trigger connection */
    syn_mqtt_task(&pt, &task);

    TEST_ASSERT_EQUAL(SYN_MQTT_CONNECTING, c.state);
    TEST_ASSERT_TRUE(mock_sock_tx_len > 0);
    TEST_ASSERT_EQUAL_UINT8(0x10, mock_sock_tx_buf[0]); /* CONNECT type */

    /* 2. Mock broker response with CONNACK (0x20, 0x02, 0x00, 0x00) */
    uint8_t connack[] = {0x20, 0x02, 0x00, 0x00};
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
    syn_mqtt_init(&c, "broker.hivemq.com", 1883, "myclient", NULL, NULL, 60, rx, sizeof(rx), tx,
                  sizeof(tx));
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
    syn_mqtt_init(&c, "broker.hivemq.com", 1883, "myclient", NULL, NULL, 60, rx, sizeof(rx), tx,
                  sizeof(tx));
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
    syn_mqtt_init(&c, "broker.hivemq.com", 1883, "myclient", NULL, NULL, 60, rx, sizeof(rx), tx,
                  sizeof(tx));
    c.state = SYN_MQTT_CONNECTED;
    c.sock = 11;
    mock_sock_connected = true;

    SYN_PT pt;
    PT_INIT(&pt);
    SYN_Task task = {0};
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

    /* 3. Run task after 5000 ms timeout, should retransmit (DUP flag 0x08 set on header ->
     * 0x30|0x08|0x02 = 0x3A) */
    mock_tick_advance(5000);
    syn_mqtt_task(&pt, &task);

    TEST_ASSERT_TRUE(mock_sock_tx_len > first_tx_len);
    TEST_ASSERT_EQUAL_UINT8(0x3A, mock_sock_tx_buf[first_tx_len]);

    /* 4. Mock PUBACK response from broker (0x40, 0x02, PID_MSB, PID_LSB) */
    uint8_t puback[] = {0x40, 0x02, (uint8_t)(c.pending_puback_id >> 8),
                        (uint8_t)(c.pending_puback_id & 255)};
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
    syn_mqtt_init(&c, "broker.hivemq.com", 1883, "myclient", NULL, NULL, 60, rx, sizeof(rx), tx,
                  sizeof(tx));
    c.state = SYN_MQTT_CONNECTED;
    c.sock = 11;
    c.on_message = on_mqtt_message;
    mock_sock_connected = true;

    SYN_PT pt;
    PT_INIT(&pt);
    SYN_Task task = {0};
    task.user_data = &c;

    /* Mock incoming PUBLISH frame from broker on topic "cmd/led" with payload "ON" */
    /* Topic len: 7, Payload: "ON" */
    uint8_t frame[] = {0x30, /* PUBLISH QoS 0 */
                       11,   /* Rem Len = 2 + 7 + 2 (no pkt id in QoS 0) = 11? wait. topic len is 2
                                bytes (7) + topic (7) + payload (2) = 11 bytes */
                       0x00, 0x07, /* Topic Len */
                       'c',  'm',  'd', '/', 'l', 'e', 'd', 'O', 'N'};
    mock_sock_set_response(frame, sizeof(frame));

    syn_mqtt_task(&pt, &task);

    TEST_ASSERT_EQUAL(1, s_mqtt_msg_count);
    TEST_ASSERT_EQUAL_STRING("cmd/led", s_mqtt_last_topic);
    TEST_ASSERT_EQUAL_UINT32(2, s_mqtt_last_len);
    TEST_ASSERT_EQUAL_STRING_LEN("ON", s_mqtt_last_payload, 2);
}

static void test_mqtt_formatting_and_boundaries(void)
{
    mock_port_reset();

    /* 1. CONNECT formatting with username and password */
    SYN_MqttClient c;
    uint8_t rx[256];
    uint8_t tx[256];
    syn_mqtt_init(&c, "broker.hivemq.com", 1883, "myclient", "myuser", "mypass", 60, rx, sizeof(rx),
                  tx, sizeof(tx));

    SYN_PT pt;
    PT_INIT(&pt);
    SYN_Task task = {0};
    task.user_data = &c;

    syn_mqtt_task(&pt, &task);
    TEST_ASSERT_EQUAL(SYN_MQTT_CONNECTING, c.state);
    /* CONNECT type (0x10) and flags (0x02 | 0x80 | 0x40 = 0xC2) */
    TEST_ASSERT_EQUAL_UINT8(0x10, mock_sock_tx_buf[0]);

    /* 2. Buffer size guard for CONNECT */
    PT_INIT(&pt);
    syn_mqtt_init(&c, "broker.hivemq.com", 1883, "myclient", NULL, NULL, 60, rx, sizeof(rx), tx,
                  5); // tiny tx buffer
    syn_mqtt_task(&pt, &task);
    /* Should fail to send, close socket, and transition back to DISCONNECTED */
    TEST_ASSERT_EQUAL(SYN_MQTT_DISCONNECTED, c.state);
    TEST_ASSERT_EQUAL(SYN_SOCKET_INVALID, c.sock);

    /* 3. Retransmit buffer size guard on publish */
    syn_mqtt_init(&c, "broker.hivemq.com", 1883, "myclient", NULL, NULL, 60, rx, sizeof(rx), tx,
                  sizeof(tx));
    c.state = SYN_MQTT_CONNECTED;
    c.sock = 11;
    mock_sock_connected = true;

    /* Long payload to exceed retransmit_buf (128 bytes) -> rejected for QoS 1, allowed for QoS 0 */
    uint8_t long_payload[130];
    memset(long_payload, 'A', sizeof(long_payload));
    SYN_Status st = syn_mqtt_publish(&c, "temp", long_payload, sizeof(long_payload), 1, false);
    TEST_ASSERT_EQUAL(SYN_ERROR, st);
    TEST_ASSERT_EQUAL_UINT16(0U, c.pending_puback_id);
    TEST_ASSERT_EQUAL_INT(0, c.retransmit_len);
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_mqtt_publish(&c, "temp", long_payload, sizeof(long_payload), 0, false));

    /* 4. Retransmit buffer size guard on subscribe */
    /* Long topic to exceed retransmit_buf (128 bytes) */
    char long_topic[130];
    memset(long_topic, 'A', sizeof(long_topic) - 1);
    long_topic[sizeof(long_topic) - 1] = '\0';
    st = syn_mqtt_subscribe(&c, long_topic, 1);
    TEST_ASSERT_EQUAL(SYN_OK, st);
    TEST_ASSERT_EQUAL_INT(0, c.retransmit_len);

    /* 5. Publish / Subscribe while not connected */
    c.state = SYN_MQTT_DISCONNECTED;
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_mqtt_publish(&c, "temp", "23", 2, 1, false));
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_mqtt_subscribe(&c, "temp", 1));

    /* 6. Extra boundary formatting checks: username without password, subscribe QoS 0, publish with
     * NULL payload */
    syn_mqtt_init(&c, "broker.hivemq.com", 1883, "myclient", "myuser", NULL, 60, rx, sizeof(rx), tx,
                  sizeof(tx));
    PT_INIT(&pt);
    syn_mqtt_task(&pt, &task);
    TEST_ASSERT_EQUAL(SYN_MQTT_CONNECTING, c.state);
    TEST_ASSERT_EQUAL_UINT8(0x10, mock_sock_tx_buf[0]);

    c.state = SYN_MQTT_CONNECTED;
    c.sock = 11;
    mock_sock_connected = true;
    TEST_ASSERT_EQUAL(SYN_OK, syn_mqtt_subscribe(&c, "temp", 0));
    TEST_ASSERT_EQUAL(SYN_OK, syn_mqtt_publish(&c, "temp", NULL, 0, 0, false));
}

static void test_mqtt_multiplier_overflow(void)
{
    mock_port_reset();
    SYN_MqttClient c;
    uint8_t rx[256];
    uint8_t tx[256];
    syn_mqtt_init(&c, "broker.hivemq.com", 1883, "myclient", NULL, NULL, 60, rx, sizeof(rx), tx,
                  sizeof(tx));
    c.state = SYN_MQTT_CONNECTED;
    c.sock = 11;
    mock_sock_connected = true;

    SYN_PT pt;
    PT_INIT(&pt);
    SYN_Task task = {0};
    task.user_data = &c;

    /* 5 bytes of remaining length all with MSB set (0x80) */
    uint8_t frame[] = {0x30, 0x80, 0x80, 0x80, 0x80, 0x80};
    mock_sock_set_response(frame, sizeof(frame));

    syn_mqtt_task(&pt, &task);

    /* Since read_remaining_len returns -1, it should close the socket and transition to
     * DISCONNECTED */
    TEST_ASSERT_EQUAL(SYN_MQTT_DISCONNECTED, c.state);
    TEST_ASSERT_EQUAL(SYN_SOCKET_INVALID, c.sock);
}

static void test_mqtt_publish_edge_cases(void)
{
    mock_port_reset();
    s_mqtt_msg_count = 0;

    SYN_MqttClient c;
    uint8_t rx[256];
    uint8_t tx[256];
    syn_mqtt_init(&c, "broker.hivemq.com", 1883, "myclient", NULL, NULL, 60, rx, sizeof(rx), tx,
                  sizeof(tx));
    c.state = SYN_MQTT_CONNECTED;
    c.sock = 11;
    c.on_message = on_mqtt_message;
    mock_sock_connected = true;

    SYN_PT pt;
    PT_INIT(&pt);
    SYN_Task task = {0};
    task.user_data = &c;

    /* A. len < 2 */
    uint8_t frame_short[] = {0x30, 0x01, 0x00};
    mock_sock_set_response(frame_short, sizeof(frame_short));
    syn_mqtt_task(&pt, &task);
    TEST_ASSERT_EQUAL(0, s_mqtt_msg_count);

    /* B. topic_len too large (2 + topic_len > len) */
    PT_INIT(&pt);
    uint8_t frame_bad_topic_len[] = {0x30, 0x05, 0x00, 0x0A, 't', 'e', 's'};
    mock_sock_set_response(frame_bad_topic_len, sizeof(frame_bad_topic_len));
    syn_mqtt_task(&pt, &task);
    TEST_ASSERT_EQUAL(0, s_mqtt_msg_count);

    /* C. qos > 0 but payload_offset + 2 > len (no packet ID) */
    PT_INIT(&pt);
    uint8_t frame_no_pid[] = {0x32, 0x05, 0x00, 0x03, 't', 'm', 'p'}; // QoS 1 (0x32)
    mock_sock_set_response(frame_no_pid, sizeof(frame_no_pid));
    syn_mqtt_task(&pt, &task);
    TEST_ASSERT_EQUAL(0, s_mqtt_msg_count);

    /* D. QoS 1 publish success (sends PUBACK) */
    PT_INIT(&pt);
    mock_sock_tx_len = 0;
    uint8_t frame_qos1[] = {0x32, 0x07, 0x00, 0x03, 't', 'm', 'p', 0x00, 0x2A}; // QoS 1, PID = 42
    mock_sock_set_response(frame_qos1, sizeof(frame_qos1));
    syn_mqtt_task(&pt, &task);
    TEST_ASSERT_EQUAL(1, s_mqtt_msg_count);
    TEST_ASSERT_EQUAL_STRING("tmp", s_mqtt_last_topic);
    TEST_ASSERT_EQUAL(0, s_mqtt_last_len); // empty payload
    // check PUBACK sent: 0x40, 0x02, 0x00, 0x2A
    TEST_ASSERT_EQUAL(4, mock_sock_tx_len);
    TEST_ASSERT_EQUAL_UINT8(0x40, mock_sock_tx_buf[0]);
    TEST_ASSERT_EQUAL_UINT8(0x02, mock_sock_tx_buf[1]);
    TEST_ASSERT_EQUAL_UINT8(0x00, mock_sock_tx_buf[2]);
    TEST_ASSERT_EQUAL_UINT8(0x2A, mock_sock_tx_buf[3]);

    /* E. QoS 2 publish success (calls callback, no PUBACK) */
    PT_INIT(&pt);
    mock_sock_tx_len = 0;
    s_mqtt_msg_count = 0;
    uint8_t frame_qos2[] = {0x34, 0x07, 0x00, 0x03, 't',
                            'm',  'p',  0x00, 0x2B}; // QoS 2 (0x34), PID = 43
    mock_sock_set_response(frame_qos2, sizeof(frame_qos2));
    syn_mqtt_task(&pt, &task);
    TEST_ASSERT_EQUAL(1, s_mqtt_msg_count);
    TEST_ASSERT_EQUAL(0, mock_sock_tx_len); // no PUBACK sent

    /* F. QoS 3 publish success (calls callback, no PUBACK) */
    PT_INIT(&pt);
    mock_sock_tx_len = 0;
    s_mqtt_msg_count = 0;
    uint8_t frame_qos3[] = {0x36, 0x07, 0x00, 0x03, 't',
                            'm',  'p',  0x00, 0x2C}; // QoS 3 (0x36), PID = 44
    mock_sock_set_response(frame_qos3, sizeof(frame_qos3));
    syn_mqtt_task(&pt, &task);
    TEST_ASSERT_EQUAL(1, s_mqtt_msg_count);
    TEST_ASSERT_EQUAL(0, mock_sock_tx_len); // no PUBACK sent
}

static void test_mqtt_state_machine_failures(void)
{
    /* 1. Host Connect Fail */
    mock_port_reset();
    SYN_MqttClient c;
    uint8_t rx[256];
    uint8_t tx[256];
    syn_mqtt_init(&c, "broker.hivemq.com", 1883, "myclient", NULL, NULL, 60, rx, sizeof(rx), tx,
                  sizeof(tx));

    SYN_PT pt;
    PT_INIT(&pt);
    SYN_Task task = {0};
    task.user_data = &c;

    mock_sock_connect_fail = true;
    syn_mqtt_task(&pt, &task);
    TEST_ASSERT_EQUAL(SYN_MQTT_DISCONNECTED, c.state);
    TEST_ASSERT_EQUAL(SYN_SOCKET_INVALID, c.sock);

    /* Resume task after connect delay to hit continue statement */
    mock_tick_advance(5000);
    syn_mqtt_task(&pt, &task);

    /* 2. Connect Send Fail */
    mock_port_reset();
    PT_INIT(&pt);
    syn_mqtt_init(&c, "broker.hivemq.com", 1883, "myclient", NULL, NULL, 60, rx, sizeof(rx), tx,
                  sizeof(tx));
    mock_sock_connect_fail = false;
    mock_sock_send_fail = true;
    syn_mqtt_task(&pt, &task);
    TEST_ASSERT_EQUAL(SYN_MQTT_DISCONNECTED, c.state);
    TEST_ASSERT_EQUAL(SYN_SOCKET_INVALID, c.sock);

    /* Resume task after send failure delay to hit continue statement */
    mock_tick_advance(5000);
    syn_mqtt_task(&pt, &task);

    /* 3. CONNECTING CONNACK Timeout */
    mock_port_reset();
    PT_INIT(&pt);
    syn_mqtt_init(&c, "broker.hivemq.com", 1883, "myclient", NULL, NULL, 60, rx, sizeof(rx), tx,
                  sizeof(tx));
    syn_mqtt_task(&pt, &task);
    TEST_ASSERT_EQUAL(SYN_MQTT_CONNECTING, c.state);
    mock_tick_advance(5000);
    syn_mqtt_task(&pt, &task);
    TEST_ASSERT_EQUAL(SYN_MQTT_DISCONNECTED, c.state);
    TEST_ASSERT_EQUAL(SYN_SOCKET_INVALID, c.sock);

    /* Resume task after CONNACK timeout delay to hit continue statement */
    mock_tick_advance(5000);
    syn_mqtt_task(&pt, &task);

    /* 4. CONNACK Short Packet (rem_len < 2) */
    mock_port_reset();
    PT_INIT(&pt);
    syn_mqtt_init(&c, "broker.hivemq.com", 1883, "myclient", NULL, NULL, 60, rx, sizeof(rx), tx,
                  sizeof(tx));
    /* Connect first */
    syn_mqtt_task(&pt, &task);
    TEST_ASSERT_EQUAL(SYN_MQTT_CONNECTING, c.state);
    /* Set short CONNACK response */
    uint8_t connack_short[] = {0x20, 0x01, 0x00};
    mock_sock_set_response(connack_short, sizeof(connack_short));
    syn_mqtt_task(&pt, &task);
    TEST_ASSERT_EQUAL(SYN_MQTT_DISCONNECTED, c.state);
    TEST_ASSERT_EQUAL(SYN_SOCKET_INVALID, c.sock);

    /* 5. CONNACK Non-Zero Code */
    mock_port_reset();
    PT_INIT(&pt);
    syn_mqtt_init(&c, "broker.hivemq.com", 1883, "myclient", NULL, NULL, 60, rx, sizeof(rx), tx,
                  sizeof(tx));
    syn_mqtt_task(&pt, &task);
    TEST_ASSERT_EQUAL(SYN_MQTT_CONNECTING, c.state);
    uint8_t connack_err[] = {0x20, 0x02, 0x00,
                             0x05}; /* Connection Refused: bad username/password */
    mock_sock_set_response(connack_err, sizeof(connack_err));
    syn_mqtt_task(&pt, &task);
    TEST_ASSERT_EQUAL(SYN_MQTT_DISCONNECTED, c.state);
    TEST_ASSERT_EQUAL(SYN_SOCKET_INVALID, c.sock);

    /* 6. Socket EOF Closure (n == 0 during poll) */
    mock_port_reset();
    PT_INIT(&pt);
    syn_mqtt_init(&c, "broker.hivemq.com", 1883, "myclient", NULL, NULL, 60, rx, sizeof(rx), tx,
                  sizeof(tx));
    c.state = SYN_MQTT_CONNECTED;
    c.sock = 11;
    mock_sock_connected = true;
    mock_sock_eof_on_empty = true;
    syn_mqtt_task(&pt, &task);
    TEST_ASSERT_EQUAL(SYN_MQTT_DISCONNECTED, c.state);
    TEST_ASSERT_EQUAL(SYN_SOCKET_INVALID, c.sock);

    /* 7. Transmit Failures for Publish/Subscribe */
    mock_port_reset();
    syn_mqtt_init(&c, "broker.hivemq.com", 1883, "myclient", NULL, NULL, 60, rx, sizeof(rx), tx,
                  sizeof(tx));
    c.state = SYN_MQTT_CONNECTED;
    c.sock = 11;
    mock_sock_connected = true;
    mock_sock_send_fail = true;
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_mqtt_publish(&c, "temp", "23.5", 4, 1, false));
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_mqtt_subscribe(&c, "temp", 1));
}

static void test_mqtt_payload_skip_logic(void)
{
    SYN_PT pt;
    PT_INIT(&pt);
    SYN_Task task = {0};

    /* 1. read_all fails (socket EOF on valid remaining length) */
    mock_port_reset();
    SYN_MqttClient c;
    uint8_t rx[32];
    uint8_t tx[32];
    syn_mqtt_init(&c, "broker.hivemq.com", 1883, "myclient", NULL, NULL, 60, rx, sizeof(rx), tx,
                  sizeof(tx));
    c.state = SYN_MQTT_CONNECTED;
    c.sock = 11;
    task.user_data = &c;
    mock_sock_connected = true;
    mock_sock_eof_on_empty = true;

    /* Rem Len = 10, but only 5 bytes provided */
    uint8_t short_packet[] = {0x30, 10, 'a', 'b', 'c', 'd', 'e'};
    mock_sock_set_response(short_packet, sizeof(short_packet));
    syn_mqtt_task(&pt, &task);

    /* Should disconnect because socket closed (EOF) before payload complete */
    TEST_ASSERT_EQUAL(SYN_MQTT_DISCONNECTED, c.state);
    TEST_ASSERT_EQUAL(SYN_SOCKET_INVALID, c.sock);

    /* 2. Successful skip logic (packet exceeds rx_buf_size) */
    mock_port_reset();
    PT_INIT(&pt);
    syn_mqtt_init(&c, "broker.hivemq.com", 1883, "myclient", NULL, NULL, 60, rx, sizeof(rx), tx,
                  sizeof(tx));
    c.state = SYN_MQTT_CONNECTED;
    c.sock = 11;
    task.user_data = &c;
    mock_sock_connected = true;

    /* rx_buf_size is 32. We send rem_len = 100.
       Total payload = 100 bytes. Header = 0x30. */
    uint8_t oversized_packet[102];
    oversized_packet[0] = 0x30;
    oversized_packet[1] = 100; // rem_len
    memset(oversized_packet + 2, 'X', 100);
    mock_sock_set_response(oversized_packet, sizeof(oversized_packet));
    syn_mqtt_task(&pt, &task);
    /* Should successfully skip and remain connected */
    TEST_ASSERT_EQUAL(SYN_MQTT_CONNECTED, c.state);
    TEST_ASSERT_EQUAL_INT(102, mock_sock_rx_pos); // read all bytes

    /* 3. Partial oversized packet (packet exceeds rx_buf_size, but socket fails/ends early) */
    mock_port_reset();
    PT_INIT(&pt);
    syn_mqtt_init(&c, "broker.hivemq.com", 1883, "myclient", NULL, NULL, 60, rx, sizeof(rx), tx,
                  sizeof(tx));
    c.state = SYN_MQTT_CONNECTED;
    c.sock = 11;
    task.user_data = &c;
    mock_sock_connected = true;

    /* rx_buf_size is 32. We send rem_len = 100, but only provide 50 bytes of payload */
    uint8_t partial_oversized[52];
    partial_oversized[0] = 0x30;
    partial_oversized[1] = 100; // rem_len
    memset(partial_oversized + 2, 'X', 50);
    mock_sock_set_response(partial_oversized, sizeof(partial_oversized));
    syn_mqtt_task(&pt, &task);
    /* Client stays connected while waiting for remaining bytes */
    TEST_ASSERT_EQUAL(SYN_MQTT_CONNECTED, c.state);

    /* 4. read_remaining_len fails (EOF before length bytes) */
    mock_port_reset();
    PT_INIT(&pt);
    syn_mqtt_init(&c, "broker.hivemq.com", 1883, "myclient", NULL, NULL, 60, rx, sizeof(rx), tx,
                  sizeof(tx));
    c.state = SYN_MQTT_CONNECTED;
    c.sock = 11;
    task.user_data = &c;
    mock_sock_connected = true;
    mock_sock_eof_on_empty = true;

    /* Header 0x30, but no length bytes follow */
    uint8_t header_only[] = {0x30};
    mock_sock_set_response(header_only, sizeof(header_only));
    syn_mqtt_task(&pt, &task);
    /* Should close socket and disconnect on EOF */
    TEST_ASSERT_EQUAL(SYN_MQTT_DISCONNECTED, c.state);
    TEST_ASSERT_EQUAL(SYN_SOCKET_INVALID, c.sock);
}

static void test_mqtt_ping_and_mismatch(void)
{
    /* 1. Keep Alive Ping Trigger */
    mock_port_reset();
    SYN_MqttClient c;
    uint8_t rx[256];
    uint8_t tx[256];
    syn_mqtt_init(&c, "broker.hivemq.com", 1883, "myclient", NULL, NULL, 5, rx, sizeof(rx), tx,
                  sizeof(tx));
    c.state = SYN_MQTT_CONNECTED;
    c.sock = 11;
    mock_sock_connected = true;
    c.last_activity_ms = mock_tick_ms;

    SYN_PT pt;
    PT_INIT(&pt);
    SYN_Task task = {0};
    task.user_data = &c;

    mock_tick_advance(5000);
    mock_sock_tx_len = 0;
    syn_mqtt_task(&pt, &task);
    /* Should send ping (0xC0, 0x00) and update activity time */
    TEST_ASSERT_EQUAL(SYN_MQTT_CONNECTED, c.state);
    TEST_ASSERT_EQUAL(2, mock_sock_tx_len);
    TEST_ASSERT_EQUAL_UINT8(0xC0, mock_sock_tx_buf[0]);
    TEST_ASSERT_EQUAL(mock_tick_ms, c.last_activity_ms);

    /* Resume task after 6000ms delay with send failure (covers lines 544 & 624) */
    mock_tick_advance(6000);
    mock_sock_send_fail = true;
    syn_mqtt_task(&pt, &task);
    TEST_ASSERT_EQUAL(SYN_MQTT_DISCONNECTED, c.state);

    /* 2. Ping Transmit Failure */
    mock_port_reset();
    PT_INIT(&pt);
    syn_mqtt_init(&c, "broker.hivemq.com", 1883, "myclient", NULL, NULL, 5, rx, sizeof(rx), tx,
                  sizeof(tx));
    c.state = SYN_MQTT_CONNECTED;
    c.sock = 11;
    mock_sock_connected = true;
    c.last_activity_ms = mock_tick_ms;

    mock_tick_advance(5000);
    mock_sock_send_fail = true;
    syn_mqtt_task(&pt, &task);
    /* Should close socket and disconnect */
    TEST_ASSERT_EQUAL(SYN_MQTT_DISCONNECTED, c.state);
    TEST_ASSERT_EQUAL(SYN_SOCKET_INVALID, c.sock);

    /* Step task when SYN_MQTT_DISCONNECTED -> line 609 */
    syn_mqtt_task(&pt, &task);

    /* 3. PINGRESP (0xD0) safe handling */
    mock_port_reset();
    PT_INIT(&pt);
    syn_mqtt_init(&c, "broker.hivemq.com", 1883, "myclient", NULL, NULL, 60, rx, sizeof(rx), tx,
                  sizeof(tx));
    c.state = SYN_MQTT_CONNECTED;
    c.sock = 11;
    mock_sock_connected = true;

    uint8_t pingresp[] = {0xD0, 0x00};
    mock_sock_set_response(pingresp, sizeof(pingresp));
    syn_mqtt_task(&pt, &task);
    TEST_ASSERT_EQUAL(SYN_MQTT_CONNECTED, c.state);

    /* 4. QoS 1 PUBACK Mismatch */
    mock_port_reset();
    PT_INIT(&pt);
    syn_mqtt_init(&c, "broker.hivemq.com", 1883, "myclient", NULL, NULL, 60, rx, sizeof(rx), tx,
                  sizeof(tx));
    c.state = SYN_MQTT_CONNECTED;
    c.sock = 11;
    mock_sock_connected = true;
    c.pending_puback_id = 100;

    /* A. Send wrong packet ID (200) */
    uint8_t puback_wrong[] = {0x40, 0x02, 0x00, 0xC8}; // PID = 200
    mock_sock_set_response(puback_wrong, sizeof(puback_wrong));
    syn_mqtt_task(&pt, &task);
    TEST_ASSERT_EQUAL(100, c.pending_puback_id); // should remain 100

    /* B. Send correct packet ID (100) */
    PT_INIT(&pt);
    uint8_t puback_correct[] = {0x40, 0x02, 0x00, 0x64}; // PID = 100
    mock_sock_set_response(puback_correct, sizeof(puback_correct));
    syn_mqtt_task(&pt, &task);
    TEST_ASSERT_EQUAL(0, c.pending_puback_id); // should be cleared
}

static void test_mqtt_pt_end(void)
{
    mock_port_reset();
    SYN_MqttClient c;
    uint8_t rx[32];
    uint8_t tx[32];
    syn_mqtt_init(&c, "broker.hivemq.com", 1883, "myclient", NULL, NULL, 60, rx, sizeof(rx), tx,
                  sizeof(tx));

    SYN_PT pt;
    PT_INIT(&pt);
    SYN_Task task = {0};
    task.user_data = &c;

    /* Set LC to a line that is not a yield switch case to trigger the fallthrough to PT_END */
    pt.lc = 9999;
    SYN_PT_Status status = syn_mqtt_task(&pt, &task);
    TEST_ASSERT_EQUAL(PT_ENDED, status);
}

static void test_mqtt_explicit_ping(void)
{
    mock_port_reset();
    SYN_MqttClient c;
    uint8_t rx[32], tx[32];
    syn_mqtt_init(&c, "broker", 1883, "client", NULL, NULL, 60, rx, sizeof(rx), tx, sizeof(tx));

    TEST_ASSERT_EQUAL(SYN_ERROR, syn_mqtt_ping(&c));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_mqtt_ping(NULL));

    c.state = SYN_MQTT_CONNECTED;
    c.sock = 1;
    mock_sock_connected = true;
    TEST_ASSERT_EQUAL(SYN_OK, syn_mqtt_ping(&c));
}

static void test_mqtt_mid_packet_rx_stalling_timeout(void)
{
    mock_port_reset();
    SYN_MqttClient c;
    uint8_t rx[32], tx[32];
    syn_mqtt_init(&c, "broker", 1883, "client", NULL, NULL, 60, rx, sizeof(rx), tx, sizeof(tx));

    c.state = SYN_MQTT_CONNECTED;
    c.sock = 10;
    c.rx_phase = SYN_MQTT_RX_REMAINING_LEN;
    c.rx_deadline = 100;

    mock_tick_ms = 200; /* Past deadline */

    SYN_PT pt;
    PT_INIT(&pt);

    SYN_Task task = {0};
    task.user_data = &c;

    syn_mqtt_task(&pt, &task);
    TEST_ASSERT_EQUAL(SYN_MQTT_DISCONNECTED, c.state);
    TEST_ASSERT_EQUAL(SYN_SOCKET_INVALID, c.sock);
    TEST_ASSERT_EQUAL(SYN_MQTT_RX_IDLE, c.rx_phase);
}

static void test_mqtt_connection_closed_during_payload_discard(void)
{
    mock_port_reset();
    SYN_MqttClient c;
    uint8_t rx[32], tx[32];
    syn_mqtt_init(&c, "broker", 1883, "client", NULL, NULL, 60, rx, sizeof(rx), tx, sizeof(tx));

    c.state = SYN_MQTT_CONNECTED;
    c.sock = 10;
    c.rx_phase = SYN_MQTT_RX_DISCARD;
    c.rx_rem_len = 100;
    c.rx_pos = 0;

    mock_sock_connected = false; /* Simulate socket closed by remote */

    SYN_PT pt;
    PT_INIT(&pt);
    SYN_Task task = {.user_data = &c};

    syn_mqtt_task(&pt, &task);
    TEST_ASSERT_EQUAL(SYN_MQTT_DISCONNECTED, c.state);
    TEST_ASSERT_EQUAL(SYN_SOCKET_INVALID, c.sock);
    TEST_ASSERT_EQUAL(SYN_MQTT_RX_IDLE, c.rx_phase);
}

static void test_mqtt_topic_and_client_id_null_or_empty(void)
{
    SYN_MqttClient c;
    uint8_t rx[128], tx[128];
    TEST_ASSERT_EQUAL(SYN_OK, syn_mqtt_init(&c, "127.0.0.1", 1883, "client", NULL, NULL, 60, rx,
                                            sizeof(rx), tx, sizeof(tx)));
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_mqtt_publish(&c, "", "msg", 3, 0, false));
}

static void test_mqtt_tx_buffer_overflow_in_connect(void)
{
    mock_port_reset();
    SYN_MqttClient c;
    uint8_t rx[32];
    uint8_t small_tx[8];
    syn_mqtt_init(&c, "broker", 1883, "very_long_client_id_name", NULL, NULL, 60, rx, sizeof(rx),
                  small_tx, sizeof(small_tx));

    SYN_PT pt;
    PT_INIT(&pt);
    SYN_Task task = {.user_data = &c};
    syn_mqtt_task(&pt, &task);
    TEST_ASSERT_EQUAL(SYN_MQTT_DISCONNECTED, c.state);
}

static void test_mqtt_rx_phase_disconnect_handling(void)
{
    mock_port_reset();
    SYN_MqttClient c;
    uint8_t rx[32], tx[128];
    syn_mqtt_init(&c, "broker", 1883, "client", NULL, NULL, 60, rx, sizeof(rx), tx, sizeof(tx));
    c.state = SYN_MQTT_CONNECTED;
    c.sock = 10;
    c.rx_phase = SYN_MQTT_RX_REMAINING_LEN;
    mock_sock_connected = false;

    SYN_PT pt;
    PT_INIT(&pt);
    SYN_Task task = {.user_data = &c};
    syn_mqtt_task(&pt, &task);
    TEST_ASSERT_EQUAL(SYN_MQTT_DISCONNECTED, c.state);
}

static void test_mqtt_ping_send_failure_and_packet_id_wraparound(void)
{
    mock_port_reset();
    SYN_MqttClient c;
    uint8_t rx[32], tx[128];
    syn_mqtt_init(&c, "broker", 1883, "client", NULL, NULL, 60, rx, sizeof(rx), tx, sizeof(tx));
    c.state = SYN_MQTT_CONNECTED;
    c.sock = 10;
    mock_sock_send_fail_after_bytes = 0;
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_mqtt_ping(&c));
    mock_sock_send_fail_after_bytes = -1;

    /* Packet ID wraparound in publish & subscribe (lines 403 & 452) */
    mock_port_reset();
    mock_sock_connected = true;
    mock_sock_tx_len = 1000;
    syn_mqtt_init(&c, "broker", 1883, "client", NULL, NULL, 60, rx, sizeof(rx), tx, sizeof(tx));
    c.sock = 10;
    c.state = SYN_MQTT_CONNECTED;
    c.next_packet_id = 65535;
    TEST_ASSERT_EQUAL(SYN_OK, syn_mqtt_subscribe(&c, "test", 1));
    TEST_ASSERT_EQUAL(1, c.next_packet_id);

    /* Test poll_rx invalid socket (line 233) */
    SYN_PT pt;
    PT_INIT(&pt);
    SYN_Task task = {.user_data = &c};
    c.sock = SYN_SOCKET_INVALID;
    syn_mqtt_task(&pt, &task);

    /* Test DISCARD phase EOF (lines 343-347) */
    PT_INIT(&pt);
    c.sock = 10;
    c.state = SYN_MQTT_CONNECTED;
    c.rx_phase = SYN_MQTT_RX_DISCARD;
    c.rx_rem_len = 50;
    c.rx_pos = 0;
    mock_sock_connected = true;
    mock_sock_rx_len = 0;
    mock_sock_rx_pos = 0;
    mock_sock_eof_on_empty = true;
    syn_mqtt_task(&pt, &task);
    TEST_ASSERT_EQUAL(SYN_SOCKET_INVALID, c.sock);
    TEST_ASSERT_EQUAL(SYN_MQTT_DISCONNECTED, c.state);
    TEST_ASSERT_EQUAL(SYN_MQTT_RX_IDLE, c.rx_phase);

    /* Test next_packet_id wrap around (line 403) */
    c.sock = 10;
    c.state = SYN_MQTT_CONNECTED;
    c.next_packet_id = 65535;
    syn_mqtt_publish(&c, "t", "m", 1, 1, false);
    TEST_ASSERT_EQUAL_UINT16(1, c.next_packet_id);

    /* Test PAYLOAD phase non-blocking recv -1 (line 314) */
    PT_INIT(&pt);
    c.sock = 10;
    c.state = SYN_MQTT_CONNECTED;
    c.rx_phase = SYN_MQTT_RX_PAYLOAD;
    c.rx_rem_len = 10;
    c.rx_pos = 0;
    c.rx_deadline = mock_tick_ms + 10000;
    mock_sock_connected = true;
    mock_sock_rx_len = 0;
    mock_sock_rx_pos = 0;
    mock_sock_eof_on_empty = false;
    syn_mqtt_task(&pt, &task);
    TEST_ASSERT_EQUAL(SYN_MQTT_CONNECTED, c.state);

    /* Test DISCARD phase socket EOF (lines 341-347) */
    PT_INIT(&pt);
    c.sock = 10;
    c.state = SYN_MQTT_CONNECTED;
    c.rx_phase = SYN_MQTT_RX_DISCARD;
    c.rx_rem_len = 100;
    c.rx_pos = 0;
    c.rx_deadline = mock_tick_ms + 10000;
    mock_sock_connected = true;
    mock_sock_rx_len = 0;
    mock_sock_rx_pos = 0;
    mock_sock_eof_on_empty = true;
    syn_mqtt_task(&pt, &task);
    TEST_ASSERT_EQUAL(SYN_MQTT_DISCONNECTED, c.state);
}

void test_mqtt_disconnect(void)
{
    mock_port_reset();

    /* NULL check */
    syn_mqtt_disconnect(NULL);

    SYN_MqttClient c;
    uint8_t rx[256];
    uint8_t tx[256];
    syn_mqtt_init(&c, "broker.example.com", 1883, "client1", NULL, NULL, 60, rx, sizeof(rx), tx,
                  sizeof(tx));

    /* Disconnect when disconnected (no-op) */
    syn_mqtt_disconnect(&c);
    TEST_ASSERT_EQUAL(SYN_MQTT_DISCONNECTED, c.state);

    /* Disconnect when connected with stale QoS state */
    c.state = SYN_MQTT_CONNECTED;
    c.sock = 10;
    c.pending_puback_id = 42;
    c.retransmit_len = 16;
    mock_sock_connected = true;
    syn_mqtt_disconnect(&c);

    TEST_ASSERT_EQUAL(SYN_MQTT_DISCONNECTED, c.state);
    TEST_ASSERT_EQUAL(SYN_SOCKET_INVALID, c.sock);
    TEST_ASSERT_EQUAL_UINT8(0xE0, mock_sock_tx_buf[0]); /* DISCONNECT packet */
    TEST_ASSERT_EQUAL(0, c.pending_puback_id);          /* stale QoS cleared */
    TEST_ASSERT_EQUAL(0, c.retransmit_len);             /* stale retransmit cleared */
}

static void test_mqtt_qos2_handshake_and_keepalive_ping(void)
{
    /* 1. mqtt_has_work(NULL) -> line 531 */
    SYN_Task null_task = {.user_data = NULL};
    SYN_PT pt;
    PT_INIT(&pt);
    TEST_ASSERT_EQUAL(PT_EXITED, syn_mqtt_task(&pt, &null_task));

    /* 2. mqtt_has_work keepalive timeout check -> line 544 */
    SYN_MqttClient c;
    uint8_t rx[128], tx[128];
    syn_mqtt_init(&c, "broker.hivemq.com", 1883, "myclient", NULL, NULL, 5, rx, sizeof(rx), tx,
                  sizeof(tx));
    c.state = SYN_MQTT_CONNECTED;
    c.sock = 10;
    mock_sock_connected = true;
    c.last_activity_ms = mock_tick_ms;

    mock_tick_advance(6000);
    PT_INIT(&pt);
    SYN_Task task = {.user_data = &c};
    syn_mqtt_task(&pt, &task);
    TEST_ASSERT_EQUAL(SYN_MQTT_CONNECTED, c.state);

    /* 3. Disconnected task delay loop -> line 608 */
    c.state = SYN_MQTT_DISCONNECTED;
    c.sock = SYN_SOCKET_INVALID;
    PT_INIT(&pt);
    TEST_ASSERT_EQUAL(PT_IDLE, syn_mqtt_task(&pt, &task));

    /* 4. Keepalive ping send failure -> lines 618-624 */
    c.state = SYN_MQTT_CONNECTED;
    c.sock = 10;
    mock_sock_connected = true;
    mock_sock_send_fail = true;
    c.last_activity_ms = mock_tick_ms;
    mock_tick_advance(6000);
    PT_INIT(&pt);
    syn_mqtt_task(&pt, &task);
    TEST_ASSERT_EQUAL(SYN_MQTT_DISCONNECTED, c.state);
}

void test_mqtt_subscribe_does_not_corrupt_retransmit_buffer(void)
{
    mock_port_reset();

    SYN_MqttClient c;
    uint8_t rx[256];
    uint8_t tx[256];
    syn_mqtt_init(&c, "broker.hivemq.com", 1883, "myclient", NULL, NULL, 60, rx, sizeof(rx), tx,
                  sizeof(tx));
    c.state = SYN_MQTT_CONNECTED;
    c.sock = 11;
    mock_sock_connected = true;

    /* 1. Publish QoS 1 -> populates retransmit_buf with PUBLISH (0x32) */
    const char *payload = "data";
    TEST_ASSERT_EQUAL(SYN_OK,
                      syn_mqtt_publish(&c, "sensors/temp", payload, strlen(payload), 1, false));
    TEST_ASSERT_TRUE(c.retransmit_len > 0);
    TEST_ASSERT_EQUAL_UINT8(0x32, c.retransmit_buf[0]);

    /* 2. Subscribe with QoS 1 -> must NOT overwrite retransmit_buf */
    TEST_ASSERT_EQUAL(SYN_OK, syn_mqtt_subscribe(&c, "sensors/temp", 1));
    TEST_ASSERT_EQUAL_UINT8(0x32, c.retransmit_buf[0]);

    /* 3. Ingest SUBACK (0x90, 0x03, packet_id_hi, packet_id_lo, return_code) */
    uint8_t suback[] = {0x90, 0x03, 0x00, 0x02, 0x01};
    mock_sock_set_response(suback, sizeof(suback));

    SYN_PT pt;
    PT_INIT(&pt);
    SYN_Task task = {.user_data = &c};
    syn_mqtt_task(&pt, &task);
    TEST_ASSERT_EQUAL(SYN_MQTT_CONNECTED, c.state);

    /* 4. Ingest PUBACK (0x40, 0x02, packet_id_hi, packet_id_lo) -> clears retransmit_len */
    uint8_t puback[] = {0x40, 0x02, (uint8_t)(c.pending_puback_id >> 8),
                        (uint8_t)(c.pending_puback_id & 0xFF)};
    mock_sock_set_response(puback, sizeof(puback));
    syn_mqtt_task(&pt, &task);
    TEST_ASSERT_EQUAL(0, c.pending_puback_id);
    TEST_ASSERT_EQUAL(0, c.retransmit_len);
}

static void test_mqtt5_varint_and_properties(void)
{
    /* 1. Variable Byte Integer boundary encoding */
    uint8_t buf[4];
    TEST_ASSERT_EQUAL_UINT(1, syn_mqtt_encode_varint(0, buf));
    TEST_ASSERT_EQUAL_HEX8(0x00, buf[0]);

    TEST_ASSERT_EQUAL_UINT(1, syn_mqtt_encode_varint(127, buf));
    TEST_ASSERT_EQUAL_HEX8(0x7F, buf[0]);

    TEST_ASSERT_EQUAL_UINT(2, syn_mqtt_encode_varint(128, buf));
    TEST_ASSERT_EQUAL_HEX8(0x80, buf[0]);
    TEST_ASSERT_EQUAL_HEX8(0x01, buf[1]);

    TEST_ASSERT_EQUAL_UINT(2, syn_mqtt_encode_varint(16383, buf));
    TEST_ASSERT_EQUAL_HEX8(0xFF, buf[0]);
    TEST_ASSERT_EQUAL_HEX8(0x7F, buf[1]);

    TEST_ASSERT_EQUAL_UINT(3, syn_mqtt_encode_varint(16384, buf));
    TEST_ASSERT_EQUAL_HEX8(0x80, buf[0]);
    TEST_ASSERT_EQUAL_HEX8(0x80, buf[1]);
    TEST_ASSERT_EQUAL_HEX8(0x01, buf[2]);

    TEST_ASSERT_EQUAL_UINT(4, syn_mqtt_encode_varint(268435455, buf));
    TEST_ASSERT_EQUAL_HEX8(0xFF, buf[0]);
    TEST_ASSERT_EQUAL_HEX8(0xFF, buf[1]);
    TEST_ASSERT_EQUAL_HEX8(0xFF, buf[2]);
    TEST_ASSERT_EQUAL_HEX8(0x7F, buf[3]);

    /* Varint overflow rejection */
    TEST_ASSERT_EQUAL_UINT(0, syn_mqtt_encode_varint(268435456, buf));
    TEST_ASSERT_EQUAL_UINT(0, syn_mqtt_encode_varint(0, NULL));

    /* 2. Variable Byte Integer decoding */
    uint32_t val = 0;
    size_t bread = 0;
    TEST_ASSERT_TRUE(syn_mqtt_decode_varint(buf, 4, &val, &bread));
    TEST_ASSERT_EQUAL_UINT32(268435455, val);
    TEST_ASSERT_EQUAL_UINT(4, bread);

    /* Incomplete buffer */
    TEST_ASSERT_FALSE(syn_mqtt_decode_varint(buf, 3, &val, &bread));
    TEST_ASSERT_FALSE(syn_mqtt_decode_varint(NULL, 4, &val, &bread));

    /* 3. User Property encoding */
    uint8_t prop_buf[64];
    size_t prop_len =
        syn_mqtt5_encode_user_prop("device_id", "syn-node-01", prop_buf, sizeof(prop_buf));
    TEST_ASSERT_TRUE(prop_len > 0);
    TEST_ASSERT_EQUAL_HEX8(SYN_MQTT5_PROP_USER_PROPERTY, prop_buf[0]);

    /* Error bounds */
    TEST_ASSERT_EQUAL_UINT(0, syn_mqtt5_encode_user_prop(NULL, "val", prop_buf, sizeof(prop_buf)));
    TEST_ASSERT_EQUAL_UINT(0, syn_mqtt5_encode_user_prop("key", NULL, prop_buf, sizeof(prop_buf)));
    TEST_ASSERT_EQUAL_UINT(
        0, syn_mqtt5_encode_user_prop("key", "val", prop_buf, 4)); /* Small buffer */
}

static void test_mqtt_has_work_wakeup_and_sustained_rx_yielding(void)
{
    uint8_t rx[128];
    uint8_t tx[128];
    SYN_MqttClient c;
    SYN_PT pt;

    /* 1. Wake from PT_WAIT_UNTIL when syn_mqtt_disconnect() sets SYN_MQTT_DISCONNECTED
     *    externally (hits syn_mqtt.c:597) */
    mock_port_reset();
    syn_mqtt_init(&c, "broker", 1883, "cid", NULL, NULL, 60, rx, sizeof(rx), tx, sizeof(tx));
    c.state = SYN_MQTT_CONNECTED;
    c.sock = 11;
    mock_sock_connected = true;
    c.last_activity_ms = mock_tick_ms;

    SYN_Task task = {0};
    task.user_data = &c;
    PT_INIT(&pt);

    /* Parks at PT_WAIT_UNTIL(pt, mqtt_has_work(c)) on line 711 */
    TEST_ASSERT_EQUAL(PT_IDLE, syn_mqtt_task(&pt, &task));

    /* External disconnect transitions c.state -> SYN_MQTT_DISCONNECTED */
    syn_mqtt_disconnect(&c);
    TEST_ASSERT_EQUAL(SYN_MQTT_DISCONNECTED, c.state);

    /* Next step evaluates mqtt_has_work(c) -> hits line 597, reconnects and sends CONNECT */
    mock_sock_tx_len = 0;
    TEST_ASSERT_EQUAL(PT_IDLE, syn_mqtt_task(&pt, &task));
    TEST_ASSERT_EQUAL(SYN_MQTT_CONNECTING, c.state);
    TEST_ASSERT_TRUE(mock_sock_tx_len > 0);
    TEST_ASSERT_EQUAL_HEX8(0x10, mock_sock_tx_buf[0]);

    /* 2. Partial header stalls in SYN_MQTT_RX_REMAINING_LEN, yields at line 711, then
     *    rx_deadline expires -> mqtt_has_work hits line 603 and resets stalled RX parser */
    mock_port_reset();
    mock_tick_ms = 1000;
    syn_mqtt_init(&c, "broker", 1883, "cid", NULL, NULL, 60, rx, sizeof(rx), tx, sizeof(tx));
    c.state = SYN_MQTT_CONNECTED;
    c.sock = 11;
    mock_sock_connected = true;
    c.last_activity_ms = mock_tick_ms;

    uint8_t partial_hdr[1] = {0x30}; /* PUBLISH fixed header byte, no remaining length yet */
    mock_sock_set_response(partial_hdr, sizeof(partial_hdr));

    memset(&task, 0, sizeof(task));
    task.user_data = &c;
    PT_INIT(&pt);

    /* Consumes 1 byte -> enters SYN_MQTT_RX_REMAINING_LEN and parks at line 711 */
    TEST_ASSERT_EQUAL(PT_IDLE, syn_mqtt_task(&pt, &task));
    TEST_ASSERT_EQUAL(SYN_MQTT_RX_REMAINING_LEN, c.rx_phase);

    /* Before rx_deadline -> stays PT_IDLE */
    mock_tick_advance(100);
    TEST_ASSERT_EQUAL(PT_IDLE, syn_mqtt_task(&pt, &task));
    TEST_ASSERT_EQUAL(SYN_MQTT_CONNECTED, c.state);

    /* Advance past rx_deadline (5s < 60s keepalive) -> hits line 603 in mqtt_has_work,
     * wakes task, disconnects stalled socket, and resets rx_phase to IDLE */
    mock_tick_ms = c.rx_deadline + 10U;
    TEST_ASSERT_EQUAL(PT_IDLE, syn_mqtt_task(&pt, &task));
    TEST_ASSERT_EQUAL(SYN_MQTT_DISCONNECTED, c.state);
    TEST_ASSERT_EQUAL(SYN_MQTT_RX_IDLE, c.rx_phase);
    TEST_ASSERT_EQUAL(SYN_SOCKET_INVALID, c.sock);

    /* 3. Sustained TCP RX (>32 poll_rx steps) leaves unread bytes in socket ->
     *    hits PT_CONTINUE_IF_NEEDED + continue at syn_mqtt.c:707-708 */
    mock_port_reset();
    syn_mqtt_init(&c, "broker", 1883, "cid", NULL, NULL, 60, rx, sizeof(rx), tx, sizeof(tx));
    c.state = SYN_MQTT_CONNECTED;
    c.sock = 11;
    mock_sock_connected = true;
    c.last_activity_ms = mock_tick_ms;

    /* 18 back-to-back 2-byte PINGRESP packets = 36 single-byte poll_rx steps (>32 budget) */
    uint8_t burst[36];
    for (size_t i = 0; i < sizeof(burst); i += 2) {
        burst[i] = 0xD0;
        burst[i + 1] = 0x00;
    }
    mock_sock_set_response(burst, sizeof(burst));

    SYN_Sched sched = {0};
    memset(&task, 0, sizeof(task));
    task.user_data = &c;
    task.sched = &sched;
    sched.running = &task;
    sched.share_contended = true;
    PT_INIT(&pt);

    /* First call consumes 32 bytes (16 PINGRESPs), 4 bytes remain -> yields PT_MORE (707) */
    TEST_ASSERT_EQUAL(PT_MORE, syn_mqtt_task(&pt, &task));
    TEST_ASSERT_EQUAL_UINT(32, mock_sock_rx_pos);

    /* Second call resumes at continue (708), consumes final 4 bytes -> yields PT_IDLE */
    TEST_ASSERT_EQUAL(PT_IDLE, syn_mqtt_task(&pt, &task));
    TEST_ASSERT_EQUAL_UINT(36, mock_sock_rx_pos);
    TEST_ASSERT_EQUAL(SYN_MQTT_CONNECTED, c.state);
}

static void test_mqtt_round3_regressions(void)
{
    mock_port_reset();

    static char big_str[65537];
    memset(big_str, 'x', 65536U);
    big_str[65536] = '\0';

    uint8_t rx[128];
    uint8_t tx[32];
    SYN_MqttClient c;
    syn_mqtt_init(&c, "broker", 1883, "cid", NULL, NULL, 60, rx, sizeof(rx), tx, sizeof(tx));
    c.state = SYN_MQTT_CONNECTED;
    c.sock = 11;
    mock_sock_connected = true;

    /* 1. R3-N04: publish/subscribe validate NULL, >65535 topic, >268435455 rem_len, and tx_buf_size
     *    BEFORE mutating packet IDs or QoS 1 retransmit state */
    c.next_packet_id = 7U;
    c.pending_puback_id = 0U;
    c.retransmit_len = 0U;

    TEST_ASSERT_EQUAL(SYN_ERROR, syn_mqtt_publish(NULL, "t", "a", 1, 1, false));
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_mqtt_publish(&c, NULL, "a", 1, 1, false));
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_mqtt_publish(&c, "t", NULL, 4, 1, false));
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_mqtt_publish(&c, big_str, "a", 1, 1, false));
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_mqtt_publish(&c, "t", tx, 268435455U, 1, false));
    /* Payload 40 exceeds tx[32] */
    uint8_t forty[40] = {0};
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_mqtt_publish(&c, "topic", forty, sizeof(forty), 1, true));
    TEST_ASSERT_EQUAL_UINT16(7U, c.next_packet_id);
    TEST_ASSERT_EQUAL_UINT16(0U, c.pending_puback_id);

    /* Valid publish with empty topic "" exercises mqtt_put_str len == 0 branch */
    TEST_ASSERT_EQUAL(SYN_OK, syn_mqtt_publish(&c, "", "ok", 2, 0, true));

    TEST_ASSERT_EQUAL(SYN_ERROR, syn_mqtt_subscribe(NULL, "t", 1));
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_mqtt_subscribe(&c, NULL, 1));
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_mqtt_subscribe(&c, big_str, 1));
    char topic30[31];
    memset(topic30, 't', 30U);
    topic30[30] = '\0';
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_mqtt_subscribe(&c, topic30, 1));
    TEST_ASSERT_EQUAL_UINT16(7U, c.next_packet_id);

    /* 2. R3-N15: varint 4-byte continuation rejection, user prop >65535 & NULL buf, connect >65535
     */
    uint8_t bad_varint[4] = {0x80U, 0x80U, 0x80U, 0x80U};
    uint32_t v_out = 0U;
    size_t b_out = 0U;
    TEST_ASSERT_FALSE(syn_mqtt_decode_varint(bad_varint, 4U, &v_out, &b_out));
    TEST_ASSERT_FALSE(syn_mqtt_decode_varint(bad_varint, 4U, NULL, &b_out));
    TEST_ASSERT_FALSE(syn_mqtt_decode_varint(bad_varint, 4U, &v_out, NULL));
    TEST_ASSERT_FALSE(syn_mqtt_decode_varint(bad_varint, 0U, &v_out, &b_out));

    uint8_t prop_buf[16];
    TEST_ASSERT_EQUAL_UINT(0U, syn_mqtt5_encode_user_prop("k", "v", NULL, 16U));
    TEST_ASSERT_EQUAL_UINT(0U,
                           syn_mqtt5_encode_user_prop(big_str, "v", prop_buf, sizeof(prop_buf)));
    TEST_ASSERT_EQUAL_UINT(0U,
                           syn_mqtt5_encode_user_prop("k", big_str, prop_buf, sizeof(prop_buf)));

    SYN_MqttClient c_big;
    syn_mqtt_init(&c_big, "broker", 1883, big_str, NULL, NULL, 60, rx, sizeof(rx), tx, sizeof(tx));
    SYN_PT pt_big;
    PT_INIT(&pt_big);
    SYN_Task task_big = {.user_data = &c_big};
    TEST_ASSERT_EQUAL(PT_IDLE, syn_mqtt_task(&pt_big, &task_big));
    TEST_ASSERT_EQUAL(SYN_MQTT_DISCONNECTED, c_big.state);
    TEST_ASSERT_EQUAL(SYN_SOCKET_INVALID, c_big.sock);

    /* 3. R3-N06 & R3-N14: readable socket error (mock_sock_recv_fail) resets session & QoS state
     *    without spinning, and advances cleanly across 5000ms reconnect backoff */
    mock_port_reset();
    uint8_t tx256[256];
    syn_mqtt_init(&c, "broker", 1883, "cid", NULL, NULL, 60, rx, sizeof(rx), tx256, sizeof(tx256));
    c.state = SYN_MQTT_CONNECTED;
    c.sock = 11;
    mock_sock_connected = true;
    TEST_ASSERT_EQUAL(SYN_OK, syn_mqtt_publish(&c, "sensors/x", "v", 1, 1, false));
    TEST_ASSERT_NOT_EQUAL(0U, c.pending_puback_id);
    TEST_ASSERT_TRUE(c.retransmit_len > 0U);

    SYN_PT pt;
    PT_INIT(&pt);
    SYN_Task task = {.user_data = &c};
    mock_sock_recv_fail = true;
    TEST_ASSERT_EQUAL(PT_IDLE, syn_mqtt_task(&pt, &task));
    TEST_ASSERT_EQUAL(SYN_MQTT_DISCONNECTED, c.state);
    TEST_ASSERT_EQUAL(SYN_SOCKET_INVALID, c.sock);
    TEST_ASSERT_EQUAL_UINT16(0U, c.pending_puback_id);
    TEST_ASSERT_EQUAL_UINT(0U, c.retransmit_len);

    /* Advance 5000ms so the post-disconnect PT_TASK_DELAY_MS continue path executes */
    mock_sock_recv_fail = false;
    mock_tick_advance(5000U);
    TEST_ASSERT_EQUAL(PT_IDLE, syn_mqtt_task(&pt, &task));
    TEST_ASSERT_EQUAL(SYN_MQTT_CONNECTING, c.state);

    /* 4. R3-N12: mid-flight task restart while CONNECTING closes existing socket on case 0 */
    TEST_ASSERT_TRUE(mock_sock_connected);
    PT_INIT(&pt);
    mock_sock_connect_fail = true;
    TEST_ASSERT_EQUAL(PT_IDLE, syn_mqtt_task(&pt, &task));
    TEST_ASSERT_FALSE(mock_sock_connected);
    TEST_ASSERT_EQUAL(SYN_SOCKET_INVALID, c.sock);
}

static void test_mqtt_round4_regressions(void)
{
    mock_port_reset();

    static char big_str[65537];
    memset(big_str, 'u', 65536U);
    big_str[65536] = '\0';

    uint8_t rx[128];
    uint8_t tx[256];
    SYN_MqttClient c;
    SYN_PT pt;
    SYN_Task task = {0};

    /* 1. R4-N11: password without username must omit password flag (0x40) and password string */
    syn_mqtt_init(&c, "broker", 1883, "cid", NULL, "orphan_pass", 60, rx, sizeof(rx), tx,
                  sizeof(tx));
    task.user_data = &c;
    PT_INIT(&pt);
    mock_sock_tx_len = 0U;
    TEST_ASSERT_EQUAL(PT_IDLE, syn_mqtt_task(&pt, &task));
    TEST_ASSERT_EQUAL(SYN_MQTT_CONNECTING, c.state);
    /* Header: 0x10, rem_len=15 (10 + 2 + 3), "MQTT"(6), lvl=4(1), flags=0x02(1) at index 9 */
    TEST_ASSERT_EQUAL_UINT(17U, mock_sock_tx_len);
    TEST_ASSERT_EQUAL_HEX8(0x02U, mock_sock_tx_buf[9]);

    /* Credential >65535 length guards in mqtt_connect_creds_len */
    syn_mqtt_init(&c, "broker", 1883, "cid", big_str, "pw", 60, rx, sizeof(rx), tx, sizeof(tx));
    PT_INIT(&pt);
    TEST_ASSERT_EQUAL(PT_IDLE, syn_mqtt_task(&pt, &task));
    TEST_ASSERT_EQUAL(SYN_MQTT_DISCONNECTED, c.state);

    syn_mqtt_init(&c, "broker", 1883, "cid", "usr", big_str, 60, rx, sizeof(rx), tx, sizeof(tx));
    PT_INIT(&pt);
    TEST_ASSERT_EQUAL(PT_IDLE, syn_mqtt_task(&pt, &task));
    TEST_ASSERT_EQUAL(SYN_MQTT_DISCONNECTED, c.state);

    /* 2. R4-N11: valid 4-byte remaining length (2,097,152) enters DISCARD without resetting */
    mock_port_reset();
    syn_mqtt_init(&c, "broker", 1883, "cid", NULL, NULL, 60, rx, sizeof(rx), tx, sizeof(tx));
    c.state = SYN_MQTT_CONNECTED;
    c.sock = 12;
    mock_sock_connected = true;
    uint8_t rem4_hdr[] = {0x30U, 0x80U, 0x80U, 0x80U, 0x01U};
    mock_sock_set_response(rem4_hdr, sizeof(rem4_hdr));
    PT_INIT(&pt);
    TEST_ASSERT_EQUAL(PT_IDLE, syn_mqtt_task(&pt, &task));
    TEST_ASSERT_EQUAL(SYN_MQTT_CONNECTED, c.state);
    TEST_ASSERT_EQUAL(SYN_MQTT_RX_DISCARD, c.rx_phase);
    TEST_ASSERT_EQUAL_UINT32(2097152U, c.rx_rem_len);

    /* 3. R4-N11: publish/subscribe QoS bounds & single in-flight QoS 1 slot + uncached timeout */
    mock_port_reset();
    syn_mqtt_init(&c, "broker", 1883, "cid", NULL, NULL, 60, rx, sizeof(rx), tx, sizeof(tx));
    c.state = SYN_MQTT_CONNECTED;
    c.sock = 12;
    mock_sock_connected = true;

    TEST_ASSERT_EQUAL(SYN_ERROR, syn_mqtt_publish(&c, "t", "x", 1U, 2U, false));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_mqtt_subscribe(&c, "t", 2U));
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_mqtt_subscribe(&c, "t", 3U));

    uint8_t uncached_payload[130];
    memset(uncached_payload, 'Q', sizeof(uncached_payload));
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_mqtt_publish(&c, "big", uncached_payload,
                                                  sizeof(uncached_payload), 1U, false));
    TEST_ASSERT_EQUAL_UINT16(0U, c.pending_puback_id);
    TEST_ASSERT_EQUAL_UINT(0U, c.retransmit_len);

    /* Valid cached QoS 1 publish locks slot; concurrent QoS 1 rejected while QoS 0 succeeds */
    TEST_ASSERT_EQUAL(SYN_OK, syn_mqtt_publish(&c, "small", "a", 1U, 1U, false));
    TEST_ASSERT_NOT_EQUAL(0U, c.pending_puback_id);
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_mqtt_publish(&c, "small", "b", 1U, 1U, false));
    TEST_ASSERT_EQUAL(SYN_OK, syn_mqtt_publish(&c, "small", "b", 1U, 0U, false));

    /* 4. R4-N11: keep-alive PINGREQ arms ping_pending, PINGRESP clears it, and missing PINGRESP
     *    resets session after MQTT_ACK_TIMEOUT_MS */
    mock_port_reset();
    syn_mqtt_init(&c, "broker", 1883, "cid", NULL, NULL, 2, rx, sizeof(rx), tx, sizeof(tx));
    c.state = SYN_MQTT_CONNECTED;
    c.sock = 12;
    mock_sock_connected = true;
    c.last_activity_ms = mock_tick_ms;
    PT_INIT(&pt);

    TEST_ASSERT_EQUAL(PT_IDLE, syn_mqtt_task(&pt, &task));
    mock_tick_advance(2000U);
    mock_sock_tx_len = 0U;
    TEST_ASSERT_EQUAL(PT_IDLE, syn_mqtt_task(&pt, &task));
    TEST_ASSERT_TRUE(c.ping_pending);
    TEST_ASSERT_EQUAL_UINT(2U, mock_sock_tx_len);

    /* Unrelated RX at +3s (> 2s keep-alive, < 5s timeout) wakes task without duplicate PINGREQ */
    mock_tick_advance(3000U);
    uint8_t suback_noise[] = {0x90U, 0x03U, 0x00U, 0x01U, 0x00U};
    mock_sock_set_response(suback_noise, sizeof(suback_noise));
    TEST_ASSERT_EQUAL(PT_IDLE, syn_mqtt_task(&pt, &task));
    TEST_ASSERT_TRUE(c.ping_pending);
    TEST_ASSERT_EQUAL_UINT(2U, mock_sock_tx_len);
    TEST_ASSERT_EQUAL(SYN_MQTT_CONNECTED, c.state);

    /* PINGRESP arrives at +4s (< 5s timeout) -> clears ping_pending */
    uint8_t pingresp[] = {0xD0U, 0x00U};
    mock_sock_set_response(pingresp, sizeof(pingresp));
    TEST_ASSERT_EQUAL(PT_IDLE, syn_mqtt_task(&pt, &task));
    TEST_ASSERT_FALSE(c.ping_pending);
    TEST_ASSERT_EQUAL(SYN_MQTT_CONNECTED, c.state);

    /* Next PINGREQ with no PINGRESP -> wakes at +5000ms and resets session */
    mock_tick_advance(2000U);
    TEST_ASSERT_EQUAL(PT_IDLE, syn_mqtt_task(&pt, &task));
    TEST_ASSERT_TRUE(c.ping_pending);
    mock_tick_advance(5000U);
    TEST_ASSERT_EQUAL(PT_IDLE, syn_mqtt_task(&pt, &task));
    TEST_ASSERT_EQUAL(SYN_MQTT_DISCONNECTED, c.state);
    TEST_ASSERT_FALSE(c.ping_pending);
}

static void test_r5_n11_mqtt_qos1_retransmit_bounds_and_sub_qos2(void)
{
    mock_port_reset();
    uint8_t rx[128];
    uint8_t tx[256];
    SYN_MqttClient c;
    syn_mqtt_init(&c, "broker", 1883, "cid", NULL, NULL, 60, rx, sizeof(rx), tx, sizeof(tx));
    c.state = SYN_MQTT_CONNECTED;
    c.sock = 15;
    mock_sock_connected = true;
    c.next_packet_id = 41U;

    /* Exact 128B QoS 1 PUBLISH: 1(hdr) + 2(varint=126? no: rem_len=126 -> 1B varint, total=128B!)
     * Topic "t" (len=1): hdr_rem = 2 + 1 + 2 = 5.
     * Payload = 121 -> rem_len = 126 (1-byte varint) -> total_len = 1 + 1 + 126 = 128B (fits!).
     * Payload = 122 -> rem_len = 127 (1-byte varint) -> total_len = 1 + 1 + 127 = 129B (exceeds!).
     */
    uint8_t payload[122];
    memset(payload, 'M', sizeof(payload));

    mock_sock_tx_len = 0U;
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_mqtt_publish(&c, "t", payload, 122U, 1U, false));
    TEST_ASSERT_EQUAL_UINT(0U, mock_sock_tx_len);
    TEST_ASSERT_EQUAL_UINT16(41U, c.next_packet_id);
    TEST_ASSERT_EQUAL_UINT16(0U, c.pending_puback_id);

    TEST_ASSERT_EQUAL(SYN_OK, syn_mqtt_publish(&c, "t", payload, 121U, 1U, false));
    TEST_ASSERT_EQUAL_UINT(128U, mock_sock_tx_len);
    TEST_ASSERT_EQUAL_UINT(128U, c.retransmit_len);
    TEST_ASSERT_EQUAL_UINT16(42U, c.pending_puback_id);

    /* Subscribe rejects QoS 2 with SYN_INVALID_PARAM without transmitting */
    mock_sock_tx_len = 0U;
    TEST_ASSERT_EQUAL(SYN_INVALID_PARAM, syn_mqtt_subscribe(&c, "sensors/#", 2U));
    TEST_ASSERT_EQUAL_UINT(0U, mock_sock_tx_len);
    TEST_ASSERT_EQUAL(SYN_OK, syn_mqtt_subscribe(&c, "sensors/#", 1U));
    TEST_ASSERT_TRUE(mock_sock_tx_len > 0U);
}

static void test_tier_a_mqtt_connack_send_fail_and_qos0_dup(void)
{
    uint8_t rx[128];
    uint8_t tx[256];
    SYN_MqttClient c;
    SYN_PT pt;
    SYN_Task task = {.user_data = &c};

    /* 1. Duplicate CONNACK while already CONNECTED resets session */
    mock_port_reset();
    syn_mqtt_init(&c, "broker", 1883, "cid", NULL, NULL, 60, rx, sizeof(rx), tx, sizeof(tx));
    c.state = SYN_MQTT_CONNECTED;
    c.sock = 11;
    mock_sock_connected = true;
    const uint8_t dup_connack[] = {0x20U, 0x02U, 0x00U, 0x00U};
    mock_sock_set_response(dup_connack, sizeof(dup_connack));
    PT_INIT(&pt);
    TEST_ASSERT_EQUAL(PT_IDLE, syn_mqtt_task(&pt, &task));
    TEST_ASSERT_EQUAL(SYN_MQTT_DISCONNECTED, c.state);
    TEST_ASSERT_EQUAL(SYN_SOCKET_INVALID, c.sock);

    /* 2. Malformed CONNACKs while CONNECTING (bad header flags, rem_len != 2, ack flags != 0) */
    const uint8_t bad_connacks[3][5] = {
        {0x21U, 0x02U, 0x00U, 0x00U, 0x00U}, /* Non-zero fixed header flags */
        {0x20U, 0x03U, 0x00U, 0x00U, 0x00U}, /* Remaining length == 3 */
        {0x20U, 0x02U, 0x02U, 0x00U, 0x00U}, /* Reserved ack flags (0x02 & 0xFE != 0) */
    };
    const size_t bad_lens[3] = {4U, 5U, 4U};
    for (size_t i = 0U; i < 3U; i++) {
        mock_port_reset();
        syn_mqtt_init(&c, "broker", 1883, "cid", NULL, NULL, 60, rx, sizeof(rx), tx, sizeof(tx));
        PT_INIT(&pt);
        TEST_ASSERT_EQUAL(PT_IDLE, syn_mqtt_task(&pt, &task));
        TEST_ASSERT_EQUAL(SYN_MQTT_CONNECTING, c.state);
        mock_sock_set_response(bad_connacks[i], bad_lens[i]);
        TEST_ASSERT_EQUAL(PT_IDLE, syn_mqtt_task(&pt, &task));
        TEST_ASSERT_EQUAL(SYN_MQTT_DISCONNECTED, c.state);
        TEST_ASSERT_EQUAL(SYN_SOCKET_INVALID, c.sock);
    }

    /* 3. Publish / Subscribe / QoS 1 retransmit update last_activity_ms on success and
     *    disconnect broken socket on transport write failure */
    mock_port_reset();
    syn_mqtt_init(&c, "broker", 1883, "cid", NULL, NULL, 60, rx, sizeof(rx), tx, sizeof(tx));
    c.state = SYN_MQTT_CONNECTED;
    c.sock = 11;
    mock_sock_connected = true;
    mock_tick_ms = 1234U;
    TEST_ASSERT_EQUAL(SYN_OK, syn_mqtt_publish(&c, "sensors/t", "v", 1U, 1U, false));
    TEST_ASSERT_EQUAL_UINT32(1234U, c.last_activity_ms);

    mock_tick_ms = 2345U;
    TEST_ASSERT_EQUAL(SYN_OK, syn_mqtt_subscribe(&c, "sensors/#", 1U));
    TEST_ASSERT_EQUAL_UINT32(2345U, c.last_activity_ms);

    /* Retransmit failure at +5000ms resets session without updating last_activity_ms */
    PT_INIT(&pt);
    mock_tick_ms = 1234U + 5000U;
    mock_sock_send_fail = true;
    TEST_ASSERT_EQUAL(PT_IDLE, syn_mqtt_task(&pt, &task));
    TEST_ASSERT_EQUAL(SYN_MQTT_DISCONNECTED, c.state);
    TEST_ASSERT_EQUAL(SYN_SOCKET_INVALID, c.sock);
    TEST_ASSERT_EQUAL_UINT32(2345U, c.last_activity_ms);

    /* Direct publish failure while CONNECTED resets session */
    mock_port_reset();
    syn_mqtt_init(&c, "broker", 1883, "cid", NULL, NULL, 60, rx, sizeof(rx), tx, sizeof(tx));
    c.state = SYN_MQTT_CONNECTED;
    c.sock = 11;
    c.last_activity_ms = 500U;
    mock_sock_connected = true;
    mock_sock_send_fail = true;
    mock_tick_ms = 900U;
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_mqtt_publish(&c, "t", "x", 1U, 0U, false));
    TEST_ASSERT_EQUAL(SYN_MQTT_DISCONNECTED, c.state);
    TEST_ASSERT_EQUAL(SYN_SOCKET_INVALID, c.sock);
    TEST_ASSERT_EQUAL_UINT32(500U, c.last_activity_ms);

    /* Direct subscribe failure while CONNECTED resets session */
    c.state = SYN_MQTT_CONNECTED;
    c.sock = 12;
    mock_sock_connected = true;
    mock_sock_send_fail = true;
    TEST_ASSERT_EQUAL(SYN_ERROR, syn_mqtt_subscribe(&c, "t", 1U));
    TEST_ASSERT_EQUAL(SYN_MQTT_DISCONNECTED, c.state);
    TEST_ASSERT_EQUAL(SYN_SOCKET_INVALID, c.sock);
    TEST_ASSERT_EQUAL_UINT32(500U, c.last_activity_ms);

    /* 4. Inbound PUBLISH with QoS == 0 && DUP == 1 ([MQTT-3.3.1-2]) resets session */
    mock_port_reset();
    s_mqtt_msg_count = 0;
    syn_mqtt_init(&c, "broker", 1883, "cid", NULL, NULL, 60, rx, sizeof(rx), tx, sizeof(tx));
    c.state = SYN_MQTT_CONNECTED;
    c.sock = 11;
    c.on_message = on_mqtt_message;
    mock_sock_connected = true;
    const uint8_t qos0_dup[] = {0x38U, 0x05U, 0x00U, 0x01U, 't', 'o', 'k'};
    mock_sock_set_response(qos0_dup, sizeof(qos0_dup));
    PT_INIT(&pt);
    TEST_ASSERT_EQUAL(PT_IDLE, syn_mqtt_task(&pt, &task));
    TEST_ASSERT_EQUAL_INT(0, s_mqtt_msg_count);
    TEST_ASSERT_EQUAL(SYN_MQTT_DISCONNECTED, c.state);
    TEST_ASSERT_EQUAL(SYN_SOCKET_INVALID, c.sock);
}

void run_mqtt_tests(void)
{
    RUN_TEST(test_mqtt_connect);
    RUN_TEST(test_mqtt_subscribe);
    RUN_TEST(test_mqtt_publish_qos0);
    RUN_TEST(test_mqtt_publish_qos1_retry);
    RUN_TEST(test_mqtt_rx_publish);
    RUN_TEST(test_mqtt_formatting_and_boundaries);
    RUN_TEST(test_mqtt_multiplier_overflow);
    RUN_TEST(test_mqtt_publish_edge_cases);
    RUN_TEST(test_mqtt_state_machine_failures);
    RUN_TEST(test_mqtt_payload_skip_logic);
    RUN_TEST(test_mqtt_ping_and_mismatch);
    RUN_TEST(test_mqtt_pt_end);
    RUN_TEST(test_mqtt_explicit_ping);
    RUN_TEST(test_mqtt_mid_packet_rx_stalling_timeout);
    RUN_TEST(test_mqtt_connection_closed_during_payload_discard);
    RUN_TEST(test_mqtt_topic_and_client_id_null_or_empty);
    RUN_TEST(test_mqtt_tx_buffer_overflow_in_connect);
    RUN_TEST(test_mqtt_rx_phase_disconnect_handling);
    RUN_TEST(test_mqtt_ping_send_failure_and_packet_id_wraparound);
    RUN_TEST(test_mqtt_disconnect);
    RUN_TEST(test_mqtt_qos2_handshake_and_keepalive_ping);
    RUN_TEST(test_mqtt_subscribe_does_not_corrupt_retransmit_buffer);
    RUN_TEST(test_mqtt5_varint_and_properties);
    RUN_TEST(test_mqtt_has_work_wakeup_and_sustained_rx_yielding);
    RUN_TEST(test_mqtt_round3_regressions);
    RUN_TEST(test_mqtt_round4_regressions);
    RUN_TEST(test_r5_n11_mqtt_qos1_retransmit_bounds_and_sub_qos2);
    RUN_TEST(test_tier_a_mqtt_connack_send_fail_and_qos0_dup);
}
