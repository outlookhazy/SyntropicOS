# STM32 Infrared (IR) Transceiver Example (`syn_ir`)

This example demonstrates how to **receive (decode)** and **transmit (encode)** signals from standard **Consumer IR Remote Controls** (NEC, Sony SIRCS, Samsung, Philips RC5, RC6, Panasonic, Denon) using an **STM32 microcontroller** and SyntropicOS **`syn_ir`**.

---

## Hardware Configuration & Wiring

### 1. IR Receiver Wiring (TSOP38238 / VS1838B)
Connect a 38kHz IR receiver module to the STM32:

| IR Receiver Module Pin | STM32 Pin | Description |
|---|---|---|
| **OUT (Data)** | **PA0** | GPIO input with Dual-Edge Interrupt (EXTI0) |
| **VCC** | **3.3V / 5V** | Power supply (3.3V for TSOP38238) |
| **GND** | **GND** | Ground reference |

### 2. IR Transmitter Wiring (940nm IR LED)
Connect a 940nm IR Transmitting LED via an NPN Transistor (2N2222 / BC547) driver:

| STM32 Pin | Transistor Base | Description |
|---|---|---|
| **PB8** | **Base (via 1kΩ resistor)** | PWM Output / GPIO Bit-Bang Transmit Pin |
| **GND** | **Emitter** | Ground |
| **5V / 3.3V** | **Anode (+) of IR LED** | Cathode (-) to Collector via 33Ω resistor |

---

## IR Transmit Modulation Methods

When transmitting IR signals, the microcontroller must modulate the signal with a **carrier frequency** (36kHz, 38kHz, 40kHz, or 56kHz).

### Method 1: Software Bit-Banging (Universal & Transceiver-Agnostic)
Generates the 38kHz carrier (13µs HIGH, 13µs LOW) using DWT microsecond delays:

```c
void send_ir_bitbang(SYN_IR_Protocol proto, uint32_t addr, uint32_t cmd) {
    SYN_IR_Frame tx_frame = { .protocol = proto, .address = addr, .command = cmd };
    SYN_IR_Pulse pulses[128];
    size_t pulse_count = 0;

    uint8_t high_us, low_us;
    uint8_t carrier_khz = ir_carrier_khz(proto);
    get_carrier_delays(carrier_khz, &high_us, &low_us);

    if (syn_ir_encode_frame(&tx_frame, pulses, 128, &pulse_count) == SYN_OK) {
        for (size_t i = 0; i < pulse_count; i++) {
            if (pulses[i].is_mark) {
                // Generate carrier frequency (e.g. 13us HIGH, 13us LOW for 38kHz)
                for (uint32_t elapsed = 0; elapsed < pulses[i].duration_us; elapsed += (high_us + low_us)) {
                    HAL_GPIO_WritePin(IR_TX_PORT, IR_TX_PIN, GPIO_PIN_SET);
                    delay_us(high_us);
                    HAL_GPIO_WritePin(IR_TX_PORT, IR_TX_PIN, GPIO_PIN_RESET);
                    delay_us(low_us);
                }
            } else {
                HAL_GPIO_WritePin(IR_TX_PORT, IR_TX_PIN, GPIO_PIN_RESET);
                delay_us(pulses[i].duration_us);
            }
        }
        HAL_GPIO_WritePin(IR_TX_PORT, IR_TX_PIN, GPIO_PIN_RESET);
    }
}
```

### Method 2: Hardware PWM Timer Carrier (Recommended for Low CPU Load)
Controls an STM32 Timer PWM output (e.g., TIM3 Channel 3 running at 38kHz, 50% duty cycle):

```c
void send_ir_pwm(SYN_IR_Protocol proto, uint32_t addr, uint32_t cmd) {
    SYN_IR_Frame tx_frame = { .protocol = proto, .address = addr, .command = cmd };
    SYN_IR_Pulse pulses[128];
    size_t pulse_count = 0;

    if (syn_ir_encode_frame(&tx_frame, pulses, 128, &pulse_count) == SYN_OK) {
        for (size_t i = 0; i < pulse_count; i++) {
            if (pulses[i].is_mark) {
                HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3); // Enable 38kHz carrier
                delay_us(pulses[i].duration_us);
            } else {
                HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_3);  // Disable carrier
                delay_us(pulses[i].duration_us);
            }
        }
        HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_3);
    }
}
```

### Carrier Frequency Lookup Table
```c
uint8_t ir_carrier_khz(SYN_IR_Protocol proto) {
    switch (proto) {
        case SYN_IR_PROTO_RC5:
        case SYN_IR_PROTO_RC6:      return 36;
        case SYN_IR_PROTO_KASEIKYO: return 37;
        case SYN_IR_PROTO_SONY_12:
        case SYN_IR_PROTO_SONY_15:
        case SYN_IR_PROTO_SONY_20:  return 40;
        default:                    return 38;
    }
}

void get_carrier_delays(uint8_t carrier_khz, uint8_t *high_us, uint8_t *low_us) {
    switch (carrier_khz) {
        case 36: *high_us = 14; *low_us = 14; break; // 35.7 kHz (RC5 / RC6)
        case 37: *high_us = 13; *low_us = 14; break; // 37.0 kHz (Kaseikyo / Panasonic)
        case 38: *high_us = 13; *low_us = 13; break; // 38.4 kHz (NEC / Samsung / Denon)
        case 40: *high_us = 12; *low_us = 13; break; // 40.0 kHz (Sony SIRCS)
        case 56: *high_us = 9;  *low_us = 9;  break; // 55.5 kHz
        default: *high_us = 13; *low_us = 13; break;
    }
}
```

---

## Output Preview

```text
[IR Transceiver] Ready. Receiving on PA0, Transmitting on PB8.
[IR TX] Transmitting NEC (Addr=0x00FF, Cmd=0x0045, Carrier=38 kHz)...
[IR RX] Decoded Frame: Protocol=0 (NEC), Addr=0x00FF, Cmd=0x0045, Repeat=NO
```
