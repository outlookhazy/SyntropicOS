# Input / Output Modules

SyntropicOS provides non-blocking, debounced drivers for physical user inputs (Buttons, Rotary Encoders) and output indicators (LEDs, Software PWM).

---

## 1. Button Driver (`syn_button.h`)

The `syn_button` driver provides debounced GPIO button management with built-in support for:
- Debouncing filter with configurable `debounce_ms` window.
- Active-High and Active-Low pin polarities.
- Multi-click tap detection (Single, Double, Triple, N-click).
- Long-press hold thresholds and auto-repeat streaming.
- Multi-button chorded combinations (`SYN_ButtonCombo`).

### Basic Initialization & Single Click

```c
#include <syntropic/input/syn_button.h>

static SYN_Button btn1;

void on_single_click(SYN_Button *btn, void *ctx) {
    // Fired after multi-click window (250ms) expires
}

void app_setup(void) {
    // Port A Pin 2, Active-Low (pull-up), 50ms debounce
    syn_button_init(&btn1, SYN_GPIO_PIN(0, 2), SYN_BUTTON_ACTIVE_LOW, 50);
    syn_button_on_single_click(&btn1, on_single_click, NULL);
}

void app_loop(void) {
    syn_button_update(&btn1);
}
```

### Multi-Click & Long-Press Configuration

```c
// Configure 200ms multi-click evaluation window
syn_button_set_click_window(&btn1, 200);

// Register tap gesture handlers
syn_button_on_single_click(&btn1, on_single, NULL);
syn_button_on_double_click(&btn1, on_double, NULL);
syn_button_on_triple_click(&btn1, on_triple, NULL);
syn_button_on_multi_click(&btn1, on_multi, NULL); // 4+ taps

// Register long press (fires ONCE at 1000ms hold threshold)
syn_button_on_long_press(&btn1, on_long_press, 1000, NULL);

// Register auto-repeat (fires every 100ms while held)
syn_button_on_repeat(&btn1, on_repeat, 100, NULL);
```

> [!NOTE]
> Entering a **Long Press** (`on_long_press`) or firing **Auto-Repeat** (`on_repeat`) automatically resets the click counter (`click_count = 0`), preventing single-click callbacks from triggering when the button is released.

### Multi-Button Combinations (`SYN_ButtonCombo`)

`SYN_ButtonCombo` enables chorded multi-button presses (e.g. pressing `BTN1` + `BTN2` simultaneously).

```c
static SYN_Button btn1;
static SYN_Button btn2;
static SYN_ButtonCombo combo;
static const SYN_Button *combo_list[2];

void on_combo_fired(SYN_Button *btn, void *ctx) {
    // Fired when BTN1 and BTN2 are both pressed
}

void app_setup(void) {
    syn_button_init(&btn1, SYN_GPIO_PIN(0, 2), SYN_BUTTON_ACTIVE_LOW, 50);
    syn_button_init(&btn2, SYN_GPIO_PIN(0, 3), SYN_BUTTON_ACTIVE_LOW, 50);

    combo_list[0] = &btn1;
    combo_list[1] = &btn2;
    syn_button_combo_init(&combo, combo_list, 2, on_combo_fired, NULL);
}

void app_loop(void) {
    syn_button_update(&btn1);
    syn_button_update(&btn2);
    syn_button_combo_update(&combo);
}
```

---

## 2. Rotary Encoder Driver (`syn_encoder.h`)

Quadrature rotary encoder decoding with velocity tracking and direction calculation.

| Function | Description |
|---|---|
| `syn_encoder_init()` | Initialize phase A & B GPIO pins |
| `syn_encoder_update()` | Sample phase pins and update step counter |
| `syn_encoder_get_delta()` | Read and reset relative step delta since last call |
| `syn_encoder_position()` | Read accumulated absolute position |
| `syn_encoder_set_stats()` | Attach `SYN_Signal` window for velocity tracking |

---

## 3. Non-Blocking LED Driver (`syn_led.h`)

Provides pattern blinking, flash sequences, and Morse-code style pattern strings.

```c
#include <syntropic/output/syn_led.h>

static SYN_LED status_led;

void setup(void) {
    syn_led_init(&status_led, LED_BUILTIN, SYN_LED_ACTIVE_HIGH);
    
    // Blink 500ms ON / 500ms OFF
    syn_led_blink(&status_led, 500, 500);
}

void loop(void) {
    syn_led_update(&status_led);
}
```

---

## 4. Software PWM Driver (`syn_soft_pwm.h`)

Enables software PWM output on any arbitrary GPIO pin without requiring hardware timers.

| Function | Description |
|---|---|
| `syn_soft_pwm_init()` | Initialize GPIO pin and period step resolution |
| `syn_soft_pwm_set_duty()` | Set raw duty cycle (`0` to `resolution`) |
| `syn_soft_pwm_set_percent()` | Set duty cycle percentage (`0` to `100`) |
| `syn_soft_pwm_tick()` | Advance PWM phase by one timer tick |

---

## 5. Matrix Keypad & Touch (`syn_keypad.h` & `syn_touch.h`)

Scanning matrix keypads ($3\times4$, $4\times4$) and capacitive touch sensor inputs with debouncing.

```c
#include <syntropic/input/syn_keypad.h>
#include <syntropic/input/syn_touch.h>

static SYN_Keypad kp;
static const SYN_GPIO_Pin row_pins[4] = {0, 1, 2, 3};
static const SYN_GPIO_Pin col_pins[4] = {4, 5, 6, 7};

void keypad_setup(void) {
    syn_keypad_init(&kp, row_pins, 4, col_pins, 4, "123A456B789C*0#D");
}

void keypad_demo(void) {
    syn_keypad_scan(&kp);
    char key = '\0';
    if (syn_keypad_get_key(&kp, &key)) {
        printf("Key Pressed: %c\n", key);
    }
}
```

---

## 6. DIP Switch Array & Analog Joystick (`syn_dipswitch.h` & `syn_joystick.h`)

Provides parallel reading of DIP switch position banks and 2-axis ADC joystick coordinate normalization with deadzone filtering.

---

## 7. Piezo Buzzer & WS2812 Smart LED (`syn_buzzer.h` & `syn_smartled.h`)

Non-blocking piezo buzzer melody/beep generator and WS2812 (NeoPixel) RGB LED bit-bang driver.

```c
#include <syntropic/output/syn_buzzer.h>
#include <syntropic/output/syn_smartled.h>

static SYN_Buzzer buz;
static SYN_SmartLED strip;
static SYN_SmartLEDColor pixels[8];

void audio_visual_demo(void) {
    syn_buzzer_init(&buz, 8, true);
    syn_smartled_init(&strip, 9, 8, pixels, SYN_SMARTLED_ORDER_GRB);

    // Play 1 kHz beep for 100 ms
    syn_buzzer_beep(&buz, 1000, 100);

    // Set WS2812 LED 0 to Red
    syn_smartled_set_pixel_rgb(&strip, 0, 255, 0, 0);
}
```

