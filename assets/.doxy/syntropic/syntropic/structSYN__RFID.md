

# Struct SYN\_RFID



[**ClassList**](annotated.md) **>** [**SYN\_RFID**](structSYN__RFID.md)



_Generic RFID Instance Context._ 

* `#include <syn_rfid.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**card\_present**](#variable-card_present)  <br> |
|  [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) | [**rst\_pin**](#variable-rst_pin)  <br> |
|  [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) | [**ss\_pin**](#variable-ss_pin)  <br> |
|  [**SYN\_RFIDType**](syn__rfid_8h.md#enum-syn_rfidtype) | [**type**](#variable-type)  <br> |
|  uint8\_t | [**uid**](#variable-uid)  <br> |
|  uint8\_t | [**uid\_len**](#variable-uid_len)  <br> |












































## Public Attributes Documentation




### variable card\_present 

```C++
bool SYN_RFID::card_present;
```



True if card detected near antenna 


        

<hr>



### variable rst\_pin 

```C++
SYN_GPIO_Pin SYN_RFID::rst_pin;
```



Hardware reset GPIO pin 


        

<hr>



### variable ss\_pin 

```C++
SYN_GPIO_Pin SYN_RFID::ss_pin;
```



SPI Slave Select / SDA GPIO pin 


        

<hr>



### variable type 

```C++
SYN_RFIDType SYN_RFID::type;
```



RFID reader model type 


        

<hr>



### variable uid 

```C++
uint8_t SYN_RFID::uid[10];
```



Extracted card UID byte array 


        

<hr>



### variable uid\_len 

```C++
uint8_t SYN_RFID::uid_len;
```



UID length (4, 7, or 10 bytes) 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_rfid.h`

