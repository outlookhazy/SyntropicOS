/**
 * @file syn_gcode.h
 * @brief G-Code & NIST RS-274 Motion Interpreter and Controller.
 * @ingroup syn_motor
 *
 * Provides a zero-heap streaming ASCII G-code parser and motion controller:
 * - Word extraction (`G`, `M`, `X`, `Y`, `Z`, `I`, `J`, `K`, `F`, `S`, `P`, `T`).
 * - Modal state tracking (G0..G4, G17..G19, G20/G21, G90/G91, G92, M3..M5, M7..M9).
 * - Trajectory pipeline integration with `SYN_Interpolator` for coordinated multi-axis moves.
 * - Non-blocking motion step execution and cooperative protothread runner.
 */

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

/**
 * @brief Motion mode (Group 1).
 */
typedef enum {
    SYN_GCODE_MOTION_RAPID = 0,   /**< G0: Rapid linear positioning */
    SYN_GCODE_MOTION_LINEAR = 1,  /**< G1: Linear cutting feed */
    SYN_GCODE_MOTION_ARC_CW = 2,  /**< G2: Clockwise circular arc */
    SYN_GCODE_MOTION_ARC_CCW = 3, /**< G3: Counter-clockwise circular arc */
    SYN_GCODE_MOTION_DWELL = 4    /**< G4: Dwell time pause */
} SYN_GCode_MotionMode;

/**
 * @brief Distance positioning mode (Group 3).
 */
typedef enum {
    SYN_GCODE_DIST_ABSOLUTE = 0,   /**< G90: Absolute coordinates */
    SYN_GCODE_DIST_INCREMENTAL = 1 /**< G91: Relative / incremental coordinates */
} SYN_GCode_DistanceMode;

/**
 * @brief Working plane selection (Group 2).
 */
typedef enum {
    SYN_GCODE_PLANE_XY = 0, /**< G17: XY plane */
    SYN_GCODE_PLANE_XZ = 1, /**< G18: XZ plane */
    SYN_GCODE_PLANE_YZ = 2  /**< G19: YZ plane */
} SYN_GCode_Plane;

/**
 * @brief Unit system (Group 6).
 */
typedef enum {
    SYN_GCODE_UNITS_MM = 0,  /**< G21: Millimeters */
    SYN_GCODE_UNITS_INCH = 1 /**< G20: Inches */
} SYN_GCode_Units;

/**
 * @brief Spindle state (Group 7).
 */
typedef enum {
    SYN_GCODE_SPINDLE_OFF = 0, /**< M5: Spindle stopped */
    SYN_GCODE_SPINDLE_CW = 1,  /**< M3: Spindle clockwise */
    SYN_GCODE_SPINDLE_CCW = 2  /**< M4: Spindle counter-clockwise */
} SYN_GCode_SpindleMode;

/**
 * @brief Coolant state (Group 8).
 */
typedef enum {
    SYN_GCODE_COOLANT_OFF = 0,   /**< M9: Coolant off */
    SYN_GCODE_COOLANT_FLOOD = 1, /**< M8: Flood coolant on */
    SYN_GCODE_COOLANT_MIST = 2   /**< M7: Mist coolant on */
} SYN_GCode_CoolantMode;

/**
 * @brief Non-modal action commands (Group 0).
 */
typedef enum {
    SYN_GCODE_NONMODAL_NONE = 0,   /**< No non-modal action in block */
    SYN_GCODE_NONMODAL_DWELL = 4,  /**< G4: Dwell pause without altering motion mode */
    SYN_GCODE_NONMODAL_HOME = 28,  /**< G28: Return to machine home position */
    SYN_GCODE_NONMODAL_OFFSET = 92 /**< G92: Preset work coordinate system offset */
} SYN_GCode_NonModal;

/* ── Parsed Block Representation ────────────────────────────────────────── */

/**
 * @brief Parsed RS-274 G-Code line block.
 */
typedef struct {
    bool has_g;      /**< True if at least one G-code word was present */
    uint32_t g_code; /**< Last numeric G-code parsed in block */

    bool has_motion;                  /**< True if Group 1 motion G-code (G0..G3) was present */
    SYN_GCode_MotionMode motion_mode; /**< Parsed Group 1 motion mode */

    bool has_plane;        /**< True if Group 2 plane G-code (G17..G19) was present */
    SYN_GCode_Plane plane; /**< Parsed Group 2 working plane */

    bool has_dist_mode;               /**< True if Group 3 distance G-code (G90/G91) was present */
    SYN_GCode_DistanceMode dist_mode; /**< Parsed Group 3 distance mode */

    bool has_units;        /**< True if Group 6 units G-code (G20/G21) was present */
    SYN_GCode_Units units; /**< Parsed Group 6 unit system */

    bool has_non_modal;           /**< True if Group 0 non-modal G-code (G4/G28/G92) was present */
    SYN_GCode_NonModal non_modal; /**< Parsed Group 0 non-modal command */

    bool has_m;      /**< True if M-code was present */
    uint32_t m_code; /**< Numeric M-code (e.g. 3, 4, 5, 8, 9, 30) */

    bool has_x; /**< True if X word was present */
    float x;    /**< Target X coordinate */

    bool has_y; /**< True if Y word was present */
    float y;    /**< Target Y coordinate */

    bool has_z; /**< True if Z word was present */
    float z;    /**< Target Z coordinate */

    bool has_i; /**< True if I word was present */
    float i;    /**< Arc center X offset */

    bool has_j; /**< True if J word was present */
    float j;    /**< Arc center Y offset */

    bool has_k; /**< True if K word was present */
    float k;    /**< Arc center Z offset */

    bool has_f; /**< True if F word was present */
    float f;    /**< Feedrate in units/min or units/sec */

    bool has_s; /**< True if S word was present */
    float s;    /**< Spindle speed (RPM) */

    bool has_p; /**< True if P word was present */
    float p;    /**< Parameter / Dwell seconds */

    bool has_t; /**< True if T word was present */
    uint32_t t; /**< Tool select index */
} SYN_GCode_Block;

/* ── Controller & Callbacks ─────────────────────────────────────────────── */

struct SYN_GCode_Controller;

/**
 * @brief Spindle state change notification callback.
 */
typedef void (*SYN_GCode_SpindleCb)(SYN_GCode_SpindleMode mode, float rpm, void *user_data);

/**
 * @brief Coolant state change notification callback.
 */
typedef void (*SYN_GCode_CoolantCb)(SYN_GCode_CoolantMode mode, void *user_data);

/**
 * @brief Tool change notification callback.
 */
typedef void (*SYN_GCode_ToolCb)(uint32_t tool_num, void *user_data);

/**
 * @brief Dwell (G4) notification callback.
 */
typedef void (*SYN_GCode_DwellCb)(float seconds, void *user_data);

/**
 * @brief G-Code controller configuration.
 */
typedef struct {
    SYN_Interpolator *interpolator; /**< Associated motion interpolator instance */
    float default_feedrate;         /**< Initial feedrate (units/sec) */
    float max_acceleration;         /**< Max trajectory acceleration (units/sec^2) */
    float max_jerk;                 /**< Max trajectory jerk (units/sec^3) */
    float step_resolution;          /**< Step spatial resolution (e.g. 0.001) */

    SYN_GCode_SpindleCb on_spindle; /**< Spindle callback */
    SYN_GCode_CoolantCb on_coolant; /**< Coolant callback */
    SYN_GCode_ToolCb on_tool;       /**< Tool change callback */
    SYN_GCode_DwellCb on_dwell;     /**< Dwell (G4) callback */
    void *user_data;                /**< User context pointer */
} SYN_GCode_Config;

/**
 * @brief G-Code interpreter and controller instance.
 */
typedef struct SYN_GCode_Controller {
    SYN_GCode_Config cfg; /**< Controller configuration */
    SYN_PT pt;            /**< Protothread state */

    SYN_Vector3F current_pos; /**< Current machine coordinate position */
    SYN_Vector3F offset_pos;  /**< G92 Coordinate system offset */

    SYN_GCode_MotionMode motion_mode; /**< Active motion mode */
    SYN_GCode_DistanceMode dist_mode; /**< Active distance mode (G90/G91) */
    SYN_GCode_Plane plane;            /**< Active plane selection */
    SYN_GCode_Units units;            /**< Active unit system */
    SYN_GCode_SpindleMode spindle;    /**< Active spindle state */
    SYN_GCode_CoolantMode coolant;    /**< Active coolant state */

    float feedrate;       /**< Active feedrate (units/sec) */
    float spindle_speed;  /**< Active spindle speed (RPM) */
    uint32_t active_tool; /**< Active tool number */
    uint32_t dwell_steps; /**< Remaining discrete step ticks for active G4 dwell */
    uint32_t dwell_ms;    /**< Pending G4 dwell duration in milliseconds for task PT */

    bool move_in_progress; /**< Motion move currently in execution */
} SYN_GCode_Controller;

/* ── Public API ─────────────────────────────────────────────────────────── */

/**
 * @brief Initialize G-Code Controller.
 * @param ctrl Controller instance.
 * @param cfg  Configuration parameters.
 * @return SYN_OK on success, SYN_INVALID_PARAM on invalid argument.
 */
SYN_Status syn_gcode_init(SYN_GCode_Controller *ctrl, const SYN_GCode_Config *cfg);

/**
 * @brief Parse a single ASCII G-Code line into a command block.
 * @param line  Null-terminated ASCII string line.
 * @param block Output parsed block structure.
 * @return SYN_OK on success, SYN_INVALID_PARAM on malformed line.
 */
SYN_Status syn_gcode_parse_line(const char *line, SYN_GCode_Block *block);

/**
 * @brief Execute a parsed G-Code block on the controller.
 * @param ctrl  Controller instance.
 * @param block Parsed block to execute.
 * @return SYN_OK on success.
 */
SYN_Status syn_gcode_execute_block(SYN_GCode_Controller *ctrl, const SYN_GCode_Block *block);

/**
 * @brief Parse and execute an ASCII G-Code line in one step.
 * @param ctrl Controller instance.
 * @param line Null-terminated ASCII string line.
 * @return SYN_OK on success.
 */
SYN_Status syn_gcode_execute_line(SYN_GCode_Controller *ctrl, const char *line);

/**
 * @brief Step the active motion move generated by the G-code controller.
 * @param ctrl    Controller instance.
 * @param out_pos Output calculated position after step.
 * @return true if move active and advanced, false if move complete / idle.
 */
bool syn_gcode_step(SYN_GCode_Controller *ctrl, SYN_Vector3F *out_pos);

/**
 * @brief Protothread continuation function for `syn_sched`.
 * @param pt   Protothread continuation structure.
 * @param task Task handle containing SYN_GCode_Controller in user_data.
 * @return SYN_PT_Status continuation status.
 */
SYN_PT_Status syn_gcode_task_pt(SYN_PT *pt, SYN_Task *task);

#ifdef __cplusplus
}
#endif

#endif /* SYN_USE_GCODE */

#endif /* SYN_GCODE_H */
