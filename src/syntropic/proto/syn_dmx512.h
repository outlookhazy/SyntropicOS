/**
 * @file syn_dmx512.h
 * @brief USITT DMX512-A Stage & Architectural Lighting Protocol Stack.
 *
 * Provides non-blocking, zero-malloc DMX512 Master transmitter and Slave
 * receiver engines supporting standard 512-channel universes, start code
 * filtering, start address slot mapping, and UART framing breaking.
 */

#ifndef SYN_DMX512_H
#define SYN_DMX512_H

#include "../common/syn_defs.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief DMX512 Protocol Constants */
#define SYN_DMX512_MAX_CHANNELS           512U
#define SYN_DMX512_START_CODE_NULL        0x00U /**< Standard dimmer start code */
#define SYN_DMX512_BAUD_RATE              250000UL

/** @brief DMX512 Receiver State Machine States */
typedef enum {
    SYN_DMX512_STATE_WAIT_BREAK = 0,
    SYN_DMX512_STATE_WAIT_START,
    SYN_DMX512_STATE_READ_SLOTS,
} SYN_DMX512_RxState;

/** @brief DMX512 Master Handle */
typedef struct {
    uint8_t  channels[SYN_DMX512_MAX_CHANNELS]; /**< Universe channel buffer (1..512) */
    uint16_t num_channels;                       /**< Configured universe length       */
    uint8_t  start_code;                         /**< Active start code (default 0x00) */
} SYN_DMX512_Master;

/** @brief DMX512 Slave Handle */
typedef struct {
    uint8_t            rx_channels[SYN_DMX512_MAX_CHANNELS]; /**< Local footprint channels */
    uint16_t           start_address;                        /**< Start address (1..512)   */
    uint16_t           footprint;                            /**< Number of channels used  */
    SYN_DMX512_RxState rx_state;                             /**< Receiver state           */
    uint16_t           current_slot;                         /**< Current slot index       */
    uint8_t            start_code;                           /**< Received start code      */
    bool               frame_complete;                       /**< Frame updated flag       */
} SYN_DMX512_Slave;

/**
 * @brief Initialize DMX512 Master engine.
 * @param master        Master handle.
 * @param num_channels  Active universe length (1..512, default 512).
 */
void syn_dmx512_master_init(SYN_DMX512_Master *master, uint16_t num_channels);

/**
 * @brief Set channel value (1-indexed channel, 1..512).
 * @param master   Master handle.
 * @param channel  Channel index (1..512).
 * @param value    8-bit slot value (0..255).
 * @return SYN_OK on success, error code on invalid channel.
 */
SYN_Status syn_dmx512_master_set_channel(SYN_DMX512_Master *master, uint16_t channel, uint8_t value);

/**
 * @brief Get channel value (1-indexed channel, 1..512).
 * @param master   Master handle.
 * @param channel  Channel index (1..512).
 * @return 8-bit slot value.
 */
uint8_t syn_dmx512_master_get_channel(const SYN_DMX512_Master *master, uint16_t channel);

/**
 * @brief Build DMX packet payload (Start Code + Channel Slots).
 * @param master   Master handle.
 * @param out_buf  Output buffer for frame payload.
 * @param max_len  Maximum capacity of out_buf.
 * @return Number of payload bytes written (1 + num_channels).
 */
size_t syn_dmx512_master_build_frame(const SYN_DMX512_Master *master, uint8_t *out_buf, size_t max_len);

/**
 * @brief Initialize DMX512 Slave engine.
 * @param slave          Slave handle.
 * @param start_address  Start channel address (1..512).
 * @param footprint      Channel footprint length (e.g. 4 for 4-channel fixture).
 */
void syn_dmx512_slave_init(SYN_DMX512_Slave *slave, uint16_t start_address, uint16_t footprint);

/**
 * @brief Signal line Break event to Slave engine (e.g. from UART framing error interrupt).
 * @param slave  Slave handle.
 */
void syn_dmx512_slave_rx_break(SYN_DMX512_Slave *slave);

/**
 * @brief Ingest incoming byte stream into Slave engine.
 * @param slave  Slave handle.
 * @param byte   Incoming byte from UART rx.
 */
void syn_dmx512_slave_rx_byte(SYN_DMX512_Slave *slave, uint8_t byte);

/**
 * @brief Read channel value from local slave footprint (0-indexed offset).
 * @param slave   Slave handle.
 * @param offset  Footprint channel offset (0 .. footprint - 1).
 * @return Channel value byte.
 */
uint8_t syn_dmx512_slave_get_channel(const SYN_DMX512_Slave *slave, uint16_t offset);

/**
 * @brief Check if a complete DMX packet was received since last query.
 * @param slave  Slave handle.
 * @return true if updated, false otherwise.
 */
bool syn_dmx512_slave_is_updated(SYN_DMX512_Slave *slave);

#ifdef __cplusplus
}
#endif

#endif /* SYN_DMX512_H */
