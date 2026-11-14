

# File syn\_powermon.c



[**FileList**](files.md) **>** [**sensor**](dir_3ec96d4fdb788c01e7dd1ddb7db4104c.md) **>** [**syn\_powermon.c**](syn__powermon_8c.md)

[Go to the source code of this file](syn__powermon_8c_source.md)

_Generic Power & Current Monitor Driver (INA219, INA226, INA3221)._ 

* `#include "syn_powermon.h"`
* `#include "../util/syn_assert.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_powermon\_feed\_raw**](#function-syn_powermon_feed_raw) ([**SYN\_PowerMon**](structSYN__PowerMon.md) \* pm, uint16\_t raw\_bus\_v, float raw\_shunt\_mv) <br>_Feed raw I2C register samples (bus voltage & shunt voltage)._  |
|  float | [**syn\_powermon\_get\_bus\_voltage**](#function-syn_powermon_get_bus_voltage) (const [**SYN\_PowerMon**](structSYN__PowerMon.md) \* pm) <br>_Get measured bus voltage in Volts._  |
|  float | [**syn\_powermon\_get\_current\_ma**](#function-syn_powermon_get_current_ma) (const [**SYN\_PowerMon**](structSYN__PowerMon.md) \* pm) <br>_Get measured current in Milliamps._  |
|  float | [**syn\_powermon\_get\_power\_mw**](#function-syn_powermon_get_power_mw) (const [**SYN\_PowerMon**](structSYN__PowerMon.md) \* pm) <br>_Get calculated power in Milliwatts._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_powermon\_init**](#function-syn_powermon_init) ([**SYN\_PowerMon**](structSYN__PowerMon.md) \* pm, [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) scl, [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) sda, uint8\_t i2c\_addr, float shunt\_resistor\_ohms, [**SYN\_PowerMonType**](syn__powermon_8h.md#enum-syn_powermontype) type) <br>_Initialize Power Monitor IC._  |




























## Public Functions Documentation




### function syn\_powermon\_feed\_raw 

_Feed raw I2C register samples (bus voltage & shunt voltage)._ 
```C++
void syn_powermon_feed_raw (
    SYN_PowerMon * pm,
    uint16_t raw_bus_v,
    float raw_shunt_mv
) 
```





**Parameters:**


* `pm` Power monitor context. 
* `raw_bus_v` Raw bus voltage register value. 
* `raw_shunt_mv` Raw shunt voltage in millivolts. 




        

<hr>



### function syn\_powermon\_get\_bus\_voltage 

_Get measured bus voltage in Volts._ 
```C++
float syn_powermon_get_bus_voltage (
    const SYN_PowerMon * pm
) 
```





**Parameters:**


* `pm` Power monitor context. 



**Returns:**

Voltage in Volts. 





        

<hr>



### function syn\_powermon\_get\_current\_ma 

_Get measured current in Milliamps._ 
```C++
float syn_powermon_get_current_ma (
    const SYN_PowerMon * pm
) 
```





**Parameters:**


* `pm` Power monitor context. 



**Returns:**

Current in mA. 





        

<hr>



### function syn\_powermon\_get\_power\_mw 

_Get calculated power in Milliwatts._ 
```C++
float syn_powermon_get_power_mw (
    const SYN_PowerMon * pm
) 
```





**Parameters:**


* `pm` Power monitor context. 



**Returns:**

Power in mW. 





        

<hr>



### function syn\_powermon\_init 

_Initialize Power Monitor IC._ 
```C++
SYN_Status syn_powermon_init (
    SYN_PowerMon * pm,
    SYN_GPIO_Pin scl,
    SYN_GPIO_Pin sda,
    uint8_t i2c_addr,
    float shunt_resistor_ohms,
    SYN_PowerMonType type
) 
```





**Parameters:**


* `pm` Power monitor context. 
* `scl` I2C SCL GPIO pin. 
* `sda` I2C SDA GPIO pin. 
* `i2c_addr` I2C slave address (e.g. 0x40 or 0x41). 
* `shunt_resistor_ohms` Shunt resistor value in Ohms (e.g. 0.1f). 
* `type` IC type (INA219 or INA226). 



**Returns:**

SYN\_OK on success. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/sensor/syn_powermon.c`

