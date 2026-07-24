

# File syn\_lut.h



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_lut.h**](syn__lut_8h.md)

[Go to the source code of this file](syn__lut_8h_source.md)

_Lookup table with linear interpolation._ [More...](#detailed-description)

* `#include <stdint.h>`
* `#include <stddef.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_LUT\_Entry**](structSYN__LUT__Entry.md) <br>_Lookup table entry (x, y pair)._  |
























## Public Static Functions

| Type | Name |
| ---: | :--- |
|  int32\_t | [**syn\_lut\_forward**](#function-syn_lut_forward) (const [**SYN\_LUT\_Entry**](structSYN__LUT__Entry.md) \* table, size\_t count, int32\_t x) <br>_Look up y for a given x, with linear interpolation._  |
|  int32\_t | [**syn\_lut\_reverse**](#function-syn_lut_reverse) (const [**SYN\_LUT\_Entry**](structSYN__LUT__Entry.md) \* table, size\_t count, int32\_t y) <br>_Look up x for a given y, with linear interpolation._  |


























## Detailed Description


For calibration curves, thermistor tables, transfer functions, etc. The table is a const array of (x, y) pairs, sorted by ascending x. Lookups interpolate linearly between the two nearest entries.


Header-only — integer math, no floating point.




**
**


```C++
static const SYN_LUT_Entry thermistor_table[] = {
    {   0, 3300 },  // 0°C → 3300 ADC counts
    {  25, 2048 },
    {  50, 1200 },
    { 100,  400 },
};

int32_t temp = syn_lut_lookup(thermistor_table, 4, adc_value);
// Returns interpolated x for a given y

int32_t adc = syn_lut_forward(thermistor_table, 4, 37);
// Returns interpolated y for a given x (37°C)
```
 





    
## Public Static Functions Documentation




### function syn\_lut\_forward 

_Look up y for a given x, with linear interpolation._ 
```C++
static inline int32_t syn_lut_forward (
    const SYN_LUT_Entry * table,
    size_t count,
    int32_t x
) 
```



Table must be sorted by ascending x. Values outside the table range are clamped to the nearest endpoint.




**Parameters:**


* `table` Array of (x, y) entries. 
* `count` Number of entries. 
* `x` Input value. 



**Returns:**

Interpolated y. 





        

<hr>



### function syn\_lut\_reverse 

_Look up x for a given y, with linear interpolation._ 
```C++
static inline int32_t syn_lut_reverse (
    const SYN_LUT_Entry * table,
    size_t count,
    int32_t y
) 
```



The y column can be monotonically increasing OR decreasing. Values outside the table range are clamped.




**Parameters:**


* `table` Array of (x, y) entries, sorted by ascending x. 
* `count` Number of entries. 
* `y` Input value (in the y domain). 



**Returns:**

Interpolated x. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/util/syn_lut.h`

