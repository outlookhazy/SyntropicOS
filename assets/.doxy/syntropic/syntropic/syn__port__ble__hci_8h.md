

# File syn\_port\_ble\_hci.h



[**FileList**](files.md) **>** [**port**](dir_df8aee55eb103ebca23cd86ea02a32de.md) **>** [**syn\_port\_ble\_hci.h**](syn__port__ble__hci_8h.md)

[Go to the source code of this file](syn__port__ble__hci_8h_source.md)

_Port Abstraction Header for Bluetooth HCI Transport (UART H:4 / SPI)._ 

* `#include "../common/syn_defs.h"`
* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_ble\_hci\_reset**](#function-syn_port_ble_hci_reset) (void) <br>_Reset external BLE controller hardware via GPIO reset pin (if present)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_port\_ble\_hci\_tx**](#function-syn_port_ble_hci_tx) (const uint8\_t \* data, uint16\_t len) <br>_Transmit raw bytes to external BLE controller via UART/SPI hardware._  |




























## Public Functions Documentation




### function syn\_port\_ble\_hci\_reset 

_Reset external BLE controller hardware via GPIO reset pin (if present)._ 
```C++
SYN_Status syn_port_ble_hci_reset (
    void
) 
```





**Returns:**

SYN\_OK on success, or SYN\_ERR\_NOT\_SUPPORTED if unmapped. 





        

<hr>



### function syn\_port\_ble\_hci\_tx 

_Transmit raw bytes to external BLE controller via UART/SPI hardware._ 
```C++
SYN_Status syn_port_ble_hci_tx (
    const uint8_t * data,
    uint16_t len
) 
```





**Parameters:**


* `data` Data buffer pointer. 
* `len` Data length in bytes. 



**Returns:**

SYN\_OK on success. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/port/syn_port_ble_hci.h`

