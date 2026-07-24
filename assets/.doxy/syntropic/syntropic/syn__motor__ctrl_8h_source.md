

# File syn\_motor\_ctrl.h

[**File List**](files.md) **>** [**motor**](dir_61470c21c7c2e7c9f977d1e3c2e6c7e0.md) **>** [**syn\_motor\_ctrl.h**](syn__motor__ctrl_8h.md)

[Go to the documentation of this file](syn__motor__ctrl_8h.md)


```C++

#ifndef SYN_MOTOR_CTRL_H
#define SYN_MOTOR_CTRL_H

#include "../common/syn_defs.h"
#include "../control/syn_pid.h"
#include "../motor/syn_motor_output.h"
#include "../util/syn_ramp.h"
#include "../util/syn_scurve.h"
#include "../port/syn_port_system.h"
#include "../system/syn_errlog.h"
#include "../log/syn_datalog.h"

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Feedback function ──────────────────────────────────────────────────── */

typedef int32_t (*SYN_MotorCtrl_ReadPos)(void *ctx);

/* ── Control mode ───────────────────────────────────────────────────────── */

typedef enum {
    SYN_MCTRL_MODE_IDLE      = 0,  
    SYN_MCTRL_MODE_VELOCITY  = 1,  
    SYN_MCTRL_MODE_POSITION  = 2,  
    SYN_MCTRL_MODE_OPEN_LOOP = 3,  
} SYN_MotorCtrl_Mode;

/* ── Controller state ───────────────────────────────────────────────────── */

typedef enum {
    SYN_MCTRL_STOPPED    = 0,  
    SYN_MCTRL_RUNNING    = 1,  
    SYN_MCTRL_ON_TARGET  = 2,  
    SYN_MCTRL_STALLED    = 3,  
    SYN_MCTRL_LIMIT      = 4,  
} SYN_MotorCtrl_State;

/* ── Callbacks ──────────────────────────────────────────────────────────── */

struct SYN_MotorCtrl;

typedef void (*SYN_MotorCtrl_StallCallback)(struct SYN_MotorCtrl *ctrl,
                                              void *ctx);

typedef void (*SYN_MotorCtrl_TargetCallback)(struct SYN_MotorCtrl *ctrl,
                                               void *ctx);

/* ── Trajectory input ───────────────────────────────────────────────────── */

typedef struct {
    int32_t position;       
    int32_t velocity;       
    int32_t acceleration;   
} SYN_MotorCtrl_Trajectory;

/* ── Tuning capture sample ──────────────────────────────────────────────── */

#define SYN_MCTRL_DATALOG_ID  0x4D43  /* 'MC' */

typedef struct {
    uint32_t tick_ms;         
    int32_t  target_pos;      
    int32_t  measured_pos;    
    int32_t  target_vel;      
    int32_t  measured_vel;    
    int32_t  ff_output;       
    int32_t  pid_output;      
    int32_t  total_output;    
} SYN_MotorCtrl_Sample;

/* ── Move metrics (computed on-the-fly, zero buffer cost) ───────────────── */

typedef struct {
    int32_t  max_error;        
    int64_t  error_sq_sum;     
    int32_t  overshoot;        
    int32_t  peak_output;      
    uint32_t sample_count;     
    uint32_t move_start_tick;  
    uint32_t settle_tick;      
} SYN_MotorCtrl_Metrics;

/* ── Configuration ──────────────────────────────────────────────────────── */

typedef struct {
    /* Motor output (mandatory — use syn_dc_motor_output(), etc.) */
    SYN_MotorOutput         motor;        
    /* Feedback source (mandatory) */
    SYN_MotorCtrl_ReadPos read_pos;     
    void                  *read_pos_ctx; 
    /* PID gains (integer, divided by 1 << pid_scale) */
    int32_t             pid_kp;
    int32_t             pid_ki;
    int32_t             pid_kd;
    uint8_t             pid_scale;    
    /* Feedforward gains (optional — 0 = disabled)
     * Applied when using set_trajectory(). Output contribution:
     *   ff_output = (ff_kv × velocity + ff_ka × acceleration) >> ff_scale
     */
    int32_t             ff_kv;        
    int32_t             ff_ka;        
    uint8_t             ff_scale;     
    /* Loop rate */
    uint16_t            update_hz;    
    /* Output limits */
    int32_t             output_min;   
    int32_t             output_max;   
    /* Position mode */
    int32_t             position_deadband; 
    /* Soft position limits (0, 0 = disabled) */
    int32_t             position_min; 
    int32_t             position_max; 
    /* Stall detection */
    uint16_t            stall_timeout_ms;  
    int32_t             stall_threshold;   
    /* Error logging (optional) */
    SYN_ErrLog            *errlog;            
} SYN_MotorCtrl_Config;

#define SYN_MOTOR_CTRL_DEFAULTS(motor_out, read_fn, read_ctx, hz, out_max)  \
    ((SYN_MotorCtrl_Config){                                                \
        .motor             = (motor_out),                                   \
        .read_pos          = (read_fn),                                     \
        .read_pos_ctx      = (read_ctx),                                    \
        .update_hz         = (hz),                                          \
        .output_min        = -(out_max),                                    \
        .output_max        = (out_max),                                     \
        .pid_kp            = 1 << 6,    /* 0.25 effective (conservative) */ \
        .pid_ki            = 0,         /* no integral by default */        \
        .pid_kd            = 0,         /* no derivative by default */      \
        .pid_scale         = 8,         /* gains ÷ 256 */                   \
        .position_deadband = 2,                                             \
        .stall_timeout_ms  = 1000,                                          \
        .stall_threshold   = 1,                                             \
    })

/* ── Controller instance ────────────────────────────────────────────────── */

typedef struct SYN_MotorCtrl {
    SYN_MotorCtrl_Config  cfg;      
    SYN_PID               pid;      
    SYN_MotorCtrl_Mode    mode;       
    SYN_MotorCtrl_State   state;      
    /* Targets */
    int32_t                target_velocity;  
    int32_t                target_position;  
    /* Trajectory (feedforward) */
    SYN_MotorCtrl_Trajectory trajectory;     
    bool                   trajectory_active; 
    int32_t                ff_output;        
    /* Measurements */
    int32_t                measured_velocity; 
    int32_t                measured_position; 
    int32_t                last_position;     
    int32_t                pid_output;        
    int32_t                total_output;      
    /* Timing */
    uint32_t               last_update_tick;

    uint32_t               stall_start_tick;
    bool                   stall_active;

    SYN_MotorCtrl_StallCallback  on_stall;
    void                         *on_stall_ctx;
    SYN_MotorCtrl_TargetCallback on_target;
    void                         *on_target_ctx;

    bool                   enabled;

    /* Tuning capture (optional — NULL to disable) */
    SYN_DataLog           *datalog;          
    SYN_MotorCtrl_Metrics  metrics;

    SYN_Ramp               profile;
    SYN_SCurve             scurve_profile;
    bool                   profile_active;
    bool                   scurve_active;
} SYN_MotorCtrl;

/* ── API ────────────────────────────────────────────────────────────────── */

SYN_Status syn_motor_ctrl_init(SYN_MotorCtrl *ctrl,
                                 const SYN_MotorCtrl_Config *cfg);

void syn_motor_ctrl_set_output(SYN_MotorCtrl *ctrl, int32_t output);

void syn_motor_ctrl_set_velocity(SYN_MotorCtrl *ctrl, int32_t units_per_sec);

void syn_motor_ctrl_set_position(SYN_MotorCtrl *ctrl, int32_t target);

void syn_motor_ctrl_set_trajectory(SYN_MotorCtrl *ctrl,
                                    const SYN_MotorCtrl_Trajectory *traj);

void syn_motor_ctrl_move_to(SYN_MotorCtrl *ctrl, int32_t target,
                             int32_t max_velocity, int32_t acceleration);

void syn_motor_ctrl_move_to_scurve(SYN_MotorCtrl *ctrl, int32_t target,
                                    int32_t max_velocity, int32_t max_accel,
                                    int32_t max_jerk);

void syn_motor_ctrl_stop(SYN_MotorCtrl *ctrl);

void syn_motor_ctrl_estop(SYN_MotorCtrl *ctrl);

SYN_MotorCtrl_State syn_motor_ctrl_update(SYN_MotorCtrl *ctrl);

void syn_motor_ctrl_on_stall(SYN_MotorCtrl *ctrl,
                               SYN_MotorCtrl_StallCallback cb, void *ctx);

void syn_motor_ctrl_on_target(SYN_MotorCtrl *ctrl,
                                SYN_MotorCtrl_TargetCallback cb, void *ctx);

void syn_motor_ctrl_set_gains(SYN_MotorCtrl *ctrl,
                                int32_t kp, int32_t ki, int32_t kd);

void syn_motor_ctrl_set_ff_gains(SYN_MotorCtrl *ctrl,
                                  int32_t ff_kv, int32_t ff_ka);

void syn_motor_ctrl_clear_stall(SYN_MotorCtrl *ctrl);

void syn_motor_ctrl_set_datalog(SYN_MotorCtrl *ctrl, SYN_DataLog *log);

/* ── Getters ────────────────────────────────────────────────────────────── */

static inline SYN_MotorCtrl_State
syn_motor_ctrl_state(const SYN_MotorCtrl *ctrl)     { return ctrl->state; }

static inline SYN_MotorCtrl_Mode
syn_motor_ctrl_mode(const SYN_MotorCtrl *ctrl)      { return ctrl->mode; }

static inline int32_t
syn_motor_ctrl_velocity(const SYN_MotorCtrl *ctrl)  { return ctrl->measured_velocity; }

static inline int32_t
syn_motor_ctrl_position(const SYN_MotorCtrl *ctrl)  { return ctrl->measured_position; }

static inline int32_t
syn_motor_ctrl_output(const SYN_MotorCtrl *ctrl)    { return ctrl->total_output; }

static inline int32_t
syn_motor_ctrl_pid_output(const SYN_MotorCtrl *ctrl) { return ctrl->pid_output; }

static inline int32_t
syn_motor_ctrl_ff_output(const SYN_MotorCtrl *ctrl)  { return ctrl->ff_output; }

static inline int32_t
syn_motor_ctrl_error(const SYN_MotorCtrl *ctrl)     { return ctrl->pid.prev_error; }

/* ── Metrics ───────────────────────────────────────────────────────────── */

void syn_motor_ctrl_reset_metrics(SYN_MotorCtrl *ctrl);

static inline const SYN_MotorCtrl_Metrics *
syn_motor_ctrl_get_metrics(const SYN_MotorCtrl *ctrl) { return &ctrl->metrics; }

int32_t syn_motor_ctrl_rms_error(const SYN_MotorCtrl *ctrl);

static inline uint32_t
syn_motor_ctrl_move_duration(const SYN_MotorCtrl *ctrl)
{
    if (ctrl->metrics.move_start_tick == 0) return 0;
    return syn_port_get_tick_ms() - ctrl->metrics.move_start_tick;
}

static inline uint32_t
syn_motor_ctrl_settle_time(const SYN_MotorCtrl *ctrl)
{
    if (ctrl->metrics.settle_tick == 0 || ctrl->metrics.move_start_tick == 0)
        return 0;
    return ctrl->metrics.settle_tick - ctrl->metrics.move_start_tick;
}

#ifdef __cplusplus
}
#endif

#endif /* SYN_MOTOR_CTRL_H */
```


