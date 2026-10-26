

# File syn\_lwm2m.c



[**FileList**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_lwm2m.c**](syn__lwm2m_8c.md)

[Go to the source code of this file](syn__lwm2m_8c_source.md)

_Zero-Heap OMA Lightweight M2M (LwM2M v1.1/v1.2) Core Client Engine._ 

* `#include "syn_lwm2m.h"`
* `#include "syntropic/port/syn_port_system.h"`
* `#include "syntropic/util/syn_assert.h"`
* `#include <stdio.h>`
* `#include <stdlib.h>`
* `#include <string.h>`























## Public Static Attributes

| Type | Name |
| ---: | :--- |
|  const [**SYN\_LwM2M\_ResourceDesc**](structSYN__LwM2M__ResourceDesc.md) | [**s\_device\_resources**](#variable-s_device_resources)   = `/* multi line expression */`<br>_Resource definitions for OMA LwM2M Device Object (Obj 3)._  |
|  const [**SYN\_LwM2M\_ResourceDesc**](structSYN__LwM2M__ResourceDesc.md) | [**s\_fw\_resources**](#variable-s_fw_resources)   = `/* multi line expression */`<br>_Resource definitions for OMA LwM2M Firmware Update Object (Obj 5)._  |
|  const uint16\_t | [**s\_single\_inst**](#variable-s_single_inst)   = `{0U}`<br>_Default single instance list (Instance 0)._  |
|  const [**SYN\_LwM2M\_ResourceDesc**](structSYN__LwM2M__ResourceDesc.md) | [**s\_temp\_resources**](#variable-s_temp_resources)   = `/* multi line expression */`<br>_Resource definitions for IPSO Temperature Sensor Object (Obj 3303)._  |














## Public Functions

| Type | Name |
| ---: | :--- |
|  size\_t | [**syn\_lwm2m\_build\_deregister\_request**](#function-syn_lwm2m_build_deregister_request) (const [**SYN\_LwM2M\_Client**](structSYN__LwM2M__Client.md) \* client, uint16\_t msg\_id, const uint8\_t \* token, uint8\_t token\_len, [**SYN\_CoapMsg**](structSYN__CoapMsg.md) \* req, [**SYN\_CoapOption**](structSYN__CoapOption.md) \* req\_opts, size\_t max\_req\_opts, size\_t \* req\_opt\_cnt) <br>_Build a CoAP Deregistration DELETE request (_ `/rd/<location>` _)._ |
|  size\_t | [**syn\_lwm2m\_build\_notify**](#function-syn_lwm2m_build_notify) ([**SYN\_LwM2M\_Client**](structSYN__LwM2M__Client.md) \* client, uint16\_t obj\_id, uint16\_t inst\_id, int32\_t res\_id, uint16\_t msg\_id, [**SYN\_CoapMsg**](structSYN__CoapMsg.md) \* resp, [**SYN\_CoapOption**](structSYN__CoapOption.md) \* resp\_opts, size\_t max\_resp\_opts, size\_t \* resp\_opt\_cnt, uint8\_t \* resp\_buf, size\_t resp\_buf\_sz) <br>_Build a CoAP Notification response for an active observation._  |
|  size\_t | [**syn\_lwm2m\_build\_register\_request**](#function-syn_lwm2m_build_register_request) (const [**SYN\_LwM2M\_Client**](structSYN__LwM2M__Client.md) \* client, uint16\_t msg\_id, const uint8\_t \* token, uint8\_t token\_len, [**SYN\_CoapMsg**](structSYN__CoapMsg.md) \* req, [**SYN\_CoapOption**](structSYN__CoapOption.md) \* req\_opts, size\_t max\_req\_opts, size\_t \* req\_opt\_cnt, uint8\_t \* payload\_buf, size\_t payload\_buf\_sz) <br>_Build a CoAP Registration POST request (_ `/rd?ep=...&lt=...` _)._ |
|  size\_t | [**syn\_lwm2m\_build\_update\_request**](#function-syn_lwm2m_build_update_request) (const [**SYN\_LwM2M\_Client**](structSYN__LwM2M__Client.md) \* client, uint16\_t msg\_id, const uint8\_t \* token, uint8\_t token\_len, uint32\_t lifetime\_s, [**SYN\_CoapMsg**](structSYN__CoapMsg.md) \* req, [**SYN\_CoapOption**](structSYN__CoapOption.md) \* req\_opts, size\_t max\_req\_opts, size\_t \* req\_opt\_cnt, uint8\_t \* payload\_buf, size\_t payload\_buf\_sz) <br>_Build a CoAP Registration Update POST request (_ `/rd/<location>` _)._ |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_lwm2m\_client\_init**](#function-syn_lwm2m_client_init) ([**SYN\_LwM2M\_Client**](structSYN__LwM2M__Client.md) \* client, const char \* endpoint\_name, uint32\_t lifetime\_s, [**SYN\_Transport**](structSYN__Transport.md) \* transport) <br>_Initialize a zero-heap LwM2M client context._  |
|  size\_t | [**syn\_lwm2m\_generate\_link\_format**](#function-syn_lwm2m_generate_link_format) (const [**SYN\_LwM2M\_Client**](structSYN__LwM2M__Client.md) \* client, char \* buf, size\_t max\_len) <br>_Generate CoRE Link Format string listing all registered object instances._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_lwm2m\_handle\_register\_response**](#function-syn_lwm2m_handle_register_response) ([**SYN\_LwM2M\_Client**](structSYN__LwM2M__Client.md) \* client, const [**SYN\_CoapMsg**](structSYN__CoapMsg.md) \* resp, const [**SYN\_CoapOption**](structSYN__CoapOption.md) \* resp\_opts, size\_t resp\_opt\_cnt) <br>_Handle server response to Registration request (parses Location-Path)._  |
|  [**SYN\_LwM2M\_Object**](structSYN__LwM2M__Object.md) | [**syn\_lwm2m\_make\_device\_object**](#function-syn_lwm2m_make_device_object) ([**SYN\_LwM2M\_DeviceContext**](structSYN__LwM2M__DeviceContext.md) \* ctx) <br>_Create an Object 3 (Device) descriptor bound to_ `ctx` _._ |
|  [**SYN\_LwM2M\_Object**](structSYN__LwM2M__Object.md) | [**syn\_lwm2m\_make\_firmware\_object**](#function-syn_lwm2m_make_firmware_object) ([**SYN\_LwM2M\_FirmwareContext**](structSYN__LwM2M__FirmwareContext.md) \* ctx) <br>_Create an Object 5 (Firmware Update) descriptor bound to_ `ctx` _._ |
|  [**SYN\_LwM2M\_Object**](structSYN__LwM2M__Object.md) | [**syn\_lwm2m\_make\_temperature\_object**](#function-syn_lwm2m_make_temperature_object) ([**SYN\_LwM2M\_SensorContext**](structSYN__LwM2M__SensorContext.md) \* ctx) <br>_Create an Object 3303 (Temperature Sensor) descriptor bound to_ `ctx` _._ |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_lwm2m\_observe\_add**](#function-syn_lwm2m_observe_add) ([**SYN\_LwM2M\_Client**](structSYN__LwM2M__Client.md) \* client, uint16\_t obj\_id, uint16\_t inst\_id, int32\_t res\_id, const uint8\_t \* token, uint8\_t token\_len, uint32\_t pmin, uint32\_t pmax) <br>_Add or update an observation for an object instance or resource._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_lwm2m\_observe\_remove**](#function-syn_lwm2m_observe_remove) ([**SYN\_LwM2M\_Client**](structSYN__LwM2M__Client.md) \* client, const uint8\_t \* token, uint8\_t token\_len) <br>_Remove an active observation by matching token._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_lwm2m\_process\_request**](#function-syn_lwm2m_process_request) ([**SYN\_LwM2M\_Client**](structSYN__LwM2M__Client.md) \* client, const [**SYN\_CoapMsg**](structSYN__CoapMsg.md) \* req, const [**SYN\_CoapOption**](structSYN__CoapOption.md) \* req\_opts, size\_t req\_opt\_cnt, [**SYN\_CoapMsg**](structSYN__CoapMsg.md) \* resp, [**SYN\_CoapOption**](structSYN__CoapOption.md) \* resp\_opts, size\_t max\_resp\_opts, size\_t \* resp\_opt\_cnt, uint8\_t \* resp\_buf, size\_t resp\_buf\_sz) <br>_Dispatch and process an incoming CoAP request against registered LwM2M objects._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_lwm2m\_register\_object**](#function-syn_lwm2m_register_object) ([**SYN\_LwM2M\_Client**](structSYN__LwM2M__Client.md) \* client, const [**SYN\_LwM2M\_Object**](structSYN__LwM2M__Object.md) \* obj) <br>_Register an object definition into the client object registry._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_lwm2m\_tlv\_decode**](#function-syn_lwm2m_tlv_decode) (const uint8\_t \* buf, size\_t buf\_len, [**SYN\_LwM2M\_TLV**](structSYN__LwM2M__TLV.md) \* tlv, size\_t \* consumed) <br>_Decode a single TLV item from a byte stream._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_lwm2m\_tlv\_decode\_bool**](#function-syn_lwm2m_tlv_decode_bool) (const [**SYN\_LwM2M\_TLV**](structSYN__LwM2M__TLV.md) \* tlv, [**bool**](syn__defs_8h.md#enum-bool) \* out\_val) <br>_Decode a boolean from TLV value bytes._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_lwm2m\_tlv\_decode\_float**](#function-syn_lwm2m_tlv_decode_float) (const [**SYN\_LwM2M\_TLV**](structSYN__LwM2M__TLV.md) \* tlv, double \* out\_val) <br>_Decode a float/double from TLV value bytes._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_lwm2m\_tlv\_decode\_int**](#function-syn_lwm2m_tlv_decode_int) (const [**SYN\_LwM2M\_TLV**](structSYN__LwM2M__TLV.md) \* tlv, int64\_t \* out\_val) <br>_Decode a signed integer from TLV value bytes._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_lwm2m\_tlv\_decode\_string**](#function-syn_lwm2m_tlv_decode_string) (const [**SYN\_LwM2M\_TLV**](structSYN__LwM2M__TLV.md) \* tlv, char \* out\_str, size\_t max\_len) <br>_Decode a string from TLV value bytes into a null-terminated buffer._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_lwm2m\_tlv\_decode\_value**](#function-syn_lwm2m_tlv_decode_value) (const [**SYN\_LwM2M\_TLV**](structSYN__LwM2M__TLV.md) \* tlv, [**SYN\_LwM2M\_ValType**](syn__lwm2m_8h.md#enum-syn_lwm2m_valtype) expected\_type, [**SYN\_LwM2M\_Value**](structSYN__LwM2M__Value.md) \* out\_val) <br>_Decode a TLV into a typed_ [_**SYN\_LwM2M\_Value**_](structSYN__LwM2M__Value.md) _container._ |
|  size\_t | [**syn\_lwm2m\_tlv\_encode**](#function-syn_lwm2m_tlv_encode) (const [**SYN\_LwM2M\_TLV**](structSYN__LwM2M__TLV.md) \* tlv, uint8\_t \* buf, size\_t max\_len) <br>_Encode a raw TLV header and value into a buffer._  |
|  size\_t | [**syn\_lwm2m\_tlv\_encode\_bool**](#function-syn_lwm2m_tlv_encode_bool) (uint16\_t res\_id, [**bool**](syn__defs_8h.md#enum-bool) val, uint8\_t \* buf, size\_t max\_len) <br>_Encode a boolean resource as TLV (1-byte int 0 or 1)._  |
|  size\_t | [**syn\_lwm2m\_tlv\_encode\_float**](#function-syn_lwm2m_tlv_encode_float) (uint16\_t res\_id, double val, uint8\_t \* buf, size\_t max\_len) <br>_Encode a float/double resource as TLV (Float32 or Float64)._  |
|  size\_t | [**syn\_lwm2m\_tlv\_encode\_instance**](#function-syn_lwm2m_tlv_encode_instance) (uint16\_t inst\_id, const uint8\_t \* tlv\_payload, size\_t payload\_len, uint8\_t \* buf, size\_t max\_len) <br>_Wrap nested resource TLVs inside an Object Instance TLV header._  |
|  size\_t | [**syn\_lwm2m\_tlv\_encode\_int**](#function-syn_lwm2m_tlv_encode_int) (uint16\_t res\_id, int64\_t val, uint8\_t \* buf, size\_t max\_len) <br>_Encode a 64-bit integer resource as TLV._  |
|  size\_t | [**syn\_lwm2m\_tlv\_encode\_opaque**](#function-syn_lwm2m_tlv_encode_opaque) (uint16\_t res\_id, const uint8\_t \* data, size\_t len, uint8\_t \* buf, size\_t max\_len) <br>_Encode opaque binary data resource as TLV._  |
|  size\_t | [**syn\_lwm2m\_tlv\_encode\_string**](#function-syn_lwm2m_tlv_encode_string) (uint16\_t res\_id, const char \* str, uint8\_t \* buf, size\_t max\_len) <br>_Encode a string resource as TLV._  |
|  size\_t | [**syn\_lwm2m\_tlv\_encode\_value**](#function-syn_lwm2m_tlv_encode_value) (uint16\_t res\_id, const [**SYN\_LwM2M\_Value**](structSYN__LwM2M__Value.md) \* val, uint8\_t \* buf, size\_t max\_len) <br>_Encode a generic_ [_**SYN\_LwM2M\_Value**_](structSYN__LwM2M__Value.md) _as TLV._ |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**device\_exec\_cb**](#function-device_exec_cb) (uint16\_t instance\_id, uint16\_t resource\_id, const uint8\_t \* args, size\_t args\_len, void \* user\_ctx) <br>_Execute callback for Device Object (Obj 3)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**device\_read\_cb**](#function-device_read_cb) (uint16\_t instance\_id, uint16\_t resource\_id, [**SYN\_LwM2M\_Value**](structSYN__LwM2M__Value.md) \* out\_val, void \* user\_ctx) <br>_Read callback for Device Object (Obj 3)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**device\_write\_cb**](#function-device_write_cb) (uint16\_t instance\_id, uint16\_t resource\_id, const [**SYN\_LwM2M\_Value**](structSYN__LwM2M__Value.md) \* in\_val, void \* user\_ctx) <br>_Write callback for Device Object (Obj 3)._  |
|  const [**SYN\_LwM2M\_Object**](structSYN__LwM2M__Object.md) \* | [**find\_object**](#function-find_object) (const [**SYN\_LwM2M\_Client**](structSYN__LwM2M__Client.md) \* client, uint16\_t obj\_id) <br>_Search client object registry for object matching ID._  |
|  const [**SYN\_LwM2M\_ResourceDesc**](structSYN__LwM2M__ResourceDesc.md) \* | [**find\_resource**](#function-find_resource) (const [**SYN\_LwM2M\_Object**](structSYN__LwM2M__Object.md) \* obj, uint16\_t res\_id) <br>_Find resource descriptor matching resource ID within an object._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**fw\_exec\_cb**](#function-fw_exec_cb) (uint16\_t instance\_id, uint16\_t resource\_id, const uint8\_t \* args, size\_t args\_len, void \* user\_ctx) <br>_Execute callback for Firmware Update Object (Obj 5)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**fw\_read\_cb**](#function-fw_read_cb) (uint16\_t instance\_id, uint16\_t resource\_id, [**SYN\_LwM2M\_Value**](structSYN__LwM2M__Value.md) \* out\_val, void \* user\_ctx) <br>_Read callback for Firmware Update Object (Obj 5)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**fw\_write\_cb**](#function-fw_write_cb) (uint16\_t instance\_id, uint16\_t resource\_id, const [**SYN\_LwM2M\_Value**](structSYN__LwM2M__Value.md) \* in\_val, void \* user\_ctx) <br>_Write callback for Firmware Update Object (Obj 5)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**temp\_read\_cb**](#function-temp_read_cb) (uint16\_t instance\_id, uint16\_t resource\_id, [**SYN\_LwM2M\_Value**](structSYN__LwM2M__Value.md) \* out\_val, void \* user\_ctx) <br>_Read callback for IPSO Temperature Sensor Object (Obj 3303)._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**validate\_instance**](#function-validate_instance) (const [**SYN\_LwM2M\_Object**](structSYN__LwM2M__Object.md) \* obj, uint16\_t inst\_id) <br>_Validate that an object instance ID exists in the object definition._  |


























## Public Static Attributes Documentation




### variable s\_device\_resources 

_Resource definitions for OMA LwM2M Device Object (Obj 3)._ 
```C++
const SYN_LwM2M_ResourceDesc s_device_resources[];
```




<hr>



### variable s\_fw\_resources 

_Resource definitions for OMA LwM2M Firmware Update Object (Obj 5)._ 
```C++
const SYN_LwM2M_ResourceDesc s_fw_resources[];
```




<hr>



### variable s\_single\_inst 

_Default single instance list (Instance 0)._ 
```C++
const uint16_t s_single_inst[1];
```




<hr>



### variable s\_temp\_resources 

_Resource definitions for IPSO Temperature Sensor Object (Obj 3303)._ 
```C++
const SYN_LwM2M_ResourceDesc s_temp_resources[];
```




<hr>
## Public Functions Documentation




### function syn\_lwm2m\_build\_deregister\_request 

_Build a CoAP Deregistration DELETE request (_ `/rd/<location>` _)._
```C++
size_t syn_lwm2m_build_deregister_request (
    const SYN_LwM2M_Client * client,
    uint16_t msg_id,
    const uint8_t * token,
    uint8_t token_len,
    SYN_CoapMsg * req,
    SYN_CoapOption * req_opts,
    size_t max_req_opts,
    size_t * req_opt_cnt
) 
```





**Parameters:**


* `client` LwM2M client context. 
* `msg_id` CoAP message ID. 
* `token` CoAP token bytes. 
* `token_len` Length of CoAP token. 
* `req` [out] Populated CoAP request header. 
* `req_opts` [out] Populated CoAP options array. 
* `max_req_opts` Capacity of options array. 
* `req_opt_cnt` [out] Number of populated options. 



**Returns:**

0 on success. 





        

<hr>



### function syn\_lwm2m\_build\_notify 

_Build a CoAP Notification response for an active observation._ 
```C++
size_t syn_lwm2m_build_notify (
    SYN_LwM2M_Client * client,
    uint16_t obj_id,
    uint16_t inst_id,
    int32_t res_id,
    uint16_t msg_id,
    SYN_CoapMsg * resp,
    SYN_CoapOption * resp_opts,
    size_t max_resp_opts,
    size_t * resp_opt_cnt,
    uint8_t * resp_buf,
    size_t resp_buf_sz
) 
```





**Parameters:**


* `client` LwM2M client context. 
* `obj_id` Object ID. 
* `inst_id` Instance ID. 
* `res_id` Resource ID (-1 for entire instance). 
* `msg_id` CoAP message ID. 
* `resp` [out] Populated CoAP notification response header. 
* `resp_opts` [out] Output response options array. 
* `max_resp_opts` Capacity of response options array. 
* `resp_opt_cnt` [out] Number of populated response options. 
* `resp_buf` [out] Buffer for notification payload. 
* `resp_buf_sz` Capacity of notification buffer. 



**Returns:**

Length of encoded payload in bytes, or 0 on error. 





        

<hr>



### function syn\_lwm2m\_build\_register\_request 

_Build a CoAP Registration POST request (_ `/rd?ep=...&lt=...` _)._
```C++
size_t syn_lwm2m_build_register_request (
    const SYN_LwM2M_Client * client,
    uint16_t msg_id,
    const uint8_t * token,
    uint8_t token_len,
    SYN_CoapMsg * req,
    SYN_CoapOption * req_opts,
    size_t max_req_opts,
    size_t * req_opt_cnt,
    uint8_t * payload_buf,
    size_t payload_buf_sz
) 
```





**Parameters:**


* `client` LwM2M client context. 
* `msg_id` CoAP message ID. 
* `token` CoAP token bytes. 
* `token_len` Length of CoAP token. 
* `req` [out] Populated CoAP request header. 
* `req_opts` [out] Populated CoAP options array. 
* `max_req_opts` Capacity of options array. 
* `req_opt_cnt` [out] Number of populated options. 
* `payload_buf` [out] Buffer for link format payload. 
* `payload_buf_sz` Capacity of payload buffer. 



**Returns:**

Length of serialized payload in bytes, or 0 on error. 





        

<hr>



### function syn\_lwm2m\_build\_update\_request 

_Build a CoAP Registration Update POST request (_ `/rd/<location>` _)._
```C++
size_t syn_lwm2m_build_update_request (
    const SYN_LwM2M_Client * client,
    uint16_t msg_id,
    const uint8_t * token,
    uint8_t token_len,
    uint32_t lifetime_s,
    SYN_CoapMsg * req,
    SYN_CoapOption * req_opts,
    size_t max_req_opts,
    size_t * req_opt_cnt,
    uint8_t * payload_buf,
    size_t payload_buf_sz
) 
```





**Parameters:**


* `client` LwM2M client context. 
* `msg_id` CoAP message ID. 
* `token` CoAP token bytes. 
* `token_len` Length of CoAP token. 
* `lifetime_s` Updated registration lifetime in seconds (0 to keep current). 
* `req` [out] Populated CoAP request header. 
* `req_opts` [out] Populated CoAP options array. 
* `max_req_opts` Capacity of options array. 
* `req_opt_cnt` [out] Number of populated options. 
* `payload_buf` [out] Optional buffer for updated link format. 
* `payload_buf_sz` Capacity of payload buffer. 



**Returns:**

Length of payload in bytes (0 if lifetime update only). 





        

<hr>



### function syn\_lwm2m\_client\_init 

_Initialize a zero-heap LwM2M client context._ 
```C++
bool syn_lwm2m_client_init (
    SYN_LwM2M_Client * client,
    const char * endpoint_name,
    uint32_t lifetime_s,
    SYN_Transport * transport
) 
```





**Parameters:**


* `client` Client context to initialize. 
* `endpoint_name` Client endpoint name (e.g. "urn:imei:862415039201923"). 
* `lifetime_s` Registration lifetime in seconds (e.g. 300). 
* `transport` Underlying datagram transport. 



**Returns:**

true on success, false on invalid parameters. 





        

<hr>



### function syn\_lwm2m\_generate\_link\_format 

_Generate CoRE Link Format string listing all registered object instances._ 
```C++
size_t syn_lwm2m_generate_link_format (
    const SYN_LwM2M_Client * client,
    char * buf,
    size_t max_len
) 
```



Example output: `</1/0>, </3/0>, </5/0>, </3303/0>`




**Parameters:**


* `client` LwM2M client context. 
* `buf` [out] Buffer to receive link format string. 
* `max_len` Capacity of link format buffer. 



**Returns:**

Number of characters written (excluding null terminator), or 0 on overflow. 





        

<hr>



### function syn\_lwm2m\_handle\_register\_response 

_Handle server response to Registration request (parses Location-Path)._ 
```C++
bool syn_lwm2m_handle_register_response (
    SYN_LwM2M_Client * client,
    const SYN_CoapMsg * resp,
    const SYN_CoapOption * resp_opts,
    size_t resp_opt_cnt
) 
```





**Parameters:**


* `client` LwM2M client context. 
* `resp` Received CoAP response. 
* `resp_opts` Received response options. 
* `resp_opt_cnt` Number of response options. 



**Returns:**

true if registration succeeded (2.01 Created), false otherwise. 





        

<hr>



### function syn\_lwm2m\_make\_device\_object 

_Create an Object 3 (Device) descriptor bound to_ `ctx` _._
```C++
SYN_LwM2M_Object syn_lwm2m_make_device_object (
    SYN_LwM2M_DeviceContext * ctx
) 
```





**Parameters:**


* `ctx` Device object context state. 



**Returns:**

Initialized LwM2M object descriptor. 





        

<hr>



### function syn\_lwm2m\_make\_firmware\_object 

_Create an Object 5 (Firmware Update) descriptor bound to_ `ctx` _._
```C++
SYN_LwM2M_Object syn_lwm2m_make_firmware_object (
    SYN_LwM2M_FirmwareContext * ctx
) 
```





**Parameters:**


* `ctx` Firmware update object context state. 



**Returns:**

Initialized LwM2M object descriptor. 





        

<hr>



### function syn\_lwm2m\_make\_temperature\_object 

_Create an Object 3303 (Temperature Sensor) descriptor bound to_ `ctx` _._
```C++
SYN_LwM2M_Object syn_lwm2m_make_temperature_object (
    SYN_LwM2M_SensorContext * ctx
) 
```





**Parameters:**


* `ctx` Temperature sensor object context state. 



**Returns:**

Initialized LwM2M object descriptor. 





        

<hr>



### function syn\_lwm2m\_observe\_add 

_Add or update an observation for an object instance or resource._ 
```C++
bool syn_lwm2m_observe_add (
    SYN_LwM2M_Client * client,
    uint16_t obj_id,
    uint16_t inst_id,
    int32_t res_id,
    const uint8_t * token,
    uint8_t token_len,
    uint32_t pmin,
    uint32_t pmax
) 
```





**Parameters:**


* `client` LwM2M client context. 
* `obj_id` Object ID. 
* `inst_id` Instance ID. 
* `res_id` Resource ID (-1 for entire instance). 
* `token` CoAP observation token. 
* `token_len` Length of token. 
* `pmin` Minimum period in seconds (0 = immediate). 
* `pmax` Maximum period in seconds (0 = none). 



**Returns:**

true on success, false if table full. 





        

<hr>



### function syn\_lwm2m\_observe\_remove 

_Remove an active observation by matching token._ 
```C++
bool syn_lwm2m_observe_remove (
    SYN_LwM2M_Client * client,
    const uint8_t * token,
    uint8_t token_len
) 
```





**Parameters:**


* `client` LwM2M client context. 
* `token` CoAP token bytes. 
* `token_len` Length of token. 



**Returns:**

true if an entry was found and removed, false otherwise. 





        

<hr>



### function syn\_lwm2m\_process\_request 

_Dispatch and process an incoming CoAP request against registered LwM2M objects._ 
```C++
SYN_Status syn_lwm2m_process_request (
    SYN_LwM2M_Client * client,
    const SYN_CoapMsg * req,
    const SYN_CoapOption * req_opts,
    size_t req_opt_cnt,
    SYN_CoapMsg * resp,
    SYN_CoapOption * resp_opts,
    size_t max_resp_opts,
    size_t * resp_opt_cnt,
    uint8_t * resp_buf,
    size_t resp_buf_sz
) 
```



Handles GET (Read / Discover), PUT/POST (Write), POST (Execute), DELETE.




**Parameters:**


* `client` LwM2M client context. 
* `req` Incoming CoAP request. 
* `req_opts` Request options (Uri-Path, Content-Format, Observe, etc.). 
* `req_opt_cnt` Number of request options. 
* `resp` [out] Prepared CoAP response. 
* `resp_opts` [out] Output response options array. 
* `max_resp_opts` Capacity of response options array. 
* `resp_opt_cnt` [out] Number of populated response options. 
* `resp_buf` [out] Buffer for response payload. 
* `resp_buf_sz` Capacity of response buffer. 



**Returns:**

SYN\_OK on successful request handling, error code otherwise. 





        

<hr>



### function syn\_lwm2m\_register\_object 

_Register an object definition into the client object registry._ 
```C++
bool syn_lwm2m_register_object (
    SYN_LwM2M_Client * client,
    const SYN_LwM2M_Object * obj
) 
```





**Parameters:**


* `client` LwM2M client context. 
* `obj` Object definition descriptor. 



**Returns:**

true on success, false if registry is full or invalid parameters. 





        

<hr>



### function syn\_lwm2m\_tlv\_decode 

_Decode a single TLV item from a byte stream._ 
```C++
bool syn_lwm2m_tlv_decode (
    const uint8_t * buf,
    size_t buf_len,
    SYN_LwM2M_TLV * tlv,
    size_t * consumed
) 
```





**Parameters:**


* `buf` Input buffer. 
* `buf_len` Available bytes in buffer. 
* `tlv` [out] Decoded TLV header and value pointer. 
* `consumed` [out] Total bytes consumed for this TLV. 



**Returns:**

true on success, false on format/length error. 





        

<hr>



### function syn\_lwm2m\_tlv\_decode\_bool 

_Decode a boolean from TLV value bytes._ 
```C++
bool syn_lwm2m_tlv_decode_bool (
    const SYN_LwM2M_TLV * tlv,
    bool * out_val
) 
```





**Parameters:**


* `tlv` Decoded TLV descriptor. 
* `out_val` [out] Decoded boolean. 



**Returns:**

true on success, false on length error or null parameters. 





        

<hr>



### function syn\_lwm2m\_tlv\_decode\_float 

_Decode a float/double from TLV value bytes._ 
```C++
bool syn_lwm2m_tlv_decode_float (
    const SYN_LwM2M_TLV * tlv,
    double * out_val
) 
```





**Parameters:**


* `tlv` Decoded TLV descriptor. 
* `out_val` [out] Decoded double. 



**Returns:**

true on success, false on length error or null parameters. 





        

<hr>



### function syn\_lwm2m\_tlv\_decode\_int 

_Decode a signed integer from TLV value bytes._ 
```C++
bool syn_lwm2m_tlv_decode_int (
    const SYN_LwM2M_TLV * tlv,
    int64_t * out_val
) 
```





**Parameters:**


* `tlv` Decoded TLV descriptor. 
* `out_val` [out] Decoded 64-bit integer. 



**Returns:**

true on success, false on length error or null parameters. 





        

<hr>



### function syn\_lwm2m\_tlv\_decode\_string 

_Decode a string from TLV value bytes into a null-terminated buffer._ 
```C++
bool syn_lwm2m_tlv_decode_string (
    const SYN_LwM2M_TLV * tlv,
    char * out_str,
    size_t max_len
) 
```





**Parameters:**


* `tlv` Decoded TLV descriptor. 
* `out_str` [out] Target buffer for string. 
* `max_len` Capacity of target buffer. 



**Returns:**

true on success, false on overflow or null parameters. 





        

<hr>



### function syn\_lwm2m\_tlv\_decode\_value 

_Decode a TLV into a typed_ [_**SYN\_LwM2M\_Value**_](structSYN__LwM2M__Value.md) _container._
```C++
bool syn_lwm2m_tlv_decode_value (
    const SYN_LwM2M_TLV * tlv,
    SYN_LwM2M_ValType expected_type,
    SYN_LwM2M_Value * out_val
) 
```





**Parameters:**


* `tlv` Decoded TLV descriptor. 
* `expected_type` Expected value type. 
* `out_val` [out] Decoded value container. 



**Returns:**

true on success, false on decoding failure. 





        

<hr>



### function syn\_lwm2m\_tlv\_encode 

_Encode a raw TLV header and value into a buffer._ 
```C++
size_t syn_lwm2m_tlv_encode (
    const SYN_LwM2M_TLV * tlv,
    uint8_t * buf,
    size_t max_len
) 
```





**Parameters:**


* `tlv` TLV item to encode. 
* `buf` [out] Target buffer. 
* `max_len` Capacity of target buffer. 



**Returns:**

Number of bytes written, or 0 on buffer overflow. 





        

<hr>



### function syn\_lwm2m\_tlv\_encode\_bool 

_Encode a boolean resource as TLV (1-byte int 0 or 1)._ 
```C++
size_t syn_lwm2m_tlv_encode_bool (
    uint16_t res_id,
    bool val,
    uint8_t * buf,
    size_t max_len
) 
```





**Parameters:**


* `res_id` Resource ID. 
* `val` Boolean value. 
* `buf` [out] Target buffer. 
* `max_len` Capacity of target buffer. 



**Returns:**

Number of bytes written, or 0 on overflow. 





        

<hr>



### function syn\_lwm2m\_tlv\_encode\_float 

_Encode a float/double resource as TLV (Float32 or Float64)._ 
```C++
size_t syn_lwm2m_tlv_encode_float (
    uint16_t res_id,
    double val,
    uint8_t * buf,
    size_t max_len
) 
```





**Parameters:**


* `res_id` Resource ID. 
* `val` Floating-point value. 
* `buf` [out] Target buffer. 
* `max_len` Capacity of target buffer. 



**Returns:**

Number of bytes written, or 0 on overflow. 





        

<hr>



### function syn\_lwm2m\_tlv\_encode\_instance 

_Wrap nested resource TLVs inside an Object Instance TLV header._ 
```C++
size_t syn_lwm2m_tlv_encode_instance (
    uint16_t inst_id,
    const uint8_t * tlv_payload,
    size_t payload_len,
    uint8_t * buf,
    size_t max_len
) 
```





**Parameters:**


* `inst_id` Object instance ID. 
* `tlv_payload` Serialized resource TLVs buffer. 
* `payload_len` Length of resource TLVs. 
* `buf` [out] Target buffer. 
* `max_len` Capacity of target buffer. 



**Returns:**

Number of bytes written, or 0 on overflow. 





        

<hr>



### function syn\_lwm2m\_tlv\_encode\_int 

_Encode a 64-bit integer resource as TLV._ 
```C++
size_t syn_lwm2m_tlv_encode_int (
    uint16_t res_id,
    int64_t val,
    uint8_t * buf,
    size_t max_len
) 
```





**Parameters:**


* `res_id` Resource ID. 
* `val` Integer value to encode. 
* `buf` [out] Target buffer. 
* `max_len` Capacity of target buffer. 



**Returns:**

Number of bytes written, or 0 on overflow. 





        

<hr>



### function syn\_lwm2m\_tlv\_encode\_opaque 

_Encode opaque binary data resource as TLV._ 
```C++
size_t syn_lwm2m_tlv_encode_opaque (
    uint16_t res_id,
    const uint8_t * data,
    size_t len,
    uint8_t * buf,
    size_t max_len
) 
```





**Parameters:**


* `res_id` Resource ID. 
* `data` Binary buffer. 
* `len` Length of binary data. 
* `buf` [out] Target buffer. 
* `max_len` Capacity of target buffer. 



**Returns:**

Number of bytes written, or 0 on overflow. 





        

<hr>



### function syn\_lwm2m\_tlv\_encode\_string 

_Encode a string resource as TLV._ 
```C++
size_t syn_lwm2m_tlv_encode_string (
    uint16_t res_id,
    const char * str,
    uint8_t * buf,
    size_t max_len
) 
```





**Parameters:**


* `res_id` Resource ID. 
* `str` Null-terminated string. 
* `buf` [out] Target buffer. 
* `max_len` Capacity of target buffer. 



**Returns:**

Number of bytes written, or 0 on overflow. 





        

<hr>



### function syn\_lwm2m\_tlv\_encode\_value 

_Encode a generic_ [_**SYN\_LwM2M\_Value**_](structSYN__LwM2M__Value.md) _as TLV._
```C++
size_t syn_lwm2m_tlv_encode_value (
    uint16_t res_id,
    const SYN_LwM2M_Value * val,
    uint8_t * buf,
    size_t max_len
) 
```





**Parameters:**


* `res_id` Resource ID. 
* `val` Typed resource value container. 
* `buf` [out] Target buffer. 
* `max_len` Capacity of target buffer. 



**Returns:**

Number of bytes written, or 0 on overflow. 





        

<hr>
## Public Static Functions Documentation




### function device\_exec\_cb 

_Execute callback for Device Object (Obj 3)._ 
```C++
static SYN_Status device_exec_cb (
    uint16_t instance_id,
    uint16_t resource_id,
    const uint8_t * args,
    size_t args_len,
    void * user_ctx
) 
```





**Parameters:**


* `instance_id` Instance ID. 
* `resource_id` Resource ID. 
* `args` Execute arguments. 
* `args_len` Arguments length. 
* `user_ctx` Device context pointer. 



**Returns:**

SYN\_OK on success, SYN\_ERROR on unsupported resource or NULL pointers. 





        

<hr>



### function device\_read\_cb 

_Read callback for Device Object (Obj 3)._ 
```C++
static SYN_Status device_read_cb (
    uint16_t instance_id,
    uint16_t resource_id,
    SYN_LwM2M_Value * out_val,
    void * user_ctx
) 
```





**Parameters:**


* `instance_id` Instance ID. 
* `resource_id` Resource ID. 
* `out_val` [out] Value container. 
* `user_ctx` Device context pointer. 



**Returns:**

SYN\_OK on success, SYN\_ERROR on unknown resource or NULL pointers. 





        

<hr>



### function device\_write\_cb 

_Write callback for Device Object (Obj 3)._ 
```C++
static SYN_Status device_write_cb (
    uint16_t instance_id,
    uint16_t resource_id,
    const SYN_LwM2M_Value * in_val,
    void * user_ctx
) 
```





**Parameters:**


* `instance_id` Instance ID. 
* `resource_id` Resource ID. 
* `in_val` Input value. 
* `user_ctx` Device context pointer. 



**Returns:**

SYN\_OK on success, SYN\_ERROR on unknown resource or NULL pointers. 





        

<hr>



### function find\_object 

_Search client object registry for object matching ID._ 
```C++
static const SYN_LwM2M_Object * find_object (
    const SYN_LwM2M_Client * client,
    uint16_t obj_id
) 
```





**Parameters:**


* `client` LwM2M client context. 
* `obj_id` Object ID. 



**Returns:**

Pointer to object descriptor if found, NULL otherwise. 





        

<hr>



### function find\_resource 

_Find resource descriptor matching resource ID within an object._ 
```C++
static const SYN_LwM2M_ResourceDesc * find_resource (
    const SYN_LwM2M_Object * obj,
    uint16_t res_id
) 
```





**Parameters:**


* `obj` Object descriptor. 
* `res_id` Resource ID. 



**Returns:**

Pointer to resource descriptor if found, NULL otherwise. 





        

<hr>



### function fw\_exec\_cb 

_Execute callback for Firmware Update Object (Obj 5)._ 
```C++
static SYN_Status fw_exec_cb (
    uint16_t instance_id,
    uint16_t resource_id,
    const uint8_t * args,
    size_t args_len,
    void * user_ctx
) 
```





**Parameters:**


* `instance_id` Instance ID. 
* `resource_id` Resource ID. 
* `args` Execute arguments. 
* `args_len` Arguments length. 
* `user_ctx` Firmware context pointer. 



**Returns:**

SYN\_OK on success, SYN\_ERROR on unsupported resource or NULL pointers. 





        

<hr>



### function fw\_read\_cb 

_Read callback for Firmware Update Object (Obj 5)._ 
```C++
static SYN_Status fw_read_cb (
    uint16_t instance_id,
    uint16_t resource_id,
    SYN_LwM2M_Value * out_val,
    void * user_ctx
) 
```





**Parameters:**


* `instance_id` Instance ID. 
* `resource_id` Resource ID. 
* `out_val` [out] Value container. 
* `user_ctx` Firmware context pointer. 



**Returns:**

SYN\_OK on success, SYN\_ERROR on unknown resource or NULL pointers. 





        

<hr>



### function fw\_write\_cb 

_Write callback for Firmware Update Object (Obj 5)._ 
```C++
static SYN_Status fw_write_cb (
    uint16_t instance_id,
    uint16_t resource_id,
    const SYN_LwM2M_Value * in_val,
    void * user_ctx
) 
```





**Parameters:**


* `instance_id` Instance ID. 
* `resource_id` Resource ID. 
* `in_val` Input value. 
* `user_ctx` Firmware context pointer. 



**Returns:**

SYN\_OK on success, SYN\_ERROR on unknown resource or NULL pointers. 





        

<hr>



### function temp\_read\_cb 

_Read callback for IPSO Temperature Sensor Object (Obj 3303)._ 
```C++
static SYN_Status temp_read_cb (
    uint16_t instance_id,
    uint16_t resource_id,
    SYN_LwM2M_Value * out_val,
    void * user_ctx
) 
```





**Parameters:**


* `instance_id` Instance ID. 
* `resource_id` Resource ID. 
* `out_val` [out] Value container. 
* `user_ctx` Sensor context pointer. 



**Returns:**

SYN\_OK on success, SYN\_ERROR on unknown resource or NULL pointers. 





        

<hr>



### function validate\_instance 

_Validate that an object instance ID exists in the object definition._ 
```C++
static bool validate_instance (
    const SYN_LwM2M_Object * obj,
    uint16_t inst_id
) 
```





**Parameters:**


* `obj` Object descriptor. 
* `inst_id` Instance ID. 



**Returns:**

true if instance exists, false otherwise. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_lwm2m.c`

