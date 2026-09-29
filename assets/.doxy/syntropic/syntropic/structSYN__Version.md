

# Struct SYN\_Version



[**ClassList**](annotated.md) **>** [**SYN\_Version**](structSYN__Version.md)



_Compile-time version information._ 

* `#include <syn_version.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  const char \* | [**app\_name**](#variable-app_name)  <br> |
|  uint32\_t | [**code**](#variable-code)  <br> |
|  const char \* | [**date**](#variable-date)  <br> |
|  const char \* | [**git\_hash**](#variable-git_hash)  <br> |
|  uint8\_t | [**month**](#variable-month)  <br> |
|  uint8\_t | [**release**](#variable-release)  <br> |
|  const char \* | [**time**](#variable-time)  <br> |
|  uint16\_t | [**year**](#variable-year)  <br> |












































## Public Attributes Documentation




### variable app\_name 

```C++
const char* SYN_Version::app_name;
```



Application name 
 


        

<hr>



### variable code 

```C++
uint32_t SYN_Version::code;
```



Packed SYN\_VERSION\_CODE 
 


        

<hr>



### variable date 

```C++
const char* SYN_Version::date;
```



Build date 
 


        

<hr>



### variable git\_hash 

```C++
const char* SYN_Version::git_hash;
```



Git short hash 
 


        

<hr>



### variable month 

```C++
uint8_t SYN_Version::month;
```



Release month (CalVer) 
 


        

<hr>



### variable release 

```C++
uint8_t SYN_Version::release;
```



Release number within the month 
 


        

<hr>



### variable time 

```C++
const char* SYN_Version::time;
```



Build time 
 


        

<hr>



### variable year 

```C++
uint16_t SYN_Version::year;
```



Release year (CalVer) 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/system/syn_version.h`

