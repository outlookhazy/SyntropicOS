

# File syn\_soft\_onewire.c

[**File List**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_soft\_onewire.c**](syn__soft__onewire_8c.md)

[Go to the documentation of this file](syn__soft__onewire_8c.md)


```C++
#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_ONEWIRE) || SYN_USE_ONEWIRE

#include "syn_soft_onewire.h"
#include "../util/syn_assert.h"

static void ow_delay(const SYN_SoftOneWire *ow, uint32_t us)
{
    volatile uint32_t cnt = us * ow->delay_loops;
    while (cnt--) {
        /* NOP */
    }
}

static void ow_drive_low(const SYN_SoftOneWire *ow)
{
    syn_port_gpio_write(ow->pin, 0u);
}

static void ow_release(const SYN_SoftOneWire *ow)
{
    syn_port_gpio_write(ow->pin, 1u);
}

static void ow_write_bit(const SYN_SoftOneWire *ow, uint8_t bit)
{
    if (bit != 0u) {
        /* Write-1: 6 µs low, 64 µs high */
        ow_drive_low(ow);
        ow_delay(ow, 6u);
        ow_release(ow);
        ow_delay(ow, 64u);
    } else {
        /* Write-0: 60 µs low, 10 µs recovery */
        ow_drive_low(ow);
        ow_delay(ow, 60u);
        ow_release(ow);
        ow_delay(ow, 10u);
    }
}

static uint8_t ow_read_bit(const SYN_SoftOneWire *ow)
{
    /* Initiate read: 6 µs drive low */
    ow_drive_low(ow);
    ow_delay(ow, 6u);
    ow_release(ow);
    /* Sample window: ~9 µs after release */
    ow_delay(ow, 9u);
    uint8_t bit = (uint8_t)(syn_port_gpio_read(ow->pin) != 0 ? 1u : 0u);
    /* Recovery: remainder of 70 µs slot */
    ow_delay(ow, 55u);
    return bit;
}

void syn_soft_onewire_init(SYN_SoftOneWire *ow,
                           SYN_GPIO_Pin     pin,
                           uint32_t         delay_loops)
{
    SYN_ASSERT(ow != NULL);
    ow->pin         = pin;
    ow->delay_loops = delay_loops;
    /* Configure as output, drive high (open-drain idle state) */
    syn_port_gpio_init(pin, SYN_GPIO_OUTPUT);
    syn_port_gpio_write(pin, 1u);
}

bool syn_soft_onewire_reset(const SYN_SoftOneWire *ow)
{
    SYN_ASSERT(ow != NULL);

    /* Drive low ≥ 480 µs */
    ow_drive_low(ow);
    ow_delay(ow, 480u);

    /* Release and wait 70 µs, then sample for presence pulse */
    ow_release(ow);
    ow_delay(ow, 70u);
    uint8_t present = (uint8_t)(syn_port_gpio_read(ow->pin) == 0 ? 1u : 0u);

    /* Wait out remainder of reset sequence */
    ow_delay(ow, 410u);

    return (present != 0u);
}

void syn_soft_onewire_write_byte(const SYN_SoftOneWire *ow, uint8_t byte)
{
    SYN_ASSERT(ow != NULL);
    uint8_t i;
    for (i = 0u; i < 8u; i++) {
        ow_write_bit(ow, byte & 0x01u);
        byte >>= 1u;
    }
}

uint8_t syn_soft_onewire_read_byte(const SYN_SoftOneWire *ow)
{
    SYN_ASSERT(ow != NULL);
    uint8_t byte = 0u;
    uint8_t i;
    for (i = 0u; i < 8u; i++) {
        if (ow_read_bit(ow) != 0u) {
            byte |= (uint8_t)(1u << i);
        }
    }
    return byte;
}

void syn_soft_onewire_write_rom(const SYN_SoftOneWire *ow,
                                const uint8_t          rom[8])
{
    SYN_ASSERT(ow  != NULL);
    SYN_ASSERT(rom != NULL);
    uint8_t i;
    for (i = 0u; i < 8u; i++) {
        syn_soft_onewire_write_byte(ow, rom[i]);
    }
}

void syn_soft_onewire_read_rom(const SYN_SoftOneWire *ow,
                               uint8_t                rom[8])
{
    SYN_ASSERT(ow  != NULL);
    SYN_ASSERT(rom != NULL);
    uint8_t i;
    for (i = 0u; i < 8u; i++) {
        rom[i] = syn_soft_onewire_read_byte(ow);
    }
}

#endif /* SYN_USE_ONEWIRE */
```


