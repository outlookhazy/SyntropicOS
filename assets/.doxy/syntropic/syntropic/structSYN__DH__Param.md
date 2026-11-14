

# Struct SYN\_DH\_Param



[**ClassList**](annotated.md) **>** [**SYN\_DH\_Param**](structSYN__DH__Param.md)



_Single joint Denavit-Hartenberg parameter specification._ 

* `#include <syn_kinematics.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**a**](#variable-a)  <br> |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**alpha**](#variable-alpha)  <br> |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**d**](#variable-d)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**is\_prismatic**](#variable-is_prismatic)  <br> |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**max\_limit**](#variable-max_limit)  <br> |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**min\_limit**](#variable-min_limit)  <br> |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**theta**](#variable-theta)  <br> |












































## Public Attributes Documentation




### variable a 

```C++
q16_t SYN_DH_Param::a;
```



Link length along X axis 


        

<hr>



### variable alpha 

```C++
q16_t SYN_DH_Param::alpha;
```



Link twist angle around X axis (radians) 


        

<hr>



### variable d 

```C++
q16_t SYN_DH_Param::d;
```



Link offset along Z axis (prismatic joint variable if is\_prismatic) 


        

<hr>



### variable is\_prismatic 

```C++
bool SYN_DH_Param::is_prismatic;
```



True if prismatic (linear) joint, false if revolute 


        

<hr>



### variable max\_limit 

```C++
q16_t SYN_DH_Param::max_limit;
```



Maximum joint position limit 


        

<hr>



### variable min\_limit 

```C++
q16_t SYN_DH_Param::min_limit;
```



Minimum joint position limit 


        

<hr>



### variable theta 

```C++
q16_t SYN_DH_Param::theta;
```



Joint angle around Z axis (revolute joint variable if !is\_prismatic) 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/motor/syn_kinematics.h`

