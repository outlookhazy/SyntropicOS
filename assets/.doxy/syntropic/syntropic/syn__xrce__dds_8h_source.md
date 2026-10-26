

# File syn\_xrce\_dds.h

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_xrce\_dds.h**](syn__xrce__dds_8h.md)

[Go to the documentation of this file](syn__xrce__dds_8h.md)


```C++

#ifndef SYN_XRCE_DDS_H
#define SYN_XRCE_DDS_H

#include "../common/syn_defs.h"
#include "../net/syn_transport.h"
#include "../pt/syn_pt.h"
#include "../sched/syn_sched.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#if !defined(SYN_USE_XRCE_DDS) || SYN_USE_XRCE_DDS

#ifdef __cplusplus
extern "C" {
#endif

/* ── Constants & Limits ─────────────────────────────────────────────────── */

#define SYN_XRCE_COOKIE 0x0100U                 
#define SYN_XRCE_CLIENT_KEY_DEFAULT 0xCAFEBABEU 
#define SYN_XRCE_SESSION_ID_DEFAULT 0x81U       
#define SYN_XRCE_STREAM_BEST_EFFORT 0x01U       
#define SYN_XRCE_STREAM_RELIABLE 0x80U          
#define SYN_XRCE_MAX_TOPIC_NAME 64U             
#define SYN_XRCE_MAX_TYPE_NAME 64U              
#define SYN_XRCE_MAX_OBJECTS 16U                
/* XRCE-DDS Submessage Identifiers */
#define SYN_XRCE_SUBMSG_CREATE_CLIENT 0x00U 
#define SYN_XRCE_SUBMSG_CREATE 0x01U        
#define SYN_XRCE_SUBMSG_DELETE 0x02U        
#define SYN_XRCE_SUBMSG_WRITE_DATA 0x03U    
#define SYN_XRCE_SUBMSG_READ_DATA 0x04U     
#define SYN_XRCE_SUBMSG_DATA 0x05U          
#define SYN_XRCE_SUBMSG_STATUS 0x06U        
#define SYN_XRCE_SUBMSG_HEARTBEAT 0x07U     
#define SYN_XRCE_SUBMSG_ACKNACK 0x08U       
/* XRCE-DDS Object Kinds */
#define SYN_XRCE_OBJKIND_PARTICIPANT 0x01U 
#define SYN_XRCE_OBJKIND_TOPIC 0x02U       
#define SYN_XRCE_OBJKIND_PUBLISHER 0x03U   
#define SYN_XRCE_OBJKIND_SUBSCRIBER 0x04U  
#define SYN_XRCE_OBJKIND_DATAWRITER 0x05U  
#define SYN_XRCE_OBJKIND_DATAREADER 0x06U  
/* XRCE-DDS Status Codes */
#define SYN_XRCE_STATUS_OK 0x00U                 
#define SYN_XRCE_STATUS_OK_MATCHED 0x01U         
#define SYN_XRCE_STATUS_ERR_ALREADY_EXISTS 0x11U 
#define SYN_XRCE_STATUS_ERR_UNKNOWN_REF 0x12U    
#define SYN_XRCE_STATUS_ERR_INVALID_DATA 0x13U   
#define SYN_XRCE_STATUS_ERR_INCOMPATIBLE 0x14U   
#define SYN_XRCE_STATUS_ERR_RESOURCES 0x15U      
/* ── CDR Codec Contexts ──────────────────────────────────────────────────── */

typedef struct {
    uint8_t *data; 
    size_t size;   
    size_t pos;    
    bool error;    
} SYN_CDR_Writer;

typedef struct {
    const uint8_t *data; 
    size_t size;         
    size_t pos;          
    bool error;          
} SYN_CDR_Reader;

/* ── Client Entity & State Types ─────────────────────────────────────────── */

typedef enum {
    SYN_XRCE_STATE_DISCONNECTED = 0, 
    SYN_XRCE_STATE_CONNECTING,       
    SYN_XRCE_STATE_CONNECTED,        
    SYN_XRCE_STATE_ERROR             
} SYN_XRCE_ClientState;

typedef struct {
    uint16_t id;     
    uint8_t kind;    
    bool configured; 
    uint16_t parent; 
} SYN_XRCE_Object;

typedef void (*SYN_XRCE_DataCb)(uint16_t reader_id, const uint8_t *payload, size_t len,
                                void *user_data);

typedef void (*SYN_XRCE_StatusCb)(uint16_t req_id, uint8_t status, void *user_data);

typedef struct {
    uint32_t client_key;          
    uint8_t session_id;           
    SYN_Transport *transport;     
    uint32_t heartbeat_period_ms; 
    SYN_XRCE_DataCb on_data;      
    SYN_XRCE_StatusCb on_status;  
    void *user_data;              
    uint8_t *rx_buf;    
    size_t rx_buf_size; 
    uint8_t *tx_buf;    
    size_t tx_buf_size; 
} SYN_XRCE_Config;

typedef struct {
    SYN_XRCE_Config cfg;        
    SYN_XRCE_ClientState state; 
    SYN_PT pt;                  
    uint16_t stream_seq_num;    
    uint16_t next_req_id;       
    uint32_t last_heartbeat_ms; 
    SYN_XRCE_Object objects[SYN_XRCE_MAX_OBJECTS]; 
    size_t object_count;                           
} SYN_XRCE_Client;

/* ── CDR Serialization API ───────────────────────────────────────────────── */

void syn_cdr_writer_init(SYN_CDR_Writer *w, uint8_t *buf, size_t size);

bool syn_cdr_write_u8(SYN_CDR_Writer *w, uint8_t v);

bool syn_cdr_write_u16(SYN_CDR_Writer *w, uint16_t v);

bool syn_cdr_write_u32(SYN_CDR_Writer *w, uint32_t v);

bool syn_cdr_write_u64(SYN_CDR_Writer *w, uint64_t v);

bool syn_cdr_write_i8(SYN_CDR_Writer *w, int8_t v);

bool syn_cdr_write_i16(SYN_CDR_Writer *w, int16_t v);

bool syn_cdr_write_i32(SYN_CDR_Writer *w, int32_t v);

bool syn_cdr_write_i64(SYN_CDR_Writer *w, int64_t v);

bool syn_cdr_write_float(SYN_CDR_Writer *w, float v);

bool syn_cdr_write_double(SYN_CDR_Writer *w, double v);

bool syn_cdr_write_bool(SYN_CDR_Writer *w, bool v);

bool syn_cdr_write_string(SYN_CDR_Writer *w, const char *str);

bool syn_cdr_write_bytes(SYN_CDR_Writer *w, const uint8_t *bytes, size_t len);

/* ── CDR Deserialization API ─────────────────────────────────────────────── */

void syn_cdr_reader_init(SYN_CDR_Reader *r, const uint8_t *buf, size_t size);

bool syn_cdr_read_u8(SYN_CDR_Reader *r, uint8_t *out);

bool syn_cdr_read_u16(SYN_CDR_Reader *r, uint16_t *out);

bool syn_cdr_read_u32(SYN_CDR_Reader *r, uint32_t *out);

bool syn_cdr_read_u64(SYN_CDR_Reader *r, uint64_t *out);

bool syn_cdr_read_i8(SYN_CDR_Reader *r, int8_t *out);

bool syn_cdr_read_i16(SYN_CDR_Reader *r, int16_t *out);

bool syn_cdr_read_i32(SYN_CDR_Reader *r, int32_t *out);

bool syn_cdr_read_i64(SYN_CDR_Reader *r, int64_t *out);

bool syn_cdr_read_float(SYN_CDR_Reader *r, float *out);

bool syn_cdr_read_double(SYN_CDR_Reader *r, double *out);

bool syn_cdr_read_bool(SYN_CDR_Reader *r, bool *out);

bool syn_cdr_read_string(SYN_CDR_Reader *r, char *out_buf, size_t max_len);

bool syn_cdr_read_bytes(SYN_CDR_Reader *r, uint8_t *out_buf, size_t len);

/* ── XRCE-DDS Client Public API ──────────────────────────────────────────── */

SYN_Status syn_xrce_client_init(SYN_XRCE_Client *client, const SYN_XRCE_Config *cfg);

SYN_Status syn_xrce_client_create_session(SYN_XRCE_Client *client);

SYN_Status syn_xrce_client_create_participant(SYN_XRCE_Client *client, uint16_t participant_id);

SYN_Status syn_xrce_client_create_topic(SYN_XRCE_Client *client, uint16_t topic_id,
                                        uint16_t participant_id, const char *topic_name,
                                        const char *type_name);

SYN_Status syn_xrce_client_create_publisher(SYN_XRCE_Client *client, uint16_t publisher_id,
                                            uint16_t participant_id);

SYN_Status syn_xrce_client_create_subscriber(SYN_XRCE_Client *client, uint16_t subscriber_id,
                                             uint16_t participant_id);

SYN_Status syn_xrce_client_create_datawriter(SYN_XRCE_Client *client, uint16_t writer_id,
                                             uint16_t publisher_id, uint16_t topic_id);

SYN_Status syn_xrce_client_create_datareader(SYN_XRCE_Client *client, uint16_t reader_id,
                                             uint16_t subscriber_id, uint16_t topic_id);

SYN_Status syn_xrce_client_write_data(SYN_XRCE_Client *client, uint16_t writer_id,
                                      const uint8_t *data, size_t len);

SYN_Status syn_xrce_client_read_data(SYN_XRCE_Client *client, uint16_t reader_id);

SYN_Status syn_xrce_client_step(SYN_XRCE_Client *client, uint32_t now_ms);

SYN_PT_Status syn_xrce_client_pt(SYN_PT *pt, SYN_Task *task);

#ifdef __cplusplus
}
#endif

#endif /* SYN_USE_XRCE_DDS */

#endif /* SYN_XRCE_DDS_H */
```


