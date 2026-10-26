

# File syn\_foc\_encoder.c



[**FileList**](files.md) **>** [**motor**](dir_61470c21c7c2e7c9f977d1e3c2e6c7e0.md) **>** [**syn\_foc\_encoder.c**](syn__foc__encoder_8c.md)

[Go to the source code of this file](syn__foc__encoder_8c_source.md)

_Sensored FOC Rotor Position & Speed Implementation._ 

* `#include "syn_foc_encoder.h"`
* `#include <string.h>`























## Public Static Attributes

| Type | Name |
| ---: | :--- |
|  const [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**g\_hall\_angle\_lut**](#variable-g_hall_angle_lut)   = `/* multi line expression */`<br>_Standard 120-degree 6-step Hall state to electrical angle lookup table (rad in Q16)._  |














## Public Functions

| Type | Name |
| ---: | :--- |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**syn\_foc\_encoder\_get\_elec\_angle**](#function-syn_foc_encoder_get_elec_angle) (const [**SYN\_FOCEncoder**](structSYN__FOCEncoder.md) \* enc) <br>_Get current electrical angle in Q16 radians [0, 2\*PI)._  |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**syn\_foc\_encoder\_get\_elec\_speed**](#function-syn_foc_encoder_get_elec_speed) (const [**SYN\_FOCEncoder**](structSYN__FOCEncoder.md) \* enc) <br>_Get current electrical speed in Q16 rad/s._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_foc\_encoder\_init**](#function-syn_foc_encoder_init) ([**SYN\_FOCEncoder**](structSYN__FOCEncoder.md) \* enc, const [**SYN\_FOCEncoderConfig**](structSYN__FOCEncoderConfig.md) \* cfg) <br>_Initialize sensored encoder module._  |
|  void | [**syn\_foc\_encoder\_update\_absolute**](#function-syn_foc_encoder_update_absolute) ([**SYN\_FOCEncoder**](structSYN__FOCEncoder.md) \* enc, uint16\_t raw\_angle\_14bit) <br>_Update position for Absolute Encoder._  |
|  void | [**syn\_foc\_encoder\_update\_hall**](#function-syn_foc_encoder_update_hall) ([**SYN\_FOCEncoder**](structSYN__FOCEncoder.md) \* enc, [**bool**](syn__defs_8h.md#enum-bool) hall\_u, [**bool**](syn__defs_8h.md#enum-bool) hall\_v, [**bool**](syn__defs_8h.md#enum-bool) hall\_w) <br>_Update position for 3-Phase Digital Hall Sensors._  |
|  void | [**syn\_foc\_encoder\_update\_quadrature**](#function-syn_foc_encoder_update_quadrature) ([**SYN\_FOCEncoder**](structSYN__FOCEncoder.md) \* enc, int32\_t raw\_count) <br>_Update position for ABZ Incremental Quadrature Encoder._  |




























## Public Static Attributes Documentation




### variable g\_hall\_angle\_lut 

_Standard 120-degree 6-step Hall state to electrical angle lookup table (rad in Q16)._ 
```C++
const q16_t g_hall_angle_lut[8];
```




<hr>
## Public Functions Documentation




### function syn\_foc\_encoder\_get\_elec\_angle 

_Get current electrical angle in Q16 radians [0, 2\*PI)._ 
```C++
q16_t syn_foc_encoder_get_elec_angle (
    const SYN_FOCEncoder * enc
) 
```





**Parameters:**


* `enc` Pointer to encoder instance. 



**Returns:**

Current electrical angle in Q16 radians. 





        

<hr>



### function syn\_foc\_encoder\_get\_elec\_speed 

_Get current electrical speed in Q16 rad/s._ 
```C++
q16_t syn_foc_encoder_get_elec_speed (
    const SYN_FOCEncoder * enc
) 
```





**Parameters:**


* `enc` Pointer to encoder instance. 



**Returns:**

Current electrical speed in Q16 rad/s. 





        

<hr>



### function syn\_foc\_encoder\_init 

_Initialize sensored encoder module._ 
```C++
SYN_Status syn_foc_encoder_init (
    SYN_FOCEncoder * enc,
    const SYN_FOCEncoderConfig * cfg
) 
```





**Parameters:**


* `enc` Pointer to encoder instance. 
* `cfg` Pointer to encoder configuration. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_foc\_encoder\_update\_absolute 

_Update position for Absolute Encoder._ 
```C++
void syn_foc_encoder_update_absolute (
    SYN_FOCEncoder * enc,
    uint16_t raw_angle_14bit
) 
```





**Parameters:**


* `enc` Pointer to encoder instance. 
* `raw_angle_14bit` Raw 14-bit absolute angle (0..16383). 




        

<hr>



### function syn\_foc\_encoder\_update\_hall 

_Update position for 3-Phase Digital Hall Sensors._ 
```C++
void syn_foc_encoder_update_hall (
    SYN_FOCEncoder * enc,
    bool hall_u,
    bool hall_v,
    bool hall_w
) 
```





**Parameters:**


* `enc` Pointer to encoder instance. 
* `hall_u` Digital pin U state (0 or 1). 
* `hall_v` Digital pin V state (0 or 1). 
* `hall_w` Digital pin W state (0 or 1). 




        

<hr>



### function syn\_foc\_encoder\_update\_quadrature 

_Update position for ABZ Incremental Quadrature Encoder._ 
```C++
void syn_foc_encoder_update_quadrature (
    SYN_FOCEncoder * enc,
    int32_t raw_count
) 
```





**Parameters:**


* `enc` Pointer to encoder instance. 
* `raw_count` Current timer counter register (QEI count). 




        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/motor/syn_foc_encoder.c`

