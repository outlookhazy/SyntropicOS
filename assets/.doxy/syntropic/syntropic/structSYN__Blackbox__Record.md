

# Struct SYN\_Blackbox\_Record



[**ClassList**](annotated.md) **>** [**SYN\_Blackbox\_Record**](structSYN__Blackbox__Record.md)



[More...](#detailed-description)

* `#include <syn_blackbox.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  int16\_t | [**accel**](#variable-accel)  <br> |
|  int16\_t | [**gyro**](#variable-gyro)  <br> |
|  uint32\_t | [**iteration**](#variable-iteration)  <br> |
|  uint16\_t | [**motor**](#variable-motor)  <br> |
|  int16\_t | [**setpoint**](#variable-setpoint)  <br> |
|  uint32\_t | [**time\_us**](#variable-time_us)  <br> |












































## Detailed Description


Blackbox Flight State Record. 


    
## Public Attributes Documentation




### variable accel 

```C++
int16_t SYN_Blackbox_Record::accel[3];
```



X, Y, Z mg 
 


        

<hr>



### variable gyro 

```C++
int16_t SYN_Blackbox_Record::gyro[3];
```



Roll, Pitch, Yaw deg/s 


        

<hr>



### variable iteration 

```C++
uint32_t SYN_Blackbox_Record::iteration;
```



Main loop iteration counter 


        

<hr>



### variable motor 

```C++
uint16_t SYN_Blackbox_Record::motor[4];
```



Motor 1..4 PWM/DShot us 
 


        

<hr>



### variable setpoint 

```C++
int16_t SYN_Blackbox_Record::setpoint[4];
```



Roll, Pitch, Yaw, Throttle 


        

<hr>



### variable time\_us 

```C++
uint32_t SYN_Blackbox_Record::time_us;
```



Microsecond timestamp 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/log/syn_blackbox.h`

