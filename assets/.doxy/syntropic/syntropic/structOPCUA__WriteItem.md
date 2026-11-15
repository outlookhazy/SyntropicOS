

# Struct OPCUA\_WriteItem



[**ClassList**](annotated.md) **>** [**OPCUA\_WriteItem**](structOPCUA__WriteItem.md)



_Decoded single WriteValue entry from a WriteRequest._ 






















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint32\_t | [**attr\_id**](#variable-attr_id)  <br> |
|  uint8\_t | [**dv\_mask**](#variable-dv_mask)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**has\_index\_range**](#variable-has_index_range)  <br> |
|  uint16\_t | [**ns**](#variable-ns)  <br> |
|  uint32\_t | [**num**](#variable-num)  <br> |
|  uint64\_t | [**source\_ts\_ms**](#variable-source_ts_ms)  <br> |
|  uint32\_t | [**status\_code**](#variable-status_code)  <br> |
|  char | [**str\_buf**](#variable-str_buf)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**str\_is\_null**](#variable-str_is_null)  <br> |
|  [**SYN\_OPCUA\_Variant**](structSYN__OPCUA__Variant.md) | [**var**](#variable-var)  <br> |












































## Public Attributes Documentation




### variable attr\_id 

```C++
uint32_t OPCUA_WriteItem::attr_id;
```



Target AttributeId 


        

<hr>



### variable dv\_mask 

```C++
uint8_t OPCUA_WriteItem::dv_mask;
```



DataValue encoding mask 


        

<hr>



### variable has\_index\_range 

```C++
bool OPCUA_WriteItem::has_index_range;
```



Non-empty IndexRange present 


        

<hr>



### variable ns 

```C++
uint16_t OPCUA_WriteItem::ns;
```



Target namespace index 


        

<hr>



### variable num 

```C++
uint32_t OPCUA_WriteItem::num;
```



Target numeric identifier 


        

<hr>



### variable source\_ts\_ms 

```C++
uint64_t OPCUA_WriteItem::source_ts_ms;
```



Optional DataValue sourceTimestamp 


        

<hr>



### variable status\_code 

```C++
uint32_t OPCUA_WriteItem::status_code;
```



Optional DataValue statusCode 


        

<hr>



### variable str\_buf 

```C++
char OPCUA_WriteItem::str_buf[SYN_OPCUA_MAX_STRING_LEN];
```



Decoded string buffer 


        

<hr>



### variable str\_is\_null 

```C++
bool OPCUA_WriteItem::str_is_null;
```



True if decoded string was null (-1) 


        

<hr>



### variable var 

```C++
SYN_OPCUA_Variant OPCUA_WriteItem::var;
```



Decoded Variant value 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/proto/syn_opcua.c`

