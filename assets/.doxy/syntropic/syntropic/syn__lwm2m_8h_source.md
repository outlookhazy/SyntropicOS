

# File syn\_lwm2m.h

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_lwm2m.h**](syn__lwm2m_8h.md)

[Go to the documentation of this file](syn__lwm2m_8h.md)


```C++

#ifndef SYN_LWM2M_H
#define SYN_LWM2M_H

#include "syntropic/common/syn_defs.h"
#include "syntropic/net/syn_coap.h"

#if !defined(SYN_USE_LWM2M) || SYN_USE_LWM2M

#ifdef __cplusplus
extern "C" {
#endif

/* ── Standard OMA LwM2M Object IDs ───────────────────────────────────────── */

#define SYN_LWM2M_OBJ_SECURITY 0U                
#define SYN_LWM2M_OBJ_SERVER 1U                  
#define SYN_LWM2M_OBJ_ACCESS_CONTROL 2U          
#define SYN_LWM2M_OBJ_DEVICE 3U                  
#define SYN_LWM2M_OBJ_CONNECTIVITY_MONITORING 4U 
#define SYN_LWM2M_OBJ_FIRMWARE_UPDATE 5U         
#define SYN_LWM2M_OBJ_LOCATION 6U                
#define SYN_LWM2M_OBJ_CONNECTIVITY_STATS 7U      
#define SYN_LWM2M_OBJ_GENERIC_SENSOR 3300U       
#define SYN_LWM2M_OBJ_TEMPERATURE 3303U          
#define SYN_LWM2M_OBJ_HUMIDITY 3304U             
#define SYN_LWM2M_OBJ_ACTUATION 3306U            
/* ── Standard OMA Content-Formats ────────────────────────────────────────── */

#define SYN_LWM2M_FORMAT_TEXT 0U         
#define SYN_LWM2M_FORMAT_LINK_FORMAT 40U 
#define SYN_LWM2M_FORMAT_OPAQUE 42U      
#define SYN_LWM2M_FORMAT_SENML_CBOR 112U 
#define SYN_LWM2M_FORMAT_TLV 11542U      
#define SYN_LWM2M_FORMAT_JSON 11543U     
#define SYN_LWM2M_FORMAT_CBOR 11544U     
/* ── Limits & Constants ─────────────────────────────────────────────────── */

#define SYN_LWM2M_MAX_OBJECTS 16U      
#define SYN_LWM2M_MAX_OBSERVERS 8U     
#define SYN_LWM2M_MAX_URI_PATH_LEN 64U 
/* ── Resource Operations Bitmask ─────────────────────────────────────────── */

#define SYN_LWM2M_OP_NONE 0x00U                           
#define SYN_LWM2M_OP_R 0x01U                              
#define SYN_LWM2M_OP_W 0x02U                              
#define SYN_LWM2M_OP_E 0x04U                              
#define SYN_LWM2M_OP_RW (SYN_LWM2M_OP_R | SYN_LWM2M_OP_W) 
/* ── Resource Value Types ────────────────────────────────────────────────── */

typedef enum {
    SYN_LWM2M_TYPE_NONE = 0, 
    SYN_LWM2M_TYPE_STRING,   
    SYN_LWM2M_TYPE_INT,      
    SYN_LWM2M_TYPE_FLOAT,    
    SYN_LWM2M_TYPE_BOOL,     
    SYN_LWM2M_TYPE_OPAQUE,   
    SYN_LWM2M_TYPE_TIME,     
    SYN_LWM2M_TYPE_OBJLNK,   
} SYN_LwM2M_ValType;

typedef struct {
    SYN_LwM2M_ValType type; 
    union {
        int64_t integer; 
        double floating; 
        bool boolean;    
        const char *str; 
        struct {
            const uint8_t *data; 
            size_t len;          
        } opaque;                
        struct {
            uint16_t obj_id;  
            uint16_t inst_id; 
        } objlnk;             
    } val;                    
} SYN_LwM2M_Value;

/* ── TLV (Type-Length-Value) Binary Format ────────────────────────────────── */

typedef enum {
    SYN_LWM2M_TLV_OBJECT_INSTANCE = 0x00U,   
    SYN_LWM2M_TLV_RESOURCE_INSTANCE = 0x40U, 
    SYN_LWM2M_TLV_MULTIPLE_RESOURCE = 0x80U, 
    SYN_LWM2M_TLV_RESOURCE = 0xC0U,          
} SYN_LwM2M_TLV_Type;

typedef struct {
    SYN_LwM2M_TLV_Type type; 
    uint16_t id;             
    const uint8_t *val;      
    size_t len;              
} SYN_LwM2M_TLV;

/* ── Object & Resource Descriptors ───────────────────────────────────────── */

typedef struct {
    uint16_t id;            
    uint8_t operations;     
    SYN_LwM2M_ValType type; 
} SYN_LwM2M_ResourceDesc;

typedef struct SYN_LwM2M_Object SYN_LwM2M_Object;

typedef SYN_Status (*SYN_LwM2M_ReadCb)(uint16_t instance_id, uint16_t resource_id,
                                       SYN_LwM2M_Value *out_val, void *user_ctx);

typedef SYN_Status (*SYN_LwM2M_WriteCb)(uint16_t instance_id, uint16_t resource_id,
                                        const SYN_LwM2M_Value *in_val, void *user_ctx);

typedef SYN_Status (*SYN_LwM2M_ExecCb)(uint16_t instance_id, uint16_t resource_id,
                                       const uint8_t *args, size_t args_len, void *user_ctx);

struct SYN_LwM2M_Object {
    uint16_t id;                             
    uint16_t instance_count;                 
    const uint16_t *instances;               
    size_t resource_count;                   
    const SYN_LwM2M_ResourceDesc *resources; 
    SYN_LwM2M_ReadCb read;                   
    SYN_LwM2M_WriteCb write;                 
    SYN_LwM2M_ExecCb exec;                   
    void *user_ctx;                          
};

/* ── Observation Table ───────────────────────────────────────────────────── */

typedef struct {
    bool active;             
    uint8_t token_len;       
    uint8_t token[8];        
    uint16_t obj_id;         
    uint16_t inst_id;        
    int32_t res_id;          
    uint32_t pmin;           
    uint32_t pmax;           
    uint32_t last_notify_ms; 
    uint32_t seq;            
} SYN_LwM2M_Observation;

/* ── LwM2M Client State Machine ──────────────────────────────────────────── */

typedef enum {
    SYN_LWM2M_STATE_DEREGISTERED = 0, 
    SYN_LWM2M_STATE_REGISTERING,      
    SYN_LWM2M_STATE_REGISTERED,       
    SYN_LWM2M_STATE_UPDATING,         
} SYN_LwM2M_ClientState;

typedef struct {
    const char *endpoint_name; 
    uint32_t lifetime_s;       
    const char *binding;       
    SYN_Transport *transport;  
    SYN_LwM2M_ClientState state;                    
    char location_path[SYN_LWM2M_MAX_URI_PATH_LEN]; 
    uint32_t last_update_ms; 
    const SYN_LwM2M_Object *objects[SYN_LWM2M_MAX_OBJECTS]; 
    size_t object_count;                                    
    SYN_LwM2M_Observation observations[SYN_LWM2M_MAX_OBSERVERS]; 
} SYN_LwM2M_Client;

/* ── TLV Codec API ───────────────────────────────────────────────────────── */

size_t syn_lwm2m_tlv_encode(const SYN_LwM2M_TLV *tlv, uint8_t *buf, size_t max_len);

size_t syn_lwm2m_tlv_encode_int(uint16_t res_id, int64_t val, uint8_t *buf, size_t max_len);

size_t syn_lwm2m_tlv_encode_float(uint16_t res_id, double val, uint8_t *buf, size_t max_len);

size_t syn_lwm2m_tlv_encode_bool(uint16_t res_id, bool val, uint8_t *buf, size_t max_len);

size_t syn_lwm2m_tlv_encode_string(uint16_t res_id, const char *str, uint8_t *buf, size_t max_len);

size_t syn_lwm2m_tlv_encode_opaque(uint16_t res_id, const uint8_t *data, size_t len, uint8_t *buf,
                                   size_t max_len);

size_t syn_lwm2m_tlv_encode_value(uint16_t res_id, const SYN_LwM2M_Value *val, uint8_t *buf,
                                  size_t max_len);

size_t syn_lwm2m_tlv_encode_instance(uint16_t inst_id, const uint8_t *tlv_payload,
                                     size_t payload_len, uint8_t *buf, size_t max_len);

bool syn_lwm2m_tlv_decode(const uint8_t *buf, size_t buf_len, SYN_LwM2M_TLV *tlv, size_t *consumed);

bool syn_lwm2m_tlv_decode_int(const SYN_LwM2M_TLV *tlv, int64_t *out_val);

bool syn_lwm2m_tlv_decode_float(const SYN_LwM2M_TLV *tlv, double *out_val);

bool syn_lwm2m_tlv_decode_bool(const SYN_LwM2M_TLV *tlv, bool *out_val);

bool syn_lwm2m_tlv_decode_string(const SYN_LwM2M_TLV *tlv, char *out_str, size_t max_len);

bool syn_lwm2m_tlv_decode_value(const SYN_LwM2M_TLV *tlv, SYN_LwM2M_ValType expected_type,
                                SYN_LwM2M_Value *out_val);

/* ── Core Link Format & Registration API ─────────────────────────────────── */

size_t syn_lwm2m_generate_link_format(const SYN_LwM2M_Client *client, char *buf, size_t max_len);

bool syn_lwm2m_client_init(SYN_LwM2M_Client *client, const char *endpoint_name, uint32_t lifetime_s,
                           SYN_Transport *transport);

bool syn_lwm2m_register_object(SYN_LwM2M_Client *client, const SYN_LwM2M_Object *obj);

size_t syn_lwm2m_build_register_request(const SYN_LwM2M_Client *client, uint16_t msg_id,
                                        const uint8_t *token, uint8_t token_len, SYN_CoapMsg *req,
                                        SYN_CoapOption *req_opts, size_t max_req_opts,
                                        size_t *req_opt_cnt, uint8_t *payload_buf,
                                        size_t payload_buf_sz);

size_t syn_lwm2m_build_update_request(const SYN_LwM2M_Client *client, uint16_t msg_id,
                                      const uint8_t *token, uint8_t token_len, uint32_t lifetime_s,
                                      SYN_CoapMsg *req, SYN_CoapOption *req_opts,
                                      size_t max_req_opts, size_t *req_opt_cnt,
                                      uint8_t *payload_buf, size_t payload_buf_sz);

size_t syn_lwm2m_build_deregister_request(const SYN_LwM2M_Client *client, uint16_t msg_id,
                                          const uint8_t *token, uint8_t token_len, SYN_CoapMsg *req,
                                          SYN_CoapOption *req_opts, size_t max_req_opts,
                                          size_t *req_opt_cnt);

bool syn_lwm2m_handle_register_response(SYN_LwM2M_Client *client, const SYN_CoapMsg *resp,
                                        const SYN_CoapOption *resp_opts, size_t resp_opt_cnt);

/* ── Request Processing & Device Management ──────────────────────────────── */

SYN_Status syn_lwm2m_process_request(SYN_LwM2M_Client *client, const SYN_CoapMsg *req,
                                     const SYN_CoapOption *req_opts, size_t req_opt_cnt,
                                     SYN_CoapMsg *resp, SYN_CoapOption *resp_opts,
                                     size_t max_resp_opts, size_t *resp_opt_cnt, uint8_t *resp_buf,
                                     size_t resp_buf_sz);

/* ── Observation & Notification API (RFC 7641) ───────────────────────────── */

bool syn_lwm2m_observe_add(SYN_LwM2M_Client *client, uint16_t obj_id, uint16_t inst_id,
                           int32_t res_id, const uint8_t *token, uint8_t token_len, uint32_t pmin,
                           uint32_t pmax);

bool syn_lwm2m_observe_remove(SYN_LwM2M_Client *client, const uint8_t *token, uint8_t token_len);

size_t syn_lwm2m_build_notify(SYN_LwM2M_Client *client, uint16_t obj_id, uint16_t inst_id,
                              int32_t res_id, uint16_t msg_id, SYN_CoapMsg *resp,
                              SYN_CoapOption *resp_opts, size_t max_resp_opts, size_t *resp_opt_cnt,
                              uint8_t *resp_buf, size_t resp_buf_sz);

/* ── Standard Standard Object Constructors ───────────────────────────────── */

typedef struct {
    const char *manufacturer;     
    const char *model_number;     
    const char *serial_number;    
    const char *firmware_ver;     
    int64_t battery_level;        
    int64_t memory_free_kb;       
    int64_t error_code;           
    int64_t current_time;         
    const char *utc_offset;       
    const char *timezone;         
    bool reboot_requested;        
    bool factory_reset_requested; 
} SYN_LwM2M_DeviceContext;

SYN_LwM2M_Object syn_lwm2m_make_device_object(SYN_LwM2M_DeviceContext *ctx);

typedef enum {
    SYN_LWM2M_FW_STATE_IDLE = 0,    
    SYN_LWM2M_FW_STATE_DOWNLOADING, 
    SYN_LWM2M_FW_STATE_DOWNLOADED,  
    SYN_LWM2M_FW_STATE_UPDATING,    
} SYN_LwM2M_FirmwareState;

typedef enum {
    SYN_LWM2M_FW_RESULT_DEFAULT = 0,    
    SYN_LWM2M_FW_RESULT_SUCCESS,        
    SYN_LWM2M_FW_RESULT_NO_FLASH,       
    SYN_LWM2M_FW_RESULT_OUT_OF_RAM,     
    SYN_LWM2M_FW_RESULT_CONN_LOST,      
    SYN_LWM2M_FW_RESULT_INTEGRITY_FAIL, 
    SYN_LWM2M_FW_RESULT_BAD_PKG_TYPE,   
    SYN_LWM2M_FW_RESULT_INVALID_URI,    
} SYN_LwM2M_FirmwareResult;

typedef struct {
    SYN_LwM2M_FirmwareState state;   
    SYN_LwM2M_FirmwareResult result; 
    char package_uri[128];           
    char pkg_name[32];               
    char pkg_version[32];            
    bool update_requested;           
} SYN_LwM2M_FirmwareContext;

SYN_LwM2M_Object syn_lwm2m_make_firmware_object(SYN_LwM2M_FirmwareContext *ctx);

typedef struct {
    double sensor_value;     
    const char *unit;        
    double min_measured_val; 
    double max_measured_val; 
} SYN_LwM2M_SensorContext;

SYN_LwM2M_Object syn_lwm2m_make_temperature_object(SYN_LwM2M_SensorContext *ctx);

#ifdef __cplusplus
}
#endif

#endif /* SYN_USE_LWM2M */

#endif /* SYN_LWM2M_H */
```


