

# Struct SYN\_PubSubBroker



[**ClassList**](annotated.md) **>** [**SYN\_PubSubBroker**](structSYN__PubSubBroker.md)



_PubSub broker instance — subscription array + count._ 

* `#include <syn_pubsub.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  size\_t | [**capacity**](#variable-capacity)  <br> |
|  size\_t | [**count**](#variable-count)  <br> |
|  [**SYN\_PubSubSub**](structSYN__PubSubSub.md) \* | [**subs**](#variable-subs)  <br> |












































## Public Attributes Documentation




### variable capacity 

```C++
size_t SYN_PubSubBroker::capacity;
```



Maximum subscriptions 
 


        

<hr>



### variable count 

```C++
size_t SYN_PubSubBroker::count;
```



Active subscription count 
 


        

<hr>



### variable subs 

```C++
SYN_PubSubSub* SYN_PubSubBroker::subs;
```



Subscription array 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/util/syn_pubsub.h`

