

# File syn\_uds.c



[**FileList**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_uds.c**](syn__uds_8c.md)

[Go to the source code of this file](syn__uds_8c_source.md)

_ISO 14229 Unified Diagnostic Services (UDS) Server Implementation._ 

* `#include "syn_uds.h"`
* `#include "../crypto/syn_aes.h"`
* `#include "../util/syn_pack.h"`
* `#include <string.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**UDS\_ReqCtx**](structUDS__ReqCtx.md) <br>_Internal UDS request processing context._  |
| struct | [**UDS\_ServiceEntry**](structUDS__ServiceEntry.md) <br>_UDS service dispatch table entry._  |


## Public Types

| Type | Name |
| ---: | :--- |
| typedef [**bool**](syn__defs_8h.md#enum-bool)(\* | [**UDS\_ServiceFn**](#typedef-uds_servicefn)  <br>_UDS service handler function signature._  |






## Public Static Attributes

| Type | Name |
| ---: | :--- |
|  const [**UDS\_ServiceEntry**](structUDS__ServiceEntry.md) | [**g\_uds\_services**](#variable-g_uds_services)   = `/* multi line expression */`<br>_Static UDS service identifier to handler dispatch table._  |














## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_uds\_clear\_pending\_reset**](#function-syn_uds_clear_pending_reset) ([**SYN\_UDS\_Server**](structSYN__UDS__Server.md) \* server) <br>_Clear pending ECU reset flag after executing reset or transmitting response._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_uds\_disable\_aes128\_security**](#function-syn_uds_disable_aes128_security) ([**SYN\_UDS\_Server**](structSYN__UDS__Server.md) \* server) <br>_Disable AES-128 security mode and revert to standard XOR security key calculation._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_uds\_dtc\_get\_status**](#function-syn_uds_dtc_get_status) ([**SYN\_UDS\_Server**](structSYN__UDS__Server.md) \* server, uint32\_t dtc, uint8\_t \* out\_status) <br>_Get current 8-bit status byte for a registered DTC._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_uds\_dtc\_report\_test\_result**](#function-syn_uds_dtc_report_test_result) ([**SYN\_UDS\_Server**](structSYN__UDS__Server.md) \* server, uint32\_t dtc, [**bool**](syn__defs_8h.md#enum-bool) failed) <br>_Report diagnostic test result for a registered DTC according to ISO 14229-1 state transitions._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_uds\_dtc\_start\_operation\_cycle**](#function-syn_uds_dtc_start_operation_cycle) ([**SYN\_UDS\_Server**](structSYN__UDS__Server.md) \* server) <br>_Advance server state to a new operation cycle._  |
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


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  int | [**find\_did\_index**](#function-find_did_index) (const [**SYN\_UDS\_Server**](structSYN__UDS__Server.md) \* server, uint16\_t did) <br>_Find the table index of a registered 16-bit Data Identifier (DID)._  |
|  int | [**find\_dtc\_index**](#function-find_dtc_index) (const [**SYN\_UDS\_Server**](structSYN__UDS__Server.md) \* server, uint32\_t dtc) <br>_Find the table index of a registered 24-bit Diagnostic Trouble Code (DTC)._  |
|  uint16\_t | [**get\_active\_security\_mask**](#function-get_active_security_mask) (const [**SYN\_UDS\_Server**](structSYN__UDS__Server.md) \* server) <br>_Compute the bitmask of currently satisfied security levels on the UDS server._  |
|  uint8\_t | [**get\_sid\_allowed\_session\_mask**](#function-get_sid_allowed_session_mask) (const [**SYN\_UDS\_Server**](structSYN__UDS__Server.md) \* server, uint8\_t sid) <br>_Look up the permitted session bitmask configured for a UDS service identifier._  |
|  uint16\_t | [**get\_sid\_required\_security\_mask**](#function-get_sid_required_security_mask) (const [**SYN\_UDS\_Server**](structSYN__UDS__Server.md) \* server, uint8\_t sid) <br>_Look up the required security level bitmask configured for a UDS service identifier._  |
|  uint8\_t | [**session\_to\_mask**](#function-session_to_mask) ([**SYN\_UDS\_Session**](syn__uds_8h.md#enum-syn_uds_session) session) <br>_Convert a UDS diagnostic session enum into its single-bit permission mask._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**uds\_check\_preconditions**](#function-uds_check_preconditions) ([**UDS\_ReqCtx**](structUDS__ReqCtx.md) \* ctx) <br>_Validate functional addressing support, session mask, and security mask for a request._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**uds\_commit\_var\_resp**](#function-uds_commit_var_resp) ([**UDS\_ReqCtx**](structUDS__ReqCtx.md) \* ctx, uint16\_t hdr\_len, uint16\_t payload\_len) <br>_Validate variable callback payload length and finalize positive UDS response._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**uds\_decode\_be\_var**](#function-uds_decode_be_var) (const uint8\_t \* buf, uint8\_t len, uint32\_t \* out\_val) <br>_Decode a big-endian UDS address or size parameter up to 32-bit range._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**uds\_dtc\_count\_by\_mask**](#function-uds_dtc_count_by_mask) ([**UDS\_ReqCtx**](structUDS__ReqCtx.md) \* ctx, uint8\_t sub) <br>_Encode matching DTC count for ReadDTCInformation count-by-status-mask sub-functions._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**uds\_dtc\_first\_or\_recent**](#function-uds_dtc_first_or_recent) ([**UDS\_ReqCtx**](structUDS__ReqCtx.md) \* ctx, uint8\_t sub) <br>_Handle ReadDTCInformation sub-functions 0x0B..0x0E (first/most recent failed/confirmed)._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**uds\_dtc\_in\_group**](#function-uds_dtc_in_group) (uint32\_t dtc, uint32\_t group) <br>_Check whether a 24-bit DTC belongs to the specified DTC group or exact DTC value._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**uds\_dtc\_info\_by\_dtc**](#function-uds_dtc_info_by_dtc) ([**UDS\_ReqCtx**](structUDS__ReqCtx.md) \* ctx, uint8\_t sub) <br>_Handle ReadDTCInformation severity-by-DTC (0x09) and fault detection counter (0x14)._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**uds\_dtc\_list\_by\_mask**](#function-uds_dtc_list_by_mask) ([**UDS\_ReqCtx**](structUDS__ReqCtx.md) \* ctx, uint8\_t sub, uint8\_t mask, [**bool**](syn__defs_8h.md#enum-bool) match\_all) <br>_Encode a list of 24-bit DTCs and status bytes filtered by status mask._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**uds\_dtc\_misc\_records**](#function-uds_dtc_misc_records) ([**UDS\_ReqCtx**](structUDS__ReqCtx.md) \* ctx, uint8\_t sub) <br>_Handle ReadDTCInformation snapshot ID, record-number, user-memory, and WWH-OBD queries._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**uds\_dtc\_records**](#function-uds_dtc_records) ([**UDS\_ReqCtx**](structUDS__ReqCtx.md) \* ctx, uint8\_t sub) <br>_Handle ReadDTCInformation snapshot and extended-data record queries by DTC._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**uds\_dtc\_severity\_and\_fdc**](#function-uds_dtc_severity_and_fdc) ([**UDS\_ReqCtx**](structUDS__ReqCtx.md) \* ctx, uint8\_t sub) <br>_Handle ReadDTCInformation severity-mask queries (0x07, 0x08)._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**uds\_dtc\_status\_queries**](#function-uds_dtc_status_queries) ([**UDS\_ReqCtx**](structUDS__ReqCtx.md) \* ctx, uint8\_t sub) <br>_Dispatch ReadDTCInformation status-mask, supported-DTC, and permanent-DTC sub-functions._  |
|  void | [**uds\_finalize\_response**](#function-uds_finalize_response) ([**UDS\_ReqCtx**](structUDS__ReqCtx.md) \* ctx) <br>_Reload non-default S3 session timer and apply positive-response suppression bit._  |
|  [**UDS\_ServiceFn**](syn__uds_8c.md#typedef-uds_servicefn) | [**uds\_find\_service**](#function-uds_find_service) (uint8\_t sid) <br>_Look up the UDS service handler function registered for a Service Identifier._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**uds\_is\_standard\_dtc\_group**](#function-uds_is_standard_dtc_group) (uint32\_t group) <br>_Check whether a 24-bit group identifier matches an ISO 14229-1 standard DTC group._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**uds\_nrc**](#function-uds_nrc) ([**UDS\_ReqCtx**](structUDS__ReqCtx.md) \* ctx, uint8\_t nrc) <br>_Encode a UDS Negative Response Code (0x7F) or suppress it per functional addressing rules._  |
|  void | [**uds\_pack\_dtc24**](#function-uds_pack_dtc24) (uint8\_t \* buf, uint16\_t pos, uint32\_t dtc) <br>_Pack a 24-bit DTC value in big-endian byte order into an output buffer._  |
|  void | [**uds\_reset\_default\_session\_state**](#function-uds_reset_default_session_state) ([**SYN\_UDS\_Server**](structSYN__UDS__Server.md) \* server) <br>_Reset session, security, communication, DTC setting, and transfer state to default._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**uds\_security\_fail**](#function-uds_security_fail) ([**UDS\_ReqCtx**](structUDS__ReqCtx.md) \* ctx) <br>_Record a failed SecurityAccess key attempt, arm lockout delay if needed, and emit NRC._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**uds\_security\_send\_key**](#function-uds_security_send_key) ([**UDS\_ReqCtx**](structUDS__ReqCtx.md) \* ctx, uint8\_t sub) <br>_Verify SecurityAccess sendKey (even sub-function) payload and unlock security level._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**uds\_sid\_access\_timing**](#function-uds_sid_access_timing) ([**UDS\_ReqCtx**](structUDS__ReqCtx.md) \* ctx) <br>_Handle AccessTimingParameter (0x83) read and set sub-functions._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**uds\_sid\_authentication**](#function-uds_sid_authentication) ([**UDS\_ReqCtx**](structUDS__ReqCtx.md) \* ctx) <br>_Handle Authentication (0x29) request._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**uds\_sid\_clear\_dtc**](#function-uds_sid_clear_dtc) ([**UDS\_ReqCtx**](structUDS__ReqCtx.md) \* ctx) <br>_Handle ClearDiagnosticInformation (0x14) request for a DTC or DTC group._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**uds\_sid\_comm\_control**](#function-uds_sid_comm_control) ([**UDS\_ReqCtx**](structUDS__ReqCtx.md) \* ctx) <br>_Handle CommunicationControl (0x28) request._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**uds\_sid\_control\_dtc\_setting**](#function-uds_sid_control_dtc_setting) ([**UDS\_ReqCtx**](structUDS__ReqCtx.md) \* ctx) <br>_Handle ControlDTCSetting (0x85) on/off request._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**uds\_sid\_dynamic\_did**](#function-uds_sid_dynamic_did) ([**UDS\_ReqCtx**](structUDS__ReqCtx.md) \* ctx) <br>_Handle DynamicallyDefineDataIdentifier (0x2C) request._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**uds\_sid\_ecu\_reset**](#function-uds_sid_ecu_reset) ([**UDS\_ReqCtx**](structUDS__ReqCtx.md) \* ctx) <br>_Handle ECUReset (0x11) request and arm deferred reset timer._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**uds\_sid\_file\_transfer**](#function-uds_sid_file_transfer) ([**UDS\_ReqCtx**](structUDS__ReqCtx.md) \* ctx) <br>_Handle RequestFileTransfer (0x38) request._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**uds\_sid\_has\_subfunction**](#function-uds_sid_has_subfunction) (uint8\_t sid) <br>_Check whether a UDS service uses a sub-function byte with suppressPosRspMsgIndicationBit._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**uds\_sid\_io\_control**](#function-uds_sid_io_control) ([**UDS\_ReqCtx**](structUDS__ReqCtx.md) \* ctx) <br>_Handle InputOutputControlByIdentifier (0x2F) request._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**uds\_sid\_link\_control**](#function-uds_sid_link_control) ([**UDS\_ReqCtx**](structUDS__ReqCtx.md) \* ctx) <br>_Handle LinkControl (0x87) baudrate verification and transition sub-functions._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**uds\_sid\_read\_did**](#function-uds_sid_read_did) ([**UDS\_ReqCtx**](structUDS__ReqCtx.md) \* ctx) <br>_Handle ReadDataByIdentifier (0x22) request for one or more DIDs._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**uds\_sid\_read\_dtc\_info**](#function-uds_sid_read_dtc_info) ([**UDS\_ReqCtx**](structUDS__ReqCtx.md) \* ctx) <br>_Handle ReadDTCInformation (0x19) request across all supported sub-functions._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**uds\_sid\_read\_memory**](#function-uds_sid_read_memory) ([**UDS\_ReqCtx**](structUDS__ReqCtx.md) \* ctx) <br>_Handle ReadMemoryByAddress (0x23) request._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**uds\_sid\_read\_periodic**](#function-uds_sid_read_periodic) ([**UDS\_ReqCtx**](structUDS__ReqCtx.md) \* ctx) <br>_Handle ReadDataByPeriodicIdentifier (0x2A) request._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**uds\_sid\_read\_scaling**](#function-uds_sid_read_scaling) ([**UDS\_ReqCtx**](structUDS__ReqCtx.md) \* ctx) <br>_Handle ReadScalingDataByIdentifier (0x24) request._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**uds\_sid\_request\_transfer**](#function-uds_sid_request_transfer) ([**UDS\_ReqCtx**](structUDS__ReqCtx.md) \* ctx) <br>_Handle RequestDownload (0x34) and RequestUpload (0x35) transfer initialization._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**uds\_sid\_roe**](#function-uds_sid_roe) ([**UDS\_ReqCtx**](structUDS__ReqCtx.md) \* ctx) <br>_Handle ResponseOnEvent (0x86) request._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**uds\_sid\_routine\_control**](#function-uds_sid_routine_control) ([**UDS\_ReqCtx**](structUDS__ReqCtx.md) \* ctx) <br>_Handle RoutineControl (0x31) start, stop, and request-results sub-functions._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**uds\_sid\_secured\_data**](#function-uds_sid_secured_data) ([**UDS\_ReqCtx**](structUDS__ReqCtx.md) \* ctx) <br>_Handle SecuredDataTransmission (0x84) request._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**uds\_sid\_security\_access**](#function-uds_sid_security_access) ([**UDS\_ReqCtx**](structUDS__ReqCtx.md) \* ctx) <br>_Handle SecurityAccess (0x27) requestSeed and sendKey sub-functions._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**uds\_sid\_session\_control**](#function-uds_sid_session_control) ([**UDS\_ReqCtx**](structUDS__ReqCtx.md) \* ctx) <br>_Handle DiagnosticSessionControl (0x10) request._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**uds\_sid\_tester\_present**](#function-uds_sid_tester_present) ([**UDS\_ReqCtx**](structUDS__ReqCtx.md) \* ctx) <br>_Handle TesterPresent (0x3E) keepalive request._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**uds\_sid\_transfer\_data**](#function-uds_sid_transfer_data) ([**UDS\_ReqCtx**](structUDS__ReqCtx.md) \* ctx) <br>_Handle TransferData (0x36) block sequence verification and download/upload dispatch._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**uds\_sid\_transfer\_exit**](#function-uds_sid_transfer_exit) ([**UDS\_ReqCtx**](structUDS__ReqCtx.md) \* ctx) <br>_Handle RequestTransferExit (0x37) request to terminate an active transfer session._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**uds\_sid\_write\_did**](#function-uds_sid_write_did) ([**UDS\_ReqCtx**](structUDS__ReqCtx.md) \* ctx) <br>_Handle WriteDataByIdentifier (0x2E) request._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**uds\_sid\_write\_memory**](#function-uds_sid_write_memory) ([**UDS\_ReqCtx**](structUDS__ReqCtx.md) \* ctx) <br>_Handle WriteMemoryByAddress (0x3D) request._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**uds\_timing\_read**](#function-uds_timing_read) ([**UDS\_ReqCtx**](structUDS__ReqCtx.md) \* ctx, [**SYN\_UDS\_AccessTimingType**](syn__uds_8h.md#enum-syn_uds_accesstimingtype) mode, uint16\_t p2, uint16\_t p2\_star) <br>_Encode P2 and P2\* timing parameters for AccessTimingParameter read sub-functions._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**uds\_timing\_set\_default**](#function-uds_timing_set_default) ([**UDS\_ReqCtx**](structUDS__ReqCtx.md) \* ctx) <br>_Handle AccessTimingParameter setTimingParametersToDefaultValues (0x83 0x02)._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**uds\_transfer\_download**](#function-uds_transfer_download) ([**UDS\_ReqCtx**](structUDS__ReqCtx.md) \* ctx, uint8\_t block\_seq) <br>_Process one TransferData (0x36) block write during an active download session._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**uds\_transfer\_upload**](#function-uds_transfer_upload) ([**UDS\_ReqCtx**](structUDS__ReqCtx.md) \* ctx, uint8\_t block\_seq) <br>_Process one TransferData (0x36) block read during an active upload session._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**uds\_transfer\_upload\_chunk**](#function-uds_transfer_upload_chunk) ([**UDS\_ReqCtx**](structUDS__ReqCtx.md) \* ctx, uint8\_t block\_seq, uint32\_t offset, uint16\_t chunk\_len) <br>_Encode one TransferData (0x36) upload response block from server memory._  |


























## Public Types Documentation




### typedef UDS\_ServiceFn 

_UDS service handler function signature._ 
```C++
typedef bool(* UDS_ServiceFn) (UDS_ReqCtx *ctx);
```




<hr>
## Public Static Attributes Documentation




### variable g\_uds\_services 

_Static UDS service identifier to handler dispatch table._ 
```C++
const UDS_ServiceEntry g_uds_services[];
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



Returns the authoritative ISO 14229-1 DTCStatusByte tracked by the UDS server.




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



Diagnostic monitors must invoke this function whenever a periodic or event-driven test completes:
* When `failed` is true (qualified failure):
  * Sets `testFailed` (bit 0), `testFailedThisOperationCycle` (bit 1), `pendingDTC` (bit 2), `confirmedDTC` (bit 3), and `testFailedSinceLastClear` (bit 5).
  * Clears `testNotCompletedSinceLastClear` (bit 4) and `testNotCompletedThisOperationCycle` (bit 6).
  * Sets Fault Detection Counter (FDC) to +127.


* When `failed` is false (qualified pass):
  * Clears `testFailed` (bit 0), `testNotCompletedSinceLastClear` (bit 4), and `testNotCompletedThisOperationCycle` (bit 6).
  * Sets Fault Detection Counter (FDC) to -128.








**Parameters:**


* `server` Pointer to UDS server instance. 
* `dtc` 24-bit Diagnostic Trouble Code. 
* `failed` True if test qualified failed, false if test qualified passed. 



**Returns:**

true if DTC found and updated, false otherwise. 





        

<hr>



### function syn\_uds\_dtc\_start\_operation\_cycle 

_Advance server state to a new operation cycle._ 
```C++
bool syn_uds_dtc_start_operation_cycle (
    SYN_UDS_Server * server
) 
```



Must be invoked at ECU ignition ON / startup and upon completion/boundary of each driving or operation cycle:
* If a registered DTC completed testing during the previous cycle without any failure (`testNotCompletedThisOperationCycle` == 0 and `testFailedThisOperationCycle` == 0), clears `pendingDTC` (bit 2) per ISO 14229-1 Table 242.
* Clears `testFailedThisOperationCycle` (bit 1) to 0.
* Sets `testNotCompletedThisOperationCycle` (bit 6) to 1.






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
## Public Static Functions Documentation




### function find\_did\_index 

_Find the table index of a registered 16-bit Data Identifier (DID)._ 
```C++
static int find_did_index (
    const SYN_UDS_Server * server,
    uint16_t did
) 
```





**Parameters:**


* `server` UDS server instance. 
* `did` 16-bit Data Identifier to look up. 



**Returns:**

Zero-based table index on match, or -1 if not found. 





        

<hr>



### function find\_dtc\_index 

_Find the table index of a registered 24-bit Diagnostic Trouble Code (DTC)._ 
```C++
static int find_dtc_index (
    const SYN_UDS_Server * server,
    uint32_t dtc
) 
```





**Parameters:**


* `server` UDS server instance. 
* `dtc` 24-bit DTC identifier to look up. 



**Returns:**

Zero-based table index on match, or -1 if not found. 





        

<hr>



### function get\_active\_security\_mask 

_Compute the bitmask of currently satisfied security levels on the UDS server._ 
```C++
static uint16_t get_active_security_mask (
    const SYN_UDS_Server * server
) 
```





**Parameters:**


* `server` UDS server instance. 



**Returns:**

Active security level bitmask including SYN\_UDS\_SECURITY\_MASK\_NONE. 





        

<hr>



### function get\_sid\_allowed\_session\_mask 

_Look up the permitted session bitmask configured for a UDS service identifier._ 
```C++
static uint8_t get_sid_allowed_session_mask (
    const SYN_UDS_Server * server,
    uint8_t sid
) 
```





**Parameters:**


* `server` UDS server instance. 
* `sid` UDS Service Identifier. 



**Returns:**

Permitted session bitmask (defaults to SYN\_UDS\_SESSION\_MASK\_ALL). 





        

<hr>



### function get\_sid\_required\_security\_mask 

_Look up the required security level bitmask configured for a UDS service identifier._ 
```C++
static uint16_t get_sid_required_security_mask (
    const SYN_UDS_Server * server,
    uint8_t sid
) 
```





**Parameters:**


* `server` UDS server instance. 
* `sid` UDS Service Identifier. 



**Returns:**

Required security level bitmask, or 0 if no custom security override exists. 





        

<hr>



### function session\_to\_mask 

_Convert a UDS diagnostic session enum into its single-bit permission mask._ 
```C++
static uint8_t session_to_mask (
    SYN_UDS_Session session
) 
```





**Parameters:**


* `session` Active UDS diagnostic session. 



**Returns:**

Session permission bitmask. 





        

<hr>



### function uds\_check\_preconditions 

_Validate functional addressing support, session mask, and security mask for a request._ 
```C++
static bool uds_check_preconditions (
    UDS_ReqCtx * ctx
) 
```





**Parameters:**


* `ctx` UDS request processing context. 



**Returns:**

true if preconditions pass, false if request was suppressed or rejected with an NRC. 





        

<hr>



### function uds\_commit\_var\_resp 

_Validate variable callback payload length and finalize positive UDS response._ 
```C++
static bool uds_commit_var_resp (
    UDS_ReqCtx * ctx,
    uint16_t hdr_len,
    uint16_t payload_len
) 
```





**Parameters:**


* `ctx` UDS request processing context. 
* `hdr_len` Fixed response header length already populated in resp\_buf[1..hdr\_len-1]. 
* `payload_len` Variable payload length returned by service callback. 



**Returns:**

true after encoding positive response or NRC 0x14 on overflow. 





        

<hr>



### function uds\_decode\_be\_var 

_Decode a big-endian UDS address or size parameter up to 32-bit range._ 
```C++
static bool uds_decode_be_var (
    const uint8_t * buf,
    uint8_t len,
    uint32_t * out_val
) 
```





**Parameters:**


* `buf` Source byte buffer. 
* `len` Parameter width in bytes (0..15 from ALFID). 
* `out_val` Output decoded 32-bit unsigned value. 



**Returns:**

true if value fits in uint32\_t, false if non-zero upper bytes overflow 32 bits. 





        

<hr>



### function uds\_dtc\_count\_by\_mask 

_Encode matching DTC count for ReadDTCInformation count-by-status-mask sub-functions._ 
```C++
static bool uds_dtc_count_by_mask (
    UDS_ReqCtx * ctx,
    uint8_t sub
) 
```





**Parameters:**


* `ctx` UDS request processing context. 
* `sub` ReadDTCInformation sub-function byte. 



**Returns:**

true after encoding positive or negative response. 





        

<hr>



### function uds\_dtc\_first\_or\_recent 

_Handle ReadDTCInformation sub-functions 0x0B..0x0E (first/most recent failed/confirmed)._ 
```C++
static bool uds_dtc_first_or_recent (
    UDS_ReqCtx * ctx,
    uint8_t sub
) 
```





**Parameters:**


* `ctx` UDS request processing context. 
* `sub` ReadDTCInformation sub-function byte. 



**Returns:**

true after encoding positive or negative response. 





        

<hr>



### function uds\_dtc\_in\_group 

_Check whether a 24-bit DTC belongs to the specified DTC group or exact DTC value._ 
```C++
static bool uds_dtc_in_group (
    uint32_t dtc,
    uint32_t group
) 
```





**Parameters:**


* `dtc` 24-bit Diagnostic Trouble Code. 
* `group` 24-bit groupOfDTC selector. 



**Returns:**

true if dtc falls within group, false otherwise. 





        

<hr>



### function uds\_dtc\_info\_by\_dtc 

_Handle ReadDTCInformation severity-by-DTC (0x09) and fault detection counter (0x14)._ 
```C++
static bool uds_dtc_info_by_dtc (
    UDS_ReqCtx * ctx,
    uint8_t sub
) 
```





**Parameters:**


* `ctx` UDS request processing context. 
* `sub` ReadDTCInformation sub-function byte. 



**Returns:**

true if sub-function was handled, false otherwise. 





        

<hr>



### function uds\_dtc\_list\_by\_mask 

_Encode a list of 24-bit DTCs and status bytes filtered by status mask._ 
```C++
static bool uds_dtc_list_by_mask (
    UDS_ReqCtx * ctx,
    uint8_t sub,
    uint8_t mask,
    bool match_all
) 
```





**Parameters:**


* `ctx` UDS request processing context. 
* `sub` ReadDTCInformation sub-function byte. 
* `mask` Status bitmask filter. 
* `match_all` true to emit all registered DTCs regardless of mask, false to filter. 



**Returns:**

true after encoding positive or negative response. 





        

<hr>



### function uds\_dtc\_misc\_records 

_Handle ReadDTCInformation snapshot ID, record-number, user-memory, and WWH-OBD queries._ 
```C++
static bool uds_dtc_misc_records (
    UDS_ReqCtx * ctx,
    uint8_t sub
) 
```





**Parameters:**


* `ctx` UDS request processing context. 
* `sub` ReadDTCInformation sub-function byte. 



**Returns:**

true if sub-function was handled, false otherwise. 





        

<hr>



### function uds\_dtc\_records 

_Handle ReadDTCInformation snapshot and extended-data record queries by DTC._ 
```C++
static bool uds_dtc_records (
    UDS_ReqCtx * ctx,
    uint8_t sub
) 
```





**Parameters:**


* `ctx` UDS request processing context. 
* `sub` ReadDTCInformation sub-function byte. 



**Returns:**

true if sub-function was handled, false otherwise. 





        

<hr>



### function uds\_dtc\_severity\_and\_fdc 

_Handle ReadDTCInformation severity-mask queries (0x07, 0x08)._ 
```C++
static bool uds_dtc_severity_and_fdc (
    UDS_ReqCtx * ctx,
    uint8_t sub
) 
```





**Parameters:**


* `ctx` UDS request processing context. 
* `sub` ReadDTCInformation sub-function byte. 



**Returns:**

true if sub-function was handled, false otherwise. 





        

<hr>



### function uds\_dtc\_status\_queries 

_Dispatch ReadDTCInformation status-mask, supported-DTC, and permanent-DTC sub-functions._ 
```C++
static bool uds_dtc_status_queries (
    UDS_ReqCtx * ctx,
    uint8_t sub
) 
```





**Parameters:**


* `ctx` UDS request processing context. 
* `sub` ReadDTCInformation sub-function byte. 



**Returns:**

true if sub-function was handled, false otherwise. 





        

<hr>



### function uds\_finalize\_response 

_Reload non-default S3 session timer and apply positive-response suppression bit._ 
```C++
static void uds_finalize_response (
    UDS_ReqCtx * ctx
) 
```





**Parameters:**


* `ctx` UDS request processing context. 




        

<hr>



### function uds\_find\_service 

_Look up the UDS service handler function registered for a Service Identifier._ 
```C++
static UDS_ServiceFn uds_find_service (
    uint8_t sid
) 
```





**Parameters:**


* `sid` UDS Service Identifier byte. 



**Returns:**

Matching [**UDS\_ServiceFn**](syn__uds_8c.md#typedef-uds_servicefn) handler, or NULL if unsupported. 





        

<hr>



### function uds\_is\_standard\_dtc\_group 

_Check whether a 24-bit group identifier matches an ISO 14229-1 standard DTC group._ 
```C++
static bool uds_is_standard_dtc_group (
    uint32_t group
) 
```





**Parameters:**


* `group` 24-bit groupOfDTC value. 



**Returns:**

true if group is a predefined standard DTC group, false otherwise. 





        

<hr>



### function uds\_nrc 

_Encode a UDS Negative Response Code (0x7F) or suppress it per functional addressing rules._ 
```C++
static bool uds_nrc (
    UDS_ReqCtx * ctx,
    uint8_t nrc
) 
```





**Parameters:**


* `ctx` UDS request processing context. 
* `nrc` ISO 14229-1 Negative Response Code byte. 



**Returns:**

true after populating or suppressing the negative response. 





        

<hr>



### function uds\_pack\_dtc24 

_Pack a 24-bit DTC value in big-endian byte order into an output buffer._ 
```C++
static void uds_pack_dtc24 (
    uint8_t * buf,
    uint16_t pos,
    uint32_t dtc
) 
```





**Parameters:**


* `buf` Target output buffer. 
* `pos` Write offset within buf. 
* `dtc` 24-bit Diagnostic Trouble Code value. 




        

<hr>



### function uds\_reset\_default\_session\_state 

_Reset session, security, communication, DTC setting, and transfer state to default._ 
```C++
static void uds_reset_default_session_state (
    SYN_UDS_Server * server
) 
```





**Parameters:**


* `server` UDS server instance. 




        

<hr>



### function uds\_security\_fail 

_Record a failed SecurityAccess key attempt, arm lockout delay if needed, and emit NRC._ 
```C++
static bool uds_security_fail (
    UDS_ReqCtx * ctx
) 
```





**Parameters:**


* `ctx` UDS request processing context. 



**Returns:**

true after encoding NRC 0x35 or 0x36. 





        

<hr>



### function uds\_security\_send\_key 

_Verify SecurityAccess sendKey (even sub-function) payload and unlock security level._ 
```C++
static bool uds_security_send_key (
    UDS_ReqCtx * ctx,
    uint8_t sub
) 
```





**Parameters:**


* `ctx` UDS request processing context. 
* `sub` Masked even SecurityAccess sub-function byte. 



**Returns:**

true after encoding positive or negative response. 





        

<hr>



### function uds\_sid\_access\_timing 

_Handle AccessTimingParameter (0x83) read and set sub-functions._ 
```C++
static bool uds_sid_access_timing (
    UDS_ReqCtx * ctx
) 
```





**Parameters:**


* `ctx` UDS request processing context. 



**Returns:**

true after encoding positive or negative response. 





        

<hr>



### function uds\_sid\_authentication 

_Handle Authentication (0x29) request._ 
```C++
static bool uds_sid_authentication (
    UDS_ReqCtx * ctx
) 
```





**Parameters:**


* `ctx` UDS request processing context. 



**Returns:**

true after encoding positive or negative response. 





        

<hr>



### function uds\_sid\_clear\_dtc 

_Handle ClearDiagnosticInformation (0x14) request for a DTC or DTC group._ 
```C++
static bool uds_sid_clear_dtc (
    UDS_ReqCtx * ctx
) 
```





**Parameters:**


* `ctx` UDS request processing context. 



**Returns:**

true after encoding positive or negative response. 





        

<hr>



### function uds\_sid\_comm\_control 

_Handle CommunicationControl (0x28) request._ 
```C++
static bool uds_sid_comm_control (
    UDS_ReqCtx * ctx
) 
```





**Parameters:**


* `ctx` UDS request processing context. 



**Returns:**

true after encoding positive or negative response. 





        

<hr>



### function uds\_sid\_control\_dtc\_setting 

_Handle ControlDTCSetting (0x85) on/off request._ 
```C++
static bool uds_sid_control_dtc_setting (
    UDS_ReqCtx * ctx
) 
```





**Parameters:**


* `ctx` UDS request processing context. 



**Returns:**

true after encoding positive or negative response. 





        

<hr>



### function uds\_sid\_dynamic\_did 

_Handle DynamicallyDefineDataIdentifier (0x2C) request._ 
```C++
static bool uds_sid_dynamic_did (
    UDS_ReqCtx * ctx
) 
```





**Parameters:**


* `ctx` UDS request processing context. 



**Returns:**

true after encoding positive or negative response. 





        

<hr>



### function uds\_sid\_ecu\_reset 

_Handle ECUReset (0x11) request and arm deferred reset timer._ 
```C++
static bool uds_sid_ecu_reset (
    UDS_ReqCtx * ctx
) 
```





**Parameters:**


* `ctx` UDS request processing context. 



**Returns:**

true after encoding positive or negative response. 





        

<hr>



### function uds\_sid\_file\_transfer 

_Handle RequestFileTransfer (0x38) request._ 
```C++
static bool uds_sid_file_transfer (
    UDS_ReqCtx * ctx
) 
```





**Parameters:**


* `ctx` UDS request processing context. 



**Returns:**

true after encoding positive or negative response. 





        

<hr>



### function uds\_sid\_has\_subfunction 

_Check whether a UDS service uses a sub-function byte with suppressPosRspMsgIndicationBit._ 
```C++
static bool uds_sid_has_subfunction (
    uint8_t sid
) 
```





**Parameters:**


* `sid` UDS Service Identifier. 



**Returns:**

true if byte 1 carries a sub-function field, false otherwise. 





        

<hr>



### function uds\_sid\_io\_control 

_Handle InputOutputControlByIdentifier (0x2F) request._ 
```C++
static bool uds_sid_io_control (
    UDS_ReqCtx * ctx
) 
```





**Parameters:**


* `ctx` UDS request processing context. 



**Returns:**

true after encoding positive or negative response. 





        

<hr>



### function uds\_sid\_link\_control 

_Handle LinkControl (0x87) baudrate verification and transition sub-functions._ 
```C++
static bool uds_sid_link_control (
    UDS_ReqCtx * ctx
) 
```





**Parameters:**


* `ctx` UDS request processing context. 



**Returns:**

true after encoding positive or negative response. 





        

<hr>



### function uds\_sid\_read\_did 

_Handle ReadDataByIdentifier (0x22) request for one or more DIDs._ 
```C++
static bool uds_sid_read_did (
    UDS_ReqCtx * ctx
) 
```





**Parameters:**


* `ctx` UDS request processing context. 



**Returns:**

true after encoding positive or negative response. 





        

<hr>



### function uds\_sid\_read\_dtc\_info 

_Handle ReadDTCInformation (0x19) request across all supported sub-functions._ 
```C++
static bool uds_sid_read_dtc_info (
    UDS_ReqCtx * ctx
) 
```





**Parameters:**


* `ctx` UDS request processing context. 



**Returns:**

true after encoding positive or negative response. 





        

<hr>



### function uds\_sid\_read\_memory 

_Handle ReadMemoryByAddress (0x23) request._ 
```C++
static bool uds_sid_read_memory (
    UDS_ReqCtx * ctx
) 
```





**Parameters:**


* `ctx` UDS request processing context. 



**Returns:**

true after encoding positive or negative response. 





        

<hr>



### function uds\_sid\_read\_periodic 

_Handle ReadDataByPeriodicIdentifier (0x2A) request._ 
```C++
static bool uds_sid_read_periodic (
    UDS_ReqCtx * ctx
) 
```





**Parameters:**


* `ctx` UDS request processing context. 



**Returns:**

true after encoding positive or negative response. 





        

<hr>



### function uds\_sid\_read\_scaling 

_Handle ReadScalingDataByIdentifier (0x24) request._ 
```C++
static bool uds_sid_read_scaling (
    UDS_ReqCtx * ctx
) 
```





**Parameters:**


* `ctx` UDS request processing context. 



**Returns:**

true after encoding positive or negative response. 





        

<hr>



### function uds\_sid\_request\_transfer 

_Handle RequestDownload (0x34) and RequestUpload (0x35) transfer initialization._ 
```C++
static bool uds_sid_request_transfer (
    UDS_ReqCtx * ctx
) 
```





**Parameters:**


* `ctx` UDS request processing context. 



**Returns:**

true after encoding positive or negative response. 





        

<hr>



### function uds\_sid\_roe 

_Handle ResponseOnEvent (0x86) request._ 
```C++
static bool uds_sid_roe (
    UDS_ReqCtx * ctx
) 
```





**Parameters:**


* `ctx` UDS request processing context. 



**Returns:**

true after encoding positive or negative response. 





        

<hr>



### function uds\_sid\_routine\_control 

_Handle RoutineControl (0x31) start, stop, and request-results sub-functions._ 
```C++
static bool uds_sid_routine_control (
    UDS_ReqCtx * ctx
) 
```





**Parameters:**


* `ctx` UDS request processing context. 



**Returns:**

true after encoding positive or negative response. 





        

<hr>



### function uds\_sid\_secured\_data 

_Handle SecuredDataTransmission (0x84) request._ 
```C++
static bool uds_sid_secured_data (
    UDS_ReqCtx * ctx
) 
```





**Parameters:**


* `ctx` UDS request processing context. 



**Returns:**

true after encoding positive or negative response. 





        

<hr>



### function uds\_sid\_security\_access 

_Handle SecurityAccess (0x27) requestSeed and sendKey sub-functions._ 
```C++
static bool uds_sid_security_access (
    UDS_ReqCtx * ctx
) 
```





**Parameters:**


* `ctx` UDS request processing context. 



**Returns:**

true after encoding positive or negative response. 





        

<hr>



### function uds\_sid\_session\_control 

_Handle DiagnosticSessionControl (0x10) request._ 
```C++
static bool uds_sid_session_control (
    UDS_ReqCtx * ctx
) 
```





**Parameters:**


* `ctx` UDS request processing context. 



**Returns:**

true after encoding positive or negative response. 





        

<hr>



### function uds\_sid\_tester\_present 

_Handle TesterPresent (0x3E) keepalive request._ 
```C++
static bool uds_sid_tester_present (
    UDS_ReqCtx * ctx
) 
```





**Parameters:**


* `ctx` UDS request processing context. 



**Returns:**

true after encoding positive or negative response. 





        

<hr>



### function uds\_sid\_transfer\_data 

_Handle TransferData (0x36) block sequence verification and download/upload dispatch._ 
```C++
static bool uds_sid_transfer_data (
    UDS_ReqCtx * ctx
) 
```





**Parameters:**


* `ctx` UDS request processing context. 



**Returns:**

true after encoding positive or negative response. 





        

<hr>



### function uds\_sid\_transfer\_exit 

_Handle RequestTransferExit (0x37) request to terminate an active transfer session._ 
```C++
static bool uds_sid_transfer_exit (
    UDS_ReqCtx * ctx
) 
```





**Parameters:**


* `ctx` UDS request processing context. 



**Returns:**

true after encoding positive or negative response. 





        

<hr>



### function uds\_sid\_write\_did 

_Handle WriteDataByIdentifier (0x2E) request._ 
```C++
static bool uds_sid_write_did (
    UDS_ReqCtx * ctx
) 
```





**Parameters:**


* `ctx` UDS request processing context. 



**Returns:**

true after encoding positive or negative response. 





        

<hr>



### function uds\_sid\_write\_memory 

_Handle WriteMemoryByAddress (0x3D) request._ 
```C++
static bool uds_sid_write_memory (
    UDS_ReqCtx * ctx
) 
```





**Parameters:**


* `ctx` UDS request processing context. 



**Returns:**

true after encoding positive or negative response. 





        

<hr>



### function uds\_timing\_read 

_Encode P2 and P2\* timing parameters for AccessTimingParameter read sub-functions._ 
```C++
static bool uds_timing_read (
    UDS_ReqCtx * ctx,
    SYN_UDS_AccessTimingType mode,
    uint16_t p2,
    uint16_t p2_star
) 
```





**Parameters:**


* `ctx` UDS request processing context. 
* `mode` AccessTimingParameter read mode sub-function. 
* `p2` Default or active P2Server\_max value in milliseconds. 
* `p2_star` Default or active P2\*Server\_max value in 10 ms units. 



**Returns:**

true after encoding positive or negative response. 





        

<hr>



### function uds\_timing\_set\_default 

_Handle AccessTimingParameter setTimingParametersToDefaultValues (0x83 0x02)._ 
```C++
static bool uds_timing_set_default (
    UDS_ReqCtx * ctx
) 
```





**Parameters:**


* `ctx` UDS request processing context. 



**Returns:**

true after encoding positive or negative response. 





        

<hr>



### function uds\_transfer\_download 

_Process one TransferData (0x36) block write during an active download session._ 
```C++
static bool uds_transfer_download (
    UDS_ReqCtx * ctx,
    uint8_t block_seq
) 
```





**Parameters:**


* `ctx` UDS request processing context. 
* `block_seq` Validated block sequence counter byte. 



**Returns:**

true after encoding positive or negative response. 





        

<hr>



### function uds\_transfer\_upload 

_Process one TransferData (0x36) block read during an active upload session._ 
```C++
static bool uds_transfer_upload (
    UDS_ReqCtx * ctx,
    uint8_t block_seq
) 
```





**Parameters:**


* `ctx` UDS request processing context. 
* `block_seq` Validated block sequence counter byte. 



**Returns:**

true after encoding positive or negative response. 





        

<hr>



### function uds\_transfer\_upload\_chunk 

_Encode one TransferData (0x36) upload response block from server memory._ 
```C++
static bool uds_transfer_upload_chunk (
    UDS_ReqCtx * ctx,
    uint8_t block_seq,
    uint32_t offset,
    uint16_t chunk_len
) 
```





**Parameters:**


* `ctx` UDS request processing context. 
* `block_seq` Block sequence counter byte for response header. 
* `offset` Byte offset from transfer\_address to read. 
* `chunk_len` Payload byte count to read into response buffer. 



**Returns:**

true after encoding positive or negative response. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_uds.c`

