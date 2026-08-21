

# File syn\_opcua.h

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_opcua.h**](syn__opcua_8h.md)

[Go to the documentation of this file](syn__opcua_8h.md)


```C++

#ifndef SYN_OPCUA_H
#define SYN_OPCUA_H

#include "../common/syn_defs.h"
#include "../net/syn_transport.h"
#include "../pt/syn_pt.h"
#include "../sched/syn_sched.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#if !defined(SYN_USE_OPCUA) || SYN_USE_OPCUA

#ifdef __cplusplus
extern "C" {
#endif

/* ── Constants & Limits ─────────────────────────────────────────────────── */

#define SYN_OPCUA_DEFAULT_PORT 4840U        
#define SYN_OPCUA_MAX_NODES 32U             
#define SYN_OPCUA_MAX_REFERENCES 8U         
#define SYN_OPCUA_MAX_STRING_LEN 64U        
#define SYN_OPCUA_PROTOCOL_VERSION 0U       
#define SYN_OPCUA_DEFAULT_BUFFER_SIZE 1024U 
/* OPC UA Standard StatusCode constants */
#define SYN_OPCUA_STATUS_GOOD 0x00000000U               
#define SYN_OPCUA_STATUS_UNCERTAIN 0x40000000U          
#define SYN_OPCUA_STATUS_BAD_UNEXPECTED 0x80010000U     
#define SYN_OPCUA_STATUS_BAD_INTERNAL_ERROR 0x80020000U 
#define SYN_OPCUA_STATUS_BAD_NODE_ID_UNKNOWN            \
    0x80340000U 
#define SYN_OPCUA_STATUS_BAD_ATTRIBUTE_ID_INVALID 0x80350000U 
#define SYN_OPCUA_STATUS_BAD_NOT_READABLE 0x803A0000U         
#define SYN_OPCUA_STATUS_BAD_NOT_WRITABLE 0x803B0000U         
#define SYN_OPCUA_STATUS_BAD_SERVICE_UNSUPPORTED \
    0x80430000U 
#define SYN_OPCUA_STATUS_BAD_SECURITY_CHECKS_FAILED \
    0x80130000U 
/* Standard OPC UA Attribute IDs */
#define SYN_OPCUA_ATTR_NODE_ID 1U       
#define SYN_OPCUA_ATTR_NODE_CLASS 2U    
#define SYN_OPCUA_ATTR_BROWSE_NAME 3U   
#define SYN_OPCUA_ATTR_DISPLAY_NAME 4U  
#define SYN_OPCUA_ATTR_VALUE 13U        
#define SYN_OPCUA_ATTR_DATA_TYPE 14U    
#define SYN_OPCUA_ATTR_VALUE_RANK 15U   
#define SYN_OPCUA_ATTR_ACCESS_LEVEL 17U 
/* Standard Node Types & Namespaces */
#define SYN_OPCUA_NS_STANDARD 0U 
#define SYN_OPCUA_NS_VENDOR 1U   
/* Standard Well-Known Node IDs */
#define SYN_OPCUA_NODEID_ROOT 84U                  
#define SYN_OPCUA_NODEID_OBJECTS_FOLDER 85U        
#define SYN_OPCUA_NODEID_SERVER 2253U              
#define SYN_OPCUA_NODEID_SERVER_SERVERSTATUS 2256U 
/* ── Type Definitions ────────────────────────────────────────────────────── */

typedef enum {
    SYN_OPCUA_NODECLASS_UNSPECIFIED = 0,
    SYN_OPCUA_NODECLASS_OBJECT = 1,
    SYN_OPCUA_NODECLASS_VARIABLE = 2,
    SYN_OPCUA_NODECLASS_METHOD = 4
} SYN_OPCUA_NodeClass;

typedef enum {
    SYN_OPCUA_TYPE_NULL = 0,
    SYN_OPCUA_TYPE_BOOLEAN = 1,
    SYN_OPCUA_TYPE_SBYTE = 2,
    SYN_OPCUA_TYPE_BYTE = 3,
    SYN_OPCUA_TYPE_INT16 = 4,
    SYN_OPCUA_TYPE_UINT16 = 5,
    SYN_OPCUA_TYPE_INT32 = 6,
    SYN_OPCUA_TYPE_UINT32 = 7,
    SYN_OPCUA_TYPE_INT64 = 8,
    SYN_OPCUA_TYPE_UINT64 = 9,
    SYN_OPCUA_TYPE_FLOAT = 10,
    SYN_OPCUA_TYPE_DOUBLE = 11,
    SYN_OPCUA_TYPE_STRING = 12,
    SYN_OPCUA_TYPE_DATETIME = 13
} SYN_OPCUA_DataType;

typedef enum { SYN_OPCUA_NODEID_NUMERIC = 0, SYN_OPCUA_NODEID_STRING = 1 } SYN_OPCUA_NodeIdType;

typedef struct {
    uint16_t ns_index;            
    SYN_OPCUA_NodeIdType id_type; 
    union {
        uint32_t num;    
        const char *str; 
    } id;                
} SYN_OPCUA_NodeId;

typedef struct {
    SYN_OPCUA_DataType type; 
    union {
        bool boolean;       
        int8_t sbyte;       
        uint8_t byte;       
        int16_t int16;      
        uint16_t uint16;    
        int32_t int32;      
        uint32_t uint32;    
        int64_t int64;      
        uint64_t uint64;    
        float float_val;    
        double double_val;  
        const char *string; 
    } val;                  
} SYN_OPCUA_Variant;

typedef struct {
    SYN_OPCUA_Variant value; 
    uint32_t status_code;    
    uint64_t source_ts_ms;   
} SYN_OPCUA_DataValue;

struct SYN_OPCUA_Server;
struct SYN_OPCUA_Node;

typedef SYN_Status (*SYN_OPCUA_ReadCb)(struct SYN_OPCUA_Server *srv,
                                       const struct SYN_OPCUA_Node *node,
                                       SYN_OPCUA_DataValue *out_val, void *user_data);

typedef SYN_Status (*SYN_OPCUA_WriteCb)(struct SYN_OPCUA_Server *srv,
                                        const struct SYN_OPCUA_Node *node,
                                        const SYN_OPCUA_DataValue *in_val, void *user_data);

typedef struct SYN_OPCUA_Node {
    SYN_OPCUA_NodeId node_id;       
    SYN_OPCUA_NodeClass node_class; 
    const char *browse_name;        
    const char *display_name;       
    SYN_OPCUA_DataType data_type;   
    SYN_OPCUA_DataValue value;      
    uint8_t access_level;           
    /* Hierarchical parent linking */
    SYN_OPCUA_NodeId parent_id; 
    SYN_OPCUA_ReadCb on_read;   
    SYN_OPCUA_WriteCb on_write; 
    void *user_data;            
} SYN_OPCUA_Node;

typedef struct {
    const char *endpoint_url; 
    const char *server_name;  
    SYN_Transport *transport; 
    void *user_data;          
    uint8_t *rx_buf;    
    size_t rx_buf_size; 
    uint8_t *tx_buf;    
    size_t tx_buf_size; 
} SYN_OPCUA_Config;

typedef enum {
    SYN_OPCUA_STATE_CLOSED = 0,     
    SYN_OPCUA_STATE_HELLO_RECEIVED, 
    SYN_OPCUA_STATE_SECURE_CHANNEL, 
    SYN_OPCUA_STATE_SESSION_ACTIVE  
} SYN_OPCUA_ServerState;

typedef struct SYN_OPCUA_Server {
    SYN_OPCUA_Config cfg;        
    SYN_OPCUA_ServerState state; 
    SYN_PT pt;                   
    uint32_t secure_channel_id; 
    uint32_t security_token_id; 
    uint32_t sequence_number;   
    uint32_t request_id;        
    SYN_OPCUA_Node nodes[SYN_OPCUA_MAX_NODES]; 
    size_t node_count;                         
} SYN_OPCUA_Server;

/* ── Public API ─────────────────────────────────────────────────────────── */

SYN_Status syn_opcua_server_init(SYN_OPCUA_Server *srv, const SYN_OPCUA_Config *cfg);

SYN_Status syn_opcua_server_register_node(SYN_OPCUA_Server *srv, const SYN_OPCUA_Node *node);

SYN_OPCUA_Node *syn_opcua_server_find_node_num(SYN_OPCUA_Server *srv, uint16_t ns, uint32_t num_id);

SYN_Status syn_opcua_server_write_variable(SYN_OPCUA_Server *srv, uint16_t ns, uint32_t num_id,
                                           const SYN_OPCUA_Variant *val);

SYN_Status syn_opcua_server_read_variable(const SYN_OPCUA_Server *srv, uint16_t ns, uint32_t num_id,
                                          SYN_OPCUA_Variant *out_val);

SYN_Status syn_opcua_server_process_message(SYN_OPCUA_Server *srv, const uint8_t *rx, size_t rx_len,
                                            uint8_t *tx, size_t max_tx, size_t *out_len);

SYN_Status syn_opcua_server_step(SYN_OPCUA_Server *srv, uint32_t now_ms);

SYN_PT_Status syn_opcua_server_pt(SYN_PT *pt, SYN_Task *task);

#ifdef __cplusplus
}
#endif

#endif /* SYN_USE_OPCUA */

#endif /* SYN_OPCUA_H */
```


