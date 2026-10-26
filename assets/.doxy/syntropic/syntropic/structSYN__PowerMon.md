

# Struct SYN\_PowerMon



[**ClassList**](annotated.md) **>** [**SYN\_PowerMon**](structSYN__PowerMon.md)



_Generic Power Monitor Instance Context._ 

* `#include <syn_powermon.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  float | [**bus\_voltage\_v**](#variable-bus_voltage_v)  <br> |
|  float | [**energy\_mwh**](#variable-energy_mwh)  <br> |
|  [**SYN\_SoftI2C**](structSYN__SoftI2C.md) | [**i2c**](#variable-i2c)  <br> |
|  uint8\_t | [**i2c\_addr**](#variable-i2c_addr)  <br> |
|  float | [**power\_mw**](#variable-power_mw)  <br> |
|  float | [**shunt\_current\_ma**](#variable-shunt_current_ma)  <br> |
|  float | [**shunt\_resistor\_ohms**](#variable-shunt_resistor_ohms)  <br> |
|  [**SYN\_PowerMonType**](syn__powermon_8h.md#enum-syn_powermontype) | [**type**](#variable-type)  <br> |












































## Public Attributes Documentation




### variable bus\_voltage\_v 

```C++
float SYN_PowerMon::bus_voltage_v;
```



Measured Bus Voltage in Volts 


        

<hr>



### variable energy\_mwh 

```C++
float SYN_PowerMon::energy_mwh;
```



Accumulated Energy in Milliwatt-hours 


        

<hr>



### variable i2c 

```C++
SYN_SoftI2C SYN_PowerMon::i2c;
```



Software I2C bus context 


        

<hr>



### variable i2c\_addr 

```C++
uint8_t SYN_PowerMon::i2c_addr;
```



7-bit I2C device address 


        

<hr>



### variable power\_mw 

```C++
float SYN_PowerMon::power_mw;
```



Measured Active Power in Milliwatts 


        

<hr>



### variable shunt\_current\_ma 

```C++
float SYN_PowerMon::shunt_current_ma;
```



Measured Shunt Current in Milliamps 


        

<hr>



### variable shunt\_resistor\_ohms 

```C++
float SYN_PowerMon::shunt_resistor_ohms;
```



Shunt resistor value (e.g. 0.1 ohms) 


        

<hr>



### variable type 

```C++
SYN_PowerMonType SYN_PowerMon::type;
```



Power monitor sensor model type 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/sensor/syn_powermon.h`

