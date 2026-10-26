

# Struct SYN\_LwM2M\_Object



[**ClassList**](annotated.md) **>** [**SYN\_LwM2M\_Object**](structSYN__LwM2M__Object.md)



_LwM2M Object Definition._ [More...](#detailed-description)

* `#include <syn_lwm2m.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_LwM2M\_ExecCb**](syn__lwm2m_8h.md#typedef-syn_lwm2m_execcb) | [**exec**](#variable-exec)  <br> |
|  uint16\_t | [**id**](#variable-id)  <br> |
|  uint16\_t | [**instance\_count**](#variable-instance_count)  <br> |
|  const uint16\_t \* | [**instances**](#variable-instances)  <br> |
|  [**SYN\_LwM2M\_ReadCb**](syn__lwm2m_8h.md#typedef-syn_lwm2m_readcb) | [**read**](#variable-read)  <br> |
|  size\_t | [**resource\_count**](#variable-resource_count)  <br> |
|  const [**SYN\_LwM2M\_ResourceDesc**](structSYN__LwM2M__ResourceDesc.md) \* | [**resources**](#variable-resources)  <br> |
|  void \* | [**user\_ctx**](#variable-user_ctx)  <br> |
|  [**SYN\_LwM2M\_WriteCb**](syn__lwm2m_8h.md#typedef-syn_lwm2m_writecb) | [**write**](#variable-write)  <br> |












































## Detailed Description


Forward declaration of LwM2M object descriptor. 


    
## Public Attributes Documentation




### variable exec 

```C++
SYN_LwM2M_ExecCb SYN_LwM2M_Object::exec;
```



Execute callback 


        

<hr>



### variable id 

```C++
uint16_t SYN_LwM2M_Object::id;
```



Object ID (e.g. 3 for Device) 


        

<hr>



### variable instance\_count 

```C++
uint16_t SYN_LwM2M_Object::instance_count;
```



Number of instances supported 


        

<hr>



### variable instances 

```C++
const uint16_t* SYN_LwM2M_Object::instances;
```



Array of valid instance IDs 


        

<hr>



### variable read 

```C++
SYN_LwM2M_ReadCb SYN_LwM2M_Object::read;
```



Read callback 


        

<hr>



### variable resource\_count 

```C++
size_t SYN_LwM2M_Object::resource_count;
```



Number of resources in descriptor 


        

<hr>



### variable resources 

```C++
const SYN_LwM2M_ResourceDesc* SYN_LwM2M_Object::resources;
```



Array of resource descriptors 


        

<hr>



### variable user\_ctx 

```C++
void* SYN_LwM2M_Object::user_ctx;
```



User context pointer 


        

<hr>



### variable write 

```C++
SYN_LwM2M_WriteCb SYN_LwM2M_Object::write;
```



Write callback 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_lwm2m.h`

