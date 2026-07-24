

# File syn\_interpolator.h

[**File List**](files.md) **>** [**motor**](dir_61470c21c7c2e7c9f977d1e3c2e6c7e0.md) **>** [**syn\_interpolator.h**](syn__interpolator_8h.md)

[Go to the documentation of this file](syn__interpolator_8h.md)


```C++

#ifndef SYN_INTERPOLATOR_H
#define SYN_INTERPOLATOR_H

#include "../common/syn_defs.h"
#include "../util/syn_scurve.h"

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SYN_INTERP_MODE_IDLE = 0,
    SYN_INTERP_MODE_LINEAR,       
    SYN_INTERP_MODE_CIRCULAR_CW,  
    SYN_INTERP_MODE_CIRCULAR_CCW, 
} SYN_Interp_Mode;

typedef struct {
    float x; 
    float y; 
    float z; 
} SYN_Vector3F;

typedef struct {
    SYN_Interp_Mode mode;       
    /* Endpoints & Geometry */
    SYN_Vector3F start_pos;      
    SYN_Vector3F target_pos;     
    SYN_Vector3F center_offset;  
    /* Current State */
    SYN_Vector3F current_pos;   
    float        total_length;  
    float        current_dist;  
    float        radius;        
    float        start_angle;   
    float        sweep_angle;   
    /* Speed & Acceleration Profiling */
    SYN_SCurve   scurve;        
    float        target_feedrate; 
    uint32_t     total_steps;   
    uint32_t     step_index;    
} SYN_Interpolator;

void syn_interpolator_init(SYN_Interpolator *interp);

SYN_Status syn_interpolator_plan_linear(SYN_Interpolator *interp,
                                         SYN_Vector3F start, SYN_Vector3F target,
                                         float feedrate, float max_accel, float max_jerk,
                                         float step_res);

SYN_Status syn_interpolator_plan_circular(SYN_Interpolator *interp,
                                           SYN_Vector3F start, SYN_Vector3F target,
                                           SYN_Vector3F center_offset, bool is_cw,
                                           float feedrate, float max_accel, float max_jerk,
                                           float step_res);

bool syn_interpolator_step(SYN_Interpolator *interp, SYN_Vector3F *out_pos);

bool syn_interpolator_eval_at_time(SYN_Interpolator *interp, float t_sec,
                                   SYN_Vector3F *out_pos, SYN_Vector3F *out_vel);

#ifdef __cplusplus
}
#endif

#endif /* SYN_INTERPOLATOR_H */
```


