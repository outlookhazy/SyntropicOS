#if __has_include("syn_config.h")
  #include "syn_config.h"
#endif

#if !defined(SYN_USE_CANOPEN) || SYN_USE_CANOPEN

/**
 * @file syn_cia303.c
 * @brief CANopen CiA 303-3 Indicator & Status LED implementation.
 */

#include "syn_cia303.h"
#include "../util/syn_assert.h"

#include <string.h>

/* NMT State constants matching CiA 301 / syn_canopen_mgr */
#ifndef SYN_CANOPEN_NMT_STOPPED
#define SYN_CANOPEN_NMT_STOPPED          0x04U
#define SYN_CANOPEN_NMT_OPERATIONAL      0x05U
#define SYN_CANOPEN_NMT_PRE_OPERATIONAL  0x7FU
#endif

void syn_cia303_init(SYN_CiA303_Indicator *ind, SYN_LED *run_led, SYN_LED *err_led)
{
    SYN_ASSERT(ind != NULL);

    memset(ind, 0, sizeof(*ind));
    ind->run_led   = run_led;
    ind->err_led   = err_led;
    ind->run_state = SYN_CIA303_RUN_OFF;
    ind->err_state = SYN_CIA303_ERR_OFF;

    if (run_led != NULL) syn_led_off(run_led);
    if (err_led != NULL) syn_led_off(err_led);
}

void syn_cia303_set_nmt_state(SYN_CiA303_Indicator *ind, uint8_t nmt_state)
{
    SYN_ASSERT(ind != NULL);

    if (ind->run_led == NULL) return;

    SYN_CiA303_RunState new_state;
    switch (nmt_state) {
    case SYN_CANOPEN_NMT_OPERATIONAL:
        new_state = SYN_CIA303_RUN_SOLID_ON;
        syn_led_on(ind->run_led);
        break;

    case SYN_CANOPEN_NMT_PRE_OPERATIONAL:
        new_state = SYN_CIA303_RUN_BLINKING;
        syn_led_blink(ind->run_led, 200, 200); /* 200ms ON, 200ms OFF */
        break;

    case SYN_CANOPEN_NMT_STOPPED:
        new_state = SYN_CIA303_RUN_SINGLE_FLASH;
        syn_led_blink(ind->run_led, 200, 1000); /* 200ms ON, 1000ms OFF */
        break;

    default:
        new_state = SYN_CIA303_RUN_OFF;
        syn_led_off(ind->run_led);
        break;
    }

    ind->run_state = new_state;
}

void syn_cia303_set_error_state(SYN_CiA303_Indicator *ind, SYN_CiA303_ErrState err_state)
{
    SYN_ASSERT(ind != NULL);

    if (ind->err_led == NULL) return;

    ind->err_state = err_state;

    switch (err_state) {
    case SYN_CIA303_ERR_SOLID_ON:
        syn_led_on(ind->err_led);
        break;

    case SYN_CIA303_ERR_SINGLE_FLASH:
        syn_led_blink(ind->err_led, 200, 1000); /* 200ms ON, 1000ms OFF */
        break;

    case SYN_CIA303_ERR_DOUBLE_FLASH:
        /* 2x 200ms pulses (handled in step or repeated blink) */
        syn_led_blink(ind->err_led, 200, 400);
        break;

    case SYN_CIA303_ERR_TRIPLE_FLASH:
        syn_led_blink(ind->err_led, 200, 300);
        break;

    case SYN_CIA303_ERR_OFF:
    default:
        syn_led_off(ind->err_led);
        break;
    }
}

void syn_cia303_step(SYN_CiA303_Indicator *ind)
{
    SYN_ASSERT(ind != NULL);

    if (ind->run_led != NULL) {
        syn_led_update(ind->run_led);
    }
    if (ind->err_led != NULL) {
        syn_led_update(ind->err_led);
    }
}

#endif /* SYN_USE_CANOPEN */
