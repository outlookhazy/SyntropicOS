

# Struct SYN\_N2K\_VesselHeading



[**ClassList**](annotated.md) **>** [**SYN\_N2K\_VesselHeading**](structSYN__N2K__VesselHeading.md)



_Vessel Heading (PGN 127250)._ 

* `#include <syn_n2k.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  int16\_t | [**deviation\_rad\_1e4**](#variable-deviation_rad_1e4)  <br> |
|  uint16\_t | [**heading\_rad\_1e4**](#variable-heading_rad_1e4)  <br> |
|  uint8\_t | [**heading\_ref**](#variable-heading_ref)  <br> |
|  uint8\_t | [**sid**](#variable-sid)  <br> |
|  int16\_t | [**variation\_rad\_1e4**](#variable-variation_rad_1e4)  <br> |












































## Public Attributes Documentation




### variable deviation\_rad\_1e4 

```C++
int16_t SYN_N2K_VesselHeading::deviation_rad_1e4;
```



Magnetic Deviation in 0.0001 radians 


        

<hr>



### variable heading\_rad\_1e4 

```C++
uint16_t SYN_N2K_VesselHeading::heading_rad_1e4;
```



Heading angle in 0.0001 radians 


        

<hr>



### variable heading\_ref 

```C++
uint8_t SYN_N2K_VesselHeading::heading_ref;
```



Heading Reference (0=True, 1=Magnetic) 


        

<hr>



### variable sid 

```C++
uint8_t SYN_N2K_VesselHeading::sid;
```



Sequence ID 


        

<hr>



### variable variation\_rad\_1e4 

```C++
int16_t SYN_N2K_VesselHeading::variation_rad_1e4;
```



Magnetic Variation in 0.0001 radians 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_n2k.h`

