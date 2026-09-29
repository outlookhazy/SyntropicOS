

# Struct FAT\_Volume



[**ClassList**](annotated.md) **>** [**FAT\_Volume**](structFAT__Volume.md)



_Internal FAT volume metadata parsed from the boot sector._ 






















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint16\_t | [**bytes\_per\_sector**](#variable-bytes_per_sector)  <br> |
|  uint32\_t | [**data\_start\_sector**](#variable-data_start_sector)  <br> |
|  uint16\_t | [**fat16\_root\_entries**](#variable-fat16_root_entries)  <br> |
|  uint32\_t | [**fat\_sectors**](#variable-fat_sectors)  <br> |
|  uint32\_t | [**fat\_start\_sector**](#variable-fat_start_sector)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**is\_fat32**](#variable-is_fat32)  <br> |
|  uint8\_t | [**num\_fats**](#variable-num_fats)  <br> |
|  uint16\_t | [**reserved\_sectors**](#variable-reserved_sectors)  <br> |
|  uint32\_t | [**root\_cluster**](#variable-root_cluster)  <br> |
|  uint32\_t | [**root\_sectors**](#variable-root_sectors)  <br> |
|  uint32\_t | [**root\_start\_sector**](#variable-root_start_sector)  <br> |
|  uint8\_t | [**sectors\_per\_cluster**](#variable-sectors_per_cluster)  <br> |












































## Public Attributes Documentation




### variable bytes\_per\_sector 

```C++
uint16_t FAT_Volume::bytes_per_sector;
```



Bytes per logical sector 
 


        

<hr>



### variable data\_start\_sector 

```C++
uint32_t FAT_Volume::data_start_sector;
```



First sector of data region 
 


        

<hr>



### variable fat16\_root\_entries 

```C++
uint16_t FAT_Volume::fat16_root_entries;
```



Root dir entry count (FAT16 only) 
 


        

<hr>



### variable fat\_sectors 

```C++
uint32_t FAT_Volume::fat_sectors;
```



Sectors occupied by each FAT 
 


        

<hr>



### variable fat\_start\_sector 

```C++
uint32_t FAT_Volume::fat_start_sector;
```



First sector of FAT 
 


        

<hr>



### variable is\_fat32 

```C++
bool FAT_Volume::is_fat32;
```



true if FAT32, false if FAT16 
 


        

<hr>



### variable num\_fats 

```C++
uint8_t FAT_Volume::num_fats;
```



Number of FAT copies 
 


        

<hr>



### variable reserved\_sectors 

```C++
uint16_t FAT_Volume::reserved_sectors;
```



Reserved sectors before first FAT 
 


        

<hr>



### variable root\_cluster 

```C++
uint32_t FAT_Volume::root_cluster;
```



Root directory cluster (FAT32 only) 


        

<hr>



### variable root\_sectors 

```C++
uint32_t FAT_Volume::root_sectors;
```



Sectors for root dir (FAT16) 
 


        

<hr>



### variable root\_start\_sector 

```C++
uint32_t FAT_Volume::root_start_sector;
```



First sector of root dir (FAT16) 
 


        

<hr>



### variable sectors\_per\_cluster 

```C++
uint8_t FAT_Volume::sectors_per_cluster;
```



Sectors per allocation cluster 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/storage/syn_fat.c`

