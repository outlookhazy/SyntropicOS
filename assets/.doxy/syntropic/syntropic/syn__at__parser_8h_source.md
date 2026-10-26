

# File syn\_at\_parser.h

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_at\_parser.h**](syn__at__parser_8h.md)

[Go to the documentation of this file](syn__at__parser_8h.md)


```C++

#ifndef SYN_AT_PARSER_H
#define SYN_AT_PARSER_H

#include "../common/syn_defs.h"
#include "../pt/syn_pt.h"
#include "../util/syn_stream.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SYN_AT_RESP_NONE = 0,  
    SYN_AT_RESP_OK,        
    SYN_AT_RESP_ERROR,     
    SYN_AT_RESP_CME_ERROR, 
    SYN_AT_RESP_PROMPT,    
    SYN_AT_RESP_LINE,      
    SYN_AT_RESP_URC,       
} SYN_AtRespType;

typedef struct {
    char *line_buf;           
    size_t line_buf_size;     
    size_t line_len;          
    SYN_AtRespType last_resp; 
    int cme_error_code;   
    bool prompt_detected; 
} SYN_AtParser;

SYN_Status syn_at_parser_init(SYN_AtParser *parser, char *buf, size_t buf_size);

void syn_at_parser_reset(SYN_AtParser *parser);

SYN_AtRespType syn_at_parser_feed_char(SYN_AtParser *parser, char c);

SYN_AtRespType syn_at_parser_feed_stream(SYN_AtParser *parser, SYN_Stream *stream);

const char *syn_at_parser_get_line(const SYN_AtParser *parser);

int syn_at_parser_get_cme_error(const SYN_AtParser *parser);

bool syn_at_parser_get_param_int(const char *line, size_t param_idx, int *out_val);

bool syn_at_parser_get_param_str(const char *line, size_t param_idx, char *out_buf, size_t max_len);

#ifdef __cplusplus
}
#endif

#endif /* SYN_AT_PARSER_H */
```


