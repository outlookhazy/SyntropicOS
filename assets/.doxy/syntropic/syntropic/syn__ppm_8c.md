

# File syn\_ppm.c



[**FileList**](files.md) **>** [**input**](dir_8ff21aebe696416b7e73f727e71a981c.md) **>** [**syn\_ppm.c**](syn__ppm_8c.md)

[Go to the source code of this file](syn__ppm_8c_source.md)

_Zero-Heap PPM Multi-Channel RC Receiver Decoder Implementation._ 

* `#include "syntropic/input/syn_ppm.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  uint16\_t | [**syn\_ppm\_get\_channel**](#function-syn_ppm_get_channel) (const [**SYN\_PPM\_Decoder**](structSYN__PPM__Decoder.md) \* ppm, uint8\_t channel\_idx) <br>_Get pulse width (us) for specified 0-indexed channel._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_ppm\_init**](#function-syn_ppm_init) ([**SYN\_PPM\_Decoder**](structSYN__PPM__Decoder.md) \* ppm) <br>_Initialize PPM decoder instance._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_ppm\_process\_pulse**](#function-syn_ppm_process_pulse) ([**SYN\_PPM\_Decoder**](structSYN__PPM__Decoder.md) \* ppm, uint16\_t pulse\_us) <br>_Process high-resolution measured pulse width (us) from Timer Input Capture interrupt._  |




























## Public Functions Documentation




### function syn\_ppm\_get\_channel 

_Get pulse width (us) for specified 0-indexed channel._ 
```C++
uint16_t syn_ppm_get_channel (
    const SYN_PPM_Decoder * ppm,
    uint8_t channel_idx
) 
```





**Parameters:**


* `ppm` Pointer to decoder struct. 
* `channel_idx` 0-indexed channel index (0..11). 



**Returns:**

Pulse width in microseconds (clamped 1000..2000 us, or 1500 us default if unreceived). 





        

<hr>



### function syn\_ppm\_init 

_Initialize PPM decoder instance._ 
```C++
SYN_Status syn_ppm_init (
    SYN_PPM_Decoder * ppm
) 
```





**Parameters:**


* `ppm` Pointer to decoder struct. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_ppm\_process\_pulse 

_Process high-resolution measured pulse width (us) from Timer Input Capture interrupt._ 
```C++
SYN_Status syn_ppm_process_pulse (
    SYN_PPM_Decoder * ppm,
    uint16_t pulse_us
) 
```





**Parameters:**


* `ppm` Pointer to decoder struct. 
* `pulse_us` Measured pulse width in microseconds (us). 



**Returns:**

SYN\_OK when a complete multi-channel PPM frame is finished, SYN\_BUSY during channel reception. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/input/syn_ppm.c`

