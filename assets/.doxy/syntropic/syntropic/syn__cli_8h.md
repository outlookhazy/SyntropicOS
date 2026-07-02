

# File syn\_cli.h



[**FileList**](files.md) **>** [**cli**](dir_da6bc3c87aefe1086f828540bad8d0dd.md) **>** [**syn\_cli.h**](syn__cli_8h.md)

[Go to the source code of this file](syn__cli_8h_source.md)

_Command-line interpreter for embedded systems._ [More...](#detailed-description)

* `#include "../common/syn_defs.h"`
* `#include <stddef.h>`
* `#include <stdbool.h>`
* `#include "../system/syn_errlog.h"`
* `#include "../sched/syn_sched.h"`















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**SYN\_CLI**](structSYN__CLI.md) <br>_CLI instance — command table, line buffer, I/O, and history._  |
| struct | [**SYN\_CLI\_Command**](structSYN__CLI__Command.md) <br>_Command descriptor._  |


## Public Types

| Type | Name |
| ---: | :--- |
| typedef int(\* | [**SYN\_CLI\_Handler**](#typedef-syn_cli_handler)  <br>_Command handler function._  |
| typedef void(\* | [**SYN\_CLI\_PutChar**](#typedef-syn_cli_putchar)  <br>_Character output function._  |
| typedef void(\* | [**SYN\_CLI\_Puts**](#typedef-syn_cli_puts)  <br>_String output function._  |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_cli\_init**](#function-syn_cli_init) ([**SYN\_CLI**](structSYN__CLI.md) \* cli, const [**SYN\_CLI\_Command**](structSYN__CLI__Command.md) \* commands, size\_t cmd\_count, [**SYN\_CLI\_PutChar**](syn__cli_8h.md#typedef-syn_cli_putchar) putchar\_fn, const char \* prompt) <br>_Initialize a CLI instance._  |
|  void | [**syn\_cli\_print\_prompt**](#function-syn_cli_print_prompt) (const [**SYN\_CLI**](structSYN__CLI.md) \* cli) <br>_Print the prompt._  |
|  void | [**syn\_cli\_printf**](#function-syn_cli_printf) (const [**SYN\_CLI**](structSYN__CLI.md) \* cli, const char \* fmt, ...) <br>_Print formatted output through the CLI's output function._  |
|  void | [**syn\_cli\_process\_char**](#function-syn_cli_process_char) ([**SYN\_CLI**](structSYN__CLI.md) \* cli, char ch) <br>_Process a single received character._  |
|  void | [**syn\_cli\_process\_line**](#function-syn_cli_process_line) ([**SYN\_CLI**](structSYN__CLI.md) \* cli, const char \* line) <br>_Process a complete null-terminated line._  |
|  void | [**syn\_cli\_set\_echo**](#function-syn_cli_set_echo) ([**SYN\_CLI**](structSYN__CLI.md) \* cli, [**bool**](syn__defs_8h.md#enum-bool) echo) <br>_Enable or disable local echo._  |
|  void | [**syn\_cli\_set\_errlog**](#function-syn_cli_set_errlog) ([**SYN\_ErrLog**](structSYN__ErrLog.md) \* errlog) <br>_Set errlog instance for the_ `errors` _built-in command._ |
|  void | [**syn\_cli\_set\_puts**](#function-syn_cli_set_puts) ([**SYN\_CLI**](structSYN__CLI.md) \* cli, [**SYN\_CLI\_Puts**](syn__cli_8h.md#typedef-syn_cli_puts) puts\_fn) <br>_Set an optional string output function for multi-char output._  |
|  void | [**syn\_cli\_set\_scheduler**](#function-syn_cli_set_scheduler) ([**SYN\_Sched**](structSYN__Sched.md) \* sched) <br>_Set scheduler instance for the_ `tasks` _built-in command._ |



























## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SYN\_CLI\_CMD\_ERRORS**](syn__cli_8h.md#define-syn_cli_cmd_errors)  `1`<br>_Enable built-in 'errors' command._  |
| define  | [**SYN\_CLI\_CMD\_TASKS**](syn__cli_8h.md#define-syn_cli_cmd_tasks)  `1`<br>_Enable built-in 'tasks' command._  |
| define  | [**SYN\_CLI\_CMD\_UPTIME**](syn__cli_8h.md#define-syn_cli_cmd_uptime)  `1`<br>_Enable built-in 'uptime' command._  |
| define  | [**SYN\_CLI\_CMD\_VERSION**](syn__cli_8h.md#define-syn_cli_cmd_version)  `1`<br>_Register built-in diagnostic commands._  |
| define  | [**SYN\_CLI\_HISTORY\_DEPTH**](syn__cli_8h.md#define-syn_cli_history_depth)  `0`<br> |
| define  | [**SYN\_CLI\_LINE\_BUF\_SIZE**](syn__cli_8h.md#define-syn_cli_line_buf_size)  `128`<br> |
| define  | [**SYN\_CLI\_MAX\_ARGS**](syn__cli_8h.md#define-syn_cli_max_args)  `16`<br> |

## Detailed Description


Provides a lightweight interactive shell over UART (or any byte stream). Commands are registered as a static table — no dynamic allocation. Input is processed one character at a time, making it easy to feed from a UART ISR or polled loop.




**
**


* Statically-registered command table (name + help + handler)
* Automatic argc/argv parsing (whitespace-delimited, with quoted strings)
* Built-in `help` command
* Backspace and line editing
* Configurable prompt
* Echo control (for terminals that don't echo locally)
* Command history (optional, configurable depth)






**
**


```C++
// Define commands
static int cmd_led(int argc, char *argv[]);
static int cmd_reset(int argc, char *argv[]);

static const SYN_CLI_Command commands[] = {
    { "led",   "led <on|off>  — Control the LED",  cmd_led   },
    { "reset", "reset         — System reset",     cmd_reset },
};

// Initialize
static SYN_CLI cli;
syn_cli_init(&cli, commands, 2, my_putchar, "> ");

// In main loop or UART RX ISR:
syn_cli_process_char(&cli, received_byte);
```
 





    
## Public Types Documentation




### typedef SYN\_CLI\_Handler 

_Command handler function._ 
```C++
typedef int(* SYN_CLI_Handler) (int argc, char *argv[]);
```





**Parameters:**


* `argc` Argument count (including the command name). 
* `argv` Argument vector. argv[0] is the command name. 



**Returns:**

0 on success, or an application-defined error code. 





        

<hr>



### typedef SYN\_CLI\_PutChar 

_Character output function._ 
```C++
typedef void(* SYN_CLI_PutChar) (char ch);
```



The CLI calls this to emit one character (for echo, prompt, and output).




**Parameters:**


* `ch` Character to output. 




        

<hr>



### typedef SYN\_CLI\_Puts 

_String output function._ 
```C++
typedef void(* SYN_CLI_Puts) (const char *str);
```



The CLI calls this to emit a null-terminated string.




**Parameters:**


* `str` Null-terminated string. 




        

<hr>
## Public Functions Documentation




### function syn\_cli\_init 

_Initialize a CLI instance._ 
```C++
void syn_cli_init (
    SYN_CLI * cli,
    const SYN_CLI_Command * commands,
    size_t cmd_count,
    SYN_CLI_PutChar putchar_fn,
    const char * prompt
) 
```





**Parameters:**


* `cli` CLI instance to initialize. 
* `commands` Array of command descriptors. 
* `cmd_count` Number of commands. 
* `putchar_fn` Character output function. 
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
    SYN_ErrLog * errlog
) 
```



If set, the `errors` command will dump all entries from this log.




**Parameters:**


* `errlog` Error log instance. 




        

<hr>



### function syn\_cli\_set\_puts 

_Set an optional string output function for multi-char output._ 
```C++
void syn_cli_set_puts (
    SYN_CLI * cli,
    SYN_CLI_Puts puts_fn
) 
```



If not set, the CLI uses putchar\_fn to output strings one character at a time. Setting this can improve efficiency.




**Parameters:**


* `cli` CLI instance. 
* `puts_fn` String output function. 




        

<hr>



### function syn\_cli\_set\_scheduler 

_Set scheduler instance for the_ `tasks` _built-in command._
```C++
void syn_cli_set_scheduler (
    SYN_Sched * sched
) 
```



If set, the `tasks` command will list all registered tasks.




**Parameters:**


* `sched` Scheduler instance. 




        

<hr>
## Macro Definition Documentation





### define SYN\_CLI\_CMD\_ERRORS 

_Enable built-in 'errors' command._ 
```C++
#define SYN_CLI_CMD_ERRORS `1`
```




<hr>



### define SYN\_CLI\_CMD\_TASKS 

_Enable built-in 'tasks' command._ 
```C++
#define SYN_CLI_CMD_TASKS `1`
```




<hr>



### define SYN\_CLI\_CMD\_UPTIME 

_Enable built-in 'uptime' command._ 
```C++
#define SYN_CLI_CMD_UPTIME `1`
```




<hr>



### define SYN\_CLI\_CMD\_VERSION 

_Register built-in diagnostic commands._ 
```C++
#define SYN_CLI_CMD_VERSION `1`
```



Adds framework-aware commands to the CLI. Each is individually guarded by a define (default: all enabled). The commands operate on the framework singletons you pass in.




**Parameters:**


* `cli` CLI instance.

Built-in commands (enable/disable via defines):
* `version` — print [**syn\_version()**](syn__version_8h.md#function-syn_version) info (SYN\_CLI\_CMD\_VERSION, default 1)
* `uptime` — print tick\_ms uptime (SYN\_CLI\_CMD\_UPTIME, default 1)
* `errors` — dump errlog entries (SYN\_CLI\_CMD\_ERRORS, default 1)
* `tasks` — show scheduler task states (SYN\_CLI\_CMD\_TASKS, default 1) 




        

<hr>



### define SYN\_CLI\_HISTORY\_DEPTH 

```C++
#define SYN_CLI_HISTORY_DEPTH `0`
```



Command history depth (0 = disabled). 


        

<hr>



### define SYN\_CLI\_LINE\_BUF\_SIZE 

```C++
#define SYN_CLI_LINE_BUF_SIZE `128`
```



Maximum length of a single command line (including null terminator). 


        

<hr>



### define SYN\_CLI\_MAX\_ARGS 

```C++
#define SYN_CLI_MAX_ARGS `16`
```



Maximum number of arguments (including the command name). 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/cli/syn_cli.h`

