

# Struct SYN\_Servo



[**ClassList**](annotated.md) **>** [**SYN\_Servo**](structSYN__Servo.md)



_Hobby servo instance — pulse range, position, smooth movement._ 

* `#include <syn_servo.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint16\_t | [**angle\_range**](#variable-angle_range)  <br> |
|  uint16\_t | [**current\_us**](#variable-current_us)  <br> |
|  uint32\_t | [**last\_tick**](#variable-last_tick)  <br> |
|  uint16\_t | [**pulse\_max**](#variable-pulse_max)  <br> |
|  uint16\_t | [**pulse\_min**](#variable-pulse_min)  <br> |
|  int32\_t | [**rate**](#variable-rate)  <br> |
|  uint16\_t | [**target\_us**](#variable-target_us)  <br> |












































## Public Attributes Documentation




### variable angle\_range 

```C++
uint16_t SYN_Servo::angle_range;
```



Full range in degrees (e.g., 180) 
 


        

<hr>



### variable current\_us 

```C++
uint16_t SYN_Servo::current_us;
```



Current pulse width (µs) 
 


        

<hr>



### variable last\_tick 

```C++
uint32_t SYN_Servo::last_tick;
```



Last update tick 
 


        

<hr>



### variable pulse\_max 

```C++
uint16_t SYN_Servo::pulse_max;
```



Maximum pulse width in µs (e.g., 2000) 
 


        

<hr>



### variable pulse\_min 

```C++
uint16_t SYN_Servo::pulse_min;
```



Minimum pulse width in µs (e.g., 1000) 
 


        

<hr>



### variable rate 

```C++
int32_t SYN_Servo::rate;
```



Movement rate (µs per ms), 0 = instant 
 


        

<hr>



### variable target\_us 

```C++
uint16_t SYN_Servo::target_us;
```



Target pulse width (µs) 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/motor/syn_servo.h`

