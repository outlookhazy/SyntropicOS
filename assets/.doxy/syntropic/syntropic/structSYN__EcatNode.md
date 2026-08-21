

# Struct SYN\_EcatNode



[**ClassList**](annotated.md) **>** [**SYN\_EcatNode**](structSYN__EcatNode.md)



_EtherCAT Node Descriptor._ 

* `#include <syn_ethercat.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint16\_t | [**al\_status**](#variable-al_status)  <br> |
|  [**SYN\_CANOpenNode**](structSYN__CANOpenNode.md) \* | [**od**](#variable-od)  <br> |
|  uint32\_t | [**rx\_pdos**](#variable-rx_pdos)  <br> |
|  [**SYN\_EcatState**](syn__ethercat_8h.md#enum-syn_ecatstate) | [**state**](#variable-state)  <br> |
|  uint16\_t | [**station\_addr**](#variable-station_addr)  <br> |
|  [**SYN\_EcatState**](syn__ethercat_8h.md#enum-syn_ecatstate) | [**target\_state**](#variable-target_state)  <br> |
|  uint32\_t | [**tx\_pdos**](#variable-tx_pdos)  <br> |
|  uint16\_t | [**wkc\_expected**](#variable-wkc_expected)  <br> |
|  uint16\_t | [**wkc\_last**](#variable-wkc_last)  <br> |












































## Public Attributes Documentation




### variable al\_status 

```C++
uint16_t SYN_EcatNode::al_status;
```



Application Layer Status Code 


        

<hr>



### variable od 

```C++
SYN_CANOpenNode* SYN_EcatNode::od;
```



Optional binding to CANopen Object Dictionary node 


        

<hr>



### variable rx\_pdos 

```C++
uint32_t SYN_EcatNode::rx_pdos;
```



Processed RxPDO count 


        

<hr>



### variable state 

```C++
SYN_EcatState SYN_EcatNode::state;
```



Current ESM state 


        

<hr>



### variable station\_addr 

```C++
uint16_t SYN_EcatNode::station_addr;
```



Configured station address (FPRD/FPWR) 


        

<hr>



### variable target\_state 

```C++
SYN_EcatState SYN_EcatNode::target_state;
```



Requested ESM state 


        

<hr>



### variable tx\_pdos 

```C++
uint32_t SYN_EcatNode::tx_pdos;
```



Processed TxPDO count 


        

<hr>



### variable wkc\_expected 

```C++
uint16_t SYN_EcatNode::wkc_expected;
```



Expected Working Counter for cyclic exchange 


        

<hr>



### variable wkc\_last 

```C++
uint16_t SYN_EcatNode::wkc_last;
```



Last received Working Counter 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_ethercat.h`

