

# Struct SYN\_DCMotor



[**ClassList**](annotated.md) **>** [**SYN\_DCMotor**](structSYN__DCMotor.md)



_DC motor instance — pins, speed, ramp state._ 

* `#include <syn_dc_motor.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  void \* | [**duty\_ctx**](#variable-duty_ctx)  <br> |
|  int32\_t | [**duty\_max**](#variable-duty_max)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**invert**](#variable-invert)  <br> |
|  uint32\_t | [**last\_tick**](#variable-last_tick)  <br> |
|  uint8\_t | [**mode**](#variable-mode)  <br> |
|  [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) | [**pin\_a**](#variable-pin_a)  <br> |
|  [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) | [**pin\_b**](#variable-pin_b)  <br> |
|  [**SYN\_Ramp**](structSYN__Ramp.md) | [**ramp**](#variable-ramp)  <br> |
|  int32\_t | [**ramp\_rate**](#variable-ramp_rate)  <br> |
|  void(\* | [**set\_duty**](#variable-set_duty)  <br> |
|  int32\_t | [**speed**](#variable-speed)  <br> |
|  int32\_t | [**target**](#variable-target)  <br> |












































## Public Attributes Documentation




### variable duty\_ctx 

```C++
void* SYN_DCMotor::duty_ctx;
```



Context for set\_duty 
 


        

<hr>



### variable duty\_max 

```C++
int32_t SYN_DCMotor::duty_max;
```



Maximum duty cycle value. Output range is [-duty\_max, +duty\_max]. Set to match your PWM timer resolution (e.g., 255 for 8-bit, 1000 for 0.1%, 4095 for 12-bit, 65535 for 16-bit). Default: SYN\_DC\_MOTOR\_DUTY\_MAX\_DEFAULT (1000). 


        

<hr>



### variable invert 

```C++
bool SYN_DCMotor::invert;
```



Invert direction 
 


        

<hr>



### variable last\_tick 

```C++
uint32_t SYN_DCMotor::last_tick;
```



Last ramp update tick 
 


        

<hr>



### variable mode 

```C++
uint8_t SYN_DCMotor::mode;
```



SYN\_DCMotorMode 
 


        

<hr>



### variable pin\_a 

```C++
SYN_GPIO_Pin SYN_DCMotor::pin_a;
```



PWM pin (or IN\_A for dual mode) 
 


        

<hr>



### variable pin\_b 

```C++
SYN_GPIO_Pin SYN_DCMotor::pin_b;
```



DIR pin (or IN\_B for dual mode) 
 


        

<hr>



### variable ramp 

```C++
SYN_Ramp SYN_DCMotor::ramp;
```



Internal velocity ramp generator 
 


        

<hr>



### variable ramp\_rate 

```C++
int32_t SYN_DCMotor::ramp_rate;
```



Rate of speed change (per ms, Q8) 
 


        

<hr>



### variable set\_duty 

```C++
void(* SYN_DCMotor::set_duty) (SYN_GPIO_Pin pin, uint16_t duty, void *ctx);
```



Duty output callback — user provides this to set actual PWM duty. 

**Parameters:**


* `pin` GPIO pin to set duty on. 
* `duty` Duty cycle value in range [0, duty\_max]. 
* `ctx` User context. 




        

<hr>



### variable speed 

```C++
int32_t SYN_DCMotor::speed;
```



Current speed (-duty\_max to +duty\_max) 


        

<hr>



### variable target 

```C++
int32_t SYN_DCMotor::target;
```



Target speed for ramping 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/motor/syn_dc_motor.h`

