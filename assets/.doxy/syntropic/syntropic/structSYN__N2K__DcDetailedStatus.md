

# Struct SYN\_N2K\_DcDetailedStatus



[**ClassList**](annotated.md) **>** [**SYN\_N2K\_DcDetailedStatus**](structSYN__N2K__DcDetailedStatus.md)



_DC Detailed Status (PGN 127506)._ 

* `#include <syn_n2k.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint16\_t | [**capacity\_ah\_1e1**](#variable-capacity_ah_1e1)  <br> |
|  uint8\_t | [**dc\_type**](#variable-dc_type)  <br> |
|  uint8\_t | [**instance**](#variable-instance)  <br> |
|  uint8\_t | [**sid**](#variable-sid)  <br> |
|  uint8\_t | [**state\_of\_charge**](#variable-state_of_charge)  <br> |
|  uint8\_t | [**state\_of\_health**](#variable-state_of_health)  <br> |
|  uint16\_t | [**time\_to\_go\_min**](#variable-time_to_go_min)  <br> |












































## Public Attributes Documentation




### variable capacity\_ah\_1e1 

```C++
uint16_t SYN_N2K_DcDetailedStatus::capacity_ah_1e1;
```



Capacity / Ripple Voltage in 0.1 AH 


        

<hr>



### variable dc\_type 

```C++
uint8_t SYN_N2K_DcDetailedStatus::dc_type;
```



DC Source Type (0=Battery, 1=Alternator, 2=Convertor, 3=Solar, 4=Wind) 


        

<hr>



### variable instance 

```C++
uint8_t SYN_N2K_DcDetailedStatus::instance;
```



DC Instance ID (0-254) 


        

<hr>



### variable sid 

```C++
uint8_t SYN_N2K_DcDetailedStatus::sid;
```



Sequence ID 


        

<hr>



### variable state\_of\_charge 

```C++
uint8_t SYN_N2K_DcDetailedStatus::state_of_charge;
```



State of Charge in 1% (0 to 100%) 


        

<hr>



### variable state\_of\_health 

```C++
uint8_t SYN_N2K_DcDetailedStatus::state_of_health;
```



State of Health in 1% (0 to 100%) 


        

<hr>



### variable time\_to\_go\_min 

```C++
uint16_t SYN_N2K_DcDetailedStatus::time_to_go_min;
```



Time Remaining in minutes (0 to 65532 min) 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_n2k.h`

