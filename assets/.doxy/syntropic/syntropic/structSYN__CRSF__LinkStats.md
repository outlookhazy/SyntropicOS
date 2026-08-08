

# Struct SYN\_CRSF\_LinkStats



[**ClassList**](annotated.md) **>** [**SYN\_CRSF\_LinkStats**](structSYN__CRSF__LinkStats.md)



[More...](#detailed-description)

* `#include <syn_crsf.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**active\_antenna**](#variable-active_antenna)  <br> |
|  uint8\_t | [**downlink\_quality**](#variable-downlink_quality)  <br> |
|  uint8\_t | [**downlink\_rssi**](#variable-downlink_rssi)  <br> |
|  int8\_t | [**downlink\_snr**](#variable-downlink_snr)  <br> |
|  uint8\_t | [**rf\_mode**](#variable-rf_mode)  <br> |
|  uint8\_t | [**tx\_power**](#variable-tx_power)  <br> |
|  uint8\_t | [**uplink\_quality**](#variable-uplink_quality)  <br> |
|  uint8\_t | [**uplink\_rssi1**](#variable-uplink_rssi1)  <br> |
|  uint8\_t | [**uplink\_rssi2**](#variable-uplink_rssi2)  <br> |
|  int8\_t | [**uplink\_snr**](#variable-uplink_snr)  <br> |












































## Detailed Description


Parsed CRSF Link Statistics Frame. 


    
## Public Attributes Documentation




### variable active\_antenna 

```C++
uint8_t SYN_CRSF_LinkStats::active_antenna;
```



Active antenna index (0 or 1). 


        

<hr>



### variable downlink\_quality 

```C++
uint8_t SYN_CRSF_LinkStats::downlink_quality;
```



Downlink Quality (0..100%). 


        

<hr>



### variable downlink\_rssi 

```C++
uint8_t SYN_CRSF_LinkStats::downlink_rssi;
```



Downlink RSSI (-dBm). 


        

<hr>



### variable downlink\_snr 

```C++
int8_t SYN_CRSF_LinkStats::downlink_snr;
```



Downlink SNR (dB). 


        

<hr>



### variable rf\_mode 

```C++
uint8_t SYN_CRSF_LinkStats::rf_mode;
```



RF mode (e.g. 50Hz, 150Hz, 500Hz). 


        

<hr>



### variable tx\_power 

```C++
uint8_t SYN_CRSF_LinkStats::tx_power;
```



Transmitter output power. 


        

<hr>



### variable uplink\_quality 

```C++
uint8_t SYN_CRSF_LinkStats::uplink_quality;
```



Link Quality (0..100%). 


        

<hr>



### variable uplink\_rssi1 

```C++
uint8_t SYN_CRSF_LinkStats::uplink_rssi1;
```



Uplink RSSI Antenna 1 (-dBm). 


        

<hr>



### variable uplink\_rssi2 

```C++
uint8_t SYN_CRSF_LinkStats::uplink_rssi2;
```



Uplink RSSI Antenna 2 (-dBm). 


        

<hr>



### variable uplink\_snr 

```C++
int8_t SYN_CRSF_LinkStats::uplink_snr;
```



Signal-to-Noise Ratio (dB). 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_crsf.h`

