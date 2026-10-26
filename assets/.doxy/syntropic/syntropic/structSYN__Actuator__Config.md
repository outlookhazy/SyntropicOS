

# Struct SYN\_Actuator\_Config



[**ClassList**](annotated.md) **>** [**SYN\_Actuator\_Config**](structSYN__Actuator__Config.md)



_Actuator configuration._ 

* `#include <syn_actuator.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_DCMotor**](structSYN__DCMotor.md) \* | [**dc\_motor**](#variable-dc_motor)  <br> |
|  int32\_t | [**deadband**](#variable-deadband)  <br> |
|  [**SYN\_ErrLog**](structSYN__ErrLog.md) \* | [**errlog**](#variable-errlog)  <br> |
|  int32\_t | [**pid\_kd**](#variable-pid_kd)  <br> |
|  int32\_t | [**pid\_ki**](#variable-pid_ki)  <br> |
|  int32\_t | [**pid\_kp**](#variable-pid_kp)  <br> |
|  uint8\_t | [**pid\_scale**](#variable-pid_scale)  <br> |
|  void \* | [**read\_ctx**](#variable-read_ctx)  <br> |
|  [**SYN\_MotorCtrl\_ReadPos**](syn__motor__ctrl_8h.md#typedef-syn_motorctrl_readpos) | [**read\_pos**](#variable-read_pos)  <br> |
|  int32\_t | [**stall\_threshold**](#variable-stall_threshold)  <br> |
|  uint16\_t | [**stall\_timeout\_ms**](#variable-stall_timeout_ms)  <br> |
|  int32\_t | [**stroke\_max**](#variable-stroke_max)  <br> |
|  int32\_t | [**stroke\_min**](#variable-stroke_min)  <br> |
|  uint16\_t | [**update\_hz**](#variable-update_hz)  <br> |












































## Public Attributes Documentation




### variable dc\_motor 

```C++
SYN_DCMotor* SYN_Actuator_Config::dc_motor;
```



DC motor for actuation 
 


        

<hr>



### variable deadband 

```C++
int32_t SYN_Actuator_Config::deadband;
```



Position deadband (ADC units) 
 


        

<hr>



### variable errlog 

```C++
SYN_ErrLog* SYN_Actuator_Config::errlog;
```



Optional error logging 
 


        

<hr>



### variable pid\_kd 

```C++
int32_t SYN_Actuator_Config::pid_kd;
```



PID derivative gain 
 


        

<hr>



### variable pid\_ki 

```C++
int32_t SYN_Actuator_Config::pid_ki;
```



PID integral gain 
 


        

<hr>



### variable pid\_kp 

```C++
int32_t SYN_Actuator_Config::pid_kp;
```



PID proportional gain 
 


        

<hr>



### variable pid\_scale 

```C++
uint8_t SYN_Actuator_Config::pid_scale;
```



Gain divisor = 1 &lt;&lt; pid\_scale 
 


        

<hr>



### variable read\_ctx 

```C++
void* SYN_Actuator_Config::read_ctx;
```



Context for read\_pos 
 


        

<hr>



### variable read\_pos 

```C++
SYN_MotorCtrl_ReadPos SYN_Actuator_Config::read_pos;
```



Read potentiometer (returns int32\_t) 


        

<hr>



### variable stall\_threshold 

```C++
int32_t SYN_Actuator_Config::stall_threshold;
```



Min motion for "not stalled" 
 


        

<hr>



### variable stall\_timeout\_ms 

```C++
uint16_t SYN_Actuator_Config::stall_timeout_ms;
```



0 = disabled 
 


        

<hr>



### variable stroke\_max 

```C++
int32_t SYN_Actuator_Config::stroke_max;
```



ADC value at fully extended 
 


        

<hr>



### variable stroke\_min 

```C++
int32_t SYN_Actuator_Config::stroke_min;
```



ADC value at fully retracted 
 


        

<hr>



### variable update\_hz 

```C++
uint16_t SYN_Actuator_Config::update_hz;
```



Control loop frequency 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/motor/syn_actuator.h`

