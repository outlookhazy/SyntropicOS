

# File syn\_rfid.c



[**FileList**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_rfid.c**](syn__rfid_8c.md)

[Go to the source code of this file](syn__rfid_8c_source.md)

_Generic RFID & NFC Card Reader Driver (MFRC522 13.56MHz, PN532, RDM6300 125kHz)._ 

* `#include "syn_rfid.h"`
* `#include "../util/syn_assert.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_rfid\_clear\_card**](#function-syn_rfid_clear_card) ([**SYN\_RFID**](structSYN__RFID.md) \* rfid) <br>_Clear card presence state (card removed)._  |
|  void | [**syn\_rfid\_feed\_card**](#function-syn_rfid_feed_card) ([**SYN\_RFID**](structSYN__RFID.md) \* rfid, const uint8\_t \* uid, uint8\_t len) <br>_Feed raw UID scan bytes._  |
|  const uint8\_t \* | [**syn\_rfid\_get\_uid**](#function-syn_rfid_get_uid) (const [**SYN\_RFID**](structSYN__RFID.md) \* rfid, uint8\_t \* len) <br>_Get last scanned UID byte array._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_rfid\_init**](#function-syn_rfid_init) ([**SYN\_RFID**](structSYN__RFID.md) \* rfid, [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) ss\_pin, [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) rst\_pin, [**SYN\_RFIDType**](syn__rfid_8h.md#enum-syn_rfidtype) type) <br>_Initialize RFID Reader context._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_rfid\_is\_card\_present**](#function-syn_rfid_is_card_present) (const [**SYN\_RFID**](structSYN__RFID.md) \* rfid) <br>_Check if a card is present._  |




























## Public Functions Documentation




### function syn\_rfid\_clear\_card 

_Clear card presence state (card removed)._ 
```C++
void syn_rfid_clear_card (
    SYN_RFID * rfid
) 
```





**Parameters:**


* `rfid` RFID context. 




        

<hr>



### function syn\_rfid\_feed\_card 

_Feed raw UID scan bytes._ 
```C++
void syn_rfid_feed_card (
    SYN_RFID * rfid,
    const uint8_t * uid,
    uint8_t len
) 
```





**Parameters:**


* `rfid` RFID context. 
* `uid` UID byte buffer. 
* `len` UID byte count (4, 7, or 10). 




        

<hr>



### function syn\_rfid\_get\_uid 

_Get last scanned UID byte array._ 
```C++
const uint8_t * syn_rfid_get_uid (
    const SYN_RFID * rfid,
    uint8_t * len
) 
```





**Parameters:**


* `rfid` RFID context. 
* `len` Pointer to receive UID length. 



**Returns:**

Pointer to UID bytes (or NULL). 





        

<hr>



### function syn\_rfid\_init 

_Initialize RFID Reader context._ 
```C++
SYN_Status syn_rfid_init (
    SYN_RFID * rfid,
    SYN_GPIO_Pin ss_pin,
    SYN_GPIO_Pin rst_pin,
    SYN_RFIDType type
) 
```





**Parameters:**


* `rfid` RFID context. 
* `ss_pin` SPI SS / I2C SDA pin. 
* `rst_pin` Reset GPIO pin. 
* `type` RFID IC type (MFRC522, PN532, RDM6300). 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_rfid\_is\_card\_present 

_Check if a card is present._ 
```C++
bool syn_rfid_is_card_present (
    const SYN_RFID * rfid
) 
```





**Parameters:**


* `rfid` RFID context. 



**Returns:**

True if card present. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_rfid.c`

