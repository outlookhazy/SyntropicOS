

# File syn\_ble\_gap.c



[**FileList**](files.md) **>** [**ble**](dir_4014da4ce18dc4bd9570be14bcfa447a.md) **>** [**syn\_ble\_gap.c**](syn__ble__gap_8c.md)

[Go to the source code of this file](syn__ble__gap_8c_source.md)

_Zero-Heap Generic Access Profile (GAP) Layer Implementation._ 

* `#include "syn_ble_gap.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_ble\_gap\_init**](#function-syn_ble_gap_init) (SYN\_BLE\_GAP \* gap, SYN\_BLE\_HCI \* hci, [**SYN\_BLE\_GAP\_EventCb**](syn__ble__gap_8h.md#typedef-syn_ble_gap_eventcb) evt\_cb, void \* user\_data) <br>_Initialize GAP engine context._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_ble\_gap\_process\_hci\_evt**](#function-syn_ble_gap_process_hci_evt) (SYN\_BLE\_GAP \* gap, uint8\_t evt\_code, const uint8\_t \* payload, uint8\_t len) <br>_Process HCI Event for GAP state updates._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_ble\_gap\_set\_adv\_data**](#function-syn_ble_gap_set_adv_data) (SYN\_BLE\_GAP \* gap, const uint8\_t \* data, uint8\_t len, uint8\_t \* tx\_buf, uint16\_t \* tx\_len) <br>_Set Advertising Data._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_ble\_gap\_set\_adv\_enable**](#function-syn_ble_gap_set_adv_enable) (SYN\_BLE\_GAP \* gap, [**bool**](syn__defs_8h.md#enum-bool) enable, uint8\_t \* tx\_buf, uint16\_t \* tx\_len) <br>_Enable or Disable Advertising._  |




























## Public Functions Documentation




### function syn\_ble\_gap\_init 

_Initialize GAP engine context._ 
```C++
SYN_Status syn_ble_gap_init (
    SYN_BLE_GAP * gap,
    SYN_BLE_HCI * hci,
    SYN_BLE_GAP_EventCb evt_cb,
    void * user_data
) 
```





**Parameters:**


* `gap` Context handle pointer. 
* `hci` Initialized HCI driver handle. 
* `evt_cb` GAP event callback. 
* `user_data` User context pointer. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_ble\_gap\_process\_hci\_evt 

_Process HCI Event for GAP state updates._ 
```C++
SYN_Status syn_ble_gap_process_hci_evt (
    SYN_BLE_GAP * gap,
    uint8_t evt_code,
    const uint8_t * payload,
    uint8_t len
) 
```





**Parameters:**


* `gap` Context handle pointer. 
* `evt_code` HCI Event code. 
* `payload` Event payload. 
* `len` Event length. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_ble\_gap\_set\_adv\_data 

_Set Advertising Data._ 
```C++
SYN_Status syn_ble_gap_set_adv_data (
    SYN_BLE_GAP * gap,
    const uint8_t * data,
    uint8_t len,
    uint8_t * tx_buf,
    uint16_t * tx_len
) 
```





**Parameters:**


* `gap` Context handle pointer. 
* `data` AD data payload. 
* `len` Data length (max 31 bytes). 
* `tx_buf` HCI TX staging buffer (min 36 bytes). 
* `tx_len` Output length pointer. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_ble\_gap\_set\_adv\_enable 

_Enable or Disable Advertising._ 
```C++
SYN_Status syn_ble_gap_set_adv_enable (
    SYN_BLE_GAP * gap,
    bool enable,
    uint8_t * tx_buf,
    uint16_t * tx_len
) 
```





**Parameters:**


* `gap` Context handle pointer. 
* `enable` True to start, false to stop. 
* `tx_buf` HCI TX staging buffer (min 6 bytes). 
* `tx_len` Output length pointer. 



**Returns:**

SYN\_OK on success. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/ble/syn_ble_gap.c`

