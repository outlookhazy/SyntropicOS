

# Struct SYN\_CANOpenNode



[**ClassList**](annotated.md) **>** [**SYN\_CANOpenNode**](structSYN__CANOpenNode.md)



_CANopen Node State Handle._ 

* `#include <syn_canopen.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_CANOpenNodeConfig**](structSYN__CANOpenNodeConfig.md) | [**cfg**](#variable-cfg)  <br> |
|  uint16\_t | [**heartbeat\_timer\_ms**](#variable-heartbeat_timer_ms)  <br> |
|  uint8\_t | [**nmt\_state**](#variable-nmt_state)  <br> |
|  uint8\_t | [**node\_id**](#variable-node_id)  <br> |
|  size\_t | [**od\_count**](#variable-od_count)  <br> |
|  const [**SYN\_CANOpenODEntry**](structSYN__CANOpenODEntry.md) \* | [**od\_table**](#variable-od_table)  <br> |
|  [**SYN\_CANOpenSDOSession**](structSYN__CANOpenSDOSession.md) | [**sdo\_session**](#variable-sdo_session)  <br> |
|  uint32\_t | [**tx\_cob\_id**](#variable-tx_cob_id)  <br> |
|  uint8\_t | [**tx\_count**](#variable-tx_count)  <br> |
|  uint8\_t | [**tx\_data**](#variable-tx_data)  <br> |
|  uint8\_t | [**tx\_head**](#variable-tx_head)  <br> |
|  uint8\_t | [**tx\_len**](#variable-tx_len)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**tx\_pending**](#variable-tx_pending)  <br> |
|  [**SYN\_CANOpenTxSlot**](structSYN__CANOpenTxSlot.md) | [**tx\_queue**](#variable-tx_queue)  <br> |
|  uint8\_t | [**tx\_tail**](#variable-tx_tail)  <br> |












































## Public Attributes Documentation




### variable cfg 

```C++
SYN_CANOpenNodeConfig SYN_CANOpenNode::cfg;
```



Node configuration copy 


        

<hr>



### variable heartbeat\_timer\_ms 

```C++
uint16_t SYN_CANOpenNode::heartbeat_timer_ms;
```



Heartbeat timer counter 


        

<hr>



### variable nmt\_state 

```C++
uint8_t SYN_CANOpenNode::nmt_state;
```



Current NMT state 


        

<hr>



### variable node\_id 

```C++
uint8_t SYN_CANOpenNode::node_id;
```



Node-ID 


        

<hr>



### variable od\_count 

```C++
size_t SYN_CANOpenNode::od_count;
```



Number of OD entries 


        

<hr>



### variable od\_table 

```C++
const SYN_CANOpenODEntry* SYN_CANOpenNode::od_table;
```



Pointer to Object Dictionary 


        

<hr>



### variable sdo\_session 

```C++
SYN_CANOpenSDOSession SYN_CANOpenNode::sdo_session;
```



Active SDO session 


        

<hr>



### variable tx\_cob\_id 

```C++
uint32_t SYN_CANOpenNode::tx_cob_id;
```



Front transmit COB-ID 


        

<hr>



### variable tx\_count 

```C++
uint8_t SYN_CANOpenNode::tx_count;
```



Queued TX frame count 


        

<hr>



### variable tx\_data 

```C++
uint8_t SYN_CANOpenNode::tx_data[8];
```



Front transmit payload 


        

<hr>



### variable tx\_head 

```C++
uint8_t SYN_CANOpenNode::tx_head;
```



TX FIFO read index 


        

<hr>



### variable tx\_len 

```C++
uint8_t SYN_CANOpenNode::tx_len;
```



Front transmit length 


        

<hr>



### variable tx\_pending 

```C++
bool SYN_CANOpenNode::tx_pending;
```



Transmit frame pending flag 


        

<hr>



### variable tx\_queue 

```C++
SYN_CANOpenTxSlot SYN_CANOpenNode::tx_queue[SYN_CANOPEN_TX_QUEUE_DEPTH];
```



Outbound transmit FIFO ring 


        

<hr>



### variable tx\_tail 

```C++
uint8_t SYN_CANOpenNode::tx_tail;
```



TX FIFO write index 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_canopen.h`

