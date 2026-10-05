

# File syn\_uds.h



[**FileList**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_uds.h**](syn__uds_8h.md)

[Go to the source code of this file](syn__uds_8h_source.md)

_ISO 14229 Unified Diagnostic Services (UDS) Server Implementation._ [More...](#detailed-description)

* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_UDS\_DIDEntry**](structSYN__UDS__DIDEntry.md) <br>_Data Identifier (DID) Registry Entry._  |
| struct | [**SYN\_UDS\_DTCEntry**](structSYN__UDS__DTCEntry.md) <br>_Diagnostic Trouble Code (DTC) Registry Entry._  |
| struct | [**SYN\_UDS\_Server**](structSYN__UDS__Server.md) <br>_UDS Server Instance Context._  |


## Public Types

| Type | Name |
| ---: | :--- |
| typedef [**bool**](syn__defs_8h.md#enum-bool)(\* | [**SYN\_UDS\_AccessTimingHandler**](#typedef-syn_uds_accesstiminghandler)  <br>_AccessTimingParameter (0x83) callback function signature._  |
| enum  | [**SYN\_UDS\_AccessTimingType**](#enum-syn_uds_accesstimingtype)  <br>_UDS AccessTimingParameter (0x83) Subfunctions._  |
| enum  | [**SYN\_UDS\_AddrMode**](#enum-syn_uds_addrmode)  <br>_UDS Addressing Modes (ISO 14229-1)_  |
| typedef [**bool**](syn__defs_8h.md#enum-bool)(\* | [**SYN\_UDS\_AuthHandler**](#typedef-syn_uds_authhandler)  <br>_Authentication (0x29) callback function signature._  |
| typedef [**bool**](syn__defs_8h.md#enum-bool)(\* | [**SYN\_UDS\_CommControlHandler**](#typedef-syn_uds_commcontrolhandler)  <br>_CommunicationControl (0x28) callback function signature._  |
| enum  | [**SYN\_UDS\_CommControlType**](#enum-syn_uds_commcontroltype)  <br>_UDS CommunicationControl (0x28) Subfunctions._  |
| typedef [**bool**](syn__defs_8h.md#enum-bool)(\* | [**SYN\_UDS\_DTCHandler**](#typedef-syn_uds_dtchandler)  <br>_ReadDTCInformation (0x19) callback function signature._  |
| typedef [**bool**](syn__defs_8h.md#enum-bool)(\* | [**SYN\_UDS\_DynamicDIDHandler**](#typedef-syn_uds_dynamicdidhandler)  <br>_DynamicallyDefineDataIdentifier (0x2C) callback function signature._  |
| typedef [**bool**](syn__defs_8h.md#enum-bool)(\* | [**SYN\_UDS\_FileTransferHandler**](#typedef-syn_uds_filetransferhandler)  <br>_RequestFileTransfer (0x38) callback function signature._  |
| typedef [**bool**](syn__defs_8h.md#enum-bool)(\* | [**SYN\_UDS\_IOControlHandler**](#typedef-syn_uds_iocontrolhandler)  <br>_InputOutputControlByIdentifier (0x2F) callback function signature._  |
| typedef [**bool**](syn__defs_8h.md#enum-bool)(\* | [**SYN\_UDS\_LinkControlHandler**](#typedef-syn_uds_linkcontrolhandler)  <br>_LinkControl (0x87) callback function signature._  |
| typedef [**bool**](syn__defs_8h.md#enum-bool)(\* | [**SYN\_UDS\_MemoryHandler**](#typedef-syn_uds_memoryhandler)  <br>_ReadMemoryByAddress (0x23) and WriteMemoryByAddress (0x3D) callback function signature._  |
| typedef [**bool**](syn__defs_8h.md#enum-bool)(\* | [**SYN\_UDS\_PeriodicDataHandler**](#typedef-syn_uds_periodicdatahandler)  <br>_ReadDataByPeriodicIdentifier (0x2A) callback function signature._  |
| typedef void(\* | [**SYN\_UDS\_ResetHandler**](#typedef-syn_uds_resethandler)  <br>_ECUReset (0x11) deferred post-TX reset callback function signature._  |
| typedef [**bool**](syn__defs_8h.md#enum-bool)(\* | [**SYN\_UDS\_ResponseOnEventHandler**](#typedef-syn_uds_responseoneventhandler)  <br>_ResponseOnEvent (0x86) callback function signature._  |
| typedef [**bool**](syn__defs_8h.md#enum-bool)(\* | [**SYN\_UDS\_RoutineControlHandler**](#typedef-syn_uds_routinecontrolhandler)  <br>_RoutineControl (0x31) callback function signature._  |
| typedef [**bool**](syn__defs_8h.md#enum-bool)(\* | [**SYN\_UDS\_ScalingDataHandler**](#typedef-syn_uds_scalingdatahandler)  <br>_ReadScalingDataByIdentifier (0x24) callback function signature._  |
| typedef [**bool**](syn__defs_8h.md#enum-bool)(\* | [**SYN\_UDS\_SecuredDataHandler**](#typedef-syn_uds_secureddatahandler)  <br>_SecuredDataTransmission (0x84) callback function signature._  |
| enum  | [**SYN\_UDS\_SecurityState**](#enum-syn_uds_securitystate)  <br>_UDS Security Access Unlock States._  |
| enum  | [**SYN\_UDS\_Session**](#enum-syn_uds_session)  <br>_UDS Diagnostic Session Types (ISO 14229-1)_  |
| typedef [**bool**](syn__defs_8h.md#enum-bool)(\* | [**SYN\_UDS\_SessionTransitionHandler**](#typedef-syn_uds_sessiontransitionhandler)  <br>_DiagnosticSessionControl (0x10) session transition rule callback signature._  |
| enum  | [**SYN\_UDS\_TransferState**](#enum-syn_uds_transferstate)  <br>_UDS Data Transfer States._  |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_uds\_clear\_pending\_reset**](#function-syn_uds_clear_pending_reset) ([**SYN\_UDS\_Server**](structSYN__UDS__Server.md) \* server) <br>_Clear pending ECU reset flag after executing reset or transmitting response._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_uds\_disable\_aes128\_security**](#function-syn_uds_disable_aes128_security) ([**SYN\_UDS\_Server**](structSYN__UDS__Server.md) \* server) <br>_Disable AES-128 security mode and revert to standard XOR security key calculation._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_uds\_dtc\_get\_status**](#function-syn_uds_dtc_get_status) ([**SYN\_UDS\_Server**](structSYN__UDS__Server.md) \* server, uint32\_t dtc, uint8\_t \* out\_status) <br>_Get current 8-bit status byte for a registered DTC._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_uds\_dtc\_report\_test\_result**](#function-syn_uds_dtc_report_test_result) ([**SYN\_UDS\_Server**](structSYN__UDS__Server.md) \* server, uint32\_t dtc, [**bool**](syn__defs_8h.md#enum-bool) failed) <br>_Report diagnostic test result for a registered DTC according to ISO 14229-1 state transitions._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_uds\_dtc\_start\_operation\_cycle**](#function-syn_uds_dtc_start_operation_cycle) ([**SYN\_UDS\_Server**](structSYN__UDS__Server.md) \* server) <br>_Advance server state to a new operation cycle. Clears testFailedThisOperationCycle and sets testNotCompletedThisOperationCycle for all registered DTCs._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_uds\_enable\_aes128\_security**](#function-syn_uds_enable_aes128_security) ([**SYN\_UDS\_Server**](structSYN__UDS__Server.md) \* server, const uint8\_t key) <br>_Enable AES-128 algorithm for UDS SecurityAccess (0x27) seed/key unlock._  |
|  uint8\_t | [**syn\_uds\_get\_pending\_reset**](#function-syn_uds_get_pending_reset) (const [**SYN\_UDS\_Server**](structSYN__UDS__Server.md) \* server) <br>_Get pending ECU reset sub-function requested by 0x11 service._  |
|  uint8\_t | [**syn\_uds\_get\_security\_level**](#function-syn_uds_get_security_level) (const [**SYN\_UDS\_Server**](structSYN__UDS__Server.md) \* server) <br>_Get currently unlocked security level (0 = locked, 1 = level 1, 2 = level 2...)._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_uds\_init**](#function-syn_uds_init) ([**SYN\_UDS\_Server**](structSYN__UDS__Server.md) \* server) <br>_Initialize UDS Server context._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_uds\_is\_sid\_functional\_supported**](#function-syn_uds_is_sid_functional_supported) (uint8\_t sid) <br>_Check if Service Identifier (SID) supports Functional Addressing (1:N Broadcast)._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_uds\_process\_request**](#function-syn_uds_process_request) ([**SYN\_UDS\_Server**](structSYN__UDS__Server.md) \* server, const uint8\_t \* req, uint16\_t req\_len, uint8\_t \* resp\_buf, uint16\_t max\_resp\_len, uint16\_t \* resp\_len, [**SYN\_UDS\_AddrMode**](syn__uds_8h.md#enum-syn_uds_addrmode) addr\_mode) <br>_Process incoming UDS request diagnostic payload and format response._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_uds\_register\_access\_timing**](#function-syn_uds_register_access_timing) ([**SYN\_UDS\_Server**](structSYN__UDS__Server.md) \* server, [**SYN\_UDS\_AccessTimingHandler**](syn__uds_8h.md#typedef-syn_uds_accesstiminghandler) handler, void \* ctx) <br>_Register AccessTimingParameter (0x83) callback handler._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_uds\_register\_auth\_handler**](#function-syn_uds_register_auth_handler) ([**SYN\_UDS\_Server**](structSYN__UDS__Server.md) \* server, [**SYN\_UDS\_AuthHandler**](syn__uds_8h.md#typedef-syn_uds_authhandler) handler, void \* ctx) <br>_Register Authentication (0x29) handler._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_uds\_register\_comm\_control**](#function-syn_uds_register_comm_control) ([**SYN\_UDS\_Server**](structSYN__UDS__Server.md) \* server, [**SYN\_UDS\_CommControlHandler**](syn__uds_8h.md#typedef-syn_uds_commcontrolhandler) handler, void \* ctx) <br>_Register CommunicationControl (0x28) callback handler._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_uds\_register\_did**](#function-syn_uds_register_did) ([**SYN\_UDS\_Server**](structSYN__UDS__Server.md) \* server, uint16\_t did, uint8\_t \* data, uint16\_t len, [**bool**](syn__defs_8h.md#enum-bool) writable) <br>_Register Data Identifier (DID) mapping in UDS Server table._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_uds\_register\_did\_ext**](#function-syn_uds_register_did_ext) ([**SYN\_UDS\_Server**](structSYN__UDS__Server.md) \* server, uint16\_t did, uint8\_t \* data, uint16\_t len, [**bool**](syn__defs_8h.md#enum-bool) writable, uint8\_t session\_mask, uint16\_t security\_mask) <br>_Register Data Identifier (DID) with custom session & security permission bitmask._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_uds\_register\_dtc**](#function-syn_uds_register_dtc) ([**SYN\_UDS\_Server**](structSYN__UDS__Server.md) \* server, uint32\_t dtc, uint8\_t status, uint8\_t severity) <br>_Register Diagnostic Trouble Code (DTC) in UDS Server table._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_uds\_register\_dtc\_handler**](#function-syn_uds_register_dtc_handler) ([**SYN\_UDS\_Server**](structSYN__UDS__Server.md) \* server, [**SYN\_UDS\_DTCHandler**](syn__uds_8h.md#typedef-syn_uds_dtchandler) handler, void \* ctx) <br>_Register ReadDTCInformation (0x19) callback handler._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_uds\_register\_dynamic\_did\_handler**](#function-syn_uds_register_dynamic_did_handler) ([**SYN\_UDS\_Server**](structSYN__UDS__Server.md) \* server, [**SYN\_UDS\_DynamicDIDHandler**](syn__uds_8h.md#typedef-syn_uds_dynamicdidhandler) handler, void \* ctx) <br>_Register DynamicallyDefineDataIdentifier (0x2C) callback handler._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_uds\_register\_file\_transfer**](#function-syn_uds_register_file_transfer) ([**SYN\_UDS\_Server**](structSYN__UDS__Server.md) \* server, [**SYN\_UDS\_FileTransferHandler**](syn__uds_8h.md#typedef-syn_uds_filetransferhandler) handler, void \* ctx) <br>_Register RequestFileTransfer (0x38) handler._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_uds\_register\_io\_control**](#function-syn_uds_register_io_control) ([**SYN\_UDS\_Server**](structSYN__UDS__Server.md) \* server, [**SYN\_UDS\_IOControlHandler**](syn__uds_8h.md#typedef-syn_uds_iocontrolhandler) handler, void \* ctx) <br>_Register InputOutputControlByIdentifier (0x2F) callback handler._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_uds\_register\_link\_control**](#function-syn_uds_register_link_control) ([**SYN\_UDS\_Server**](structSYN__UDS__Server.md) \* server, [**SYN\_UDS\_LinkControlHandler**](syn__uds_8h.md#typedef-syn_uds_linkcontrolhandler) handler, void \* ctx) <br>_Register LinkControl (0x87) callback handler._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_uds\_register\_memory\_handler**](#function-syn_uds_register_memory_handler) ([**SYN\_UDS\_Server**](structSYN__UDS__Server.md) \* server, [**SYN\_UDS\_MemoryHandler**](syn__uds_8h.md#typedef-syn_uds_memoryhandler) handler, void \* ctx) <br>_Register ReadMemoryByAddress (0x23) and WriteMemoryByAddress (0x3D) handler._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_uds\_register\_periodic\_data\_handler**](#function-syn_uds_register_periodic_data_handler) ([**SYN\_UDS\_Server**](structSYN__UDS__Server.md) \* server, [**SYN\_UDS\_PeriodicDataHandler**](syn__uds_8h.md#typedef-syn_uds_periodicdatahandler) handler, void \* ctx) <br>_Register ReadDataByPeriodicIdentifier (0x2A) callback handler._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_uds\_register\_roe\_handler**](#function-syn_uds_register_roe_handler) ([**SYN\_UDS\_Server**](structSYN__UDS__Server.md) \* server, [**SYN\_UDS\_ResponseOnEventHandler**](syn__uds_8h.md#typedef-syn_uds_responseoneventhandler) handler, void \* ctx) <br>_Register ResponseOnEvent (0x86) callback handler._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_uds\_register\_routine\_control**](#function-syn_uds_register_routine_control) ([**SYN\_UDS\_Server**](structSYN__UDS__Server.md) \* server, [**SYN\_UDS\_RoutineControlHandler**](syn__uds_8h.md#typedef-syn_uds_routinecontrolhandler) handler, void \* ctx) <br>_Register RoutineControl (0x31) callback handler._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_uds\_register\_scaling\_data\_handler**](#function-syn_uds_register_scaling_data_handler) ([**SYN\_UDS\_Server**](structSYN__UDS__Server.md) \* server, [**SYN\_UDS\_ScalingDataHandler**](syn__uds_8h.md#typedef-syn_uds_scalingdatahandler) handler, void \* ctx) <br>_Register ReadScalingDataByIdentifier (0x24) callback handler._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_uds\_register\_secured\_data**](#function-syn_uds_register_secured_data) ([**SYN\_UDS\_Server**](structSYN__UDS__Server.md) \* server, [**SYN\_UDS\_SecuredDataHandler**](syn__uds_8h.md#typedef-syn_uds_secureddatahandler) handler, void \* ctx) <br>_Register SecuredDataTransmission (0x84) callback handler._  |
|  void | [**syn\_uds\_set\_reset\_handler**](#function-syn_uds_set_reset_handler) ([**SYN\_UDS\_Server**](structSYN__UDS__Server.md) \* server, [**SYN\_UDS\_ResetHandler**](syn__uds_8h.md#typedef-syn_uds_resethandler) cb, void \* ctx) <br>_Register deferred post-TX ECU reset handler callback._  |
|  void | [**syn\_uds\_set\_reset\_wait\_ms**](#function-syn_uds_set_reset_wait_ms) ([**SYN\_UDS\_Server**](structSYN__UDS__Server.md) \* server, uint16\_t wait\_ms) <br>_Set post-TX ECU reset delay window duration in milliseconds._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_uds\_set\_security\_seed\_bytes**](#function-syn_uds_set_security_seed_bytes) ([**SYN\_UDS\_Server**](structSYN__UDS__Server.md) \* server, const uint8\_t seed) <br>_Set custom 16-byte seed for AES-128 SecurityAccess._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_uds\_set\_service\_security\_mask**](#function-syn_uds_set_service_security_mask) ([**SYN\_UDS\_Server**](structSYN__UDS__Server.md) \* server, uint8\_t sid, uint16\_t security\_mask) <br>_Configure required security level mask for a specific Service Identifier._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_uds\_set\_service\_session\_mask**](#function-syn_uds_set_service_session_mask) ([**SYN\_UDS\_Server**](structSYN__UDS__Server.md) \* server, uint8\_t sid, uint8\_t session\_mask) <br>_Configure allowed diagnostic session mask for a specific Service Identifier._  |
|  void | [**syn\_uds\_set\_session\_transition\_handler**](#function-syn_uds_set_session_transition_handler) ([**SYN\_UDS\_Server**](structSYN__UDS__Server.md) \* server, [**SYN\_UDS\_SessionTransitionHandler**](syn__uds_8h.md#typedef-syn_uds_sessiontransitionhandler) cb, void \* ctx) <br>_Register optional session transition policy callback._  |
|  void | [**syn\_uds\_tick**](#function-syn_uds_tick) ([**SYN\_UDS\_Server**](structSYN__UDS__Server.md) \* server, uint32\_t dt\_ms) <br>_Advance periodic S3 server timer by dt\_ms milliseconds. Reverts session to DEFAULT and locks security state if S3 timer expires (5000ms)._  |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_UDS\_DEFAULT\_RESET\_TX\_WAIT\_MS**](syn__uds_8h.md#define-syn_uds_default_reset_tx_wait_ms)  `50U`<br> |
| define  | [**SYN\_UDS\_DTC\_CLASS\_MASK**](syn__uds_8h.md#define-syn_uds_dtc_class_mask)  `(0x1FU)`<br> |
| define  | [**SYN\_UDS\_DTC\_FORMAT\_ISO14229\_1**](syn__uds_8h.md#define-syn_uds_dtc_format_iso14229_1)  `0x00U`<br> |
| define  | [**SYN\_UDS\_DTC\_FORMAT\_ISO15031\_6**](syn__uds_8h.md#define-syn_uds_dtc_format_iso15031_6)  `0x01U`<br> |
| define  | [**SYN\_UDS\_DTC\_FORMAT\_ISO27145\_4**](syn__uds_8h.md#define-syn_uds_dtc_format_iso27145_4)  `0x03U`<br> |
| define  | [**SYN\_UDS\_DTC\_FORMAT\_SAE\_J1939\_73**](syn__uds_8h.md#define-syn_uds_dtc_format_sae_j1939_73)  `0x02U`<br> |
| define  | [**SYN\_UDS\_DTC\_GROUP\_ALL**](syn__uds_8h.md#define-syn_uds_dtc_group_all)  `0xFFFFFFU`<br> |
| define  | [**SYN\_UDS\_DTC\_GROUP\_BODY**](syn__uds_8h.md#define-syn_uds_dtc_group_body)  `0x800000U`<br> |
| define  | [**SYN\_UDS\_DTC\_GROUP\_CHASSIS**](syn__uds_8h.md#define-syn_uds_dtc_group_chassis)  `0x400000U`<br> |
| define  | [**SYN\_UDS\_DTC\_GROUP\_EMISSIONS**](syn__uds_8h.md#define-syn_uds_dtc_group_emissions)  `0x000000U`<br> |
| define  | [**SYN\_UDS\_DTC\_GROUP\_NETWORK**](syn__uds_8h.md#define-syn_uds_dtc_group_network)  `0xC00000U`<br> |
| define  | [**SYN\_UDS\_DTC\_GROUP\_POWERTRAIN**](syn__uds_8h.md#define-syn_uds_dtc_group_powertrain)  `0x100000U`<br> |
| define  | [**SYN\_UDS\_DTC\_REPORT\_BY\_SEVERITY\_MASK**](syn__uds_8h.md#define-syn_uds_dtc_report_by_severity_mask)  `0x08U`<br> |
| define  | [**SYN\_UDS\_DTC\_REPORT\_BY\_STATUS\_MASK**](syn__uds_8h.md#define-syn_uds_dtc_report_by_status_mask)  `0x02U`<br> |
| define  | [**SYN\_UDS\_DTC\_REPORT\_EMISSIONS\_OBD\_BY\_STATUS\_MASK**](syn__uds_8h.md#define-syn_uds_dtc_report_emissions_obd_by_status_mask)  `0x13U`<br> |
| define  | [**SYN\_UDS\_DTC\_REPORT\_EXT\_DATA\_RECORD\_BY\_DTC**](syn__uds_8h.md#define-syn_uds_dtc_report_ext_data_record_by_dtc)  `0x06U`<br> |
| define  | [**SYN\_UDS\_DTC\_REPORT\_EXT\_DATA\_RECORD\_BY\_RECORD\_NUM**](syn__uds_8h.md#define-syn_uds_dtc_report_ext_data_record_by_record_num)  `0x16U`<br> |
| define  | [**SYN\_UDS\_DTC\_REPORT\_FAULT\_DETECTION\_COUNTER**](syn__uds_8h.md#define-syn_uds_dtc_report_fault_detection_counter)  `0x14U`<br> |
| define  | [**SYN\_UDS\_DTC\_REPORT\_FIRST\_CONFIRMED**](syn__uds_8h.md#define-syn_uds_dtc_report_first_confirmed)  `0x0CU`<br> |
| define  | [**SYN\_UDS\_DTC\_REPORT\_FIRST\_TEST\_FAILED**](syn__uds_8h.md#define-syn_uds_dtc_report_first_test_failed)  `0x0BU`<br> |
| define  | [**SYN\_UDS\_DTC\_REPORT\_MIRROR\_MEMORY\_BY\_STATUS\_MASK**](syn__uds_8h.md#define-syn_uds_dtc_report_mirror_memory_by_status_mask)  `0x0FU`<br> |
| define  | [**SYN\_UDS\_DTC\_REPORT\_MIRROR\_MEMORY\_EXT\_DATA**](syn__uds_8h.md#define-syn_uds_dtc_report_mirror_memory_ext_data)  `0x10U`<br> |
| define  | [**SYN\_UDS\_DTC\_REPORT\_MOST\_RECENT\_CONFIRMED**](syn__uds_8h.md#define-syn_uds_dtc_report_most_recent_confirmed)  `0x0EU`<br> |
| define  | [**SYN\_UDS\_DTC\_REPORT\_MOST\_RECENT\_TEST\_FAILED**](syn__uds_8h.md#define-syn_uds_dtc_report_most_recent_test_failed)  `0x0DU`<br> |
| define  | [**SYN\_UDS\_DTC\_REPORT\_NUMBER\_BY\_SEVERITY\_MASK**](syn__uds_8h.md#define-syn_uds_dtc_report_number_by_severity_mask)  `0x07U`<br> |
| define  | [**SYN\_UDS\_DTC\_REPORT\_NUMBER\_BY\_STATUS\_MASK**](syn__uds_8h.md#define-syn_uds_dtc_report_number_by_status_mask)  `0x01U`<br> |
| define  | [**SYN\_UDS\_DTC\_REPORT\_NUMBER\_EMISSIONS\_OBD\_BY\_STATUS\_MASK**](syn__uds_8h.md#define-syn_uds_dtc_report_number_emissions_obd_by_status_mask)  `0x12U`<br> |
| define  | [**SYN\_UDS\_DTC\_REPORT\_NUMBER\_MIRROR\_MEMORY\_BY\_STATUS\_MASK**](syn__uds_8h.md#define-syn_uds_dtc_report_number_mirror_memory_by_status_mask)  `0x11U`<br> |
| define  | [**SYN\_UDS\_DTC\_REPORT\_SEVERITY\_INFO**](syn__uds_8h.md#define-syn_uds_dtc_report_severity_info)  `0x09U`<br> |
| define  | [**SYN\_UDS\_DTC\_REPORT\_SNAPSHOT\_IDENTIFICATION**](syn__uds_8h.md#define-syn_uds_dtc_report_snapshot_identification)  `0x03U`<br> |
| define  | [**SYN\_UDS\_DTC\_REPORT\_SNAPSHOT\_RECORD\_BY\_DTC**](syn__uds_8h.md#define-syn_uds_dtc_report_snapshot_record_by_dtc)  `0x04U`<br> |
| define  | [**SYN\_UDS\_DTC\_REPORT\_STORED\_DATA\_BY\_RECORD\_NUM**](syn__uds_8h.md#define-syn_uds_dtc_report_stored_data_by_record_num)  `0x05U`<br> |
| define  | [**SYN\_UDS\_DTC\_REPORT\_SUPPORTED**](syn__uds_8h.md#define-syn_uds_dtc_report_supported)  `0x0AU`<br> |
| define  | [**SYN\_UDS\_DTC\_REPORT\_USER\_DEF\_MEMORY\_BY\_STATUS\_MASK**](syn__uds_8h.md#define-syn_uds_dtc_report_user_def_memory_by_status_mask)  `0x17U`<br> |
| define  | [**SYN\_UDS\_DTC\_REPORT\_USER\_DEF\_MEMORY\_EXT\_DATA\_BY\_DTC**](syn__uds_8h.md#define-syn_uds_dtc_report_user_def_memory_ext_data_by_dtc)  `0x19U`<br> |
| define  | [**SYN\_UDS\_DTC\_REPORT\_USER\_DEF\_MEMORY\_SNAPSHOT\_BY\_DTC**](syn__uds_8h.md#define-syn_uds_dtc_report_user_def_memory_snapshot_by_dtc)  `0x18U`<br> |
| define  | [**SYN\_UDS\_DTC\_REPORT\_WITH\_PERMANENT\_STATUS**](syn__uds_8h.md#define-syn_uds_dtc_report_with_permanent_status)  `0x15U`<br> |
| define  | [**SYN\_UDS\_DTC\_REPORT\_WWH\_OBD\_BY\_MASK\_RECORD**](syn__uds_8h.md#define-syn_uds_dtc_report_wwh_obd_by_mask_record)  `0x42U`<br> |
| define  | [**SYN\_UDS\_DTC\_REPORT\_WWH\_OBD\_WITH\_PERMANENT\_STATUS**](syn__uds_8h.md#define-syn_uds_dtc_report_wwh_obd_with_permanent_status)  `0x55U`<br> |
| define  | [**SYN\_UDS\_DTC\_SEVERITY\_CHECK\_AT\_NEXT\_HALT**](syn__uds_8h.md#define-syn_uds_dtc_severity_check_at_next_halt)  `(0x02U &lt;&lt; 5)`<br> |
| define  | [**SYN\_UDS\_DTC\_SEVERITY\_CHECK\_IMMEDIATELY**](syn__uds_8h.md#define-syn_uds_dtc_severity_check_immediately)  `(0x03U &lt;&lt; 5)`<br> |
| define  | [**SYN\_UDS\_DTC\_SEVERITY\_MAINTENANCE\_REQUIRED**](syn__uds_8h.md#define-syn_uds_dtc_severity_maintenance_required)  `(0x01U &lt;&lt; 5)`<br> |
| define  | [**SYN\_UDS\_DTC\_SEVERITY\_MASK**](syn__uds_8h.md#define-syn_uds_dtc_severity_mask)  `(0x07U &lt;&lt; 5)`<br> |
| define  | [**SYN\_UDS\_DTC\_SEVERITY\_NO\_SEVERITY**](syn__uds_8h.md#define-syn_uds_dtc_severity_no_severity)  `(0x00U &lt;&lt; 5)`<br> |
| define  | [**SYN\_UDS\_DTC\_STATUS\_AVAILABILITY\_MASK**](syn__uds_8h.md#define-syn_uds_dtc_status_availability_mask)  `0xFFU`<br> |
| define  | [**SYN\_UDS\_DTC\_STATUS\_CONFIRMED\_DTC**](syn__uds_8h.md#define-syn_uds_dtc_status_confirmed_dtc)  `(1U &lt;&lt; 3)`<br> |
| define  | [**SYN\_UDS\_DTC\_STATUS\_PENDING\_DTC**](syn__uds_8h.md#define-syn_uds_dtc_status_pending_dtc)  `(1U &lt;&lt; 2)`<br> |
| define  | [**SYN\_UDS\_DTC\_STATUS\_TEST\_FAILED**](syn__uds_8h.md#define-syn_uds_dtc_status_test_failed)  `(1U &lt;&lt; 0)`<br> |
| define  | [**SYN\_UDS\_DTC\_STATUS\_TEST\_FAILED\_SINCE\_LAST\_CLEAR**](syn__uds_8h.md#define-syn_uds_dtc_status_test_failed_since_last_clear)  `(1U &lt;&lt; 5)`<br> |
| define  | [**SYN\_UDS\_DTC\_STATUS\_TEST\_FAILED\_THIS\_OP\_CYCLE**](syn__uds_8h.md#define-syn_uds_dtc_status_test_failed_this_op_cycle)  `(1U &lt;&lt; 1)`<br> |
| define  | [**SYN\_UDS\_DTC\_STATUS\_TEST\_NOT\_COMPLETED\_SINCE\_LAST\_CLEAR**](syn__uds_8h.md#define-syn_uds_dtc_status_test_not_completed_since_last_clear)  `(1U &lt;&lt; 4)`<br> |
| define  | [**SYN\_UDS\_DTC\_STATUS\_TEST\_NOT\_COMPLETED\_THIS\_OP\_CYCLE**](syn__uds_8h.md#define-syn_uds_dtc_status_test_not_completed_this_op_cycle)  `(1U &lt;&lt; 6)`<br> |
| define  | [**SYN\_UDS\_DTC\_STATUS\_WARNING\_INDICATOR\_REQUESTED**](syn__uds_8h.md#define-syn_uds_dtc_status_warning_indicator_requested)  `(1U &lt;&lt; 7)`<br> |
| define  | [**SYN\_UDS\_MAX\_DIDS**](syn__uds_8h.md#define-syn_uds_max_dids)  `16U`<br> |
| define  | [**SYN\_UDS\_MAX\_DTCS**](syn__uds_8h.md#define-syn_uds_max_dtcs)  `32U`<br> |
| define  | [**SYN\_UDS\_MAX\_SERVICE\_OVERRIDES**](syn__uds_8h.md#define-syn_uds_max_service_overrides)  `8U`<br> |
| define  | [**SYN\_UDS\_NRC\_CONDITIONS\_NOT\_CORRECT**](syn__uds_8h.md#define-syn_uds_nrc_conditions_not_correct)  `0x22U`<br> |
| define  | [**SYN\_UDS\_NRC\_EXCEEDED\_NUMBER\_OF\_ATTEMPTS**](syn__uds_8h.md#define-syn_uds_nrc_exceeded_number_of_attempts)  `0x36U`<br> |
| define  | [**SYN\_UDS\_NRC\_GENERAL\_PROGRAMMING\_FAILURE**](syn__uds_8h.md#define-syn_uds_nrc_general_programming_failure)  `0x72U`<br> |
| define  | [**SYN\_UDS\_NRC\_INCORRECT\_MESSAGE\_LENGTH**](syn__uds_8h.md#define-syn_uds_nrc_incorrect_message_length)  `0x13U`<br> |
| define  | [**SYN\_UDS\_NRC\_INVALID\_KEY**](syn__uds_8h.md#define-syn_uds_nrc_invalid_key)  `0x35U`<br> |
| define  | [**SYN\_UDS\_NRC\_REQUEST\_OUT\_OF\_RANGE**](syn__uds_8h.md#define-syn_uds_nrc_request_out_of_range)  `0x31U`<br> |
| define  | [**SYN\_UDS\_NRC\_REQUEST\_SEQUENCE\_ERROR**](syn__uds_8h.md#define-syn_uds_nrc_request_sequence_error)  `0x24U`<br> |
| define  | [**SYN\_UDS\_NRC\_REQUIRED\_TIME\_DELAY\_NOT\_EXPIRED**](syn__uds_8h.md#define-syn_uds_nrc_required_time_delay_not_expired)  `0x37U`<br> |
| define  | [**SYN\_UDS\_NRC\_RESPONSE\_TOO\_LONG**](syn__uds_8h.md#define-syn_uds_nrc_response_too_long)  `0x14U`<br> |
| define  | [**SYN\_UDS\_NRC\_SECURITY\_ACCESS\_DENIED**](syn__uds_8h.md#define-syn_uds_nrc_security_access_denied)  `0x33U`<br> |
| define  | [**SYN\_UDS\_NRC\_SERVICE\_NOT\_SUPPORTED**](syn__uds_8h.md#define-syn_uds_nrc_service_not_supported)  `0x11U`<br> |
| define  | [**SYN\_UDS\_NRC\_SUBFUNCTION\_NOT\_SUPPORTED**](syn__uds_8h.md#define-syn_uds_nrc_subfunction_not_supported)  `0x12U`<br> |
| define  | [**SYN\_UDS\_NRC\_SUBFUNCTION\_NOT\_SUPPORTED\_IN\_ACTIVE\_SESSION**](syn__uds_8h.md#define-syn_uds_nrc_subfunction_not_supported_in_active_session)  `0x7EU`<br> |
| define  | [**SYN\_UDS\_NRC\_SUCCESS**](syn__uds_8h.md#define-syn_uds_nrc_success)  `0x00U`<br> |
| define  | [**SYN\_UDS\_NRC\_TRANSFER\_DATA\_SUSPENDED**](syn__uds_8h.md#define-syn_uds_nrc_transfer_data_suspended)  `0x71U`<br> |
| define  | [**SYN\_UDS\_NRC\_WRONG\_BLOCK\_SEQUENCE\_COUNTER**](syn__uds_8h.md#define-syn_uds_nrc_wrong_block_sequence_counter)  `0x73U`<br> |
| define  | [**SYN\_UDS\_RESET\_DISABLE\_RAPID\_POWER\_SHUTDOWN**](syn__uds_8h.md#define-syn_uds_reset_disable_rapid_power_shutdown)  `0x05U`<br> |
| define  | [**SYN\_UDS\_RESET\_ENABLE\_RAPID\_POWER\_SHUTDOWN**](syn__uds_8h.md#define-syn_uds_reset_enable_rapid_power_shutdown)  `0x04U`<br> |
| define  | [**SYN\_UDS\_RESET\_HARD**](syn__uds_8h.md#define-syn_uds_reset_hard)  `0x01U`<br> |
| define  | [**SYN\_UDS\_RESET\_KEY\_OFF\_ON**](syn__uds_8h.md#define-syn_uds_reset_key_off_on)  `0x02U`<br> |
| define  | [**SYN\_UDS\_RESET\_SOFT**](syn__uds_8h.md#define-syn_uds_reset_soft)  `0x03U`<br> |
| define  | [**SYN\_UDS\_RESPONSE\_NEGATIVE**](syn__uds_8h.md#define-syn_uds_response_negative)  `0x7FU`<br> |
| define  | [**SYN\_UDS\_S3\_TIMEOUT\_MS**](syn__uds_8h.md#define-syn_uds_s3_timeout_ms)  `5000U`<br> |
| define  | [**SYN\_UDS\_SECURITY\_DELAY\_MS**](syn__uds_8h.md#define-syn_uds_security_delay_ms)  `10000U`<br> |
| define  | [**SYN\_UDS\_SECURITY\_MASK\_ALL**](syn__uds_8h.md#define-syn_uds_security_mask_all)  `(0xFFFFU)`<br> |
| define  | [**SYN\_UDS\_SECURITY\_MASK\_LEVEL\_1**](syn__uds_8h.md#define-syn_uds_security_mask_level_1)  `(1U &lt;&lt; 1)`<br> |
| define  | [**SYN\_UDS\_SECURITY\_MASK\_LEVEL\_2**](syn__uds_8h.md#define-syn_uds_security_mask_level_2)  `(1U &lt;&lt; 2)`<br> |
| define  | [**SYN\_UDS\_SECURITY\_MASK\_LEVEL\_3**](syn__uds_8h.md#define-syn_uds_security_mask_level_3)  `(1U &lt;&lt; 3)`<br> |
| define  | [**SYN\_UDS\_SECURITY\_MASK\_NONE**](syn__uds_8h.md#define-syn_uds_security_mask_none)  `(1U &lt;&lt; 0)`<br> |
| define  | [**SYN\_UDS\_SECURITY\_MAX\_ATTEMPTS**](syn__uds_8h.md#define-syn_uds_security_max_attempts)  `3U`<br> |
| define  | [**SYN\_UDS\_SESSION\_MASK\_ALL**](syn__uds_8h.md#define-syn_uds_session_mask_all)  `(0x0FU)`<br> |
| define  | [**SYN\_UDS\_SESSION\_MASK\_DEFAULT**](syn__uds_8h.md#define-syn_uds_session_mask_default)  `(1U &lt;&lt; 0)`<br> |
| define  | [**SYN\_UDS\_SESSION\_MASK\_EXTENDED**](syn__uds_8h.md#define-syn_uds_session_mask_extended)  `(1U &lt;&lt; 2)`<br> |
| define  | [**SYN\_UDS\_SESSION\_MASK\_PROGRAMMING**](syn__uds_8h.md#define-syn_uds_session_mask_programming)  `(1U &lt;&lt; 1)`<br> |
| define  | [**SYN\_UDS\_SESSION\_MASK\_SAFETY**](syn__uds_8h.md#define-syn_uds_session_mask_safety)  `(1U &lt;&lt; 3)`<br> |
| define  | [**SYN\_UDS\_SID\_ACCESS\_TIMING\_PARAMETER**](syn__uds_8h.md#define-syn_uds_sid_access_timing_parameter)  `0x83U`<br> |
| define  | [**SYN\_UDS\_SID\_AUTHENTICATION**](syn__uds_8h.md#define-syn_uds_sid_authentication)  `0x29U`<br> |
| define  | [**SYN\_UDS\_SID\_CLEAR\_DIAGNOSTIC\_INFORMATION**](syn__uds_8h.md#define-syn_uds_sid_clear_diagnostic_information)  `0x14U`<br> |
| define  | [**SYN\_UDS\_SID\_COMMUNICATION\_CONTROL**](syn__uds_8h.md#define-syn_uds_sid_communication_control)  `0x28U`<br> |
| define  | [**SYN\_UDS\_SID\_CONTROL\_DTC\_SETTING**](syn__uds_8h.md#define-syn_uds_sid_control_dtc_setting)  `0x85U`<br> |
| define  | [**SYN\_UDS\_SID\_DIAGNOSTIC\_SESSION\_CONTROL**](syn__uds_8h.md#define-syn_uds_sid_diagnostic_session_control)  `0x10U`<br> |
| define  | [**SYN\_UDS\_SID\_DYNAMICALLY\_DEFINE\_DATA\_IDENTIFIER**](syn__uds_8h.md#define-syn_uds_sid_dynamically_define_data_identifier)  `0x2CU`<br> |
| define  | [**SYN\_UDS\_SID\_ECU\_RESET**](syn__uds_8h.md#define-syn_uds_sid_ecu_reset)  `0x11U`<br> |
| define  | [**SYN\_UDS\_SID\_INPUT\_OUTPUT\_CONTROL\_BY\_IDENTIFIER**](syn__uds_8h.md#define-syn_uds_sid_input_output_control_by_identifier)  `0x2FU`<br> |
| define  | [**SYN\_UDS\_SID\_LINK\_CONTROL**](syn__uds_8h.md#define-syn_uds_sid_link_control)  `0x87U`<br> |
| define  | [**SYN\_UDS\_SID\_READ\_DATA\_BY\_IDENTIFIER**](syn__uds_8h.md#define-syn_uds_sid_read_data_by_identifier)  `0x22U`<br> |
| define  | [**SYN\_UDS\_SID\_READ\_DATA\_BY\_PERIODIC\_IDENTIFIER**](syn__uds_8h.md#define-syn_uds_sid_read_data_by_periodic_identifier)  `0x2AU`<br> |
| define  | [**SYN\_UDS\_SID\_READ\_DTC\_INFORMATION**](syn__uds_8h.md#define-syn_uds_sid_read_dtc_information)  `0x19U`<br> |
| define  | [**SYN\_UDS\_SID\_READ\_MEMORY\_BY\_ADDRESS**](syn__uds_8h.md#define-syn_uds_sid_read_memory_by_address)  `0x23U`<br> |
| define  | [**SYN\_UDS\_SID\_READ\_SCALING\_DATA\_BY\_IDENTIFIER**](syn__uds_8h.md#define-syn_uds_sid_read_scaling_data_by_identifier)  `0x24U`<br> |
| define  | [**SYN\_UDS\_SID\_REQUEST\_DOWNLOAD**](syn__uds_8h.md#define-syn_uds_sid_request_download)  `0x34U`<br> |
| define  | [**SYN\_UDS\_SID\_REQUEST\_FILE\_TRANSFER**](syn__uds_8h.md#define-syn_uds_sid_request_file_transfer)  `0x38U`<br> |
| define  | [**SYN\_UDS\_SID\_REQUEST\_TRANSFER\_EXIT**](syn__uds_8h.md#define-syn_uds_sid_request_transfer_exit)  `0x37U`<br> |
| define  | [**SYN\_UDS\_SID\_REQUEST\_UPLOAD**](syn__uds_8h.md#define-syn_uds_sid_request_upload)  `0x35U`<br> |
| define  | [**SYN\_UDS\_SID\_RESPONSE\_ON\_EVENT**](syn__uds_8h.md#define-syn_uds_sid_response_on_event)  `0x86U`<br> |
| define  | [**SYN\_UDS\_SID\_ROUTINE\_CONTROL**](syn__uds_8h.md#define-syn_uds_sid_routine_control)  `0x31U`<br> |
| define  | [**SYN\_UDS\_SID\_SECURED\_DATA\_TRANSMISSION**](syn__uds_8h.md#define-syn_uds_sid_secured_data_transmission)  `0x84U`<br> |
| define  | [**SYN\_UDS\_SID\_SECURITY\_ACCESS**](syn__uds_8h.md#define-syn_uds_sid_security_access)  `0x27U`<br> |
| define  | [**SYN\_UDS\_SID\_TESTER\_PRESENT**](syn__uds_8h.md#define-syn_uds_sid_tester_present)  `0x3EU`<br> |
| define  | [**SYN\_UDS\_SID\_TRANSFER\_DATA**](syn__uds_8h.md#define-syn_uds_sid_transfer_data)  `0x36U`<br> |
| define  | [**SYN\_UDS\_SID\_WRITE\_DATA\_BY\_IDENTIFIER**](syn__uds_8h.md#define-syn_uds_sid_write_data_by_identifier)  `0x2EU`<br> |
| define  | [**SYN\_UDS\_SID\_WRITE\_MEMORY\_BY\_ADDRESS**](syn__uds_8h.md#define-syn_uds_sid_write_memory_by_address)  `0x3DU`<br> |

## Detailed Description


Provides a zero-allocation, lightweight UDS server protocol stack for automotive diagnostic session management, Data Identifier (DID) read/write, security access seed/key unlocks, routine controls, and ECU reset over ISO-TP. 


    
## Public Types Documentation




### typedef SYN\_UDS\_AccessTimingHandler 

_AccessTimingParameter (0x83) callback function signature._ 
```C++
typedef bool(* SYN_UDS_AccessTimingHandler) (SYN_UDS_AccessTimingType timing_type, uint16_t *p2_max_ms, uint16_t *p2_star_max_10ms, void *ctx);
```




<hr>



### enum SYN\_UDS\_AccessTimingType 

_UDS AccessTimingParameter (0x83) Subfunctions._ 
```C++
enum SYN_UDS_AccessTimingType {
    SYN_UDS_TIMING_READ_EXTENDED = 0x01U,
    SYN_UDS_TIMING_SET_TO_DEFAULT = 0x02U,
    SYN_UDS_TIMING_READ_ACTIVE = 0x03U,
    SYN_UDS_TIMING_SET_TO_GIVEN = 0x04U
};
```




<hr>



### enum SYN\_UDS\_AddrMode 

_UDS Addressing Modes (ISO 14229-1)_ 
```C++
enum SYN_UDS_AddrMode {
    SYN_UDS_ADDR_PHYSICAL = 0U,
    SYN_UDS_ADDR_FUNCTIONAL = 1U
};
```




<hr>



### typedef SYN\_UDS\_AuthHandler 

_Authentication (0x29) callback function signature._ 
```C++
typedef bool(* SYN_UDS_AuthHandler) (uint8_t subfunction, const uint8_t *in_data, uint16_t in_len, uint8_t *out_buf, uint16_t max_out_len, uint16_t *out_len, void *ctx);
```




<hr>



### typedef SYN\_UDS\_CommControlHandler 

_CommunicationControl (0x28) callback function signature._ 
```C++
typedef bool(* SYN_UDS_CommControlHandler) (SYN_UDS_CommControlType control_type, uint8_t comm_type, void *ctx);
```




<hr>



### enum SYN\_UDS\_CommControlType 

_UDS CommunicationControl (0x28) Subfunctions._ 
```C++
enum SYN_UDS_CommControlType {
    SYN_UDS_COMM_ENABLE_RX_AND_TX = 0x00U,
    SYN_UDS_COMM_ENABLE_RX_DISABLE_TX = 0x01U,
    SYN_UDS_COMM_DISABLE_RX_ENABLE_TX = 0x02U,
    SYN_UDS_COMM_DISABLE_RX_AND_TX = 0x03U,
    SYN_UDS_COMM_ENABLE_RX_TX_ENHANCED = 0x04U,
    SYN_UDS_COMM_ENABLE_RX_ENHANCED = 0x05U
};
```




<hr>



### typedef SYN\_UDS\_DTCHandler 

_ReadDTCInformation (0x19) callback function signature._ 
```C++
typedef bool(* SYN_UDS_DTCHandler) (uint8_t subfunction, const uint8_t *in_data, uint16_t in_len, uint8_t *out_buf, uint16_t max_out_len, uint16_t *out_len, void *ctx);
```




<hr>



### typedef SYN\_UDS\_DynamicDIDHandler 

_DynamicallyDefineDataIdentifier (0x2C) callback function signature._ 
```C++
typedef bool(* SYN_UDS_DynamicDIDHandler) (uint8_t subfunction, uint16_t dyn_did, const uint8_t *in_data, uint16_t in_len, uint8_t *out_buf, uint16_t max_out_len, uint16_t *out_len, void *ctx);
```




<hr>



### typedef SYN\_UDS\_FileTransferHandler 

_RequestFileTransfer (0x38) callback function signature._ 
```C++
typedef bool(* SYN_UDS_FileTransferHandler) (uint8_t mode, const char *file_path, uint16_t path_len, uint8_t *out_buf, uint16_t max_out_len, uint16_t *out_len, void *ctx);
```




<hr>



### typedef SYN\_UDS\_IOControlHandler 

_InputOutputControlByIdentifier (0x2F) callback function signature._ 
```C++
typedef bool(* SYN_UDS_IOControlHandler) (uint16_t did, uint8_t control_opt, const uint8_t *in_data, uint16_t in_len, uint8_t *out_buf, uint16_t max_out_len, uint16_t *out_len, void *ctx);
```




<hr>



### typedef SYN\_UDS\_LinkControlHandler 

_LinkControl (0x87) callback function signature._ 
```C++
typedef bool(* SYN_UDS_LinkControlHandler) (uint8_t subfunction, const uint8_t *in_data, uint16_t in_len, uint8_t *out_buf, uint16_t max_out_len, uint16_t *out_len, void *ctx);
```




<hr>



### typedef SYN\_UDS\_MemoryHandler 

_ReadMemoryByAddress (0x23) and WriteMemoryByAddress (0x3D) callback function signature._ 
```C++
typedef bool(* SYN_UDS_MemoryHandler) (bool is_write, uint32_t address, uint32_t size, uint8_t *data_buf, void *ctx);
```




<hr>



### typedef SYN\_UDS\_PeriodicDataHandler 

_ReadDataByPeriodicIdentifier (0x2A) callback function signature._ 
```C++
typedef bool(* SYN_UDS_PeriodicDataHandler) (uint8_t mode, const uint8_t *in_data, uint16_t in_len, uint8_t *out_buf, uint16_t max_out_len, uint16_t *out_len, void *ctx);
```




<hr>



### typedef SYN\_UDS\_ResetHandler 

_ECUReset (0x11) deferred post-TX reset callback function signature._ 
```C++
typedef void(* SYN_UDS_ResetHandler) (uint8_t reset_type, void *ctx);
```




<hr>



### typedef SYN\_UDS\_ResponseOnEventHandler 

_ResponseOnEvent (0x86) callback function signature._ 
```C++
typedef bool(* SYN_UDS_ResponseOnEventHandler) (uint8_t subfunction, const uint8_t *in_data, uint16_t in_len, uint8_t *out_buf, uint16_t max_out_len, uint16_t *out_len, void *ctx);
```




<hr>



### typedef SYN\_UDS\_RoutineControlHandler 

_RoutineControl (0x31) callback function signature._ 
```C++
typedef bool(* SYN_UDS_RoutineControlHandler) (uint8_t subfunction, uint16_t routine_id, const uint8_t *in_data, uint16_t in_len, uint8_t *out_buf, uint16_t max_out_len, uint16_t *out_len, void *ctx);
```




<hr>



### typedef SYN\_UDS\_ScalingDataHandler 

_ReadScalingDataByIdentifier (0x24) callback function signature._ 
```C++
typedef bool(* SYN_UDS_ScalingDataHandler) (uint16_t did, uint8_t *out_buf, uint16_t max_out_len, uint16_t *out_len, void *ctx);
```




<hr>



### typedef SYN\_UDS\_SecuredDataHandler 

_SecuredDataTransmission (0x84) callback function signature._ 
```C++
typedef bool(* SYN_UDS_SecuredDataHandler) (const uint8_t *in_data, uint16_t in_len, uint8_t *out_buf, uint16_t max_out_len, uint16_t *out_len, void *ctx);
```




<hr>



### enum SYN\_UDS\_SecurityState 

_UDS Security Access Unlock States._ 
```C++
enum SYN_UDS_SecurityState {
    SYN_UDS_SECURITY_LOCKED = 0x00U,
    SYN_UDS_SECURITY_SEED_SENT = 0x01U,
    SYN_UDS_SECURITY_UNLOCKED = 0x02U
};
```




<hr>



### enum SYN\_UDS\_Session 

_UDS Diagnostic Session Types (ISO 14229-1)_ 
```C++
enum SYN_UDS_Session {
    SYN_UDS_SESSION_DEFAULT = 0x01U,
    SYN_UDS_SESSION_PROGRAMMING = 0x02U,
    SYN_UDS_SESSION_EXTENDED = 0x03U,
    SYN_UDS_SESSION_SAFETY_SYSTEM = 0x04U
};
```




<hr>



### typedef SYN\_UDS\_SessionTransitionHandler 

_DiagnosticSessionControl (0x10) session transition rule callback signature._ 
```C++
typedef bool(* SYN_UDS_SessionTransitionHandler) (SYN_UDS_Session from_session, SYN_UDS_Session to_session, void *ctx);
```




<hr>



### enum SYN\_UDS\_TransferState 

_UDS Data Transfer States._ 
```C++
enum SYN_UDS_TransferState {
    SYN_UDS_TRANSFER_IDLE = 0U,
    SYN_UDS_TRANSFER_DOWNLOAD = 1U,
    SYN_UDS_TRANSFER_UPLOAD = 2U
};
```




<hr>
## Public Functions Documentation




### function syn\_uds\_clear\_pending\_reset 

_Clear pending ECU reset flag after executing reset or transmitting response._ 
```C++
void syn_uds_clear_pending_reset (
    SYN_UDS_Server * server
) 
```





**Parameters:**


* `server` Pointer to UDS server instance. 




        

<hr>



### function syn\_uds\_disable\_aes128\_security 

_Disable AES-128 security mode and revert to standard XOR security key calculation._ 
```C++
bool syn_uds_disable_aes128_security (
    SYN_UDS_Server * server
) 
```





**Parameters:**


* `server` Pointer to UDS server instance. 



**Returns:**

true on success, false if server is NULL. 





        

<hr>



### function syn\_uds\_dtc\_get\_status 

_Get current 8-bit status byte for a registered DTC._ 
```C++
bool syn_uds_dtc_get_status (
    SYN_UDS_Server * server,
    uint32_t dtc,
    uint8_t * out_status
) 
```





**Parameters:**


* `server` Pointer to UDS server instance. 
* `dtc` 24-bit Diagnostic Trouble Code. 
* `out_status` Output pointer to store DTC status byte. 



**Returns:**

true if DTC found, false otherwise. 





        

<hr>



### function syn\_uds\_dtc\_report\_test\_result 

_Report diagnostic test result for a registered DTC according to ISO 14229-1 state transitions._ 
```C++
bool syn_uds_dtc_report_test_result (
    SYN_UDS_Server * server,
    uint32_t dtc,
    bool failed
) 
```



Updates DTC status bits (testFailed, testFailedThisOperationCycle, pendingDTC, confirmedDTC, etc.) and fault detection counter.




**Parameters:**


* `server` Pointer to UDS server instance. 
* `dtc` 24-bit Diagnostic Trouble Code. 
* `failed` True if test failed, false if test passed. 



**Returns:**

true if DTC found and updated, false otherwise. 





        

<hr>



### function syn\_uds\_dtc\_start\_operation\_cycle 

_Advance server state to a new operation cycle. Clears testFailedThisOperationCycle and sets testNotCompletedThisOperationCycle for all registered DTCs._ 
```C++
bool syn_uds_dtc_start_operation_cycle (
    SYN_UDS_Server * server
) 
```





**Parameters:**


* `server` Pointer to UDS server instance. 



**Returns:**

true on success, false if server is NULL. 





        

<hr>



### function syn\_uds\_enable\_aes128\_security 

_Enable AES-128 algorithm for UDS SecurityAccess (0x27) seed/key unlock._ 
```C++
bool syn_uds_enable_aes128_security (
    SYN_UDS_Server * server,
    const uint8_t key
) 
```





**Parameters:**


* `server` Pointer to UDS server instance. 
* `key` 16-byte AES-128 secret key. 



**Returns:**

true on success, false if server or key is NULL. 





        

<hr>



### function syn\_uds\_get\_pending\_reset 

_Get pending ECU reset sub-function requested by 0x11 service._ 
```C++
uint8_t syn_uds_get_pending_reset (
    const SYN_UDS_Server * server
) 
```





**Parameters:**


* `server` Pointer to UDS server instance. 



**Returns:**

Pending reset type (e.g. 0x01 = hard, 0x02 = keyOffOn, 0x03 = soft), or 0 if none. 





        

<hr>



### function syn\_uds\_get\_security\_level 

_Get currently unlocked security level (0 = locked, 1 = level 1, 2 = level 2...)._ 
```C++
uint8_t syn_uds_get_security_level (
    const SYN_UDS_Server * server
) 
```





**Parameters:**


* `server` Pointer to UDS server instance. 



**Returns:**

Active security level uint8\_t. 





        

<hr>



### function syn\_uds\_init 

_Initialize UDS Server context._ 
```C++
bool syn_uds_init (
    SYN_UDS_Server * server
) 
```





**Parameters:**


* `server` Pointer to UDS server instance. 



**Returns:**

true on success, false if server is NULL. 





        

<hr>



### function syn\_uds\_is\_sid\_functional\_supported 

_Check if Service Identifier (SID) supports Functional Addressing (1:N Broadcast)._ 
```C++
bool syn_uds_is_sid_functional_supported (
    uint8_t sid
) 
```





**Parameters:**


* `sid` Service Identifier (SID). 



**Returns:**

true if service supports functional addressing, false if physical addressing only. 





        

<hr>



### function syn\_uds\_process\_request 

_Process incoming UDS request diagnostic payload and format response._ 
```C++
bool syn_uds_process_request (
    SYN_UDS_Server * server,
    const uint8_t * req,
    uint16_t req_len,
    uint8_t * resp_buf,
    uint16_t max_resp_len,
    uint16_t * resp_len,
    SYN_UDS_AddrMode addr_mode
) 
```





**Parameters:**


* `server` Pointer to UDS server instance. 
* `req` Pointer to input request bytes. 
* `req_len` Length of request payload in bytes. 
* `resp_buf` Output buffer for response bytes. 
* `max_resp_len` Capacity of output response buffer. 
* `resp_len` Pointer to store generated response byte length. 
* `addr_mode` Addressing mode (SYN\_UDS\_ADDR\_PHYSICAL or SYN\_UDS\_ADDR\_FUNCTIONAL). 



**Returns:**

true if response frame generated, false on invalid parameters. 





        

<hr>



### function syn\_uds\_register\_access\_timing 

_Register AccessTimingParameter (0x83) callback handler._ 
```C++
bool syn_uds_register_access_timing (
    SYN_UDS_Server * server,
    SYN_UDS_AccessTimingHandler handler,
    void * ctx
) 
```





**Parameters:**


* `server` Pointer to UDS server instance. 
* `handler` Callback function invoked when Service 0x83 is processed. 
* `ctx` User context passed to handler. 



**Returns:**

true on success, false if server is NULL. 





        

<hr>



### function syn\_uds\_register\_auth\_handler 

_Register Authentication (0x29) handler._ 
```C++
bool syn_uds_register_auth_handler (
    SYN_UDS_Server * server,
    SYN_UDS_AuthHandler handler,
    void * ctx
) 
```





**Parameters:**


* `server` Pointer to UDS server instance. 
* `handler` Callback function invoked when Service 0x29 is processed. 
* `ctx` User context passed to handler. 



**Returns:**

true on success, false if server is NULL. 





        

<hr>



### function syn\_uds\_register\_comm\_control 

_Register CommunicationControl (0x28) callback handler._ 
```C++
bool syn_uds_register_comm_control (
    SYN_UDS_Server * server,
    SYN_UDS_CommControlHandler handler,
    void * ctx
) 
```





**Parameters:**


* `server` Pointer to UDS server instance. 
* `handler` Callback function invoked when Service 0x28 is processed. 
* `ctx` User context passed to handler. 



**Returns:**

true on success, false if server is NULL. 





        

<hr>



### function syn\_uds\_register\_did 

_Register Data Identifier (DID) mapping in UDS Server table._ 
```C++
bool syn_uds_register_did (
    SYN_UDS_Server * server,
    uint16_t did,
    uint8_t * data,
    uint16_t len,
    bool writable
) 
```





**Parameters:**


* `server` Pointer to UDS server instance. 
* `did` 16-bit Data Identifier code (e.g., 0xF190 for VIN). 
* `data` Pointer to data memory buffer. 
* `len` Data byte length. 
* `writable` True if DID allows WriteDataByIdentifier (0x2E). 



**Returns:**

true on success, false if table full or invalid params. 





        

<hr>



### function syn\_uds\_register\_did\_ext 

_Register Data Identifier (DID) with custom session & security permission bitmask._ 
```C++
bool syn_uds_register_did_ext (
    SYN_UDS_Server * server,
    uint16_t did,
    uint8_t * data,
    uint16_t len,
    bool writable,
    uint8_t session_mask,
    uint16_t security_mask
) 
```





**Parameters:**


* `server` Pointer to UDS server instance. 
* `did` 16-bit Data Identifier code (e.g., 0xF190 for VIN). 
* `data` Pointer to data memory buffer. 
* `len` Data byte length. 
* `writable` True if DID allows WriteDataByIdentifier (0x2E). 
* `session_mask` Permitted session bitmask (SYN\_UDS\_SESSION\_MASK\_\*). 
* `security_mask` Permitted security level bitmask (SYN\_UDS\_SECURITY\_MASK\_\*). 



**Returns:**

true on success, false if table full or invalid params. 





        

<hr>



### function syn\_uds\_register\_dtc 

_Register Diagnostic Trouble Code (DTC) in UDS Server table._ 
```C++
bool syn_uds_register_dtc (
    SYN_UDS_Server * server,
    uint32_t dtc,
    uint8_t status,
    uint8_t severity
) 
```





**Parameters:**


* `server` Pointer to UDS server instance. 
* `dtc` 24-bit Diagnostic Trouble Code (e.g., 0x012345). 
* `status` Initial DTCStatusByte bitmask. 
* `severity` DTCSeverityByte (severity & class). 



**Returns:**

true on success, false if table full or invalid params. 





        

<hr>



### function syn\_uds\_register\_dtc\_handler 

_Register ReadDTCInformation (0x19) callback handler._ 
```C++
bool syn_uds_register_dtc_handler (
    SYN_UDS_Server * server,
    SYN_UDS_DTCHandler handler,
    void * ctx
) 
```





**Parameters:**


* `server` Pointer to UDS server instance. 
* `handler` Callback function invoked for custom DTC subfunctions. 
* `ctx` User context passed to handler. 



**Returns:**

true on success, false if server is NULL. 





        

<hr>



### function syn\_uds\_register\_dynamic\_did\_handler 

_Register DynamicallyDefineDataIdentifier (0x2C) callback handler._ 
```C++
bool syn_uds_register_dynamic_did_handler (
    SYN_UDS_Server * server,
    SYN_UDS_DynamicDIDHandler handler,
    void * ctx
) 
```





**Parameters:**


* `server` Pointer to UDS server instance. 
* `handler` Callback function invoked when Service 0x2C is processed. 
* `ctx` User context passed to handler. 



**Returns:**

true on success, false if server is NULL. 





        

<hr>



### function syn\_uds\_register\_file\_transfer 

_Register RequestFileTransfer (0x38) handler._ 
```C++
bool syn_uds_register_file_transfer (
    SYN_UDS_Server * server,
    SYN_UDS_FileTransferHandler handler,
    void * ctx
) 
```





**Parameters:**


* `server` Pointer to UDS server instance. 
* `handler` Callback function invoked when Service 0x38 is processed. 
* `ctx` User context passed to handler. 



**Returns:**

true on success, false if server is NULL. 





        

<hr>



### function syn\_uds\_register\_io\_control 

_Register InputOutputControlByIdentifier (0x2F) callback handler._ 
```C++
bool syn_uds_register_io_control (
    SYN_UDS_Server * server,
    SYN_UDS_IOControlHandler handler,
    void * ctx
) 
```





**Parameters:**


* `server` Pointer to UDS server instance. 
* `handler` Callback function invoked when Service 0x2F is processed. 
* `ctx` User context passed to handler. 



**Returns:**

true on success, false if server is NULL. 





        

<hr>



### function syn\_uds\_register\_link\_control 

_Register LinkControl (0x87) callback handler._ 
```C++
bool syn_uds_register_link_control (
    SYN_UDS_Server * server,
    SYN_UDS_LinkControlHandler handler,
    void * ctx
) 
```





**Parameters:**


* `server` Pointer to UDS server instance. 
* `handler` Callback function invoked when Service 0x87 is processed. 
* `ctx` User context passed to handler. 



**Returns:**

true on success, false if server is NULL. 





        

<hr>



### function syn\_uds\_register\_memory\_handler 

_Register ReadMemoryByAddress (0x23) and WriteMemoryByAddress (0x3D) handler._ 
```C++
bool syn_uds_register_memory_handler (
    SYN_UDS_Server * server,
    SYN_UDS_MemoryHandler handler,
    void * ctx
) 
```





**Parameters:**


* `server` Pointer to UDS server instance. 
* `handler` Callback function invoked when memory services are processed. 
* `ctx` User context passed to handler. 



**Returns:**

true on success, false if server is NULL. 





        

<hr>



### function syn\_uds\_register\_periodic\_data\_handler 

_Register ReadDataByPeriodicIdentifier (0x2A) callback handler._ 
```C++
bool syn_uds_register_periodic_data_handler (
    SYN_UDS_Server * server,
    SYN_UDS_PeriodicDataHandler handler,
    void * ctx
) 
```





**Parameters:**


* `server` Pointer to UDS server instance. 
* `handler` Callback function invoked when Service 0x2A is processed. 
* `ctx` User context passed to handler. 



**Returns:**

true on success, false if server is NULL. 





        

<hr>



### function syn\_uds\_register\_roe\_handler 

_Register ResponseOnEvent (0x86) callback handler._ 
```C++
bool syn_uds_register_roe_handler (
    SYN_UDS_Server * server,
    SYN_UDS_ResponseOnEventHandler handler,
    void * ctx
) 
```





**Parameters:**


* `server` Pointer to UDS server instance. 
* `handler` Callback function invoked when Service 0x86 is processed. 
* `ctx` User context passed to handler. 



**Returns:**

true on success, false if server is NULL. 





        

<hr>



### function syn\_uds\_register\_routine\_control 

_Register RoutineControl (0x31) callback handler._ 
```C++
bool syn_uds_register_routine_control (
    SYN_UDS_Server * server,
    SYN_UDS_RoutineControlHandler handler,
    void * ctx
) 
```





**Parameters:**


* `server` Pointer to UDS server instance. 
* `handler` Callback function invoked when Service 0x31 is processed. 
* `ctx` User context passed to handler. 



**Returns:**

true on success, false if server is NULL. 





        

<hr>



### function syn\_uds\_register\_scaling\_data\_handler 

_Register ReadScalingDataByIdentifier (0x24) callback handler._ 
```C++
bool syn_uds_register_scaling_data_handler (
    SYN_UDS_Server * server,
    SYN_UDS_ScalingDataHandler handler,
    void * ctx
) 
```





**Parameters:**


* `server` Pointer to UDS server instance. 
* `handler` Callback function invoked when Service 0x24 is processed. 
* `ctx` User context passed to handler. 



**Returns:**

true on success, false if server is NULL. 





        

<hr>



### function syn\_uds\_register\_secured\_data 

_Register SecuredDataTransmission (0x84) callback handler._ 
```C++
bool syn_uds_register_secured_data (
    SYN_UDS_Server * server,
    SYN_UDS_SecuredDataHandler handler,
    void * ctx
) 
```





**Parameters:**


* `server` Pointer to UDS server instance. 
* `handler` Callback function invoked when Service 0x84 is processed. 
* `ctx` User context passed to handler. 



**Returns:**

true on success, false if server is NULL. 





        

<hr>



### function syn\_uds\_set\_reset\_handler 

_Register deferred post-TX ECU reset handler callback._ 
```C++
void syn_uds_set_reset_handler (
    SYN_UDS_Server * server,
    SYN_UDS_ResetHandler cb,
    void * ctx
) 
```





**Parameters:**


* `server` Pointer to UDS server instance. 
* `cb` Callback function to execute after post-TX reset delay. 
* `ctx` Context pointer passed to callback function. 




        

<hr>



### function syn\_uds\_set\_reset\_wait\_ms 

_Set post-TX ECU reset delay window duration in milliseconds._ 
```C++
void syn_uds_set_reset_wait_ms (
    SYN_UDS_Server * server,
    uint16_t wait_ms
) 
```





**Parameters:**


* `server` Pointer to UDS server instance. 
* `wait_ms` Delay duration in ms before executing reset callback. 




        

<hr>



### function syn\_uds\_set\_security\_seed\_bytes 

_Set custom 16-byte seed for AES-128 SecurityAccess._ 
```C++
bool syn_uds_set_security_seed_bytes (
    SYN_UDS_Server * server,
    const uint8_t seed
) 
```





**Parameters:**


* `server` Pointer to UDS server instance. 
* `seed` 16-byte seed buffer. 



**Returns:**

true on success, false if server or seed is NULL. 





        

<hr>



### function syn\_uds\_set\_service\_security\_mask 

_Configure required security level mask for a specific Service Identifier._ 
```C++
bool syn_uds_set_service_security_mask (
    SYN_UDS_Server * server,
    uint8_t sid,
    uint16_t security_mask
) 
```





**Parameters:**


* `server` Pointer to UDS server instance. 
* `sid` Service Identifier (e.g. 0x27, 0x34). 
* `security_mask` Required security level bitmask (SYN\_UDS\_SECURITY\_MASK\_\*). 



**Returns:**

true on success, false if server is NULL. 





        

<hr>



### function syn\_uds\_set\_service\_session\_mask 

_Configure allowed diagnostic session mask for a specific Service Identifier._ 
```C++
bool syn_uds_set_service_session_mask (
    SYN_UDS_Server * server,
    uint8_t sid,
    uint8_t session_mask
) 
```





**Parameters:**


* `server` Pointer to UDS server instance. 
* `sid` Service Identifier (e.g. 0x27, 0x34). 
* `session_mask` Allowed session bitmask (SYN\_UDS\_SESSION\_MASK\_\*). 



**Returns:**

true on success, false if server is NULL. 





        

<hr>



### function syn\_uds\_set\_session\_transition\_handler 

_Register optional session transition policy callback._ 
```C++
void syn_uds_set_session_transition_handler (
    SYN_UDS_Server * server,
    SYN_UDS_SessionTransitionHandler cb,
    void * ctx
) 
```





**Parameters:**


* `server` Pointer to UDS server instance. 
* `cb` Callback function to validate session transition graph permissions. 
* `ctx` Context pointer passed to callback function. 




        

<hr>



### function syn\_uds\_tick 

_Advance periodic S3 server timer by dt\_ms milliseconds. Reverts session to DEFAULT and locks security state if S3 timer expires (5000ms)._ 
```C++
void syn_uds_tick (
    SYN_UDS_Server * server,
    uint32_t dt_ms
) 
```





**Parameters:**


* `server` Pointer to UDS server instance. 
* `dt_ms` Milliseconds elapsed since last tick. 




        

<hr>
## Macro Definition Documentation





### define SYN\_UDS\_DEFAULT\_RESET\_TX\_WAIT\_MS 

```C++
#define SYN_UDS_DEFAULT_RESET_TX_WAIT_MS `50U`
```



Default post-TX ECU reset wait time in ms 


        

<hr>



### define SYN\_UDS\_DTC\_CLASS\_MASK 

```C++
#define SYN_UDS_DTC_CLASS_MASK `(0x1FU)`
```



DTC class mask 


        

<hr>



### define SYN\_UDS\_DTC\_FORMAT\_ISO14229\_1 

```C++
#define SYN_UDS_DTC_FORMAT_ISO14229_1 `0x00U`
```



ISO 14229-1 DTC format 


        

<hr>



### define SYN\_UDS\_DTC\_FORMAT\_ISO15031\_6 

```C++
#define SYN_UDS_DTC_FORMAT_ISO15031_6 `0x01U`
```



ISO 15031-6 DTC format 


        

<hr>



### define SYN\_UDS\_DTC\_FORMAT\_ISO27145\_4 

```C++
#define SYN_UDS_DTC_FORMAT_ISO27145_4 `0x03U`
```



ISO 27145-4 DTC format 


        

<hr>



### define SYN\_UDS\_DTC\_FORMAT\_SAE\_J1939\_73 

```C++
#define SYN_UDS_DTC_FORMAT_SAE_J1939_73 `0x02U`
```



SAE J1939-73 DTC format 


        

<hr>



### define SYN\_UDS\_DTC\_GROUP\_ALL 

```C++
#define SYN_UDS_DTC_GROUP_ALL `0xFFFFFFU`
```



All DTC groups 


        

<hr>



### define SYN\_UDS\_DTC\_GROUP\_BODY 

```C++
#define SYN_UDS_DTC_GROUP_BODY `0x800000U`
```



Body DTC group 


        

<hr>



### define SYN\_UDS\_DTC\_GROUP\_CHASSIS 

```C++
#define SYN_UDS_DTC_GROUP_CHASSIS `0x400000U`
```



Chassis DTC group 


        

<hr>



### define SYN\_UDS\_DTC\_GROUP\_EMISSIONS 

```C++
#define SYN_UDS_DTC_GROUP_EMISSIONS `0x000000U`
```



Emissions-related systems DTC group 


        

<hr>



### define SYN\_UDS\_DTC\_GROUP\_NETWORK 

```C++
#define SYN_UDS_DTC_GROUP_NETWORK `0xC00000U`
```



Network DTC group 


        

<hr>



### define SYN\_UDS\_DTC\_GROUP\_POWERTRAIN 

```C++
#define SYN_UDS_DTC_GROUP_POWERTRAIN `0x100000U`
```



Powertrain DTC group 


        

<hr>



### define SYN\_UDS\_DTC\_REPORT\_BY\_SEVERITY\_MASK 

```C++
#define SYN_UDS_DTC_REPORT_BY_SEVERITY_MASK `0x08U`
```



Report DTCs by severity mask 


        

<hr>



### define SYN\_UDS\_DTC\_REPORT\_BY\_STATUS\_MASK 

```C++
#define SYN_UDS_DTC_REPORT_BY_STATUS_MASK `0x02U`
```



Report DTCs by status mask 


        

<hr>



### define SYN\_UDS\_DTC\_REPORT\_EMISSIONS\_OBD\_BY\_STATUS\_MASK 

```C++
#define SYN_UDS_DTC_REPORT_EMISSIONS_OBD_BY_STATUS_MASK `0x13U`
```



Report OBD DTCs 


        

<hr>



### define SYN\_UDS\_DTC\_REPORT\_EXT\_DATA\_RECORD\_BY\_DTC 

```C++
#define SYN_UDS_DTC_REPORT_EXT_DATA_RECORD_BY_DTC `0x06U`
```



Report extended data record by DTC 


        

<hr>



### define SYN\_UDS\_DTC\_REPORT\_EXT\_DATA\_RECORD\_BY\_RECORD\_NUM 

```C++
#define SYN_UDS_DTC_REPORT_EXT_DATA_RECORD_BY_RECORD_NUM `0x16U`
```



Report extended data by record num 


        

<hr>



### define SYN\_UDS\_DTC\_REPORT\_FAULT\_DETECTION\_COUNTER 

```C++
#define SYN_UDS_DTC_REPORT_FAULT_DETECTION_COUNTER `0x14U`
```



Report fault detection counter 


        

<hr>



### define SYN\_UDS\_DTC\_REPORT\_FIRST\_CONFIRMED 

```C++
#define SYN_UDS_DTC_REPORT_FIRST_CONFIRMED `0x0CU`
```



Report first confirmed DTC 


        

<hr>



### define SYN\_UDS\_DTC\_REPORT\_FIRST\_TEST\_FAILED 

```C++
#define SYN_UDS_DTC_REPORT_FIRST_TEST_FAILED `0x0BU`
```



Report first test failed DTC 


        

<hr>



### define SYN\_UDS\_DTC\_REPORT\_MIRROR\_MEMORY\_BY\_STATUS\_MASK 

```C++
#define SYN_UDS_DTC_REPORT_MIRROR_MEMORY_BY_STATUS_MASK `0x0FU`
```



Report mirror memory by status mask 


        

<hr>



### define SYN\_UDS\_DTC\_REPORT\_MIRROR\_MEMORY\_EXT\_DATA 

```C++
#define SYN_UDS_DTC_REPORT_MIRROR_MEMORY_EXT_DATA `0x10U`
```



Report mirror memory extended data 


        

<hr>



### define SYN\_UDS\_DTC\_REPORT\_MOST\_RECENT\_CONFIRMED 

```C++
#define SYN_UDS_DTC_REPORT_MOST_RECENT_CONFIRMED `0x0EU`
```



Report most recent confirmed DTC 


        

<hr>



### define SYN\_UDS\_DTC\_REPORT\_MOST\_RECENT\_TEST\_FAILED 

```C++
#define SYN_UDS_DTC_REPORT_MOST_RECENT_TEST_FAILED `0x0DU`
```



Report most recent test failed DTC \ 


        

<hr>



### define SYN\_UDS\_DTC\_REPORT\_NUMBER\_BY\_SEVERITY\_MASK 

```C++
#define SYN_UDS_DTC_REPORT_NUMBER_BY_SEVERITY_MASK `0x07U`
```



Report number of DTCs by severity mask 


        

<hr>



### define SYN\_UDS\_DTC\_REPORT\_NUMBER\_BY\_STATUS\_MASK 

```C++
#define SYN_UDS_DTC_REPORT_NUMBER_BY_STATUS_MASK `0x01U`
```



Report number of DTCs by status mask \ 


        

<hr>



### define SYN\_UDS\_DTC\_REPORT\_NUMBER\_EMISSIONS\_OBD\_BY\_STATUS\_MASK 

```C++
#define SYN_UDS_DTC_REPORT_NUMBER_EMISSIONS_OBD_BY_STATUS_MASK `0x12U`
```



Report number of OBD DTCs 


        

<hr>



### define SYN\_UDS\_DTC\_REPORT\_NUMBER\_MIRROR\_MEMORY\_BY\_STATUS\_MASK 

```C++
#define SYN_UDS_DTC_REPORT_NUMBER_MIRROR_MEMORY_BY_STATUS_MASK `0x11U`
```



Report number of mirror memory DTCs 


        

<hr>



### define SYN\_UDS\_DTC\_REPORT\_SEVERITY\_INFO 

```C++
#define SYN_UDS_DTC_REPORT_SEVERITY_INFO `0x09U`
```



Report DTC severity info 


        

<hr>



### define SYN\_UDS\_DTC\_REPORT\_SNAPSHOT\_IDENTIFICATION 

```C++
#define SYN_UDS_DTC_REPORT_SNAPSHOT_IDENTIFICATION `0x03U`
```



Report DTC snapshot identification \ 


        

<hr>



### define SYN\_UDS\_DTC\_REPORT\_SNAPSHOT\_RECORD\_BY\_DTC 

```C++
#define SYN_UDS_DTC_REPORT_SNAPSHOT_RECORD_BY_DTC `0x04U`
```



Report DTC snapshot record by DTC 


        

<hr>



### define SYN\_UDS\_DTC\_REPORT\_STORED\_DATA\_BY\_RECORD\_NUM 

```C++
#define SYN_UDS_DTC_REPORT_STORED_DATA_BY_RECORD_NUM `0x05U`
```



Report stored data by record number 


        

<hr>



### define SYN\_UDS\_DTC\_REPORT\_SUPPORTED 

```C++
#define SYN_UDS_DTC_REPORT_SUPPORTED `0x0AU`
```



Report supported DTCs 


        

<hr>



### define SYN\_UDS\_DTC\_REPORT\_USER\_DEF\_MEMORY\_BY\_STATUS\_MASK 

```C++
#define SYN_UDS_DTC_REPORT_USER_DEF_MEMORY_BY_STATUS_MASK `0x17U`
```



Report user-def memory by status mask 


        

<hr>



### define SYN\_UDS\_DTC\_REPORT\_USER\_DEF\_MEMORY\_EXT\_DATA\_BY\_DTC 

```C++
#define SYN_UDS_DTC_REPORT_USER_DEF_MEMORY_EXT_DATA_BY_DTC `0x19U`
```



Report user-def ext data by DTC 


        

<hr>



### define SYN\_UDS\_DTC\_REPORT\_USER\_DEF\_MEMORY\_SNAPSHOT\_BY\_DTC 

```C++
#define SYN_UDS_DTC_REPORT_USER_DEF_MEMORY_SNAPSHOT_BY_DTC `0x18U`
```



Report user-def snapshot by DTC 


        

<hr>



### define SYN\_UDS\_DTC\_REPORT\_WITH\_PERMANENT\_STATUS 

```C++
#define SYN_UDS_DTC_REPORT_WITH_PERMANENT_STATUS `0x15U`
```



Report DTCs with permanent status 


        

<hr>



### define SYN\_UDS\_DTC\_REPORT\_WWH\_OBD\_BY\_MASK\_RECORD 

```C++
#define SYN_UDS_DTC_REPORT_WWH_OBD_BY_MASK_RECORD `0x42U`
```



Report WWH-OBD by mask record 


        

<hr>



### define SYN\_UDS\_DTC\_REPORT\_WWH\_OBD\_WITH\_PERMANENT\_STATUS 

```C++
#define SYN_UDS_DTC_REPORT_WWH_OBD_WITH_PERMANENT_STATUS `0x55U`
```



Report WWH-OBD with permanent status 


        

<hr>



### define SYN\_UDS\_DTC\_SEVERITY\_CHECK\_AT\_NEXT\_HALT 

```C++
#define SYN_UDS_DTC_SEVERITY_CHECK_AT_NEXT_HALT `(0x02U << 5)`
```



Check at next halt 


        

<hr>



### define SYN\_UDS\_DTC\_SEVERITY\_CHECK\_IMMEDIATELY 

```C++
#define SYN_UDS_DTC_SEVERITY_CHECK_IMMEDIATELY `(0x03U << 5)`
```



Check immediately 


        

<hr>



### define SYN\_UDS\_DTC\_SEVERITY\_MAINTENANCE\_REQUIRED 

```C++
#define SYN_UDS_DTC_SEVERITY_MAINTENANCE_REQUIRED `(0x01U << 5)`
```



Maintenance required 


        

<hr>



### define SYN\_UDS\_DTC\_SEVERITY\_MASK 

```C++
#define SYN_UDS_DTC_SEVERITY_MASK `(0x07U << 5)`
```



DTC severity mask 


        

<hr>



### define SYN\_UDS\_DTC\_SEVERITY\_NO\_SEVERITY 

```C++
#define SYN_UDS_DTC_SEVERITY_NO_SEVERITY `(0x00U << 5)`
```



No severity 


        

<hr>



### define SYN\_UDS\_DTC\_STATUS\_AVAILABILITY\_MASK 

```C++
#define SYN_UDS_DTC_STATUS_AVAILABILITY_MASK `0xFFU`
```



DTC status availability bitmask 


        

<hr>



### define SYN\_UDS\_DTC\_STATUS\_CONFIRMED\_DTC 

```C++
#define SYN_UDS_DTC_STATUS_CONFIRMED_DTC `(1U << 3)`
```



Confirmed DTC bit 


        

<hr>



### define SYN\_UDS\_DTC\_STATUS\_PENDING\_DTC 

```C++
#define SYN_UDS_DTC_STATUS_PENDING_DTC `(1U << 2)`
```



Pending DTC bit 


        

<hr>



### define SYN\_UDS\_DTC\_STATUS\_TEST\_FAILED 

```C++
#define SYN_UDS_DTC_STATUS_TEST_FAILED `(1U << 0)`
```



Test failed bit 


        

<hr>



### define SYN\_UDS\_DTC\_STATUS\_TEST\_FAILED\_SINCE\_LAST\_CLEAR 

```C++
#define SYN_UDS_DTC_STATUS_TEST_FAILED_SINCE_LAST_CLEAR `(1U << 5)`
```



Test failed since clear 


        

<hr>



### define SYN\_UDS\_DTC\_STATUS\_TEST\_FAILED\_THIS\_OP\_CYCLE 

```C++
#define SYN_UDS_DTC_STATUS_TEST_FAILED_THIS_OP_CYCLE `(1U << 1)`
```



Test failed this operation cycle 


        

<hr>



### define SYN\_UDS\_DTC\_STATUS\_TEST\_NOT\_COMPLETED\_SINCE\_LAST\_CLEAR 

```C++
#define SYN_UDS_DTC_STATUS_TEST_NOT_COMPLETED_SINCE_LAST_CLEAR `(1U << 4)`
```



Test not completed since clear 


        

<hr>



### define SYN\_UDS\_DTC\_STATUS\_TEST\_NOT\_COMPLETED\_THIS\_OP\_CYCLE 

```C++
#define SYN_UDS_DTC_STATUS_TEST_NOT_COMPLETED_THIS_OP_CYCLE `(1U << 6)`
```



Test not completed this op cycle 


        

<hr>



### define SYN\_UDS\_DTC\_STATUS\_WARNING\_INDICATOR\_REQUESTED 

```C++
#define SYN_UDS_DTC_STATUS_WARNING_INDICATOR_REQUESTED `(1U << 7)`
```



Warning indicator requested \ 


        

<hr>



### define SYN\_UDS\_MAX\_DIDS 

```C++
#define SYN_UDS_MAX_DIDS `16U`
```



Maximum supported DIDs 


        

<hr>



### define SYN\_UDS\_MAX\_DTCS 

```C++
#define SYN_UDS_MAX_DTCS `32U`
```



Maximum stored DTC count 


        

<hr>



### define SYN\_UDS\_MAX\_SERVICE\_OVERRIDES 

```C++
#define SYN_UDS_MAX_SERVICE_OVERRIDES `8U`
```



Maximum supported per-service policy overrides (default: 8) 


        

<hr>



### define SYN\_UDS\_NRC\_CONDITIONS\_NOT\_CORRECT 

```C++
#define SYN_UDS_NRC_CONDITIONS_NOT_CORRECT `0x22U`
```



Conditions not correct (0x22) 


        

<hr>



### define SYN\_UDS\_NRC\_EXCEEDED\_NUMBER\_OF\_ATTEMPTS 

```C++
#define SYN_UDS_NRC_EXCEEDED_NUMBER_OF_ATTEMPTS `0x36U`
```



Exceeded number of attempts (0x36) 


        

<hr>



### define SYN\_UDS\_NRC\_GENERAL\_PROGRAMMING\_FAILURE 

```C++
#define SYN_UDS_NRC_GENERAL_PROGRAMMING_FAILURE `0x72U`
```



General programming failure (0x72) 


        

<hr>



### define SYN\_UDS\_NRC\_INCORRECT\_MESSAGE\_LENGTH 

```C++
#define SYN_UDS_NRC_INCORRECT_MESSAGE_LENGTH `0x13U`
```



Incorrect message length or invalid format (0x13) 


        

<hr>



### define SYN\_UDS\_NRC\_INVALID\_KEY 

```C++
#define SYN_UDS_NRC_INVALID_KEY `0x35U`
```



Invalid key (0x35) 


        

<hr>



### define SYN\_UDS\_NRC\_REQUEST\_OUT\_OF\_RANGE 

```C++
#define SYN_UDS_NRC_REQUEST_OUT_OF_RANGE `0x31U`
```



Request out of range (0x31) 


        

<hr>



### define SYN\_UDS\_NRC\_REQUEST\_SEQUENCE\_ERROR 

```C++
#define SYN_UDS_NRC_REQUEST_SEQUENCE_ERROR `0x24U`
```



Request sequence error (0x24) 


        

<hr>



### define SYN\_UDS\_NRC\_REQUIRED\_TIME\_DELAY\_NOT\_EXPIRED 

```C++
#define SYN_UDS_NRC_REQUIRED_TIME_DELAY_NOT_EXPIRED `0x37U`
```



Required time delay not expired (0x37) 


        

<hr>



### define SYN\_UDS\_NRC\_RESPONSE\_TOO\_LONG 

```C++
#define SYN_UDS_NRC_RESPONSE_TOO_LONG `0x14U`
```



Response too long (0x14) 


        

<hr>



### define SYN\_UDS\_NRC\_SECURITY\_ACCESS\_DENIED 

```C++
#define SYN_UDS_NRC_SECURITY_ACCESS_DENIED `0x33U`
```



Security access denied (0x33) 


        

<hr>



### define SYN\_UDS\_NRC\_SERVICE\_NOT\_SUPPORTED 

```C++
#define SYN_UDS_NRC_SERVICE_NOT_SUPPORTED `0x11U`
```



Service not supported (0x11) 


        

<hr>



### define SYN\_UDS\_NRC\_SUBFUNCTION\_NOT\_SUPPORTED 

```C++
#define SYN_UDS_NRC_SUBFUNCTION_NOT_SUPPORTED `0x12U`
```



Sub-function not supported (0x12) 


        

<hr>



### define SYN\_UDS\_NRC\_SUBFUNCTION\_NOT\_SUPPORTED\_IN\_ACTIVE\_SESSION 

```C++
#define SYN_UDS_NRC_SUBFUNCTION_NOT_SUPPORTED_IN_ACTIVE_SESSION `0x7EU`
```



Sub-function not supported in active session (0x7E) 


        

<hr>



### define SYN\_UDS\_NRC\_SUCCESS 

```C++
#define SYN_UDS_NRC_SUCCESS `0x00U`
```



Positive response (0x00) 


        

<hr>



### define SYN\_UDS\_NRC\_TRANSFER\_DATA\_SUSPENDED 

```C++
#define SYN_UDS_NRC_TRANSFER_DATA_SUSPENDED `0x71U`
```



Transfer data suspended (0x71) 


        

<hr>



### define SYN\_UDS\_NRC\_WRONG\_BLOCK\_SEQUENCE\_COUNTER 

```C++
#define SYN_UDS_NRC_WRONG_BLOCK_SEQUENCE_COUNTER `0x73U`
```



Wrong block sequence counter (0x73) 


        

<hr>



### define SYN\_UDS\_RESET\_DISABLE\_RAPID\_POWER\_SHUTDOWN 

```C++
#define SYN_UDS_RESET_DISABLE_RAPID_POWER_SHUTDOWN `0x05U`
```



Disable rapid power shutdown 


        

<hr>



### define SYN\_UDS\_RESET\_ENABLE\_RAPID\_POWER\_SHUTDOWN 

```C++
#define SYN_UDS_RESET_ENABLE_RAPID_POWER_SHUTDOWN `0x04U`
```



Enable rapid power shutdown 


        

<hr>



### define SYN\_UDS\_RESET\_HARD 

```C++
#define SYN_UDS_RESET_HARD `0x01U`
```



Hard reset 


        

<hr>



### define SYN\_UDS\_RESET\_KEY\_OFF\_ON 

```C++
#define SYN_UDS_RESET_KEY_OFF_ON `0x02U`
```



Key off/on reset 


        

<hr>



### define SYN\_UDS\_RESET\_SOFT 

```C++
#define SYN_UDS_RESET_SOFT `0x03U`
```



Soft reset 


        

<hr>



### define SYN\_UDS\_RESPONSE\_NEGATIVE 

```C++
#define SYN_UDS_RESPONSE_NEGATIVE `0x7FU`
```



Negative response code header 


        

<hr>



### define SYN\_UDS\_S3\_TIMEOUT\_MS 

```C++
#define SYN_UDS_S3_TIMEOUT_MS `5000U`
```



S3 session timeout in ms 


        

<hr>



### define SYN\_UDS\_SECURITY\_DELAY\_MS 

```C++
#define SYN_UDS_SECURITY_DELAY_MS `10000U`
```



Security delay penalty in ms 


        

<hr>



### define SYN\_UDS\_SECURITY\_MASK\_ALL 

```C++
#define SYN_UDS_SECURITY_MASK_ALL `(0xFFFFU)`
```



Allowed in all security states 


        

<hr>



### define SYN\_UDS\_SECURITY\_MASK\_LEVEL\_1 

```C++
#define SYN_UDS_SECURITY_MASK_LEVEL_1 `(1U << 1)`
```



Security Level 1 required 


        

<hr>



### define SYN\_UDS\_SECURITY\_MASK\_LEVEL\_2 

```C++
#define SYN_UDS_SECURITY_MASK_LEVEL_2 `(1U << 2)`
```



Security Level 2 required 


        

<hr>



### define SYN\_UDS\_SECURITY\_MASK\_LEVEL\_3 

```C++
#define SYN_UDS_SECURITY_MASK_LEVEL_3 `(1U << 3)`
```



Security Level 3 required 


        

<hr>



### define SYN\_UDS\_SECURITY\_MASK\_NONE 

```C++
#define SYN_UDS_SECURITY_MASK_NONE `(1U << 0)`
```



Unlocked without security (Level 0 / Locked) 


        

<hr>



### define SYN\_UDS\_SECURITY\_MAX\_ATTEMPTS 

```C++
#define SYN_UDS_SECURITY_MAX_ATTEMPTS `3U`
```



Max security unlock attempts 


        

<hr>



### define SYN\_UDS\_SESSION\_MASK\_ALL 

```C++
#define SYN_UDS_SESSION_MASK_ALL `(0x0FU)`
```



Allowed in all sessions 


        

<hr>



### define SYN\_UDS\_SESSION\_MASK\_DEFAULT 

```C++
#define SYN_UDS_SESSION_MASK_DEFAULT `(1U << 0)`
```



Default Session 


        

<hr>



### define SYN\_UDS\_SESSION\_MASK\_EXTENDED 

```C++
#define SYN_UDS_SESSION_MASK_EXTENDED `(1U << 2)`
```



Extended Diagnostic Session 


        

<hr>



### define SYN\_UDS\_SESSION\_MASK\_PROGRAMMING 

```C++
#define SYN_UDS_SESSION_MASK_PROGRAMMING `(1U << 1)`
```



Programming Session 


        

<hr>



### define SYN\_UDS\_SESSION\_MASK\_SAFETY 

```C++
#define SYN_UDS_SESSION_MASK_SAFETY `(1U << 3)`
```



Safety System Session 


        

<hr>



### define SYN\_UDS\_SID\_ACCESS\_TIMING\_PARAMETER 

```C++
#define SYN_UDS_SID_ACCESS_TIMING_PARAMETER `0x83U`
```



AccessTimingParameter 


        

<hr>



### define SYN\_UDS\_SID\_AUTHENTICATION 

```C++
#define SYN_UDS_SID_AUTHENTICATION `0x29U`
```



Authentication 


        

<hr>



### define SYN\_UDS\_SID\_CLEAR\_DIAGNOSTIC\_INFORMATION 

```C++
#define SYN_UDS_SID_CLEAR_DIAGNOSTIC_INFORMATION `0x14U`
```



ClearDiagnosticInformation 


        

<hr>



### define SYN\_UDS\_SID\_COMMUNICATION\_CONTROL 

```C++
#define SYN_UDS_SID_COMMUNICATION_CONTROL `0x28U`
```



CommunicationControl 


        

<hr>



### define SYN\_UDS\_SID\_CONTROL\_DTC\_SETTING 

```C++
#define SYN_UDS_SID_CONTROL_DTC_SETTING `0x85U`
```



ControlDTCSetting 


        

<hr>



### define SYN\_UDS\_SID\_DIAGNOSTIC\_SESSION\_CONTROL 

```C++
#define SYN_UDS_SID_DIAGNOSTIC_SESSION_CONTROL `0x10U`
```



DiagnosticSessionControl 


        

<hr>



### define SYN\_UDS\_SID\_DYNAMICALLY\_DEFINE\_DATA\_IDENTIFIER 

```C++
#define SYN_UDS_SID_DYNAMICALLY_DEFINE_DATA_IDENTIFIER `0x2CU`
```



DynamicallyDefineDataIdentifier \ 


        

<hr>



### define SYN\_UDS\_SID\_ECU\_RESET 

```C++
#define SYN_UDS_SID_ECU_RESET `0x11U`
```



ECUReset 


        

<hr>



### define SYN\_UDS\_SID\_INPUT\_OUTPUT\_CONTROL\_BY\_IDENTIFIER 

```C++
#define SYN_UDS_SID_INPUT_OUTPUT_CONTROL_BY_IDENTIFIER `0x2FU`
```



InputOutputControlByIdentifier \ 


        

<hr>



### define SYN\_UDS\_SID\_LINK\_CONTROL 

```C++
#define SYN_UDS_SID_LINK_CONTROL `0x87U`
```



LinkControl 


        

<hr>



### define SYN\_UDS\_SID\_READ\_DATA\_BY\_IDENTIFIER 

```C++
#define SYN_UDS_SID_READ_DATA_BY_IDENTIFIER `0x22U`
```



ReadDataByIdentifier 


        

<hr>



### define SYN\_UDS\_SID\_READ\_DATA\_BY\_PERIODIC\_IDENTIFIER 

```C++
#define SYN_UDS_SID_READ_DATA_BY_PERIODIC_IDENTIFIER `0x2AU`
```



ReadDataByPeriodicIdentifier 


        

<hr>



### define SYN\_UDS\_SID\_READ\_DTC\_INFORMATION 

```C++
#define SYN_UDS_SID_READ_DTC_INFORMATION `0x19U`
```



ReadDTCInformation 


        

<hr>



### define SYN\_UDS\_SID\_READ\_MEMORY\_BY\_ADDRESS 

```C++
#define SYN_UDS_SID_READ_MEMORY_BY_ADDRESS `0x23U`
```



ReadMemoryByAddress 


        

<hr>



### define SYN\_UDS\_SID\_READ\_SCALING\_DATA\_BY\_IDENTIFIER 

```C++
#define SYN_UDS_SID_READ_SCALING_DATA_BY_IDENTIFIER `0x24U`
```



ReadScalingDataByIdentifier 


        

<hr>



### define SYN\_UDS\_SID\_REQUEST\_DOWNLOAD 

```C++
#define SYN_UDS_SID_REQUEST_DOWNLOAD `0x34U`
```



RequestDownload 


        

<hr>



### define SYN\_UDS\_SID\_REQUEST\_FILE\_TRANSFER 

```C++
#define SYN_UDS_SID_REQUEST_FILE_TRANSFER `0x38U`
```



RequestFileTransfer 


        

<hr>



### define SYN\_UDS\_SID\_REQUEST\_TRANSFER\_EXIT 

```C++
#define SYN_UDS_SID_REQUEST_TRANSFER_EXIT `0x37U`
```



RequestTransferExit 


        

<hr>



### define SYN\_UDS\_SID\_REQUEST\_UPLOAD 

```C++
#define SYN_UDS_SID_REQUEST_UPLOAD `0x35U`
```



RequestUpload 


        

<hr>



### define SYN\_UDS\_SID\_RESPONSE\_ON\_EVENT 

```C++
#define SYN_UDS_SID_RESPONSE_ON_EVENT `0x86U`
```



ResponseOnEvent 


        

<hr>



### define SYN\_UDS\_SID\_ROUTINE\_CONTROL 

```C++
#define SYN_UDS_SID_ROUTINE_CONTROL `0x31U`
```



RoutineControl 


        

<hr>



### define SYN\_UDS\_SID\_SECURED\_DATA\_TRANSMISSION 

```C++
#define SYN_UDS_SID_SECURED_DATA_TRANSMISSION `0x84U`
```



SecuredDataTransmission 


        

<hr>



### define SYN\_UDS\_SID\_SECURITY\_ACCESS 

```C++
#define SYN_UDS_SID_SECURITY_ACCESS `0x27U`
```



SecurityAccess 


        

<hr>



### define SYN\_UDS\_SID\_TESTER\_PRESENT 

```C++
#define SYN_UDS_SID_TESTER_PRESENT `0x3EU`
```



TesterPresent 


        

<hr>



### define SYN\_UDS\_SID\_TRANSFER\_DATA 

```C++
#define SYN_UDS_SID_TRANSFER_DATA `0x36U`
```



TransferData 


        

<hr>



### define SYN\_UDS\_SID\_WRITE\_DATA\_BY\_IDENTIFIER 

```C++
#define SYN_UDS_SID_WRITE_DATA_BY_IDENTIFIER `0x2EU`
```



WriteDataByIdentifier 


        

<hr>



### define SYN\_UDS\_SID\_WRITE\_MEMORY\_BY\_ADDRESS 

```C++
#define SYN_UDS_SID_WRITE_MEMORY_BY_ADDRESS `0x3DU`
```



WriteMemoryByAddress 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_uds.h`

