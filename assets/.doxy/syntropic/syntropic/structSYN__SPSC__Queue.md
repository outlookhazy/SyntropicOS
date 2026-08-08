

# Struct SYN\_SPSC\_Queue



[**ClassList**](annotated.md) **>** [**SYN\_SPSC\_Queue**](structSYN__SPSC__Queue.md)



_SPSC Queue Control Structure._ 

* `#include <syn_spsc_queue.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint8\_t \* | [**buffer**](#variable-buffer)  <br> |
|  size\_t | [**capacity**](#variable-capacity)  <br> |
|  size\_t | [**elem\_size**](#variable-elem_size)  <br> |
|  volatile size\_t | [**head**](#variable-head)  <br> |
|  volatile size\_t | [**tail**](#variable-tail)  <br> |












































## Public Attributes Documentation




### variable buffer 

```C++
uint8_t* SYN_SPSC_Queue::buffer;
```



Pointer to caller-provided element buffer 


        

<hr>



### variable capacity 

```C++
size_t SYN_SPSC_Queue::capacity;
```



Maximum element capacity 


        

<hr>



### variable elem\_size 

```C++
size_t SYN_SPSC_Queue::elem_size;
```



Size of each element in bytes 


        

<hr>



### variable head 

```C++
volatile size_t SYN_SPSC_Queue::head;
```



Write index (Producer modifies) 


        

<hr>



### variable tail 

```C++
volatile size_t SYN_SPSC_Queue::tail;
```



Read index (Consumer modifies) 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/util/syn_spsc_queue.h`

