

# File syn\_version.h



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**system**](dir_e076fcb374a73e741803d1638643d902.md) **>** [**syn\_version.h**](syn__version_8h.md)

[Go to the source code of this file](syn__version_8h_source.md)

_Build version and metadata — compile-time constants._ [More...](#detailed-description)

* `#include <stdint.h>`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_Version**](structSYN__Version.md) <br>_Compile-time version information._  |
























## Public Static Functions

| Type | Name |
| ---: | :--- |
|  const [**SYN\_Version**](structSYN__Version.md) \* | [**syn\_version**](#function-syn_version) (void) <br>_Get the compile-time version info._  |
|  int | [**syn\_version\_at\_least**](#function-syn_version_at_least) (uint16\_t year, uint8\_t month, uint8\_t rel) <br>_Check if running version is at least year.month.release._  |

























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_APP\_NAME**](syn__version_8h.md#define-syn_app_name)  `"SyntropicOS"`<br> |
| define  | [**SYN\_BUILD\_DATE**](syn__version_8h.md#define-syn_build_date)  `\_\_DATE\_\_`<br> |
| define  | [**SYN\_BUILD\_TIME**](syn__version_8h.md#define-syn_build_time)  `\_\_TIME\_\_`<br> |
| define  | [**SYN\_GIT\_HASH**](syn__version_8h.md#define-syn_git_hash)  `"unknown"`<br> |
| define  | [**SYN\_VERSION\_CODE**](syn__version_8h.md#define-syn_version_code)  `/* multi line expression */`<br> |
| define  | [**SYN\_VERSION\_MONTH**](syn__version_8h.md#define-syn_version_month)  `7`<br>_Release month (CalVer)._  |
| define  | [**SYN\_VERSION\_RELEASE**](syn__version_8h.md#define-syn_version_release)  `0`<br>_Release number within the month (CalVer)._  |
| define  | [**SYN\_VERSION\_YEAR**](syn__version_8h.md#define-syn_version_year)  `2026`<br>_Release year (CalVer)._  |

## Detailed Description


Provides version numbers, build date/time, and optional git hash. Users define these in their build system or syn\_config.h.




**
**


```C++
printf("SyntropicOS v%d.%d.%d built %s\n",
       SYN_VERSION_YEAR, SYN_VERSION_MONTH, SYN_VERSION_RELEASE,
       SYN_BUILD_DATE);

// Or use the struct:
const SYN_Version *v = syn_version();
printf("%s\n", v->string);
```
 





    
## Public Static Functions Documentation




### function syn\_version 

_Get the compile-time version info._ 
```C++
static inline const SYN_Version * syn_version (
    void
) 
```





**Returns:**

Pointer to static version struct. 





        

<hr>



### function syn\_version\_at\_least 

_Check if running version is at least year.month.release._ 
```C++
static inline int syn_version_at_least (
    uint16_t year,
    uint8_t month,
    uint8_t rel
) 
```





**Parameters:**


* `year` Required year component. 
* `month` Required month component. 
* `rel` Required release component. 



**Returns:**

Non-zero if the current version meets or exceeds the requirements. 





        

<hr>
## Macro Definition Documentation





### define SYN\_APP\_NAME 

```C++
#define SYN_APP_NAME `"SyntropicOS"`
```



Application name. 


        

<hr>



### define SYN\_BUILD\_DATE 

```C++
#define SYN_BUILD_DATE `__DATE__`
```



Build date string (e.g., "Jun 27 2026"). 


        

<hr>



### define SYN\_BUILD\_TIME 

```C++
#define SYN_BUILD_TIME `__TIME__`
```



Build time string (e.g., "14:30:00"). 


        

<hr>



### define SYN\_GIT\_HASH 

```C++
#define SYN_GIT_HASH `"unknown"`
```



Git hash (short). Override in build system: -DSYN\_GIT\_HASH="abc1234" 


        

<hr>



### define SYN\_VERSION\_CODE 

```C++
#define SYN_VERSION_CODE `/* multi line expression */`
```



Packed version: 0xYYYYMMRR (year.month.release). 


        

<hr>



### define SYN\_VERSION\_MONTH 

_Release month (CalVer)._ 
```C++
#define SYN_VERSION_MONTH `7`
```




<hr>



### define SYN\_VERSION\_RELEASE 

_Release number within the month (CalVer)._ 
```C++
#define SYN_VERSION_RELEASE `0`
```




<hr>



### define SYN\_VERSION\_YEAR 

_Release year (CalVer)._ 
```C++
#define SYN_VERSION_YEAR `2026`
```




<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/system/syn_version.h`

