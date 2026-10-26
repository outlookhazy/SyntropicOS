

# Struct SYN\_WASM\_Label



[**ClassList**](annotated.md) **>** [**SYN\_WASM\_Label**](structSYN__WASM__Label.md)



_WebAssembly control flow label stack entry._ 

* `#include <syn_wasm.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t | [**opcode**](#variable-opcode)  <br> |
|  uint32\_t | [**stack\_sp**](#variable-stack_sp)  <br> |
|  uint32\_t | [**target\_pc**](#variable-target_pc)  <br> |












































## Public Attributes Documentation




### variable opcode 

```C++
uint8_t SYN_WASM_Label::opcode;
```



Opcode: 0x02 block, 0x03 loop, 0x04 if 


        

<hr>



### variable stack\_sp 

```C++
uint32_t SYN_WASM_Label::stack_sp;
```



Operand stack pointer on entry 


        

<hr>



### variable target\_pc 

```C++
uint32_t SYN_WASM_Label::target_pc;
```



Target program counter on break 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/vm/syn_wasm.h`

