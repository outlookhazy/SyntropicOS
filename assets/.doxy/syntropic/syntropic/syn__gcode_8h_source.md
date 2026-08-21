

# File syn\_gcode.h

[**File List**](files.md) **>** [**motor**](dir_61470c21c7c2e7c9f977d1e3c2e6c7e0.md) **>** [**syn\_gcode.h**](syn__gcode_8h.md)

[Go to the documentation of this file](syn__gcode_8h.md)


```C++

#ifndef SYN_GCODE_H
#define SYN_GCODE_H

#include "../common/syn_defs.h"
#include "../pt/syn_pt.h"
#include "../sched/syn_sched.h"
#include "syn_interpolator.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#if !defined(SYN_USE_GCODE) || SYN_USE_GCODE

#ifdef __cplusplus
extern "C" {
#endif

/* ── Modal State Enums ──────────────────────────────────────────────────── */

typedef enum {
    SYN_GCODE_MOTION_RAPID = 0,   
    SYN_GCODE_MOTION_LINEAR = 1,  
    SYN_GCODE_MOTION_ARC_CW = 2,  
    SYN_GCODE_MOTION_ARC_CCW = 3, 
    SYN_GCODE_MOTION_DWELL = 4    
} SYN_GCode_MotionMode;

typedef enum {
    SYN_GCODE_DIST_ABSOLUTE = 0,   
    SYN_GCODE_DIST_INCREMENTAL = 1 
} SYN_GCode_DistanceMode;

typedef enum {
    SYN_GCODE_PLANE_XY = 0, 
    SYN_GCODE_PLANE_XZ = 1, 
    SYN_GCODE_PLANE_YZ = 2  
} SYN_GCode_Plane;

typedef enum {
    SYN_GCODE_UNITS_MM = 0,  
    SYN_GCODE_UNITS_INCH = 1 
} SYN_GCode_Units;

typedef enum {
    SYN_GCODE_SPINDLE_OFF = 0, 
    SYN_GCODE_SPINDLE_CW = 1,  
    SYN_GCODE_SPINDLE_CCW = 2  
} SYN_GCode_SpindleMode;

typedef enum {
    SYN_GCODE_COOLANT_OFF = 0,   
    SYN_GCODE_COOLANT_FLOOD = 1, 
    SYN_GCODE_COOLANT_MIST = 2   
} SYN_GCode_CoolantMode;

/* ── Parsed Block Representation ────────────────────────────────────────── */

typedef struct {
    bool has_g;      
    uint32_t g_code; 
    bool has_m;      
    uint32_t m_code; 
    bool has_x; 
    float x;    
    bool has_y; 
    float y;    
    bool has_z; 
    float z;    
    bool has_i; 
    float i;    
    bool has_j; 
    float j;    
    bool has_k; 
    float k;    
    bool has_f; 
    float f;    
    bool has_s; 
    float s;    
    bool has_p; 
    float p;    
    bool has_t; 
    uint32_t t; 
} SYN_GCode_Block;

/* ── Controller & Callbacks ─────────────────────────────────────────────── */

struct SYN_GCode_Controller;

typedef void (*SYN_GCode_SpindleCb)(SYN_GCode_SpindleMode mode, float rpm, void *user_data);

typedef void (*SYN_GCode_CoolantCb)(SYN_GCode_CoolantMode mode, void *user_data);

typedef void (*SYN_GCode_ToolCb)(uint32_t tool_num, void *user_data);

typedef struct {
    SYN_Interpolator *interpolator; 
    float default_feedrate;         
    float max_acceleration;         
    float max_jerk;                 
    float step_resolution;          
    SYN_GCode_SpindleCb on_spindle; 
    SYN_GCode_CoolantCb on_coolant; 
    SYN_GCode_ToolCb on_tool;       
    void *user_data;                
} SYN_GCode_Config;

typedef struct SYN_GCode_Controller {
    SYN_GCode_Config cfg; 
    SYN_PT pt;            
    SYN_Vector3F current_pos; 
    SYN_Vector3F offset_pos;  
    SYN_GCode_MotionMode motion_mode; 
    SYN_GCode_DistanceMode dist_mode; 
    SYN_GCode_Plane plane;            
    SYN_GCode_Units units;            
    SYN_GCode_SpindleMode spindle;    
    SYN_GCode_CoolantMode coolant;    
    float feedrate;       
    float spindle_speed;  
    uint32_t active_tool; 
    bool move_in_progress; 
} SYN_GCode_Controller;

/* ── Public API ─────────────────────────────────────────────────────────── */

SYN_Status syn_gcode_init(SYN_GCode_Controller *ctrl, const SYN_GCode_Config *cfg);

SYN_Status syn_gcode_parse_line(const char *line, SYN_GCode_Block *block);

SYN_Status syn_gcode_execute_block(SYN_GCode_Controller *ctrl, const SYN_GCode_Block *block);

SYN_Status syn_gcode_execute_line(SYN_GCode_Controller *ctrl, const char *line);

bool syn_gcode_step(SYN_GCode_Controller *ctrl, SYN_Vector3F *out_pos);

SYN_PT_Status syn_gcode_task_pt(SYN_PT *pt, SYN_Task *task);

#ifdef __cplusplus
}
#endif

#endif /* SYN_USE_GCODE */

#endif /* SYN_GCODE_H */
```


