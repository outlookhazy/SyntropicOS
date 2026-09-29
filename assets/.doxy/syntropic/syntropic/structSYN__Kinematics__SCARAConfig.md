

# Struct SYN\_Kinematics\_SCARAConfig



[**ClassList**](annotated.md) **>** [**SYN\_Kinematics\_SCARAConfig**](structSYN__Kinematics__SCARAConfig.md)



_Configuration parameters for 4-DOF SCARA Robot._ 

* `#include <syn_kinematics.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**d\_max**](#variable-d_max)  <br> |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**l1**](#variable-l1)  <br> |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**l2**](#variable-l2)  <br> |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**z\_home**](#variable-z_home)  <br> |












































## Public Attributes Documentation




### variable d\_max 

```C++
q16_t SYN_Kinematics_SCARAConfig::d_max;
```



Maximum vertical stroke (Z axis) 


        

<hr>



### variable l1 

```C++
q16_t SYN_Kinematics_SCARAConfig::l1;
```



Link 1 length (base to joint 2) 


        

<hr>



### variable l2 

```C++
q16_t SYN_Kinematics_SCARAConfig::l2;
```



Link 2 length (joint 2 to quill) 


        

<hr>



### variable z\_home 

```C++
q16_t SYN_Kinematics_SCARAConfig::z_home;
```



Z-axis home offset 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/motor/syn_kinematics.h`

