

# Struct SYN\_GBT27930\_Telemetry



[**ClassList**](annotated.md) **>** [**SYN\_GBT27930\_Telemetry**](structSYN__GBT27930__Telemetry.md)



_Real-time Charging Telemetry (BCL / BCS / CCS / BSM)._ 

* `#include <syn_gbt27930.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**charge\_mode**](#variable-charge_mode)  <br> |
|  uint16\_t | [**curr\_demand\_a**](#variable-curr_demand_a)  <br> |
|  uint8\_t | [**max\_cell\_temp\_c**](#variable-max_cell_temp_c)  <br> |
|  uint16\_t | [**max\_cell\_volt\_v**](#variable-max_cell_volt_v)  <br> |
|  uint16\_t | [**measured\_curr\_a**](#variable-measured_curr_a)  <br> |
|  uint16\_t | [**measured\_volt\_v**](#variable-measured_volt_v)  <br> |
|  uint8\_t | [**min\_cell\_temp\_c**](#variable-min_cell_temp_c)  <br> |
|  uint16\_t | [**remaining\_time\_min**](#variable-remaining_time_min)  <br> |
|  uint8\_t | [**soc\_percent**](#variable-soc_percent)  <br> |
|  uint16\_t | [**volt\_demand\_v**](#variable-volt_demand_v)  <br> |












































## Public Attributes Documentation




### variable charge\_mode 

```C++
uint8_t SYN_GBT27930_Telemetry::charge_mode;
```



0x01=Constant Voltage, 0x02=Constant Current 


        

<hr>



### variable curr\_demand\_a 

```C++
uint16_t SYN_GBT27930_Telemetry::curr_demand_a;
```



Current Demand (0.1 A/bit, offset -400A) 


        

<hr>



### variable max\_cell\_temp\_c 

```C++
uint8_t SYN_GBT27930_Telemetry::max_cell_temp_c;
```



Measured Max Cell Temp (1 °C/bit, offset -50°C) 


        

<hr>



### variable max\_cell\_volt\_v 

```C++
uint16_t SYN_GBT27930_Telemetry::max_cell_volt_v;
```



Measured Max Cell Voltage (0.01 V/bit) 


        

<hr>



### variable measured\_curr\_a 

```C++
uint16_t SYN_GBT27930_Telemetry::measured_curr_a;
```



Measured Output Current (0.1 A/bit, offset -400A) 


        

<hr>



### variable measured\_volt\_v 

```C++
uint16_t SYN_GBT27930_Telemetry::measured_volt_v;
```



Measured Output Voltage (0.1 V/bit) 


        

<hr>



### variable min\_cell\_temp\_c 

```C++
uint8_t SYN_GBT27930_Telemetry::min_cell_temp_c;
```



Measured Min Cell Temp (1 °C/bit, offset -50°C) 


        

<hr>



### variable remaining\_time\_min 

```C++
uint16_t SYN_GBT27930_Telemetry::remaining_time_min;
```



Remaining charging time in minutes 


        

<hr>



### variable soc\_percent 

```C++
uint8_t SYN_GBT27930_Telemetry::soc_percent;
```



Battery SOC percentage (0..100 %) 


        

<hr>



### variable volt\_demand\_v 

```C++
uint16_t SYN_GBT27930_Telemetry::volt_demand_v;
```



Voltage Demand (0.1 V/bit) 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_gbt27930.h`

