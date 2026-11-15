

# File syn\_adpcm.c



[**FileList**](files.md) **>** [**audio**](dir_716e324b7bf36f9621c6c050b7298dab.md) **>** [**syn\_adpcm.c**](syn__adpcm_8c.md)

[Go to the source code of this file](syn__adpcm_8c_source.md)

_IMA/DVI ADPCM Speech and Audio Codec implementation._ 

* `#include "../util/syn_assert.h"`
* `#include "syn_adpcm.h"`
* `#include <string.h>`























## Public Static Attributes

| Type | Name |
| ---: | :--- |
|  const int8\_t | [**g\_adpcm\_index\_table**](#variable-g_adpcm_index_table)   = `/* multi line expression */`<br> |
|  const int16\_t | [**g\_adpcm\_step\_table**](#variable-g_adpcm_step_table)   = `/* multi line expression */`<br> |














## Public Functions

| Type | Name |
| ---: | :--- |
|  size\_t | [**syn\_adpcm\_decode\_block**](#function-syn_adpcm_decode_block) ([**SYN\_ADPCM\_State**](structSYN__ADPCM__State.md) \* state, const uint8\_t \* in, int16\_t \* out, size\_t sample\_count) <br>_Decode a buffer of packed 4-bit ADPCM bytes into 16-bit PCM samples._  |
|  int16\_t | [**syn\_adpcm\_decode\_sample**](#function-syn_adpcm_decode_sample) ([**SYN\_ADPCM\_State**](structSYN__ADPCM__State.md) \* state, uint8\_t nibble) <br>_Decode a single 4-bit ADPCM nibble to a 16-bit PCM sample._  |
|  size\_t | [**syn\_adpcm\_encode\_block**](#function-syn_adpcm_encode_block) ([**SYN\_ADPCM\_State**](structSYN__ADPCM__State.md) \* state, const int16\_t \* in, uint8\_t \* out, size\_t sample\_count) <br>_Encode 16-bit PCM samples into packed 4-bit ADPCM bytes._  |
|  uint8\_t | [**syn\_adpcm\_encode\_sample**](#function-syn_adpcm_encode_sample) ([**SYN\_ADPCM\_State**](structSYN__ADPCM__State.md) \* state, int16\_t sample) <br>_Encode a single 16-bit PCM sample to a 4-bit ADPCM nibble._  |
|  void | [**syn\_adpcm\_init**](#function-syn_adpcm_init) ([**SYN\_ADPCM\_State**](structSYN__ADPCM__State.md) \* state) <br>_Initialize or reset an IMA-ADPCM state context._  |




























## Public Static Attributes Documentation




### variable g\_adpcm\_index\_table 

```C++
const int8_t g_adpcm_index_table[16];
```



IMA-ADPCM Step Index Adjust Table (16 entries) 


        

<hr>



### variable g\_adpcm\_step\_table 

```C++
const int16_t g_adpcm_step_table[89];
```



IMA-ADPCM Quantization Step Table (89 entries) 


        

<hr>
## Public Functions Documentation




### function syn\_adpcm\_decode\_block 

_Decode a buffer of packed 4-bit ADPCM bytes into 16-bit PCM samples._ 
```C++
size_t syn_adpcm_decode_block (
    SYN_ADPCM_State * state,
    const uint8_t * in,
    int16_t * out,
    size_t sample_count
) 
```





**Parameters:**


* `state` State context pointer. 
* `in` Input packed ADPCM byte buffer (2 nibbles per byte, low nibble first). 
* `out` Output 16-bit PCM sample buffer. 
* `sample_count` Total number of samples to decode. 



**Returns:**

Number of samples successfully decoded. 





        

<hr>



### function syn\_adpcm\_decode\_sample 

_Decode a single 4-bit ADPCM nibble to a 16-bit PCM sample._ 
```C++
int16_t syn_adpcm_decode_sample (
    SYN_ADPCM_State * state,
    uint8_t nibble
) 
```





**Parameters:**


* `state` State context pointer. 
* `nibble` 4-bit ADPCM code (lower 4 bits used). 



**Returns:**

Decoded 16-bit PCM sample. 





        

<hr>



### function syn\_adpcm\_encode\_block 

_Encode 16-bit PCM samples into packed 4-bit ADPCM bytes._ 
```C++
size_t syn_adpcm_encode_block (
    SYN_ADPCM_State * state,
    const int16_t * in,
    uint8_t * out,
    size_t sample_count
) 
```





**Parameters:**


* `state` State context pointer. 
* `in` Input 16-bit PCM sample buffer. 
* `out` Output packed ADPCM byte buffer (requires at least (sample\_count + 1)/2 bytes). 
* `sample_count` Number of samples to encode. 



**Returns:**

Number of packed bytes produced. 





        

<hr>



### function syn\_adpcm\_encode\_sample 

_Encode a single 16-bit PCM sample to a 4-bit ADPCM nibble._ 
```C++
uint8_t syn_adpcm_encode_sample (
    SYN_ADPCM_State * state,
    int16_t sample
) 
```





**Parameters:**


* `state` State context pointer. 
* `sample` 16-bit linear PCM sample. 



**Returns:**

Encoded 4-bit ADPCM nibble (in bits 0-3). 





        

<hr>



### function syn\_adpcm\_init 

_Initialize or reset an IMA-ADPCM state context._ 
```C++
void syn_adpcm_init (
    SYN_ADPCM_State * state
) 
```





**Parameters:**


* `state` State context pointer. 




        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/audio/syn_adpcm.c`

