

# Struct SYN\_Mailbox



[**ClassList**](annotated.md) **>** [**SYN\_Mailbox**](structSYN__Mailbox.md)



_Typed message queue — fixed-size SPSC ring buffer._ 

* `#include <syn_mailbox.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t \* | [**buf**](#variable-buf)  <br> |
|  size\_t | [**capacity**](#variable-capacity)  <br> |
|  volatile size\_t | [**head**](#variable-head)  <br> |
|  size\_t | [**msg\_size**](#variable-msg_size)  <br> |
|  uint32\_t | [**overflow**](#variable-overflow)  <br> |
|  volatile size\_t | [**tail**](#variable-tail)  <br> |












































## Public Attributes Documentation




### variable buf 

```C++
uint8_t* SYN_Mailbox::buf;
```



Message storage (flat byte array) 
 


        

<hr>



### variable capacity 

```C++
size_t SYN_Mailbox::capacity;
```



Max messages (slots) 
 


        

<hr>



### variable head 

```C++
volatile size_t SYN_Mailbox::head;
```



Write index (producer) 
 


        

<hr>



### variable msg\_size 

```C++
size_t SYN_Mailbox::msg_size;
```



Size of each message in bytes 
 


        

<hr>



### variable overflow 

```C++
uint32_t SYN_Mailbox::overflow;
```



Messages dropped (queue full) 
 


        

<hr>



### variable tail 

```C++
volatile size_t SYN_Mailbox::tail;
```



Read index (consumer) 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/sched/syn_mailbox.h`

