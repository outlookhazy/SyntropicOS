

# File startup\_ch32v307.c



[**FileList**](files.md) **>** [**ch32v307**](dir_876381587f01eae594c70ce97acc1e04.md) **>** [**startup\_ch32v307.c**](startup__ch32v307_8c.md)

[Go to the source code of this file](startup__ch32v307_8c_source.md)

_Bare-metal startup code and vector table for WCH CH32V307VCT6 (QingKe V4F RISC-V)._ 

* `#include <stdint.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint32\_t | [**\_ebss**](#variable-_ebss)  <br>_End address of uninitialized data section in RAM._  |
|  uint32\_t | [**\_edata**](#variable-_edata)  <br>_End address of initialized data section in RAM._  |
|  uint32\_t | [**\_estack**](#variable-_estack)  <br>_Initial top of stack address in RAM._  |
|  uint32\_t | [**\_sbss**](#variable-_sbss)  <br>_Start address of uninitialized data section in RAM._  |
|  uint32\_t | [**\_sdata**](#variable-_sdata)  <br>_Start address of initialized data section in RAM._  |
|  uint32\_t | [**\_sidata**](#variable-_sidata)  <br>_Start address of initialized data section in Flash._  |
















## Public Functions

| Type | Name |
| ---: | :--- |
|  SYN\_ISR\_ATTR void | [**Default\_Handler**](#function-default_handler) (void) <br>_Default unhandled exception/interrupt catch handler._  |
|  SYN\_ISR\_ATTR void | [**NMI\_Handler**](#function-nmi_handler) (void) <br>_Non-Maskable Interrupt (NMI) handler._  |
|  void | [**Reset\_Handler**](#function-reset_handler) (void) <br>_MCU Reset handler — initializes memory sections and jumps to_ [_**main()**_](startup__ch32v307_8c.md#function-main) _._ |
|  int | [**main**](#function-main) (void) <br>_Main application entry point function declaration._  |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_ISR\_ATTR**](startup__ch32v307_8c.md#define-syn_isr_attr)  <br> |

## Public Attributes Documentation




### variable \_ebss 

_End address of uninitialized data section in RAM._ 
```C++
uint32_t _ebss;
```




<hr>



### variable \_edata 

_End address of initialized data section in RAM._ 
```C++
uint32_t _edata;
```




<hr>



### variable \_estack 

_Initial top of stack address in RAM._ 
```C++
uint32_t _estack;
```




<hr>



### variable \_sbss 

_Start address of uninitialized data section in RAM._ 
```C++
uint32_t _sbss;
```




<hr>



### variable \_sdata 

_Start address of initialized data section in RAM._ 
```C++
uint32_t _sdata;
```




<hr>



### variable \_sidata 

_Start address of initialized data section in Flash._ 
```C++
uint32_t _sidata;
```




<hr>
## Public Functions Documentation




### function Default\_Handler 

_Default unhandled exception/interrupt catch handler._ 
```C++
SYN_ISR_ATTR void Default_Handler (
    void
) 
```




<hr>



### function NMI\_Handler 

_Non-Maskable Interrupt (NMI) handler._ 
```C++
SYN_ISR_ATTR void NMI_Handler (
    void
) 
```




<hr>



### function Reset\_Handler 

_MCU Reset handler — initializes memory sections and jumps to_ [_**main()**_](startup__ch32v307_8c.md#function-main) _._
```C++
void Reset_Handler (
    void
) 
```




<hr>



### function main 

_Main application entry point function declaration._ 
```C++
int main (
    void
) 
```





**Returns:**

Exit status code. 





        

<hr>
## Macro Definition Documentation





### define SYN\_ISR\_ATTR 

```C++
#define SYN_ISR_ATTR 
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/port/ch32v307/startup_ch32v307.c`

