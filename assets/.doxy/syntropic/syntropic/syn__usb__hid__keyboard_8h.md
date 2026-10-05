

# File syn\_usb\_hid\_keyboard.h



[**FileList**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_usb\_hid\_keyboard.h**](syn__usb__hid__keyboard_8h.md)

[Go to the source code of this file](syn__usb__hid__keyboard_8h_source.md)

_Zero-Heap USB HID Boot Keyboard Device Helper Module._ 

* `#include "syntropic/common/syn_defs.h"`
* `#include "syntropic/drivers/syn_usb_hid.h"`
* `#include <stdbool.h>`
* `#include <stddef.h>`
* `#include <stdint.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_USB\_HID\_KeyboardReport**](structSYN__USB__HID__KeyboardReport.md) <br> |






## Public Attributes

| Type | Name |
| ---: | :--- |
|  const uint8\_t | [**SYN\_USB\_HID\_KEYBOARD\_REPORT\_DESC**](#variable-syn_usb_hid_keyboard_report_desc)  <br> |
















## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_usb\_hid\_keyboard\_press**](#function-syn_usb_hid_keyboard_press) ([**SYN\_USB\_HID**](structSYN__USB__HID.md) \* hid, uint8\_t modifier, uint8\_t keycode) <br>_Send a single keystroke (press 1 key + optional modifier)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_usb\_hid\_keyboard\_release**](#function-syn_usb_hid_keyboard_release) ([**SYN\_USB\_HID**](structSYN__USB__HID.md) \* hid) <br>_Send a keyboard release report (all keys released)._  |
|  [**SYN\_Status**](syn__defs_8h.md#enum-syn_status) | [**syn\_usb\_hid\_keyboard\_send**](#function-syn_usb_hid_keyboard_send) ([**SYN\_USB\_HID**](structSYN__USB__HID.md) \* hid, uint8\_t modifier, const uint8\_t keycodes) <br>_Send a USB HID Keyboard report containing active keycodes._  |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_USB\_HID\_KEY\_0**](syn__usb__hid__keyboard_8h.md#define-syn_usb_hid_key_0)  `0x27U`<br>_USB HID Key 0 code._  |
| define  | [**SYN\_USB\_HID\_KEY\_1**](syn__usb__hid__keyboard_8h.md#define-syn_usb_hid_key_1)  `0x1EU`<br>_USB HID Key 1 code._  |
| define  | [**SYN\_USB\_HID\_KEY\_2**](syn__usb__hid__keyboard_8h.md#define-syn_usb_hid_key_2)  `0x1FU`<br>_USB HID Key 2 code._  |
| define  | [**SYN\_USB\_HID\_KEY\_3**](syn__usb__hid__keyboard_8h.md#define-syn_usb_hid_key_3)  `0x20U`<br>_USB HID Key 3 code._  |
| define  | [**SYN\_USB\_HID\_KEY\_4**](syn__usb__hid__keyboard_8h.md#define-syn_usb_hid_key_4)  `0x21U`<br>_USB HID Key 4 code._  |
| define  | [**SYN\_USB\_HID\_KEY\_5**](syn__usb__hid__keyboard_8h.md#define-syn_usb_hid_key_5)  `0x22U`<br>_USB HID Key 5 code._  |
| define  | [**SYN\_USB\_HID\_KEY\_6**](syn__usb__hid__keyboard_8h.md#define-syn_usb_hid_key_6)  `0x23U`<br>_USB HID Key 6 code._  |
| define  | [**SYN\_USB\_HID\_KEY\_7**](syn__usb__hid__keyboard_8h.md#define-syn_usb_hid_key_7)  `0x24U`<br>_USB HID Key 7 code._  |
| define  | [**SYN\_USB\_HID\_KEY\_8**](syn__usb__hid__keyboard_8h.md#define-syn_usb_hid_key_8)  `0x25U`<br>_USB HID Key 8 code._  |
| define  | [**SYN\_USB\_HID\_KEY\_9**](syn__usb__hid__keyboard_8h.md#define-syn_usb_hid_key_9)  `0x26U`<br>_USB HID Key 9 code._  |
| define  | [**SYN\_USB\_HID\_KEY\_A**](syn__usb__hid__keyboard_8h.md#define-syn_usb_hid_key_a)  `0x04U`<br>_USB HID Key A code._  |
| define  | [**SYN\_USB\_HID\_KEY\_B**](syn__usb__hid__keyboard_8h.md#define-syn_usb_hid_key_b)  `0x05U`<br>_USB HID Key B code._  |
| define  | [**SYN\_USB\_HID\_KEY\_BACKSPACE**](syn__usb__hid__keyboard_8h.md#define-syn_usb_hid_key_backspace)  `0x2AU`<br>_USB HID Backspace key code._  |
| define  | [**SYN\_USB\_HID\_KEY\_C**](syn__usb__hid__keyboard_8h.md#define-syn_usb_hid_key_c)  `0x06U`<br>_USB HID Key C code._  |
| define  | [**SYN\_USB\_HID\_KEY\_D**](syn__usb__hid__keyboard_8h.md#define-syn_usb_hid_key_d)  `0x07U`<br>_USB HID Key D code._  |
| define  | [**SYN\_USB\_HID\_KEY\_E**](syn__usb__hid__keyboard_8h.md#define-syn_usb_hid_key_e)  `0x08U`<br>_USB HID Key E code._  |
| define  | [**SYN\_USB\_HID\_KEY\_ENTER**](syn__usb__hid__keyboard_8h.md#define-syn_usb_hid_key_enter)  `0x28U`<br>_USB HID Enter key code._  |
| define  | [**SYN\_USB\_HID\_KEY\_ESCAPE**](syn__usb__hid__keyboard_8h.md#define-syn_usb_hid_key_escape)  `0x29U`<br>_USB HID Escape key code._  |
| define  | [**SYN\_USB\_HID\_KEY\_F**](syn__usb__hid__keyboard_8h.md#define-syn_usb_hid_key_f)  `0x09U`<br>_USB HID Key F code._  |
| define  | [**SYN\_USB\_HID\_KEY\_G**](syn__usb__hid__keyboard_8h.md#define-syn_usb_hid_key_g)  `0x0AU`<br>_USB HID Key G code._  |
| define  | [**SYN\_USB\_HID\_KEY\_H**](syn__usb__hid__keyboard_8h.md#define-syn_usb_hid_key_h)  `0x0BU`<br>_USB HID Key H code._  |
| define  | [**SYN\_USB\_HID\_KEY\_I**](syn__usb__hid__keyboard_8h.md#define-syn_usb_hid_key_i)  `0x0CU`<br>_USB HID Key I code._  |
| define  | [**SYN\_USB\_HID\_KEY\_J**](syn__usb__hid__keyboard_8h.md#define-syn_usb_hid_key_j)  `0x0DU`<br>_USB HID Key J code._  |
| define  | [**SYN\_USB\_HID\_KEY\_K**](syn__usb__hid__keyboard_8h.md#define-syn_usb_hid_key_k)  `0x0EU`<br>_USB HID Key K code._  |
| define  | [**SYN\_USB\_HID\_KEY\_L**](syn__usb__hid__keyboard_8h.md#define-syn_usb_hid_key_l)  `0x0FU`<br>_USB HID Key L code._  |
| define  | [**SYN\_USB\_HID\_KEY\_M**](syn__usb__hid__keyboard_8h.md#define-syn_usb_hid_key_m)  `0x10U`<br>_USB HID Key M code._  |
| define  | [**SYN\_USB\_HID\_KEY\_N**](syn__usb__hid__keyboard_8h.md#define-syn_usb_hid_key_n)  `0x11U`<br>_USB HID Key N code._  |
| define  | [**SYN\_USB\_HID\_KEY\_NONE**](syn__usb__hid__keyboard_8h.md#define-syn_usb_hid_key_none)  `0x00U`<br>_USB HID No key pressed code._  |
| define  | [**SYN\_USB\_HID\_KEY\_O**](syn__usb__hid__keyboard_8h.md#define-syn_usb_hid_key_o)  `0x12U`<br>_USB HID Key O code._  |
| define  | [**SYN\_USB\_HID\_KEY\_P**](syn__usb__hid__keyboard_8h.md#define-syn_usb_hid_key_p)  `0x13U`<br>_USB HID Key P code._  |
| define  | [**SYN\_USB\_HID\_KEY\_Q**](syn__usb__hid__keyboard_8h.md#define-syn_usb_hid_key_q)  `0x14U`<br>_USB HID Key Q code._  |
| define  | [**SYN\_USB\_HID\_KEY\_R**](syn__usb__hid__keyboard_8h.md#define-syn_usb_hid_key_r)  `0x15U`<br>_USB HID Key R code._  |
| define  | [**SYN\_USB\_HID\_KEY\_S**](syn__usb__hid__keyboard_8h.md#define-syn_usb_hid_key_s)  `0x16U`<br>_USB HID Key S code._  |
| define  | [**SYN\_USB\_HID\_KEY\_SPACE**](syn__usb__hid__keyboard_8h.md#define-syn_usb_hid_key_space)  `0x2CU`<br>_USB HID Space key code._  |
| define  | [**SYN\_USB\_HID\_KEY\_T**](syn__usb__hid__keyboard_8h.md#define-syn_usb_hid_key_t)  `0x17U`<br>_USB HID Key T code._  |
| define  | [**SYN\_USB\_HID\_KEY\_TAB**](syn__usb__hid__keyboard_8h.md#define-syn_usb_hid_key_tab)  `0x2BU`<br>_USB HID Tab key code._  |
| define  | [**SYN\_USB\_HID\_KEY\_U**](syn__usb__hid__keyboard_8h.md#define-syn_usb_hid_key_u)  `0x18U`<br>_USB HID Key U code._  |
| define  | [**SYN\_USB\_HID\_KEY\_V**](syn__usb__hid__keyboard_8h.md#define-syn_usb_hid_key_v)  `0x19U`<br>_USB HID Key V code._  |
| define  | [**SYN\_USB\_HID\_KEY\_W**](syn__usb__hid__keyboard_8h.md#define-syn_usb_hid_key_w)  `0x1AU`<br>_USB HID Key W code._  |
| define  | [**SYN\_USB\_HID\_KEY\_X**](syn__usb__hid__keyboard_8h.md#define-syn_usb_hid_key_x)  `0x1BU`<br>_USB HID Key X code._  |
| define  | [**SYN\_USB\_HID\_KEY\_Y**](syn__usb__hid__keyboard_8h.md#define-syn_usb_hid_key_y)  `0x1CU`<br>_USB HID Key Y code._  |
| define  | [**SYN\_USB\_HID\_KEY\_Z**](syn__usb__hid__keyboard_8h.md#define-syn_usb_hid_key_z)  `0x1DU`<br>_USB HID Key Z code._  |
| define  | [**SYN\_USB\_HID\_MOD\_LALT**](syn__usb__hid__keyboard_8h.md#define-syn_usb_hid_mod_lalt)  `(1U &lt;&lt; 2)`<br>_Left Alt modifier bitmask._  |
| define  | [**SYN\_USB\_HID\_MOD\_LCTRL**](syn__usb__hid__keyboard_8h.md#define-syn_usb_hid_mod_lctrl)  `(1U &lt;&lt; 0)`<br>_Left Ctrl modifier bitmask._  |
| define  | [**SYN\_USB\_HID\_MOD\_LGUI**](syn__usb__hid__keyboard_8h.md#define-syn_usb_hid_mod_lgui)  `(1U &lt;&lt; 3)`<br>_Left GUI modifier bitmask._  |
| define  | [**SYN\_USB\_HID\_MOD\_LSHIFT**](syn__usb__hid__keyboard_8h.md#define-syn_usb_hid_mod_lshift)  `(1U &lt;&lt; 1)`<br>_Left Shift modifier bitmask._  |
| define  | [**SYN\_USB\_HID\_MOD\_RALT**](syn__usb__hid__keyboard_8h.md#define-syn_usb_hid_mod_ralt)  `(1U &lt;&lt; 6)`<br>_Right Alt modifier bitmask._  |
| define  | [**SYN\_USB\_HID\_MOD\_RCTRL**](syn__usb__hid__keyboard_8h.md#define-syn_usb_hid_mod_rctrl)  `(1U &lt;&lt; 4)`<br>_Right Ctrl modifier bitmask._  |
| define  | [**SYN\_USB\_HID\_MOD\_RGUI**](syn__usb__hid__keyboard_8h.md#define-syn_usb_hid_mod_rgui)  `(1U &lt;&lt; 7)`<br>_Right GUI modifier bitmask._  |
| define  | [**SYN\_USB\_HID\_MOD\_RSHIFT**](syn__usb__hid__keyboard_8h.md#define-syn_usb_hid_mod_rshift)  `(1U &lt;&lt; 5)`<br>_Right Shift modifier bitmask._  |

## Public Attributes Documentation




### variable SYN\_USB\_HID\_KEYBOARD\_REPORT\_DESC 

```C++
const uint8_t SYN_USB_HID_KEYBOARD_REPORT_DESC[63];
```



Standard USB HID Keyboard Report Descriptor (63 Bytes) 


        

<hr>
## Public Functions Documentation




### function syn\_usb\_hid\_keyboard\_press 

_Send a single keystroke (press 1 key + optional modifier)._ 
```C++
SYN_Status syn_usb_hid_keyboard_press (
    SYN_USB_HID * hid,
    uint8_t modifier,
    uint8_t keycode
) 
```





**Parameters:**


* `hid` Pointer to USB HID instance. 
* `modifier` Modifier bitmask (CTRL, SHIFT, ALT, GUI). 
* `keycode` HID keycode (e.g. SYN\_USB\_HID\_KEY\_A). 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_usb\_hid\_keyboard\_release 

_Send a keyboard release report (all keys released)._ 
```C++
SYN_Status syn_usb_hid_keyboard_release (
    SYN_USB_HID * hid
) 
```





**Parameters:**


* `hid` Pointer to USB HID instance. 



**Returns:**

SYN\_OK on success. 





        

<hr>



### function syn\_usb\_hid\_keyboard\_send 

_Send a USB HID Keyboard report containing active keycodes._ 
```C++
SYN_Status syn_usb_hid_keyboard_send (
    SYN_USB_HID * hid,
    uint8_t modifier,
    const uint8_t keycodes
) 
```





**Parameters:**


* `hid` Pointer to USB HID instance. 
* `modifier` Modifier bitmask (CTRL, SHIFT, ALT, GUI). 
* `keycodes` Array of up to 6 pressed keycodes (or NULL for no key). 



**Returns:**

SYN\_OK on success. 





        

<hr>
## Macro Definition Documentation





### define SYN\_USB\_HID\_KEY\_0 

_USB HID Key 0 code._ 
```C++
#define SYN_USB_HID_KEY_0 `0x27U`
```




<hr>



### define SYN\_USB\_HID\_KEY\_1 

_USB HID Key 1 code._ 
```C++
#define SYN_USB_HID_KEY_1 `0x1EU`
```




<hr>



### define SYN\_USB\_HID\_KEY\_2 

_USB HID Key 2 code._ 
```C++
#define SYN_USB_HID_KEY_2 `0x1FU`
```




<hr>



### define SYN\_USB\_HID\_KEY\_3 

_USB HID Key 3 code._ 
```C++
#define SYN_USB_HID_KEY_3 `0x20U`
```




<hr>



### define SYN\_USB\_HID\_KEY\_4 

_USB HID Key 4 code._ 
```C++
#define SYN_USB_HID_KEY_4 `0x21U`
```




<hr>



### define SYN\_USB\_HID\_KEY\_5 

_USB HID Key 5 code._ 
```C++
#define SYN_USB_HID_KEY_5 `0x22U`
```




<hr>



### define SYN\_USB\_HID\_KEY\_6 

_USB HID Key 6 code._ 
```C++
#define SYN_USB_HID_KEY_6 `0x23U`
```




<hr>



### define SYN\_USB\_HID\_KEY\_7 

_USB HID Key 7 code._ 
```C++
#define SYN_USB_HID_KEY_7 `0x24U`
```




<hr>



### define SYN\_USB\_HID\_KEY\_8 

_USB HID Key 8 code._ 
```C++
#define SYN_USB_HID_KEY_8 `0x25U`
```




<hr>



### define SYN\_USB\_HID\_KEY\_9 

_USB HID Key 9 code._ 
```C++
#define SYN_USB_HID_KEY_9 `0x26U`
```




<hr>



### define SYN\_USB\_HID\_KEY\_A 

_USB HID Key A code._ 
```C++
#define SYN_USB_HID_KEY_A `0x04U`
```




<hr>



### define SYN\_USB\_HID\_KEY\_B 

_USB HID Key B code._ 
```C++
#define SYN_USB_HID_KEY_B `0x05U`
```




<hr>



### define SYN\_USB\_HID\_KEY\_BACKSPACE 

_USB HID Backspace key code._ 
```C++
#define SYN_USB_HID_KEY_BACKSPACE `0x2AU`
```




<hr>



### define SYN\_USB\_HID\_KEY\_C 

_USB HID Key C code._ 
```C++
#define SYN_USB_HID_KEY_C `0x06U`
```




<hr>



### define SYN\_USB\_HID\_KEY\_D 

_USB HID Key D code._ 
```C++
#define SYN_USB_HID_KEY_D `0x07U`
```




<hr>



### define SYN\_USB\_HID\_KEY\_E 

_USB HID Key E code._ 
```C++
#define SYN_USB_HID_KEY_E `0x08U`
```




<hr>



### define SYN\_USB\_HID\_KEY\_ENTER 

_USB HID Enter key code._ 
```C++
#define SYN_USB_HID_KEY_ENTER `0x28U`
```




<hr>



### define SYN\_USB\_HID\_KEY\_ESCAPE 

_USB HID Escape key code._ 
```C++
#define SYN_USB_HID_KEY_ESCAPE `0x29U`
```




<hr>



### define SYN\_USB\_HID\_KEY\_F 

_USB HID Key F code._ 
```C++
#define SYN_USB_HID_KEY_F `0x09U`
```




<hr>



### define SYN\_USB\_HID\_KEY\_G 

_USB HID Key G code._ 
```C++
#define SYN_USB_HID_KEY_G `0x0AU`
```




<hr>



### define SYN\_USB\_HID\_KEY\_H 

_USB HID Key H code._ 
```C++
#define SYN_USB_HID_KEY_H `0x0BU`
```




<hr>



### define SYN\_USB\_HID\_KEY\_I 

_USB HID Key I code._ 
```C++
#define SYN_USB_HID_KEY_I `0x0CU`
```




<hr>



### define SYN\_USB\_HID\_KEY\_J 

_USB HID Key J code._ 
```C++
#define SYN_USB_HID_KEY_J `0x0DU`
```




<hr>



### define SYN\_USB\_HID\_KEY\_K 

_USB HID Key K code._ 
```C++
#define SYN_USB_HID_KEY_K `0x0EU`
```




<hr>



### define SYN\_USB\_HID\_KEY\_L 

_USB HID Key L code._ 
```C++
#define SYN_USB_HID_KEY_L `0x0FU`
```




<hr>



### define SYN\_USB\_HID\_KEY\_M 

_USB HID Key M code._ 
```C++
#define SYN_USB_HID_KEY_M `0x10U`
```




<hr>



### define SYN\_USB\_HID\_KEY\_N 

_USB HID Key N code._ 
```C++
#define SYN_USB_HID_KEY_N `0x11U`
```




<hr>



### define SYN\_USB\_HID\_KEY\_NONE 

_USB HID No key pressed code._ 
```C++
#define SYN_USB_HID_KEY_NONE `0x00U`
```



Standard USB HID Keyboard Keycodes 


        

<hr>



### define SYN\_USB\_HID\_KEY\_O 

_USB HID Key O code._ 
```C++
#define SYN_USB_HID_KEY_O `0x12U`
```




<hr>



### define SYN\_USB\_HID\_KEY\_P 

_USB HID Key P code._ 
```C++
#define SYN_USB_HID_KEY_P `0x13U`
```




<hr>



### define SYN\_USB\_HID\_KEY\_Q 

_USB HID Key Q code._ 
```C++
#define SYN_USB_HID_KEY_Q `0x14U`
```




<hr>



### define SYN\_USB\_HID\_KEY\_R 

_USB HID Key R code._ 
```C++
#define SYN_USB_HID_KEY_R `0x15U`
```




<hr>



### define SYN\_USB\_HID\_KEY\_S 

_USB HID Key S code._ 
```C++
#define SYN_USB_HID_KEY_S `0x16U`
```




<hr>



### define SYN\_USB\_HID\_KEY\_SPACE 

_USB HID Space key code._ 
```C++
#define SYN_USB_HID_KEY_SPACE `0x2CU`
```




<hr>



### define SYN\_USB\_HID\_KEY\_T 

_USB HID Key T code._ 
```C++
#define SYN_USB_HID_KEY_T `0x17U`
```




<hr>



### define SYN\_USB\_HID\_KEY\_TAB 

_USB HID Tab key code._ 
```C++
#define SYN_USB_HID_KEY_TAB `0x2BU`
```




<hr>



### define SYN\_USB\_HID\_KEY\_U 

_USB HID Key U code._ 
```C++
#define SYN_USB_HID_KEY_U `0x18U`
```




<hr>



### define SYN\_USB\_HID\_KEY\_V 

_USB HID Key V code._ 
```C++
#define SYN_USB_HID_KEY_V `0x19U`
```




<hr>



### define SYN\_USB\_HID\_KEY\_W 

_USB HID Key W code._ 
```C++
#define SYN_USB_HID_KEY_W `0x1AU`
```




<hr>



### define SYN\_USB\_HID\_KEY\_X 

_USB HID Key X code._ 
```C++
#define SYN_USB_HID_KEY_X `0x1BU`
```




<hr>



### define SYN\_USB\_HID\_KEY\_Y 

_USB HID Key Y code._ 
```C++
#define SYN_USB_HID_KEY_Y `0x1CU`
```




<hr>



### define SYN\_USB\_HID\_KEY\_Z 

_USB HID Key Z code._ 
```C++
#define SYN_USB_HID_KEY_Z `0x1DU`
```




<hr>



### define SYN\_USB\_HID\_MOD\_LALT 

_Left Alt modifier bitmask._ 
```C++
#define SYN_USB_HID_MOD_LALT `(1U << 2)`
```




<hr>



### define SYN\_USB\_HID\_MOD\_LCTRL 

_Left Ctrl modifier bitmask._ 
```C++
#define SYN_USB_HID_MOD_LCTRL `(1U << 0)`
```



USB HID Keyboard Modifier Bitmasks per USB HID Usage Tables §10 


        

<hr>



### define SYN\_USB\_HID\_MOD\_LGUI 

_Left GUI modifier bitmask._ 
```C++
#define SYN_USB_HID_MOD_LGUI `(1U << 3)`
```




<hr>



### define SYN\_USB\_HID\_MOD\_LSHIFT 

_Left Shift modifier bitmask._ 
```C++
#define SYN_USB_HID_MOD_LSHIFT `(1U << 1)`
```




<hr>



### define SYN\_USB\_HID\_MOD\_RALT 

_Right Alt modifier bitmask._ 
```C++
#define SYN_USB_HID_MOD_RALT `(1U << 6)`
```




<hr>



### define SYN\_USB\_HID\_MOD\_RCTRL 

_Right Ctrl modifier bitmask._ 
```C++
#define SYN_USB_HID_MOD_RCTRL `(1U << 4)`
```




<hr>



### define SYN\_USB\_HID\_MOD\_RGUI 

_Right GUI modifier bitmask._ 
```C++
#define SYN_USB_HID_MOD_RGUI `(1U << 7)`
```




<hr>



### define SYN\_USB\_HID\_MOD\_RSHIFT 

_Right Shift modifier bitmask._ 
```C++
#define SYN_USB_HID_MOD_RSHIFT `(1U << 5)`
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_usb_hid_keyboard.h`

