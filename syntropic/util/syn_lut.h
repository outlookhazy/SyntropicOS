/**
 * @file syn_lut.h
 * @brief Lookup table with linear interpolation.
 *
 * For calibration curves, thermistor tables, transfer functions, etc.
 * The table is a const array of (x, y) pairs, sorted by ascending x.
 * Lookups interpolate linearly between the two nearest entries.
 *
 * Header-only — integer math, no floating point.
 *
 * @par Usage
 * @code
 *   static const SYN_LUT_Entry thermistor_table[] = {
 *       {   0, 3300 },  // 0°C → 3300 ADC counts
 *       {  25, 2048 },
 *       {  50, 1200 },
 *       { 100,  400 },
 *   };
 *
 *   int32_t temp = syn_lut_lookup(thermistor_table, 4, adc_value);
 *   // Returns interpolated x for a given y
 *
 *   int32_t adc = syn_lut_forward(thermistor_table, 4, 37);
 *   // Returns interpolated y for a given x (37°C)
 * @endcode
 * @ingroup syn_core
 */

#ifndef SYN_LUT_H
#define SYN_LUT_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Table entry ────────────────────────────────────────────────────────── */

/** @brief Lookup table entry (x, y pair). */
typedef struct {
    int32_t x;  /**< Input (independent) value.  */
    int32_t y;  /**< Output (dependent) value.   */
} SYN_LUT_Entry;

/* ── Forward lookup: x → y (interpolated) ───────────────────────────────── */

/**
 * @brief Look up y for a given x, with linear interpolation.
 *
 * Table must be sorted by ascending x. Values outside the table range
 * are clamped to the nearest endpoint.
 *
 * @param table  Array of (x, y) entries.
 * @param count  Number of entries.
 * @param x      Input value.
 * @return Interpolated y.
 */
static inline int32_t syn_lut_forward(const SYN_LUT_Entry *table,
                                       size_t count, int32_t x)
{
    if (count == 0) return 0;
    if (count == 1 || x <= table[0].x) return table[0].y;
    if (x >= table[count - 1].x)       return table[count - 1].y;

    /* Find the bracketing interval */
    size_t i;
    for (i = 1; i < count; i++) {
        if (x <= table[i].x) break;
    }

    /* Linear interpolation */
    int32_t x0 = table[i - 1].x, y0 = table[i - 1].y;
    int32_t x1 = table[i].x,     y1 = table[i].y;
    int32_t dx = x1 - x0;

    if (dx == 0) return y0;

    return y0 + ((y1 - y0) * (x - x0)) / dx;
}

/* ── Reverse lookup: y → x (interpolated) ───────────────────────────────── */

/**
 * @brief Look up x for a given y, with linear interpolation.
 *
 * The y column can be monotonically increasing OR decreasing.
 * Values outside the table range are clamped.
 *
 * @param table  Array of (x, y) entries, sorted by ascending x.
 * @param count  Number of entries.
 * @param y      Input value (in the y domain).
 * @return Interpolated x.
 */
static inline int32_t syn_lut_reverse(const SYN_LUT_Entry *table,
                                       size_t count, int32_t y)
{
    if (count == 0) return 0;
    if (count == 1) return table[0].x;

    /* Detect direction (is y ascending or descending?) */
    int ascending = (table[count - 1].y >= table[0].y) ? 1 : 0;

    /* Find the bracketing interval */
    size_t i;
    for (i = 1; i < count; i++) {
        if (ascending) {
            if (y <= table[i].y) break;
        } else {
            if (y >= table[i].y) break;
        }
    }

    if (i >= count) i = count - 1;

    int32_t x0 = table[i - 1].x, y0 = table[i - 1].y;
    int32_t x1 = table[i].x,     y1 = table[i].y;
    int32_t dy = y1 - y0;

    if (dy == 0) return x0;

    return x0 + ((x1 - x0) * (y - y0)) / dy;
}

#ifdef __cplusplus
}
#endif

#endif /* SYN_LUT_H */
