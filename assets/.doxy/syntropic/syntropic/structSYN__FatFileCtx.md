

# Struct SYN\_FatFileCtx



[**ClassList**](annotated.md) **>** [**SYN\_FatFileCtx**](structSYN__FatFileCtx.md)



_Open file tracking context (pooled)._ 






















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint32\_t | [**dir\_offset**](#variable-dir_offset)  <br> |
|  uint32\_t | [**dir\_sector**](#variable-dir_sector)  <br> |
|  int | [**mode**](#variable-mode)  <br> |
|  char | [**name**](#variable-name)  <br> |
|  uint32\_t | [**offset**](#variable-offset)  <br> |
|  uint32\_t | [**size**](#variable-size)  <br> |
|  uint32\_t | [**start\_cluster**](#variable-start_cluster)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**used**](#variable-used)  <br> |












































## Public Attributes Documentation




### variable dir\_offset 

```C++
uint32_t SYN_FatFileCtx::dir_offset;
```



Byte offset of dir entry within sector 
 


        

<hr>



### variable dir\_sector 

```C++
uint32_t SYN_FatFileCtx::dir_sector;
```



Sector containing the dir entry 
 


        

<hr>



### variable mode 

```C++
int SYN_FatFileCtx::mode;
```



Open mode (read, write, etc.) 
 


        

<hr>



### variable name 

```C++
char SYN_FatFileCtx::name[11];
```



8.3 FAT directory name 
 


        

<hr>



### variable offset 

```C++
uint32_t SYN_FatFileCtx::offset;
```



Current read/write position 
 


        

<hr>



### variable size 

```C++
uint32_t SYN_FatFileCtx::size;
```



File size in bytes 
 


        

<hr>



### variable start\_cluster 

```C++
uint32_t SYN_FatFileCtx::start_cluster;
```



First cluster of file data 
 


        

<hr>



### variable used 

```C++
bool SYN_FatFileCtx::used;
```



true if this slot is in use 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/storage/syn_fat.c`

