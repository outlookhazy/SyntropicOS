

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
    SYN_MQTT_DISCONNECTED,           
    SYN_MQTT_CONNECTING,             
    SYN_MQTT_CONNECTED,              
} SYN_MqttState;

typedef struct {
    SYN_Socket       sock;           
    SYN_MqttState    state;          
    const char      *host;           
    uint16_t         port;           
    const char      *client_id;      
    const char      *username;       
    const char      *password;       
    uint16_t         keep_alive_s;   
    void (*on_message)(const char *topic, const uint8_t *payload, size_t len, void *ctx);
    void            *ctx;            
    uint8_t         *rx_buf;         
    size_t           rx_buf_size;    
    uint8_t         *tx_buf;         
    size_t           tx_buf_size;    
    uint32_t         last_activity_ms; 
    uint16_t         next_packet_id;  
    uint16_t         pending_puback_id; 
    uint32_t         pending_puback_ms; 
    uint8_t          retransmit_buf[128]; 
    size_t           retransmit_len;    
} SYN_MqttClient;

SYN_Status syn_mqtt_init(SYN_MqttClient *client, const char *host, uint16_t port,
                         const char *client_id, const char *username, const char *password,
                         uint16_t keep_alive_s,
                         uint8_t *rx_buf, size_t rx_buf_size,
                         uint8_t *tx_buf, size_t tx_buf_size);

SYN_Status syn_mqtt_publish(SYN_MqttClient *client, const char *topic,
                            const void *payload, size_t len, uint8_t qos, bool retain);

SYN_Status syn_mqtt_subscribe(SYN_MqttClient *client, const char *topic, uint8_t qos);

SYN_PT_Status syn_mqtt_task(SYN_PT *pt, SYN_Task *task);

#ifdef __cplusplus
}
#endif

#endif /* SYN_MQTT_H */
```


