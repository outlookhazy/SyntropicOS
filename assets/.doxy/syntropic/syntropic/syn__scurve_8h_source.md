

# File syn\_scurve.h

[**File List**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**util**](dir_2b7a0faa62b8c8b128527d3a000d1ad9.md) **>** [**syn\_scurve.h**](syn__scurve_8h.md)

[Go to the documentation of this file](syn__scurve_8h.md)


```C++

#ifndef SYN_SCURVE_H
#define SYN_SCURVE_H

#include "../common/syn_defs.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int32_t p;        
    int32_t v;        
    int32_t a;        
    int32_t j;        
    int32_t target_p; 
    int32_t v_max;    
    int32_t a_max;    
    int32_t j_max;    
    int32_t phase_ticks[7]; 
    int32_t current_phase;  
    int32_t ticks_in_phase; 
    int32_t dir;            
    bool done;        
} SYN_SCurve;

/* ── Kinematic state getters ───────────────────────────────────────────── */

static inline int32_t syn_scurve_position(const SYN_SCurve *sc) { return sc->p; }

static inline int32_t syn_scurve_velocity(const SYN_SCurve *sc) { return sc->v; }

static inline int32_t syn_scurve_acceleration(const SYN_SCurve *sc) { return sc->a; }

static inline bool syn_scurve_done(const SYN_SCurve *sc) { return sc->done; }

/* ── API ────────────────────────────────────────────────────────────────── */

void syn_scurve_init(SYN_SCurve *sc, int32_t initial_p);

void syn_scurve_set_constraints(SYN_SCurve *sc, int32_t v_max, int32_t a_max, int32_t j_max);

void syn_scurve_set_target(SYN_SCurve *sc, int32_t target);

int32_t syn_scurve_update(SYN_SCurve *sc);

#ifdef __cplusplus
}
#endif
#endif // SYN_SCURVE_H
```


