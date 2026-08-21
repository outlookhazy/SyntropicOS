

# Struct SYN\_MotorOutput



[**ClassList**](annotated.md) **>** [**SYN\_MotorOutput**](structSYN__MotorOutput.md)



_Abstract motor output interface (vtable)._ [More...](#detailed-description)

* `#include <syn_motor_output.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  void(\* | [**brake**](#variable-brake)  <br>_Active brake — short motor windings or active stop._  |
|  void(\* | [**coast**](#variable-coast)  <br>_Coast — free-spin, both outputs off._  |
|  void \* | [**ctx**](#variable-ctx)  <br>_User context passed to all callbacks._  |
|  void(\* | [**set\_output**](#variable-set_output)  <br>_Drive the motor at the given signed output level._  |












































## Detailed Description


The motor controller calls these functions to drive the motor without knowing the underlying hardware. This allows any motor driver to be used with syn\_motor\_ctrl, syn\_autotune, etc. 


    
## Public Attributes Documentation




### variable brake 

_Active brake — short motor windings or active stop._ 
```C++
void(* SYN_MotorOutput::brake) (void *ctx);
```



Motor decelerates as quickly as possible. Called during emergency stop (e-stop).




**Parameters:**


* `ctx` User context. 




        

<hr>



### variable coast 

_Coast — free-spin, both outputs off._ 
```C++
void(* SYN_MotorOutput::coast) (void *ctx);
```



Motor decelerates due to friction only. Called when the controller enters idle or stops normally.




**Parameters:**


* `ctx` User context. 




        

<hr>



### variable ctx 

_User context passed to all callbacks._ 
```C++
void* SYN_MotorOutput::ctx;
```




<hr>



### variable set\_output 

_Drive the motor at the given signed output level._ 
```C++
void(* SYN_MotorOutput::set_output) (void *ctx, int32_t output);
```



The output range matches the controller's [output\_min, output\_max]. Positive = forward, negative = reverse, 0 = stop. The implementation must map this to the appropriate hardware command (PWM duty, step rate, etc.).




**Parameters:**


* `ctx` User context (motor instance pointer). 
* `output` Signed output level. 




        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/motor/syn_motor_output.h`

