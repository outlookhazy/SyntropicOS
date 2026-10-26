

# File syn\_kwp2000.h

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_kwp2000.h**](syn__kwp2000_8h.md)

[Go to the documentation of this file](syn__kwp2000_8h.md)


```C++

#ifndef SYN_KWP2000_H
#define SYN_KWP2000_H

#include "../common/syn_defs.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef SYN_KWP2000_MAX_LIDS
#define SYN_KWP2000_MAX_LIDS 16U 
#endif
#ifndef SYN_KWP2000_MAX_CIDS
#define SYN_KWP2000_MAX_CIDS 16U 
#endif
#define SYN_KWP2000_SID_START_DIAGNOSTIC_SESSION 0x10U     
#define SYN_KWP2000_SID_ECU_RESET 0x11U                    
#define SYN_KWP2000_SID_CLEAR_DIAGNOSTIC_INFORMATION 0x14U 
#define SYN_KWP2000_SID_READ_DTC_BY_STATUS 0x18U      
#define SYN_KWP2000_SID_READ_ECU_IDENTIFICATION 0x1AU 
#define SYN_KWP2000_SID_READ_DATA_BY_LOCAL_IDENTIFIER 0x21U   
#define SYN_KWP2000_SID_READ_DATA_BY_COMMON_IDENTIFIER 0x22U  
#define SYN_KWP2000_SID_SECURITY_ACCESS 0x27U                 
#define SYN_KWP2000_SID_WRITE_DATA_BY_COMMON_IDENTIFIER 0x2EU 
#define SYN_KWP2000_SID_START_ROUTINE_BY_LOCAL_IDENTIFIER \
    0x31U                                           
#define SYN_KWP2000_SID_REQUEST_DOWNLOAD 0x34U      
#define SYN_KWP2000_SID_TRANSFER_DATA 0x36U         
#define SYN_KWP2000_SID_REQUEST_TRANSFER_EXIT 0x37U 
#define SYN_KWP2000_SID_TESTER_PRESENT 0x3EU        
#define SYN_KWP2000_SID_NEGATIVE_RESPONSE 0x7FU     
#define SYN_KWP2000_SESSION_DEFAULT 0x81U     
#define SYN_KWP2000_SESSION_PROGRAMMING 0x85U 
#define SYN_KWP2000_SESSION_EXTENDED 0x86U    
#ifndef SYN_KWP2000_S3_TIMEOUT_MS
#define SYN_KWP2000_S3_TIMEOUT_MS 5000U 
#endif
#define SYN_KWP2000_NRC_SERVICE_NOT_SUPPORTED 0x11U     
#define SYN_KWP2000_NRC_SUBFUNCTION_NOT_SUPPORTED 0x12U 
#define SYN_KWP2000_NRC_INCORRECT_MESSAGE_LENGTH \
    0x13U 
#define SYN_KWP2000_NRC_CONDITIONS_NOT_CORRECT 0x22U      
#define SYN_KWP2000_NRC_REQUEST_OUT_OF_RANGE 0x31U        
#define SYN_KWP2000_NRC_SECURITY_ACCESS_DENIED 0x33U      
#define SYN_KWP2000_NRC_INVALID_KEY 0x35U                 
#define SYN_KWP2000_NRC_EXCEEDED_NUMBER_OF_ATTEMPTS 0x36U 
#define SYN_KWP2000_NRC_REQUIRED_TIME_DELAY_NOT_EXPIRED \
    0x37U 
#define SYN_KWP2000_NRC_RESPONSE_PENDING                    \
    0x78U 
typedef uint8_t SYN_KWP2000_Session;

typedef struct {
    uint8_t lid;      
    const void *data; 
    uint16_t len;     
    bool writable;    
} SYN_KWP2000_LID;

typedef struct {
    uint16_t cid;     
    const void *data; 
    uint16_t len;     
    bool writable;    
} SYN_KWP2000_CID;

typedef bool (*SYN_KWP2000_SessionHandler)(SYN_KWP2000_Session from_session,
                                           SYN_KWP2000_Session to_session, void *user_ctx);

typedef void (*SYN_KWP2000_ResetHandler)(uint8_t reset_type, void *user_ctx);

typedef bool (*SYN_KWP2000_SeedHandler)(uint8_t level, uint8_t *seed_buf, uint16_t *seed_len,
                                        void *user_ctx);

typedef bool (*SYN_KWP2000_KeyHandler)(uint8_t level, const uint8_t *key, uint16_t key_len,
                                       void *user_ctx);

typedef bool (*SYN_KWP2000_RoutineHandler)(uint8_t routine_id, const uint8_t *in_data,
                                           uint16_t in_len, uint8_t *out_buf, uint16_t max_out_len,
                                           uint16_t *out_len, void *user_ctx);

typedef bool (*SYN_KWP2000_MemoryHandler)(bool is_write, uint32_t address, uint32_t size,
                                          uint8_t *data_buf, void *user_ctx);

typedef struct {
    SYN_KWP2000_Session current_session; 
    bool security_unlocked;              
    uint8_t security_level;              
    uint32_t s3_timer_ms;                
    SYN_KWP2000_LID lids[SYN_KWP2000_MAX_LIDS]; 
    uint16_t lid_count;                         
    SYN_KWP2000_CID cids[SYN_KWP2000_MAX_CIDS]; 
    uint16_t cid_count;                         
    SYN_KWP2000_SessionHandler session_cb; 
    void *session_ctx;                     
    SYN_KWP2000_ResetHandler reset_cb; 
    void *reset_ctx;                   
    uint8_t pending_reset_type;        
    uint32_t reset_wait_ms;            
    SYN_KWP2000_SeedHandler seed_cb; 
    SYN_KWP2000_KeyHandler key_cb;   
    void *security_ctx;              
    SYN_KWP2000_RoutineHandler routine_cb; 
    void *routine_ctx;                     
    SYN_KWP2000_MemoryHandler memory_cb; 
    void *memory_ctx;                    
    bool is_downloading;        
    uint32_t download_addr;     
    uint32_t download_size;     
    uint32_t download_received; 
} SYN_KWP2000_Server;

/* ── API Declarations ────────────────────────────────────────────────── */

SYN_Status syn_kwp2000_init(SYN_KWP2000_Server *server);

SYN_Status syn_kwp2000_register_lid(SYN_KWP2000_Server *server, uint8_t lid, const void *data,
                                    uint16_t len, bool writable);

SYN_Status syn_kwp2000_register_cid(SYN_KWP2000_Server *server, uint16_t cid, const void *data,
                                    uint16_t len, bool writable);

SYN_Status syn_kwp2000_set_session_handler(SYN_KWP2000_Server *server,
                                           SYN_KWP2000_SessionHandler cb, void *user_ctx);

SYN_Status syn_kwp2000_set_reset_handler(SYN_KWP2000_Server *server, SYN_KWP2000_ResetHandler cb,
                                         void *user_ctx);

SYN_Status syn_kwp2000_set_security_handler(SYN_KWP2000_Server *server,
                                            SYN_KWP2000_SeedHandler seed_cb,
                                            SYN_KWP2000_KeyHandler key_cb, void *user_ctx);

SYN_Status syn_kwp2000_set_routine_handler(SYN_KWP2000_Server *server,
                                           SYN_KWP2000_RoutineHandler cb, void *user_ctx);

SYN_Status syn_kwp2000_set_memory_handler(SYN_KWP2000_Server *server, SYN_KWP2000_MemoryHandler cb,
                                          void *user_ctx);

SYN_Status syn_kwp2000_process_request(SYN_KWP2000_Server *server, const uint8_t *req_buf,
                                       uint16_t req_len, uint8_t *resp_buf, uint16_t max_resp_len,
                                       uint16_t *resp_len);

void syn_kwp2000_tick(SYN_KWP2000_Server *server, uint32_t dt_ms);

#ifdef __cplusplus
}
#endif

#endif /* SYN_KWP2000_H */
```


