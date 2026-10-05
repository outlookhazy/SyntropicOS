

# File syn\_biometric.h



[**FileList**](files.md) **>** [**sensor**](dir_3ec96d4fdb788c01e7dd1ddb7db4104c.md) **>** [**syn\_biometric.h**](syn__biometric_8h.md)

[Go to the source code of this file](syn__biometric_8h_source.md)

_Generic Pulse Oximeter & Heart Rate Sensor Driver (MAX30102, MAX30100)._ 

* `#include "../common/syn_defs.h"`
* `#include "../drivers/syn_soft_i2c.h"`
* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_Biometric**](structSYN__Biometric.md) <br>_Generic Biometric Sensor Context._  |


## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**SYN\_BiometricType**](#enum-syn_biometrictype)  <br>_Biometric Sensor Type._  |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_biometric\_feed\_samples**](#function-syn_biometric_feed_samples) ([**SYN\_Biometric**](structSYN__Biometric.md) \* bio, uint32\_t red\_raw, uint32\_t ir\_raw) <br>_Feed raw Red & IR photodiode samples._  |
|  uint16\_t | [**syn\_biometric\_get\_bpm**](#function-syn_biometric_get_bpm) (const [**SYN\_Biometric**](structSYN__Biometric.md) \* bio) <br>_Get measured Heart Rate in BPM._  |
|  float | [**syn\_biometric\_get\_spo2**](#function-syn_biometric_get_spo2) (const [**SYN\_Biometric**](structSYN__Biometric.md) \* bio) <br>_Get measured Blood Oxygen Saturation %._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_biometric\_init**](#function-syn_biometric_init) ([**SYN\_Biometric**](structSYN__Biometric.md) \* bio, [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) scl, [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) sda, uint8\_t i2c\_addr, [**SYN\_BiometricType**](syn__biometric_8h.md#enum-syn_biometrictype) type) <br>_Initialize Biometric Sensor._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_biometric\_is\_finger\_detected**](#function-syn_biometric_is_finger_detected) (const [**SYN\_Biometric**](structSYN__Biometric.md) \* bio) <br>_Check if finger is detected._  |




























## Public Types Documentation




### enum SYN\_BiometricType 

_Biometric Sensor Type._ 
```C++
enum SYN_BiometricType {
    SYN_BIOMETRIC_MAX30102 = 0,
    SYN_BIOMETRIC_MAX30100 = 1
};
```




<hr>
## Public Functions Documentation




### function syn\_biometric\_feed\_samples 

_Feed raw Red & IR photodiode samples._ 
```C++
void syn_biometric_feed_samples (
    SYN_Biometric * bio,
    uint32_t red_raw,
    uint32_t ir_raw
) 
```





**Parameters:**


* `bio` Biometric sensor context. 
* `red_raw` Raw 18-bit Red LED sample. 
* `ir_raw` Raw 18-bit IR LED sample. 




        

<hr>



### function syn\_biometric\_get\_bpm 

_Get measured Heart Rate in BPM._ 
```C++
uint16_t syn_biometric_get_bpm (
    const SYN_Biometric * bio
) 
```





**Parameters:**


* `bio` Biometric sensor context. 



**Returns:**

Heart rate in Beats Per Minute. 





        

<hr>



### function syn\_biometric\_get\_spo2 

_Get measured Blood Oxygen Saturation %._ 
```C++
float syn_biometric_get_spo2 (
    const SYN_Biometric * bio
) 
```





**Parameters:**


* `bio` Biometric sensor context. 



**Returns:**

SpO2 percentage. 





        

<hr>



### function syn\_biometric\_init 

_Initialize Biometric Sensor._ 
```C++
SYN_Status syn_biometric_init (
    SYN_Biometric * bio,
    SYN_GPIO_Pin scl,
    SYN_GPIO_Pin sda,
    uint8_t i2c_addr,
    SYN_BiometricType type
) 
```





**Parameters:**


* `bio` Biometric sensor context. 
* `scl` I2C SCL GPIO pin. 
* `sda` I2C SDA GPIO pin. 
* `i2c_addr` I2C slave address (e.g. 0x57). 
* `type` Sensor type (MAX30102 or MAX30100). 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_biometric\_is\_finger\_detected 

_Check if finger is detected._ 
```C++
bool syn_biometric_is_finger_detected (
    const SYN_Biometric * bio
) 
```





**Parameters:**


* `bio` Biometric sensor context. 



**Returns:**

True if finger present. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/sensor/syn_biometric.h`

