

# Struct SYN\_Scale



[**ClassList**](annotated.md) **>** [**SYN\_Scale**](structSYN__Scale.md)



_Generic Weight Scale Instance Context._ 

* `#include <syn_scale.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) | [**dout\_pin**](#variable-dout_pin)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**is\_stable**](#variable-is_stable)  <br> |
|  float | [**last\_weight\_grams**](#variable-last_weight_grams)  <br> |
|  float | [**scale\_factor**](#variable-scale_factor)  <br> |
|  [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) | [**sck\_pin**](#variable-sck_pin)  <br> |
|  int32\_t | [**tare\_offset**](#variable-tare_offset)  <br> |
|  [**SYN\_ScaleType**](syn__scale_8h.md#enum-syn_scaletype) | [**type**](#variable-type)  <br> |












































## Public Attributes Documentation




### variable dout\_pin 

```C++
SYN_GPIO_Pin SYN_Scale::dout_pin;
```



Serial data out / SDA GPIO pin 


        

<hr>



### variable is\_stable 

```C++
bool SYN_Scale::is_stable;
```



True if scale reading has settled 


        

<hr>



### variable last\_weight\_grams 

```C++
float SYN_Scale::last_weight_grams;
```



Calculated weight in grams 


        

<hr>



### variable scale\_factor 

```C++
float SYN_Scale::scale_factor;
```



Counts per gram scale factor 


        

<hr>



### variable sck\_pin 

```C++
SYN_GPIO_Pin SYN_Scale::sck_pin;
```



Serial clock / SCL GPIO pin 


        

<hr>



### variable tare\_offset 

```C++
int32_t SYN_Scale::tare_offset;
```



Zero-point tare offset raw reading 


        

<hr>



### variable type 

```C++
SYN_ScaleType SYN_Scale::type;
```



Scale ADC model type 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/sensor/syn_scale.h`

