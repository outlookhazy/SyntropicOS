

# Struct DirEntryLoc



[**ClassList**](annotated.md) **>** [**DirEntryLoc**](structDirEntryLoc.md)



_Result of a directory scan — location of a matching entry._ 






















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint32\_t | [**file\_size**](#variable-file_size)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**found**](#variable-found)  <br> |
|  uint32\_t | [**offset**](#variable-offset)  <br> |
|  uint32\_t | [**sector**](#variable-sector)  <br> |
|  uint32\_t | [**start\_cluster**](#variable-start_cluster)  <br> |












































## Public Attributes Documentation




### variable file\_size 

```C++
uint32_t DirEntryLoc::file_size;
```



Size in bytes of the found file 
 


        

<hr>



### variable found 

```C++
bool DirEntryLoc::found;
```



true if a matching entry was found 
 


        

<hr>



### variable offset 

```C++
uint32_t DirEntryLoc::offset;
```



Byte offset within that sector 
 


        

<hr>



### variable sector 

```C++
uint32_t DirEntryLoc::sector;
```



Sector containing the entry 
 


        

<hr>



### variable start\_cluster 

```C++
uint32_t DirEntryLoc::start_cluster;
```



First cluster of the found file 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/storage/syn_fat.c`

