

# File syn\_pubsub.h



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_pubsub.h**](syn__pubsub_8h.md)

[Go to the source code of this file](syn__pubsub_8h_source.md)

_Synchronous publish/subscribe event broker._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include <stdint.h>`
* `#include <stddef.h>`
* `#include <stdbool.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_PubSubBroker**](structSYN__PubSubBroker.md) <br>_PubSub broker instance — subscription array + count._  |
| struct | [**SYN\_PubSubSub**](structSYN__PubSubSub.md) <br>_A single subscription record._  |


## Public Types

| Type | Name |
| ---: | :--- |
| typedef void(\* | [**SYN\_PubSubHandler**](#typedef-syn_pubsubhandler)  <br>_Event callback function signature._  |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_pubsub\_init**](#function-syn_pubsub_init) ([**SYN\_PubSubBroker**](structSYN__PubSubBroker.md) \* broker, [**SYN\_PubSubSub**](structSYN__PubSubSub.md) \* sub\_array, size\_t capacity) <br>_Initialize a pubsub broker._  |
|  void | [**syn\_pubsub\_publish**](#function-syn_pubsub_publish) ([**SYN\_PubSubBroker**](structSYN__PubSubBroker.md) \* broker, uint16\_t topic, const void \* payload, size\_t len) <br>_Publish an event to all subscribers of a topic._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_pubsub\_subscribe**](#function-syn_pubsub_subscribe) ([**SYN\_PubSubBroker**](structSYN__PubSubBroker.md) \* broker, uint16\_t topic, [**SYN\_PubSubHandler**](syn__pubsub_8h.md#typedef-syn_pubsubhandler) handler, void \* ctx) <br>_Subscribe to a topic._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_pubsub\_unsubscribe**](#function-syn_pubsub_unsubscribe) ([**SYN\_PubSubBroker**](structSYN__PubSubBroker.md) \* broker, uint16\_t topic, [**SYN\_PubSubHandler**](syn__pubsub_8h.md#typedef-syn_pubsubhandler) handler) <br>_Unsubscribe from a topic._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  size\_t | [**syn\_pubsub\_count**](#function-syn_pubsub_count) (const [**SYN\_PubSubBroker**](structSYN__PubSubBroker.md) \* broker) <br>_Get current number of active subscriptions._  |
|  void | [**syn\_pubsub\_reset**](#function-syn_pubsub_reset) ([**SYN\_PubSubBroker**](structSYN__PubSubBroker.md) \* broker) <br>_Reset the broker, removing all subscriptions._  |

























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_PUBSUB\_TOPIC\_ALL**](syn__pubsub_8h.md#define-syn_pubsub_topic_all)  `0xFFFF`<br> |

## Detailed Description


Provides a decoupled event system. Modules can subscribe to specific topics and receive callbacks when events are published.


Uses a static caller-provided array for storing subscriptions (zero allocation). 


    
## Public Types Documentation




### typedef SYN\_PubSubHandler 

_Event callback function signature._ 
```C++
typedef void(* SYN_PubSubHandler) (uint16_t topic, const void *payload, size_t len, void *ctx);
```





**Parameters:**


* `topic` The topic ID that triggered this callback. 
* `payload` Pointer to the event data (can be NULL). 
* `len` Size of the event data in bytes (can be 0). 
* `ctx` User context pointer provided at subscription. 




        

<hr>
## Public Functions Documentation




### function syn\_pubsub\_init 

_Initialize a pubsub broker._ 
```C++
void syn_pubsub_init (
    SYN_PubSubBroker * broker,
    SYN_PubSubSub * sub_array,
    size_t capacity
) 
```





**Parameters:**


* `broker` Pointer to broker instance. 
* `sub_array` Pointer to an array of [**SYN\_PubSubSub**](structSYN__PubSubSub.md) structures. 
* `capacity` Number of elements in sub\_array. 




        

<hr>



### function syn\_pubsub\_publish 

_Publish an event to all subscribers of a topic._ 
```C++
void syn_pubsub_publish (
    SYN_PubSubBroker * broker,
    uint16_t topic,
    const void * payload,
    size_t len
) 
```





**Parameters:**


* `broker` Pointer to broker instance. 
* `topic` Topic ID of the event. 
* `payload` Pointer to event data. 
* `len` Size of event data in bytes. 




        

<hr>



### function syn\_pubsub\_subscribe 

_Subscribe to a topic._ 
```C++
bool syn_pubsub_subscribe (
    SYN_PubSubBroker * broker,
    uint16_t topic,
    SYN_PubSubHandler handler,
    void * ctx
) 
```





**Parameters:**


* `broker` Pointer to broker instance. 
* `topic` Topic ID to listen for, or SYN\_PUBSUB\_TOPIC\_ALL. 
* `handler` Callback function to invoke. 
* `ctx` Optional user context pointer. 



**Returns:**

true if subscribed successfully, false if broker is full. 





        

<hr>



### function syn\_pubsub\_unsubscribe 

_Unsubscribe from a topic._ 
```C++
bool syn_pubsub_unsubscribe (
    SYN_PubSubBroker * broker,
    uint16_t topic,
    SYN_PubSubHandler handler
) 
```





**Parameters:**


* `broker` Pointer to broker instance. 
* `topic` Topic ID to unsubscribe from. 
* `handler` The specific callback function to remove. 



**Returns:**

true if removed, false if not found. 





        

<hr>
## Public Static Functions Documentation




### function syn\_pubsub\_count 

_Get current number of active subscriptions._ 
```C++
static inline size_t syn_pubsub_count (
    const SYN_PubSubBroker * broker
) 
```





**Parameters:**


* `broker` Broker. 



**Returns:**

Subscription count. 





        

<hr>



### function syn\_pubsub\_reset 

_Reset the broker, removing all subscriptions._ 
```C++
static inline void syn_pubsub_reset (
    SYN_PubSubBroker * broker
) 
```





**Parameters:**


* `broker` Broker. 




        

<hr>
## Macro Definition Documentation





### define SYN\_PUBSUB\_TOPIC\_ALL 

```C++
#define SYN_PUBSUB_TOPIC_ALL `0xFFFF`
```



Special topic ID used to subscribe to ALL events. 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/util/syn_pubsub.h`

