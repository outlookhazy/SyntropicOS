

# Struct SYN\_Kinematics\_DeltaConfig



[**ClassList**](annotated.md) **>** [**SYN\_Kinematics\_DeltaConfig**](structSYN__Kinematics__DeltaConfig.md)



_Configuration parameters for 3-Axis Delta Parallel Robot._ 

* `#include <syn_kinematics.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**base\_radius**](#variable-base_radius)  <br> |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**effector\_radius**](#variable-effector_radius)  <br> |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**lower\_arm\_len**](#variable-lower_arm_len)  <br> |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**upper\_arm\_len**](#variable-upper_arm_len)  <br> |












































## Public Attributes Documentation




### variable base\_radius 

```C++
q16_t SYN_Kinematics_DeltaConfig::base_radius;
```



Base equilateral triangle / platform radius (sb) 


        

<hr>



### variable effector\_radius 

```C++
q16_t SYN_Kinematics_DeltaConfig::effector_radius;
```



End-effector platform radius (sp) 


        

<hr>



### variable lower\_arm\_len 

```C++
q16_t SYN_Kinematics_DeltaConfig::lower_arm_len;
```



Lower parallel arm / parallelogram rod length (l) 


        

<hr>



### variable upper\_arm\_len 

```C++
q16_t SYN_Kinematics_DeltaConfig::upper_arm_len;
```



Upper arm / bicep length (L) 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/motor/syn_kinematics.h`

