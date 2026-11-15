

# Struct SYN\_IOLink\_DeviceConfig



[**ClassList**](annotated.md) **>** [**SYN\_IOLink\_DeviceConfig**](structSYN__IOLink__DeviceConfig.md)



_Device configuration descriptor._ 

* `#include <syn_iolink.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_IOLink\_ISDU\_ReadCb**](syn__iolink_8h.md#typedef-syn_iolink_isdu_readcb) | [**on\_read**](#variable-on_read)  <br> |
|  [**SYN\_IOLink\_ISDU\_WriteCb**](syn__iolink_8h.md#typedef-syn_iolink_isdu_writecb) | [**on\_write**](#variable-on_write)  <br> |
|  [**SYN\_IOLink\_DirectParams**](structSYN__IOLink__DirectParams.md) | [**params**](#variable-params)  <br> |
|  void \* | [**user\_data**](#variable-user_data)  <br> |












































## Public Attributes Documentation




### variable on\_read 

```C++
SYN_IOLink_ISDU_ReadCb SYN_IOLink_DeviceConfig::on_read;
```



Dynamic ISDU read callback 


        

<hr>



### variable on\_write 

```C++
SYN_IOLink_ISDU_WriteCb SYN_IOLink_DeviceConfig::on_write;
```



Dynamic ISDU write callback 


        

<hr>



### variable params 

```C++
SYN_IOLink_DirectParams SYN_IOLink_DeviceConfig::params;
```



Device parameter page configuration 


        

<hr>



### variable user\_data 

```C++
void* SYN_IOLink_DeviceConfig::user_data;
```



User context pointer 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_iolink.h`

