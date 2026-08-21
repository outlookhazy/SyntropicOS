

# Struct SYN\_OTA\_Manager



[**ClassList**](annotated.md) **>** [**SYN\_OTA\_Manager**](structSYN__OTA__Manager.md)



_Zero-heap Streaming OTA Manager context._ 

* `#include <syn_ota.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_FwBootManager**](structSYN__FwBootManager.md) \* | [**boot\_mgr**](#variable-boot_mgr)  <br> |
|  [**SYN\_OTA\_CryptoMode**](syn__ota_8h.md#enum-syn_ota_cryptomode) | [**crypto\_mode**](#variable-crypto_mode)  <br> |
|  uint32\_t | [**expected\_crc**](#variable-expected_crc)  <br> |
|  uint32\_t | [**expected\_total\_sz**](#variable-expected_total_sz)  <br> |
|  [**SYN\_FwUpdate**](structSYN__FwUpdate.md) | [**fw\_upd**](#variable-fw_upd)  <br> |
|  [**SYN\_OTA\_ErrorCode**](syn__ota_8h.md#enum-syn_ota_errorcode) | [**last\_error**](#variable-last_error)  <br> |
|  void \* | [**lwm2m\_fw\_ctx**](#variable-lwm2m_fw_ctx)  <br> |
|  uint8\_t \* | [**page\_buf**](#variable-page_buf)  <br> |
|  size\_t | [**page\_buf\_sz**](#variable-page_buf_sz)  <br> |
|  uint32\_t | [**slot\_size**](#variable-slot_size)  <br> |
|  [**SYN\_OTA\_State**](syn__ota_8h.md#enum-syn_ota_state) | [**state**](#variable-state)  <br> |
|  uint8\_t | [**target\_slot**](#variable-target_slot)  <br> |
|  uint32\_t | [**target\_slot\_addr**](#variable-target_slot_addr)  <br> |
|  uint32\_t | [**target\_version**](#variable-target_version)  <br> |












































## Public Attributes Documentation




### variable boot\_mgr 

```C++
SYN_FwBootManager* SYN_OTA_Manager::boot_mgr;
```



Pointer to dual-bank boot manager 
 


        

<hr>



### variable crypto\_mode 

```C++
SYN_OTA_CryptoMode SYN_OTA_Manager::crypto_mode;
```



Selected cryptographic mode 
 


        

<hr>



### variable expected\_crc 

```C++
uint32_t SYN_OTA_Manager::expected_crc;
```



Expected CRC-32 of image data 
 


        

<hr>



### variable expected\_total\_sz 

```C++
uint32_t SYN_OTA_Manager::expected_total_sz;
```



Declared total firmware size (bytes) 


        

<hr>



### variable fw\_upd 

```C++
SYN_FwUpdate SYN_OTA_Manager::fw_upd;
```



Underlying streaming flash updater 
 


        

<hr>



### variable last\_error 

```C++
SYN_OTA_ErrorCode SYN_OTA_Manager::last_error;
```



Last recorded error code 
 


        

<hr>



### variable lwm2m\_fw\_ctx 

```C++
void* SYN_OTA_Manager::lwm2m_fw_ctx;
```



Optional SYN\_LwM2M\_FirmwareContext\* 


        

<hr>



### variable page\_buf 

```C++
uint8_t* SYN_OTA_Manager::page_buf;
```



Flash write page buffer 
 


        

<hr>



### variable page\_buf\_sz 

```C++
size_t SYN_OTA_Manager::page_buf_sz;
```



Page buffer size 
 


        

<hr>



### variable slot\_size 

```C++
uint32_t SYN_OTA_Manager::slot_size;
```



Maximum slot size in bytes 
 


        

<hr>



### variable state 

```C++
SYN_OTA_State SYN_OTA_Manager::state;
```



Current OTA lifecycle state 
 


        

<hr>



### variable target\_slot 

```C++
uint8_t SYN_OTA_Manager::target_slot;
```



Chosen slot (SLOT\_A, SLOT\_B, or AUTO) 


        

<hr>



### variable target\_slot\_addr 

```C++
uint32_t SYN_OTA_Manager::target_slot_addr;
```



Target slot base flash address 
 


        

<hr>



### variable target\_version 

```C++
uint32_t SYN_OTA_Manager::target_version;
```



Declared new version code 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/system/syn_ota.h`

