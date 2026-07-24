

# Struct SYN\_AutoTune\_Config



[**ClassList**](annotated.md) **>** [**SYN\_AutoTune\_Config**](structSYN__AutoTune__Config.md)



_Auto-tuner configuration._ 

* `#include <syn_autotune.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_DataLog**](structSYN__DataLog.md) \* | [**datalog**](#variable-datalog)  <br> |
|  [**SYN\_AutoTune\_Flags**](syn__autotune_8h.md#enum-syn_autotune_flags) | [**flags**](#variable-flags)  <br> |
|  uint16\_t | [**gain\_multiplier\_pct**](#variable-gain_multiplier_pct)  <br> |
|  [**SYN\_AutoTune\_Limits**](structSYN__AutoTune__Limits.md) | [**limits**](#variable-limits)  <br> |
|  uint32\_t | [**measure\_ms**](#variable-measure_ms)  <br> |
|  [**SYN\_AutoTune\_Method**](syn__autotune_8h.md#enum-syn_autotune_method) | [**method**](#variable-method)  <br> |
|  [**SYN\_AutoTune\_Mode**](syn__autotune_8h.md#enum-syn_autotune_mode) | [**mode**](#variable-mode)  <br> |
|  uint32\_t | [**ramp\_ms**](#variable-ramp_ms)  <br> |
|  uint8\_t | [**relay\_cycles**](#variable-relay_cycles)  <br> |
|  int32\_t | [**setpoint**](#variable-setpoint)  <br> |
|  uint32\_t | [**settle\_ms**](#variable-settle_ms)  <br> |
|  int32\_t | [**test\_output**](#variable-test_output)  <br> |
|  uint32\_t | [**watchdog\_ms**](#variable-watchdog_ms)  <br> |












































## Public Attributes Documentation




### variable datalog 

```C++
SYN_DataLog* SYN_AutoTune_Config::datalog;
```



If set, capture tuning telemetry 


        

<hr>



### variable flags 

```C++
SYN_AutoTune_Flags SYN_AutoTune_Config::flags;
```



Feature enablement flags 
 


        

<hr>



### variable gain\_multiplier\_pct 

```C++
uint16_t SYN_AutoTune_Config::gain_multiplier_pct;
```



Gain multiplier percentage (1-200). Default: 100. Allows applying a safety margin to calculated PID gains (e.g., 80 for 80%). 


        

<hr>



### variable limits 

```C++
SYN_AutoTune_Limits SYN_AutoTune_Config::limits;
```



Physical constraints 
 


        

<hr>



### variable measure\_ms 

```C++
uint32_t SYN_AutoTune_Config::measure_ms;
```



Duration to average velocity 
 


        

<hr>



### variable method 

```C++
SYN_AutoTune_Method SYN_AutoTune_Config::method;
```



PID gain formula 
 


        

<hr>



### variable mode 

```C++
SYN_AutoTune_Mode SYN_AutoTune_Config::mode;
```



Operating mode 
 


        

<hr>



### variable ramp\_ms 

```C++
uint32_t SYN_AutoTune_Config::ramp_ms;
```



Ramp time — ms to ramp from 0 to test\_output. Default: 500 ms. Prevents jerking heavy loads with a step input. 


        

<hr>



### variable relay\_cycles 

```C++
uint8_t SYN_AutoTune_Config::relay_cycles;
```



Oscillation cycles to measure 


        

<hr>



### variable setpoint 

```C++
int32_t SYN_AutoTune_Config::setpoint;
```



Position to oscillate around 
 


        

<hr>



### variable settle\_ms 

```C++
uint32_t SYN_AutoTune_Config::settle_ms;
```



Time to reach steady-state 
 


        

<hr>



### variable test\_output 

```C++
int32_t SYN_AutoTune_Config::test_output;
```



Motor output during test (% of output\_max). If 0, the tuner will probe for the minimum motion output. 


        

<hr>



### variable watchdog\_ms 

```C++
uint32_t SYN_AutoTune_Config::watchdog_ms;
```



Abort if update() not called within this time (ms). Protects against application crashes. Default: 500 ms. 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/control/syn_autotune.h`

