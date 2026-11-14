

# Struct SYN\_OCPP\_DisplayMessage



[**ClassList**](annotated.md) **>** [**SYN\_OCPP\_DisplayMessage**](structSYN__OCPP__DisplayMessage.md)



_OCPP 2.1 Customer Display Message Structure._ 

* `#include <syn_ocpp.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  const char \* | [**content**](#variable-content)  <br> |
|  uint32\_t | [**display\_time\_s**](#variable-display_time_s)  <br> |
|  const char \* | [**header**](#variable-header)  <br> |
|  uint32\_t | [**message\_id**](#variable-message_id)  <br> |












































## Public Attributes Documentation




### variable content 

```C++
const char* SYN_OCPP_DisplayMessage::content;
```



Display text content 


        

<hr>



### variable display\_time\_s 

```C++
uint32_t SYN_OCPP_DisplayMessage::display_time_s;
```



Display duration in seconds 


        

<hr>



### variable header 

```C++
const char* SYN_OCPP_DisplayMessage::header;
```



Optional header string 


        

<hr>



### variable message\_id 

```C++
uint32_t SYN_OCPP_DisplayMessage::message_id;
```



Unique display message ID 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_ocpp.h`

