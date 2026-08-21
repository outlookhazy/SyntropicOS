

# Struct SYN\_ModbusMasterQueue



[**ClassList**](annotated.md) **>** [**SYN\_ModbusMasterQueue**](structSYN__ModbusMasterQueue.md)



_Modbus Master transaction queue manager._ 

* `#include <syn_modbus_master.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**count**](#variable-count)  <br> |
|  uint8\_t | [**head**](#variable-head)  <br> |
|  uint8\_t | [**max\_retries**](#variable-max_retries)  <br> |
|  [**SYN\_ModbusMasterQuery**](structSYN__ModbusMasterQuery.md) | [**queries**](#variable-queries)  <br> |
|  uint8\_t | [**retry\_count**](#variable-retry_count)  <br> |
|  uint8\_t | [**tail**](#variable-tail)  <br> |












































## Public Attributes Documentation




### variable count 

```C++
uint8_t SYN_ModbusMasterQueue::count;
```



Number of queued queries. 


        

<hr>



### variable head 

```C++
uint8_t SYN_ModbusMasterQueue::head;
```



Queue head index. 


        

<hr>



### variable max\_retries 

```C++
uint8_t SYN_ModbusMasterQueue::max_retries;
```



Maximum retries per query. 


        

<hr>



### variable queries 

```C++
SYN_ModbusMasterQuery SYN_ModbusMasterQueue::queries[SYN_MODBUS_QUEUE_SIZE];
```



Fixed-size query buffer. 


        

<hr>



### variable retry\_count 

```C++
uint8_t SYN_ModbusMasterQueue::retry_count;
```



Current retry attempt counter. 


        

<hr>



### variable tail 

```C++
uint8_t SYN_ModbusMasterQueue::tail;
```



Queue tail index. 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_modbus_master.h`

