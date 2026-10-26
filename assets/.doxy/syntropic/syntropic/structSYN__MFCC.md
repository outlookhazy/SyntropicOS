

# Struct SYN\_MFCC



[**ClassList**](annotated.md) **>** [**SYN\_MFCC**](structSYN__MFCC.md)



[More...](#detailed-description)

* `#include <syn_mfcc.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**fft\_imag**](#variable-fft_imag)  <br> |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**fft\_real**](#variable-fft_real)  <br> |
|  uint16\_t | [**fft\_size**](#variable-fft_size)  <br> |
|  [**q16\_t**](syn__qmath_8h.md#typedef-q16_t) | [**mel\_energies**](#variable-mel_energies)  <br> |
|  [**q7\_t**](syn__qmath_8h.md#typedef-q7_t) | [**mfcc\_coeffs**](#variable-mfcc_coeffs)  <br> |
|  uint16\_t | [**sample\_rate\_hz**](#variable-sample_rate_hz)  <br> |












































## Detailed Description


MFCC Extractor Configuration and Workspace Context 


    
## Public Attributes Documentation




### variable fft\_imag 

```C++
q16_t SYN_MFCC::fft_imag[SYN_MFCC_MAX_FFT_SIZE];
```



Internal FFT imag workspace 


        

<hr>



### variable fft\_real 

```C++
q16_t SYN_MFCC::fft_real[SYN_MFCC_MAX_FFT_SIZE];
```



Internal FFT real workspace 


        

<hr>



### variable fft\_size 

```C++
uint16_t SYN_MFCC::fft_size;
```



FFT length (must be &lt;= 128, power of 2) 


        

<hr>



### variable mel\_energies 

```C++
q16_t SYN_MFCC::mel_energies[SYN_MFCC_NUM_MEL_FILTERS];
```



Mel filterbank log energies 


        

<hr>



### variable mfcc\_coeffs 

```C++
q7_t SYN_MFCC::mfcc_coeffs[SYN_MFCC_NUM_COEFFS];
```



Output 13 Q7 MFCC feature vector 


        

<hr>



### variable sample\_rate\_hz 

```C++
uint16_t SYN_MFCC::sample_rate_hz;
```



Audio sampling rate in Hz (e.g. 16000) 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/dsp/syn_mfcc.h`

