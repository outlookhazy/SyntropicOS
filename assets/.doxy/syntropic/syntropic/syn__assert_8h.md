

# File syn\_assert.h



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_assert.h**](syn__assert_8h.md)

[Go to the source code of this file](syn__assert_8h_source.md)

_Configurable assertion macro for SyntropicOS._ [More...](#detailed-description)

* `#include "../common/syn_compiler.h"`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**SYN\_NORETURN**](syn__compiler_8h.md#define-syn_noreturn) void | [**syn\_assert\_failed**](#function-syn_assert_failed) (const char \* file, int line) <br>_Called when an assertion fails._  |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_ASSERT**](syn__assert_8h.md#define-syn_assert) (expr) `/* multi line expression */`<br>_Assert that_ `expr` _is true. If false, call_[_**syn\_assert\_failed()**_](syn__assert_8h.md#function-syn_assert_failed) _._ |

## Detailed Description


[**SYN\_ASSERT(expr)**](syn__assert_8h.md#define-syn_assert) evaluates `expr` and, if false, calls [**syn\_assert\_failed()**](syn__assert_8h.md#function-syn_assert_failed) with the file name and line number. The user can override [**syn\_assert\_failed()**](syn__assert_8h.md#function-syn_assert_failed) to implement custom behavior (e.g., log to UART, blink an LED, enter debugger).


Define SYN\_DISABLE\_ASSERT before including this header (or in syn\_config.h) to compile out all assertions for release builds. 


    
## Public Functions Documentation




### function syn\_assert\_failed 

_Called when an assertion fails._ 
```C++
SYN_NORETURN void syn_assert_failed (
    const char * file,
    int line
) 
```



This function is declared weak so you can provide your own implementation. The default (in [**syn\_port\_stubs.c**](syn__port__stubs_8c.md)) enters an infinite loop.




**Parameters:**


* `file` Source file name where the assertion failed. 
* `line` Line number where the assertion failed. 




        

<hr>
## Macro Definition Documentation





### define SYN\_ASSERT 

_Assert that_ `expr` _is true. If false, call_[_**syn\_assert\_failed()**_](syn__assert_8h.md#function-syn_assert_failed) _._
```C++
#define SYN_ASSERT (
    expr
) `/* multi line expression */`
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/util/syn_assert.h`

