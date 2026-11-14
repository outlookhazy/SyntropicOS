

# Struct SYN\_EcatSlaveInfo



[**ClassList**](annotated.md) **>** [**SYN\_EcatSlaveInfo**](structSYN__EcatSlaveInfo.md)



_EtherCAT Slave Information Entry._ 

* `#include <syn_ethercat.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint16\_t | [**al\_status\_code**](#variable-al_status_code)  <br> |
|  uint16\_t | [**auto\_inc\_addr**](#variable-auto_inc_addr)  <br> |
|  uint8\_t | [**fmmu\_count**](#variable-fmmu_count)  <br> |
|  uint32\_t | [**product\_code**](#variable-product_code)  <br> |
|  uint32\_t | [**revision**](#variable-revision)  <br> |
|  uint8\_t | [**sm\_count**](#variable-sm_count)  <br> |
|  [**SYN\_EcatState**](syn__ethercat_8h.md#enum-syn_ecatstate) | [**state**](#variable-state)  <br> |
|  uint16\_t | [**station\_addr**](#variable-station_addr)  <br> |
|  uint32\_t | [**vendor\_id**](#variable-vendor_id)  <br> |












































## Public Attributes Documentation




### variable al\_status\_code 

```C++
uint16_t SYN_EcatSlaveInfo::al_status_code;
```



Last AL status error code 


        

<hr>



### variable auto\_inc\_addr 

```C++
uint16_t SYN_EcatSlaveInfo::auto_inc_addr;
```



Auto-increment ring position 


        

<hr>



### variable fmmu\_count 

```C++
uint8_t SYN_EcatSlaveInfo::fmmu_count;
```



Configured FMMU count 


        

<hr>



### variable product\_code 

```C++
uint32_t SYN_EcatSlaveInfo::product_code;
```



Product code read from SII EEPROM 


        

<hr>



### variable revision 

```C++
uint32_t SYN_EcatSlaveInfo::revision;
```



Revision number read from SII EEPROM 


        

<hr>



### variable sm\_count 

```C++
uint8_t SYN_EcatSlaveInfo::sm_count;
```



Configured SyncManager count 


        

<hr>



### variable state 

```C++
SYN_EcatState SYN_EcatSlaveInfo::state;
```



Current AL State 


        

<hr>



### variable station\_addr 

```C++
uint16_t SYN_EcatSlaveInfo::station_addr;
```



Assigned configured station address 


        

<hr>



### variable vendor\_id 

```C++
uint32_t SYN_EcatSlaveInfo::vendor_id;
```



Vendor ID read from SII EEPROM 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_ethercat.h`

