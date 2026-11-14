

# File syn\_pmbus.h



[**FileList**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_pmbus.h**](syn__pmbus_8h.md)

[Go to the source code of this file](syn__pmbus_8h_source.md)

_PMBus (Power Management Bus 1.2 / 1.3) Protocol Engine & Linear Format Converters._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include "syn_smbus.h"`
* `#include <stdbool.h>`
* `#include <stdint.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_pmbus\_encode\_read\_cmd**](#function-syn_pmbus_encode_read_cmd) ([**SYN\_SMBUS\_Packet**](structSYN__SMBUS__Packet.md) \* pkt, uint8\_t slave\_addr, uint8\_t cmd, [**bool**](syn__defs_8h.md#enum-bool) use\_pec) <br>_Encode a PMBus telemetry read request into an SMBus packet._  |
|  uint16\_t | [**syn\_pmbus\_float\_to\_linear11**](#function-syn_pmbus_float_to_linear11) (float val) <br>_Convert floating-point value to PMBus 16-bit Linear11 format._  |
|  uint16\_t | [**syn\_pmbus\_float\_to\_linear16**](#function-syn_pmbus_float_to_linear16) (float val, uint8\_t vout\_mode) <br>_Convert floating-point output voltage to PMBus 16-bit Linear16 format._  |
|  float | [**syn\_pmbus\_linear11\_to\_float**](#function-syn_pmbus_linear11_to_float) (uint16\_t raw) <br>_Convert PMBus 16-bit Linear11 format (5-bit exponent + 11-bit mantissa) to float._  |
|  float | [**syn\_pmbus\_linear16\_to\_float**](#function-syn_pmbus_linear16_to_float) (uint16\_t raw, uint8\_t vout\_mode) <br>_Convert PMBus 16-bit Linear16 format (unsigned 16-bit mantissa + VOUT\_MODE exponent) to float._  |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_PMBUS\_CMD\_CAPABILITY**](syn__pmbus_8h.md#define-syn_pmbus_cmd_capability)  `0x19`<br> |
| define  | [**SYN\_PMBUS\_CMD\_CLEAR\_FAULTS**](syn__pmbus_8h.md#define-syn_pmbus_cmd_clear_faults)  `0x03`<br> |
| define  | [**SYN\_PMBUS\_CMD\_ON\_OFF\_CONFIG**](syn__pmbus_8h.md#define-syn_pmbus_cmd_on_off_config)  `0x02`<br> |
| define  | [**SYN\_PMBUS\_CMD\_OPERATION**](syn__pmbus_8h.md#define-syn_pmbus_cmd_operation)  `0x01`<br> |
| define  | [**SYN\_PMBUS\_CMD\_PAGE**](syn__pmbus_8h.md#define-syn_pmbus_cmd_page)  `0x00`<br> |
| define  | [**SYN\_PMBUS\_CMD\_PHASE**](syn__pmbus_8h.md#define-syn_pmbus_cmd_phase)  `0x04`<br> |
| define  | [**SYN\_PMBUS\_CMD\_PMBUS\_REVISION**](syn__pmbus_8h.md#define-syn_pmbus_cmd_pmbus_revision)  `0x98`<br> |
| define  | [**SYN\_PMBUS\_CMD\_READ\_DUTY\_CYCLE**](syn__pmbus_8h.md#define-syn_pmbus_cmd_read_duty_cycle)  `0x94`<br> |
| define  | [**SYN\_PMBUS\_CMD\_READ\_FAN\_SPEED\_1**](syn__pmbus_8h.md#define-syn_pmbus_cmd_read_fan_speed_1)  `0x90`<br> |
| define  | [**SYN\_PMBUS\_CMD\_READ\_FREQUENCY**](syn__pmbus_8h.md#define-syn_pmbus_cmd_read_frequency)  `0x95`<br> |
| define  | [**SYN\_PMBUS\_CMD\_READ\_IIN**](syn__pmbus_8h.md#define-syn_pmbus_cmd_read_iin)  `0x89`<br> |
| define  | [**SYN\_PMBUS\_CMD\_READ\_IOUT**](syn__pmbus_8h.md#define-syn_pmbus_cmd_read_iout)  `0x8C`<br> |
| define  | [**SYN\_PMBUS\_CMD\_READ\_PIN**](syn__pmbus_8h.md#define-syn_pmbus_cmd_read_pin)  `0x97`<br> |
| define  | [**SYN\_PMBUS\_CMD\_READ\_POUT**](syn__pmbus_8h.md#define-syn_pmbus_cmd_read_pout)  `0x96`<br> |
| define  | [**SYN\_PMBUS\_CMD\_READ\_TEMPERATURE\_1**](syn__pmbus_8h.md#define-syn_pmbus_cmd_read_temperature_1)  `0x8D`<br> |
| define  | [**SYN\_PMBUS\_CMD\_READ\_TEMPERATURE\_2**](syn__pmbus_8h.md#define-syn_pmbus_cmd_read_temperature_2)  `0x8E`<br> |
| define  | [**SYN\_PMBUS\_CMD\_READ\_VIN**](syn__pmbus_8h.md#define-syn_pmbus_cmd_read_vin)  `0x88`<br> |
| define  | [**SYN\_PMBUS\_CMD\_READ\_VOUT**](syn__pmbus_8h.md#define-syn_pmbus_cmd_read_vout)  `0x8B`<br> |
| define  | [**SYN\_PMBUS\_CMD\_RESTORE\_DEFAULT\_ALL**](syn__pmbus_8h.md#define-syn_pmbus_cmd_restore_default_all)  `0x12`<br> |
| define  | [**SYN\_PMBUS\_CMD\_STATUS\_BYTE**](syn__pmbus_8h.md#define-syn_pmbus_cmd_status_byte)  `0x78`<br> |
| define  | [**SYN\_PMBUS\_CMD\_STATUS\_CBUFFER**](syn__pmbus_8h.md#define-syn_pmbus_cmd_status_cbuffer)  `0x7E`<br> |
| define  | [**SYN\_PMBUS\_CMD\_STATUS\_FANS\_1\_2**](syn__pmbus_8h.md#define-syn_pmbus_cmd_status_fans_1_2)  `0x81`<br> |
| define  | [**SYN\_PMBUS\_CMD\_STATUS\_INPUT**](syn__pmbus_8h.md#define-syn_pmbus_cmd_status_input)  `0x7C`<br> |
| define  | [**SYN\_PMBUS\_CMD\_STATUS\_IOUT**](syn__pmbus_8h.md#define-syn_pmbus_cmd_status_iout)  `0x7B`<br> |
| define  | [**SYN\_PMBUS\_CMD\_STATUS\_MFR\_SPECIFIC**](syn__pmbus_8h.md#define-syn_pmbus_cmd_status_mfr_specific)  `0x80`<br> |
| define  | [**SYN\_PMBUS\_CMD\_STATUS\_OTHER**](syn__pmbus_8h.md#define-syn_pmbus_cmd_status_other)  `0x7F`<br> |
| define  | [**SYN\_PMBUS\_CMD\_STATUS\_TEMPERATURE**](syn__pmbus_8h.md#define-syn_pmbus_cmd_status_temperature)  `0x7D`<br> |
| define  | [**SYN\_PMBUS\_CMD\_STATUS\_VOUT**](syn__pmbus_8h.md#define-syn_pmbus_cmd_status_vout)  `0x7A`<br> |
| define  | [**SYN\_PMBUS\_CMD\_STATUS\_WORD**](syn__pmbus_8h.md#define-syn_pmbus_cmd_status_word)  `0x79`<br> |
| define  | [**SYN\_PMBUS\_CMD\_STORE\_DEFAULT\_ALL**](syn__pmbus_8h.md#define-syn_pmbus_cmd_store_default_all)  `0x11`<br> |
| define  | [**SYN\_PMBUS\_CMD\_VOUT\_COMMAND**](syn__pmbus_8h.md#define-syn_pmbus_cmd_vout_command)  `0x21`<br> |
| define  | [**SYN\_PMBUS\_CMD\_VOUT\_DROOP**](syn__pmbus_8h.md#define-syn_pmbus_cmd_vout_droop)  `0x28`<br> |
| define  | [**SYN\_PMBUS\_CMD\_VOUT\_MARGIN\_HIGH**](syn__pmbus_8h.md#define-syn_pmbus_cmd_vout_margin_high)  `0x25`<br> |
| define  | [**SYN\_PMBUS\_CMD\_VOUT\_MARGIN\_LOW**](syn__pmbus_8h.md#define-syn_pmbus_cmd_vout_margin_low)  `0x26`<br> |
| define  | [**SYN\_PMBUS\_CMD\_VOUT\_MAX**](syn__pmbus_8h.md#define-syn_pmbus_cmd_vout_max)  `0x24`<br> |
| define  | [**SYN\_PMBUS\_CMD\_VOUT\_MODE**](syn__pmbus_8h.md#define-syn_pmbus_cmd_vout_mode)  `0x20`<br> |
| define  | [**SYN\_PMBUS\_CMD\_VOUT\_TRANSITION\_RATE**](syn__pmbus_8h.md#define-syn_pmbus_cmd_vout_transition_rate)  `0x27`<br> |
| define  | [**SYN\_PMBUS\_CMD\_WRITE\_PROTECT**](syn__pmbus_8h.md#define-syn_pmbus_cmd_write_protect)  `0x10`<br> |
| define  | [**SYN\_PMBUS\_STATUS\_BYTE\_BUSY**](syn__pmbus_8h.md#define-syn_pmbus_status_byte_busy)  `(1u &lt;&lt; 7)`<br> |
| define  | [**SYN\_PMBUS\_STATUS\_BYTE\_CBUF\_FAULT**](syn__pmbus_8h.md#define-syn_pmbus_status_byte_cbuf_fault)  `(1u &lt;&lt; 1)`<br> |
| define  | [**SYN\_PMBUS\_STATUS\_BYTE\_IOUT\_OC**](syn__pmbus_8h.md#define-syn_pmbus_status_byte_iout_oc)  `(1u &lt;&lt; 4)`<br> |
| define  | [**SYN\_PMBUS\_STATUS\_BYTE\_NONE\_OF\_ABOVE**](syn__pmbus_8h.md#define-syn_pmbus_status_byte_none_of_above)  `(1u &lt;&lt; 0)`<br> |
| define  | [**SYN\_PMBUS\_STATUS\_BYTE\_OFF**](syn__pmbus_8h.md#define-syn_pmbus_status_byte_off)  `(1u &lt;&lt; 6)`<br> |
| define  | [**SYN\_PMBUS\_STATUS\_BYTE\_TEMP\_FAULT**](syn__pmbus_8h.md#define-syn_pmbus_status_byte_temp_fault)  `(1u &lt;&lt; 2)`<br> |
| define  | [**SYN\_PMBUS\_STATUS\_BYTE\_VIN\_UV**](syn__pmbus_8h.md#define-syn_pmbus_status_byte_vin_uv)  `(1u &lt;&lt; 3)`<br> |
| define  | [**SYN\_PMBUS\_STATUS\_BYTE\_VOUT\_OV**](syn__pmbus_8h.md#define-syn_pmbus_status_byte_vout_ov)  `(1u &lt;&lt; 5)`<br> |

## Detailed Description


Provides PMBus standard command definitions, telemetry status decoding, and Linear11 / Linear16 numeric data converters. 


    
## Public Functions Documentation




### function syn\_pmbus\_encode\_read\_cmd 

_Encode a PMBus telemetry read request into an SMBus packet._ 
```C++
void syn_pmbus_encode_read_cmd (
    SYN_SMBUS_Packet * pkt,
    uint8_t slave_addr,
    uint8_t cmd,
    bool use_pec
) 
```





**Parameters:**


* `pkt` Output SMBus packet. 
* `slave_addr` 7-bit PMBus slave address. 
* `cmd` PMBus command code (e.g., SYN\_PMBUS\_CMD\_READ\_VIN). 
* `use_pec` Enable Packet Error Checking (PEC). 




        

<hr>



### function syn\_pmbus\_float\_to\_linear11 

_Convert floating-point value to PMBus 16-bit Linear11 format._ 
```C++
uint16_t syn_pmbus_float_to_linear11 (
    float val
) 
```





**Parameters:**


* `val` Floating-point value. 



**Returns:**

Encoded 16-bit Linear11 word. 





        

<hr>



### function syn\_pmbus\_float\_to\_linear16 

_Convert floating-point output voltage to PMBus 16-bit Linear16 format._ 
```C++
uint16_t syn_pmbus_float_to_linear16 (
    float val,
    uint8_t vout_mode
) 
```





**Parameters:**


* `val` Floating-point voltage value. 
* `vout_mode` VOUT\_MODE byte (or raw 5-bit signed exponent N). 



**Returns:**

Encoded 16-bit Linear16 raw mantissa word. 





        

<hr>



### function syn\_pmbus\_linear11\_to\_float 

_Convert PMBus 16-bit Linear11 format (5-bit exponent + 11-bit mantissa) to float._ 
```C++
float syn_pmbus_linear11_to_float (
    uint16_t raw
) 
```





**Parameters:**


* `raw` 16-bit raw Linear11 word. 



**Returns:**

Decoded floating-point value. 





        

<hr>



### function syn\_pmbus\_linear16\_to\_float 

_Convert PMBus 16-bit Linear16 format (unsigned 16-bit mantissa + VOUT\_MODE exponent) to float._ 
```C++
float syn_pmbus_linear16_to_float (
    uint16_t raw,
    uint8_t vout_mode
) 
```





**Parameters:**


* `raw` 16-bit raw mantissa word. 
* `vout_mode` VOUT\_MODE byte (or raw 5-bit signed exponent N). 



**Returns:**

Decoded floating-point output voltage. 





        

<hr>
## Macro Definition Documentation





### define SYN\_PMBUS\_CMD\_CAPABILITY 

```C++
#define SYN_PMBUS_CMD_CAPABILITY `0x19`
```



Bus speed and alert capabilities 


        

<hr>



### define SYN\_PMBUS\_CMD\_CLEAR\_FAULTS 

```C++
#define SYN_PMBUS_CMD_CLEAR_FAULTS `0x03`
```



Clear fault flags 


        

<hr>



### define SYN\_PMBUS\_CMD\_ON\_OFF\_CONFIG 

```C++
#define SYN_PMBUS_CMD_ON_OFF_CONFIG `0x02`
```



On/off switch configuration 


        

<hr>



### define SYN\_PMBUS\_CMD\_OPERATION 

```C++
#define SYN_PMBUS_CMD_OPERATION `0x01`
```



On/off and margin control 


        

<hr>



### define SYN\_PMBUS\_CMD\_PAGE 

```C++
#define SYN_PMBUS_CMD_PAGE `0x00`
```



Select page/channel 


        

<hr>



### define SYN\_PMBUS\_CMD\_PHASE 

```C++
#define SYN_PMBUS_CMD_PHASE `0x04`
```



Phase selection 


        

<hr>



### define SYN\_PMBUS\_CMD\_PMBUS\_REVISION 

```C++
#define SYN_PMBUS_CMD_PMBUS_REVISION `0x98`
```



PMBus spec version 


        

<hr>



### define SYN\_PMBUS\_CMD\_READ\_DUTY\_CYCLE 

```C++
#define SYN_PMBUS_CMD_READ_DUTY_CYCLE `0x94`
```



Read PWM duty cycle 


        

<hr>



### define SYN\_PMBUS\_CMD\_READ\_FAN\_SPEED\_1 

```C++
#define SYN_PMBUS_CMD_READ_FAN_SPEED_1 `0x90`
```



Read fan speed 1 (RPM) 


        

<hr>



### define SYN\_PMBUS\_CMD\_READ\_FREQUENCY 

```C++
#define SYN_PMBUS_CMD_READ_FREQUENCY `0x95`
```



Read switching frequency 


        

<hr>



### define SYN\_PMBUS\_CMD\_READ\_IIN 

```C++
#define SYN_PMBUS_CMD_READ_IIN `0x89`
```



Read input current 


        

<hr>



### define SYN\_PMBUS\_CMD\_READ\_IOUT 

```C++
#define SYN_PMBUS_CMD_READ_IOUT `0x8C`
```



Read output current 


        

<hr>



### define SYN\_PMBUS\_CMD\_READ\_PIN 

```C++
#define SYN_PMBUS_CMD_READ_PIN `0x97`
```



Read input power 


        

<hr>



### define SYN\_PMBUS\_CMD\_READ\_POUT 

```C++
#define SYN_PMBUS_CMD_READ_POUT `0x96`
```



Read output power 


        

<hr>



### define SYN\_PMBUS\_CMD\_READ\_TEMPERATURE\_1 

```C++
#define SYN_PMBUS_CMD_READ_TEMPERATURE_1 `0x8D`
```



Read primary temperature 


        

<hr>



### define SYN\_PMBUS\_CMD\_READ\_TEMPERATURE\_2 

```C++
#define SYN_PMBUS_CMD_READ_TEMPERATURE_2 `0x8E`
```



Read secondary temperature 


        

<hr>



### define SYN\_PMBUS\_CMD\_READ\_VIN 

```C++
#define SYN_PMBUS_CMD_READ_VIN `0x88`
```



Read input voltage 


        

<hr>



### define SYN\_PMBUS\_CMD\_READ\_VOUT 

```C++
#define SYN_PMBUS_CMD_READ_VOUT `0x8B`
```



Read output voltage 


        

<hr>



### define SYN\_PMBUS\_CMD\_RESTORE\_DEFAULT\_ALL 

```C++
#define SYN_PMBUS_CMD_RESTORE_DEFAULT_ALL `0x12`
```



Restore default settings from NVM 


        

<hr>



### define SYN\_PMBUS\_CMD\_STATUS\_BYTE 

```C++
#define SYN_PMBUS_CMD_STATUS_BYTE `0x78`
```



Summary status byte 


        

<hr>



### define SYN\_PMBUS\_CMD\_STATUS\_CBUFFER 

```C++
#define SYN_PMBUS_CMD_STATUS_CBUFFER `0x7E`
```



Communication/logic status 


        

<hr>



### define SYN\_PMBUS\_CMD\_STATUS\_FANS\_1\_2 

```C++
#define SYN_PMBUS_CMD_STATUS_FANS_1_2 `0x81`
```



Fan 1 & 2 status flags 


        

<hr>



### define SYN\_PMBUS\_CMD\_STATUS\_INPUT 

```C++
#define SYN_PMBUS_CMD_STATUS_INPUT `0x7C`
```



Input status flags 


        

<hr>



### define SYN\_PMBUS\_CMD\_STATUS\_IOUT 

```C++
#define SYN_PMBUS_CMD_STATUS_IOUT `0x7B`
```



Output current status flags 


        

<hr>



### define SYN\_PMBUS\_CMD\_STATUS\_MFR\_SPECIFIC 

```C++
#define SYN_PMBUS_CMD_STATUS_MFR_SPECIFIC `0x80`
```



Manufacturer status flags 


        

<hr>



### define SYN\_PMBUS\_CMD\_STATUS\_OTHER 

```C++
#define SYN_PMBUS_CMD_STATUS_OTHER `0x7F`
```



Other status flags 


        

<hr>



### define SYN\_PMBUS\_CMD\_STATUS\_TEMPERATURE 

```C++
#define SYN_PMBUS_CMD_STATUS_TEMPERATURE `0x7D`
```



Temperature status flags 


        

<hr>



### define SYN\_PMBUS\_CMD\_STATUS\_VOUT 

```C++
#define SYN_PMBUS_CMD_STATUS_VOUT `0x7A`
```



Output voltage status flags 


        

<hr>



### define SYN\_PMBUS\_CMD\_STATUS\_WORD 

```C++
#define SYN_PMBUS_CMD_STATUS_WORD `0x79`
```



Summary status word 


        

<hr>



### define SYN\_PMBUS\_CMD\_STORE\_DEFAULT\_ALL 

```C++
#define SYN_PMBUS_CMD_STORE_DEFAULT_ALL `0x11`
```



Store default settings to NVM 


        

<hr>



### define SYN\_PMBUS\_CMD\_VOUT\_COMMAND 

```C++
#define SYN_PMBUS_CMD_VOUT_COMMAND `0x21`
```



Output voltage setpoint 


        

<hr>



### define SYN\_PMBUS\_CMD\_VOUT\_DROOP 

```C++
#define SYN_PMBUS_CMD_VOUT_DROOP `0x28`
```



Voltage droop rate 


        

<hr>



### define SYN\_PMBUS\_CMD\_VOUT\_MARGIN\_HIGH 

```C++
#define SYN_PMBUS_CMD_VOUT_MARGIN_HIGH `0x25`
```



High margin voltage setting 


        

<hr>



### define SYN\_PMBUS\_CMD\_VOUT\_MARGIN\_LOW 

```C++
#define SYN_PMBUS_CMD_VOUT_MARGIN_LOW `0x26`
```



Low margin voltage setting 


        

<hr>



### define SYN\_PMBUS\_CMD\_VOUT\_MAX 

```C++
#define SYN_PMBUS_CMD_VOUT_MAX `0x24`
```



Maximum output voltage limit 


        

<hr>



### define SYN\_PMBUS\_CMD\_VOUT\_MODE 

```C++
#define SYN_PMBUS_CMD_VOUT_MODE `0x20`
```



Output voltage mode & exponent 


        

<hr>



### define SYN\_PMBUS\_CMD\_VOUT\_TRANSITION\_RATE 

```C++
#define SYN_PMBUS_CMD_VOUT_TRANSITION_RATE `0x27`
```



Voltage slew rate 


        

<hr>



### define SYN\_PMBUS\_CMD\_WRITE\_PROTECT 

```C++
#define SYN_PMBUS_CMD_WRITE_PROTECT `0x10`
```



Write protect configuration 


        

<hr>



### define SYN\_PMBUS\_STATUS\_BYTE\_BUSY 

```C++
#define SYN_PMBUS_STATUS_BYTE_BUSY `(1u << 7)`
```



Bit 7: Device busy 


        

<hr>



### define SYN\_PMBUS\_STATUS\_BYTE\_CBUF\_FAULT 

```C++
#define SYN_PMBUS_STATUS_BYTE_CBUF_FAULT `(1u << 1)`
```



Bit 1: CBUF / Communications fault 


        

<hr>



### define SYN\_PMBUS\_STATUS\_BYTE\_IOUT\_OC 

```C++
#define SYN_PMBUS_STATUS_BYTE_IOUT_OC `(1u << 4)`
```



Bit 4: IOUT Overcurrent fault 


        

<hr>



### define SYN\_PMBUS\_STATUS\_BYTE\_NONE\_OF\_ABOVE 

```C++
#define SYN_PMBUS_STATUS_BYTE_NONE_OF_ABOVE `(1u << 0)`
```



Bit 0: Other fault occurred 


        

<hr>



### define SYN\_PMBUS\_STATUS\_BYTE\_OFF 

```C++
#define SYN_PMBUS_STATUS_BYTE_OFF `(1u << 6)`
```



Bit 6: Output disabled / off 


        

<hr>



### define SYN\_PMBUS\_STATUS\_BYTE\_TEMP\_FAULT 

```C++
#define SYN_PMBUS_STATUS_BYTE_TEMP_FAULT `(1u << 2)`
```



Bit 2: Temperature fault/warning 


        

<hr>



### define SYN\_PMBUS\_STATUS\_BYTE\_VIN\_UV 

```C++
#define SYN_PMBUS_STATUS_BYTE_VIN_UV `(1u << 3)`
```



Bit 3: VIN Undervoltage fault 


        

<hr>



### define SYN\_PMBUS\_STATUS\_BYTE\_VOUT\_OV 

```C++
#define SYN_PMBUS_STATUS_BYTE_VOUT_OV `(1u << 5)`
```



Bit 5: VOUT Overvoltage fault 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_pmbus.h`

