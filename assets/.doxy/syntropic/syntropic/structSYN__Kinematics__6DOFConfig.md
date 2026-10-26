

# Struct SYN\_Kinematics\_6DOFConfig



[**ClassList**](annotated.md) **>** [**SYN\_Kinematics\_6DOFConfig**](structSYN__Kinematics__6DOFConfig.md)



_Configuration parameters for 6-DOF Articulated Arm with Spherical Wrist._ 

* `#include <syn_kinematics.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**a1**](#variable-a1)  <br> |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**a2**](#variable-a2)  <br> |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**d1**](#variable-d1)  <br> |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**d4**](#variable-d4)  <br> |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**d6**](#variable-d6)  <br> |












































## Public Attributes Documentation




### variable a1 

```C++
q16_t SYN_Kinematics_6DOFConfig::a1;
```



Shoulder offset along X axis 


        

<hr>



### variable a2 

```C++
q16_t SYN_Kinematics_6DOFConfig::a2;
```



Upper arm length (shoulder to elbow) 


        

<hr>



### variable d1 

```C++
q16_t SYN_Kinematics_6DOFConfig::d1;
```



Base height (Z offset from base to shoulder) 


        

<hr>



### variable d4 

```C++
q16_t SYN_Kinematics_6DOFConfig::d4;
```



Forearm length (elbow to wrist center) 


        

<hr>



### variable d6 

```C++
q16_t SYN_Kinematics_6DOFConfig::d6;
```



Tool flange offset (wrist center to tool tip) 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/motor/syn_kinematics.h`

