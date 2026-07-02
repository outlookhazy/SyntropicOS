

# Struct SYN\_CLI



[**ClassList**](annotated.md) **>** [**SYN\_CLI**](structSYN__CLI.md)



_CLI instance — command table, line buffer, I/O, and history._ 

* `#include <syn_cli.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  size\_t | [**command\_count**](#variable-command_count)  <br> |
|  const [**SYN\_CLI\_Command**](structSYN__CLI__Command.md) \* | [**commands**](#variable-commands)  <br> |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**echo**](#variable-echo)  <br> |
|  uint8\_t | [**escape\_state**](#variable-escape_state)  <br> |
|  char | [**line\_buf**](#variable-line_buf)  <br> |
|  size\_t | [**line\_pos**](#variable-line_pos)  <br> |
|  const char \* | [**prompt**](#variable-prompt)  <br> |
|  [**SYN\_CLI\_PutChar**](syn__cli_8h.md#typedef-syn_cli_putchar) | [**putchar\_fn**](#variable-putchar_fn)  <br> |
|  [**SYN\_CLI\_Puts**](syn__cli_8h.md#typedef-syn_cli_puts) | [**puts\_fn**](#variable-puts_fn)  <br> |












































## Public Attributes Documentation




### variable command\_count 

```C++
size_t SYN_CLI::command_count;
```



Number of commands 
 


        

<hr>



### variable commands 

```C++
const SYN_CLI_Command* SYN_CLI::commands;
```



Registered command array 
 


        

<hr>



### variable echo 

```C++
bool SYN_CLI::echo;
```



Echo enabled 
 


        

<hr>



### variable escape\_state 

```C++
uint8_t SYN_CLI::escape_state;
```



ANSI escape sequence state 
 


        

<hr>



### variable line\_buf 

```C++
char SYN_CLI::line_buf[SYN_CLI_LINE_BUF_SIZE];
```



Input line buffer 
 


        

<hr>



### variable line\_pos 

```C++
size_t SYN_CLI::line_pos;
```



Current cursor position 
 


        

<hr>



### variable prompt 

```C++
const char* SYN_CLI::prompt;
```



Prompt string 
 


        

<hr>



### variable putchar\_fn 

```C++
SYN_CLI_PutChar SYN_CLI::putchar_fn;
```



Single-char output function 


        

<hr>



### variable puts\_fn 

```C++
SYN_CLI_Puts SYN_CLI::puts_fn;
```



String output function 
 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/cli/syn_cli.h`

