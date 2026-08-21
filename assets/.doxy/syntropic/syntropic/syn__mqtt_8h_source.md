

# File syn\_mqtt.h

[**File List**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_mqtt.h**](syn__mqtt_8h.md)

[Go to the documentation of this file](syn__mqtt_8h.md)


```C++

#ifndef SYN_MQTT_H
#define SYN_MQTT_H

#include "../common/syn_defs.h"
#include "../port/syn_port_socket.h"
#include "../pt/syn_pt.h"
#include "../sched/syn_task.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SYN_MQTT_VERSION_3_1_1 = 4, 
    SYN_MQTT_VERSION_5_0 = 5    
} SYN_MqttVersion;

#define SYN_MQTT5_PROP_PAYLOAD_FORMAT_INDICATOR 0x01U 
#define SYN_MQTT5_PROP_MESSAGE_EXPIRY_INTERVAL 0x02U  
#define SYN_MQTT5_PROP_CONTENT_TYPE 0x03U             
#define SYN_MQTT5_PROP_RESPONSE_TOPIC 0x08U           
#define SYN_MQTT5_PROP_CORRELATION_DATA 0x09U         
#define SYN_MQTT5_PROP_SUBSCRIPTION_IDENTIFIER 0x0BU  
#define SYN_MQTT5_PROP_SESSION_EXPIRY_INTERVAL 0x11U  
#define SYN_MQTT5_PROP_ASSIGNED_CLIENT_ID 0x12U       
#define SYN_MQTT5_PROP_SERVER_KEEP_ALIVE 0x13U        
#define SYN_MQTT5_PROP_AUTH_METHOD 0x15U              
#define SYN_MQTT5_PROP_AUTH_DATA 0x16U                
#define SYN_MQTT5_PROP_REQ_PROBLEM_INFO 0x17U         
#define SYN_MQTT5_PROP_WILL_DELAY_INTERVAL 0x18U      
#define SYN_MQTT5_PROP_REQ_RESPONSE_INFO 0x19U        
#define SYN_MQTT5_PROP_RESPONSE_INFO 0x1AU            
#define SYN_MQTT5_PROP_SERVER_REFERENCE 0x1CU         
#define SYN_MQTT5_PROP_REASON_STRING 0x1FU            
#define SYN_MQTT5_PROP_RECEIVE_MAXIMUM 0x21U          
#define SYN_MQTT5_PROP_TOPIC_ALIAS_MAXIMUM 0x22U      
#define SYN_MQTT5_PROP_TOPIC_ALIAS 0x23U              
#define SYN_MQTT5_PROP_MAXIMUM_QOS 0x24U              
#define SYN_MQTT5_PROP_RETAIN_AVAILABLE 0x25U         
#define SYN_MQTT5_PROP_USER_PROPERTY 0x26U            
#define SYN_MQTT5_PROP_MAX_PACKET_SIZE 0x27U          
#define SYN_MQTT5_PROP_WILDCARD_SUB_AVAIL 0x28U       
#define SYN_MQTT5_PROP_SUB_ID_AVAIL 0x29U             
#define SYN_MQTT5_PROP_SHARED_SUB_AVAIL 0x2AU         
typedef struct {
    const char *key; 
    const char *val; 
} SYN_Mqtt5_UserProp;

typedef enum {
    SYN_MQTT_DISCONNECTED, 
    SYN_MQTT_CONNECTING,   
    SYN_MQTT_CONNECTED,    
} SYN_MqttState;

typedef enum {
    SYN_MQTT_RX_IDLE,          
    SYN_MQTT_RX_REMAINING_LEN, 
    SYN_MQTT_RX_PAYLOAD,       
    SYN_MQTT_RX_DISCARD,       
} SYN_MqttRxPhase;

typedef struct {
    SYN_Socket sock;       
    SYN_MqttState state;   
    const char *host;      
    uint16_t port;         
    const char *client_id; 
    const char *username;  
    const char *password;  
    uint16_t keep_alive_s; 
    void (*on_message)(const char *topic, const uint8_t *payload, size_t len, void *ctx);
    void *ctx; 
    uint8_t *rx_buf;    
    size_t rx_buf_size; 
    uint8_t *tx_buf;    
    size_t tx_buf_size; 
    uint32_t last_activity_ms; 
    uint16_t next_packet_id;   
    uint16_t pending_puback_id;  
    uint32_t pending_puback_ms;  
    uint8_t retransmit_buf[128]; 
    size_t retransmit_len;       
    /* ── Non-blocking RX State Machine ─────────────────────────────────── */
    SYN_MqttRxPhase rx_phase; 
    uint8_t rx_header;        
    uint32_t rx_rem_len;      
    uint32_t rx_mult;         
    size_t rx_pos;            
    uint32_t rx_deadline;     
} SYN_MqttClient;

SYN_Status syn_mqtt_init(SYN_MqttClient *client, const char *host, uint16_t port,
                         const char *client_id, const char *username, const char *password,
                         uint16_t keep_alive_s, uint8_t *rx_buf, size_t rx_buf_size,
                         uint8_t *tx_buf, size_t tx_buf_size);

SYN_Status syn_mqtt_publish(SYN_MqttClient *client, const char *topic, const void *payload,
                            size_t len, uint8_t qos, bool retain);

SYN_Status syn_mqtt_subscribe(SYN_MqttClient *client, const char *topic, uint8_t qos);

SYN_Status syn_mqtt_ping(SYN_MqttClient *client);

void syn_mqtt_disconnect(SYN_MqttClient *client);

SYN_PT_Status syn_mqtt_task(SYN_PT *pt, SYN_Task *task);

size_t syn_mqtt_encode_varint(uint32_t val, uint8_t buf[4]);

bool syn_mqtt_decode_varint(const uint8_t *buf, size_t buf_len, uint32_t *val, size_t *bytes_read);

size_t syn_mqtt5_encode_user_prop(const char *key, const char *val, uint8_t *buf,
                                  size_t max_buf_len);

#ifdef __cplusplus
}
#endif

#endif /* SYN_MQTT_H */
```


