

# Struct SYN\_WorkItem



[**ClassList**](annotated.md) **>** [**SYN\_WorkItem**](structSYN__WorkItem.md)



[More...](#detailed-description)

* `#include <syn_workqueue.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  void \* | [**ctx**](#variable-ctx)  <br> |
|  [**SYN\_WorkFunc**](syn__workqueue_8h.md#typedef-syn_workfunc) | [**func**](#variable-func)  <br> |












































## Detailed Description


Single work item. 


    
## Public Attributes Documentation




### variable ctx 

```C++
void* SYN_WorkItem::ctx;
```



User context pointer for the callback 


        

<hr>



### variable func 

```C++
SYN_WorkFunc SYN_WorkItem::func;
```



Deferred callback function pointer 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/sched/syn_workqueue.h`

