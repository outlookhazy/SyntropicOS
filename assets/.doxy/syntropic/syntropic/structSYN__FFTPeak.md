

# Struct SYN\_FFTPeak



[**ClassList**](annotated.md) **>** [**SYN\_FFTPeak**](structSYN__FFTPeak.md)



_FFT peak information structure._ 

* `#include <syn_fft.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint16\_t | [**bin**](#variable-bin)  <br> |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**freq\_hz**](#variable-freq_hz)  <br> |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**magnitude**](#variable-magnitude)  <br> |












































## Public Attributes Documentation




### variable bin 

```C++
uint16_t SYN_FFTPeak::bin;
```



FFT bin index 


        

<hr>



### variable freq\_hz 

```C++
q16_t SYN_FFTPeak::freq_hz;
```



Peak frequency in Hz in Q16.16 


        

<hr>



### variable magnitude 

```C++
q16_t SYN_FFTPeak::magnitude;
```



Peak magnitude in Q16.16 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/dsp/syn_fft.h`

