

# File syn\_compiler.h



[**FileList**](files.md) **>** [**common**](dir_b4b1f229ee23b400200c904b1998093f.md) **>** [**syn\_compiler.h**](syn__compiler_8h.md)

[Go to the source code of this file](syn__compiler_8h_source.md)

_Compiler-portable macros for SyntropicOS._ [More...](#detailed-description)

































































## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_ALIGN**](syn__compiler_8h.md#define-syn_align) (n) <br> |
| define  | [**SYN\_COMPILER\_BARRIER**](syn__compiler_8h.md#define-syn_compiler_barrier) () `((void)0)`<br> |
| define  | [**SYN\_COMPILER\_UNKNOWN**](syn__compiler_8h.md#define-syn_compiler_unknown)  `1`<br>_Compiler could not be identified._  |
| define  | [**SYN\_FALLTHROUGH**](syn__compiler_8h.md#define-syn_fallthrough)  <br> |
| define  | [**SYN\_INLINE**](syn__compiler_8h.md#define-syn_inline)  `static inline`<br> |
| define  | [**SYN\_NORETURN**](syn__compiler_8h.md#define-syn_noreturn)  <br> |
| define  | [**SYN\_PACKED**](syn__compiler_8h.md#define-syn_packed)  <br> |
| define  | [**SYN\_SECTION**](syn__compiler_8h.md#define-syn_section) (name) <br> |
| define  | [**SYN\_STATIC\_ASSERT**](syn__compiler_8h.md#define-syn_static_assert) (expr, msg) `typedef char [**SYN\_STATIC\_ASSERT\_JOIN**](syn__compiler_8h.md#define-syn_static_assert_join)(syn\_static\_assert\_, \_\_LINE\_\_)[(expr) ? 1 : -1]`<br>_Compile-time assertion (C99 fallback)._  |
| define  | [**SYN\_STATIC\_ASSERT\_JOIN**](syn__compiler_8h.md#define-syn_static_assert_join) (a, b) `[**SYN\_STATIC\_ASSERT\_JOIN2**](syn__compiler_8h.md#define-syn_static_assert_join2)(a, b)`<br>_Token-pasting helper for SYN\_STATIC\_ASSERT._  |
| define  | [**SYN\_STATIC\_ASSERT\_JOIN2**](syn__compiler_8h.md#define-syn_static_assert_join2) (a, b) `a##b`<br> |
| define  | [**SYN\_UNUSED**](syn__compiler_8h.md#define-syn_unused)  <br> |
| define  | [**SYN\_WEAK**](syn__compiler_8h.md#define-syn_weak)  <br> |

## Detailed Description


Abstracts compiler-specific attributes and intrinsics so the rest of the library (and user code) can use a single set of macros across GCC, Clang, ARMCC (Keil), and IAR. 


    
## Macro Definition Documentation





### define SYN\_ALIGN 

```C++
#define SYN_ALIGN (
    n
) 
```



Align a variable to N bytes. 


        

<hr>



### define SYN\_COMPILER\_BARRIER 

```C++
#define SYN_COMPILER_BARRIER (
    
) `((void)0)`
```



Prevent the compiler from reordering loads/stores across this point. Zero runtime cost — does NOT emit a hardware fence instruction. Pair with syn\_port\_memory\_barrier() when cross-core visibility is needed. 


        

<hr>



### define SYN\_COMPILER\_UNKNOWN 

_Compiler could not be identified._ 
```C++
#define SYN_COMPILER_UNKNOWN `1`
```




<hr>



### define SYN\_FALLTHROUGH 

```C++
#define SYN_FALLTHROUGH 
```



Suppress -Wimplicit-fallthrough warnings for intentional fallthrough. 


        

<hr>



### define SYN\_INLINE 

```C++
#define SYN_INLINE `static inline`
```



Portable always-inline hint. 


        

<hr>



### define SYN\_NORETURN 

```C++
#define SYN_NORETURN 
```



Mark a function that never returns (e.g. fault handlers). 


        

<hr>



### define SYN\_PACKED 

```C++
#define SYN_PACKED 
```



Pack a struct (place after the closing brace, before the semicolon). 


        

<hr>



### define SYN\_SECTION 

```C++
#define SYN_SECTION (
    name
) 
```



Place a symbol in a named linker section. 


        

<hr>



### define SYN\_STATIC\_ASSERT 

_Compile-time assertion (C99 fallback)._ 
```C++
#define SYN_STATIC_ASSERT (
    expr,
    msg
) `typedef char SYN_STATIC_ASSERT_JOIN (syn_static_assert_, __LINE__)[(expr) ? 1 : -1]`
```




<hr>



### define SYN\_STATIC\_ASSERT\_JOIN 

_Token-pasting helper for SYN\_STATIC\_ASSERT._ 
```C++
#define SYN_STATIC_ASSERT_JOIN (
    a,
    b
) `SYN_STATIC_ASSERT_JOIN2 (a, b)`
```




<hr>



### define SYN\_STATIC\_ASSERT\_JOIN2 

```C++
#define SYN_STATIC_ASSERT_JOIN2 (
    a,
    b
) `a##b`
```



Compile-time assertion. Uses C11 \_Static\_assert when available. 


        

<hr>



### define SYN\_UNUSED 

```C++
#define SYN_UNUSED 
```



Suppress "unused variable/parameter" warnings. 


        

<hr>



### define SYN\_WEAK 

```C++
#define SYN_WEAK 
```



Mark a function as a weak symbol so the user can override it. 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/common/syn_compiler.h`

