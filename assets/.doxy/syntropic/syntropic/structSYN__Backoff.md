

# Struct SYN\_Backoff



[**ClassList**](annotated.md) **>** [**SYN\_Backoff**](structSYN__Backoff.md)



_Exponential backoff state._ 

* `#include <syn_backoff.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**attempts**](#variable-attempts)  <br> |
|  uint32\_t | [**current\_ms**](#variable-current_ms)  <br> |
|  uint8\_t | [**factor**](#variable-factor)  <br> |
|  uint8\_t | [**max\_attempts**](#variable-max_attempts)  <br> |
|  uint32\_t | [**max\_ms**](#variable-max_ms)  <br> |
|  uint32\_t | [**min\_ms**](#variable-min_ms)  <br> |












































## Public Attributes Documentation




### variable attempts 

```C++
uint8_t SYN_Backoff::attempts;
```



Number of failed attempts 
 


        

<hr>



### variable current\_ms 

```C++
uint32_t SYN_Backoff::current_ms;
```



Current timeout duration (ms) 


        

<hr>



### variable factor 

```C++
uint8_t SYN_Backoff::factor;
```



Multiplier (usually 2) 
 


        

<hr>



### variable max\_attempts 

```C++
uint8_t SYN_Backoff::max_attempts;
```



Maximum number of attempts 
 


        

<hr>



### variable max\_ms 

```C++
uint32_t SYN_Backoff::max_ms;
```



Maximum timeout (ms) 
 


        

<hr>



### variable min\_ms 

```C++
uint32_t SYN_Backoff::min_ms;
```



Initial timeout (ms) 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/util/syn_backoff.h`

