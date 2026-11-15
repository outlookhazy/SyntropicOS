

# Struct SYN\_BLDC\_PhaseOutputs



[**ClassList**](annotated.md) **>** [**SYN\_BLDC\_PhaseOutputs**](structSYN__BLDC__PhaseOutputs.md)



[More...](#detailed-description)

* `#include <syn_bldc_6step.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint16\_t | [**duty**](#variable-duty)  <br> |
|  [**SYN\_BLDC\_GateState**](syn__bldc__6step_8h.md#enum-syn_bldc_gatestate) | [**u**](#variable-u)  <br> |
|  [**SYN\_BLDC\_GateState**](syn__bldc__6step_8h.md#enum-syn_bldc_gatestate) | [**v**](#variable-v)  <br> |
|  [**SYN\_BLDC\_GateState**](syn__bldc__6step_8h.md#enum-syn_bldc_gatestate) | [**w**](#variable-w)  <br> |












































## Detailed Description


3-Phase Gate Output Command. 


    
## Public Attributes Documentation




### variable duty 

```C++
uint16_t SYN_BLDC_PhaseOutputs::duty;
```



Active PWM duty cycle (0..1000). 


        

<hr>



### variable u 

```C++
SYN_BLDC_GateState SYN_BLDC_PhaseOutputs::u;
```



Phase U gate state. 


        

<hr>



### variable v 

```C++
SYN_BLDC_GateState SYN_BLDC_PhaseOutputs::v;
```



Phase V gate state. 


        

<hr>



### variable w 

```C++
SYN_BLDC_GateState SYN_BLDC_PhaseOutputs::w;
```



Phase W gate state. 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/motor/syn_bldc_6step.h`

