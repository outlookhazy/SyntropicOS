

# File syn\_ocpp.h



[**FileList**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_ocpp.h**](syn__ocpp_8h.md)

[Go to the source code of this file](syn__ocpp_8h_source.md)

_Open Charge Point Protocol over JSON (OCPP-J 1.6 / 2.0.1) Dual-Role Engine._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_OCPP\_ChargePointInfo**](structSYN__OCPP__ChargePointInfo.md) <br>_Charge Point information for BootNotification._  |
| struct | [**SYN\_OCPP\_Client**](structSYN__OCPP__Client.md) <br>_OCPP Client instance state (EVSE Role)._  |
| struct | [**SYN\_OCPP\_DisplayMessage**](structSYN__OCPP__DisplayMessage.md) <br>_OCPP 2.1 Customer Display Message Structure._  |
| struct | [**SYN\_OCPP\_MeterValues**](structSYN__OCPP__MeterValues.md) <br>_Meter Values reading structure._  |
| struct | [**SYN\_OCPP\_Server**](structSYN__OCPP__Server.md) <br>_OCPP Server instance state (CSMS Central System Role)._  |


## Public Types

| Type | Name |
| ---: | :--- |
| typedef void(\* | [**SYN\_OCPP\_AuthorizationHandler**](#typedef-syn_ocpp_authorizationhandler)  <br>_Authorization response callback._  |
| enum  | [**SYN\_OCPP\_AuthorizationStatus**](#enum-syn_ocpp_authorizationstatus)  <br>_OCPP Authorization Status Enumeration._  |
| enum  | [**SYN\_OCPP\_ChargePointStatus**](#enum-syn_ocpp_chargepointstatus)  <br>_OCPP Connector Status Enumeration (OCPP 1.6 / 2.0.1 / 2.1)._  |
| typedef void(\* | [**SYN\_OCPP\_RegistrationHandler**](#typedef-syn_ocpp_registrationhandler)  <br>_Registration response callback._  |
| enum  | [**SYN\_OCPP\_RegistrationStatus**](#enum-syn_ocpp_registrationstatus)  <br>_OCPP Registration Status Enumeration._  |
| typedef [**bool**](syn__defs_8h.md#enum-bool)(\* | [**SYN\_OCPP\_RemoteStartHandler**](#typedef-syn_ocpp_remotestarthandler)  <br>_Remote Start Transaction handler callback._  |
| typedef [**bool**](syn__defs_8h.md#enum-bool)(\* | [**SYN\_OCPP\_RemoteStopHandler**](#typedef-syn_ocpp_remotestophandler)  <br>_Remote Stop Transaction handler callback._  |
| typedef [**SYN\_OCPP\_AuthorizationStatus**](syn__ocpp_8h.md#enum-syn_ocpp_authorizationstatus)(\* | [**SYN\_OCPP\_ServerAuthorizeHandler**](#typedef-syn_ocpp_serverauthorizehandler)  <br>_CSMS Server station authorize callback._  |
| typedef [**SYN\_OCPP\_RegistrationStatus**](syn__ocpp_8h.md#enum-syn_ocpp_registrationstatus)(\* | [**SYN\_OCPP\_ServerBootHandler**](#typedef-syn_ocpp_serverboothandler)  <br>_CSMS Server station boot callback._  |
| typedef int32\_t(\* | [**SYN\_OCPP\_ServerStartTxHandler**](#typedef-syn_ocpp_serverstarttxhandler)  <br>_CSMS Server station start transaction callback._  |
| typedef void(\* | [**SYN\_OCPP\_StartTxHandler**](#typedef-syn_ocpp_starttxhandler)  <br>_Start Transaction response callback._  |
| enum  | [**SYN\_OCPP\_V2GMode**](#enum-syn_ocpp_v2gmode)  <br>_OCPP 2.1 ISO 15118-20 Bidirectional V2G Energy Transfer Control Mode._  |
| enum  | [**SYN\_OCPP\_Version**](#enum-syn_ocpp_version)  <br>_Supported OCPP Protocol Versions._  |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_ocpp\_format\_authorize**](#function-syn_ocpp_format_authorize) ([**SYN\_OCPP\_Client**](structSYN__OCPP__Client.md) \* client, const char \* id\_tag, char \* out\_buf, size\_t max\_len, size\_t \* out\_len) <br>_Format an OCPP-J Authorize.req Call frame._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_ocpp\_format\_boot\_notification**](#function-syn_ocpp_format_boot_notification) ([**SYN\_OCPP\_Client**](structSYN__OCPP__Client.md) \* client, const [**SYN\_OCPP\_ChargePointInfo**](structSYN__OCPP__ChargePointInfo.md) \* info, char \* out\_buf, size\_t max\_len, size\_t \* out\_len) <br>_Format an OCPP-J BootNotification.req Call frame._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_ocpp\_format\_call\_error**](#function-syn_ocpp_format_call_error) (const char \* msg\_id, const char \* error\_code, const char \* error\_description, char \* out\_buf, size\_t max\_len, size\_t \* out\_len) <br>_Format an OCPP-J CallError (MessageTypeId 4) response frame._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_ocpp\_format\_display\_message**](#function-syn_ocpp_format_display_message) ([**SYN\_OCPP\_Client**](structSYN__OCPP__Client.md) \* client, const [**SYN\_OCPP\_DisplayMessage**](structSYN__OCPP__DisplayMessage.md) \* msg, char \* out\_buf, size\_t max\_len, size\_t \* out\_len) <br>_Format an OCPP 2.1 Customer DisplayMessage Call frame._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_ocpp\_format\_heartbeat**](#function-syn_ocpp_format_heartbeat) ([**SYN\_OCPP\_Client**](structSYN__OCPP__Client.md) \* client, char \* out\_buf, size\_t max\_len, size\_t \* out\_len) <br>_Format an OCPP-J Heartbeat.req Call frame._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_ocpp\_format\_meter\_values**](#function-syn_ocpp_format_meter_values) ([**SYN\_OCPP\_Client**](structSYN__OCPP__Client.md) \* client, uint32\_t connector\_id, const [**SYN\_OCPP\_MeterValues**](structSYN__OCPP__MeterValues.md) \* values, char \* out\_buf, size\_t max\_len, size\_t \* out\_len) <br>_Format an OCPP-J MeterValues.req Call frame._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_ocpp\_format\_start\_transaction**](#function-syn_ocpp_format_start_transaction) ([**SYN\_OCPP\_Client**](structSYN__OCPP__Client.md) \* client, uint32\_t connector\_id, const char \* id\_tag, uint32\_t meter\_start\_wh, char \* out\_buf, size\_t max\_len, size\_t \* out\_len) <br>_Format an OCPP-J StartTransaction.req Call frame._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_ocpp\_format\_status\_notification**](#function-syn_ocpp_format_status_notification) ([**SYN\_OCPP\_Client**](structSYN__OCPP__Client.md) \* client, uint32\_t connector\_id, [**SYN\_OCPP\_ChargePointStatus**](syn__ocpp_8h.md#enum-syn_ocpp_chargepointstatus) status, const char \* error\_code, char \* out\_buf, size\_t max\_len, size\_t \* out\_len) <br>_Format an OCPP-J StatusNotification.req Call frame._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_ocpp\_format\_stop\_transaction**](#function-syn_ocpp_format_stop_transaction) ([**SYN\_OCPP\_Client**](structSYN__OCPP__Client.md) \* client, int32\_t transaction\_id, uint32\_t meter\_stop\_wh, const char \* reason, char \* out\_buf, size\_t max\_len, size\_t \* out\_len) <br>_Format an OCPP-J StopTransaction.req Call frame._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_ocpp\_format\_v2g\_energy\_transfer**](#function-syn_ocpp_format_v2g_energy_transfer) ([**SYN\_OCPP\_Client**](structSYN__OCPP__Client.md) \* client, [**SYN\_OCPP\_V2GMode**](syn__ocpp_8h.md#enum-syn_ocpp_v2gmode) mode, int32\_t power\_limit\_w, char \* out\_buf, size\_t max\_len, size\_t \* out\_len) <br>_Format an OCPP 2.1 ISO 15118-20 V2G Energy Transfer Request Call frame._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_ocpp\_init**](#function-syn_ocpp_init) ([**SYN\_OCPP\_Client**](structSYN__OCPP__Client.md) \* client) <br>_Initialize an OCPP-J client instance._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_ocpp\_process\_message**](#function-syn_ocpp_process_message) ([**SYN\_OCPP\_Client**](structSYN__OCPP__Client.md) \* client, const char \* in\_buf, size\_t in\_len, char \* out\_resp, size\_t max\_resp\_len, size\_t \* out\_resp\_len) <br>_Process an incoming OCPP-J JSON frame._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_ocpp\_server\_format\_remote\_start**](#function-syn_ocpp_server_format_remote_start) ([**SYN\_OCPP\_Server**](structSYN__OCPP__Server.md) \* server, uint32\_t connector\_id, const char \* id\_tag, char \* out\_buf, size\_t max\_len, size\_t \* out\_len) <br>_Format a RemoteStartTransaction.req Call frame from CSMS server to station._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_ocpp\_server\_format\_remote\_stop**](#function-syn_ocpp_server_format_remote_stop) ([**SYN\_OCPP\_Server**](structSYN__OCPP__Server.md) \* server, int32\_t transaction\_id, char \* out\_buf, size\_t max\_len, size\_t \* out\_len) <br>_Format a RemoteStopTransaction.req Call frame from CSMS server to station._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_ocpp\_server\_init**](#function-syn_ocpp_server_init) ([**SYN\_OCPP\_Server**](structSYN__OCPP__Server.md) \* server) <br>_Initialize an OCPP CSMS Server instance._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_ocpp\_server\_process\_message**](#function-syn_ocpp_server_process_message) ([**SYN\_OCPP\_Server**](structSYN__OCPP__Server.md) \* server, const char \* in\_buf, size\_t in\_len, char \* out\_resp, size\_t max\_resp\_len, size\_t \* out\_resp\_len) <br>_Process incoming station request frame on CSMS server and generate response CallResult._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_ocpp\_server\_set\_callbacks**](#function-syn_ocpp_server_set_callbacks) ([**SYN\_OCPP\_Server**](structSYN__OCPP__Server.md) \* server, [**SYN\_OCPP\_ServerBootHandler**](syn__ocpp_8h.md#typedef-syn_ocpp_serverboothandler) boot\_cb, [**SYN\_OCPP\_ServerAuthorizeHandler**](syn__ocpp_8h.md#typedef-syn_ocpp_serverauthorizehandler) auth\_cb, [**SYN\_OCPP\_ServerStartTxHandler**](syn__ocpp_8h.md#typedef-syn_ocpp_serverstarttxhandler) start\_tx\_cb, void \* user\_ctx) <br>_Set event callbacks for OCPP CSMS Server._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_ocpp\_set\_callbacks**](#function-syn_ocpp_set_callbacks) ([**SYN\_OCPP\_Client**](structSYN__OCPP__Client.md) \* client, [**SYN\_OCPP\_RegistrationHandler**](syn__ocpp_8h.md#typedef-syn_ocpp_registrationhandler) reg\_cb, [**SYN\_OCPP\_AuthorizationHandler**](syn__ocpp_8h.md#typedef-syn_ocpp_authorizationhandler) auth\_cb, [**SYN\_OCPP\_StartTxHandler**](syn__ocpp_8h.md#typedef-syn_ocpp_starttxhandler) start\_tx\_cb, [**SYN\_OCPP\_RemoteStartHandler**](syn__ocpp_8h.md#typedef-syn_ocpp_remotestarthandler) remote\_start\_cb, [**SYN\_OCPP\_RemoteStopHandler**](syn__ocpp_8h.md#typedef-syn_ocpp_remotestophandler) remote\_stop\_cb, void \* user\_ctx) <br>_Set event and remote command callbacks for OCPP client._  |
|  void | [**syn\_ocpp\_tick**](#function-syn_ocpp_tick) ([**SYN\_OCPP\_Client**](structSYN__OCPP__Client.md) \* client, uint32\_t dt\_ms, char \* out\_hb\_buf, size\_t max\_len, size\_t \* out\_len) <br>_Tick periodic Heartbeat timer for OCPP client._  |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_OCPP\_ERROR\_FORMATION\_VIOLATION**](syn__ocpp_8h.md#define-syn_ocpp_error_formation_violation)  `"FormationViolation"`<br> |
| define  | [**SYN\_OCPP\_ERROR\_GENERIC\_ERROR**](syn__ocpp_8h.md#define-syn_ocpp_error_generic_error)  `"GenericError"`<br> |
| define  | [**SYN\_OCPP\_ERROR\_INTERNAL\_ERROR**](syn__ocpp_8h.md#define-syn_ocpp_error_internal_error)  `"InternalError"`<br> |
| define  | [**SYN\_OCPP\_ERROR\_NOT\_IMPLEMENTED**](syn__ocpp_8h.md#define-syn_ocpp_error_not_implemented)  `"NotImplemented"`<br> |
| define  | [**SYN\_OCPP\_ERROR\_NOT\_SUPPORTED**](syn__ocpp_8h.md#define-syn_ocpp_error_not_supported)  `"NotSupported"`<br> |
| define  | [**SYN\_OCPP\_ERROR\_OCCURRENCE\_CONSTRAINT\_VIOLATION**](syn__ocpp_8h.md#define-syn_ocpp_error_occurrence_constraint_violation)  `"OccurenceConstraintViolation"`<br> |
| define  | [**SYN\_OCPP\_ERROR\_PROPERTY\_CONSTRAINT\_VIOLATION**](syn__ocpp_8h.md#define-syn_ocpp_error_property_constraint_violation)  `"PropertyConstraintViolation"`<br> |
| define  | [**SYN\_OCPP\_ERROR\_PROTOCOL\_ERROR**](syn__ocpp_8h.md#define-syn_ocpp_error_protocol_error)  `"ProtocolError"`<br> |
| define  | [**SYN\_OCPP\_ERROR\_SECURITY\_ERROR**](syn__ocpp_8h.md#define-syn_ocpp_error_security_error)  `"SecurityError"`<br> |
| define  | [**SYN\_OCPP\_ERROR\_TYPE\_CONSTRAINT\_VIOLATION**](syn__ocpp_8h.md#define-syn_ocpp_error_type_constraint_violation)  `"TypeConstraintViolation"`<br> |
| define  | [**SYN\_OCPP\_MSG\_TYPE\_CALL**](syn__ocpp_8h.md#define-syn_ocpp_msg_type_call)  `2U`<br> |
| define  | [**SYN\_OCPP\_MSG\_TYPE\_CALLERROR**](syn__ocpp_8h.md#define-syn_ocpp_msg_type_callerror)  `4U`<br> |
| define  | [**SYN\_OCPP\_MSG\_TYPE\_CALLRESULT**](syn__ocpp_8h.md#define-syn_ocpp_msg_type_callresult)  `3U`<br> |
| define  | [**SYN\_OCPP\_SUBPROTOCOL\_1\_6**](syn__ocpp_8h.md#define-syn_ocpp_subprotocol_1_6)  `"ocpp1.6"`<br> |
| define  | [**SYN\_OCPP\_SUBPROTOCOL\_2\_0\_1**](syn__ocpp_8h.md#define-syn_ocpp_subprotocol_2_0_1)  `"ocpp2.0.1"`<br> |
| define  | [**SYN\_OCPP\_SUBPROTOCOL\_2\_1**](syn__ocpp_8h.md#define-syn_ocpp_subprotocol_2_1)  `"ocpp2.1"`<br> |

## Detailed Description


Implements a lightweight, zero-allocation OCPP-J protocol engine supporting both Charge Point (EVSE Client) and Central Management System (CSMS Server) roles. 


    
## Public Types Documentation




### typedef SYN\_OCPP\_AuthorizationHandler 

_Authorization response callback._ 
```C++
typedef void(* SYN_OCPP_AuthorizationHandler) (const char *id_tag, SYN_OCPP_AuthorizationStatus status, void *user_ctx);
```




<hr>



### enum SYN\_OCPP\_AuthorizationStatus 

_OCPP Authorization Status Enumeration._ 
```C++
enum SYN_OCPP_AuthorizationStatus {
    SYN_OCPP_AUTH_ACCEPTED = 0,
    SYN_OCPP_AUTH_BLOCKED,
    SYN_OCPP_AUTH_EXPIRED,
    SYN_OCPP_AUTH_INVALID
};
```




<hr>



### enum SYN\_OCPP\_ChargePointStatus 

_OCPP Connector Status Enumeration (OCPP 1.6 / 2.0.1 / 2.1)._ 
```C++
enum SYN_OCPP_ChargePointStatus {
    SYN_OCPP_STATUS_AVAILABLE = 0,
    SYN_OCPP_STATUS_PREPARING,
    SYN_OCPP_STATUS_CHARGING,
    SYN_OCPP_STATUS_SUSPENDED_EV,
    SYN_OCPP_STATUS_SUSPENDED_EVSE,
    SYN_OCPP_STATUS_FINISHING,
    SYN_OCPP_STATUS_RESERVED,
    SYN_OCPP_STATUS_UNAVAILABLE,
    SYN_OCPP_STATUS_FAULTED
};
```




<hr>



### typedef SYN\_OCPP\_RegistrationHandler 

_Registration response callback._ 
```C++
typedef void(* SYN_OCPP_RegistrationHandler) (SYN_OCPP_RegistrationStatus status, uint32_t heartbeat_interval_sec, void *user_ctx);
```




<hr>



### enum SYN\_OCPP\_RegistrationStatus 

_OCPP Registration Status Enumeration._ 
```C++
enum SYN_OCPP_RegistrationStatus {
    SYN_OCPP_REGISTRATION_ACCEPTED = 0,
    SYN_OCPP_REGISTRATION_PENDING,
    SYN_OCPP_REGISTRATION_REJECTED
};
```




<hr>



### typedef SYN\_OCPP\_RemoteStartHandler 

_Remote Start Transaction handler callback._ 
```C++
typedef bool(* SYN_OCPP_RemoteStartHandler) (uint32_t connector_id, const char *id_tag, void *user_ctx);
```




<hr>



### typedef SYN\_OCPP\_RemoteStopHandler 

_Remote Stop Transaction handler callback._ 
```C++
typedef bool(* SYN_OCPP_RemoteStopHandler) (int32_t transaction_id, void *user_ctx);
```




<hr>



### typedef SYN\_OCPP\_ServerAuthorizeHandler 

_CSMS Server station authorize callback._ 
```C++
typedef SYN_OCPP_AuthorizationStatus(* SYN_OCPP_ServerAuthorizeHandler) (const char *id_tag, void *user_ctx);
```




<hr>



### typedef SYN\_OCPP\_ServerBootHandler 

_CSMS Server station boot callback._ 
```C++
typedef SYN_OCPP_RegistrationStatus(* SYN_OCPP_ServerBootHandler) (const SYN_OCPP_ChargePointInfo *info, uint32_t *heartbeat_sec, void *user_ctx);
```




<hr>



### typedef SYN\_OCPP\_ServerStartTxHandler 

_CSMS Server station start transaction callback._ 
```C++
typedef int32_t(* SYN_OCPP_ServerStartTxHandler) (uint32_t connector_id, const char *id_tag, uint32_t meter_start_wh, void *user_ctx);
```




<hr>



### typedef SYN\_OCPP\_StartTxHandler 

_Start Transaction response callback._ 
```C++
typedef void(* SYN_OCPP_StartTxHandler) (int32_t transaction_id, SYN_OCPP_AuthorizationStatus status, void *user_ctx);
```




<hr>



### enum SYN\_OCPP\_V2GMode 

_OCPP 2.1 ISO 15118-20 Bidirectional V2G Energy Transfer Control Mode._ 
```C++
enum SYN_OCPP_V2GMode {
    SYN_OCPP_V2G_SCHEDULED = 0,
    SYN_OCPP_V2G_DYNAMIC
};
```




<hr>



### enum SYN\_OCPP\_Version 

_Supported OCPP Protocol Versions._ 
```C++
enum SYN_OCPP_Version {
    SYN_OCPP_VERSION_1_6 = 0,
    SYN_OCPP_VERSION_2_0_1,
    SYN_OCPP_VERSION_2_1
};
```




<hr>
## Public Functions Documentation




### function syn\_ocpp\_format\_authorize 

_Format an OCPP-J Authorize.req Call frame._ 
```C++
SYN_Status syn_ocpp_format_authorize (
    SYN_OCPP_Client * client,
    const char * id_tag,
    char * out_buf,
    size_t max_len,
    size_t * out_len
) 
```





**Parameters:**


* `client` Pointer to client instance. 
* `id_tag` RFID tag string. 
* `out_buf` Output text buffer. 
* `max_len` Buffer capacity. 
* `out_len` Pointer to store length. 



**Returns:**

SYN\_OK on success, error code on failure. 





        

<hr>



### function syn\_ocpp\_format\_boot\_notification 

_Format an OCPP-J BootNotification.req Call frame._ 
```C++
SYN_Status syn_ocpp_format_boot_notification (
    SYN_OCPP_Client * client,
    const SYN_OCPP_ChargePointInfo * info,
    char * out_buf,
    size_t max_len,
    size_t * out_len
) 
```





**Parameters:**


* `client` Pointer to client instance. 
* `info` Charge point info. 
* `out_buf` Output text buffer. 
* `max_len` Buffer capacity. 
* `out_len` Pointer to store length. 



**Returns:**

SYN\_OK on success, error code on failure. 





        

<hr>



### function syn\_ocpp\_format\_call\_error 

_Format an OCPP-J CallError (MessageTypeId 4) response frame._ 
```C++
SYN_Status syn_ocpp_format_call_error (
    const char * msg_id,
    const char * error_code,
    const char * error_description,
    char * out_buf,
    size_t max_len,
    size_t * out_len
) 
```





**Parameters:**


* `msg_id` Unique message identifier string to match Call request. 
* `error_code` Error code string (e.g. SYN\_OCPP\_ERROR\_NOT\_IMPLEMENTED). 
* `error_description` Optional human-readable error description string. 
* `out_buf` Output text buffer. 
* `max_len` Buffer capacity. 
* `out_len` Pointer to store length. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM or SYN\_ERROR on failure. 





        

<hr>



### function syn\_ocpp\_format\_display\_message 

_Format an OCPP 2.1 Customer DisplayMessage Call frame._ 
```C++
SYN_Status syn_ocpp_format_display_message (
    SYN_OCPP_Client * client,
    const SYN_OCPP_DisplayMessage * msg,
    char * out_buf,
    size_t max_len,
    size_t * out_len
) 
```





**Parameters:**


* `client` Pointer to client instance. 
* `msg` Pointer to DisplayMessage structure. 
* `out_buf` Output text buffer. 
* `max_len` Buffer capacity. 
* `out_len` Pointer to store length. 



**Returns:**

SYN\_OK on success, error code on failure. 





        

<hr>



### function syn\_ocpp\_format\_heartbeat 

_Format an OCPP-J Heartbeat.req Call frame._ 
```C++
SYN_Status syn_ocpp_format_heartbeat (
    SYN_OCPP_Client * client,
    char * out_buf,
    size_t max_len,
    size_t * out_len
) 
```





**Parameters:**


* `client` Pointer to client instance. 
* `out_buf` Output text buffer. 
* `max_len` Buffer capacity. 
* `out_len` Pointer to store length. 



**Returns:**

SYN\_OK on success, error code on failure. 





        

<hr>



### function syn\_ocpp\_format\_meter\_values 

_Format an OCPP-J MeterValues.req Call frame._ 
```C++
SYN_Status syn_ocpp_format_meter_values (
    SYN_OCPP_Client * client,
    uint32_t connector_id,
    const SYN_OCPP_MeterValues * values,
    char * out_buf,
    size_t max_len,
    size_t * out_len
) 
```





**Parameters:**


* `client` Pointer to client instance. 
* `connector_id` Connector ID. 
* `values` Meter readings pointer. 
* `out_buf` Output text buffer. 
* `max_len` Buffer capacity. 
* `out_len` Pointer to store length. 



**Returns:**

SYN\_OK on success, error code on failure. 





        

<hr>



### function syn\_ocpp\_format\_start\_transaction 

_Format an OCPP-J StartTransaction.req Call frame._ 
```C++
SYN_Status syn_ocpp_format_start_transaction (
    SYN_OCPP_Client * client,
    uint32_t connector_id,
    const char * id_tag,
    uint32_t meter_start_wh,
    char * out_buf,
    size_t max_len,
    size_t * out_len
) 
```





**Parameters:**


* `client` Pointer to client instance. 
* `connector_id` Connector ID. 
* `id_tag` RFID tag string. 
* `meter_start_wh` Meter reading in Wh. 
* `out_buf` Output text buffer. 
* `max_len` Buffer capacity. 
* `out_len` Pointer to store length. 



**Returns:**

SYN\_OK on success, error code on failure. 





        

<hr>



### function syn\_ocpp\_format\_status\_notification 

_Format an OCPP-J StatusNotification.req Call frame._ 
```C++
SYN_Status syn_ocpp_format_status_notification (
    SYN_OCPP_Client * client,
    uint32_t connector_id,
    SYN_OCPP_ChargePointStatus status,
    const char * error_code,
    char * out_buf,
    size_t max_len,
    size_t * out_len
) 
```





**Parameters:**


* `client` Pointer to client instance. 
* `connector_id` Connector ID. 
* `status` Status enum. 
* `error_code` Error code string. 
* `out_buf` Output text buffer. 
* `max_len` Buffer capacity. 
* `out_len` Pointer to store length. 



**Returns:**

SYN\_OK on success, error code on failure. 





        

<hr>



### function syn\_ocpp\_format\_stop\_transaction 

_Format an OCPP-J StopTransaction.req Call frame._ 
```C++
SYN_Status syn_ocpp_format_stop_transaction (
    SYN_OCPP_Client * client,
    int32_t transaction_id,
    uint32_t meter_stop_wh,
    const char * reason,
    char * out_buf,
    size_t max_len,
    size_t * out_len
) 
```





**Parameters:**


* `client` Pointer to client instance. 
* `transaction_id` Active transaction ID. 
* `meter_stop_wh` Meter reading in Wh. 
* `reason` Reason string. 
* `out_buf` Output text buffer. 
* `max_len` Buffer capacity. 
* `out_len` Pointer to store length. 



**Returns:**

SYN\_OK on success, error code on failure. 





        

<hr>



### function syn\_ocpp\_format\_v2g\_energy\_transfer 

_Format an OCPP 2.1 ISO 15118-20 V2G Energy Transfer Request Call frame._ 
```C++
SYN_Status syn_ocpp_format_v2g_energy_transfer (
    SYN_OCPP_Client * client,
    SYN_OCPP_V2GMode mode,
    int32_t power_limit_w,
    char * out_buf,
    size_t max_len,
    size_t * out_len
) 
```





**Parameters:**


* `client` Pointer to client instance. 
* `mode` V2G control mode (Scheduled / Dynamic). 
* `power_limit_w` Power limit in Watts (positive for charging, negative for discharging). 
* `out_buf` Output text buffer. 
* `max_len` Buffer capacity. 
* `out_len` Pointer to store length. 



**Returns:**

SYN\_OK on success, error code on failure. 





        

<hr>



### function syn\_ocpp\_init 

_Initialize an OCPP-J client instance._ 
```C++
SYN_Status syn_ocpp_init (
    SYN_OCPP_Client * client
) 
```





**Parameters:**


* `client` Pointer to client instance. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM if client is NULL. 





        

<hr>



### function syn\_ocpp\_process\_message 

_Process an incoming OCPP-J JSON frame._ 
```C++
SYN_Status syn_ocpp_process_message (
    SYN_OCPP_Client * client,
    const char * in_buf,
    size_t in_len,
    char * out_resp,
    size_t max_resp_len,
    size_t * out_resp_len
) 
```





**Parameters:**


* `client` Pointer to client instance. 
* `in_buf` Incoming text frame. 
* `in_len` Frame length. 
* `out_resp` Optional response buffer. 
* `max_resp_len` Response capacity. 
* `out_resp_len` Pointer to store response length. 



**Returns:**

SYN\_OK on success, error code on failure. 





        

<hr>



### function syn\_ocpp\_server\_format\_remote\_start 

_Format a RemoteStartTransaction.req Call frame from CSMS server to station._ 
```C++
SYN_Status syn_ocpp_server_format_remote_start (
    SYN_OCPP_Server * server,
    uint32_t connector_id,
    const char * id_tag,
    char * out_buf,
    size_t max_len,
    size_t * out_len
) 
```





**Parameters:**


* `server` Pointer to server instance. 
* `connector_id` Target connector ID. 
* `id_tag` Target RFID tag string. 
* `out_buf` Output text buffer for JSON payload. 
* `max_len` Capacity of output buffer. 
* `out_len` Pointer to store output byte count. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM or SYN\_ERROR on failure. 





        

<hr>



### function syn\_ocpp\_server\_format\_remote\_stop 

_Format a RemoteStopTransaction.req Call frame from CSMS server to station._ 
```C++
SYN_Status syn_ocpp_server_format_remote_stop (
    SYN_OCPP_Server * server,
    int32_t transaction_id,
    char * out_buf,
    size_t max_len,
    size_t * out_len
) 
```





**Parameters:**


* `server` Pointer to server instance. 
* `transaction_id` Active transaction ID to stop. 
* `out_buf` Output text buffer for JSON payload. 
* `max_len` Capacity of output buffer. 
* `out_len` Pointer to store output byte count. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM or SYN\_ERROR on failure. 





        

<hr>



### function syn\_ocpp\_server\_init 

_Initialize an OCPP CSMS Server instance._ 
```C++
SYN_Status syn_ocpp_server_init (
    SYN_OCPP_Server * server
) 
```





**Parameters:**


* `server` Pointer to server instance. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM if server is NULL. 





        

<hr>



### function syn\_ocpp\_server\_process\_message 

_Process incoming station request frame on CSMS server and generate response CallResult._ 
```C++
SYN_Status syn_ocpp_server_process_message (
    SYN_OCPP_Server * server,
    const char * in_buf,
    size_t in_len,
    char * out_resp,
    size_t max_resp_len,
    size_t * out_resp_len
) 
```





**Parameters:**


* `server` Pointer to server instance. 
* `in_buf` Incoming JSON frame from station. 
* `in_len` Length of incoming JSON frame. 
* `out_resp` Output buffer for CallResult response frame. 
* `max_resp_len` Capacity of output response buffer. 
* `out_resp_len` Pointer to store response byte count. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM or SYN\_ERROR on failure. 





        

<hr>



### function syn\_ocpp\_server\_set\_callbacks 

_Set event callbacks for OCPP CSMS Server._ 
```C++
SYN_Status syn_ocpp_server_set_callbacks (
    SYN_OCPP_Server * server,
    SYN_OCPP_ServerBootHandler boot_cb,
    SYN_OCPP_ServerAuthorizeHandler auth_cb,
    SYN_OCPP_ServerStartTxHandler start_tx_cb,
    void * user_ctx
) 
```





**Parameters:**


* `server` Pointer to server instance. 
* `boot_cb` Station registration callback. 
* `auth_cb` RFID authorization callback. 
* `start_tx_cb` StartTransaction callback. 
* `user_ctx` User context pointer. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM if server is NULL. 





        

<hr>



### function syn\_ocpp\_set\_callbacks 

_Set event and remote command callbacks for OCPP client._ 
```C++
SYN_Status syn_ocpp_set_callbacks (
    SYN_OCPP_Client * client,
    SYN_OCPP_RegistrationHandler reg_cb,
    SYN_OCPP_AuthorizationHandler auth_cb,
    SYN_OCPP_StartTxHandler start_tx_cb,
    SYN_OCPP_RemoteStartHandler remote_start_cb,
    SYN_OCPP_RemoteStopHandler remote_stop_cb,
    void * user_ctx
) 
```





**Parameters:**


* `client` Pointer to client instance. 
* `reg_cb` Registration response callback. 
* `auth_cb` Authorization response callback. 
* `start_tx_cb` StartTransaction response callback. 
* `remote_start_cb` RemoteStart command callback. 
* `remote_stop_cb` RemoteStop command callback. 
* `user_ctx` User context pointer. 



**Returns:**

SYN\_OK on success, SYN\_INVALID\_PARAM if client is NULL. 





        

<hr>



### function syn\_ocpp\_tick 

_Tick periodic Heartbeat timer for OCPP client._ 
```C++
void syn_ocpp_tick (
    SYN_OCPP_Client * client,
    uint32_t dt_ms,
    char * out_hb_buf,
    size_t max_len,
    size_t * out_len
) 
```





**Parameters:**


* `client` Pointer to client instance. 
* `dt_ms` Elapsed time step in ms. 
* `out_hb_buf` Output heartbeat buffer. 
* `max_len` Buffer capacity. 
* `out_len` Pointer to store length. 




        

<hr>
## Macro Definition Documentation





### define SYN\_OCPP\_ERROR\_FORMATION\_VIOLATION 

```C++
#define SYN_OCPP_ERROR_FORMATION_VIOLATION `"FormationViolation"`
```



Syntax formation violation 


        

<hr>



### define SYN\_OCPP\_ERROR\_GENERIC\_ERROR 

```C++
#define SYN_OCPP_ERROR_GENERIC_ERROR `"GenericError"`
```



Generic error 


        

<hr>



### define SYN\_OCPP\_ERROR\_INTERNAL\_ERROR 

```C++
#define SYN_OCPP_ERROR_INTERNAL_ERROR `"InternalError"`
```



Internal error 


        

<hr>



### define SYN\_OCPP\_ERROR\_NOT\_IMPLEMENTED 

```C++
#define SYN_OCPP_ERROR_NOT_IMPLEMENTED `"NotImplemented"`
```



Action not implemented 


        

<hr>



### define SYN\_OCPP\_ERROR\_NOT\_SUPPORTED 

```C++
#define SYN_OCPP_ERROR_NOT_SUPPORTED `"NotSupported"`
```



Action not supported 


        

<hr>



### define SYN\_OCPP\_ERROR\_OCCURRENCE\_CONSTRAINT\_VIOLATION 

```C++
#define SYN_OCPP_ERROR_OCCURRENCE_CONSTRAINT_VIOLATION `"OccurenceConstraintViolation"`
```



Field occurrence violation 


        

<hr>



### define SYN\_OCPP\_ERROR\_PROPERTY\_CONSTRAINT\_VIOLATION 

```C++
#define SYN_OCPP_ERROR_PROPERTY_CONSTRAINT_VIOLATION `"PropertyConstraintViolation"`
```



Field property constraint violation 


        

<hr>



### define SYN\_OCPP\_ERROR\_PROTOCOL\_ERROR 

```C++
#define SYN_OCPP_ERROR_PROTOCOL_ERROR `"ProtocolError"`
```



Protocol error 


        

<hr>



### define SYN\_OCPP\_ERROR\_SECURITY\_ERROR 

```C++
#define SYN_OCPP_ERROR_SECURITY_ERROR `"SecurityError"`
```



Security error 


        

<hr>



### define SYN\_OCPP\_ERROR\_TYPE\_CONSTRAINT\_VIOLATION 

```C++
#define SYN_OCPP_ERROR_TYPE_CONSTRAINT_VIOLATION `"TypeConstraintViolation"`
```



Data type constraint violation 


        

<hr>



### define SYN\_OCPP\_MSG\_TYPE\_CALL 

```C++
#define SYN_OCPP_MSG_TYPE_CALL `2U`
```



Client/Server Request Call 


        

<hr>



### define SYN\_OCPP\_MSG\_TYPE\_CALLERROR 

```C++
#define SYN_OCPP_MSG_TYPE_CALLERROR `4U`
```



Error Response 


        

<hr>



### define SYN\_OCPP\_MSG\_TYPE\_CALLRESULT 

```C++
#define SYN_OCPP_MSG_TYPE_CALLRESULT `3U`
```



Success Response 


        

<hr>



### define SYN\_OCPP\_SUBPROTOCOL\_1\_6 

```C++
#define SYN_OCPP_SUBPROTOCOL_1_6 `"ocpp1.6"`
```



Official OCPP 1.6-J WebSocket Subprotocol 


        

<hr>



### define SYN\_OCPP\_SUBPROTOCOL\_2\_0\_1 

```C++
#define SYN_OCPP_SUBPROTOCOL_2_0_1 `"ocpp2.0.1"`
```



Official OCPP 2.0.1 WebSocket Subprotocol 


        

<hr>



### define SYN\_OCPP\_SUBPROTOCOL\_2\_1 

```C++
#define SYN_OCPP_SUBPROTOCOL_2_1 `"ocpp2.1"`
```



Official OCPP 2.1 WebSocket Subprotocol 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_ocpp.h`

