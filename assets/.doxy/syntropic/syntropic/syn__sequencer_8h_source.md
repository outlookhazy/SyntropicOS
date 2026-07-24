

# File syn\_sequencer.h

[**File List**](files.md) **>** [**sched**](dir_19ac69a9d5854741ebbc92bce8e94ea7.md) **>** [**syn\_sequencer.h**](syn__sequencer_8h.md)

[Go to the documentation of this file](syn__sequencer_8h.md)


```C++

#ifndef SYN_SEQUENCER_H
#define SYN_SEQUENCER_H

#include "../port/syn_port_system.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Step action ────────────────────────────────────────────────────────── */

typedef void (*SYN_SeqAction)(void *ctx);

typedef struct {
    SYN_SeqAction  action;     
    void           *ctx;        
    uint32_t        delay_ms;   
} SYN_SeqStep;

/* ── Sequencer state ────────────────────────────────────────────────────── */

typedef enum {
    SYN_SEQ_IDLE     = 0,            
    SYN_SEQ_RUNNING  = 1,            
    SYN_SEQ_WAITING  = 2,            
    SYN_SEQ_DONE     = 3,            
} SYN_SeqState;

/* ── Completion callback ────────────────────────────────────────────────── */

struct SYN_Sequencer;

typedef void (*SYN_SeqCompleteCallback)(struct SYN_Sequencer *seq,
                                          void *ctx);

typedef struct SYN_Sequencer {
    const SYN_SeqStep  *steps;          
    uint16_t             step_count;    
    uint16_t             current;       
    SYN_SeqState        state;         
    uint32_t             wait_start;    
    bool                 loop;          
    uint16_t             loop_count;    
    SYN_SeqCompleteCallback on_complete; 
    void                    *on_complete_ctx; 
} SYN_Sequencer;

/* ── API ────────────────────────────────────────────────────────────────── */

void syn_seq_init(SYN_Sequencer *seq,
                   const SYN_SeqStep *steps, uint16_t count);

void syn_seq_on_complete(SYN_Sequencer *seq,
                          SYN_SeqCompleteCallback cb, void *ctx);

void syn_seq_set_loop(SYN_Sequencer *seq, bool loop);

void syn_seq_start(SYN_Sequencer *seq);

void syn_seq_stop(SYN_Sequencer *seq);

bool syn_seq_update(SYN_Sequencer *seq);

static inline bool syn_seq_is_done(const SYN_Sequencer *seq)
{
    return seq->state == SYN_SEQ_DONE || seq->state == SYN_SEQ_IDLE;
}

static inline uint16_t syn_seq_current_step(const SYN_Sequencer *seq)
{
    return seq->current;
}

static inline uint16_t syn_seq_loops(const SYN_Sequencer *seq)
{
    return seq->loop_count;
}

#ifdef __cplusplus
}
#endif

#endif /* SYN_SEQUENCER_H */
```


