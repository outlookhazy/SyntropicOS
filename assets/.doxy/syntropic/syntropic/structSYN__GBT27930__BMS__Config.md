

# Struct SYN\_GBT27930\_BMS\_Config



[**ClassList**](annotated.md) **>** [**SYN\_GBT27930\_BMS\_Config**](structSYN__GBT27930__BMS__Config.md)



_BMS Static Parameters (BRM / BCP)._ 

* `#include <syn_gbt27930.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**battery\_type**](#variable-battery_type)  <br> |
|  uint16\_t | [**max\_cell\_volt\_v**](#variable-max_cell_volt_v)  <br> |
|  uint16\_t | [**max\_charge\_curr\_a**](#variable-max_charge_curr_a)  <br> |
|  uint16\_t | [**max\_charge\_volt\_v**](#variable-max_charge_volt_v)  <br> |
|  uint16\_t | [**max\_temp\_c**](#variable-max_temp_c)  <br> |
|  uint16\_t | [**rated\_capacity\_ah**](#variable-rated_capacity_ah)  <br> |
|  uint16\_t | [**rated\_voltage\_v**](#variable-rated_voltage_v)  <br> |
|  uint8\_t | [**vin**](#variable-vin)  <br> |












































## Public Attributes Documentation




### variable battery\_type 

```C++
uint8_t SYN_GBT27930_BMS_Config::battery_type;
```



0x01=Pb-acid, 0x02=NiMH, 0x03=LiFePO4, 0x06=NMC 


        

<hr>



### variable max\_cell\_volt\_v 

```C++
uint16_t SYN_GBT27930_BMS_Config::max_cell_volt_v;
```



Max Permissible Single Cell Voltage (0.01 V/bit) 


        

<hr>



### variable max\_charge\_curr\_a 

```C++
uint16_t SYN_GBT27930_BMS_Config::max_charge_curr_a;
```



Permissible Max Charge Current (0.1 A/bit, offset -400A) 


        

<hr>



### variable max\_charge\_volt\_v 

```C++
uint16_t SYN_GBT27930_BMS_Config::max_charge_volt_v;
```



Permissible Max Charge Voltage (0.1 V/bit) 


        

<hr>



### variable max\_temp\_c 

```C++
uint16_t SYN_GBT27930_BMS_Config::max_temp_c;
```



Permissible Max Temperature (1 °C/bit, offset -50°C) 


        

<hr>



### variable rated\_capacity\_ah 

```C++
uint16_t SYN_GBT27930_BMS_Config::rated_capacity_ah;
```



Rated Capacity (0.1 Ah/bit) 


        

<hr>



### variable rated\_voltage\_v 

```C++
uint16_t SYN_GBT27930_BMS_Config::rated_voltage_v;
```



Rated Total Voltage (0.1 V/bit) 


        

<hr>



### variable vin 

```C++
uint8_t SYN_GBT27930_BMS_Config::vin[17];
```



17-character Vehicle Identification Number 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_gbt27930.h`

