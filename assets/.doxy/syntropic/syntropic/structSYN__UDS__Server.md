

# Struct SYN\_UDS\_Server



[**ClassList**](annotated.md) **>** [**SYN\_UDS\_Server**](structSYN__UDS__Server.md)



_UDS Server Instance Context._ 

* `#include <syn_uds.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint16\_t | [**active\_p2\_max\_ms**](#variable-active_p2_max_ms)  <br> |
|  uint16\_t | [**active\_p2\_star\_max\_10ms**](#variable-active_p2_star_max_10ms)  <br> |
|  uint32\_t | [**active\_seed**](#variable-active_seed)  <br> |
|  uint8\_t | [**active\_seed\_bytes**](#variable-active_seed_bytes)  <br> |
|  uint8\_t | [**active\_seed\_subfunction**](#variable-active_seed_subfunction)  <br> |
|  uint8\_t | [**aes\_security\_key**](#variable-aes_security_key)  <br> |
|  [**SYN\_UDS\_AuthHandler**](syn__uds_8h.md#typedef-syn_uds_authhandler) | [**auth\_cb**](#variable-auth_cb)  <br> |
|  void \* | [**auth\_ctx**](#variable-auth_ctx)  <br> |
|  [**SYN\_UDS\_CommControlHandler**](syn__uds_8h.md#typedef-syn_uds_commcontrolhandler) | [**comm\_control\_cb**](#variable-comm_control_cb)  <br> |
|  void \* | [**comm\_control\_ctx**](#variable-comm_control_ctx)  <br> |
|  [**SYN\_UDS\_CommControlType**](syn__uds_8h.md#enum-syn_uds_commcontroltype) | [**comm\_control\_state**](#variable-comm_control_state)  <br> |
|  uint8\_t | [**comm\_type**](#variable-comm_type)  <br> |
|  uint32\_t | [**current\_seed**](#variable-current_seed)  <br> |
|  uint8\_t | [**current\_seed\_bytes**](#variable-current_seed_bytes)  <br> |
|  uint8\_t | [**custom\_security\_count**](#variable-custom_security_count)  <br> |
|  uint16\_t | [**custom\_security\_masks**](#variable-custom_security_masks)  <br> |
|  uint8\_t | [**custom\_security\_sids**](#variable-custom_security_sids)  <br> |
|  uint8\_t | [**custom\_session\_count**](#variable-custom_session_count)  <br> |
|  uint8\_t | [**custom\_session\_masks**](#variable-custom_session_masks)  <br> |
|  uint8\_t | [**custom\_session\_sids**](#variable-custom_session_sids)  <br> |
|  uint8\_t | [**did\_count**](#variable-did_count)  <br> |
|  [**SYN\_UDS\_DIDEntry**](structSYN__UDS__DIDEntry.md) | [**did\_table**](#variable-did_table)  <br> |
|  [**SYN\_UDS\_DTCHandler**](syn__uds_8h.md#typedef-syn_uds_dtchandler) | [**dtc\_cb**](#variable-dtc_cb)  <br> |
|  uint8\_t | [**dtc\_count**](#variable-dtc_count)  <br> |
|  void \* | [**dtc\_ctx**](#variable-dtc_ctx)  <br> |
|  [**SYN\_UDS\_DTCEntry**](structSYN__UDS__DTCEntry.md) | [**dtc\_table**](#variable-dtc_table)  <br> |
|  [**SYN\_UDS\_DynamicDIDHandler**](syn__uds_8h.md#typedef-syn_uds_dynamicdidhandler) | [**dynamic\_did\_cb**](#variable-dynamic_did_cb)  <br> |
|  void \* | [**dynamic\_did\_ctx**](#variable-dynamic_did_ctx)  <br> |
|  uint8\_t | [**expected\_block\_seq**](#variable-expected_block_seq)  <br> |
|  [**SYN\_UDS\_FileTransferHandler**](syn__uds_8h.md#typedef-syn_uds_filetransferhandler) | [**file\_transfer\_cb**](#variable-file_transfer_cb)  <br> |
|  void \* | [**file\_transfer\_ctx**](#variable-file_transfer_ctx)  <br> |
|  [**SYN\_UDS\_IOControlHandler**](syn__uds_8h.md#typedef-syn_uds_iocontrolhandler) | [**io\_control\_cb**](#variable-io_control_cb)  <br> |
|  void \* | [**io\_control\_ctx**](#variable-io_control_ctx)  <br> |
|  [**SYN\_UDS\_LinkControlHandler**](syn__uds_8h.md#typedef-syn_uds_linkcontrolhandler) | [**link\_control\_cb**](#variable-link_control_cb)  <br> |
|  void \* | [**link\_control\_ctx**](#variable-link_control_ctx)  <br> |
|  [**SYN\_UDS\_MemoryHandler**](syn__uds_8h.md#typedef-syn_uds_memoryhandler) | [**memory\_cb**](#variable-memory_cb)  <br> |
|  void \* | [**memory\_ctx**](#variable-memory_ctx)  <br> |
|  uint16\_t | [**p2\_max\_ms**](#variable-p2_max_ms)  <br> |
|  uint16\_t | [**p2\_star\_max\_10ms**](#variable-p2_star_max_10ms)  <br> |
|  [**SYN\_UDS\_PeriodicDataHandler**](syn__uds_8h.md#typedef-syn_uds_periodicdatahandler) | [**periodic\_cb**](#variable-periodic_cb)  <br> |
|  void \* | [**periodic\_ctx**](#variable-periodic_ctx)  <br> |
|  [**SYN\_UDS\_ResetHandler**](syn__uds_8h.md#typedef-syn_uds_resethandler) | [**reset\_cb**](#variable-reset_cb)  <br> |
|  void \* | [**reset\_ctx**](#variable-reset_ctx)  <br> |
|  uint16\_t | [**reset\_tx\_wait\_ms**](#variable-reset_tx_wait_ms)  <br> |
|  uint8\_t | [**reset\_type\_requested**](#variable-reset_type_requested)  <br> |
|  uint32\_t | [**reset\_wait\_elapsed\_ms**](#variable-reset_wait_elapsed_ms)  <br> |
|  [**SYN\_UDS\_ResponseOnEventHandler**](syn__uds_8h.md#typedef-syn_uds_responseoneventhandler) | [**roe\_cb**](#variable-roe_cb)  <br> |
|  void \* | [**roe\_ctx**](#variable-roe_ctx)  <br> |
|  [**SYN\_UDS\_RoutineControlHandler**](syn__uds_8h.md#typedef-syn_uds_routinecontrolhandler) | [**routine\_cb**](#variable-routine_cb)  <br> |
|  void \* | [**routine\_ctx**](#variable-routine_ctx)  <br> |
|  uint32\_t | [**s3\_timer\_ms**](#variable-s3_timer_ms)  <br> |
|  [**SYN\_UDS\_ScalingDataHandler**](syn__uds_8h.md#typedef-syn_uds_scalingdatahandler) | [**scaling\_cb**](#variable-scaling_cb)  <br> |
|  void \* | [**scaling\_ctx**](#variable-scaling_ctx)  <br> |
|  [**SYN\_UDS\_SecuredDataHandler**](syn__uds_8h.md#typedef-syn_uds_secureddatahandler) | [**secured\_data\_cb**](#variable-secured_data_cb)  <br> |
|  void \* | [**secured\_data\_ctx**](#variable-secured_data_ctx)  <br> |
|  uint32\_t | [**security\_delay\_timer\_ms**](#variable-security_delay_timer_ms)  <br> |
|  uint8\_t | [**security\_error\_count**](#variable-security_error_count)  <br> |
|  uint8\_t | [**security\_level**](#variable-security_level)  <br> |
|  [**SYN\_UDS\_SecurityState**](syn__uds_8h.md#enum-syn_uds_securitystate) | [**security\_state**](#variable-security_state)  <br> |
|  [**SYN\_UDS\_Session**](syn__uds_8h.md#enum-syn_uds_session) | [**session**](#variable-session)  <br> |
|  [**SYN\_UDS\_SessionTransitionHandler**](syn__uds_8h.md#typedef-syn_uds_sessiontransitionhandler) | [**session\_transition\_cb**](#variable-session_transition_cb)  <br> |
|  void \* | [**session\_transition\_ctx**](#variable-session_transition_ctx)  <br> |
|  [**SYN\_UDS\_AccessTimingHandler**](syn__uds_8h.md#typedef-syn_uds_accesstiminghandler) | [**timing\_cb**](#variable-timing_cb)  <br> |
|  void \* | [**timing\_ctx**](#variable-timing_ctx)  <br> |
|  uint32\_t | [**transfer\_address**](#variable-transfer_address)  <br> |
|  uint32\_t | [**transfer\_bytes\_processed**](#variable-transfer_bytes_processed)  <br> |
|  uint32\_t | [**transfer\_size**](#variable-transfer_size)  <br> |
|  [**SYN\_UDS\_TransferState**](syn__uds_8h.md#enum-syn_uds_transferstate) | [**transfer\_state**](#variable-transfer_state)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**use\_aes128\_security**](#variable-use_aes128_security)  <br> |












































## Public Attributes Documentation




### variable active\_p2\_max\_ms 

```C++
uint16_t SYN_UDS_Server::active_p2_max_ms;
```



Active P2Server\_max timing (ms). 


        

<hr>



### variable active\_p2\_star\_max\_10ms 

```C++
uint16_t SYN_UDS_Server::active_p2_star_max_10ms;
```



Active P2\*Server\_max timing (10ms units). 


        

<hr>



### variable active\_seed 

```C++
uint32_t SYN_UDS_Server::active_seed;
```



Issued transaction security seed value 


        

<hr>



### variable active\_seed\_bytes 

```C++
uint8_t SYN_UDS_Server::active_seed_bytes[16];
```



Issued transaction AES-128 security seed (16 bytes) 


        

<hr>



### variable active\_seed\_subfunction 

```C++
uint8_t SYN_UDS_Server::active_seed_subfunction;
```



Active seed request subfunction (e.g. 0x01, 0x03) 


        

<hr>



### variable aes\_security\_key 

```C++
uint8_t SYN_UDS_Server::aes_security_key[16];
```



AES-128 security secret key (16 bytes) 


        

<hr>



### variable auth\_cb 

```C++
SYN_UDS_AuthHandler SYN_UDS_Server::auth_cb;
```



Authentication (0x29) callback. 


        

<hr>



### variable auth\_ctx 

```C++
void* SYN_UDS_Server::auth_ctx;
```



Context pointer for auth callback. 


        

<hr>



### variable comm\_control\_cb 

```C++
SYN_UDS_CommControlHandler SYN_UDS_Server::comm_control_cb;
```



CommunicationControl callback 


        

<hr>



### variable comm\_control\_ctx 

```C++
void* SYN_UDS_Server::comm_control_ctx;
```



CommunicationControl context pointer 


        

<hr>



### variable comm\_control\_state 

```C++
SYN_UDS_CommControlType SYN_UDS_Server::comm_control_state;
```



CommunicationControl state 


        

<hr>



### variable comm\_type 

```C++
uint8_t SYN_UDS_Server::comm_type;
```



Communication type byte 


        

<hr>



### variable current\_seed 

```C++
uint32_t SYN_UDS_Server::current_seed;
```



Master template security seed value 


        

<hr>



### variable current\_seed\_bytes 

```C++
uint8_t SYN_UDS_Server::current_seed_bytes[16];
```



Master template AES-128 security seed (16 bytes) 


        

<hr>



### variable custom\_security\_count 

```C++
uint8_t SYN_UDS_Server::custom_security_count;
```



Custom security override count 


        

<hr>



### variable custom\_security\_masks 

```C++
uint16_t SYN_UDS_Server::custom_security_masks[SYN_UDS_MAX_SERVICE_OVERRIDES];
```



Custom security mask values 


        

<hr>



### variable custom\_security\_sids 

```C++
uint8_t SYN_UDS_Server::custom_security_sids[SYN_UDS_MAX_SERVICE_OVERRIDES];
```



Custom security mask SID overrides 


        

<hr>



### variable custom\_session\_count 

```C++
uint8_t SYN_UDS_Server::custom_session_count;
```



Custom session override count 


        

<hr>



### variable custom\_session\_masks 

```C++
uint8_t SYN_UDS_Server::custom_session_masks[SYN_UDS_MAX_SERVICE_OVERRIDES];
```



Custom session mask values 


        

<hr>



### variable custom\_session\_sids 

```C++
uint8_t SYN_UDS_Server::custom_session_sids[SYN_UDS_MAX_SERVICE_OVERRIDES];
```



Custom session mask SID overrides 


        

<hr>



### variable did\_count 

```C++
uint8_t SYN_UDS_Server::did_count;
```



Registered DID count 


        

<hr>



### variable did\_table 

```C++
SYN_UDS_DIDEntry SYN_UDS_Server::did_table[SYN_UDS_MAX_DIDS];
```



Registered DID entries array 


        

<hr>



### variable dtc\_cb 

```C++
SYN_UDS_DTCHandler SYN_UDS_Server::dtc_cb;
```



ReadDTCInformation (0x19) callback. 


        

<hr>



### variable dtc\_count 

```C++
uint8_t SYN_UDS_Server::dtc_count;
```



Registered DTC count 


        

<hr>



### variable dtc\_ctx 

```C++
void* SYN_UDS_Server::dtc_ctx;
```



Context pointer for DTC callback. 


        

<hr>



### variable dtc\_table 

```C++
SYN_UDS_DTCEntry SYN_UDS_Server::dtc_table[SYN_UDS_MAX_DTCS];
```



Registered DTC entries array 


        

<hr>



### variable dynamic\_did\_cb 

```C++
SYN_UDS_DynamicDIDHandler SYN_UDS_Server::dynamic_did_cb;
```



DynamicDID (0x2C) callback. 


        

<hr>



### variable dynamic\_did\_ctx 

```C++
void* SYN_UDS_Server::dynamic_did_ctx;
```



Context pointer for dynamic DID callback. 


        

<hr>



### variable expected\_block\_seq 

```C++
uint8_t SYN_UDS_Server::expected_block_seq;
```



Expected block sequence counter 


        

<hr>



### variable file\_transfer\_cb 

```C++
SYN_UDS_FileTransferHandler SYN_UDS_Server::file_transfer_cb;
```



File transfer (0x38) callback. 


        

<hr>



### variable file\_transfer\_ctx 

```C++
void* SYN_UDS_Server::file_transfer_ctx;
```



Context pointer for file transfer callback. 


        

<hr>



### variable io\_control\_cb 

```C++
SYN_UDS_IOControlHandler SYN_UDS_Server::io_control_cb;
```



IOControl (0x2F) callback. 


        

<hr>



### variable io\_control\_ctx 

```C++
void* SYN_UDS_Server::io_control_ctx;
```



Context pointer for IO control callback. 


        

<hr>



### variable link\_control\_cb 

```C++
SYN_UDS_LinkControlHandler SYN_UDS_Server::link_control_cb;
```



LinkControl (0x87) callback. 


        

<hr>



### variable link\_control\_ctx 

```C++
void* SYN_UDS_Server::link_control_ctx;
```



Context pointer for link control callback. 


        

<hr>



### variable memory\_cb 

```C++
SYN_UDS_MemoryHandler SYN_UDS_Server::memory_cb;
```



Memory (0x23/0x3D) callback. 


        

<hr>



### variable memory\_ctx 

```C++
void* SYN_UDS_Server::memory_ctx;
```



Context pointer for memory callback. 


        

<hr>



### variable p2\_max\_ms 

```C++
uint16_t SYN_UDS_Server::p2_max_ms;
```



Default P2Server\_max timing (ms). 


        

<hr>



### variable p2\_star\_max\_10ms 

```C++
uint16_t SYN_UDS_Server::p2_star_max_10ms;
```



Default P2\*Server\_max timing (10ms units). 


        

<hr>



### variable periodic\_cb 

```C++
SYN_UDS_PeriodicDataHandler SYN_UDS_Server::periodic_cb;
```



PeriodicData (0x2A) callback. 


        

<hr>



### variable periodic\_ctx 

```C++
void* SYN_UDS_Server::periodic_ctx;
```



Context pointer for periodic callback. 


        

<hr>



### variable reset\_cb 

```C++
SYN_UDS_ResetHandler SYN_UDS_Server::reset_cb;
```



ECUReset (0x11) deferred callback. 


        

<hr>



### variable reset\_ctx 

```C++
void* SYN_UDS_Server::reset_ctx;
```



Context pointer for reset callback. 


        

<hr>



### variable reset\_tx\_wait\_ms 

```C++
uint16_t SYN_UDS_Server::reset_tx_wait_ms;
```



Post-TX ECU reset delay window in ms. 


        

<hr>



### variable reset\_type\_requested 

```C++
uint8_t SYN_UDS_Server::reset_type_requested;
```



Pending ECU reset type requested 


        

<hr>



### variable reset\_wait\_elapsed\_ms 

```C++
uint32_t SYN_UDS_Server::reset_wait_elapsed_ms;
```



Internal reset delay accumulator in ms. 


        

<hr>



### variable roe\_cb 

```C++
SYN_UDS_ResponseOnEventHandler SYN_UDS_Server::roe_cb;
```



ResponseOnEvent (0x86) callback. 


        

<hr>



### variable roe\_ctx 

```C++
void* SYN_UDS_Server::roe_ctx;
```



Context pointer for ROE callback. 


        

<hr>



### variable routine\_cb 

```C++
SYN_UDS_RoutineControlHandler SYN_UDS_Server::routine_cb;
```



RoutineControl (0x31) callback. 


        

<hr>



### variable routine\_ctx 

```C++
void* SYN_UDS_Server::routine_ctx;
```



Context pointer for routine callback. 


        

<hr>



### variable s3\_timer\_ms 

```C++
uint32_t SYN_UDS_Server::s3_timer_ms;
```



S3 session timer in ms 


        

<hr>



### variable scaling\_cb 

```C++
SYN_UDS_ScalingDataHandler SYN_UDS_Server::scaling_cb;
```



ScalingData (0x24) callback. 


        

<hr>



### variable scaling\_ctx 

```C++
void* SYN_UDS_Server::scaling_ctx;
```



Context pointer for scaling callback. 


        

<hr>



### variable secured\_data\_cb 

```C++
SYN_UDS_SecuredDataHandler SYN_UDS_Server::secured_data_cb;
```



SecuredDataTransmission callback. 


        

<hr>



### variable secured\_data\_ctx 

```C++
void* SYN_UDS_Server::secured_data_ctx;
```



Context pointer for secured data callback. 


        

<hr>



### variable security\_delay\_timer\_ms 

```C++
uint32_t SYN_UDS_Server::security_delay_timer_ms;
```



Security delay penalty timer in ms 


        

<hr>



### variable security\_error\_count 

```C++
uint8_t SYN_UDS_Server::security_error_count;
```



Failed security unlock attempts counter 


        

<hr>



### variable security\_level 

```C++
uint8_t SYN_UDS_Server::security_level;
```



Unlocked security level (0 = locked, 1 = level 1, 2 = level 2...) 


        

<hr>



### variable security\_state 

```C++
SYN_UDS_SecurityState SYN_UDS_Server::security_state;
```



Security access unlock state 


        

<hr>



### variable session 

```C++
SYN_UDS_Session SYN_UDS_Server::session;
```



Current diagnostic session state 


        

<hr>



### variable session\_transition\_cb 

```C++
SYN_UDS_SessionTransitionHandler SYN_UDS_Server::session_transition_cb;
```



Session transition policy callback. 


        

<hr>



### variable session\_transition\_ctx 

```C++
void* SYN_UDS_Server::session_transition_ctx;
```



Session transition context pointer. 


        

<hr>



### variable timing\_cb 

```C++
SYN_UDS_AccessTimingHandler SYN_UDS_Server::timing_cb;
```



AccessTimingParameter callback. 


        

<hr>



### variable timing\_ctx 

```C++
void* SYN_UDS_Server::timing_ctx;
```



Context pointer for timing callback. 


        

<hr>



### variable transfer\_address 

```C++
uint32_t SYN_UDS_Server::transfer_address;
```



Target transfer memory address 


        

<hr>



### variable transfer\_bytes\_processed 

```C++
uint32_t SYN_UDS_Server::transfer_bytes_processed;
```



Transferred byte count 


        

<hr>



### variable transfer\_size 

```C++
uint32_t SYN_UDS_Server::transfer_size;
```



Total transfer size in bytes 


        

<hr>



### variable transfer\_state 

```C++
SYN_UDS_TransferState SYN_UDS_Server::transfer_state;
```



Active data transfer state 


        

<hr>



### variable use\_aes128\_security 

```C++
bool SYN_UDS_Server::use_aes128_security;
```



True if AES-128 security mode is active 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_uds.h`

