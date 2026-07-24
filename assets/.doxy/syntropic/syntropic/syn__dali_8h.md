

# File syn\_dali.h



[**FileList**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_dali.h**](syn__dali_8h.md)

[Go to the source code of this file](syn__dali_8h_source.md)

_Digital Addressable Lighting Interface (DALI / IEC 62386) Protocol Stack._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include <stdint.h>`
* `#include <stdbool.h>`
* `#include <stddef.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_DALI\_BackwardFrame**](structSYN__DALI__BackwardFrame.md) <br>_DALI Backward Frame (Control Gear -&gt; Master, 8-bit payload)_  |
| struct | [**SYN\_DALI\_ForwardFrame**](structSYN__DALI__ForwardFrame.md) <br>_DALI Forward Frame (Master -&gt; Control Gear, 16-bit payload)_  |
| struct | [**SYN\_DALI\_SlaveConfig**](structSYN__DALI__SlaveConfig.md) <br>_DALI Slave Configuration._  |
| struct | [**SYN\_DALI\_SlaveState**](structSYN__DALI__SlaveState.md) <br>_DALI Slave Runtime State._  |


## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**SYN\_DALI\_AddressType**](#enum-syn_dali_addresstype)  <br>_DALI Address Types._  |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_dali\_decode\_backward**](#function-syn_dali_decode_backward) (uint8\_t raw\_8, [**SYN\_DALI\_BackwardFrame**](structSYN__DALI__BackwardFrame.md) \* frame) <br>_Decode an 8-bit Slave Backward Frame._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_dali\_decode\_forward**](#function-syn_dali_decode_forward) (uint16\_t raw\_16, [**SYN\_DALI\_ForwardFrame**](structSYN__DALI__ForwardFrame.md) \* frame) <br>_Decode a 16-bit Master Forward Frame._  |
|  uint8\_t | [**syn\_dali\_encode\_backward**](#function-syn_dali_encode_backward) (uint8\_t data) <br>_Encode an 8-bit Slave Backward Frame._  |
|  uint16\_t | [**syn\_dali\_encode\_forward**](#function-syn_dali_encode_forward) (uint8\_t addr, uint8\_t data\_cmd) <br>_Encode a 16-bit Master Forward Frame._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_dali\_manchester\_decode\_byte**](#function-syn_dali_manchester_decode_byte) (const uint8\_t \* bit\_in, uint8\_t \* val\_out) <br>_Decode Manchester bi-phase bit pairs into a byte value._  |
|  size\_t | [**syn\_dali\_manchester\_encode\_byte**](#function-syn_dali_manchester_encode_byte) (uint8\_t val, uint8\_t \* bit\_out) <br>_Encode a byte sequence into Manchester bi-phase bits._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_dali\_slave\_init**](#function-syn_dali_slave_init) ([**SYN\_DALI\_SlaveState**](structSYN__DALI__SlaveState.md) \* slave, const [**SYN\_DALI\_SlaveConfig**](structSYN__DALI__SlaveConfig.md) \* cfg) <br>_Initialize a DALI Slave Control Gear instance._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_dali\_slave\_process**](#function-syn_dali_slave_process) ([**SYN\_DALI\_SlaveState**](structSYN__DALI__SlaveState.md) \* slave, const [**SYN\_DALI\_ForwardFrame**](structSYN__DALI__ForwardFrame.md) \* req, uint8\_t \* resp\_data, [**bool**](syn__defs_8h.md#enum-bool) \* has\_resp) <br>_Process an incoming DALI Forward Frame on a Slave Control Gear node._  |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_DALI\_CMD\_ADD\_TO\_GROUP\_BASE**](syn__dali_8h.md#define-syn_dali_cmd_add_to_group_base)  `0x60U /\* 0x60..0x6F \*/`<br> |
| define  | [**SYN\_DALI\_CMD\_DOWN**](syn__dali_8h.md#define-syn_dali_cmd_down)  `0x02U`<br> |
| define  | [**SYN\_DALI\_CMD\_ENABLE\_WRITE\_MEMORY**](syn__dali_8h.md#define-syn_dali_cmd_enable_write_memory)  `0x81U`<br> |
| define  | [**SYN\_DALI\_CMD\_GO\_TO\_SCENE\_BASE**](syn__dali_8h.md#define-syn_dali_cmd_go_to_scene_base)  `0x10U /\* 0x10..0x1F \*/`<br> |
| define  | [**SYN\_DALI\_CMD\_OFF**](syn__dali_8h.md#define-syn_dali_cmd_off)  `0x00U`<br> |
| define  | [**SYN\_DALI\_CMD\_ON\_AND\_STEP\_UP**](syn__dali_8h.md#define-syn_dali_cmd_on_and_step_up)  `0x08U`<br> |
| define  | [**SYN\_DALI\_CMD\_QUERY\_ACTUAL\_LEVEL**](syn__dali_8h.md#define-syn_dali_cmd_query_actual_level)  `0xA0U`<br> |
| define  | [**SYN\_DALI\_CMD\_QUERY\_CONTENT\_DTR**](syn__dali_8h.md#define-syn_dali_cmd_query_content_dtr)  `0x98U`<br> |
| define  | [**SYN\_DALI\_CMD\_QUERY\_CONTENT\_DTR1**](syn__dali_8h.md#define-syn_dali_cmd_query_content_dtr1)  `0x9CU`<br> |
| define  | [**SYN\_DALI\_CMD\_QUERY\_CONTENT\_DTR2**](syn__dali_8h.md#define-syn_dali_cmd_query_content_dtr2)  `0x9DU`<br> |
| define  | [**SYN\_DALI\_CMD\_QUERY\_CONTROL\_GEAR**](syn__dali_8h.md#define-syn_dali_cmd_query_control_gear)  `0x91U`<br> |
| define  | [**SYN\_DALI\_CMD\_QUERY\_DEVICE\_TYPE**](syn__dali_8h.md#define-syn_dali_cmd_query_device_type)  `0x99U`<br> |
| define  | [**SYN\_DALI\_CMD\_QUERY\_GROUPS\_0\_7**](syn__dali_8h.md#define-syn_dali_cmd_query_groups_0_7)  `0xC0U`<br> |
| define  | [**SYN\_DALI\_CMD\_QUERY\_GROUPS\_8\_15**](syn__dali_8h.md#define-syn_dali_cmd_query_groups_8_15)  `0xC1U`<br> |
| define  | [**SYN\_DALI\_CMD\_QUERY\_LAMP\_FAILURE**](syn__dali_8h.md#define-syn_dali_cmd_query_lamp_failure)  `0x92U`<br> |
| define  | [**SYN\_DALI\_CMD\_QUERY\_LAMP\_POWER\_ON**](syn__dali_8h.md#define-syn_dali_cmd_query_lamp_power_on)  `0x93U`<br> |
| define  | [**SYN\_DALI\_CMD\_QUERY\_MAX\_LEVEL**](syn__dali_8h.md#define-syn_dali_cmd_query_max_level)  `0xA1U`<br> |
| define  | [**SYN\_DALI\_CMD\_QUERY\_MIN\_LEVEL**](syn__dali_8h.md#define-syn_dali_cmd_query_min_level)  `0xA2U`<br> |
| define  | [**SYN\_DALI\_CMD\_QUERY\_PHYSICAL\_MIN\_LEVEL**](syn__dali_8h.md#define-syn_dali_cmd_query_physical_min_level)  `0x9AU`<br> |
| define  | [**SYN\_DALI\_CMD\_QUERY\_POWER\_ON\_LEVEL**](syn__dali_8h.md#define-syn_dali_cmd_query_power_on_level)  `0xA3U`<br> |
| define  | [**SYN\_DALI\_CMD\_QUERY\_RANDOM\_ADDR\_H**](syn__dali_8h.md#define-syn_dali_cmd_query_random_addr_h)  `0xC2U`<br> |
| define  | [**SYN\_DALI\_CMD\_QUERY\_RANDOM\_ADDR\_L**](syn__dali_8h.md#define-syn_dali_cmd_query_random_addr_l)  `0xC4U`<br> |
| define  | [**SYN\_DALI\_CMD\_QUERY\_RANDOM\_ADDR\_M**](syn__dali_8h.md#define-syn_dali_cmd_query_random_addr_m)  `0xC3U`<br> |
| define  | [**SYN\_DALI\_CMD\_QUERY\_STATUS**](syn__dali_8h.md#define-syn_dali_cmd_query_status)  `0x90U`<br> |
| define  | [**SYN\_DALI\_CMD\_QUERY\_SYS\_FAIL\_LEVEL**](syn__dali_8h.md#define-syn_dali_cmd_query_sys_fail_level)  `0xA4U`<br> |
| define  | [**SYN\_DALI\_CMD\_RECALL\_MAX**](syn__dali_8h.md#define-syn_dali_cmd_recall_max)  `0x05U`<br> |
| define  | [**SYN\_DALI\_CMD\_RECALL\_MIN**](syn__dali_8h.md#define-syn_dali_cmd_recall_min)  `0x06U`<br> |
| define  | [**SYN\_DALI\_CMD\_REMOVE\_FROM\_GROUP\_BASE**](syn__dali_8h.md#define-syn_dali_cmd_remove_from_group_base)  `0x70U /\* 0x70..0x7F \*/`<br> |
| define  | [**SYN\_DALI\_CMD\_REMOVE\_FROM\_SCENE\_BASE**](syn__dali_8h.md#define-syn_dali_cmd_remove_from_scene_base)  `0x50U /\* 0x50..0x5F \*/`<br> |
| define  | [**SYN\_DALI\_CMD\_RESET**](syn__dali_8h.md#define-syn_dali_cmd_reset)  `0x20U`<br> |
| define  | [**SYN\_DALI\_CMD\_STEP\_DOWN**](syn__dali_8h.md#define-syn_dali_cmd_step_down)  `0x04U`<br> |
| define  | [**SYN\_DALI\_CMD\_STEP\_DOWN\_AND\_OFF**](syn__dali_8h.md#define-syn_dali_cmd_step_down_and_off)  `0x07U`<br> |
| define  | [**SYN\_DALI\_CMD\_STEP\_UP**](syn__dali_8h.md#define-syn_dali_cmd_step_up)  `0x03U`<br> |
| define  | [**SYN\_DALI\_CMD\_STORE\_ACTUAL\_LEVEL\_IN\_DTR**](syn__dali_8h.md#define-syn_dali_cmd_store_actual_level_in_dtr)  `0x21U`<br> |
| define  | [**SYN\_DALI\_CMD\_STORE\_DTR\_AS\_FADE\_RATE**](syn__dali_8h.md#define-syn_dali_cmd_store_dtr_as_fade_rate)  `0x2FU`<br> |
| define  | [**SYN\_DALI\_CMD\_STORE\_DTR\_AS\_FADE\_TIME**](syn__dali_8h.md#define-syn_dali_cmd_store_dtr_as_fade_time)  `0x2EU`<br> |
| define  | [**SYN\_DALI\_CMD\_STORE\_DTR\_AS\_MAX\_LEVEL**](syn__dali_8h.md#define-syn_dali_cmd_store_dtr_as_max_level)  `0x2AU`<br> |
| define  | [**SYN\_DALI\_CMD\_STORE\_DTR\_AS\_MIN\_LEVEL**](syn__dali_8h.md#define-syn_dali_cmd_store_dtr_as_min_level)  `0x2BU`<br> |
| define  | [**SYN\_DALI\_CMD\_STORE\_DTR\_AS\_POWER\_ON\_LEVEL**](syn__dali_8h.md#define-syn_dali_cmd_store_dtr_as_power_on_level)  `0x2DU`<br> |
| define  | [**SYN\_DALI\_CMD\_STORE\_DTR\_AS\_SCENE\_BASE**](syn__dali_8h.md#define-syn_dali_cmd_store_dtr_as_scene_base)  `0x40U /\* 0x40..0x4F \*/`<br> |
| define  | [**SYN\_DALI\_CMD\_STORE\_DTR\_AS\_SHORT\_ADDR**](syn__dali_8h.md#define-syn_dali_cmd_store_dtr_as_short_addr)  `0x80U`<br> |
| define  | [**SYN\_DALI\_CMD\_STORE\_DTR\_AS\_SYS\_FAIL\_LEVEL**](syn__dali_8h.md#define-syn_dali_cmd_store_dtr_as_sys_fail_level)  `0x2CU`<br> |
| define  | [**SYN\_DALI\_CMD\_UP**](syn__dali_8h.md#define-syn_dali_cmd_up)  `0x01U`<br> |
| define  | [**SYN\_DALI\_MASK\_LEVEL**](syn__dali_8h.md#define-syn_dali_mask_level)  `255U`<br> |
| define  | [**SYN\_DALI\_SHORT\_ADDR\_UNASSIGNED**](syn__dali_8h.md#define-syn_dali_short_addr_unassigned)  `0xFFU`<br> |
| define  | [**SYN\_DALI\_SPEC\_COMPARE**](syn__dali_8h.md#define-syn_dali_spec_compare)  `0x03U`<br> |
| define  | [**SYN\_DALI\_SPEC\_DTR0**](syn__dali_8h.md#define-syn_dali_spec_dtr0)  `0xA3U`<br> |
| define  | [**SYN\_DALI\_SPEC\_DTR1**](syn__dali_8h.md#define-syn_dali_spec_dtr1)  `0xC3U`<br> |
| define  | [**SYN\_DALI\_SPEC\_DTR2**](syn__dali_8h.md#define-syn_dali_spec_dtr2)  `0xC5U`<br> |
| define  | [**SYN\_DALI\_SPEC\_INITIALISE**](syn__dali_8h.md#define-syn_dali_spec_initialise)  `0x01U`<br> |
| define  | [**SYN\_DALI\_SPEC\_PROGRAM\_SHORT\_ADDR**](syn__dali_8h.md#define-syn_dali_spec_program_short_addr)  `0x08U`<br> |
| define  | [**SYN\_DALI\_SPEC\_QUERY\_SHORT\_ADDR**](syn__dali_8h.md#define-syn_dali_spec_query_short_addr)  `0x0AU`<br> |
| define  | [**SYN\_DALI\_SPEC\_RANDOMISE**](syn__dali_8h.md#define-syn_dali_spec_randomise)  `0x02U`<br> |
| define  | [**SYN\_DALI\_SPEC\_SEARCHADDRH**](syn__dali_8h.md#define-syn_dali_spec_searchaddrh)  `0x05U`<br> |
| define  | [**SYN\_DALI\_SPEC\_SEARCHADDRL**](syn__dali_8h.md#define-syn_dali_spec_searchaddrl)  `0x07U`<br> |
| define  | [**SYN\_DALI\_SPEC\_SEARCHADDRM**](syn__dali_8h.md#define-syn_dali_spec_searchaddrm)  `0x06U`<br> |
| define  | [**SYN\_DALI\_SPEC\_TERMINATE**](syn__dali_8h.md#define-syn_dali_spec_terminate)  `0x00U`<br> |
| define  | [**SYN\_DALI\_SPEC\_VERIFY\_SHORT\_ADDR**](syn__dali_8h.md#define-syn_dali_spec_verify_short_addr)  `0x09U`<br> |
| define  | [**SYN\_DALI\_SPEC\_WITHDRAW**](syn__dali_8h.md#define-syn_dali_spec_withdraw)  `0x04U`<br> |

## Detailed Description


Implements non-blocking, zero-malloc DALI (IEC 62386-101/102) Manchester bi-phase frame encoding/decoding, address matching (Short, Group, Broadcast), Direct Arc Power control ($0..254$), standard commands, configuration commands, and query responses.




**
**


```C++
static SYN_DALI_SlaveState dali_slave;
SYN_DALI_SlaveConfig cfg = {
    .short_address = 5,
    .min_level = 10,
    .max_level = 254,
    .power_on_level = 254,
    .system_failure_level = 254
};
syn_dali_slave_init(&dali_slave, &cfg);

SYN_DALI_ForwardFrame req;
if (syn_dali_decode_forward(raw_16, &req)) {
    uint8_t resp = 0;
    bool has_resp = false;
    syn_dali_slave_process(&dali_slave, &req, &resp, &has_resp);
}
```
 





    
## Public Types Documentation




### enum SYN\_DALI\_AddressType 

_DALI Address Types._ 
```C++
enum SYN_DALI_AddressType {
    SYN_DALI_ADDR_SHORT = 0,
    SYN_DALI_ADDR_GROUP,
    SYN_DALI_ADDR_BROADCAST,
    SYN_DALI_ADDR_SPECIAL
};
```




<hr>
## Public Functions Documentation




### function syn\_dali\_decode\_backward 

_Decode an 8-bit Slave Backward Frame._ 
```C++
bool syn_dali_decode_backward (
    uint8_t raw_8,
    SYN_DALI_BackwardFrame * frame
) 
```





**Parameters:**


* `raw_8` Raw 8-bit frame integer. 
* `frame` [out] Decoded frame structure. 



**Returns:**

true on valid frame decoding. 





        

<hr>



### function syn\_dali\_decode\_forward 

_Decode a 16-bit Master Forward Frame._ 
```C++
bool syn_dali_decode_forward (
    uint16_t raw_16,
    SYN_DALI_ForwardFrame * frame
) 
```





**Parameters:**


* `raw_16` Raw 16-bit frame integer. 
* `frame` [out] Decoded frame structure. 



**Returns:**

true on valid frame decoding. 





        

<hr>



### function syn\_dali\_encode\_backward 

_Encode an 8-bit Slave Backward Frame._ 
```C++
uint8_t syn_dali_encode_backward (
    uint8_t data
) 
```





**Parameters:**


* `data` 8-bit response payload. 



**Returns:**

Raw 8-bit frame integer. 





        

<hr>



### function syn\_dali\_encode\_forward 

_Encode a 16-bit Master Forward Frame._ 
```C++
uint16_t syn_dali_encode_forward (
    uint8_t addr,
    uint8_t data_cmd
) 
```





**Parameters:**


* `addr` Address byte (Short, Group, Broadcast). 
* `data_cmd` Data / Command byte. 



**Returns:**

Raw 16-bit frame integer. 





        

<hr>



### function syn\_dali\_manchester\_decode\_byte 

_Decode Manchester bi-phase bit pairs into a byte value._ 
```C++
bool syn_dali_manchester_decode_byte (
    const uint8_t * bit_in,
    uint8_t * val_out
) 
```





**Parameters:**


* `bit_in` Input uint8\_t array containing bi-phase pairs. 
* `val_out` [out] Decoded byte. 



**Returns:**

true on valid Manchester alignment. 





        

<hr>



### function syn\_dali\_manchester\_encode\_byte 

_Encode a byte sequence into Manchester bi-phase bits._ 
```C++
size_t syn_dali_manchester_encode_byte (
    uint8_t val,
    uint8_t * bit_out
) 
```





**Parameters:**


* `val` Input byte value. 
* `bit_out` Output uint16\_t array (minimum 16 elements for bi-phase pairs). 



**Returns:**

Number of bi-phase transitions written. 





        

<hr>



### function syn\_dali\_slave\_init 

_Initialize a DALI Slave Control Gear instance._ 
```C++
SYN_Status syn_dali_slave_init (
    SYN_DALI_SlaveState * slave,
    const SYN_DALI_SlaveConfig * cfg
) 
```





**Parameters:**


* `slave` Slave state handle. 
* `cfg` Slave configuration pointer. 



**Returns:**

SYN\_OK on success, error code otherwise. 





        

<hr>



### function syn\_dali\_slave\_process 

_Process an incoming DALI Forward Frame on a Slave Control Gear node._ 
```C++
SYN_Status syn_dali_slave_process (
    SYN_DALI_SlaveState * slave,
    const SYN_DALI_ForwardFrame * req,
    uint8_t * resp_data,
    bool * has_resp
) 
```





**Parameters:**


* `slave` Slave state handle. 
* `req` Incoming forward frame request. 
* `resp_data` [out] 8-bit response payload (if has\_resp is set to true). 
* `has_resp` [out] Set to true if a backward response frame must be transmitted. 



**Returns:**

SYN\_OK on success, error code on invalid parameters. 





        

<hr>
## Macro Definition Documentation





### define SYN\_DALI\_CMD\_ADD\_TO\_GROUP\_BASE 

```C++
#define SYN_DALI_CMD_ADD_TO_GROUP_BASE `0x60U /* 0x60..0x6F */`
```




<hr>



### define SYN\_DALI\_CMD\_DOWN 

```C++
#define SYN_DALI_CMD_DOWN `0x02U`
```




<hr>



### define SYN\_DALI\_CMD\_ENABLE\_WRITE\_MEMORY 

```C++
#define SYN_DALI_CMD_ENABLE_WRITE_MEMORY `0x81U`
```




<hr>



### define SYN\_DALI\_CMD\_GO\_TO\_SCENE\_BASE 

```C++
#define SYN_DALI_CMD_GO_TO_SCENE_BASE `0x10U /* 0x10..0x1F */`
```




<hr>



### define SYN\_DALI\_CMD\_OFF 

```C++
#define SYN_DALI_CMD_OFF `0x00U`
```




<hr>



### define SYN\_DALI\_CMD\_ON\_AND\_STEP\_UP 

```C++
#define SYN_DALI_CMD_ON_AND_STEP_UP `0x08U`
```




<hr>



### define SYN\_DALI\_CMD\_QUERY\_ACTUAL\_LEVEL 

```C++
#define SYN_DALI_CMD_QUERY_ACTUAL_LEVEL `0xA0U`
```




<hr>



### define SYN\_DALI\_CMD\_QUERY\_CONTENT\_DTR 

```C++
#define SYN_DALI_CMD_QUERY_CONTENT_DTR `0x98U`
```




<hr>



### define SYN\_DALI\_CMD\_QUERY\_CONTENT\_DTR1 

```C++
#define SYN_DALI_CMD_QUERY_CONTENT_DTR1 `0x9CU`
```




<hr>



### define SYN\_DALI\_CMD\_QUERY\_CONTENT\_DTR2 

```C++
#define SYN_DALI_CMD_QUERY_CONTENT_DTR2 `0x9DU`
```




<hr>



### define SYN\_DALI\_CMD\_QUERY\_CONTROL\_GEAR 

```C++
#define SYN_DALI_CMD_QUERY_CONTROL_GEAR `0x91U`
```




<hr>



### define SYN\_DALI\_CMD\_QUERY\_DEVICE\_TYPE 

```C++
#define SYN_DALI_CMD_QUERY_DEVICE_TYPE `0x99U`
```




<hr>



### define SYN\_DALI\_CMD\_QUERY\_GROUPS\_0\_7 

```C++
#define SYN_DALI_CMD_QUERY_GROUPS_0_7 `0xC0U`
```




<hr>



### define SYN\_DALI\_CMD\_QUERY\_GROUPS\_8\_15 

```C++
#define SYN_DALI_CMD_QUERY_GROUPS_8_15 `0xC1U`
```




<hr>



### define SYN\_DALI\_CMD\_QUERY\_LAMP\_FAILURE 

```C++
#define SYN_DALI_CMD_QUERY_LAMP_FAILURE `0x92U`
```




<hr>



### define SYN\_DALI\_CMD\_QUERY\_LAMP\_POWER\_ON 

```C++
#define SYN_DALI_CMD_QUERY_LAMP_POWER_ON `0x93U`
```




<hr>



### define SYN\_DALI\_CMD\_QUERY\_MAX\_LEVEL 

```C++
#define SYN_DALI_CMD_QUERY_MAX_LEVEL `0xA1U`
```




<hr>



### define SYN\_DALI\_CMD\_QUERY\_MIN\_LEVEL 

```C++
#define SYN_DALI_CMD_QUERY_MIN_LEVEL `0xA2U`
```




<hr>



### define SYN\_DALI\_CMD\_QUERY\_PHYSICAL\_MIN\_LEVEL 

```C++
#define SYN_DALI_CMD_QUERY_PHYSICAL_MIN_LEVEL `0x9AU`
```




<hr>



### define SYN\_DALI\_CMD\_QUERY\_POWER\_ON\_LEVEL 

```C++
#define SYN_DALI_CMD_QUERY_POWER_ON_LEVEL `0xA3U`
```




<hr>



### define SYN\_DALI\_CMD\_QUERY\_RANDOM\_ADDR\_H 

```C++
#define SYN_DALI_CMD_QUERY_RANDOM_ADDR_H `0xC2U`
```




<hr>



### define SYN\_DALI\_CMD\_QUERY\_RANDOM\_ADDR\_L 

```C++
#define SYN_DALI_CMD_QUERY_RANDOM_ADDR_L `0xC4U`
```




<hr>



### define SYN\_DALI\_CMD\_QUERY\_RANDOM\_ADDR\_M 

```C++
#define SYN_DALI_CMD_QUERY_RANDOM_ADDR_M `0xC3U`
```




<hr>



### define SYN\_DALI\_CMD\_QUERY\_STATUS 

```C++
#define SYN_DALI_CMD_QUERY_STATUS `0x90U`
```




<hr>



### define SYN\_DALI\_CMD\_QUERY\_SYS\_FAIL\_LEVEL 

```C++
#define SYN_DALI_CMD_QUERY_SYS_FAIL_LEVEL `0xA4U`
```




<hr>



### define SYN\_DALI\_CMD\_RECALL\_MAX 

```C++
#define SYN_DALI_CMD_RECALL_MAX `0x05U`
```




<hr>



### define SYN\_DALI\_CMD\_RECALL\_MIN 

```C++
#define SYN_DALI_CMD_RECALL_MIN `0x06U`
```




<hr>



### define SYN\_DALI\_CMD\_REMOVE\_FROM\_GROUP\_BASE 

```C++
#define SYN_DALI_CMD_REMOVE_FROM_GROUP_BASE `0x70U /* 0x70..0x7F */`
```




<hr>



### define SYN\_DALI\_CMD\_REMOVE\_FROM\_SCENE\_BASE 

```C++
#define SYN_DALI_CMD_REMOVE_FROM_SCENE_BASE `0x50U /* 0x50..0x5F */`
```




<hr>



### define SYN\_DALI\_CMD\_RESET 

```C++
#define SYN_DALI_CMD_RESET `0x20U`
```




<hr>



### define SYN\_DALI\_CMD\_STEP\_DOWN 

```C++
#define SYN_DALI_CMD_STEP_DOWN `0x04U`
```




<hr>



### define SYN\_DALI\_CMD\_STEP\_DOWN\_AND\_OFF 

```C++
#define SYN_DALI_CMD_STEP_DOWN_AND_OFF `0x07U`
```




<hr>



### define SYN\_DALI\_CMD\_STEP\_UP 

```C++
#define SYN_DALI_CMD_STEP_UP `0x03U`
```




<hr>



### define SYN\_DALI\_CMD\_STORE\_ACTUAL\_LEVEL\_IN\_DTR 

```C++
#define SYN_DALI_CMD_STORE_ACTUAL_LEVEL_IN_DTR `0x21U`
```




<hr>



### define SYN\_DALI\_CMD\_STORE\_DTR\_AS\_FADE\_RATE 

```C++
#define SYN_DALI_CMD_STORE_DTR_AS_FADE_RATE `0x2FU`
```




<hr>



### define SYN\_DALI\_CMD\_STORE\_DTR\_AS\_FADE\_TIME 

```C++
#define SYN_DALI_CMD_STORE_DTR_AS_FADE_TIME `0x2EU`
```




<hr>



### define SYN\_DALI\_CMD\_STORE\_DTR\_AS\_MAX\_LEVEL 

```C++
#define SYN_DALI_CMD_STORE_DTR_AS_MAX_LEVEL `0x2AU`
```




<hr>



### define SYN\_DALI\_CMD\_STORE\_DTR\_AS\_MIN\_LEVEL 

```C++
#define SYN_DALI_CMD_STORE_DTR_AS_MIN_LEVEL `0x2BU`
```




<hr>



### define SYN\_DALI\_CMD\_STORE\_DTR\_AS\_POWER\_ON\_LEVEL 

```C++
#define SYN_DALI_CMD_STORE_DTR_AS_POWER_ON_LEVEL `0x2DU`
```




<hr>



### define SYN\_DALI\_CMD\_STORE\_DTR\_AS\_SCENE\_BASE 

```C++
#define SYN_DALI_CMD_STORE_DTR_AS_SCENE_BASE `0x40U /* 0x40..0x4F */`
```




<hr>



### define SYN\_DALI\_CMD\_STORE\_DTR\_AS\_SHORT\_ADDR 

```C++
#define SYN_DALI_CMD_STORE_DTR_AS_SHORT_ADDR `0x80U`
```




<hr>



### define SYN\_DALI\_CMD\_STORE\_DTR\_AS\_SYS\_FAIL\_LEVEL 

```C++
#define SYN_DALI_CMD_STORE_DTR_AS_SYS_FAIL_LEVEL `0x2CU`
```




<hr>



### define SYN\_DALI\_CMD\_UP 

```C++
#define SYN_DALI_CMD_UP `0x01U`
```




<hr>



### define SYN\_DALI\_MASK\_LEVEL 

```C++
#define SYN_DALI_MASK_LEVEL `255U`
```



MASK level (no change) 
 


        

<hr>



### define SYN\_DALI\_SHORT\_ADDR\_UNASSIGNED 

```C++
#define SYN_DALI_SHORT_ADDR_UNASSIGNED `0xFFU`
```



Unassigned Short Address 


        

<hr>



### define SYN\_DALI\_SPEC\_COMPARE 

```C++
#define SYN_DALI_SPEC_COMPARE `0x03U`
```




<hr>



### define SYN\_DALI\_SPEC\_DTR0 

```C++
#define SYN_DALI_SPEC_DTR0 `0xA3U`
```




<hr>



### define SYN\_DALI\_SPEC\_DTR1 

```C++
#define SYN_DALI_SPEC_DTR1 `0xC3U`
```




<hr>



### define SYN\_DALI\_SPEC\_DTR2 

```C++
#define SYN_DALI_SPEC_DTR2 `0xC5U`
```




<hr>



### define SYN\_DALI\_SPEC\_INITIALISE 

```C++
#define SYN_DALI_SPEC_INITIALISE `0x01U`
```




<hr>



### define SYN\_DALI\_SPEC\_PROGRAM\_SHORT\_ADDR 

```C++
#define SYN_DALI_SPEC_PROGRAM_SHORT_ADDR `0x08U`
```




<hr>



### define SYN\_DALI\_SPEC\_QUERY\_SHORT\_ADDR 

```C++
#define SYN_DALI_SPEC_QUERY_SHORT_ADDR `0x0AU`
```




<hr>



### define SYN\_DALI\_SPEC\_RANDOMISE 

```C++
#define SYN_DALI_SPEC_RANDOMISE `0x02U`
```




<hr>



### define SYN\_DALI\_SPEC\_SEARCHADDRH 

```C++
#define SYN_DALI_SPEC_SEARCHADDRH `0x05U`
```




<hr>



### define SYN\_DALI\_SPEC\_SEARCHADDRL 

```C++
#define SYN_DALI_SPEC_SEARCHADDRL `0x07U`
```




<hr>



### define SYN\_DALI\_SPEC\_SEARCHADDRM 

```C++
#define SYN_DALI_SPEC_SEARCHADDRM `0x06U`
```




<hr>



### define SYN\_DALI\_SPEC\_TERMINATE 

```C++
#define SYN_DALI_SPEC_TERMINATE `0x00U`
```




<hr>



### define SYN\_DALI\_SPEC\_VERIFY\_SHORT\_ADDR 

```C++
#define SYN_DALI_SPEC_VERIFY_SHORT_ADDR `0x09U`
```




<hr>



### define SYN\_DALI\_SPEC\_WITHDRAW 

```C++
#define SYN_DALI_SPEC_WITHDRAW `0x04U`
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_dali.h`

