

# File syn\_cli.h

[**File List**](files.md) **>** [**cli**](dir_da6bc3c87aefe1086f828540bad8d0dd.md) **>** [**syn\_cli.h**](syn__cli_8h.md)

[Go to the documentation of this file](syn__cli_8h.md)


```C++

#ifndef SYN_CLI_H
#define SYN_CLI_H

#include "../common/syn_defs.h"

#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Configuration ──────────────────────────────────────────────────────── */

#ifndef SYN_CLI_LINE_BUF_SIZE
  #define SYN_CLI_LINE_BUF_SIZE   128
#endif

#ifndef SYN_CLI_MAX_ARGS
  #define SYN_CLI_MAX_ARGS        16
#endif

#ifndef SYN_CLI_HISTORY_DEPTH
  #define SYN_CLI_HISTORY_DEPTH    0
#endif

/* ── Types ──────────────────────────────────────────────────────────────── */

typedef int (*SYN_CLI_Handler)(int argc, char *argv[]);

typedef struct {
    const char       *name;     
    const char       *help;     
    SYN_CLI_Handler  handler;  
} SYN_CLI_Command;

typedef void (*SYN_CLI_PutChar)(char ch);

typedef void (*SYN_CLI_Puts)(const char *str);

typedef struct {
    /* Command table */
    const SYN_CLI_Command *commands;     
    size_t                  command_count; 
    /* I/O */
    SYN_CLI_PutChar        putchar_fn;   
    SYN_CLI_Puts           puts_fn;      
    /* Line buffer */
    char    line_buf[SYN_CLI_LINE_BUF_SIZE]; 
    size_t  line_pos;                    
    /* Configuration */
    const char *prompt;                  
    bool        echo;                    
    uint8_t     escape_state;            
    /* History */
#if SYN_CLI_HISTORY_DEPTH > 0
    char    history[SYN_CLI_HISTORY_DEPTH][SYN_CLI_LINE_BUF_SIZE]; 
    size_t  history_count;               
    size_t  history_write;               
    size_t  history_read;                
#endif
} SYN_CLI;

/* ── API ────────────────────────────────────────────────────────────────── */

void syn_cli_init(SYN_CLI *cli,
                   const SYN_CLI_Command *commands,
                   size_t cmd_count,
                   SYN_CLI_PutChar putchar_fn,
                   const char *prompt);

void syn_cli_set_puts(SYN_CLI *cli, SYN_CLI_Puts puts_fn);

void syn_cli_set_echo(SYN_CLI *cli, bool echo);

void syn_cli_process_char(SYN_CLI *cli, char ch);

void syn_cli_process_line(SYN_CLI *cli, const char *line);

void syn_cli_print_prompt(const SYN_CLI *cli);

void syn_cli_printf(const SYN_CLI *cli, const char *fmt, ...);

/* ── Built-in commands ──────────────────────────────────────────────────── */

#ifndef SYN_CLI_CMD_VERSION
#define SYN_CLI_CMD_VERSION  1
#endif
#ifndef SYN_CLI_CMD_UPTIME
#define SYN_CLI_CMD_UPTIME   1
#endif
#ifndef SYN_CLI_CMD_ERRORS
#define SYN_CLI_CMD_ERRORS   1
#endif
#ifndef SYN_CLI_CMD_TASKS
#define SYN_CLI_CMD_TASKS    1
#endif

#include "../system/syn_errlog.h"
void syn_cli_set_errlog(SYN_ErrLog *errlog);

#include "../sched/syn_sched.h"
void syn_cli_set_scheduler(SYN_Sched *sched);

#ifdef __cplusplus
}
#endif

#endif /* SYN_CLI_H */
```


