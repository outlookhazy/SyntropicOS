

# Struct SYN\_PubSubSub



[**ClassList**](annotated.md) **>** [**SYN\_PubSubSub**](structSYN__PubSubSub.md)



_A single subscription record._ [More...](#detailed-description)

* `#include <syn_pubsub.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  void \* | [**ctx**](#variable-ctx)  <br> |
|  [**SYN\_PubSubHandler**](syn__pubsub_8h.md#typedef-syn_pubsubhandler) | [**handler**](#variable-handler)  <br> |
|  uint16\_t | [**topic**](#variable-topic)  <br> |












































## Detailed Description




**Note:**

Treated as opaque by the user. 





    
## Public Attributes Documentation




### variable ctx 

```C++
void* SYN_PubSubSub::ctx;
```



User context 
 


        

<hr>



### variable handler 

```C++
SYN_PubSubHandler SYN_PubSubSub::handler;
```



Callback function 
 


        

<hr>



### variable topic 

```C++
uint16_t SYN_PubSubSub::topic;
```



Topic this subscription listens for 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/util/syn_pubsub.h`

