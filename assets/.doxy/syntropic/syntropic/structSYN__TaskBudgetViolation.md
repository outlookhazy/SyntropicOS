

# Struct SYN\_TaskBudgetViolation



[**ClassList**](annotated.md) **>** [**SYN\_TaskBudgetViolation**](structSYN__TaskBudgetViolation.md)



_Cooperative chunk latency budget violation record._ 

* `#include <syn_task_profile.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint32\_t | [**budget\_us**](#variable-budget_us)  <br> |
|  uint32\_t | [**max\_yield\_us**](#variable-max_yield_us)  <br> |
|  uint8\_t | [**peer\_priority**](#variable-peer_priority)  <br> |
|  size\_t | [**task\_idx**](#variable-task_idx)  <br> |
|  const char \* | [**task\_name**](#variable-task_name)  <br> |
|  uint8\_t | [**task\_priority**](#variable-task_priority)  <br> |












































## Public Attributes Documentation




### variable budget\_us 

```C++
uint32_t SYN_TaskBudgetViolation::budget_us;
```



Allowed chunk budget from peer max\_wait\_ms 


        

<hr>



### variable max\_yield\_us 

```C++
uint32_t SYN_TaskBudgetViolation::max_yield_us;
```



Observed peak continuous execution (us) 


        

<hr>



### variable peer\_priority 

```C++
uint8_t SYN_TaskBudgetViolation::peer_priority;
```



Priority of tightest constraining peer 


        

<hr>



### variable task\_idx 

```C++
size_t SYN_TaskBudgetViolation::task_idx;
```



Index of offending task in scheduler 


        

<hr>



### variable task\_name 

```C++
const char* SYN_TaskBudgetViolation::task_name;
```



Name of offending task 


        

<hr>



### variable task\_priority 

```C++
uint8_t SYN_TaskBudgetViolation::task_priority;
```



Priority of offending task 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/debug/syn_task_profile.h`

