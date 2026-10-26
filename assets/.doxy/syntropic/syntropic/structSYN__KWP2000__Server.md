

# Struct SYN\_KWP2000\_Server



[**ClassList**](annotated.md) **>** [**SYN\_KWP2000\_Server**](structSYN__KWP2000__Server.md)



_KWP2000 Server instance state._ 

* `#include <syn_kwp2000.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint16\_t | [**cid\_count**](#variable-cid_count)  <br> |
|  [**SYN\_KWP2000\_CID**](structSYN__KWP2000__CID.md) | [**cids**](#variable-cids)  <br> |
|  [**SYN\_KWP2000\_Session**](syn__kwp2000_8h.md#typedef-syn_kwp2000_session) | [**current\_session**](#variable-current_session)  <br> |
|  uint32\_t | [**download\_addr**](#variable-download_addr)  <br> |
|  uint32\_t | [**download\_received**](#variable-download_received)  <br> |
|  uint32\_t | [**download\_size**](#variable-download_size)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**is\_downloading**](#variable-is_downloading)  <br> |
|  [**SYN\_KWP2000\_KeyHandler**](syn__kwp2000_8h.md#typedef-syn_kwp2000_keyhandler) | [**key\_cb**](#variable-key_cb)  <br> |
|  uint16\_t | [**lid\_count**](#variable-lid_count)  <br> |
|  [**SYN\_KWP2000\_LID**](structSYN__KWP2000__LID.md) | [**lids**](#variable-lids)  <br> |
|  [**SYN\_KWP2000\_MemoryHandler**](syn__kwp2000_8h.md#typedef-syn_kwp2000_memoryhandler) | [**memory\_cb**](#variable-memory_cb)  <br> |
|  void \* | [**memory\_ctx**](#variable-memory_ctx)  <br> |
|  uint8\_t | [**pending\_reset\_type**](#variable-pending_reset_type)  <br> |
|  [**SYN\_KWP2000\_ResetHandler**](syn__kwp2000_8h.md#typedef-syn_kwp2000_resethandler) | [**reset\_cb**](#variable-reset_cb)  <br> |
|  void \* | [**reset\_ctx**](#variable-reset_ctx)  <br> |
|  uint32\_t | [**reset\_wait\_ms**](#variable-reset_wait_ms)  <br> |
|  [**SYN\_KWP2000\_RoutineHandler**](syn__kwp2000_8h.md#typedef-syn_kwp2000_routinehandler) | [**routine\_cb**](#variable-routine_cb)  <br> |
|  void \* | [**routine\_ctx**](#variable-routine_ctx)  <br> |
|  uint32\_t | [**s3\_timer\_ms**](#variable-s3_timer_ms)  <br> |
|  void \* | [**security\_ctx**](#variable-security_ctx)  <br> |
|  uint8\_t | [**security\_level**](#variable-security_level)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**security\_unlocked**](#variable-security_unlocked)  <br> |
|  [**SYN\_KWP2000\_SeedHandler**](syn__kwp2000_8h.md#typedef-syn_kwp2000_seedhandler) | [**seed\_cb**](#variable-seed_cb)  <br> |
|  [**SYN\_KWP2000\_SessionHandler**](syn__kwp2000_8h.md#typedef-syn_kwp2000_sessionhandler) | [**session\_cb**](#variable-session_cb)  <br> |
|  void \* | [**session\_ctx**](#variable-session_ctx)  <br> |












































## Public Attributes Documentation




### variable cid\_count 

```C++
uint16_t SYN_KWP2000_Server::cid_count;
```



Active CID count 
 


        

<hr>



### variable cids 

```C++
SYN_KWP2000_CID SYN_KWP2000_Server::cids[SYN_KWP2000_MAX_CIDS];
```



Registered CIDs 
 


        

<hr>



### variable current\_session 

```C++
SYN_KWP2000_Session SYN_KWP2000_Server::current_session;
```



Current diagnostic session 
 


        

<hr>



### variable download\_addr 

```C++
uint32_t SYN_KWP2000_Server::download_addr;
```



Target download address 
 


        

<hr>



### variable download\_received 

```C++
uint32_t SYN_KWP2000_Server::download_received;
```



Bytes received so far 
 


        

<hr>



### variable download\_size 

```C++
uint32_t SYN_KWP2000_Server::download_size;
```



Expected total transfer size 
 


        

<hr>



### variable is\_downloading 

```C++
bool SYN_KWP2000_Server::is_downloading;
```



Is RequestDownload active? 
 


        

<hr>



### variable key\_cb 

```C++
SYN_KWP2000_KeyHandler SYN_KWP2000_Server::key_cb;
```



Security key handler 
 


        

<hr>



### variable lid\_count 

```C++
uint16_t SYN_KWP2000_Server::lid_count;
```



Active LID count 
 


        

<hr>



### variable lids 

```C++
SYN_KWP2000_LID SYN_KWP2000_Server::lids[SYN_KWP2000_MAX_LIDS];
```



Registered LIDs 
 


        

<hr>



### variable memory\_cb 

```C++
SYN_KWP2000_MemoryHandler SYN_KWP2000_Server::memory_cb;
```



Memory access handler 
 


        

<hr>



### variable memory\_ctx 

```C++
void* SYN_KWP2000_Server::memory_ctx;
```



Memory context 
 


        

<hr>



### variable pending\_reset\_type 

```C++
uint8_t SYN_KWP2000_Server::pending_reset_type;
```



Pending reset type (0 if none) 
 


        

<hr>



### variable reset\_cb 

```C++
SYN_KWP2000_ResetHandler SYN_KWP2000_Server::reset_cb;
```



Reset handler 
 


        

<hr>



### variable reset\_ctx 

```C++
void* SYN_KWP2000_Server::reset_ctx;
```



Reset context 
 


        

<hr>



### variable reset\_wait\_ms 

```C++
uint32_t SYN_KWP2000_Server::reset_wait_ms;
```



Delay before executing reset 
 


        

<hr>



### variable routine\_cb 

```C++
SYN_KWP2000_RoutineHandler SYN_KWP2000_Server::routine_cb;
```



Routine control handler 
 


        

<hr>



### variable routine\_ctx 

```C++
void* SYN_KWP2000_Server::routine_ctx;
```



Routine context 
 


        

<hr>



### variable s3\_timer\_ms 

```C++
uint32_t SYN_KWP2000_Server::s3_timer_ms;
```



S3 session keep-alive timer ms 
 


        

<hr>



### variable security\_ctx 

```C++
void* SYN_KWP2000_Server::security_ctx;
```



Security context 
 


        

<hr>



### variable security\_level 

```C++
uint8_t SYN_KWP2000_Server::security_level;
```



Current active security level 
 


        

<hr>



### variable security\_unlocked 

```C++
bool SYN_KWP2000_Server::security_unlocked;
```



Is security access unlocked? 
 


        

<hr>



### variable seed\_cb 

```C++
SYN_KWP2000_SeedHandler SYN_KWP2000_Server::seed_cb;
```



Security seed handler 
 


        

<hr>



### variable session\_cb 

```C++
SYN_KWP2000_SessionHandler SYN_KWP2000_Server::session_cb;
```



Session transition handler 
 


        

<hr>



### variable session\_ctx 

```C++
void* SYN_KWP2000_Server::session_ctx;
```



Session context 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_kwp2000.h`

