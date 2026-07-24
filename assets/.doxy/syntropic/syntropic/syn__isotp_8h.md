

# File syn\_isotp.h



[**FileList**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_isotp.h**](syn__isotp_8h.md)

[Go to the source code of this file](syn__isotp_8h_source.md)

_ISO 15765-2 (DoCAN / ISO-TP) Multi-Frame CAN Transport Protocol._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include "../drivers/syn_can.h"`
* `#include <stdint.h>`
* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <sys/types.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_ISOTP\_Link**](structSYN__ISOTP__Link.md) <br>_ISO 15765-2 Link Handle._  |


## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**SYN\_ISOTP\_RxState**](#enum-syn_isotp_rxstate)  <br>_ISO-TP Rx States._  |
| enum  | [**SYN\_ISOTP\_TxState**](#enum-syn_isotp_txstate)  <br>_ISO-TP Tx States._  |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_isotp\_get\_tx\_frame**](#function-syn_isotp_get_tx_frame) ([**SYN\_ISOTP\_Link**](structSYN__ISOTP__Link.md) \* link, [**SYN\_CAN\_Frame**](structSYN__CAN__Frame.md) \* frame) <br>_Retrieve next pending outgoing CAN frame from ISO-TP engine._  |
|  void | [**syn\_isotp\_init**](#function-syn_isotp_init) ([**SYN\_ISOTP\_Link**](structSYN__ISOTP__Link.md) \* link, uint32\_t rx\_id, uint32\_t tx\_id, uint8\_t \* rx\_buf, size\_t rx\_buf\_size, uint8\_t \* tx\_buf, size\_t tx\_buf\_size) <br>_Initialize ISO-TP Link in Classic CAN mode (8-byte frames)._  |
|  void | [**syn\_isotp\_process\_rx\_frame**](#function-syn_isotp_process_rx_frame) ([**SYN\_ISOTP\_Link**](structSYN__ISOTP__Link.md) \* link, const [**SYN\_CAN\_Frame**](structSYN__CAN__Frame.md) \* frame) <br>_Process an incoming CAN frame into the ISO-TP engine._  |
|  ssize\_t | [**syn\_isotp\_receive**](#function-syn_isotp_receive) ([**SYN\_ISOTP\_Link**](structSYN__ISOTP__Link.md) \* link, uint8\_t \* out\_buf, size\_t max\_len) <br>_Read completed assembled message from ISO-TP receiver._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_isotp\_send**](#function-syn_isotp_send) ([**SYN\_ISOTP\_Link**](structSYN__ISOTP__Link.md) \* link, const uint8\_t \* payload, size\_t payload\_len) <br>_Initiate transmission of a multi-byte payload._  |
|  void | [**syn\_isotp\_set\_timeouts**](#function-syn_isotp_set_timeouts) ([**SYN\_ISOTP\_Link**](structSYN__ISOTP__Link.md) \* link, uint32\_t n\_bs\_ms, uint32\_t n\_cr\_ms) <br>_Configure custom ISO 15765-2 network layer timeouts._  |
|  void | [**syn\_isotp\_step**](#function-syn_isotp_step) ([**SYN\_ISOTP\_Link**](structSYN__ISOTP__Link.md) \* link, uint32\_t dt\_ms) <br>_Step ISO-TP timers (milliseconds)._  |
|  void | [**syn\_isotp\_step\_us**](#function-syn_isotp_step_us) ([**SYN\_ISOTP\_Link**](structSYN__ISOTP__Link.md) \* link, uint32\_t dt\_us) <br>_Step ISO-TP timers (microseconds for fine STmin pacing)._  |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_ISOTP\_DEFAULT\_N\_BS\_MS**](syn__isotp_8h.md#define-syn_isotp_default_n_bs_ms)  `1000U`<br>_ISO 15765-2 Network Layer Default Timeouts (ISO 15765-2:2016)_  |
| define  | [**SYN\_ISOTP\_DEFAULT\_N\_CR\_MS**](syn__isotp_8h.md#define-syn_isotp_default_n_cr_ms)  `1000U`<br> |
| define  | [**SYN\_ISOTP\_FC\_CTS**](syn__isotp_8h.md#define-syn_isotp_fc_cts)  `0x00U`<br>_Flow Control Status (FC)_  |
| define  | [**SYN\_ISOTP\_FC\_OVERFLOW**](syn__isotp_8h.md#define-syn_isotp_fc_overflow)  `0x02U`<br> |
| define  | [**SYN\_ISOTP\_FC\_WAIT**](syn__isotp_8h.md#define-syn_isotp_fc_wait)  `0x01U`<br> |
| define  | [**SYN\_ISOTP\_MAX\_PAYLOAD**](syn__isotp_8h.md#define-syn_isotp_max_payload)  `4095U`<br>_ISO-TP Protocol Constants._  |
| define  | [**SYN\_ISOTP\_PCI\_CF**](syn__isotp_8h.md#define-syn_isotp_pci_cf)  `0x20U`<br> |
| define  | [**SYN\_ISOTP\_PCI\_FC**](syn__isotp_8h.md#define-syn_isotp_pci_fc)  `0x30U`<br> |
| define  | [**SYN\_ISOTP\_PCI\_FF**](syn__isotp_8h.md#define-syn_isotp_pci_ff)  `0x10U`<br> |
| define  | [**SYN\_ISOTP\_PCI\_SF**](syn__isotp_8h.md#define-syn_isotp_pci_sf)  `0x00U`<br> |

## Detailed Description


Implements non-blocking, zero-malloc ISO 15765-2 segmentation & reassembly for multi-byte CAN payload transmission with Single Frame (SF), First Frame (FF), Consecutive Frame (CF), and Flow Control (FC) support.


Support for CAN FD (64-byte payload frames & ISO 15765-2:2016 extended length) is opt-in via #define SYN\_USE\_CAN\_FD 1. 


    
## Public Types Documentation




### enum SYN\_ISOTP\_RxState 

_ISO-TP Rx States._ 
```C++
enum SYN_ISOTP_RxState {
    SYN_ISOTP_RX_IDLE = 0,
    SYN_ISOTP_RX_WAIT_CF,
    SYN_ISOTP_RX_COMPLETE
};
```




<hr>



### enum SYN\_ISOTP\_TxState 

_ISO-TP Tx States._ 
```C++
enum SYN_ISOTP_TxState {
    SYN_ISOTP_TX_IDLE = 0,
    SYN_ISOTP_TX_SEND_SF,
    SYN_ISOTP_TX_SEND_FF,
    SYN_ISOTP_TX_WAIT_FC,
    SYN_ISOTP_TX_SEND_CF
};
```




<hr>
## Public Functions Documentation




### function syn\_isotp\_get\_tx\_frame 

_Retrieve next pending outgoing CAN frame from ISO-TP engine._ 
```C++
bool syn_isotp_get_tx_frame (
    SYN_ISOTP_Link * link,
    SYN_CAN_Frame * frame
) 
```





**Parameters:**


* `link` Link handle. 
* `frame` Output CAN frame buffer. 



**Returns:**

true if a frame was generated, false otherwise. 





        

<hr>



### function syn\_isotp\_init 

_Initialize ISO-TP Link in Classic CAN mode (8-byte frames)._ 
```C++
void syn_isotp_init (
    SYN_ISOTP_Link * link,
    uint32_t rx_id,
    uint32_t tx_id,
    uint8_t * rx_buf,
    size_t rx_buf_size,
    uint8_t * tx_buf,
    size_t tx_buf_size
) 
```





**Parameters:**


* `link` Link handle. 
* `rx_id` Incoming CAN identifier. 
* `tx_id` Outgoing CAN identifier. 
* `rx_buf` Receive assembly buffer. 
* `rx_buf_size` Receive buffer capacity. 
* `tx_buf` Transmit buffer. 
* `tx_buf_size` Transmit buffer capacity. 




        

<hr>



### function syn\_isotp\_process\_rx\_frame 

_Process an incoming CAN frame into the ISO-TP engine._ 
```C++
void syn_isotp_process_rx_frame (
    SYN_ISOTP_Link * link,
    const SYN_CAN_Frame * frame
) 
```





**Parameters:**


* `link` Link handle. 
* `frame` Incoming CAN frame. 




        

<hr>



### function syn\_isotp\_receive 

_Read completed assembled message from ISO-TP receiver._ 
```C++
ssize_t syn_isotp_receive (
    SYN_ISOTP_Link * link,
    uint8_t * out_buf,
    size_t max_len
) 
```





**Parameters:**


* `link` Link handle. 
* `out_buf` Output buffer. 
* `max_len` Capacity of out\_buf. 



**Returns:**

Number of bytes copied, or -1 if no message ready. 





        

<hr>



### function syn\_isotp\_send 

_Initiate transmission of a multi-byte payload._ 
```C++
SYN_Status syn_isotp_send (
    SYN_ISOTP_Link * link,
    const uint8_t * payload,
    size_t payload_len
) 
```





**Parameters:**


* `link` Link handle. 
* `payload` Payload buffer. 
* `payload_len` Length of payload (1 to max buffer capacity). 



**Returns:**

SYN\_OK on success, error code on busy/invalid params. 





        

<hr>



### function syn\_isotp\_set\_timeouts 

_Configure custom ISO 15765-2 network layer timeouts._ 
```C++
void syn_isotp_set_timeouts (
    SYN_ISOTP_Link * link,
    uint32_t n_bs_ms,
    uint32_t n_cr_ms
) 
```





**Parameters:**


* `link` Link handle. 
* `n_bs_ms` N\_Bs max timeout in milliseconds (0 = use default 1000ms). 
* `n_cr_ms` N\_Cr max timeout in milliseconds (0 = use default 1000ms). 




        

<hr>



### function syn\_isotp\_step 

_Step ISO-TP timers (milliseconds)._ 
```C++
void syn_isotp_step (
    SYN_ISOTP_Link * link,
    uint32_t dt_ms
) 
```





**Parameters:**


* `link` Link handle. 
* `dt_ms` Elapsed time in milliseconds. 




        

<hr>



### function syn\_isotp\_step\_us 

_Step ISO-TP timers (microseconds for fine STmin pacing)._ 
```C++
void syn_isotp_step_us (
    SYN_ISOTP_Link * link,
    uint32_t dt_us
) 
```





**Parameters:**


* `link` Link handle. 
* `dt_us` Elapsed time in microseconds. 




        

<hr>
## Macro Definition Documentation





### define SYN\_ISOTP\_DEFAULT\_N\_BS\_MS 

_ISO 15765-2 Network Layer Default Timeouts (ISO 15765-2:2016)_ 
```C++
#define SYN_ISOTP_DEFAULT_N_BS_MS `1000U`
```



N\_Bs max time for FC reception (1000 ms) 


        

<hr>



### define SYN\_ISOTP\_DEFAULT\_N\_CR\_MS 

```C++
#define SYN_ISOTP_DEFAULT_N_CR_MS `1000U`
```



N\_Cr max time for CF reception (1000 ms) 


        

<hr>



### define SYN\_ISOTP\_FC\_CTS 

_Flow Control Status (FC)_ 
```C++
#define SYN_ISOTP_FC_CTS `0x00U`
```



Continue To Send 
 


        

<hr>



### define SYN\_ISOTP\_FC\_OVERFLOW 

```C++
#define SYN_ISOTP_FC_OVERFLOW `0x02U`
```



Buffer Overflow 
 


        

<hr>



### define SYN\_ISOTP\_FC\_WAIT 

```C++
#define SYN_ISOTP_FC_WAIT `0x01U`
```



Wait 
 


        

<hr>



### define SYN\_ISOTP\_MAX\_PAYLOAD 

_ISO-TP Protocol Constants._ 
```C++
#define SYN_ISOTP_MAX_PAYLOAD `4095U`
```



Standard 12-bit max payload (Classic CAN) 


        

<hr>



### define SYN\_ISOTP\_PCI\_CF 

```C++
#define SYN_ISOTP_PCI_CF `0x20U`
```



Consecutive Frame 
 


        

<hr>



### define SYN\_ISOTP\_PCI\_FC 

```C++
#define SYN_ISOTP_PCI_FC `0x30U`
```



Flow Control Frame 
 


        

<hr>



### define SYN\_ISOTP\_PCI\_FF 

```C++
#define SYN_ISOTP_PCI_FF `0x10U`
```



First Frame 
 


        

<hr>



### define SYN\_ISOTP\_PCI\_SF 

```C++
#define SYN_ISOTP_PCI_SF `0x00U`
```



Single Frame 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_isotp.h`

