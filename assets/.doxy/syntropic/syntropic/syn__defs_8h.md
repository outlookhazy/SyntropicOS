

# File syn\_defs.h



[**FileList**](files.md) **>** [**common**](dir_b4b1f229ee23b400200c904b1998093f.md) **>** [**syn\_defs.h**](syn__defs_8h.md)

[Go to the source code of this file](syn__defs_8h_source.md)

_Common type definitions and status codes for SyntropicOS._ [More...](#detailed-description)

* `#include <stdint.h>`
* `#include <stddef.h>`

















## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**SYN\_GPIO\_Mode**](#enum-syn_gpio_mode)  <br>_GPIO pin mode / direction._  |
| typedef uint16\_t | [**SYN\_GPIO\_Pin**](#typedef-syn_gpio_pin)  <br>_Abstract GPIO pin identifier._  |
| enum  | [**SYN\_GPIO\_State**](#enum-syn_gpio_state)  <br>_Logical state of a GPIO pin._  |
| enum  | [**SYN\_Status**](#enum-syn_status)  <br>_Return status used by all SyntropicOS functions._  |
| typedef uint8\_t | [**SYN\_UARTInstance**](#typedef-syn_uartinstance)  <br>_Identifies a UART peripheral instance (0, 1, 2, …)._  |
| enum  | [**bool**](#enum-bool)  <br>_C99 bool fallback for pre-C99 compilers._  |















































## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_ABS**](syn__defs_8h.md#define-syn_abs) (x) `(((x) &lt; 0) ? -(x) : (x))`<br> |
| define  | [**SYN\_ARRAY\_SIZE**](syn__defs_8h.md#define-syn_array_size) (arr) `(sizeof(arr) / sizeof((arr)[0]))`<br> |
| define  | [**SYN\_CLAMP**](syn__defs_8h.md#define-syn_clamp) (val, lo, hi) `(((val) &lt; (lo)) ? (lo) : (((val) &gt; (hi)) ? (hi) : (val)))`<br> |
| define  | [**SYN\_MAX**](syn__defs_8h.md#define-syn_max) (a, b) `(((a) &gt; (b)) ? (a) : (b))`<br>_Maximum of two values._  |
| define  | [**SYN\_MIN**](syn__defs_8h.md#define-syn_min) (a, b) `(((a) &lt; (b)) ? (a) : (b))`<br>_Minimum of two values._  |
| define  | [**SYN\_SIGN**](syn__defs_8h.md#define-syn_sign) (x) `(((x) &gt; 0) - ((x) &lt; 0))`<br> |
| define  | [**SYN\_UNUSED\_VAR**](syn__defs_8h.md#define-syn_unused_var) (x) `((void)(x))`<br> |

## Detailed Description


This header is included by virtually every other SyntropicOS file. It provides the fundamental types, enumerations, and constants used throughout the library. 


    
## Public Types Documentation




### enum SYN\_GPIO\_Mode 

_GPIO pin mode / direction._ 
```C++
enum SYN_GPIO_Mode {
    SYN_GPIO_INPUT = 0,
    SYN_GPIO_OUTPUT = 1,
    SYN_GPIO_INPUT_PULLUP = 2,
    SYN_GPIO_INPUT_PULLDOWN = 3,
    SYN_GPIO_OUTPUT_OD = 4
};
```




<hr>



### typedef SYN\_GPIO\_Pin 

_Abstract GPIO pin identifier._ 
```C++
typedef uint16_t SYN_GPIO_Pin;
```



The meaning of this value is platform-specific. It may encode a port+pin pair, a flat pin number, or any scheme the port layer defines. 


        

<hr>



### enum SYN\_GPIO\_State 

_Logical state of a GPIO pin._ 
```C++
enum SYN_GPIO_State {
    SYN_GPIO_LOW = 0,
    SYN_GPIO_HIGH = 1
};
```




<hr>



### enum SYN\_Status 

_Return status used by all SyntropicOS functions._ 
```C++
enum SYN_Status {
    SYN_OK = 0,
    SYN_ERROR = -1,
    SYN_BUSY = -2,
    SYN_TIMEOUT = -3,
    SYN_INVALID_PARAM = -4,
    SYN_NOT_IMPLEMENTED = -5
};
```




<hr>



### typedef SYN\_UARTInstance 

_Identifies a UART peripheral instance (0, 1, 2, …)._ 
```C++
typedef uint8_t SYN_UARTInstance;
```




<hr>



### enum bool 

_C99 bool fallback for pre-C99 compilers._ 
```C++
enum bool {
    false = 0,
    true = 1
};
```




<hr>
## Macro Definition Documentation





### define SYN\_ABS 

```C++
#define SYN_ABS (
    x
) `(((x) < 0) ? -(x) : (x))`
```



Absolute value 


        

<hr>



### define SYN\_ARRAY\_SIZE 

```C++
#define SYN_ARRAY_SIZE (
    arr
) `(sizeof(arr) / sizeof((arr)[0]))`
```



Number of elements in a statically-allocated array. 


        

<hr>



### define SYN\_CLAMP 

```C++
#define SYN_CLAMP (
    val,
    lo,
    hi
) `(((val) < (lo)) ? (lo) : (((val) > (hi)) ? (hi) : (val)))`
```



Clamp value to [lo, hi] 


        

<hr>



### define SYN\_MAX 

_Maximum of two values._ 
```C++
#define SYN_MAX (
    a,
    b
) `(((a) > (b)) ? (a) : (b))`
```




<hr>



### define SYN\_MIN 

_Minimum of two values._ 
```C++
#define SYN_MIN (
    a,
    b
) `(((a) < (b)) ? (a) : (b))`
```




<hr>



### define SYN\_SIGN 

```C++
#define SYN_SIGN (
    x
) `(((x) > 0) - ((x) < 0))`
```



Sign: returns -1, 0, or +1 


        

<hr>



### define SYN\_UNUSED\_VAR 

```C++
#define SYN_UNUSED_VAR (
    x
) `((void)(x))`
```



Suppress unused-variable warnings. 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/common/syn_defs.h`

