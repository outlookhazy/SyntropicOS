

# Struct EXTI\_Entry



[**ClassList**](annotated.md) **>** [**EXTI\_Entry**](structEXTI__Entry.md)



_EXTI callback table entry._ 






















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**active**](#variable-active)  <br> |
|  [**SYN\_EXTI\_Callback**](syn__exti_8h.md#typedef-syn_exti_callback) | [**cb**](#variable-cb)  <br> |
|  void \* | [**ctx**](#variable-ctx)  <br> |
|  [**SYN\_GPIO\_Pin**](syn__defs_8h.md#typedef-syn_gpio_pin) | [**pin**](#variable-pin)  <br> |












































## Public Attributes Documentation




### variable active 

```C++
bool EXTI_Entry::active;
```



true if interrupt is enabled 
 


        

<hr>



### variable cb 

```C++
SYN_EXTI_Callback EXTI_Entry::cb;
```



User callback 
 


        

<hr>



### variable ctx 

```C++
void* EXTI_Entry::ctx;
```



User context 
 


        

<hr>



### variable pin 

```C++
SYN_GPIO_Pin EXTI_Entry::pin;
```



Registered pin 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/drivers/syn_exti.c`

