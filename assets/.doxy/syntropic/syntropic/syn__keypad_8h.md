

# File syn\_keypad.h



[**FileList**](files.md) **>** [**input**](dir_8ff21aebe696416b7e73f727e71a981c.md) **>** [**syn\_keypad.h**](syn__keypad_8h.md)

[Go to the source code of this file](syn__keypad_8h_source.md)

_Non-blocking Matrix Keypad Scanner (3x4, 4x4, etc.)._ 

* `#include "../common/syn_defs.h"`
* `#include "../port/syn_port_gpio.h"`
* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_Keypad**](structSYN__Keypad.md) <br>_Matrix Keypad Context._  |


## Public Types

| Type | Name |
| ---: | :--- |
| typedef void(\* | [**SYN\_KeypadCallback**](#typedef-syn_keypadcallback)  <br>_Keypad Event Callback function prototype._  |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_keypad\_get\_key**](#function-syn_keypad_get_key) (const [**SYN\_Keypad**](structSYN__Keypad.md) \* kp, char \* out\_key) <br>_Get the currently pressed key character._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_keypad\_init**](#function-syn_keypad_init) ([**SYN\_Keypad**](structSYN__Keypad.md) \* kp, const [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) \* rows, uint8\_t num\_rows, const [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) \* cols, uint8\_t num\_cols, const char \* keymap) <br>_Initialize a Matrix Keypad instance._  |
|  void | [**syn\_keypad\_scan**](#function-syn_keypad_scan) ([**SYN\_Keypad**](structSYN__Keypad.md) \* kp) <br>_Perform a non-blocking matrix scan._  |
|  void | [**syn\_keypad\_set\_callback**](#function-syn_keypad_set_callback) ([**SYN\_Keypad**](structSYN__Keypad.md) \* kp, [**SYN\_KeypadCallback**](syn__keypad_8h.md#typedef-syn_keypadcallback) callback, void \* user\_ctx) <br>_Set event callback for key press and release events._  |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_KEYPAD\_MAX\_COLS**](syn__keypad_8h.md#define-syn_keypad_max_cols)  `8`<br> |
| define  | [**SYN\_KEYPAD\_MAX\_ROWS**](syn__keypad_8h.md#define-syn_keypad_max_rows)  `8`<br> |

## Public Types Documentation




### typedef SYN\_KeypadCallback 

_Keypad Event Callback function prototype._ 
```C++
typedef void(* SYN_KeypadCallback) (SYN_Keypad *kp, char key, bool pressed, void *user_ctx);
```





**Parameters:**


* `kp` Pointer to keypad context. 
* `key` Ascii character key from keymap. 
* `pressed` True on key press down, false on release. 
* `user_ctx` User-defined callback context pointer. 




        

<hr>
## Public Functions Documentation




### function syn\_keypad\_get\_key 

_Get the currently pressed key character._ 
```C++
bool syn_keypad_get_key (
    const SYN_Keypad * kp,
    char * out_key
) 
```





**Parameters:**


* `kp` Keypad context. 
* `out_key` [out] Key character (if pressed). 



**Returns:**

True if a key is currently held pressed. 





        

<hr>



### function syn\_keypad\_init 

_Initialize a Matrix Keypad instance._ 
```C++
SYN_Status syn_keypad_init (
    SYN_Keypad * kp,
    const SYN_GPIO_Pin * rows,
    uint8_t num_rows,
    const SYN_GPIO_Pin * cols,
    uint8_t num_cols,
    const char * keymap
) 
```



Configures row pins as outputs and col pins as inputs with pull-down/up.




**Parameters:**


* `kp` Keypad context. 
* `rows` Array of row GPIO pins. 
* `num_rows` Number of row pins (1-8). 
* `cols` Array of column GPIO pins. 
* `num_cols` Number of column pins (1-8). 
* `keymap` String keymap of length (num\_rows \* num\_cols). 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_keypad\_scan 

_Perform a non-blocking matrix scan._ 
```C++
void syn_keypad_scan (
    SYN_Keypad * kp
) 
```





**Parameters:**


* `kp` Keypad context. 




        

<hr>



### function syn\_keypad\_set\_callback 

_Set event callback for key press and release events._ 
```C++
void syn_keypad_set_callback (
    SYN_Keypad * kp,
    SYN_KeypadCallback callback,
    void * user_ctx
) 
```





**Parameters:**


* `kp` Keypad context. 
* `callback` Event handler callback function. 
* `user_ctx` User context pointer. 




        

<hr>
## Macro Definition Documentation





### define SYN\_KEYPAD\_MAX\_COLS 

```C++
#define SYN_KEYPAD_MAX_COLS `8`
```



Maximum supported keypad columns (8) 


        

<hr>



### define SYN\_KEYPAD\_MAX\_ROWS 

```C++
#define SYN_KEYPAD_MAX_ROWS `8`
```



Maximum supported keypad rows (8) 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/input/syn_keypad.h`

