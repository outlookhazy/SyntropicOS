

# File syn\_keypad.c

[**File List**](files.md) **>** [**input**](dir_8ff21aebe696416b7e73f727e71a981c.md) **>** [**syn\_keypad.c**](syn__keypad_8c.md)

[Go to the documentation of this file](syn__keypad_8c.md)


```C++

#include "syn_keypad.h"

#include "../util/syn_assert.h"

#include <string.h>

SYN_Status syn_keypad_init(SYN_Keypad *kp, const SYN_GPIO_Pin *rows, uint8_t num_rows,
                           const SYN_GPIO_Pin *cols, uint8_t num_cols, const char *keymap)
{
    SYN_ASSERT(kp != NULL);
    SYN_ASSERT(rows != NULL);
    SYN_ASSERT(cols != NULL);
    SYN_ASSERT(num_rows > 0 && num_rows <= SYN_KEYPAD_MAX_ROWS);
    SYN_ASSERT(num_cols > 0 && num_cols <= SYN_KEYPAD_MAX_COLS);
    SYN_ASSERT(keymap != NULL);

    memset(kp, 0, sizeof(*kp));
    kp->num_rows = num_rows;
    kp->num_cols = num_cols;

    memcpy(kp->rows, rows, sizeof(SYN_GPIO_Pin) * num_rows);
    memcpy(kp->cols, cols, sizeof(SYN_GPIO_Pin) * num_cols);
    memcpy(kp->keymap, keymap, (size_t)num_rows * num_cols);

    /* Rows as Outputs (Idle LOW) */
    for (uint8_t r = 0; r < num_rows; r++) {
        syn_port_gpio_init(kp->rows[r], SYN_GPIO_OUTPUT);
        syn_port_gpio_write(kp->rows[r], SYN_GPIO_LOW);
    }

    /* Cols as Inputs with PULL-DOWN */
    for (uint8_t c = 0; c < num_cols; c++) {
        syn_port_gpio_init(kp->cols[c], SYN_GPIO_INPUT_PULLDOWN);
    }

    return SYN_OK;
}

void syn_keypad_scan(SYN_Keypad *kp)
{
    if (kp == NULL)
        return;

    char found_key = 0;

    for (uint8_t r = 0; r < kp->num_rows; r++) {
        /* Drive row HIGH */
        syn_port_gpio_write(kp->rows[r], SYN_GPIO_HIGH);

        for (uint8_t c = 0; c < kp->num_cols; c++) {
            if (syn_port_gpio_read(kp->cols[c]) == SYN_GPIO_HIGH) {
                found_key = kp->keymap[r * kp->num_cols + c];
                break;
            }
        }

        /* Set row back to LOW */
        syn_port_gpio_write(kp->rows[r], SYN_GPIO_LOW);

        if (found_key != 0)
            break;
    }

    /* Process state changes */
    if (found_key != 0) {
        if (!kp->is_pressed || kp->active_key != found_key) {
            kp->active_key = found_key;
            kp->is_pressed = true;
            kp->press_count++;

            if (kp->on_event) {
                kp->on_event(kp, found_key, true, kp->user_ctx);
            }
        }
    } else {
        if (kp->is_pressed) {
            char released_key = kp->active_key;
            kp->is_pressed = false;
            kp->active_key = 0;

            if (kp->on_event) {
                kp->on_event(kp, released_key, false, kp->user_ctx);
            }
        }
    }
}

bool syn_keypad_get_key(const SYN_Keypad *kp, char *out_key)
{
    if (kp == NULL || !kp->is_pressed)
        return false;
    if (out_key != NULL) {
        *out_key = kp->active_key;
    }
    return true;
}

void syn_keypad_set_callback(SYN_Keypad *kp, SYN_KeypadCallback callback, void *user_ctx)
{
    if (kp == NULL)
        return;
    kp->on_event = callback;
    kp->user_ctx = user_ctx;
}
```


