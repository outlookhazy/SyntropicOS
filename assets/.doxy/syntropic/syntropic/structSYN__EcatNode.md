

# Struct SYN\_EcatNode



[**ClassList**](annotated.md) **>** [**SYN\_EcatNode**](structSYN__EcatNode.md)



_EtherCAT Node (Slave ESC) Descriptor._ 

* `#include <syn_ethercat.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint16\_t | [**al\_status**](#variable-al_status)  <br> |
|  [**SYN\_EcatFMMUConfig**](structSYN__EcatFMMUConfig.md) | [**fmmu**](#variable-fmmu)  <br> |
|  uint8\_t | [**mbox\_in**](#variable-mbox_in)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**mbox\_in\_full**](#variable-mbox_in_full)  <br> |
|  uint8\_t | [**mbox\_out**](#variable-mbox_out)  <br> |
|  [**SYN\_CANOpenNode**](structSYN__CANOpenNode.md) \* | [**od**](#variable-od)  <br> |
|  uint8\_t | [**pdo\_in**](#variable-pdo_in)  <br> |
|  uint8\_t | [**pdo\_out**](#variable-pdo_out)  <br> |
|  uint32\_t | [**rx\_pdos**](#variable-rx_pdos)  <br> |
|  uint32\_t | [**sii\_addr**](#variable-sii_addr)  <br> |
|  uint16\_t | [**sii\_ctrl**](#variable-sii_ctrl)  <br> |
|  uint32\_t | [**sii\_words**](#variable-sii_words)  <br> |
|  [**SYN\_EcatSMConfig**](structSYN__EcatSMConfig.md) | [**sm**](#variable-sm)  <br> |
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



### variable fmmu 

```C++
SYN_EcatFMMUConfig SYN_EcatNode::fmmu[4];
```



Slave FMMU 0..3 registers 


        

<hr>



### variable mbox\_in 

```C++
uint8_t SYN_EcatNode::mbox_in[SYN_ECAT_MBOX_SIZE];
```



SM1 Slave-&gt;Master mailbox DPRAM 


        

<hr>



### variable mbox\_in\_full 

```C++
bool SYN_EcatNode::mbox_in_full;
```



SM1 mailbox response queued flag 


        

<hr>



### variable mbox\_out 

```C++
uint8_t SYN_EcatNode::mbox_out[SYN_ECAT_MBOX_SIZE];
```



SM0 Master-&gt;Slave mailbox DPRAM 


        

<hr>



### variable od 

```C++
SYN_CANOpenNode* SYN_EcatNode::od;
```



Optional CANopen Object Dictionary for CoE 


        

<hr>



### variable pdo\_in 

```C++
uint8_t SYN_EcatNode::pdo_in[SYN_ECAT_PDO_BUF_SIZE];
```



TxPDO DPRAM (0x1140..0x117F) 


        

<hr>



### variable pdo\_out 

```C++
uint8_t SYN_EcatNode::pdo_out[SYN_ECAT_PDO_BUF_SIZE];
```



RxPDO DPRAM (0x1100..0x113F) 


        

<hr>



### variable rx\_pdos 

```C++
uint32_t SYN_EcatNode::rx_pdos;
```



Processed RxPDO count 


        

<hr>



### variable sii\_addr 

```C++
uint32_t SYN_EcatNode::sii_addr;
```



SII EEPROM word offset register 


        

<hr>



### variable sii\_ctrl 

```C++
uint16_t SYN_EcatNode::sii_ctrl;
```



SII EEPROM control/status register 


        

<hr>



### variable sii\_words 

```C++
uint32_t SYN_EcatNode::sii_words[SYN_ECAT_SII_WORDS];
```



Emulated SII EEPROM 32-bit word table 


        

<hr>



### variable sm 

```C++
SYN_EcatSMConfig SYN_EcatNode::sm[4];
```



Slave SyncManager 0..3 registers 


        

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



Expected Working Counter threshold 


        

<hr>



### variable wkc\_last 

```C++
uint16_t SYN_EcatNode::wkc_last;
```



Last processed Working Counter 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_ethercat.h`

