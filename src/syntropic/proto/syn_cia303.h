/**
 * @file syn_cia303.h
 * @brief CANopen CiA 303-3 Indicator & Status LED Specification implementation.
 *
 * Provides standard CiA 303-3 Green "RUN" and Red "ERROR" LED indicator
 * patterns for CANopen node status visualization, fully integrated with
 * SyntropicOS's syn_led controller and syn_canopen_mgr NMT engine.
 */

#ifndef SYN_CIA303_H
#define SYN_CIA303_H

#include "../common/syn_defs.h"
#include "../output/syn_led.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief CiA 303-3 Green RUN LED indicator states */
typedef enum {
    SYN_CIA303_RUN_OFF          = 0, /**< Power off or reset state                     */
    SYN_CIA303_RUN_SINGLE_FLASH = 1, /**< Stopped state (200ms ON, 1000ms OFF)         */
    SYN_CIA303_RUN_BLINKING     = 2, /**< Pre-operational state (200ms ON, 200ms OFF)   */
    SYN_CIA303_RUN_SOLID_ON     = 3, /**< Operational state                             */
} SYN_CiA303_RunState;

/** @brief CiA 303-3 Red ERROR LED indicator states */
typedef enum {
    SYN_CIA303_ERR_OFF          = 0, /**< No error                                      */
    SYN_CIA303_ERR_SINGLE_FLASH = 1, /**< Warning limit reached (200ms ON, 1000ms OFF)  */
    SYN_CIA303_ERR_DOUBLE_FLASH = 2, /**< Heartbeat / Guard event (2x 200ms, 1000ms OFF) */
    SYN_CIA303_ERR_TRIPLE_FLASH = 3, /**< Sync error (3x 200ms, 1000ms OFF)              */
    SYN_CIA303_ERR_SOLID_ON     = 4, /**< Bus-off / Fatal error                          */
} SYN_CiA303_ErrState;

/** @brief CiA 303-3 Status Indicator Handle */
typedef struct {
    SYN_LED             *run_led;   /**< Green RUN LED instance     */
    SYN_LED             *err_led;   /**< Red ERROR LED instance     */
    SYN_CiA303_RunState  run_state; /**< Active RUN state           */
    SYN_CiA303_ErrState  err_state; /**< Active ERR state           */
} SYN_CiA303_Indicator;

/**
 * @brief Initialize CiA 303-3 Indicator controller.
 * @param ind      Indicator instance.
 * @param run_led  Green RUN LED instance (initialized).
 * @param err_led  Red ERROR LED instance (initialized).
 */
void syn_cia303_init(SYN_CiA303_Indicator *ind, SYN_LED *run_led, SYN_LED *err_led);

/**
 * @brief Update RUN LED state according to CANopen NMT state.
 * @param ind        Indicator instance.
 * @param nmt_state  CANopen NMT state byte (e.g. Pre-Op, Op, Stopped).
 */
void syn_cia303_set_nmt_state(SYN_CiA303_Indicator *ind, uint8_t nmt_state);

/**
 * @brief Update Red ERROR LED state.
 * @param ind        Indicator instance.
 * @param err_state  CiA 303-3 Error state enum.
 */
void syn_cia303_set_error_state(SYN_CiA303_Indicator *ind, SYN_CiA303_ErrState err_state);

/**
 * @brief Step indicator LEDs (call periodically or from main loop).
 * @param ind    Indicator instance.
 */
void syn_cia303_step(SYN_CiA303_Indicator *ind);

#ifdef __cplusplus
}
#endif

#endif /* SYN_CIA303_H */
