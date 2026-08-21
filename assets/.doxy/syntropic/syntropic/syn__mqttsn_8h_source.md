

# File syn\_mqttsn.h

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_mqttsn.h**](syn__mqttsn_8h.md)

[Go to the documentation of this file](syn__mqttsn_8h.md)


```C++

#ifndef SYN_MQTTSN_H
#define SYN_MQTTSN_H

#include "../common/syn_defs.h"
#include "../net/syn_transport.h"
#include "../pt/syn_pt.h"
#include "../sched/syn_sched.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#if !defined(SYN_USE_MQTTSN) || SYN_USE_MQTTSN

#ifdef __cplusplus
extern "C" {
#endif

/* ── Constants & Limits ─────────────────────────────────────────────────── */

#define SYN_MQTTSN_PROTOCOL_ID 0x01U      
#define SYN_MQTTSN_DEFAULT_DURATION_S 60U 
#define SYN_MQTTSN_MAX_CLIENT_ID 24U      
#define SYN_MQTTSN_MAX_TOPIC_NAME 64U     
#define SYN_MQTTSN_MAX_REGISTRATIONS 16U  
/* ── Message Types ──────────────────────────────────────────────────────── */

#define SYN_MQTTSN_MSG_ADVERTISE 0x00U     
#define SYN_MQTTSN_MSG_SEARCHGW 0x01U      
#define SYN_MQTTSN_MSG_GWINFO 0x02U        
#define SYN_MQTTSN_MSG_CONNECT 0x04U       
#define SYN_MQTTSN_MSG_CONNACK 0x05U       
#define SYN_MQTTSN_MSG_WILLTOPICREQ 0x06U  
#define SYN_MQTTSN_MSG_WILLTOPIC 0x07U     
#define SYN_MQTTSN_MSG_WILLMSGREQ 0x08U    
#define SYN_MQTTSN_MSG_WILLMSG 0x09U       
#define SYN_MQTTSN_MSG_REGISTER 0x0AU      
#define SYN_MQTTSN_MSG_REGACK 0x0BU        
#define SYN_MQTTSN_MSG_PUBLISH 0x0CU       
#define SYN_MQTTSN_MSG_PUBACK 0x0DU        
#define SYN_MQTTSN_MSG_PUBCOMP 0x0EU       
#define SYN_MQTTSN_MSG_PUBREC 0x0FU        
#define SYN_MQTTSN_MSG_PUBREL 0x10U        
#define SYN_MQTTSN_MSG_SUBSCRIBE 0x12U     
#define SYN_MQTTSN_MSG_SUBACK 0x13U        
#define SYN_MQTTSN_MSG_UNSUBSCRIBE 0x14U   
#define SYN_MQTTSN_MSG_UNSUBACK 0x15U      
#define SYN_MQTTSN_MSG_PINGREQ 0x16U       
#define SYN_MQTTSN_MSG_PINGRESP 0x17U      
#define SYN_MQTTSN_MSG_DISCONNECT 0x18U    
#define SYN_MQTTSN_MSG_WILLTOPICUPD 0x1AU  
#define SYN_MQTTSN_MSG_WILLTOPICRESP 0x1BU 
#define SYN_MQTTSN_MSG_WILLMSGUPD 0x1CU    
#define SYN_MQTTSN_MSG_WILLMSGRESP 0x1DU   
/* ── Return Codes ───────────────────────────────────────────────────────── */

#define SYN_MQTTSN_RC_ACCEPTED 0x00U             
#define SYN_MQTTSN_RC_REJ_CONGESTION 0x01U       
#define SYN_MQTTSN_RC_REJ_INVALID_TOPIC_ID 0x02U 
#define SYN_MQTTSN_RC_REJ_NOT_SUPPORTED 0x03U    
/* ── Flag Bitmasks ──────────────────────────────────────────────────────── */

#define SYN_MQTTSN_FLAG_DUP 0x80U           
#define SYN_MQTTSN_FLAG_QOS_MASK 0x60U      
#define SYN_MQTTSN_FLAG_QOS_0 0x00U         
#define SYN_MQTTSN_FLAG_QOS_1 0x20U         
#define SYN_MQTTSN_FLAG_QOS_2 0x40U         
#define SYN_MQTTSN_FLAG_QOS_NEG1 0x60U      
#define SYN_MQTTSN_FLAG_RETAIN 0x10U        
#define SYN_MQTTSN_FLAG_WILL 0x08U          
#define SYN_MQTTSN_FLAG_CLEAN_SESSION 0x04U 
#define SYN_MQTTSN_FLAG_TOPIC_MASK 0x03U    
#define SYN_MQTTSN_FLAG_TOPIC_NORMAL 0x00U  
#define SYN_MQTTSN_FLAG_TOPIC_PREDEF 0x01U  
#define SYN_MQTTSN_FLAG_TOPIC_SHORT 0x02U   
/* ── Client State Enum ──────────────────────────────────────────────────── */

typedef enum {
    SYN_MQTTSN_STATE_DISCONNECTED = 0, 
    SYN_MQTTSN_STATE_SEARCHING_GW,     
    SYN_MQTTSN_STATE_CONNECTING,       
    SYN_MQTTSN_STATE_CONNECTED,        
    SYN_MQTTSN_STATE_ASLEEP,           
    SYN_MQTTSN_STATE_AWAKE             
} SYN_MQTTSN_State;

/* ── Callback Signatures ────────────────────────────────────────────────── */

struct SYN_MQTTSN_Client;

typedef void (*SYN_MQTTSN_MessageCb)(struct SYN_MQTTSN_Client *client, uint16_t topic_id,
                                     uint8_t topic_type, const uint8_t *payload, size_t len,
                                     void *user_data);

/* ── Topic Registry Entry ───────────────────────────────────────────────── */

typedef struct {
    char name[SYN_MQTTSN_MAX_TOPIC_NAME]; 
    uint16_t topic_id;                    
    bool active;                          
} SYN_MQTTSN_TopicEntry;

/* ── Configuration Descriptor ────────────────────────────────────────────── */

typedef struct {
    SYN_Transport *transport;        
    const char *client_id;           
    uint16_t duration_s;             
    bool clean_session;              
    SYN_MQTTSN_MessageCb on_message; 
    void *user_data;                 
    uint8_t *rx_buf;    
    size_t rx_buf_size; 
    uint8_t *tx_buf;    
    size_t tx_buf_size; 
} SYN_MQTTSN_Config;

/* ── Client Context ─────────────────────────────────────────────────────── */

typedef struct SYN_MQTTSN_Client {
    SYN_MQTTSN_Config cfg;  
    SYN_MQTTSN_State state; 
    SYN_PT pt;              
    uint8_t gateway_id;        
    uint16_t next_msg_id;      
    uint32_t last_activity_ms; 
    SYN_MQTTSN_TopicEntry topics[SYN_MQTTSN_MAX_REGISTRATIONS]; 
    size_t topic_count;                                         
} SYN_MQTTSN_Client;

/* ── Public Client API ──────────────────────────────────────────────────── */

SYN_Status syn_mqttsn_client_init(SYN_MQTTSN_Client *client, const SYN_MQTTSN_Config *cfg);

SYN_Status syn_mqttsn_client_searchgw(SYN_MQTTSN_Client *client, uint8_t radius);

SYN_Status syn_mqttsn_client_connect(SYN_MQTTSN_Client *client);

SYN_Status syn_mqttsn_client_register_topic(SYN_MQTTSN_Client *client, const char *topic_name,
                                            uint16_t *out_msg_id);

SYN_Status syn_mqttsn_client_publish(SYN_MQTTSN_Client *client, uint16_t topic_id, int8_t qos,
                                     bool retain, const uint8_t *payload, size_t len);

SYN_Status syn_mqttsn_client_publish_short(SYN_MQTTSN_Client *client, const char short_topic[2],
                                           int8_t qos, bool retain, const uint8_t *payload,
                                           size_t len);

SYN_Status syn_mqttsn_client_publish_predefined(SYN_MQTTSN_Client *client, uint16_t predef_id,
                                                int8_t qos, bool retain, const uint8_t *payload,
                                                size_t len);

SYN_Status syn_mqttsn_client_subscribe(SYN_MQTTSN_Client *client, const char *topic_name,
                                       uint16_t predef_id, int8_t qos);

SYN_Status syn_mqttsn_client_unsubscribe(SYN_MQTTSN_Client *client, const char *topic_name,
                                         uint16_t predef_id);

SYN_Status syn_mqttsn_client_sleep(SYN_MQTTSN_Client *client, uint16_t sleep_duration_s);

SYN_Status syn_mqttsn_client_wake(SYN_MQTTSN_Client *client);

SYN_Status syn_mqttsn_client_disconnect(SYN_MQTTSN_Client *client);

SYN_Status syn_mqttsn_client_step(SYN_MQTTSN_Client *client, uint32_t now_ms);

SYN_PT_Status syn_mqttsn_client_pt(SYN_PT *pt, SYN_Task *task);

#ifdef __cplusplus
}
#endif

#endif /* !defined(SYN_USE_MQTTSN) || SYN_USE_MQTTSN */

#endif /* SYN_MQTTSN_H */
```


