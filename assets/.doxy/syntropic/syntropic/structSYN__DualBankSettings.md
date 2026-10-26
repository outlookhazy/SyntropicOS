

# Struct SYN\_DualBankSettings



[**ClassList**](annotated.md) **>** [**SYN\_DualBankSettings**](structSYN__DualBankSettings.md)



_Dual-bank settings container for atomic power-loss safe writes._ 

* `#include <syn_settings.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**active\_bank**](#variable-active_bank)  <br> |
|  uint32\_t | [**active\_crc32**](#variable-active_crc32)  <br> |
|  [**SYN\_Settings**](structSYN__Settings.md) | [**bank\_a**](#variable-bank_a)  <br> |
|  [**SYN\_Settings**](structSYN__Settings.md) | [**bank\_b**](#variable-bank_b)  <br> |












































## Public Attributes Documentation




### variable active\_bank 

```C++
uint8_t SYN_DualBankSettings::active_bank;
```



0 = Bank A, 1 = Bank B 


        

<hr>



### variable active\_crc32 

```C++
uint32_t SYN_DualBankSettings::active_crc32;
```



CRC32 of active bank 
 


        

<hr>



### variable bank\_a 

```C++
SYN_Settings SYN_DualBankSettings::bank_a;
```



Primary flash bank A 
 


        

<hr>



### variable bank\_b 

```C++
SYN_Settings SYN_DualBankSettings::bank_b;
```



Secondary flash bank B 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/storage/syn_settings.h`

