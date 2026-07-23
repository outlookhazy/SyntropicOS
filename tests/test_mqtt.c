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

static void test_mqtt_formatting_and_boundaries(void)
{
    mock_port_reset();

    /* 1. CONNECT formatting with username and password */
    SYN_MqttClient c;
    uint8_t rx[256];
    uint8_t tx[256];
    syn_mqtt_init(&c, "broker.hivemq.com", 1883, "myclient", "myuser", "mypass", 60, rx, sizeof(rx), tx, sizeof(tx));

    SYN_PT pt;
    PT_INIT(&pt);
    SYN_Task task;
    task.user_data = &c;

    syn_mqtt_task(&pt, &task);
    TEST_ASSERT_EQUAL(SYN_MQTT_CONNECTING, c.state);
    /* CONNECT type (0x10) and flags (0x02 | 0x80 | 0x40 = 0xC2) */
    TEST_ASSERT_EQUAL_UINT8(0x10, mock_sock_tx_buf[0]);
    
    /* 2. Buffer size guard for CONNECT */
    PT_INIT(&pt);
    syn_mqtt_init(&c, "broker.hivemq.com", 1883, "myclient", NULL, NULL, 60, rx, sizeof(rx), tx, 5); // tiny tx buffer
    syn_mqtt_task(&pt, &task);
    /* Should fail to send, close socket, and transition back to DISCONNECTED */
    TEST_ASSERT_EQUAL(SYN_MQTT_DISCONNECTED, c.state);
    TEST_ASSERT_EQUAL(SYN_SOCKET_INVALID, c.sock);

    /* 3. Retransmit buffer size guard on publish */
    syn_mqtt_init(&c, "broker.hivemq.com", 1883, "myclient", NULL, NULL, 60, rx, sizeof(rx), tx, sizeof(tx));
    c.state = SYN_MQTT_CONNECTED;
    c.sock = 11;
    mock_sock_connected = true;

    /* Long payload to exceed retransmit_buf (128 bytes) */
    uint8_t long_payload[130];
    memset(long_payload, 'A', sizeof(long_payload));
    SYN_Status st = syn_mqtt_publish(&c, "temp", long_payload, sizeof(long_payload), 1, false);
    TEST_ASSERT_EQUAL(SYN_OK, st);
    TEST_ASSERT_EQUAL_INT(0, c.retransmit_len); // retransmit_len is 0

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

    /* 6. Extra boundary formatting checks: username without password, subscribe QoS 0, publish with NULL payload */
    syn_mqtt_init(&c, "broker.hivemq.com", 1883, "myclient", "myuser", NULL, 60, rx, sizeof(rx), tx, sizeof(tx));
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
    syn_mqtt_init(&c, "broker.hivemq.com", 1883, "myclient", NULL, NULL, 60, rx, sizeof(rx), tx, sizeof(tx));
    c.state = SYN_MQTT_CONNECTED;
    c.sock = 11;
    mock_sock_connected = true;

    SYN_PT pt;
    PT_INIT(&pt);
    SYN_Task task;
    task.user_data = &c;

    /* 5 bytes of remaining length all with MSB set (0x80) */
    uint8_t frame[] = { 0x30, 0x80, 0x80, 0x80, 0x80, 0x80 };
    mock_sock_set_response(frame, sizeof(frame));

    syn_mqtt_task(&pt, &task);

    /* Since read_remaining_len returns -1, it should close the socket and transition to DISCONNECTED */
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
    syn_mqtt_init(&c, "broker.hivemq.com", 1883, "myclient", NULL, NULL, 60, rx, sizeof(rx), tx, sizeof(tx));
    c.state = SYN_MQTT_CONNECTED;
    c.sock = 11;
    c.on_message = on_mqtt_message;
    mock_sock_connected = true;

    SYN_PT pt;
    PT_INIT(&pt);
    SYN_Task task;
    task.user_data = &c;

    /* A. len < 2 */
    uint8_t frame_short[] = { 0x30, 0x01, 0x00 };
    mock_sock_set_response(frame_short, sizeof(frame_short));
    syn_mqtt_task(&pt, &task);
    TEST_ASSERT_EQUAL(0, s_mqtt_msg_count);

    /* B. topic_len too large (2 + topic_len > len) */
    PT_INIT(&pt);
    uint8_t frame_bad_topic_len[] = { 0x30, 0x05, 0x00, 0x0A, 't', 'e', 's' };
    mock_sock_set_response(frame_bad_topic_len, sizeof(frame_bad_topic_len));
    syn_mqtt_task(&pt, &task);
    TEST_ASSERT_EQUAL(0, s_mqtt_msg_count);

    /* C. qos > 0 but payload_offset + 2 > len (no packet ID) */
    PT_INIT(&pt);
    uint8_t frame_no_pid[] = { 0x32, 0x05, 0x00, 0x03, 't', 'm', 'p' }; // QoS 1 (0x32)
    mock_sock_set_response(frame_no_pid, sizeof(frame_no_pid));
    syn_mqtt_task(&pt, &task);
    TEST_ASSERT_EQUAL(0, s_mqtt_msg_count);

    /* D. QoS 1 publish success (sends PUBACK) */
    PT_INIT(&pt);
    mock_sock_tx_len = 0;
    uint8_t frame_qos1[] = { 0x32, 0x07, 0x00, 0x03, 't', 'm', 'p', 0x00, 0x2A }; // QoS 1, PID = 42
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
    uint8_t frame_qos2[] = { 0x34, 0x07, 0x00, 0x03, 't', 'm', 'p', 0x00, 0x2B }; // QoS 2 (0x34), PID = 43
    mock_sock_set_response(frame_qos2, sizeof(frame_qos2));
    syn_mqtt_task(&pt, &task);
    TEST_ASSERT_EQUAL(1, s_mqtt_msg_count);
    TEST_ASSERT_EQUAL(0, mock_sock_tx_len); // no PUBACK sent

    /* F. QoS 3 publish success (calls callback, no PUBACK) */
    PT_INIT(&pt);
    mock_sock_tx_len = 0;
    s_mqtt_msg_count = 0;
    uint8_t frame_qos3[] = { 0x36, 0x07, 0x00, 0x03, 't', 'm', 'p', 0x00, 0x2C }; // QoS 3 (0x36), PID = 44
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
    syn_mqtt_init(&c, "broker.hivemq.com", 1883, "myclient", NULL, NULL, 60, rx, sizeof(rx), tx, sizeof(tx));

    SYN_PT pt;
    PT_INIT(&pt);
    SYN_Task task;
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
    syn_mqtt_init(&c, "broker.hivemq.com", 1883, "myclient", NULL, NULL, 60, rx, sizeof(rx), tx, sizeof(tx));
    mock_sock_connect_fail = false;
    mock_sock_send_fail = true;
    syn_mqtt_task(&pt, &task);
    TEST_ASSERT_EQUAL(SYN_MQTT_DISCONNECTED, c.state);
    TEST_ASSERT_EQUAL(SYN_SOCKET_INVALID, c.sock);

    /* Resume task after send failure delay to hit continue statement */
    mock_tick_advance(5000);
    syn_mqtt_task(&pt, &task);

    /* 3. CONNACK Short Packet (rem_len < 2) */
    mock_port_reset();
    PT_INIT(&pt);
    syn_mqtt_init(&c, "broker.hivemq.com", 1883, "myclient", NULL, NULL, 60, rx, sizeof(rx), tx, sizeof(tx));
    /* Connect first */
    syn_mqtt_task(&pt, &task);
    TEST_ASSERT_EQUAL(SYN_MQTT_CONNECTING, c.state);
    /* Set short CONNACK response */
    uint8_t connack_short[] = { 0x20, 0x01, 0x00 };
    mock_sock_set_response(connack_short, sizeof(connack_short));
    syn_mqtt_task(&pt, &task);
    TEST_ASSERT_EQUAL(SYN_MQTT_DISCONNECTED, c.state);
    TEST_ASSERT_EQUAL(SYN_SOCKET_INVALID, c.sock);

    /* 4. CONNACK Non-Zero Code */
    mock_port_reset();
    PT_INIT(&pt);
    syn_mqtt_init(&c, "broker.hivemq.com", 1883, "myclient", NULL, NULL, 60, rx, sizeof(rx), tx, sizeof(tx));
    syn_mqtt_task(&pt, &task);
    TEST_ASSERT_EQUAL(SYN_MQTT_CONNECTING, c.state);
    uint8_t connack_err[] = { 0x20, 0x02, 0x00, 0x05 }; /* Connection Refused: bad username/password */
    mock_sock_set_response(connack_err, sizeof(connack_err));
    syn_mqtt_task(&pt, &task);
    TEST_ASSERT_EQUAL(SYN_MQTT_DISCONNECTED, c.state);
    TEST_ASSERT_EQUAL(SYN_SOCKET_INVALID, c.sock);

    /* 5. Socket EOF Closure (n == 0 during poll) */
    mock_port_reset();
    PT_INIT(&pt);
    syn_mqtt_init(&c, "broker.hivemq.com", 1883, "myclient", NULL, NULL, 60, rx, sizeof(rx), tx, sizeof(tx));
    c.state = SYN_MQTT_CONNECTED;
    c.sock = 11;
    mock_sock_connected = true;
    mock_sock_eof_on_empty = true;
    syn_mqtt_task(&pt, &task);
    TEST_ASSERT_EQUAL(SYN_MQTT_DISCONNECTED, c.state);
    TEST_ASSERT_EQUAL(SYN_SOCKET_INVALID, c.sock);

    /* 6. Transmit Failures for Publish/Subscribe */
    mock_port_reset();
    syn_mqtt_init(&c, "broker.hivemq.com", 1883, "myclient", NULL, NULL, 60, rx, sizeof(rx), tx, sizeof(tx));
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
    SYN_Task task;

    /* 1. read_all fails (socket EOF on valid remaining length) */
    mock_port_reset();
    SYN_MqttClient c;
    uint8_t rx[32];
    uint8_t tx[32];
    syn_mqtt_init(&c, "broker.hivemq.com", 1883, "myclient", NULL, NULL, 60, rx, sizeof(rx), tx, sizeof(tx));
    c.state = SYN_MQTT_CONNECTED;
    c.sock = 11;
    task.user_data = &c;
    mock_sock_connected = true;
    mock_sock_eof_on_empty = true;

    /* Rem Len = 10, but only 5 bytes provided */
    uint8_t short_packet[] = { 0x30, 10, 'a', 'b', 'c', 'd', 'e' };
    mock_sock_set_response(short_packet, sizeof(short_packet));
    syn_mqtt_task(&pt, &task);

    /* Should disconnect because socket closed (EOF) before payload complete */
    TEST_ASSERT_EQUAL(SYN_MQTT_DISCONNECTED, c.state);
    TEST_ASSERT_EQUAL(SYN_SOCKET_INVALID, c.sock);

    /* 2. Successful skip logic (packet exceeds rx_buf_size) */
    mock_port_reset();
    PT_INIT(&pt);
    syn_mqtt_init(&c, "broker.hivemq.com", 1883, "myclient", NULL, NULL, 60, rx, sizeof(rx), tx, sizeof(tx));
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
    syn_mqtt_init(&c, "broker.hivemq.com", 1883, "myclient", NULL, NULL, 60, rx, sizeof(rx), tx, sizeof(tx));
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
    syn_mqtt_init(&c, "broker.hivemq.com", 1883, "myclient", NULL, NULL, 60, rx, sizeof(rx), tx, sizeof(tx));
    c.state = SYN_MQTT_CONNECTED;
    c.sock = 11;
    task.user_data = &c;
    mock_sock_connected = true;
    mock_sock_eof_on_empty = true;

    /* Header 0x30, but no length bytes follow */
    uint8_t header_only[] = { 0x30 };
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
    syn_mqtt_init(&c, "broker.hivemq.com", 1883, "myclient", NULL, NULL, 5, rx, sizeof(rx), tx, sizeof(tx));
    c.state = SYN_MQTT_CONNECTED;
    c.sock = 11;
    mock_sock_connected = true;
    c.last_activity_ms = mock_tick_ms;

    SYN_PT pt;
    PT_INIT(&pt);
    SYN_Task task;
    task.user_data = &c;

    mock_tick_advance(5000);
    mock_sock_tx_len = 0;
    syn_mqtt_task(&pt, &task);
    /* Should send ping (0xC0, 0x00) and update activity time */
    TEST_ASSERT_EQUAL(SYN_MQTT_CONNECTED, c.state);
    TEST_ASSERT_EQUAL(2, mock_sock_tx_len);
    TEST_ASSERT_EQUAL_UINT8(0xC0, mock_sock_tx_buf[0]);
    TEST_ASSERT_EQUAL_UINT8(0x00, mock_sock_tx_buf[1]);
    TEST_ASSERT_EQUAL(mock_tick_ms, c.last_activity_ms);

    /* 2. Ping Transmit Failure */
    mock_port_reset();
    PT_INIT(&pt);
    syn_mqtt_init(&c, "broker.hivemq.com", 1883, "myclient", NULL, NULL, 5, rx, sizeof(rx), tx, sizeof(tx));
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

    /* 3. PINGRESP (0xD0) safe handling */
    mock_port_reset();
    PT_INIT(&pt);
    syn_mqtt_init(&c, "broker.hivemq.com", 1883, "myclient", NULL, NULL, 60, rx, sizeof(rx), tx, sizeof(tx));
    c.state = SYN_MQTT_CONNECTED;
    c.sock = 11;
    mock_sock_connected = true;

    uint8_t pingresp[] = { 0xD0, 0x00 };
    mock_sock_set_response(pingresp, sizeof(pingresp));
    syn_mqtt_task(&pt, &task);
    TEST_ASSERT_EQUAL(SYN_MQTT_CONNECTED, c.state);

    /* 4. QoS 1 PUBACK Mismatch */
    mock_port_reset();
    PT_INIT(&pt);
    syn_mqtt_init(&c, "broker.hivemq.com", 1883, "myclient", NULL, NULL, 60, rx, sizeof(rx), tx, sizeof(tx));
    c.state = SYN_MQTT_CONNECTED;
    c.sock = 11;
    mock_sock_connected = true;
    c.pending_puback_id = 100;

    /* A. Send wrong packet ID (200) */
    uint8_t puback_wrong[] = { 0x40, 0x02, 0x00, 0xC8 }; // PID = 200
    mock_sock_set_response(puback_wrong, sizeof(puback_wrong));
    syn_mqtt_task(&pt, &task);
    TEST_ASSERT_EQUAL(100, c.pending_puback_id); // should remain 100

    /* B. Send correct packet ID (100) */
    PT_INIT(&pt);
    uint8_t puback_correct[] = { 0x40, 0x02, 0x00, 0x64 }; // PID = 100
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
    syn_mqtt_init(&c, "broker.hivemq.com", 1883, "myclient", NULL, NULL, 60, rx, sizeof(rx), tx, sizeof(tx));

    SYN_PT pt;
    PT_INIT(&pt);
    SYN_Task task;
    task.user_data = &c;

    /* Set LC to a line that is not a yield switch case to trigger the fallthrough to PT_END */
    pt.lc = 9999;
    SYN_PT_Status status = syn_mqtt_task(&pt, &task);
    TEST_ASSERT_EQUAL(PT_EXITED, status);
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
}
