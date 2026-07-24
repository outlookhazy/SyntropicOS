

# File syn\_cli.c



[**FileList**](files.md) **>** [**cli**](dir_da6bc3c87aefe1086f828540bad8d0dd.md) **>** [**syn\_cli.c**](syn__cli_8c.md)

[Go to the source code of this file](syn__cli_8c_source.md)

_Command-line interpreter implementation._ 

* `#include "syn_cli.h"`
* `#include "../port/syn_port_serial.h"`
* `#include "../util/syn_assert.h"`
* `#include "../util/syn_fmt.h"`
* `#include <string.h>`
* `#include <stdio.h>`
* `#include <stdarg.h>`
* `#include "../system/syn_version.h"`
* `#include "../port/syn_port_system.h"`
* `#include "../system/syn_errlog.h"`
* `#include "../sched/syn_sched.h"`























## Public Static Attributes

| Type | Name |
| ---: | :--- |
|  struct [**SYN\_ErrLog**](structSYN__ErrLog.md) \* | [**s\_cli\_errlog**](#variable-s_cli_errlog)   = `NULL`<br>_Global error log singleton reference for CLI diagnostics._  |
|  struct [**SYN\_Sched**](structSYN__Sched.md) \* | [**s\_cli\_sched**](#variable-s_cli_sched)   = `NULL`<br>_Global scheduler singleton reference for CLI task listing._  |














## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_cli\_init**](#function-syn_cli_init) ([**SYN\_CLI**](structSYN__CLI.md) \* cli, const [**SYN\_CLI\_Command**](structSYN__CLI__Command.md) \* commands, size\_t cmd\_count, const char \* prompt) <br>_Initialize a CLI instance._  |
|  void | [**syn\_cli\_print\_prompt**](#function-syn_cli_print_prompt) (const [**SYN\_CLI**](structSYN__CLI.md) \* cli) <br>_Print the prompt._  |
|  void | [**syn\_cli\_printf**](#function-syn_cli_printf) (const [**SYN\_CLI**](structSYN__CLI.md) \* cli, const char \* fmt, ...) <br>_Print formatted output through the CLI's output function._  |
|  void | [**syn\_cli\_process\_char**](#function-syn_cli_process_char) ([**SYN\_CLI**](structSYN__CLI.md) \* cli, char ch) <br>_Process a single received character._  |
|  void | [**syn\_cli\_process\_line**](#function-syn_cli_process_line) ([**SYN\_CLI**](structSYN__CLI.md) \* cli, const char \* line) <br>_Process a complete null-terminated line._  |
|  void | [**syn\_cli\_set\_echo**](#function-syn_cli_set_echo) ([**SYN\_CLI**](structSYN__CLI.md) \* cli, [**bool**](syn__defs_8h.md#enum-bool) echo) <br>_Enable or disable local echo._  |
|  void | [**syn\_cli\_set\_errlog**](#function-syn_cli_set_errlog) (struct [**SYN\_ErrLog**](structSYN__ErrLog.md) \* errlog) <br>_Set errlog instance for the_ `errors` _built-in command._ |
|  void | [**syn\_cli\_set\_scheduler**](#function-syn_cli_set_scheduler) (struct [**SYN\_Sched**](structSYN__Sched.md) \* sched) <br>_Set scheduler instance for the_ `tasks` _built-in command._ |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  void | [**cli\_builtin\_errors**](#function-cli_builtin_errors) (const [**SYN\_CLI**](structSYN__CLI.md) \* cli) <br>_Built-in 'errors' command handler._  |
|  void | [**cli\_builtin\_tasks**](#function-cli_builtin_tasks) (const [**SYN\_CLI**](structSYN__CLI.md) \* cli) <br>_Built-in 'tasks' command handler._  |
|  void | [**cli\_builtin\_uptime**](#function-cli_builtin_uptime) (const [**SYN\_CLI**](structSYN__CLI.md) \* cli) <br>_Built-in 'uptime' command handler._  |
|  void | [**cli\_builtin\_version**](#function-cli_builtin_version) (const [**SYN\_CLI**](structSYN__CLI.md) \* cli) <br>_Built-in 'version' command handler._  |
|  void | [**cli\_dispatch**](#function-cli_dispatch) ([**SYN\_CLI**](structSYN__CLI.md) \* cli, char \* line) <br>_Look up and execute a command._  |
|  void | [**cli\_putchar**](#function-cli_putchar) (const [**SYN\_CLI**](structSYN__CLI.md) \* cli, char ch) <br>_Emit a single character via the console serial port._  |
|  void | [**cli\_puts**](#function-cli_puts) (const [**SYN\_CLI**](structSYN__CLI.md) \* cli, const char \* str) <br>_Emit a null-terminated string via the console serial port._  |
|  int | [**cli\_tokenize**](#function-cli_tokenize) (char \* line, char \* argv, int max\_args) <br>_Tokenize a line buffer in-place into argv[]._  |


























## Public Static Attributes Documentation




### variable s\_cli\_errlog 

_Global error log singleton reference for CLI diagnostics._ 
```C++
struct SYN_ErrLog* s_cli_errlog;
```




<hr>



### variable s\_cli\_sched 

_Global scheduler singleton reference for CLI task listing._ 
```C++
struct SYN_Sched* s_cli_sched;
```




<hr>
## Public Functions Documentation




### function syn\_cli\_init 

_Initialize a CLI instance._ 
```C++
void syn_cli_init (
    SYN_CLI * cli,
    const SYN_CLI_Command * commands,
    size_t cmd_count,
    const char * prompt
) 
```



Output goes directly to syn\_port\_serial\_write — no callback needed.




**Parameters:**


* `cli` CLI instance to initialize. 
* `commands` Array of command descriptors. 
* `cmd_count` Number of commands. 
* `prompt` Prompt string (e.g., "&gt; " or "syntropic&gt; "). Stored by pointer, not copied. 




        

<hr>



### function syn\_cli\_print\_prompt 

_Print the prompt._ 
```C++
void syn_cli_print_prompt (
    const SYN_CLI * cli
) 
```



Called automatically after command execution. Can also be called manually (e.g., on startup to show the initial prompt).




**Parameters:**


* `cli` CLI instance. 




        

<hr>



### function syn\_cli\_printf 

_Print formatted output through the CLI's output function._ 
```C++
void syn_cli_printf (
    const SYN_CLI * cli,
    const char * fmt,
    ...
) 
```



Convenience function for command handlers that need printf-style output.




**Parameters:**


* `cli` CLI instance. 
* `fmt` Format string. 
* `...` Arguments. 




        

<hr>



### function syn\_cli\_process\_char 

_Process a single received character._ 
```C++
void syn_cli_process_char (
    SYN_CLI * cli,
    char ch
) 
```



Call this for each byte received from the input stream (UART RX ISR, polled read, etc.). The CLI handles line editing (backspace) and dispatches the command when a newline is received.




**Parameters:**


* `cli` CLI instance. 
* `ch` Received character. 




        

<hr>



### function syn\_cli\_process\_line 

_Process a complete null-terminated line._ 
```C++
void syn_cli_process_line (
    SYN_CLI * cli,
    const char * line
) 
```



Alternative to process\_char when you already have a full line (e.g., from a buffered read). Does not echo or handle editing.




**Parameters:**


* `cli` CLI instance. 
* `line` Null-terminated command line. 




        

<hr>



### function syn\_cli\_set\_echo 

_Enable or disable local echo._ 
```C++
void syn_cli_set_echo (
    SYN_CLI * cli,
    bool echo
) 
```



When enabled (default), the CLI echoes each received character back to the terminal. Disable if the terminal handles its own echo.




**Parameters:**


* `cli` CLI instance. 
* `echo` true to enable echo, false to disable. 




        

<hr>



### function syn\_cli\_set\_errlog 

_Set errlog instance for the_ `errors` _built-in command._
```C++
void syn_cli_set_errlog (
    struct SYN_ErrLog * errlog
) 
```





**Parameters:**


* `errlog` Error log instance (or NULL to detach). 




        

<hr>



### function syn\_cli\_set\_scheduler 

_Set scheduler instance for the_ `tasks` _built-in command._
```C++
void syn_cli_set_scheduler (
    struct SYN_Sched * sched
) 
```





**Parameters:**


* `sched` Scheduler instance (or NULL to detach). 




        

<hr>
## Public Static Functions Documentation




### function cli\_builtin\_errors 

_Built-in 'errors' command handler._ 
```C++
static void cli_builtin_errors (
    const SYN_CLI * cli
) 
```





**Parameters:**


* `cli` CLI instance. 




        

<hr>



### function cli\_builtin\_tasks 

_Built-in 'tasks' command handler._ 
```C++
static void cli_builtin_tasks (
    const SYN_CLI * cli
) 
```





**Parameters:**


* `cli` CLI instance. 




        

<hr>



### function cli\_builtin\_uptime 

_Built-in 'uptime' command handler._ 
```C++
static void cli_builtin_uptime (
    const SYN_CLI * cli
) 
```





**Parameters:**


* `cli` CLI instance. 




        

<hr>



### function cli\_builtin\_version 

_Built-in 'version' command handler._ 
```C++
static void cli_builtin_version (
    const SYN_CLI * cli
) 
```





**Parameters:**


* `cli` CLI instance. 




        

<hr>



### function cli\_dispatch 

_Look up and execute a command._ 
```C++
static void cli_dispatch (
    SYN_CLI * cli,
    char * line
) 
```





**Parameters:**


* `cli` CLI instance. 
* `line` Input line (modified in-place during tokenization). 




        

<hr>



### function cli\_putchar 

_Emit a single character via the console serial port._ 
```C++
static void cli_putchar (
    const SYN_CLI * cli,
    char ch
) 
```





**Parameters:**


* `cli` CLI instance (unused, kept for internal API consistency). 
* `ch` Character to emit. 




        

<hr>



### function cli\_puts 

_Emit a null-terminated string via the console serial port._ 
```C++
static void cli_puts (
    const SYN_CLI * cli,
    const char * str
) 
```





**Parameters:**


* `cli` CLI instance (unused, kept for internal API consistency). 
* `str` String to print. 




        

<hr>



### function cli\_tokenize 

_Tokenize a line buffer in-place into argv[]._ 
```C++
static int cli_tokenize (
    char * line,
    char * argv,
    int max_args
) 
```



Handles double-quoted strings (quotes are stripped).




**Parameters:**


* `line` Input line (modified in-place). 
* `argv` [out] Array of token pointers. 
* `max_args` Maximum number of tokens. 



**Returns:**

Token count (argc). 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/cli/syn_cli.c`

