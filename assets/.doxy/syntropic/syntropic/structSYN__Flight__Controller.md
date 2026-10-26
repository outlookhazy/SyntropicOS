

# Struct SYN\_Flight\_Controller



[**ClassList**](annotated.md) **>** [**SYN\_Flight\_Controller**](structSYN__Flight__Controller.md)



[More...](#detailed-description)

* `#include <syn_flight_pid.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_PID**](structSYN__PID.md) | [**pid\_angle\_pitch**](#variable-pid_angle_pitch)  <br> |
|  [**SYN\_PID**](structSYN__PID.md) | [**pid\_angle\_roll**](#variable-pid_angle_roll)  <br> |
|  [**SYN\_PID**](structSYN__PID.md) | [**pid\_rate\_pitch**](#variable-pid_rate_pitch)  <br> |
|  [**SYN\_PID**](structSYN__PID.md) | [**pid\_rate\_roll**](#variable-pid_rate_roll)  <br> |
|  [**SYN\_PID**](structSYN__PID.md) | [**pid\_rate\_yaw**](#variable-pid_rate_yaw)  <br> |












































## Detailed Description


3-Axis Flight PID Controller Instance. 


    
## Public Attributes Documentation




### variable pid\_angle\_pitch 

```C++
SYN_PID SYN_Flight_Controller::pid_angle_pitch;
```



Outer angle loop PID controller for Pitch axis 


        

<hr>



### variable pid\_angle\_roll 

```C++
SYN_PID SYN_Flight_Controller::pid_angle_roll;
```



Outer angle loop PID controller for Roll axis 


        

<hr>



### variable pid\_rate\_pitch 

```C++
SYN_PID SYN_Flight_Controller::pid_rate_pitch;
```



Inner rate loop PID controller for Pitch axis 


        

<hr>



### variable pid\_rate\_roll 

```C++
SYN_PID SYN_Flight_Controller::pid_rate_roll;
```



Inner rate loop PID controller for Roll axis 


        

<hr>



### variable pid\_rate\_yaw 

```C++
SYN_PID SYN_Flight_Controller::pid_rate_yaw;
```



Inner rate loop PID controller for Yaw axis 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/control/syn_flight_pid.h`

