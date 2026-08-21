

# File syn\_uds.h

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_uds.h**](syn__uds_8h.md)

[Go to the documentation of this file](syn__uds_8h.md)


```C++

#ifndef SYN_UDS_H
#define SYN_UDS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef SYN_UDS_MAX_DIDS
#define SYN_UDS_MAX_DIDS 16U 
#endif
#ifndef SYN_UDS_MAX_SERVICE_OVERRIDES
#define SYN_UDS_MAX_SERVICE_OVERRIDES \
    8U 
#endif
#define SYN_UDS_SID_DIAGNOSTIC_SESSION_CONTROL 0x10U       
#define SYN_UDS_SID_ECU_RESET 0x11U                        
#define SYN_UDS_SID_CLEAR_DIAGNOSTIC_INFORMATION 0x14U     
#define SYN_UDS_SID_READ_DTC_INFORMATION 0x19U             
#define SYN_UDS_SID_READ_DATA_BY_IDENTIFIER 0x22U          
#define SYN_UDS_SID_READ_MEMORY_BY_ADDRESS 0x23U           
#define SYN_UDS_SID_READ_SCALING_DATA_BY_IDENTIFIER 0x24U  
#define SYN_UDS_SID_SECURITY_ACCESS 0x27U                  
#define SYN_UDS_SID_COMMUNICATION_CONTROL 0x28U            
#define SYN_UDS_SID_AUTHENTICATION 0x29U                   
#define SYN_UDS_SID_READ_DATA_BY_PERIODIC_IDENTIFIER 0x2AU 
#define SYN_UDS_SID_DYNAMICALLY_DEFINE_DATA_IDENTIFIER                                  \
    0x2CU                                          
#define SYN_UDS_SID_WRITE_DATA_BY_IDENTIFIER 0x2EU 
#define SYN_UDS_SID_INPUT_OUTPUT_CONTROL_BY_IDENTIFIER                                  \
    0x2FU                                           
#define SYN_UDS_SID_ROUTINE_CONTROL 0x31U           
#define SYN_UDS_SID_REQUEST_DOWNLOAD 0x34U          
#define SYN_UDS_SID_REQUEST_UPLOAD 0x35U            
#define SYN_UDS_SID_TRANSFER_DATA 0x36U             
#define SYN_UDS_SID_REQUEST_TRANSFER_EXIT 0x37U     
#define SYN_UDS_SID_REQUEST_FILE_TRANSFER 0x38U     
#define SYN_UDS_SID_WRITE_MEMORY_BY_ADDRESS 0x3DU   
#define SYN_UDS_SID_TESTER_PRESENT 0x3EU            
#define SYN_UDS_SID_ACCESS_TIMING_PARAMETER 0x83U   
#define SYN_UDS_SID_SECURED_DATA_TRANSMISSION 0x84U 
#define SYN_UDS_SID_CONTROL_DTC_SETTING 0x85U       
#define SYN_UDS_SID_RESPONSE_ON_EVENT 0x86U         
#define SYN_UDS_SID_LINK_CONTROL 0x87U              
#define SYN_UDS_RESET_HARD 0x01U                         
#define SYN_UDS_RESET_KEY_OFF_ON 0x02U                   
#define SYN_UDS_RESET_SOFT 0x03U                         
#define SYN_UDS_RESET_ENABLE_RAPID_POWER_SHUTDOWN 0x04U  
#define SYN_UDS_RESET_DISABLE_RAPID_POWER_SHUTDOWN 0x05U 
#define SYN_UDS_DTC_REPORT_NUMBER_BY_STATUS_MASK                                          \
    0x01U                                       
#define SYN_UDS_DTC_REPORT_BY_STATUS_MASK 0x02U 
#define SYN_UDS_DTC_REPORT_SNAPSHOT_IDENTIFICATION                                              \
    0x03U                                               
#define SYN_UDS_DTC_REPORT_SNAPSHOT_RECORD_BY_DTC 0x04U 
#define SYN_UDS_DTC_REPORT_STORED_DATA_BY_RECORD_NUM \
    0x05U                                               
#define SYN_UDS_DTC_REPORT_EXT_DATA_RECORD_BY_DTC 0x06U 
#define SYN_UDS_DTC_REPORT_NUMBER_BY_SEVERITY_MASK \
    0x07U                                          
#define SYN_UDS_DTC_REPORT_BY_SEVERITY_MASK 0x08U  
#define SYN_UDS_DTC_REPORT_SEVERITY_INFO 0x09U     
#define SYN_UDS_DTC_REPORT_SUPPORTED 0x0AU         
#define SYN_UDS_DTC_REPORT_FIRST_TEST_FAILED 0x0BU 
#define SYN_UDS_DTC_REPORT_FIRST_CONFIRMED 0x0CU   
#define SYN_UDS_DTC_REPORT_MOST_RECENT_TEST_FAILED                                             \
    0x0DU                                              
#define SYN_UDS_DTC_REPORT_MOST_RECENT_CONFIRMED 0x0EU 
#define SYN_UDS_DTC_REPORT_MIRROR_MEMORY_BY_STATUS_MASK \
    0x0FU                                               
#define SYN_UDS_DTC_REPORT_MIRROR_MEMORY_EXT_DATA 0x10U 
#define SYN_UDS_DTC_REPORT_NUMBER_MIRROR_MEMORY_BY_STATUS_MASK \
    0x11U 
#define SYN_UDS_DTC_REPORT_NUMBER_EMISSIONS_OBD_BY_STATUS_MASK \
    0x12U                                                     
#define SYN_UDS_DTC_REPORT_EMISSIONS_OBD_BY_STATUS_MASK 0x13U 
#define SYN_UDS_DTC_REPORT_FAULT_DETECTION_COUNTER 0x14U      
#define SYN_UDS_DTC_REPORT_WITH_PERMANENT_STATUS 0x15U 
#define SYN_UDS_DTC_REPORT_EXT_DATA_RECORD_BY_RECORD_NUM \
    0x16U 
#define SYN_UDS_DTC_REPORT_USER_DEF_MEMORY_BY_STATUS_MASK \
    0x17U 
#define SYN_UDS_DTC_REPORT_USER_DEF_MEMORY_SNAPSHOT_BY_DTC \
    0x18U 
#define SYN_UDS_DTC_REPORT_USER_DEF_MEMORY_EXT_DATA_BY_DTC \
    0x19U                                               
#define SYN_UDS_DTC_REPORT_WWH_OBD_BY_MASK_RECORD 0x42U 
#define SYN_UDS_DTC_REPORT_WWH_OBD_WITH_PERMANENT_STATUS \
    0x55U 
#define SYN_UDS_DTC_GROUP_EMISSIONS 0x000000U  
#define SYN_UDS_DTC_GROUP_POWERTRAIN 0x100000U 
#define SYN_UDS_DTC_GROUP_CHASSIS 0x400000U    
#define SYN_UDS_DTC_GROUP_BODY 0x800000U       
#define SYN_UDS_DTC_GROUP_NETWORK 0xC00000U    
#define SYN_UDS_DTC_GROUP_ALL 0xFFFFFFU        
#define SYN_UDS_RESPONSE_NEGATIVE 0x7FU 
#define SYN_UDS_NRC_SUCCESS 0x00U                   
#define SYN_UDS_NRC_SERVICE_NOT_SUPPORTED 0x11U     
#define SYN_UDS_NRC_SUBFUNCTION_NOT_SUPPORTED 0x12U 
#define SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH \
    0x13U 
#define SYN_UDS_NRC_RESPONSE_TOO_LONG 0x14U           
#define SYN_UDS_NRC_CONDITIONS_NOT_CORRECT 0x22U      
#define SYN_UDS_NRC_REQUEST_OUT_OF_RANGE 0x31U        
#define SYN_UDS_NRC_SECURITY_ACCESS_DENIED 0x33U      
#define SYN_UDS_NRC_INVALID_KEY 0x35U                 
#define SYN_UDS_NRC_EXCEEDED_NUMBER_OF_ATTEMPTS 0x36U 
#define SYN_UDS_NRC_REQUIRED_TIME_DELAY_NOT_EXPIRED \
    0x37U                                             
#define SYN_UDS_NRC_REQUEST_SEQUENCE_ERROR 0x24U      
#define SYN_UDS_NRC_TRANSFER_DATA_SUSPENDED 0x71U     
#define SYN_UDS_NRC_GENERAL_PROGRAMMING_FAILURE 0x72U 
#define SYN_UDS_NRC_WRONG_BLOCK_SEQUENCE_COUNTER 0x73U 
#define SYN_UDS_NRC_SUBFUNCTION_NOT_SUPPORTED_IN_ACTIVE_SESSION \
    0x7EU 
typedef enum {
    SYN_UDS_ADDR_PHYSICAL = 0U,  
    SYN_UDS_ADDR_FUNCTIONAL = 1U 
} SYN_UDS_AddrMode;

typedef enum {
    SYN_UDS_TRANSFER_IDLE = 0U,     
    SYN_UDS_TRANSFER_DOWNLOAD = 1U, 
    SYN_UDS_TRANSFER_UPLOAD = 2U    
} SYN_UDS_TransferState;

typedef enum {
    SYN_UDS_SESSION_DEFAULT = 0x01U,      
    SYN_UDS_SESSION_PROGRAMMING = 0x02U,  
    SYN_UDS_SESSION_EXTENDED = 0x03U,     
    SYN_UDS_SESSION_SAFETY_SYSTEM = 0x04U 
} SYN_UDS_Session;

#define SYN_UDS_SESSION_MASK_DEFAULT (1U << 0)     
#define SYN_UDS_SESSION_MASK_PROGRAMMING (1U << 1) 
#define SYN_UDS_SESSION_MASK_EXTENDED (1U << 2)    
#define SYN_UDS_SESSION_MASK_SAFETY (1U << 3)      
#define SYN_UDS_SESSION_MASK_ALL (0x0FU)           
#define SYN_UDS_SECURITY_MASK_NONE (1U << 0)    
#define SYN_UDS_SECURITY_MASK_LEVEL_1 (1U << 1) 
#define SYN_UDS_SECURITY_MASK_LEVEL_2 (1U << 2) 
#define SYN_UDS_SECURITY_MASK_LEVEL_3 (1U << 3) 
#define SYN_UDS_SECURITY_MASK_ALL (0xFFFFU)     
typedef struct {
    uint16_t did;           
    uint8_t *data;          
    uint16_t len;           
    bool writable;          
    uint8_t session_mask;   
    uint16_t security_mask; 
} SYN_UDS_DIDEntry;

typedef enum {
    SYN_UDS_SECURITY_LOCKED = 0x00U,    
    SYN_UDS_SECURITY_SEED_SENT = 0x01U, 
    SYN_UDS_SECURITY_UNLOCKED = 0x02U   
} SYN_UDS_SecurityState;

typedef enum {
    SYN_UDS_COMM_ENABLE_RX_AND_TX = 0x00U,      
    SYN_UDS_COMM_ENABLE_RX_DISABLE_TX = 0x01U,  
    SYN_UDS_COMM_DISABLE_RX_ENABLE_TX = 0x02U,  
    SYN_UDS_COMM_DISABLE_RX_AND_TX = 0x03U,     
    SYN_UDS_COMM_ENABLE_RX_TX_ENHANCED = 0x04U, 
    SYN_UDS_COMM_ENABLE_RX_ENHANCED = 0x05U     
} SYN_UDS_CommControlType;

typedef enum {
    SYN_UDS_TIMING_READ_EXTENDED = 0x01U,  
    SYN_UDS_TIMING_SET_TO_DEFAULT = 0x02U, 
    SYN_UDS_TIMING_READ_ACTIVE = 0x03U,    
    SYN_UDS_TIMING_SET_TO_GIVEN = 0x04U    
} SYN_UDS_AccessTimingType;

typedef bool (*SYN_UDS_CommControlHandler)(SYN_UDS_CommControlType control_type, uint8_t comm_type,
                                           void *ctx);

typedef bool (*SYN_UDS_AccessTimingHandler)(SYN_UDS_AccessTimingType timing_type,
                                            uint16_t *p2_max_ms, uint16_t *p2_star_max_10ms,
                                            void *ctx);

typedef bool (*SYN_UDS_SecuredDataHandler)(const uint8_t *in_data, uint16_t in_len,
                                           uint8_t *out_buf, uint16_t max_out_len,
                                           uint16_t *out_len, void *ctx);

typedef bool (*SYN_UDS_MemoryHandler)(bool is_write, uint32_t address, uint32_t size,
                                      uint8_t *data_buf, void *ctx);

typedef bool (*SYN_UDS_AuthHandler)(uint8_t subfunction, const uint8_t *in_data, uint16_t in_len,
                                    uint8_t *out_buf, uint16_t max_out_len, uint16_t *out_len,
                                    void *ctx);

typedef bool (*SYN_UDS_FileTransferHandler)(uint8_t mode, const char *file_path, uint16_t path_len,
                                            uint8_t *out_buf, uint16_t max_out_len,
                                            uint16_t *out_len, void *ctx);

typedef bool (*SYN_UDS_RoutineControlHandler)(uint8_t subfunction, uint16_t routine_id,
                                              const uint8_t *in_data, uint16_t in_len,
                                              uint8_t *out_buf, uint16_t max_out_len,
                                              uint16_t *out_len, void *ctx);

typedef bool (*SYN_UDS_IOControlHandler)(uint16_t did, uint8_t control_opt, const uint8_t *in_data,
                                         uint16_t in_len, uint8_t *out_buf, uint16_t max_out_len,
                                         uint16_t *out_len, void *ctx);

typedef bool (*SYN_UDS_LinkControlHandler)(uint8_t subfunction, const uint8_t *in_data,
                                           uint16_t in_len, uint8_t *out_buf, uint16_t max_out_len,
                                           uint16_t *out_len, void *ctx);

typedef bool (*SYN_UDS_ResponseOnEventHandler)(uint8_t subfunction, const uint8_t *in_data,
                                               uint16_t in_len, uint8_t *out_buf,
                                               uint16_t max_out_len, uint16_t *out_len, void *ctx);

typedef bool (*SYN_UDS_ScalingDataHandler)(uint16_t did, uint8_t *out_buf, uint16_t max_out_len,
                                           uint16_t *out_len, void *ctx);

typedef bool (*SYN_UDS_PeriodicDataHandler)(uint8_t mode, const uint8_t *in_data, uint16_t in_len,
                                            uint8_t *out_buf, uint16_t max_out_len,
                                            uint16_t *out_len, void *ctx);

typedef bool (*SYN_UDS_DynamicDIDHandler)(uint8_t subfunction, uint16_t dyn_did,
                                          const uint8_t *in_data, uint16_t in_len, uint8_t *out_buf,
                                          uint16_t max_out_len, uint16_t *out_len, void *ctx);

#ifndef SYN_UDS_MAX_DTCS
#define SYN_UDS_MAX_DTCS 32U 
#endif

#define SYN_UDS_DTC_STATUS_AVAILABILITY_MASK 0xFFU 
#define SYN_UDS_DTC_STATUS_TEST_FAILED (1U << 0) 
#define SYN_UDS_DTC_STATUS_TEST_FAILED_THIS_OP_CYCLE \
    (1U << 1)                                      
#define SYN_UDS_DTC_STATUS_PENDING_DTC (1U << 2)   
#define SYN_UDS_DTC_STATUS_CONFIRMED_DTC (1U << 3) 
#define SYN_UDS_DTC_STATUS_TEST_NOT_COMPLETED_SINCE_LAST_CLEAR \
    (1U << 4) 
#define SYN_UDS_DTC_STATUS_TEST_FAILED_SINCE_LAST_CLEAR (1U << 5) 
#define SYN_UDS_DTC_STATUS_TEST_NOT_COMPLETED_THIS_OP_CYCLE \
    (1U << 6) 
#define SYN_UDS_DTC_STATUS_WARNING_INDICATOR_REQUESTED \
    (1U << 7) 
#define SYN_UDS_DTC_SEVERITY_NO_SEVERITY (0x00U << 5)          
#define SYN_UDS_DTC_SEVERITY_MAINTENANCE_REQUIRED (0x01U << 5) 
#define SYN_UDS_DTC_SEVERITY_CHECK_AT_NEXT_HALT (0x02U << 5)   
#define SYN_UDS_DTC_SEVERITY_CHECK_IMMEDIATELY (0x03U << 5)    
#define SYN_UDS_DTC_SEVERITY_MASK (0x07U << 5)                 
#define SYN_UDS_DTC_CLASS_MASK (0x1FU)                         
#define SYN_UDS_DTC_FORMAT_ISO14229_1 0x00U   
#define SYN_UDS_DTC_FORMAT_ISO15031_6 0x01U   
#define SYN_UDS_DTC_FORMAT_SAE_J1939_73 0x02U 
#define SYN_UDS_DTC_FORMAT_ISO27145_4 0x03U   
typedef struct {
    uint32_t dtc;     
    uint8_t status;   
    uint8_t severity; 
    int8_t fault_cnt; 
} SYN_UDS_DTCEntry;

typedef bool (*SYN_UDS_DTCHandler)(uint8_t subfunction, const uint8_t *in_data, uint16_t in_len,
                                   uint8_t *out_buf, uint16_t max_out_len, uint16_t *out_len,
                                   void *ctx);

typedef void (*SYN_UDS_ResetHandler)(uint8_t reset_type, void *ctx);

typedef bool (*SYN_UDS_SessionTransitionHandler)(SYN_UDS_Session from_session,
                                                 SYN_UDS_Session to_session, void *ctx);

#ifndef SYN_UDS_S3_TIMEOUT_MS
#define SYN_UDS_S3_TIMEOUT_MS 5000U 
#endif

#ifndef SYN_UDS_SECURITY_MAX_ATTEMPTS
#define SYN_UDS_SECURITY_MAX_ATTEMPTS 3U 
#endif

#ifndef SYN_UDS_SECURITY_DELAY_MS
#define SYN_UDS_SECURITY_DELAY_MS 10000U 
#endif

#ifndef SYN_UDS_DEFAULT_RESET_TX_WAIT_MS
#define SYN_UDS_DEFAULT_RESET_TX_WAIT_MS 50U 
#endif
typedef struct {
    SYN_UDS_Session session;              
    SYN_UDS_SecurityState security_state; 
    uint8_t
        security_level;    
    uint32_t current_seed; 
    uint32_t active_seed;  
    uint8_t active_seed_subfunction;  
    bool use_aes128_security;         
    uint8_t aes_security_key[16];     
    uint8_t current_seed_bytes[16];   
    uint8_t active_seed_bytes[16];    
    uint32_t s3_timer_ms;             
    uint8_t security_error_count;     
    uint32_t security_delay_timer_ms; 
    SYN_UDS_CommControlType comm_control_state;   
    uint8_t comm_type;                            
    SYN_UDS_CommControlHandler comm_control_cb;   
    void *comm_control_ctx;                       
    uint16_t p2_max_ms;                           
    uint16_t p2_star_max_10ms;                    
    uint16_t active_p2_max_ms;                    
    uint16_t active_p2_star_max_10ms;             
    SYN_UDS_AccessTimingHandler timing_cb;        
    void *timing_ctx;                             
    SYN_UDS_SecuredDataHandler secured_data_cb;   
    void *secured_data_ctx;                       
    SYN_UDS_MemoryHandler memory_cb;              
    void *memory_ctx;                             
    SYN_UDS_AuthHandler auth_cb;                  
    void *auth_ctx;                               
    SYN_UDS_FileTransferHandler file_transfer_cb; 
    void *file_transfer_ctx;                    
    SYN_UDS_RoutineControlHandler routine_cb;   
    void *routine_ctx;                          
    SYN_UDS_IOControlHandler io_control_cb;     
    void *io_control_ctx;                       
    SYN_UDS_LinkControlHandler link_control_cb; 
    void *link_control_ctx;                     
    SYN_UDS_ResponseOnEventHandler roe_cb;      
    void *roe_ctx;                              
    SYN_UDS_ScalingDataHandler scaling_cb;      
    void *scaling_ctx;                          
    SYN_UDS_PeriodicDataHandler periodic_cb;    
    void *periodic_ctx;                         
    SYN_UDS_DynamicDIDHandler dynamic_did_cb;   
    void *dynamic_did_ctx;                      
    SYN_UDS_DTCHandler dtc_cb;                  
    void *dtc_ctx;                              
    SYN_UDS_ResetHandler reset_cb;              
    void *reset_ctx;                            
    uint16_t reset_tx_wait_ms;                  
    uint32_t reset_wait_elapsed_ms;             
    SYN_UDS_SessionTransitionHandler
        session_transition_cb;                    
    void *session_transition_ctx;                 
    SYN_UDS_DIDEntry did_table[SYN_UDS_MAX_DIDS]; 
    uint8_t did_count;                            
    SYN_UDS_DTCEntry dtc_table[SYN_UDS_MAX_DTCS]; 
    uint8_t dtc_count;                            
    SYN_UDS_TransferState transfer_state;         
    uint32_t transfer_address;                    
    uint32_t transfer_size;                       
    uint32_t transfer_bytes_processed;            
    uint8_t expected_block_seq;                   
    uint8_t reset_type_requested;                 
    uint8_t custom_session_sids[SYN_UDS_MAX_SERVICE_OVERRIDES];  
    uint8_t custom_session_masks[SYN_UDS_MAX_SERVICE_OVERRIDES]; 
    uint8_t custom_session_count; 
    uint8_t custom_security_sids[SYN_UDS_MAX_SERVICE_OVERRIDES]; 
    uint16_t
        custom_security_masks[SYN_UDS_MAX_SERVICE_OVERRIDES]; 
    uint8_t custom_security_count;                            
} SYN_UDS_Server;

bool syn_uds_enable_aes128_security(SYN_UDS_Server *server, const uint8_t key[16]);

bool syn_uds_disable_aes128_security(SYN_UDS_Server *server);

bool syn_uds_set_security_seed_bytes(SYN_UDS_Server *server, const uint8_t seed[16]);

uint8_t syn_uds_get_security_level(const SYN_UDS_Server *server);

bool syn_uds_init(SYN_UDS_Server *server);

void syn_uds_tick(SYN_UDS_Server *server, uint32_t dt_ms);

void syn_uds_set_session_transition_handler(SYN_UDS_Server *server,
                                            SYN_UDS_SessionTransitionHandler cb, void *ctx);

void syn_uds_set_reset_handler(SYN_UDS_Server *server, SYN_UDS_ResetHandler cb, void *ctx);

void syn_uds_set_reset_wait_ms(SYN_UDS_Server *server, uint16_t wait_ms);

uint8_t syn_uds_get_pending_reset(const SYN_UDS_Server *server);

void syn_uds_clear_pending_reset(SYN_UDS_Server *server);

bool syn_uds_register_did(SYN_UDS_Server *server, uint16_t did, uint8_t *data, uint16_t len,
                          bool writable);

bool syn_uds_register_did_ext(SYN_UDS_Server *server, uint16_t did, uint8_t *data, uint16_t len,
                              bool writable, uint8_t session_mask, uint16_t security_mask);

bool syn_uds_register_dtc(SYN_UDS_Server *server, uint32_t dtc, uint8_t status, uint8_t severity);

bool syn_uds_register_dtc_handler(SYN_UDS_Server *server, SYN_UDS_DTCHandler handler, void *ctx);

bool syn_uds_register_comm_control(SYN_UDS_Server *server, SYN_UDS_CommControlHandler handler,
                                   void *ctx);

bool syn_uds_register_access_timing(SYN_UDS_Server *server, SYN_UDS_AccessTimingHandler handler,
                                    void *ctx);

bool syn_uds_register_secured_data(SYN_UDS_Server *server, SYN_UDS_SecuredDataHandler handler,
                                   void *ctx);

bool syn_uds_register_memory_handler(SYN_UDS_Server *server, SYN_UDS_MemoryHandler handler,
                                     void *ctx);

bool syn_uds_register_auth_handler(SYN_UDS_Server *server, SYN_UDS_AuthHandler handler, void *ctx);

bool syn_uds_register_file_transfer(SYN_UDS_Server *server, SYN_UDS_FileTransferHandler handler,
                                    void *ctx);

bool syn_uds_register_routine_control(SYN_UDS_Server *server, SYN_UDS_RoutineControlHandler handler,
                                      void *ctx);

bool syn_uds_register_io_control(SYN_UDS_Server *server, SYN_UDS_IOControlHandler handler,
                                 void *ctx);

bool syn_uds_register_link_control(SYN_UDS_Server *server, SYN_UDS_LinkControlHandler handler,
                                   void *ctx);

bool syn_uds_register_roe_handler(SYN_UDS_Server *server, SYN_UDS_ResponseOnEventHandler handler,
                                  void *ctx);

bool syn_uds_register_scaling_data_handler(SYN_UDS_Server *server,
                                           SYN_UDS_ScalingDataHandler handler, void *ctx);

bool syn_uds_register_periodic_data_handler(SYN_UDS_Server *server,
                                            SYN_UDS_PeriodicDataHandler handler, void *ctx);

bool syn_uds_register_dynamic_did_handler(SYN_UDS_Server *server, SYN_UDS_DynamicDIDHandler handler,
                                          void *ctx);

bool syn_uds_set_service_session_mask(SYN_UDS_Server *server, uint8_t sid, uint8_t session_mask);

bool syn_uds_set_service_security_mask(SYN_UDS_Server *server, uint8_t sid, uint16_t security_mask);

bool syn_uds_process_request(SYN_UDS_Server *server, const uint8_t *req, uint16_t req_len,
                             uint8_t *resp_buf, uint16_t max_resp_len, uint16_t *resp_len,
                             SYN_UDS_AddrMode addr_mode);

bool syn_uds_is_sid_functional_supported(uint8_t sid);

bool syn_uds_dtc_report_test_result(SYN_UDS_Server *server, uint32_t dtc, bool failed);

bool syn_uds_dtc_start_operation_cycle(SYN_UDS_Server *server);

bool syn_uds_dtc_get_status(SYN_UDS_Server *server, uint32_t dtc, uint8_t *out_status);

#ifdef __cplusplus
}
#endif

#endif /* SYN_UDS_H */
```


