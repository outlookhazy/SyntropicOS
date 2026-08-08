

# Struct SYN\_WAV\_Info



[**ClassList**](annotated.md) **>** [**SYN\_WAV\_Info**](structSYN__WAV__Info.md)



_Parsed WAV Header Metadata._ 

* `#include <syn_wav.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint16\_t | [**audio\_format**](#variable-audio_format)  <br> |
|  uint16\_t | [**bits\_per\_sample**](#variable-bits_per_sample)  <br> |
|  uint16\_t | [**block\_align**](#variable-block_align)  <br> |
|  uint32\_t | [**byte\_rate**](#variable-byte_rate)  <br> |
|  size\_t | [**data\_offset**](#variable-data_offset)  <br> |
|  uint32\_t | [**data\_size**](#variable-data_size)  <br> |
|  uint16\_t | [**num\_channels**](#variable-num_channels)  <br> |
|  uint32\_t | [**sample\_rate**](#variable-sample_rate)  <br> |
|  uint32\_t | [**total\_samples**](#variable-total_samples)  <br> |












































## Public Attributes Documentation




### variable audio\_format 

```C++
uint16_t SYN_WAV_Info::audio_format;
```



Format tag (1 = PCM, 17 = IMA ADPCM) 
 


        

<hr>



### variable bits\_per\_sample 

```C++
uint16_t SYN_WAV_Info::bits_per_sample;
```



Bits per sample (8, 16, 4) 
 


        

<hr>



### variable block\_align 

```C++
uint16_t SYN_WAV_Info::block_align;
```



Block alignment size in bytes 
 


        

<hr>



### variable byte\_rate 

```C++
uint32_t SYN_WAV_Info::byte_rate;
```



Average bytes per second 
 


        

<hr>



### variable data\_offset 

```C++
size_t SYN_WAV_Info::data_offset;
```



Byte offset in buffer where PCM/audio starts 


        

<hr>



### variable data\_size 

```C++
uint32_t SYN_WAV_Info::data_size;
```



Size of data chunk in bytes 
 


        

<hr>



### variable num\_channels 

```C++
uint16_t SYN_WAV_Info::num_channels;
```



Number of channels (1 = Mono, 2 = Stereo) 
 


        

<hr>



### variable sample\_rate 

```C++
uint32_t SYN_WAV_Info::sample_rate;
```



Sampling rate in Hz 
 


        

<hr>



### variable total\_samples 

```C++
uint32_t SYN_WAV_Info::total_samples;
```



Calculated total sample count 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/audio/syn_wav.h`

