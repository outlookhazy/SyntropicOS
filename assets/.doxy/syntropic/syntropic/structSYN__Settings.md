

# Struct SYN\_Settings



[**ClassList**](annotated.md) **>** [**SYN\_Settings**](structSYN__Settings.md)



_Persistent settings instance — flash-backed with change detection._ 

* `#include <syn_settings.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint16\_t | [**checksum**](#variable-checksum)  <br> |
|  void \* | [**data**](#variable-data)  <br> |
|  uint16\_t | [**data\_size**](#variable-data_size)  <br> |
|  const void \* | [**defaults**](#variable-defaults)  <br> |
|  [**SYN\_SettingsChangeCallback**](syn__settings_8h.md#typedef-syn_settingschangecallback) | [**on\_change**](#variable-on_change)  <br> |
|  void \* | [**on\_change\_ctx**](#variable-on_change_ctx)  <br> |
|  [**SYN\_ParamStore**](structSYN__ParamStore.md) | [**store**](#variable-store)  <br> |












































## Public Attributes Documentation




### variable checksum 

```C++
uint16_t SYN_Settings::checksum;
```



CRC-16 at last save/load 
 


        

<hr>



### variable data 

```C++
void* SYN_Settings::data;
```



Pointer to user's settings struct 


        

<hr>



### variable data\_size 

```C++
uint16_t SYN_Settings::data_size;
```



sizeof(settings struct) 
 


        

<hr>



### variable defaults 

```C++
const void* SYN_Settings::defaults;
```



Pointer to const default values 
 


        

<hr>



### variable on\_change 

```C++
SYN_SettingsChangeCallback SYN_Settings::on_change;
```



Change callback 
 


        

<hr>



### variable on\_change\_ctx 

```C++
void* SYN_Settings::on_change_ctx;
```



Callback context 
 


        

<hr>



### variable store 

```C++
SYN_ParamStore SYN_Settings::store;
```



Backing wear-leveled flash store 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/storage/syn_settings.h`

