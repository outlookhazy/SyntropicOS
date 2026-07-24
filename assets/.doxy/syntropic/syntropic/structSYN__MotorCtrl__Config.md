

# Struct SYN\_MotorCtrl\_Config



[**ClassList**](annotated.md) **>** [**SYN\_MotorCtrl\_Config**](structSYN__MotorCtrl__Config.md)



_Motor controller configuration (passed to init, copied internally)._ 

* `#include <syn_motor_ctrl.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_ErrLog**](structSYN__ErrLog.md) \* | [**errlog**](#variable-errlog)  <br> |
|  int32\_t | [**ff\_ka**](#variable-ff_ka)  <br> |
|  int32\_t | [**ff\_kv**](#variable-ff_kv)  <br> |
|  uint8\_t | [**ff\_scale**](#variable-ff_scale)  <br> |
|  [**SYN\_MotorOutput**](structSYN__MotorOutput.md) | [**motor**](#variable-motor)  <br> |
|  int32\_t | [**output\_max**](#variable-output_max)  <br> |
|  int32\_t | [**output\_min**](#variable-output_min)  <br> |
|  int32\_t | [**pid\_kd**](#variable-pid_kd)  <br>_PID derivative gain (÷ 1 &lt;&lt; pid\_scale)._  |
|  int32\_t | [**pid\_ki**](#variable-pid_ki)  <br>_PID integral gain (÷ 1 &lt;&lt; pid\_scale)._  |
|  int32\_t | [**pid\_kp**](#variable-pid_kp)  <br>_PID proportional gain (÷ 1 &lt;&lt; pid\_scale)._  |
|  uint8\_t | [**pid\_scale**](#variable-pid_scale)  <br> |
|  int32\_t | [**position\_deadband**](#variable-position_deadband)  <br> |
|  int32\_t | [**position\_max**](#variable-position_max)  <br> |
|  int32\_t | [**position\_min**](#variable-position_min)  <br> |
|  [**SYN\_MotorCtrl\_ReadPos**](syn__motor__ctrl_8h.md#typedef-syn_motorctrl_readpos) | [**read\_pos**](#variable-read_pos)  <br> |
|  void \* | [**read\_pos\_ctx**](#variable-read_pos_ctx)  <br> |
|  int32\_t | [**stall\_threshold**](#variable-stall_threshold)  <br> |
|  uint16\_t | [**stall\_timeout\_ms**](#variable-stall_timeout_ms)  <br> |
|  uint16\_t | [**update\_hz**](#variable-update_hz)  <br> |












































## Public Attributes Documentation




### variable errlog 

```C++
SYN_ErrLog* SYN_MotorCtrl_Config::errlog;
```



If set, stall/limit events logged 


        

<hr>



### variable ff\_ka 

```C++
int32_t SYN_MotorCtrl_Config::ff_ka;
```



Acceleration feedforward gain 
 


        

<hr>



### variable ff\_kv 

```C++
int32_t SYN_MotorCtrl_Config::ff_kv;
```



Velocity feedforward gain 
 


        

<hr>



### variable ff\_scale 

```C++
uint8_t SYN_MotorCtrl_Config::ff_scale;
```



FF divisor = 1 &lt;&lt; ff\_scale 
 


        

<hr>



### variable motor 

```C++
SYN_MotorOutput SYN_MotorCtrl_Config::motor;
```



Motor output interface 
 


        

<hr>



### variable output\_max 

```C++
int32_t SYN_MotorCtrl_Config::output_max;
```



Max PID output (e.g., +255) 
 


        

<hr>



### variable output\_min 

```C++
int32_t SYN_MotorCtrl_Config::output_min;
```



Min PID output (e.g., -255) 
 


        

<hr>



### variable pid\_kd 

_PID derivative gain (÷ 1 &lt;&lt; pid\_scale)._ 
```C++
int32_t SYN_MotorCtrl_Config::pid_kd;
```




<hr>



### variable pid\_ki 

_PID integral gain (÷ 1 &lt;&lt; pid\_scale)._ 
```C++
int32_t SYN_MotorCtrl_Config::pid_ki;
```




<hr>



### variable pid\_kp 

_PID proportional gain (÷ 1 &lt;&lt; pid\_scale)._ 
```C++
int32_t SYN_MotorCtrl_Config::pid_kp;
```




<hr>



### variable pid\_scale 

```C++
uint8_t SYN_MotorCtrl_Config::pid_scale;
```



Gain divisor = 1 &lt;&lt; pid\_scale 
 


        

<hr>



### variable position\_deadband 

```C++
int32_t SYN_MotorCtrl_Config::position_deadband;
```



Units within target = done 


        

<hr>



### variable position\_max 

```C++
int32_t SYN_MotorCtrl_Config::position_max;
```



Max allowed position (soft limit) 


        

<hr>



### variable position\_min 

```C++
int32_t SYN_MotorCtrl_Config::position_min;
```



Min allowed position (soft limit) 


        

<hr>



### variable read\_pos 

```C++
SYN_MotorCtrl_ReadPos SYN_MotorCtrl_Config::read_pos;
```



Position read function 
 


        

<hr>



### variable read\_pos\_ctx 

```C++
void* SYN_MotorCtrl_Config::read_pos_ctx;
```



Context for read\_pos 
 


        

<hr>



### variable stall\_threshold 

```C++
int32_t SYN_MotorCtrl_Config::stall_threshold;
```



Min units/period for "not stalled" 


        

<hr>



### variable stall\_timeout\_ms 

```C++
uint16_t SYN_MotorCtrl_Config::stall_timeout_ms;
```



0 = disabled 
 


        

<hr>



### variable update\_hz 

```C++
uint16_t SYN_MotorCtrl_Config::update_hz;
```



Control loop frequency 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/motor/syn_motor_ctrl.h`

