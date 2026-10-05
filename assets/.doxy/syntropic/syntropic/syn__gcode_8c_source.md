

# File syn\_gcode.c

[**File List**](files.md) **>** [**motor**](dir_61470c21c7c2e7c9f977d1e3c2e6c7e0.md) **>** [**syn\_gcode.c**](syn__gcode_8c.md)

[Go to the documentation of this file](syn__gcode_8c.md)


```C++

#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_GCODE) || SYN_USE_GCODE

#include "syn_gcode.h"

#include <ctype.h>
#include <math.h>
#include <string.h>

#define SYN_GCODE_DEFAULT_FEEDRATE 100.0f
#define SYN_GCODE_DEFAULT_ACCEL 500.0f
#define SYN_GCODE_DEFAULT_JERK 2000.0f
#define SYN_GCODE_DEFAULT_STEP_RES 0.001f

/* ── Fast ASCII Number Scanners ─────────────────────────────────────────── */

static const char *skip_whitespace(const char *p)
{
    while (*p != '\0' && (isspace((unsigned char)*p) || *p == '\r' || *p == '\n')) {
        p++;
    }
    return p;
}

static const char *parse_float_val(const char *p, float *out_val)
{
    p = skip_whitespace(p);
    float sign = 1.0f;
    if (*p == '-') {
        sign = -1.0f;
        p++;
    } else if (*p == '+') {
        p++;
    }

    float int_part = 0.0f;
    bool has_digits = false;
    while (*p >= '0' && *p <= '9') {
        int_part = (int_part * 10.0f) + (float)(*p - '0');
        has_digits = true;
        p++;
    }

    float frac_part = 0.0f;
    float div = 1.0f;
    if (*p == '.') {
        p++;
        while (*p >= '0' && *p <= '9') {
            frac_part = (frac_part * 10.0f) + (float)(*p - '0');
            div *= 10.0f;
            has_digits = true;
            p++;
        }
    }

    if (!has_digits) {
        return NULL;
    }

    *out_val = sign * (int_part + (frac_part / div));
    return p;
}

static const char *parse_uint_val(const char *p, uint32_t *out_val)
{
    p = skip_whitespace(p);
    uint32_t val = 0U;
    bool has_digits = false;
    while (*p >= '0' && *p <= '9') {
        val = (val * 10U) + (uint32_t)(*p - '0');
        has_digits = true;
        p++;
    }
    if (!has_digits) {
        return NULL;
    }
    *out_val = val;
    return p;
}

/* ── Public API Implementation ───────────────────────────────────────────── */

SYN_Status syn_gcode_init(SYN_GCode_Controller *ctrl, const SYN_GCode_Config *cfg)
{
    if (ctrl == NULL) {
        return SYN_INVALID_PARAM;
    }

    (void)memset(ctrl, 0, sizeof(*ctrl));
    if (cfg != NULL) {
        ctrl->cfg = *cfg;
    }

    if (ctrl->cfg.default_feedrate <= 0.0f) {
        ctrl->cfg.default_feedrate = SYN_GCODE_DEFAULT_FEEDRATE;
    }
    if (ctrl->cfg.max_acceleration <= 0.0f) {
        ctrl->cfg.max_acceleration = SYN_GCODE_DEFAULT_ACCEL;
    }
    if (ctrl->cfg.max_jerk <= 0.0f) {
        ctrl->cfg.max_jerk = SYN_GCODE_DEFAULT_JERK;
    }
    if (ctrl->cfg.step_resolution <= 0.0f) {
        ctrl->cfg.step_resolution = SYN_GCODE_DEFAULT_STEP_RES;
    }

    ctrl->feedrate = ctrl->cfg.default_feedrate;
    ctrl->motion_mode = SYN_GCODE_MOTION_RAPID;
    ctrl->dist_mode = SYN_GCODE_DIST_ABSOLUTE;
    ctrl->plane = SYN_GCODE_PLANE_XY;
    ctrl->units = SYN_GCODE_UNITS_MM;
    ctrl->spindle = SYN_GCODE_SPINDLE_OFF;
    ctrl->coolant = SYN_GCODE_COOLANT_OFF;

    if (ctrl->cfg.interpolator != NULL) {
        syn_interpolator_init(ctrl->cfg.interpolator);
    }

    return SYN_OK;
}

SYN_Status syn_gcode_parse_line(const char *line, SYN_GCode_Block *block)
{
    if (line == NULL || block == NULL) {
        return SYN_INVALID_PARAM;
    }

    (void)memset(block, 0, sizeof(*block));

    const char *p = line;
    while (*p != '\0') {
        p = skip_whitespace(p);
        if (*p == '\0' || *p == ';') {
            break; /* End of line or comment */
        }

        if (*p == '(') {
            /* Skip parenthesized comment */
            while (*p != '\0' && *p != ')') {
                p++;
            }
            if (*p == ')') {
                p++;
            }
            continue;
        }

        char letter = (char)toupper((unsigned char)*p);
        p++;

        switch (letter) {
        case 'G': {
            uint32_t val = 0U;
            p = parse_uint_val(p, &val);
            if (p != NULL) {
                block->has_g = true;
                block->g_code = val;
            }
            break;
        }
        case 'M': {
            uint32_t val = 0U;
            p = parse_uint_val(p, &val);
            if (p != NULL) {
                block->has_m = true;
                block->m_code = val;
            }
            break;
        }
        case 'X':
            p = parse_float_val(p, &block->x);
            if (p != NULL) {
                block->has_x = true;
            }
            break;
        case 'Y':
            p = parse_float_val(p, &block->y);
            if (p != NULL) {
                block->has_y = true;
            }
            break;
        case 'Z':
            p = parse_float_val(p, &block->z);
            if (p != NULL) {
                block->has_z = true;
            }
            break;
        case 'I':
            p = parse_float_val(p, &block->i);
            if (p != NULL) {
                block->has_i = true;
            }
            break;
        case 'J':
            p = parse_float_val(p, &block->j);
            if (p != NULL) {
                block->has_j = true;
            }
            break;
        case 'K':
            p = parse_float_val(p, &block->k);
            if (p != NULL) {
                block->has_k = true;
            }
            break;
        case 'F':
            p = parse_float_val(p, &block->f);
            if (p != NULL) {
                block->has_f = true;
            }
            break;
        case 'S':
            p = parse_float_val(p, &block->s);
            if (p != NULL) {
                block->has_s = true;
            }
            break;
        case 'P':
            p = parse_float_val(p, &block->p);
            if (p != NULL) {
                block->has_p = true;
            }
            break;
        case 'T': {
            uint32_t val = 0U;
            p = parse_uint_val(p, &val);
            if (p != NULL) {
                block->has_t = true;
                block->t = val;
            }
            break;
        }
        default:
            /* Skip unknown characters */
            break;
        }

        if (p == NULL) {
            return SYN_INVALID_PARAM;
        }
    }

    return SYN_OK;
}

SYN_Status syn_gcode_execute_block(SYN_GCode_Controller *ctrl, const SYN_GCode_Block *block)
{
    if (ctrl == NULL || block == NULL) {
        return SYN_INVALID_PARAM;
    }

    /* 1. Process Feedrate & Spindle Speed updates */
    if (block->has_f && block->f > 0.0f) {
        ctrl->feedrate = block->f;
    }
    if (block->has_s && block->s >= 0.0f) {
        ctrl->spindle_speed = block->s;
    }
    if (block->has_t) {
        ctrl->active_tool = block->t;
        if (ctrl->cfg.on_tool != NULL) {
            ctrl->cfg.on_tool(block->t, ctrl->cfg.user_data);
        }
    }

    /* 2. Process M-Codes */
    if (block->has_m) {
        switch (block->m_code) {
        case 3: /* Spindle CW */
            ctrl->spindle = SYN_GCODE_SPINDLE_CW;
            if (ctrl->cfg.on_spindle != NULL) {
                ctrl->cfg.on_spindle(SYN_GCODE_SPINDLE_CW, ctrl->spindle_speed,
                                     ctrl->cfg.user_data);
            }
            break;
        case 4: /* Spindle CCW */
            ctrl->spindle = SYN_GCODE_SPINDLE_CCW;
            if (ctrl->cfg.on_spindle != NULL) {
                ctrl->cfg.on_spindle(SYN_GCODE_SPINDLE_CCW, ctrl->spindle_speed,
                                     ctrl->cfg.user_data);
            }
            break;
        case 5: /* Spindle Stop */
            ctrl->spindle = SYN_GCODE_SPINDLE_OFF;
            if (ctrl->cfg.on_spindle != NULL) {
                ctrl->cfg.on_spindle(SYN_GCODE_SPINDLE_OFF, 0.0f, ctrl->cfg.user_data);
            }
            break;
        case 7: /* Coolant Mist */
            ctrl->coolant = SYN_GCODE_COOLANT_MIST;
            if (ctrl->cfg.on_coolant != NULL) {
                ctrl->cfg.on_coolant(SYN_GCODE_COOLANT_MIST, ctrl->cfg.user_data);
            }
            break;
        case 8: /* Coolant Flood */
            ctrl->coolant = SYN_GCODE_COOLANT_FLOOD;
            if (ctrl->cfg.on_coolant != NULL) {
                ctrl->cfg.on_coolant(SYN_GCODE_COOLANT_FLOOD, ctrl->cfg.user_data);
            }
            break;
        case 9: /* Coolant Off */
            ctrl->coolant = SYN_GCODE_COOLANT_OFF;
            if (ctrl->cfg.on_coolant != NULL) {
                ctrl->cfg.on_coolant(SYN_GCODE_COOLANT_OFF, ctrl->cfg.user_data);
            }
            break;
        default:
            break;
        }
    }

    /* 3. Process G-Codes (Modal state updates) */
    if (block->has_g) {
        switch (block->g_code) {
        case 0:
            ctrl->motion_mode = SYN_GCODE_MOTION_RAPID;
            break;
        case 1:
            ctrl->motion_mode = SYN_GCODE_MOTION_LINEAR;
            break;
        case 2:
            ctrl->motion_mode = SYN_GCODE_MOTION_ARC_CW;
            break;
        case 3:
            ctrl->motion_mode = SYN_GCODE_MOTION_ARC_CCW;
            break;
        case 4:
            ctrl->motion_mode = SYN_GCODE_MOTION_DWELL;
            break;
        case 17:
            ctrl->plane = SYN_GCODE_PLANE_XY;
            break;
        case 18:
            ctrl->plane = SYN_GCODE_PLANE_XZ;
            break;
        case 19:
            ctrl->plane = SYN_GCODE_PLANE_YZ;
            break;
        case 20:
            ctrl->units = SYN_GCODE_UNITS_INCH;
            break;
        case 21:
            ctrl->units = SYN_GCODE_UNITS_MM;
            break;
        case 90:
            ctrl->dist_mode = SYN_GCODE_DIST_ABSOLUTE;
            break;
        case 91:
            ctrl->dist_mode = SYN_GCODE_DIST_INCREMENTAL;
            break;
        case 92:
            /* Coordinate system preset / offset */
            if (block->has_x) {
                ctrl->offset_pos.x = ctrl->current_pos.x - block->x;
            }
            if (block->has_y) {
                ctrl->offset_pos.y = ctrl->current_pos.y - block->y;
            }
            if (block->has_z) {
                ctrl->offset_pos.z = ctrl->current_pos.z - block->z;
            }
            return SYN_OK;
        default:
            break;
        }
    }

    /* 4. Motion Execution */
    if (block->has_x || block->has_y || block->has_z || block->has_i || block->has_j ||
        block->has_k) {
        SYN_Vector3F target = ctrl->current_pos;

        if (ctrl->dist_mode == SYN_GCODE_DIST_ABSOLUTE) {
            if (block->has_x) {
                target.x = block->x + ctrl->offset_pos.x;
            }
            if (block->has_y) {
                target.y = block->y + ctrl->offset_pos.y;
            }
            if (block->has_z) {
                target.z = block->z + ctrl->offset_pos.z;
            }
        } else {
            /* Incremental */
            if (block->has_x) {
                target.x += block->x;
            }
            if (block->has_y) {
                target.y += block->y;
            }
            if (block->has_z) {
                target.z += block->z;
            }
        }

        if (ctrl->cfg.interpolator != NULL) {
            if (ctrl->motion_mode == SYN_GCODE_MOTION_RAPID ||
                ctrl->motion_mode == SYN_GCODE_MOTION_LINEAR) {
                SYN_Status st = syn_interpolator_plan_linear(
                    ctrl->cfg.interpolator, ctrl->current_pos, target, ctrl->feedrate,
                    ctrl->cfg.max_acceleration, ctrl->cfg.max_jerk, ctrl->cfg.step_resolution);
                if (st == SYN_OK) {
                    ctrl->move_in_progress = true;
                }
            } else if (ctrl->motion_mode == SYN_GCODE_MOTION_ARC_CW ||
                       ctrl->motion_mode == SYN_GCODE_MOTION_ARC_CCW) {
                SYN_Vector3F center = {
                    .x = block->has_i ? block->i : 0.0f,
                    .y = block->has_j ? block->j : 0.0f,
                    .z = block->has_k ? block->k : 0.0f,
                };
                bool is_cw = (ctrl->motion_mode == SYN_GCODE_MOTION_ARC_CW);
                SYN_Status st = syn_interpolator_plan_circular(
                    ctrl->cfg.interpolator, ctrl->current_pos, target, center, is_cw,
                    ctrl->feedrate, ctrl->cfg.max_acceleration, ctrl->cfg.max_jerk,
                    ctrl->cfg.step_resolution);
                if (st == SYN_OK) {
                    ctrl->move_in_progress = true;
                }
            }
        } else {
            ctrl->current_pos = target;
        }
    }

    return SYN_OK;
}

SYN_Status syn_gcode_execute_line(SYN_GCode_Controller *ctrl, const char *line)
{
    if (ctrl == NULL || line == NULL) {
        return SYN_INVALID_PARAM;
    }
    SYN_GCode_Block block;
    SYN_Status st = syn_gcode_parse_line(line, &block);
    if (st != SYN_OK) {
        return st;
    }
    return syn_gcode_execute_block(ctrl, &block);
}

bool syn_gcode_step(SYN_GCode_Controller *ctrl, SYN_Vector3F *out_pos)
{
    if (ctrl == NULL) {
        return false;
    }

    if (ctrl->cfg.interpolator != NULL && ctrl->move_in_progress) {
        bool active = syn_interpolator_step(ctrl->cfg.interpolator, &ctrl->current_pos);
        if (out_pos != NULL) {
            *out_pos = ctrl->current_pos;
        }
        if (!active) {
            ctrl->move_in_progress = false;
        }
        return active;
    }

    if (out_pos != NULL) {
        *out_pos = ctrl->current_pos;
    }
    return false;
}

SYN_PT_Status syn_gcode_task_pt(SYN_PT *pt, SYN_Task *task)
{
    if (pt == NULL || task == NULL || task->user_data == NULL) {
        return PT_ENDED;
    }

    SYN_GCode_Controller *ctrl = (SYN_GCode_Controller *)task->user_data;

    PT_BEGIN(pt);

    for (;;) {
        if (ctrl->move_in_progress) {
            SYN_Vector3F pos;
            (void)syn_gcode_step(ctrl, &pos);
        }
        PT_YIELD(pt);
    }

    PT_END(pt); /* LCOV_EXCL_LINE: Infinite task loop end */
}

#endif /* SYN_USE_GCODE */
```


