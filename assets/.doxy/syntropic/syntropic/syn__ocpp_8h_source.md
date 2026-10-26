

# File syn\_ocpp.h

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_ocpp.h**](syn__ocpp_8h.md)

[Go to the documentation of this file](syn__ocpp_8h.md)


```C++

#ifndef SYN_OCPP_H
#define SYN_OCPP_H

#include "../common/syn_defs.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SYN_OCPP_MSG_TYPE_CALL 2U       
#define SYN_OCPP_MSG_TYPE_CALLRESULT 3U 
#define SYN_OCPP_MSG_TYPE_CALLERROR 4U  
#define SYN_OCPP_ERROR_NOT_IMPLEMENTED "NotImplemented"         
#define SYN_OCPP_ERROR_NOT_SUPPORTED "NotSupported"             
#define SYN_OCPP_ERROR_INTERNAL_ERROR "InternalError"           
#define SYN_OCPP_ERROR_PROTOCOL_ERROR "ProtocolError"           
#define SYN_OCPP_ERROR_SECURITY_ERROR "SecurityError"           
#define SYN_OCPP_ERROR_FORMATION_VIOLATION "FormationViolation" 
#define SYN_OCPP_ERROR_PROPERTY_CONSTRAINT_VIOLATION \
    "PropertyConstraintViolation" 
#define SYN_OCPP_ERROR_OCCURRENCE_CONSTRAINT_VIOLATION \
    "OccurenceConstraintViolation" 
#define SYN_OCPP_ERROR_TYPE_CONSTRAINT_VIOLATION \
    "TypeConstraintViolation"                       
#define SYN_OCPP_ERROR_GENERIC_ERROR "GenericError" 
#define SYN_OCPP_SUBPROTOCOL_2_1 "ocpp2.1"     
#define SYN_OCPP_SUBPROTOCOL_2_0_1 "ocpp2.0.1" 
#define SYN_OCPP_SUBPROTOCOL_1_6 "ocpp1.6"     
typedef enum {
    SYN_OCPP_VERSION_1_6 = 0, 
    SYN_OCPP_VERSION_2_0_1,   
    SYN_OCPP_VERSION_2_1      
} SYN_OCPP_Version;

typedef enum {
    SYN_OCPP_V2G_SCHEDULED = 0, 
    SYN_OCPP_V2G_DYNAMIC        
} SYN_OCPP_V2GMode;

typedef struct {
    uint32_t message_id;     
    const char *header;      
    const char *content;     
    uint32_t display_time_s; 
} SYN_OCPP_DisplayMessage;

typedef enum {
    SYN_OCPP_STATUS_AVAILABLE = 0,  
    SYN_OCPP_STATUS_PREPARING,      
    SYN_OCPP_STATUS_CHARGING,       
    SYN_OCPP_STATUS_SUSPENDED_EV,   
    SYN_OCPP_STATUS_SUSPENDED_EVSE, 
    SYN_OCPP_STATUS_FINISHING,      
    SYN_OCPP_STATUS_RESERVED,       
    SYN_OCPP_STATUS_UNAVAILABLE,    
    SYN_OCPP_STATUS_FAULTED         
} SYN_OCPP_ChargePointStatus;

typedef enum {
    SYN_OCPP_REGISTRATION_ACCEPTED = 0, 
    SYN_OCPP_REGISTRATION_PENDING,      
    SYN_OCPP_REGISTRATION_REJECTED      
} SYN_OCPP_RegistrationStatus;

typedef enum {
    SYN_OCPP_AUTH_ACCEPTED = 0, 
    SYN_OCPP_AUTH_BLOCKED,      
    SYN_OCPP_AUTH_EXPIRED,      
    SYN_OCPP_AUTH_INVALID       
} SYN_OCPP_AuthorizationStatus;

typedef struct {
    const char *charge_point_vendor; 
    const char *charge_point_model;  
    const char *serial_number;       
    const char *firmware_version;    
} SYN_OCPP_ChargePointInfo;

typedef struct {
    uint32_t energy_wh;  
    uint16_t voltage_v;  
    uint16_t current_a;  
    uint16_t power_kw;   
    uint8_t soc_percent; 
} SYN_OCPP_MeterValues;

/* ── Client Callbacks ── */

typedef bool (*SYN_OCPP_RemoteStartHandler)(uint32_t connector_id, const char *id_tag,
                                            void *user_ctx);

typedef bool (*SYN_OCPP_RemoteStopHandler)(int32_t transaction_id, void *user_ctx);

typedef void (*SYN_OCPP_RegistrationHandler)(SYN_OCPP_RegistrationStatus status,
                                             uint32_t heartbeat_interval_sec, void *user_ctx);

typedef void (*SYN_OCPP_AuthorizationHandler)(const char *id_tag,
                                              SYN_OCPP_AuthorizationStatus status, void *user_ctx);

typedef void (*SYN_OCPP_StartTxHandler)(int32_t transaction_id, SYN_OCPP_AuthorizationStatus status,
                                        void *user_ctx);

typedef struct {
    SYN_OCPP_Version version;  
    SYN_OCPP_V2GMode v2g_mode; 
    SYN_OCPP_RegistrationStatus registration_status; 
    uint32_t heartbeat_interval_sec;                 
    uint32_t heartbeat_timer_ms;                     
    uint32_t connector_id;                       
    SYN_OCPP_ChargePointStatus connector_status; 
    int32_t active_transaction_id; 
    char active_id_tag[32];        
    uint32_t tx_start_energy_wh;   
    uint32_t message_counter; 
    SYN_OCPP_RegistrationHandler reg_cb;         
    SYN_OCPP_AuthorizationHandler auth_cb;       
    SYN_OCPP_StartTxHandler start_tx_cb;         
    SYN_OCPP_RemoteStartHandler remote_start_cb; 
    SYN_OCPP_RemoteStopHandler remote_stop_cb;   
    void *user_ctx;                              
} SYN_OCPP_Client;

/* ── Server Callbacks ── */

typedef SYN_OCPP_RegistrationStatus (*SYN_OCPP_ServerBootHandler)(
    const SYN_OCPP_ChargePointInfo *info, uint32_t *heartbeat_sec, void *user_ctx);

typedef SYN_OCPP_AuthorizationStatus (*SYN_OCPP_ServerAuthorizeHandler)(const char *id_tag,
                                                                        void *user_ctx);

typedef int32_t (*SYN_OCPP_ServerStartTxHandler)(uint32_t connector_id, const char *id_tag,
                                                 uint32_t meter_start_wh, void *user_ctx);

typedef struct {
    uint32_t message_counter;                  
    int32_t next_transaction_id;               
    SYN_OCPP_ServerBootHandler boot_cb;        
    SYN_OCPP_ServerAuthorizeHandler auth_cb;   
    SYN_OCPP_ServerStartTxHandler start_tx_cb; 
    void *user_ctx;                            
} SYN_OCPP_Server;

/* ── Client API Declarations ─────────────────────────────────────────── */

SYN_Status syn_ocpp_init(SYN_OCPP_Client *client);

SYN_Status syn_ocpp_set_callbacks(SYN_OCPP_Client *client, SYN_OCPP_RegistrationHandler reg_cb,
                                  SYN_OCPP_AuthorizationHandler auth_cb,
                                  SYN_OCPP_StartTxHandler start_tx_cb,
                                  SYN_OCPP_RemoteStartHandler remote_start_cb,
                                  SYN_OCPP_RemoteStopHandler remote_stop_cb, void *user_ctx);

SYN_Status syn_ocpp_format_boot_notification(SYN_OCPP_Client *client,
                                             const SYN_OCPP_ChargePointInfo *info, char *out_buf,
                                             size_t max_len, size_t *out_len);

SYN_Status syn_ocpp_format_heartbeat(SYN_OCPP_Client *client, char *out_buf, size_t max_len,
                                     size_t *out_len);

SYN_Status syn_ocpp_format_status_notification(SYN_OCPP_Client *client, uint32_t connector_id,
                                               SYN_OCPP_ChargePointStatus status,
                                               const char *error_code, char *out_buf,
                                               size_t max_len, size_t *out_len);

SYN_Status syn_ocpp_format_authorize(SYN_OCPP_Client *client, const char *id_tag, char *out_buf,
                                     size_t max_len, size_t *out_len);

SYN_Status syn_ocpp_format_start_transaction(SYN_OCPP_Client *client, uint32_t connector_id,
                                             const char *id_tag, uint32_t meter_start_wh,
                                             char *out_buf, size_t max_len, size_t *out_len);

SYN_Status syn_ocpp_format_stop_transaction(SYN_OCPP_Client *client, int32_t transaction_id,
                                            uint32_t meter_stop_wh, const char *reason,
                                            char *out_buf, size_t max_len, size_t *out_len);

SYN_Status syn_ocpp_format_meter_values(SYN_OCPP_Client *client, uint32_t connector_id,
                                        const SYN_OCPP_MeterValues *values, char *out_buf,
                                        size_t max_len, size_t *out_len);

SYN_Status syn_ocpp_format_display_message(SYN_OCPP_Client *client,
                                           const SYN_OCPP_DisplayMessage *msg, char *out_buf,
                                           size_t max_len, size_t *out_len);

SYN_Status syn_ocpp_format_v2g_energy_transfer(SYN_OCPP_Client *client, SYN_OCPP_V2GMode mode,
                                               int32_t power_limit_w, char *out_buf, size_t max_len,
                                               size_t *out_len);

SYN_Status syn_ocpp_process_message(SYN_OCPP_Client *client, const char *in_buf, size_t in_len,
                                    char *out_resp, size_t max_resp_len, size_t *out_resp_len);

void syn_ocpp_tick(SYN_OCPP_Client *client, uint32_t dt_ms, char *out_hb_buf, size_t max_len,
                   size_t *out_len);

/* ── Server API Declarations (CSMS Role) ─────────────────────────────── */

SYN_Status syn_ocpp_server_init(SYN_OCPP_Server *server);

SYN_Status syn_ocpp_server_set_callbacks(SYN_OCPP_Server *server,
                                         SYN_OCPP_ServerBootHandler boot_cb,
                                         SYN_OCPP_ServerAuthorizeHandler auth_cb,
                                         SYN_OCPP_ServerStartTxHandler start_tx_cb, void *user_ctx);

SYN_Status syn_ocpp_server_format_remote_start(SYN_OCPP_Server *server, uint32_t connector_id,
                                               const char *id_tag, char *out_buf, size_t max_len,
                                               size_t *out_len);

SYN_Status syn_ocpp_server_format_remote_stop(SYN_OCPP_Server *server, int32_t transaction_id,
                                              char *out_buf, size_t max_len, size_t *out_len);

SYN_Status syn_ocpp_server_process_message(SYN_OCPP_Server *server, const char *in_buf,
                                           size_t in_len, char *out_resp, size_t max_resp_len,
                                           size_t *out_resp_len);

SYN_Status syn_ocpp_format_call_error(const char *msg_id, const char *error_code,
                                      const char *error_description, char *out_buf, size_t max_len,
                                      size_t *out_len);

#ifdef __cplusplus
}
#endif

#endif /* SYN_OCPP_H */
```


