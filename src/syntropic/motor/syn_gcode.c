/**
 * @file syn_gcode.c
 * @brief G-Code & NIST RS-274 Motion Interpreter and Controller Implementation.
 */

#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_GCODE) || SYN_USE_GCODE

#include "syn_gcode.h"

#include <ctype.h>
#include <math.h>
#include <string.h>

/** @brief Default feedrate in mm/min */
#define SYN_GCODE_DEFAULT_FEEDRATE 100.0f
/** @brief Default acceleration in mm/s^2 */
#define SYN_GCODE_DEFAULT_ACCEL 500.0f
/** @brief Default jerk in mm/s^3 */
#define SYN_GCODE_DEFAULT_JERK 2000.0f
/** @brief Default step resolution in seconds */
#define SYN_GCODE_DEFAULT_STEP_RES 0.001f
/** @brief Seconds per minute conversion factor for G94 units/min feedrate */
#define SYN_GCODE_SEC_PER_MIN 60.0f
/** @brief Millimeters per inch conversion factor for G20 */
#define SYN_GCODE_MM_PER_INCH 25.4f
/** @brief Maximum significant fractional digits accumulated in float scanner */
#define SYN_GCODE_MAX_FRAC_DIGITS 9U
/** @brief Maximum exact single-precision float representable in uint32_t */
#define SYN_GCODE_MAX_U32_FLOAT 4294967040.0f

/* ── Fast ASCII Number Scanners ─────────────────────────────────────────── */

/**
 * @brief Skip whitespace characters in G-code string.
 * @param p Pointer to input string.
 * @return Pointer to next non-whitespace character.
 */
static const char *skip_whitespace(const char *p)
{
    while (*p != '\0' && (isspace((unsigned char)*p) || *p == '\r' || *p == '\n')) {
        p++;
    }
    return p;
}

/**
 * @brief Scan optional decimal fraction digits, capping float accumulation to avoid Inf/Inf NaN.
 * @param p          Pointer positioned at optional '.' character.
 * @param has_digits Updated to true if at least one fractional digit is consumed.
 * @param out_frac   Receives fractional value in [0.0f, 1.0f).
 * @return Pointer past any fractional digits.
 */
static const char *parse_frac_part(const char *p, bool *has_digits, float *out_frac)
{
    float frac_part = 0.0f;
    float div = 1.0f;
    uint8_t frac_digits = 0U;

    if (*p == '.') {
        p++;
        while ((*p >= '0') && (*p <= '9')) {
            if (frac_digits < SYN_GCODE_MAX_FRAC_DIGITS) {
                frac_part = (frac_part * 10.0f) + (float)(*p - '0');
                div *= 10.0f;
                frac_digits++;
            }
            *has_digits = true;
            p++;
        }
    }

    *out_frac = frac_part / div;
    return p;
}

/**
 * @brief Parse floating point number from G-code word.
 * @param p Pointer to number start.
 * @param out_val Pointer to receive parsed float.
 * @return Pointer after parsed number, or NULL on parse error.
 */
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
    while ((*p >= '0') && (*p <= '9')) {
        int_part = (int_part * 10.0f) + (float)(*p - '0');
        has_digits = true;
        p++;
    }

    float frac_val = 0.0f;
    p = parse_frac_part(p, &has_digits, &frac_val);

    float res = sign * (int_part + frac_val);
    if (!has_digits || !isfinite(res)) {
        return NULL;
    }

    *out_val = res;
    return p;
}

/**
 * @brief Parse unsigned integer from G-code word.
 * @param p Pointer to integer start.
 * @param out_val Pointer to receive parsed unsigned integer.
 * @return Pointer after parsed integer, or NULL on parse error.
 */
static const char *parse_uint_val(const char *p, uint32_t *out_val)
{
    p = skip_whitespace(p);
    uint32_t val = 0U;
    bool has_digits = false;
    while (*p >= '0' && *p <= '9') {
        uint32_t digit = (uint32_t)(*p - '0');
        if (val > (UINT32_MAX - digit) / 10U) {
            return NULL;
        }
        val = (val * 10U) + digit;
        has_digits = true;
        p++;
    }
    if (!has_digits) {
        return NULL;
    }
    *out_val = val;
    return p;
}

/**
 * @brief Classify numeric G-code into its RS-274 modal or non-modal group on block.
 * @param block  Target parsed block.
 * @param g_code Numeric G-code value.
 * @return true if g_code is a supported RS-274 G-code, false otherwise.
 */
static bool gcode_classify_g_word(SYN_GCode_Block *block, uint32_t g_code)
{
    if (g_code <= 3U) {
        block->has_motion = true;
        block->motion_mode = (SYN_GCode_MotionMode)g_code;
        return true;
    }
    if (g_code >= 17U && g_code <= 19U) {
        block->has_plane = true;
        block->plane = (SYN_GCode_Plane)(g_code - 17U);
        return true;
    }
    if (g_code == 20U || g_code == 21U) {
        block->has_units = true;
        block->units = (g_code == 20U) ? SYN_GCODE_UNITS_INCH : SYN_GCODE_UNITS_MM;
        return true;
    }
    if (g_code == 90U || g_code == 91U) {
        block->has_dist_mode = true;
        block->dist_mode = (g_code == 91U) ? SYN_GCODE_DIST_INCREMENTAL : SYN_GCODE_DIST_ABSOLUTE;
        return true;
    }
    if (g_code == 4U || g_code == 28U || g_code == 92U) {
        block->has_non_modal = true;
        block->non_modal = (SYN_GCode_NonModal)g_code;
        return true;
    }
    return (g_code == 94U);
}

/**
 * @brief Check whether numeric M-code is supported by the interpreter.
 * @param m_code Numeric M-code value.
 * @return true if supported (M0..M9, M30), false otherwise.
 */
static bool gcode_is_valid_m_code(uint32_t m_code)
{
    return (m_code <= 9U) || (m_code == 30U);
}

/**
 * @brief Parse coordinate or arc-offset float word (X, Y, Z, I, J, K).
 * @param letter    Uppercase word letter.
 * @param p         Pointer to numeric value text.
 * @param block     Target parsed block.
 * @param out_match Set true if letter was handled by this helper.
 * @return Updated scan pointer, or NULL on numeric parse error.
 */
static const char *gcode_parse_coord_word(char letter, const char *p, SYN_GCode_Block *block,
                                          bool *out_match)
{
    float *val_ptr = NULL;
    bool *flag_ptr = NULL;
    *out_match = true;

    switch (letter) {
    case 'X':
        val_ptr = &block->x;
        flag_ptr = &block->has_x;
        break;
    case 'Y':
        val_ptr = &block->y;
        flag_ptr = &block->has_y;
        break;
    case 'Z':
        val_ptr = &block->z;
        flag_ptr = &block->has_z;
        break;
    case 'I':
        val_ptr = &block->i;
        flag_ptr = &block->has_i;
        break;
    case 'J':
        val_ptr = &block->j;
        flag_ptr = &block->has_j;
        break;
    case 'K':
        val_ptr = &block->k;
        flag_ptr = &block->has_k;
        break;
    default:
        *out_match = false;
        return p;
    }

    p = parse_float_val(p, val_ptr);
    if (p != NULL) {
        *flag_ptr = true;
    }
    return p;
}

/**
 * @brief Parse float parameter word (F, S, P) with non-negative validation.
 * @param letter    Uppercase word letter.
 * @param p         Pointer to numeric value text.
 * @param block     Target parsed block.
 * @param out_match Set true if letter was handled by this helper.
 * @return Updated scan pointer, or NULL on numeric or range validation error.
 */
static const char *gcode_parse_param_word(char letter, const char *p, SYN_GCode_Block *block,
                                          bool *out_match)
{
    *out_match = true;
    if (letter == 'F') {
        p = parse_float_val(p, &block->f);
        if (p == NULL || block->f <= 0.0f) {
            return NULL;
        }
        block->has_f = true;
        return p;
    }
    if (letter == 'S') {
        p = parse_float_val(p, &block->s);
        if (p == NULL || block->s < 0.0f) {
            return NULL;
        }
        block->has_s = true;
        return p;
    }
    if (letter == 'P') {
        p = parse_float_val(p, &block->p);
        if (p == NULL || block->p < 0.0f) {
            return NULL;
        }
        block->has_p = true;
        return p;
    }
    *out_match = false;
    return p;
}

/**
 * @brief Parse command or line-number integer word (G, M, T, N).
 * @param letter Uppercase word letter.
 * @param p      Pointer to numeric value text.
 * @param block  Target parsed block.
 * @return Updated scan pointer, or NULL on invalid/unsupported word.
 */
static const char *gcode_parse_cmd_word(char letter, const char *p, SYN_GCode_Block *block)
{
    bool matched = false;
    p = gcode_parse_param_word(letter, p, block, &matched);
    if (matched) {
        return p;
    }

    uint32_t uval = 0U;
    if (letter == 'G') {
        p = parse_uint_val(p, &uval);
        if (p == NULL || !gcode_classify_g_word(block, uval)) {
            return NULL;
        }
        block->has_g = true;
        block->g_code = uval;
        return p;
    }
    if (letter == 'M') {
        p = parse_uint_val(p, &uval);
        if (p == NULL || !gcode_is_valid_m_code(uval)) {
            return NULL;
        }
        block->has_m = true;
        block->m_code = uval;
        return p;
    }
    if (letter == 'T') {
        p = parse_uint_val(p, &uval);
        if (p != NULL) {
            block->has_t = true;
            block->t = uval;
        }
        return p;
    }
    if (letter == 'N') {
        return parse_uint_val(p, &uval);
    }
    return NULL;
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

    if (!isfinite(ctrl->cfg.default_feedrate) || (ctrl->cfg.default_feedrate <= 0.0f)) {
        ctrl->cfg.default_feedrate = SYN_GCODE_DEFAULT_FEEDRATE / SYN_GCODE_SEC_PER_MIN;
    }
    if (!isfinite(ctrl->cfg.max_acceleration) || (ctrl->cfg.max_acceleration <= 0.0f)) {
        ctrl->cfg.max_acceleration = SYN_GCODE_DEFAULT_ACCEL;
    }
    if (!isfinite(ctrl->cfg.max_jerk) || (ctrl->cfg.max_jerk <= 0.0f)) {
        ctrl->cfg.max_jerk = SYN_GCODE_DEFAULT_JERK;
    }
    if (!isfinite(ctrl->cfg.step_resolution) || (ctrl->cfg.step_resolution <= 0.0f)) {
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

        bool matched = false;
        p = gcode_parse_coord_word(letter, p, block, &matched);
        if (!matched) {
            p = gcode_parse_cmd_word(letter, p, block);
        }
        if (p == NULL) {
            return SYN_INVALID_PARAM;
        }
    }

    return SYN_OK;
}

/**
 * @brief Dispatch M-code spindle and coolant state updates.
 * @param ctrl   Controller instance.
 * @param m_code Numeric M-code value.
 */
static void gcode_apply_m_code(SYN_GCode_Controller *ctrl, uint32_t m_code)
{
    if (m_code >= 3U && m_code <= 5U) {
        SYN_GCode_SpindleMode mode = SYN_GCODE_SPINDLE_OFF;
        float rpm = 0.0f;
        if (m_code == 3U) {
            mode = SYN_GCODE_SPINDLE_CW;
            rpm = ctrl->spindle_speed;
        } else if (m_code == 4U) {
            mode = SYN_GCODE_SPINDLE_CCW;
            rpm = ctrl->spindle_speed;
        }
        ctrl->spindle = mode;
        if (ctrl->cfg.on_spindle != NULL) {
            ctrl->cfg.on_spindle(mode, rpm, ctrl->cfg.user_data);
        }
    } else if (m_code >= 7U && m_code <= 9U) {
        SYN_GCode_CoolantMode mode = SYN_GCODE_COOLANT_OFF;
        if (m_code == 7U) {
            mode = SYN_GCODE_COOLANT_MIST;
        } else if (m_code == 8U) {
            mode = SYN_GCODE_COOLANT_FLOOD;
        }
        ctrl->coolant = mode;
        if (ctrl->cfg.on_coolant != NULL) {
            ctrl->cfg.on_coolant(mode, ctrl->cfg.user_data);
        }
    }
}

/**
 * @brief Plan linear move on interpolator or step directly when interpolator is NULL.
 * @param ctrl   Controller instance.
 * @param target Target machine coordinate position in millimeters.
 */
static void gcode_plan_or_jump_linear(SYN_GCode_Controller *ctrl, SYN_Vector3F target)
{
    ctrl->dwell_steps = 0U;
    ctrl->dwell_ms = 0U;
    if (ctrl->cfg.interpolator != NULL) {
        SYN_Status st = syn_interpolator_plan_linear(
            ctrl->cfg.interpolator, ctrl->current_pos, target, ctrl->feedrate,
            ctrl->cfg.max_acceleration, ctrl->cfg.max_jerk, ctrl->cfg.step_resolution);
        if (st == SYN_OK) {
            ctrl->move_in_progress = true;
        }
    } else {
        ctrl->current_pos = target;
    }
}

/**
 * @brief Return specified axes (or all axes when none specified) to machine home (0, 0, 0).
 * @param ctrl  Controller instance.
 * @param block Effective parsed block.
 */
static void gcode_execute_home(SYN_GCode_Controller *ctrl, const SYN_GCode_Block *block)
{
    SYN_Vector3F home = ctrl->current_pos;
    bool any_axis = block->has_x || block->has_y || block->has_z;
    if (!any_axis || block->has_x) {
        home.x = 0.0f;
    }
    if (!any_axis || block->has_y) {
        home.y = 0.0f;
    }
    if (!any_axis || block->has_z) {
        home.z = 0.0f;
    }
    gcode_plan_or_jump_linear(ctrl, home);
}

/**
 * @brief Convert a positive float count to uint32_t with rounding and saturation.
 * @param val Positive float count.
 * @return Rounded count in [1U, UINT32_MAX].
 */
static uint32_t gcode_round_pos_u32(float val)
{
    if (!isfinite(val) || (val >= SYN_GCODE_MAX_U32_FLOAT)) {
        return UINT32_MAX;
    }
    uint32_t rounded = (uint32_t)(val + 0.5f);
    return (rounded > 0U) ? rounded : 1U;
}

/**
 * @brief Execute G4 non-modal dwell command.
 * @param ctrl  Controller instance.
 * @param block Effective parsed block.
 */
static void gcode_execute_dwell(SYN_GCode_Controller *ctrl, const SYN_GCode_Block *block)
{
    float dwell_s = block->has_p ? block->p : 0.0f;
    if (!isfinite(dwell_s) || (dwell_s < 0.0f)) {
        dwell_s = 0.0f;
    }
    if (ctrl->cfg.on_dwell != NULL) {
        ctrl->cfg.on_dwell(dwell_s, ctrl->cfg.user_data);
    }
    if ((dwell_s > 0.0f) && isfinite(ctrl->cfg.step_resolution) &&
        (ctrl->cfg.step_resolution > 0.0f)) {
        ctrl->dwell_steps = gcode_round_pos_u32(dwell_s / ctrl->cfg.step_resolution);
        ctrl->dwell_ms = gcode_round_pos_u32(dwell_s * 1000.0f);
    } else {
        ctrl->dwell_steps = 0U;
        ctrl->dwell_ms = 0U;
    }
}

/**
 * @brief Execute Group 0 non-modal action (G4 dwell, G28 home, G92 offset).
 * @param ctrl  Controller instance.
 * @param block Effective parsed block.
 * @param scale Active unit conversion factor to millimeters.
 * @return true if non-modal command consumed the block's coordinate words.
 */
static bool gcode_execute_non_modal(SYN_GCode_Controller *ctrl, const SYN_GCode_Block *block,
                                    float scale)
{
    if (!block->has_non_modal || block->non_modal == SYN_GCODE_NONMODAL_NONE) {
        return false;
    }
    if (block->non_modal == SYN_GCODE_NONMODAL_OFFSET) {
        float ox = block->has_x ? (ctrl->current_pos.x - (block->x * scale)) : ctrl->offset_pos.x;
        float oy = block->has_y ? (ctrl->current_pos.y - (block->y * scale)) : ctrl->offset_pos.y;
        float oz = block->has_z ? (ctrl->current_pos.z - (block->z * scale)) : ctrl->offset_pos.z;
        if (isfinite(ox) && isfinite(oy) && isfinite(oz)) {
            ctrl->offset_pos.x = ox;
            ctrl->offset_pos.y = oy;
            ctrl->offset_pos.z = oz;
        }
    } else if (block->non_modal == SYN_GCODE_NONMODAL_HOME) {
        gcode_execute_home(ctrl, block);
    } else if (block->non_modal == SYN_GCODE_NONMODAL_DWELL) {
        gcode_execute_dwell(ctrl, block);
    }
    return true;
}

/**
 * @brief Compute target machine position in millimeters for absolute or incremental mode.
 * @param ctrl  Controller instance.
 * @param block Effective parsed block.
 * @param scale Active unit conversion factor to millimeters.
 * @return Target machine position in millimeters.
 */
static SYN_Vector3F gcode_compute_target(const SYN_GCode_Controller *ctrl,
                                         const SYN_GCode_Block *block, float scale)
{
    SYN_Vector3F target = ctrl->current_pos;
    if (ctrl->dist_mode == SYN_GCODE_DIST_ABSOLUTE) {
        if (block->has_x) {
            target.x = (block->x * scale) + ctrl->offset_pos.x;
        }
        if (block->has_y) {
            target.y = (block->y * scale) + ctrl->offset_pos.y;
        }
        if (block->has_z) {
            target.z = (block->z * scale) + ctrl->offset_pos.z;
        }
    } else {
        if (block->has_x) {
            target.x += block->x * scale;
        }
        if (block->has_y) {
            target.y += block->y * scale;
        }
        if (block->has_z) {
            target.z += block->z * scale;
        }
    }
    return target;
}

/**
 * @brief Check whether block contains any axis coordinate or arc center offset word.
 * @param block Effective parsed block.
 * @return true if any of X, Y, Z, I, J, K is present.
 */
static bool gcode_has_motion_coords(const SYN_GCode_Block *block)
{
    return block->has_x || block->has_y || block->has_z || block->has_i || block->has_j ||
           block->has_k;
}

/**
 * @brief Execute Group 1 linear or circular motion move if coordinate words are present.
 * @param ctrl  Controller instance.
 * @param block Effective parsed block.
 * @param scale Active unit conversion factor to millimeters.
 */
static void gcode_execute_motion(SYN_GCode_Controller *ctrl, const SYN_GCode_Block *block,
                                 float scale)
{
    if (!gcode_has_motion_coords(block)) {
        return;
    }

    ctrl->dwell_steps = 0U;
    ctrl->dwell_ms = 0U;
    SYN_Vector3F target = gcode_compute_target(ctrl, block, scale);
    if (!isfinite(target.x) || !isfinite(target.y) || !isfinite(target.z)) {
        return;
    }
    if (ctrl->cfg.interpolator == NULL) {
        ctrl->current_pos = target;
        return;
    }

    if (ctrl->motion_mode == SYN_GCODE_MOTION_RAPID ||
        ctrl->motion_mode == SYN_GCODE_MOTION_LINEAR) {
        gcode_plan_or_jump_linear(ctrl, target);
    } else if (ctrl->motion_mode == SYN_GCODE_MOTION_ARC_CW ||
               ctrl->motion_mode == SYN_GCODE_MOTION_ARC_CCW) {
        SYN_Vector3F center = {
            .x = block->has_i ? (block->i * scale) : 0.0f,
            .y = block->has_j ? (block->j * scale) : 0.0f,
            .z = block->has_k ? (block->k * scale) : 0.0f,
        };
        bool is_cw = (ctrl->motion_mode == SYN_GCODE_MOTION_ARC_CW);
        SYN_Status st = syn_interpolator_plan_circular(
            ctrl->cfg.interpolator, ctrl->current_pos, target, center, is_cw, ctrl->feedrate,
            ctrl->cfg.max_acceleration, ctrl->cfg.max_jerk, ctrl->cfg.step_resolution);
        if (st == SYN_OK) {
            ctrl->move_in_progress = true;
        }
    }
}

/**
 * @brief Apply modal group state transitions (units, plane, distance, motion) to controller.
 * @param ctrl  Controller instance.
 * @param block Effective parsed block.
 */
static void gcode_apply_modal_groups(SYN_GCode_Controller *ctrl, const SYN_GCode_Block *block)
{
    if (block->has_units) {
        ctrl->units = block->units;
    }
    if (block->has_plane) {
        ctrl->plane = block->plane;
    }
    if (block->has_dist_mode) {
        ctrl->dist_mode = block->dist_mode;
    }
    if (block->has_motion) {
        ctrl->motion_mode = block->motion_mode;
    }
}

/**
 * @brief Check whether block already has any RS-274 modal or non-modal group flag populated.
 * @param block Parsed block.
 * @return true if at least one group flag is set.
 */
static bool gcode_has_classified_group(const SYN_GCode_Block *block)
{
    return block->has_motion || block->has_plane || block->has_dist_mode || block->has_units ||
           block->has_non_modal;
}

/**
 * @brief Apply feedrate, spindle speed, tool selection, and M-code words from block.
 * @param ctrl  Controller instance.
 * @param block Effective parsed block.
 * @param scale Active unit conversion factor to millimeters.
 */
static void gcode_apply_aux_words(SYN_GCode_Controller *ctrl, const SYN_GCode_Block *block,
                                  float scale)
{
    if (block->has_f && isfinite(block->f) && (block->f > 0.0f)) {
        /* Convert G94 units/min feedrate to mm/sec */
        float feed_mm_s = (block->f * scale) / SYN_GCODE_SEC_PER_MIN;
        if (isfinite(feed_mm_s)) {
            ctrl->feedrate = feed_mm_s;
        }
    }
    if (block->has_s && isfinite(block->s) && (block->s >= 0.0f)) {
        ctrl->spindle_speed = block->s;
    }
    if (block->has_t) {
        ctrl->active_tool = block->t;
        if (ctrl->cfg.on_tool != NULL) {
            ctrl->cfg.on_tool(block->t, ctrl->cfg.user_data);
        }
    }
    if (block->has_m) {
        gcode_apply_m_code(ctrl, block->m_code);
    }
}

/**
 * @brief Verify that all present X/Y/Z/I/J/K axis words in @p block hold finite float values.
 * @param block Parsed G-Code block to validate.
 * @return true if every present coordinate/offset word is finite, false otherwise.
 */
static bool gcode_block_coords_finite(const SYN_GCode_Block *block)
{
    if ((block->has_x && !isfinite(block->x)) || (block->has_y && !isfinite(block->y)) ||
        (block->has_z && !isfinite(block->z))) {
        return false;
    }
    return (!block->has_i || isfinite(block->i)) && (!block->has_j || isfinite(block->j)) &&
           (!block->has_k || isfinite(block->k));
}

SYN_Status syn_gcode_execute_block(SYN_GCode_Controller *ctrl, const SYN_GCode_Block *block)
{
    if (ctrl == NULL || block == NULL) {
        return SYN_INVALID_PARAM;
    }

    SYN_GCode_Block eff = *block;
    if (!gcode_block_coords_finite(&eff)) {
        return SYN_INVALID_PARAM;
    }
    if (eff.has_g && !gcode_has_classified_group(&eff)) {
        if (!gcode_classify_g_word(&eff, eff.g_code)) {
            return SYN_INVALID_PARAM;
        }
    }
    if (eff.has_m && !gcode_is_valid_m_code(eff.m_code)) {
        return SYN_INVALID_PARAM;
    }

    /* 1. Apply RS-274 modal groups (units/plane/distance/motion) before scaling F or axes */
    gcode_apply_modal_groups(ctrl, &eff);
    float scale = (ctrl->units == SYN_GCODE_UNITS_INCH) ? SYN_GCODE_MM_PER_INCH : 1.0f;

    /* 2. Process Feedrate, Spindle Speed, Tool & M-Codes */
    gcode_apply_aux_words(ctrl, &eff, scale);

    /* 3. Execute Group 0 non-modal commands (G4, G28, G92) or Group 1 motion */
    if (!gcode_execute_non_modal(ctrl, &eff, scale)) {
        gcode_execute_motion(ctrl, &eff, scale);
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

    if (ctrl->dwell_steps > 0U) {
        ctrl->dwell_steps--;
        if (ctrl->dwell_steps == 0U) {
            ctrl->dwell_ms = 0U;
        }
        if (out_pos != NULL) {
            *out_pos = ctrl->current_pos;
        }
        return true;
    }

    if (ctrl->move_in_progress) {
        if (ctrl->cfg.interpolator == NULL) {
            ctrl->move_in_progress = false;
        } else {
            bool active = syn_interpolator_step(ctrl->cfg.interpolator, &ctrl->current_pos);
            if (out_pos != NULL) {
                *out_pos = ctrl->current_pos;
            }
            if (!active) {
                ctrl->move_in_progress = false;
            }
            return active;
        }
    }

    if (out_pos != NULL) {
        *out_pos = ctrl->current_pos;
    }
    return false;
}

SYN_PT_Status syn_gcode_task_pt(SYN_PT *pt, SYN_Task *task)
{
    if (pt == NULL || task == NULL || task->user_data == NULL) {
        return PT_EXITED;
    }

    SYN_GCode_Controller *ctrl = (SYN_GCode_Controller *)task->user_data;

    PT_BEGIN(pt);

    for (;;) {
        if (ctrl->dwell_ms > 0U) {
            uint32_t wait_ms = ctrl->dwell_ms;
            ctrl->dwell_ms = 0U;
            ctrl->dwell_steps = 0U;
            PT_DELAY_MS(pt, &task->delay_until, wait_ms);
        }
        SYN_Vector3F pos;
        if (syn_gcode_step(ctrl, &pos)) {
            PT_CONTINUE(pt);
        } else {
            PT_PASS(pt);
        }
    }

    PT_END(pt); /* LCOV_EXCL_LINE: Infinite task loop end */
}

#endif /* SYN_USE_GCODE */
