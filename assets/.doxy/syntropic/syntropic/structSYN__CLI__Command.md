

# Struct SYN\_CLI\_Command



[**ClassList**](annotated.md) **>** [**SYN\_CLI\_Command**](structSYN__CLI__Command.md)



_Command descriptor._ [More...](#detailed-description)

* `#include <syn_cli.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**SYN\_CLI\_Handler**](syn__cli_8h.md#typedef-syn_cli_handler) | [**handler**](#variable-handler)  <br> |
|  const char \* | [**help**](#variable-help)  <br> |
|  const char \* | [**name**](#variable-name)  <br> |












































## Detailed Description


Typically defined as a const static array. 


    
## Public Attributes Documentation




### variable handler 

```C++
SYN_CLI_Handler SYN_CLI_Command::handler;
```



Function called when command matches 
 


        

<hr>



### variable help 

```C++
const char* SYN_CLI_Command::help;
```



Help text shown by the `help` command 
 


        

<hr>



### variable name 

```C++
const char* SYN_CLI_Command::name;
```



Command name (matched against input) 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/cli/syn_cli.h`

