

# Struct SYN\_LwM2M\_ReqMeta



[**ClassList**](annotated.md) **>** [**SYN\_LwM2M\_ReqMeta**](structSYN__LwM2M__ReqMeta.md)



_Parsed CoAP request metadata for LwM2M routing and payload decoding._ 






















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint16\_t | [**content\_format**](#variable-content_format)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**has\_content\_format**](#variable-has_content_format)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**has\_observe**](#variable-has_observe)  <br> |
|  uint16\_t | [**inst\_id**](#variable-inst_id)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**invalid\_uri**](#variable-invalid_uri)  <br> |
|  uint16\_t | [**obj\_id**](#variable-obj_id)  <br> |
|  uint32\_t | [**observe\_val**](#variable-observe_val)  <br> |
|  size\_t | [**path\_segs**](#variable-path_segs)  <br> |
|  uint16\_t | [**res\_id**](#variable-res_id)  <br> |












































## Public Attributes Documentation




### variable content\_format 

```C++
uint16_t SYN_LwM2M_ReqMeta::content_format;
```



CoAP Content-Format identifier 


        

<hr>



### variable has\_content\_format 

```C++
bool SYN_LwM2M_ReqMeta::has_content_format;
```



Content-Format option present 


        

<hr>



### variable has\_observe 

```C++
bool SYN_LwM2M_ReqMeta::has_observe;
```



Observe option present 


        

<hr>



### variable inst\_id 

```C++
uint16_t SYN_LwM2M_ReqMeta::inst_id;
```



Target Instance ID 


        

<hr>



### variable invalid\_uri 

```C++
bool SYN_LwM2M_ReqMeta::invalid_uri;
```



True when any Uri-Path segment is malformed 


        

<hr>



### variable obj\_id 

```C++
uint16_t SYN_LwM2M_ReqMeta::obj_id;
```



Target Object ID 


        

<hr>



### variable observe\_val 

```C++
uint32_t SYN_LwM2M_ReqMeta::observe_val;
```



Decoded Observe option value (0=register, 1=deregister) 


        

<hr>



### variable path\_segs 

```C++
size_t SYN_LwM2M_ReqMeta::path_segs;
```



Number of Uri-Path segments 


        

<hr>



### variable res\_id 

```C++
uint16_t SYN_LwM2M_ReqMeta::res_id;
```



Target Resource ID 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_lwm2m.c`

