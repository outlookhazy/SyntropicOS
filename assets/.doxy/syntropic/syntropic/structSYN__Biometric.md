

# Struct SYN\_Biometric



[**ClassList**](annotated.md) **>** [**SYN\_Biometric**](structSYN__Biometric.md)



_Generic Biometric Sensor Context._ 

* `#include <syn_biometric.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**finger\_detected**](#variable-finger_detected)  <br> |
|  uint16\_t | [**heart\_rate\_bpm**](#variable-heart_rate_bpm)  <br> |
|  [**SYN\_SoftI2C**](structSYN__SoftI2C.md) | [**i2c**](#variable-i2c)  <br> |
|  uint8\_t | [**i2c\_addr**](#variable-i2c_addr)  <br> |
|  float | [**spo2\_pct**](#variable-spo2_pct)  <br> |
|  [**SYN\_BiometricType**](syn__biometric_8h.md#enum-syn_biometrictype) | [**type**](#variable-type)  <br> |












































## Public Attributes Documentation




### variable finger\_detected 

```C++
bool SYN_Biometric::finger_detected;
```



True if finger placed on sensor glass 


        

<hr>



### variable heart\_rate\_bpm 

```C++
uint16_t SYN_Biometric::heart_rate_bpm;
```



Measured Heart Rate in BPM (e.g. 72) 


        

<hr>



### variable i2c 

```C++
SYN_SoftI2C SYN_Biometric::i2c;
```



Software I2C bus context 


        

<hr>



### variable i2c\_addr 

```C++
uint8_t SYN_Biometric::i2c_addr;
```



7-bit I2C device address 


        

<hr>



### variable spo2\_pct 

```C++
float SYN_Biometric::spo2_pct;
```



Measured Blood Oxygen Saturation % (e.g. 98.5%) 


        

<hr>



### variable type 

```C++
SYN_BiometricType SYN_Biometric::type;
```



Biometric sensor model type 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/sensor/syn_biometric.h`

