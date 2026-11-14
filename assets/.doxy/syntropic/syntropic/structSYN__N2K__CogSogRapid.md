

# Struct SYN\_N2K\_CogSogRapid



[**ClassList**](annotated.md) **>** [**SYN\_N2K\_CogSogRapid**](structSYN__N2K__CogSogRapid.md)



_Rapid Update COG & SOG (PGN 129026)._ 

* `#include <syn_n2k.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint16\_t | [**cog\_rad\_1e4**](#variable-cog_rad_1e4)  <br> |
|  uint8\_t | [**cog\_ref**](#variable-cog_ref)  <br> |
|  uint8\_t | [**sid**](#variable-sid)  <br> |
|  uint16\_t | [**sog\_m\_s\_1e2**](#variable-sog_m_s_1e2)  <br> |












































## Public Attributes Documentation




### variable cog\_rad\_1e4 

```C++
uint16_t SYN_N2K_CogSogRapid::cog_rad_1e4;
```



Course Over Ground in 0.0001 radians (0 to 62831) 


        

<hr>



### variable cog\_ref 

```C++
uint8_t SYN_N2K_CogSogRapid::cog_ref;
```



Course Reference (0=True, 1=Magnetic) 


        

<hr>



### variable sid 

```C++
uint8_t SYN_N2K_CogSogRapid::sid;
```



Sequence ID / Measurement counter 


        

<hr>



### variable sog\_m\_s\_1e2 

```C++
uint16_t SYN_N2K_CogSogRapid::sog_m_s_1e2;
```



Speed Over Ground in 0.01 m/s (0 to 65532) 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_n2k.h`

