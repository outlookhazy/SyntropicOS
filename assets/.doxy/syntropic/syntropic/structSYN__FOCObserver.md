

# Struct SYN\_FOCObserver



[**ClassList**](annotated.md) **>** [**SYN\_FOCObserver**](structSYN__FOCObserver.md)



_Sliding Mode Observer state structure._ 

* `#include <syn_foc_observer.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**E\_alpha\_est**](#variable-e_alpha_est)  <br> |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**E\_beta\_est**](#variable-e_beta_est)  <br> |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**I\_alpha\_est**](#variable-i_alpha_est)  <br> |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**I\_beta\_est**](#variable-i_beta_est)  <br> |
|  [**SYN\_FOCObserverConfig**](structSYN__FOCObserverConfig.md) | [**cfg**](#variable-cfg)  <br> |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**omega\_e**](#variable-omega_e)  <br> |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**pll\_i\_accum**](#variable-pll_i_accum)  <br> |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**theta\_e**](#variable-theta_e)  <br> |












































## Public Attributes Documentation




### variable E\_alpha\_est 

```C++
q16_t SYN_FOCObserver::E_alpha_est;
```



Estimated alpha-axis back-EMF (Q16.16) 


        

<hr>



### variable E\_beta\_est 

```C++
q16_t SYN_FOCObserver::E_beta_est;
```



Estimated beta-axis back-EMF (Q16.16) 
 


        

<hr>



### variable I\_alpha\_est 

```C++
q16_t SYN_FOCObserver::I_alpha_est;
```



Estimated alpha-axis phase current (Q16.16) 


        

<hr>



### variable I\_beta\_est 

```C++
q16_t SYN_FOCObserver::I_beta_est;
```



Estimated beta-axis phase current (Q16.16) 
 


        

<hr>



### variable cfg 

```C++
SYN_FOCObserverConfig SYN_FOCObserver::cfg;
```



Observer configuration 


        

<hr>



### variable omega\_e 

```C++
q16_t SYN_FOCObserver::omega_e;
```



Estimated rotor electrical speed in rad/s (Q16.16) 
 


        

<hr>



### variable pll\_i\_accum 

```C++
q16_t SYN_FOCObserver::pll_i_accum;
```



Software PLL integral error accumulator 
 


        

<hr>



### variable theta\_e 

```C++
q16_t SYN_FOCObserver::theta_e;
```



Estimated rotor electrical angle in rad (Q16.16, 0 to 2pi) 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/motor/syn_foc_observer.h`

