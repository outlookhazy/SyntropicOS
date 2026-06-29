# Input / Output

## Input

| Module | Header | Config | Description |
|---|---|---|---|
| Button | `input/syn_button.h` | `SYN_USE_BUTTON` | Debounced button with press, release, and long-press callbacks. Uses `syn_fsm` internally. |
| Encoder | `input/syn_encoder.h` | `SYN_USE_ENCODER` | Rotary encoder with quadrature decoding and velocity tracking |

## Output

| Module | Header | Config | Description |
|---|---|---|---|
| LED | `output/syn_led.h` | `SYN_USE_LED` | Non-blocking LED controller: blink, flash N times, pattern strings (e.g., `".-."` for Morse-like sequences) |
| Soft PWM | `output/syn_soft_pwm.h` | `SYN_USE_SOFT_PWM` | Software PWM on any GPIO pin — no hardware timer required |
