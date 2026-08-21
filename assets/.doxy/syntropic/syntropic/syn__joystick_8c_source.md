

# File syn\_joystick.c

[**File List**](files.md) **>** [**input**](dir_8ff21aebe696416b7e73f727e71a981c.md) **>** [**syn\_joystick.c**](syn__joystick_8c.md)

[Go to the documentation of this file](syn__joystick_8c.md)


```C++

#include "syn_joystick.h"

#include "../util/syn_assert.h"

#include <math.h>
#include <string.h>

SYN_Status syn_joystick_init(SYN_Joystick *joy, uint16_t center_x, uint16_t center_y,
                             uint16_t adc_max, uint16_t deadband)
{
    SYN_ASSERT(joy != NULL);
    SYN_ASSERT(adc_max > 0);

    memset(joy, 0, sizeof(*joy));
    joy->center_x = center_x;
    joy->center_y = center_y;
    joy->adc_max = adc_max;
    joy->deadband = deadband;
    joy->dir = SYN_JOYSTICK_DIR_CENTER;

    return SYN_OK;
}

void syn_joystick_feed_adc(SYN_Joystick *joy, uint16_t raw_x, uint16_t raw_y, bool btn)
{
    if (joy == NULL)
        return;

    joy->button_pressed = btn;

    int32_t dx = (int32_t)raw_x - (int32_t)joy->center_x;
    int32_t dy = (int32_t)raw_y - (int32_t)joy->center_y;

    /* Apply deadband filtering */
    if (dx > -((int32_t)joy->deadband) && dx < (int32_t)joy->deadband)
        dx = 0;
    if (dy > -((int32_t)joy->deadband) && dy < (int32_t)joy->deadband)
        dy = 0;

    /* Calculate percentage (-100 to +100) */
    int32_t range_x = (dx >= 0) ? (int32_t)(joy->adc_max - joy->center_x) : (int32_t)joy->center_x;
    int32_t range_y = (dy >= 0) ? (int32_t)(joy->adc_max - joy->center_y) : (int32_t)joy->center_y;

    if (range_x > 0) {
        joy->current_x_pct = (int16_t)((dx * 100) / range_x);
    } else {
        /* LCOV_EXCL_START: Zero range fallback for joystick X percentage */
        joy->current_x_pct = 0;
        /* LCOV_EXCL_STOP */
    }

    if (range_y > 0) {
        joy->current_y_pct = (int16_t)((dy * 100) / range_y);
    } else {
        joy->current_y_pct = 0;
    }

    /* Clamp percentages */
    if (joy->current_x_pct > 100)
        joy->current_x_pct = 100;
    /* LCOV_EXCL_START: Unsigned raw_x/y guarantees current_x/y_pct >= -100 */
    if (joy->current_x_pct < -100)
        joy->current_x_pct = -100;
    if (joy->current_y_pct > 100)
        joy->current_y_pct = 100;
    if (joy->current_y_pct < -100)
        joy->current_y_pct = -100;
    /* LCOV_EXCL_STOP */

    /* Decode 8-way directional state */
    int16_t x = joy->current_x_pct;
    int16_t y = joy->current_y_pct;

    if (x == 0 && y == 0) {
        joy->dir = SYN_JOYSTICK_DIR_CENTER;
    } else if (y > 30 && x > 30) {
        joy->dir = SYN_JOYSTICK_DIR_UP_RIGHT;
    } else if (y > 30 && x < -30) {
        joy->dir = SYN_JOYSTICK_DIR_UP_LEFT;
    } else if (y < -30 && x > 30) {
        joy->dir = SYN_JOYSTICK_DIR_DOWN_RIGHT;
    } else if (y < -30 && x < -30) {
        joy->dir = SYN_JOYSTICK_DIR_DOWN_LEFT;
    } else if (y > 50) {
        joy->dir = SYN_JOYSTICK_DIR_UP;
    } else if (y < -50) {
        joy->dir = SYN_JOYSTICK_DIR_DOWN;
    } else if (x > 50) {
        joy->dir = SYN_JOYSTICK_DIR_RIGHT;
    } else if (x < -50) {
        joy->dir = SYN_JOYSTICK_DIR_LEFT;
    } else {
        joy->dir = SYN_JOYSTICK_DIR_CENTER;
    }
}

int16_t syn_joystick_get_x_pct(const SYN_Joystick *joy)
{
    if (joy == NULL)
        return 0;
    return joy->current_x_pct;
}

int16_t syn_joystick_get_y_pct(const SYN_Joystick *joy)
{
    if (joy == NULL)
        return 0;
    return joy->current_y_pct;
}

SYN_JoystickDir syn_joystick_get_dir(const SYN_Joystick *joy)
{
    if (joy == NULL)
        return SYN_JOYSTICK_DIR_CENTER;
    return joy->dir;
}
```


