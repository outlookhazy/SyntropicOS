/**
 * @file syn_usb_midi.h
 * @brief Zero-Heap USB 2.0 MIDI Class Device Driver.
 * @ingroup syn_drivers
 *
 * Implements USB Class Definition for MIDI Devices v1.0 (Audio Streaming subclass).
 * Provides 4-byte USB MIDI Event Packet encoding and decoding for Note On/Off,
 * Control Change, Pitch Bend, and Program Change messages.
 */

#ifndef SYN_USB_MIDI_H
#define SYN_USB_MIDI_H

#include "syntropic/common/syn_defs.h"
#include "syntropic/drivers/syn_usb.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SYN_USB_MIDI_MAX_PACKET_SIZE 64U /**< Maximum USB MIDI packet size in bytes */
#define SYN_USB_MIDI_DESC_SIZE 73U       /**< Audio Control + MIDI Streaming descriptor size */

/** USB MIDI Code Index Number (CIN) definitions per USB MIDI Spec Table 4-1 */
#define SYN_USB_MIDI_CIN_MISC 0x0U             /**< Miscellaneous function codes */
#define SYN_USB_MIDI_CIN_CABLE_EVENT 0x1U      /**< Cable events */
#define SYN_USB_MIDI_CIN_SYSTEM_COMMON_2 0x2U  /**< 2-byte System Common messages */
#define SYN_USB_MIDI_CIN_SYSTEM_COMMON_3 0x3U  /**< 3-byte System Common messages */
#define SYN_USB_MIDI_CIN_SYSEX_START 0x4U      /**< SysEx starts or continues */
#define SYN_USB_MIDI_CIN_SYSEX_ENDS_1 0x5U     /**< Single-byte SysEx ends */
#define SYN_USB_MIDI_CIN_SYSEX_ENDS_2 0x6U     /**< 2-byte SysEx ends */
#define SYN_USB_MIDI_CIN_SYSEX_ENDS_3 0x7U     /**< 3-byte SysEx ends */
#define SYN_USB_MIDI_CIN_NOTE_OFF 0x8U         /**< Note Off message (0x80) */
#define SYN_USB_MIDI_CIN_NOTE_ON 0x9U          /**< Note On message (0x90) */
#define SYN_USB_MIDI_CIN_POLY_KEYPRESS 0xAU    /**< Poly KeyPress message (0xA0) */
#define SYN_USB_MIDI_CIN_CONTROL_CHANGE 0xBU   /**< Control Change message (0xB0) */
#define SYN_USB_MIDI_CIN_PROGRAM_CHANGE 0xCU   /**< Program Change message (0xC0) */
#define SYN_USB_MIDI_CIN_CHANNEL_PRESSURE 0xDU /**< Channel Pressure message (0xD0) */
#define SYN_USB_MIDI_CIN_PITCH_BEND 0xEU       /**< Pitch Bend message (0xE0) */
#define SYN_USB_MIDI_CIN_SINGLE_BYTE 0xFU      /**< Single Byte message */

/** 4-byte USB MIDI Event Packet */
typedef struct {
    uint8_t header; /**< Cable Number (4 bits) | Code Index Number CIN (4 bits) */
    uint8_t midi0;  /**< MIDI Status byte (e.g. 0x90 Note On)                 */
    uint8_t midi1;  /**< MIDI Parameter 1 (e.g. Note Number)                    */
    uint8_t midi2;  /**< MIDI Parameter 2 (e.g. Velocity)                       */
} SYN_USB_MIDI_Packet;

/** USB MIDI Instance Context */
typedef struct {
    uint8_t ep_in;                                /**< Bulk IN Endpoint address */
    uint8_t ep_out;                               /**< Bulk OUT Endpoint address */
    uint8_t iface_num;                            /**< Assigned interface index */
    uint8_t cable_num;                            /**< Virtual Cable Number (0..15) */
    uint8_t tx_buf[SYN_USB_MIDI_MAX_PACKET_SIZE]; /**< IN packet buffer */
    uint16_t tx_len;                              /**< Pending IN byte length */
    uint8_t rx_buf[SYN_USB_MIDI_MAX_PACKET_SIZE]; /**< OUT packet buffer */
    uint16_t rx_len;                              /**< Unread OUT byte length */
} SYN_USB_MIDI;

/**
 * @brief Initialize USB MIDI Class Instance.
 * @param midi Pointer to USB MIDI instance.
 * @return SYN_OK on success, SYN_INVALID_PARAM if NULL.
 */
SYN_Status syn_usb_midi_init(SYN_USB_MIDI *midi);

/**
 * @brief Register USB MIDI class driver with USB device core.
 * @param dev Pointer to USB device core context.
 * @param midi Pointer to USB MIDI instance.
 * @return SYN_OK on success, SYN_INVALID_PARAM if NULL.
 */
SYN_Status syn_usb_midi_register(SYN_USB_Device *dev, SYN_USB_MIDI *midi);

/**
 * @brief Encode and queue a Note On MIDI message.
 * @param midi Pointer to USB MIDI instance.
 * @param channel MIDI Channel (0..15).
 * @param note MIDI Note Number (0..127).
 * @param velocity Note Velocity (0..127).
 * @return SYN_OK on success, SYN_INVALID_PARAM if invalid.
 */
SYN_Status syn_usb_midi_send_note_on(SYN_USB_MIDI *midi, uint8_t channel, uint8_t note,
                                     uint8_t velocity);

/**
 * @brief Encode and queue a Note Off MIDI message.
 * @param midi Pointer to USB MIDI instance.
 * @param channel MIDI Channel (0..15).
 * @param note MIDI Note Number (0..127).
 * @param velocity Release Velocity (0..127).
 * @return SYN_OK on success, SYN_INVALID_PARAM if invalid.
 */
SYN_Status syn_usb_midi_send_note_off(SYN_USB_MIDI *midi, uint8_t channel, uint8_t note,
                                      uint8_t velocity);

/**
 * @brief Encode and queue a Control Change (CC) MIDI message.
 * @param midi Pointer to USB MIDI instance.
 * @param channel MIDI Channel (0..15).
 * @param controller Controller Number (0..127).
 * @param value Controller Value (0..127).
 * @return SYN_OK on success, SYN_INVALID_PARAM if invalid.
 */
SYN_Status syn_usb_midi_send_cc(SYN_USB_MIDI *midi, uint8_t channel, uint8_t controller,
                                uint8_t value);

/**
 * @brief Encode and queue a Pitch Bend MIDI message.
 * @param midi Pointer to USB MIDI instance.
 * @param channel MIDI Channel (0..15).
 * @param value Pitch Bend value (-8192..+8191, center 0).
 * @return SYN_OK on success, SYN_INVALID_PARAM if invalid.
 */
SYN_Status syn_usb_midi_send_pitch_bend(SYN_USB_MIDI *midi, uint8_t channel, int16_t value);

/**
 * @brief Parse a 4-byte raw USB MIDI packet into structured event packet.
 * @param raw_bytes 4-byte raw buffer.
 * @param packet Output structured packet pointer.
 * @return SYN_OK on success, SYN_INVALID_PARAM if NULL.
 */
SYN_Status syn_usb_midi_parse_packet(const uint8_t raw_bytes[4], SYN_USB_MIDI_Packet *packet);

#ifdef __cplusplus
}
#endif

#endif /* SYN_USB_MIDI_H */
