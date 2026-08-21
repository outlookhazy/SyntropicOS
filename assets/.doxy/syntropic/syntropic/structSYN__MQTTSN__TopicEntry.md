

# Struct SYN\_MQTTSN\_TopicEntry



[**ClassList**](annotated.md) **>** [**SYN\_MQTTSN\_TopicEntry**](structSYN__MQTTSN__TopicEntry.md)



_Registered topic entry mapping string name to 16-bit Topic ID._ 

* `#include <syn_mqttsn.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**active**](#variable-active)  <br> |
|  char | [**name**](#variable-name)  <br> |
|  uint16\_t | [**topic\_id**](#variable-topic_id)  <br> |












































## Public Attributes Documentation




### variable active 

```C++
bool SYN_MQTTSN_TopicEntry::active;
```



Entry active flag 


        

<hr>



### variable name 

```C++
char SYN_MQTTSN_TopicEntry::name[SYN_MQTTSN_MAX_TOPIC_NAME];
```



Topic name string 


        

<hr>



### variable topic\_id 

```C++
uint16_t SYN_MQTTSN_TopicEntry::topic_id;
```



Gateway-assigned Topic ID 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_mqttsn.h`

