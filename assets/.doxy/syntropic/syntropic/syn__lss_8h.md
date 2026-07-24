

# File syn\_lss.h



[**FileList**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_lss.h**](syn__lss_8h.md)

[Go to the source code of this file](syn__lss_8h_source.md)

_CANopen CiA 305 Layer Setting Services (LSS) & Protocols._ [More...](#detailed-description)

* `#include <stdint.h>`
* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include "../common/syn_defs.h"`
* `#include "../drivers/syn_can.h"`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_LSSAddress**](structSYN__LSSAddress.md) <br>_LSS Address Structure (Unique 128-bit identity)_  |
| struct | [**SYN\_LSSSlave**](structSYN__LSSSlave.md) <br>_LSS Slave Instance State._  |






















## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_lss\_master\_build\_config\_bit\_timing**](#function-syn_lss_master_build_config_bit_timing) ([**SYN\_CAN\_Frame**](structSYN__CAN__Frame.md) \* frame, uint8\_t baud\_table\_idx) <br>_Build an LSS Master request frame to configure Bit Timing / Baud rate._  |
|  void | [**syn\_lss\_master\_build\_config\_node\_id**](#function-syn_lss_master_build_config_node_id) ([**SYN\_CAN\_Frame**](structSYN__CAN__Frame.md) \* frame, uint8\_t new\_node\_id) <br>_Build an LSS Master request frame to configure Node-ID._  |
|  void | [**syn\_lss\_master\_build\_store\_config**](#function-syn_lss_master_build_store_config) ([**SYN\_CAN\_Frame**](structSYN__CAN__Frame.md) \* frame) <br>_Build an LSS Master request frame to store configuration to NVM._  |
|  void | [**syn\_lss\_master\_build\_switch\_mode**](#function-syn_lss_master_build_switch_mode) ([**SYN\_CAN\_Frame**](structSYN__CAN__Frame.md) \* frame, uint8\_t mode) <br>_Build an LSS Master request frame to switch global mode._  |
|  void | [**syn\_lss\_slave\_init**](#function-syn_lss_slave_init) ([**SYN\_LSSSlave**](structSYN__LSSSlave.md) \* slave, const [**SYN\_LSSAddress**](structSYN__LSSAddress.md) \* addr, uint8\_t initial\_node\_id) <br>_Initialize LSS Slave state instance._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_lss\_slave\_process**](#function-syn_lss_slave_process) ([**SYN\_LSSSlave**](structSYN__LSSSlave.md) \* slave, const [**SYN\_CAN\_Frame**](structSYN__CAN__Frame.md) \* frame, [**SYN\_CAN\_Frame**](structSYN__CAN__Frame.md) \* resp) <br>_Process an incoming CAN frame for LSS Slave processing._  |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_LSS\_BAUD\_1000K**](syn__lss_8h.md#define-syn_lss_baud_1000k)  `0x00U`<br>_LSS Bit Timing Table Entries (CiA 305 Standard Baud Rates)_  |
| define  | [**SYN\_LSS\_BAUD\_100K**](syn__lss_8h.md#define-syn_lss_baud_100k)  `0x05U`<br> |
| define  | [**SYN\_LSS\_BAUD\_10K**](syn__lss_8h.md#define-syn_lss_baud_10k)  `0x08U`<br> |
| define  | [**SYN\_LSS\_BAUD\_125K**](syn__lss_8h.md#define-syn_lss_baud_125k)  `0x04U`<br> |
| define  | [**SYN\_LSS\_BAUD\_20K**](syn__lss_8h.md#define-syn_lss_baud_20k)  `0x07U`<br> |
| define  | [**SYN\_LSS\_BAUD\_250K**](syn__lss_8h.md#define-syn_lss_baud_250k)  `0x03U`<br> |
| define  | [**SYN\_LSS\_BAUD\_500K**](syn__lss_8h.md#define-syn_lss_baud_500k)  `0x02U`<br> |
| define  | [**SYN\_LSS\_BAUD\_50K**](syn__lss_8h.md#define-syn_lss_baud_50k)  `0x06U`<br> |
| define  | [**SYN\_LSS\_BAUD\_800K**](syn__lss_8h.md#define-syn_lss_baud_800k)  `0x01U`<br> |
| define  | [**SYN\_LSS\_COB\_ID\_MASTER**](syn__lss_8h.md#define-syn_lss_cob_id_master)  `0x7E5U`<br>_LSS CAN Identifier Definitions._  |
| define  | [**SYN\_LSS\_COB\_ID\_SLAVE**](syn__lss_8h.md#define-syn_lss_cob_id_slave)  `0x7E4U`<br> |
| define  | [**SYN\_LSS\_CS\_ACTIVATE\_BIT\_TIMING**](syn__lss_8h.md#define-syn_lss_cs_activate_bit_timing)  `0x15U`<br> |
| define  | [**SYN\_LSS\_CS\_CONFIGURE\_BIT\_TIMING**](syn__lss_8h.md#define-syn_lss_cs_configure_bit_timing)  `0x13U`<br> |
| define  | [**SYN\_LSS\_CS\_CONFIGURE\_NODE\_ID**](syn__lss_8h.md#define-syn_lss_cs_configure_node_id)  `0x11U`<br> |
| define  | [**SYN\_LSS\_CS\_INQUIRE\_PRODUCT**](syn__lss_8h.md#define-syn_lss_cs_inquire_product)  `0x5BU`<br> |
| define  | [**SYN\_LSS\_CS\_INQUIRE\_REV**](syn__lss_8h.md#define-syn_lss_cs_inquire_rev)  `0x5CU`<br> |
| define  | [**SYN\_LSS\_CS\_INQUIRE\_SERIAL**](syn__lss_8h.md#define-syn_lss_cs_inquire_serial)  `0x5DU`<br> |
| define  | [**SYN\_LSS\_CS\_INQUIRE\_VENDOR**](syn__lss_8h.md#define-syn_lss_cs_inquire_vendor)  `0x5AU`<br> |
| define  | [**SYN\_LSS\_CS\_STORE\_CONFIG**](syn__lss_8h.md#define-syn_lss_cs_store_config)  `0x17U`<br> |
| define  | [**SYN\_LSS\_CS\_SWITCH\_MODE\_GLOBAL**](syn__lss_8h.md#define-syn_lss_cs_switch_mode_global)  `0x04U`<br>_LSS Command Specifiers (CS)_  |
| define  | [**SYN\_LSS\_CS\_SWITCH\_MODE\_SELECT**](syn__lss_8h.md#define-syn_lss_cs_switch_mode_select)  `0x40U`<br> |
| define  | [**SYN\_LSS\_MODE\_CONFIGURATION**](syn__lss_8h.md#define-syn_lss_mode_configuration)  `0x01U`<br> |
| define  | [**SYN\_LSS\_MODE\_OPERATION**](syn__lss_8h.md#define-syn_lss_mode_operation)  `0x00U`<br>_LSS Operating Modes._  |

## Detailed Description


Implements non-blocking, zero-malloc LSS slave & master protocol engines for dynamic Node-ID allocation, bit-timing (baud rate) configuration, LSS Fastscan, and NVM configuration storage over CAN. 


    
## Public Functions Documentation




### function syn\_lss\_master\_build\_config\_bit\_timing 

_Build an LSS Master request frame to configure Bit Timing / Baud rate._ 
```C++
void syn_lss_master_build_config_bit_timing (
    SYN_CAN_Frame * frame,
    uint8_t baud_table_idx
) 
```





**Parameters:**


* `frame` Output frame buffer. 
* `baud_table_idx` Baud rate index (SYN\_LSS\_BAUD\_\*). 




        

<hr>



### function syn\_lss\_master\_build\_config\_node\_id 

_Build an LSS Master request frame to configure Node-ID._ 
```C++
void syn_lss_master_build_config_node_id (
    SYN_CAN_Frame * frame,
    uint8_t new_node_id
) 
```





**Parameters:**


* `frame` Output frame buffer. 
* `new_node_id` Target Node-ID (1..127). 




        

<hr>



### function syn\_lss\_master\_build\_store\_config 

_Build an LSS Master request frame to store configuration to NVM._ 
```C++
void syn_lss_master_build_store_config (
    SYN_CAN_Frame * frame
) 
```





**Parameters:**


* `frame` Output frame buffer. 




        

<hr>



### function syn\_lss\_master\_build\_switch\_mode 

_Build an LSS Master request frame to switch global mode._ 
```C++
void syn_lss_master_build_switch_mode (
    SYN_CAN_Frame * frame,
    uint8_t mode
) 
```





**Parameters:**


* `frame` Output frame buffer. 
* `mode` Target LSS mode (SYN\_LSS\_MODE\_OPERATION or SYN\_LSS\_MODE\_CONFIGURATION). 




        

<hr>



### function syn\_lss\_slave\_init 

_Initialize LSS Slave state instance._ 
```C++
void syn_lss_slave_init (
    SYN_LSSSlave * slave,
    const SYN_LSSAddress * addr,
    uint8_t initial_node_id
) 
```





**Parameters:**


* `slave` Pointer to slave handle. 
* `addr` Pointer to LSS unique 128-bit identity address. 
* `initial_node_id` Starting node-ID (or 0xFF for unconfigured). 




        

<hr>



### function syn\_lss\_slave\_process 

_Process an incoming CAN frame for LSS Slave processing._ 
```C++
bool syn_lss_slave_process (
    SYN_LSSSlave * slave,
    const SYN_CAN_Frame * frame,
    SYN_CAN_Frame * resp
) 
```





**Parameters:**


* `slave` Pointer to LSS Slave handle. 
* `frame` Incoming CAN frame (COB-ID 0x7E5). 
* `resp` Output CAN frame buffer (populated with 0x7E4 response if required). 



**Returns:**

True if a response frame should be transmitted over CAN. 





        

<hr>
## Macro Definition Documentation





### define SYN\_LSS\_BAUD\_1000K 

_LSS Bit Timing Table Entries (CiA 305 Standard Baud Rates)_ 
```C++
#define SYN_LSS_BAUD_1000K `0x00U`
```



1000 kbit/s 


        

<hr>



### define SYN\_LSS\_BAUD\_100K 

```C++
#define SYN_LSS_BAUD_100K `0x05U`
```



100 kbit/s 


        

<hr>



### define SYN\_LSS\_BAUD\_10K 

```C++
#define SYN_LSS_BAUD_10K `0x08U`
```



10 kbit/s 


        

<hr>



### define SYN\_LSS\_BAUD\_125K 

```C++
#define SYN_LSS_BAUD_125K `0x04U`
```



125 kbit/s 


        

<hr>



### define SYN\_LSS\_BAUD\_20K 

```C++
#define SYN_LSS_BAUD_20K `0x07U`
```



20 kbit/s 


        

<hr>



### define SYN\_LSS\_BAUD\_250K 

```C++
#define SYN_LSS_BAUD_250K `0x03U`
```



250 kbit/s 


        

<hr>



### define SYN\_LSS\_BAUD\_500K 

```C++
#define SYN_LSS_BAUD_500K `0x02U`
```



500 kbit/s 


        

<hr>



### define SYN\_LSS\_BAUD\_50K 

```C++
#define SYN_LSS_BAUD_50K `0x06U`
```



50 kbit/s 


        

<hr>



### define SYN\_LSS\_BAUD\_800K 

```C++
#define SYN_LSS_BAUD_800K `0x01U`
```



800 kbit/s 


        

<hr>



### define SYN\_LSS\_COB\_ID\_MASTER 

_LSS CAN Identifier Definitions._ 
```C++
#define SYN_LSS_COB_ID_MASTER `0x7E5U`
```



Master to Slave Request COB-ID 


        

<hr>



### define SYN\_LSS\_COB\_ID\_SLAVE 

```C++
#define SYN_LSS_COB_ID_SLAVE `0x7E4U`
```



Slave to Master Response COB-ID 


        

<hr>



### define SYN\_LSS\_CS\_ACTIVATE\_BIT\_TIMING 

```C++
#define SYN_LSS_CS_ACTIVATE_BIT_TIMING `0x15U`
```



Activate Bit Timing 


        

<hr>



### define SYN\_LSS\_CS\_CONFIGURE\_BIT\_TIMING 

```C++
#define SYN_LSS_CS_CONFIGURE_BIT_TIMING `0x13U`
```



Configure Bit Timing 


        

<hr>



### define SYN\_LSS\_CS\_CONFIGURE\_NODE\_ID 

```C++
#define SYN_LSS_CS_CONFIGURE_NODE_ID `0x11U`
```



Configure Node-ID 


        

<hr>



### define SYN\_LSS\_CS\_INQUIRE\_PRODUCT 

```C++
#define SYN_LSS_CS_INQUIRE_PRODUCT `0x5BU`
```



Inquire Product-Code 


        

<hr>



### define SYN\_LSS\_CS\_INQUIRE\_REV 

```C++
#define SYN_LSS_CS_INQUIRE_REV `0x5CU`
```



Inquire Revision-Number 


        

<hr>



### define SYN\_LSS\_CS\_INQUIRE\_SERIAL 

```C++
#define SYN_LSS_CS_INQUIRE_SERIAL `0x5DU`
```



Inquire Serial-Number 


        

<hr>



### define SYN\_LSS\_CS\_INQUIRE\_VENDOR 

```C++
#define SYN_LSS_CS_INQUIRE_VENDOR `0x5AU`
```



Inquire Vendor-ID 


        

<hr>



### define SYN\_LSS\_CS\_STORE\_CONFIG 

```C++
#define SYN_LSS_CS_STORE_CONFIG `0x17U`
```



Store Configuration 


        

<hr>



### define SYN\_LSS\_CS\_SWITCH\_MODE\_GLOBAL 

_LSS Command Specifiers (CS)_ 
```C++
#define SYN_LSS_CS_SWITCH_MODE_GLOBAL `0x04U`
```



Switch mode global 


        

<hr>



### define SYN\_LSS\_CS\_SWITCH\_MODE\_SELECT 

```C++
#define SYN_LSS_CS_SWITCH_MODE_SELECT `0x40U`
```



Switch mode selective vendor 


        

<hr>



### define SYN\_LSS\_MODE\_CONFIGURATION 

```C++
#define SYN_LSS_MODE_CONFIGURATION `0x01U`
```



LSS Configuration Mode 


        

<hr>



### define SYN\_LSS\_MODE\_OPERATION 

_LSS Operating Modes._ 
```C++
#define SYN_LSS_MODE_OPERATION `0x00U`
```



LSS Operation Mode 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_lss.h`

