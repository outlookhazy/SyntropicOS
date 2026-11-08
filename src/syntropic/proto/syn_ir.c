#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_IR) || SYN_USE_IR

/**
 * @file syn_ir.c
 * @brief Infrared (IR) Remote Control Protocol Engine implementation.
 */

#include "../util/syn_assert.h"
#include "syn_ir.h"

#include <string.h>

/* ── Protocol Descriptors ────────────────────────────────────────────────── */

/** @brief Internal infrared protocol timing descriptor. */
typedef struct {
    SYN_IR_Protocol proto;        /**< Protocol enum identifier */
    const char *name;             /**< Human readable protocol name */
    uint8_t carrier_khz;          /**< Modulation carrier frequency in kHz */
    SYN_IR_EncodingType encoding; /**< Pulse encoding type (PDM, PWM, Manchester) */
    uint16_t leader_mark_us;      /**< Preamble header mark duration in us */
    uint16_t leader_space_us;     /**< Preamble header space duration in us */
    uint16_t bit_mark_us;         /**< PDM mark duration or PWM '0' mark */
    uint16_t zero_space_us;       /**< PDM '0' space or PWM fixed space */
    uint16_t one_space_us;        /**< PDM '1' space */
    uint16_t one_mark_us;         /**< PWM '1' mark */
    uint16_t half_bit_us;         /**< Manchester half-bit duration */
    uint8_t total_bits;           /**< Total bits in frame */
    uint8_t addr_bits;            /**< Number of address bits */
    uint8_t cmd_bits;             /**< Number of command bits */
    uint16_t repeat_space_us;     /**< Repeat header space duration in us */
    uint16_t tolerance_us;        /**< Measurement tolerance window in us */
} SYN_IR_ProtoDesc;

/** Protocol timing lookup table. */
static const SYN_IR_ProtoDesc proto_table[SYN_IR_PROTO_COUNT] = {
    [SYN_IR_PROTO_UNKNOWN] = {.proto = SYN_IR_PROTO_UNKNOWN, .name = "Unknown"},
    [SYN_IR_PROTO_NEC] = {.proto = SYN_IR_PROTO_NEC,
                          .name = "NEC",
                          .carrier_khz = 38,
                          .encoding = SYN_IR_ENC_PDM,
                          .leader_mark_us = 9000,
                          .leader_space_us = 4500,
                          .bit_mark_us = 560,
                          .zero_space_us = 560,
                          .one_space_us = 1690,
                          .total_bits = 32,
                          .addr_bits = 8,
                          .cmd_bits = 8,
                          .repeat_space_us = 2250,
                          .tolerance_us = 200},
    [SYN_IR_PROTO_NEC_EXTENDED] = {.proto = SYN_IR_PROTO_NEC_EXTENDED,
                                   .name = "NEC-Extended",
                                   .carrier_khz = 38,
                                   .encoding = SYN_IR_ENC_PDM,
                                   .leader_mark_us = 9000,
                                   .leader_space_us = 4500,
                                   .bit_mark_us = 560,
                                   .zero_space_us = 560,
                                   .one_space_us = 1690,
                                   .total_bits = 32,
                                   .addr_bits = 16,
                                   .cmd_bits = 8,
                                   .repeat_space_us = 2250,
                                   .tolerance_us = 200},
    [SYN_IR_PROTO_SONY_12] = {.proto = SYN_IR_PROTO_SONY_12,
                              .name = "Sony-12",
                              .carrier_khz = 40,
                              .encoding = SYN_IR_ENC_PWM,
                              .leader_mark_us = 2400,
                              .leader_space_us = 600,
                              .bit_mark_us = 600,   /* '0' mark */
                              .one_mark_us = 1200,  /* '1' mark */
                              .zero_space_us = 600, /* fixed space */
                              .total_bits = 12,
                              .addr_bits = 5,
                              .cmd_bits = 7,
                              .tolerance_us = 150},
    [SYN_IR_PROTO_SONY_15] = {.proto = SYN_IR_PROTO_SONY_15,
                              .name = "Sony-15",
                              .carrier_khz = 40,
                              .encoding = SYN_IR_ENC_PWM,
                              .leader_mark_us = 2400,
                              .leader_space_us = 600,
                              .bit_mark_us = 600,
                              .one_mark_us = 1200,
                              .zero_space_us = 600,
                              .total_bits = 15,
                              .addr_bits = 8,
                              .cmd_bits = 7,
                              .tolerance_us = 150},
    [SYN_IR_PROTO_SONY_20] = {.proto = SYN_IR_PROTO_SONY_20,
                              .name = "Sony-20",
                              .carrier_khz = 40,
                              .encoding = SYN_IR_ENC_PWM,
                              .leader_mark_us = 2400,
                              .leader_space_us = 600,
                              .bit_mark_us = 600,
                              .one_mark_us = 1200,
                              .zero_space_us = 600,
                              .total_bits = 20,
                              .addr_bits = 5,
                              .cmd_bits = 7,
                              .tolerance_us = 150},
    [SYN_IR_PROTO_RC5] = {.proto = SYN_IR_PROTO_RC5,
                          .name = "Philips-RC5",
                          .carrier_khz = 36,
                          .encoding = SYN_IR_ENC_MANCHESTER,
                          .half_bit_us = 889,
                          .total_bits = 14,
                          .addr_bits = 5,
                          .cmd_bits = 6,
                          .tolerance_us = 200},
    [SYN_IR_PROTO_RC6] = {.proto = SYN_IR_PROTO_RC6,
                          .name = "Philips-RC6",
                          .carrier_khz = 36,
                          .encoding = SYN_IR_ENC_MANCHESTER,
                          .leader_mark_us = 2666,
                          .leader_space_us = 889,
                          .half_bit_us = 444,
                          .total_bits = 21,
                          .addr_bits = 8,
                          .cmd_bits = 8,
                          .tolerance_us = 150},
    [SYN_IR_PROTO_SAMSUNG] = {.proto = SYN_IR_PROTO_SAMSUNG,
                              .name = "Samsung",
                              .carrier_khz = 38,
                              .encoding = SYN_IR_ENC_PDM,
                              .leader_mark_us = 4500,
                              .leader_space_us = 4500,
                              .bit_mark_us = 590,
                              .zero_space_us = 590,
                              .one_space_us = 1690,
                              .total_bits = 32,
                              .addr_bits = 16,
                              .cmd_bits = 8,
                              .tolerance_us = 200},
    [SYN_IR_PROTO_KASEIKYO] = {.proto = SYN_IR_PROTO_KASEIKYO,
                               .name = "Kaseikyo/Panasonic",
                               .carrier_khz = 37,
                               .encoding = SYN_IR_ENC_PDM,
                               .leader_mark_us = 3456,
                               .leader_space_us = 1728,
                               .bit_mark_us = 432,
                               .zero_space_us = 432,
                               .one_space_us = 1296,
                               .total_bits = 48,
                               .addr_bits = 16,
                               .cmd_bits = 16,
                               .tolerance_us = 150},
    [SYN_IR_PROTO_DENON] = {.proto = SYN_IR_PROTO_DENON,
                            .name = "Denon/Sharp",
                            .carrier_khz = 38,
                            .encoding = SYN_IR_ENC_PDM,
                            .leader_mark_us = 0,
                            .leader_space_us = 0,
                            .bit_mark_us = 310,
                            .zero_space_us = 780,
                            .one_space_us = 1780,
                            .total_bits = 15,
                            .addr_bits = 5,
                            .cmd_bits = 8,
                            .tolerance_us = 150},
    [SYN_IR_PROTO_APPLE] = {.proto = SYN_IR_PROTO_APPLE,
                            .name = "Apple",
                            .carrier_khz = 38,
                            .encoding = SYN_IR_ENC_PDM,
                            .leader_mark_us = 9000,
                            .leader_space_us = 4500,
                            .bit_mark_us = 560,
                            .zero_space_us = 560,
                            .one_space_us = 1690,
                            .total_bits = 32,
                            .addr_bits = 16,
                            .cmd_bits = 8,
                            .repeat_space_us = 2250,
                            .tolerance_us = 200}};

/* ── Helper Functions ────────────────────────────────────────────────────── */

/**
 * @brief Check if measured pulse timing matches expected duration within tolerance.
 * @param actual Measured pulse duration in us.
 * @param expected Nominal target duration in us.
 * @param tolerance Allowable +/- error window in us.
 * @return true if within tolerance, false otherwise.
 */
static bool timing_match(uint16_t actual, uint16_t expected, uint16_t tolerance)
{
    uint16_t diff = (actual > expected) ? (actual - expected) : (expected - actual);
    return (diff <= tolerance);
}

const char *syn_ir_protocol_name(SYN_IR_Protocol proto)
{
    if (proto >= SYN_IR_PROTO_COUNT) {
        return "Unknown";
    }
    return proto_table[proto].name;
}

/**
 * @brief Reset IR decoder state machine.
 * @param decoder Decoder instance pointer.
 */
static void reset_decoder_state(SYN_IR_Decoder *decoder)
{
    bool have_last = decoder->have_last;
    SYN_IR_Frame last_f = decoder->last_frame;

    memset(decoder, 0, sizeof(SYN_IR_Decoder));
    decoder->have_last = have_last;
    decoder->last_frame = last_f;
    decoder->state = SYN_IR_STATE_IDLE;
    decoder->active_proto = SYN_IR_PROTO_UNKNOWN;
}

SYN_Status syn_ir_decoder_init(SYN_IR_Decoder *decoder)
{
    if (decoder == NULL) {
        return SYN_INVALID_PARAM;
    }
    memset(decoder, 0, sizeof(SYN_IR_Decoder));
    decoder->state = SYN_IR_STATE_IDLE;
    decoder->active_proto = SYN_IR_PROTO_UNKNOWN;
    return SYN_OK;
}

/**
 * @brief Unpack 32-bit NEC, NEC-Extended, or Apple IR bit stream into a decoded frame.
 * @param bit_idx Total number of decoded bits.
 * @param bits Raw 32-bit LSB-first received bit pattern.
 * @param frame_out Output decoded IR frame.
 * @return true if bit count and checksum/signature are valid, false otherwise.
 */
static bool unpack_nec_family(uint8_t bit_idx, uint64_t bits, SYN_IR_Frame *frame_out)
{
    if (bit_idx != 32U) {
        return false;
    }

    uint8_t addr = (uint8_t)(bits & 0xFFU);
    uint8_t addr_inv = (uint8_t)((bits >> 8U) & 0xFFU);
    uint8_t cmd = (uint8_t)((bits >> 16U) & 0xFFU);
    uint8_t cmd_inv = (uint8_t)((bits >> 24U) & 0xFFU);

    if ((bits & 0xFFFFU) == 0xEE87U) {
        frame_out->protocol = SYN_IR_PROTO_APPLE;
        frame_out->address = (uint16_t)(bits & 0xFFFFU);
        frame_out->command = cmd;
        return true;
    }
    if ((cmd ^ cmd_inv) == 0xFFU) {
        if ((addr ^ addr_inv) == 0xFFU) {
            frame_out->protocol = SYN_IR_PROTO_NEC;
            frame_out->address = addr;
        } else {
            frame_out->protocol = SYN_IR_PROTO_NEC_EXTENDED;
            frame_out->address = (uint16_t)(bits & 0xFFFFU);
        }
        frame_out->command = cmd;
        return true;
    }
    return false;
}

/**
 * @brief Unpack 12-bit, 15-bit, or 20-bit Sony SIRC bit stream into a decoded frame.
 * @param bit_idx Total number of decoded bits.
 * @param bits Raw LSB-first received bit pattern.
 * @param frame_out Output decoded IR frame.
 * @return true if bit count matches a valid Sony variant, false otherwise.
 */
static bool unpack_sony_family(uint8_t bit_idx, uint64_t bits, SYN_IR_Frame *frame_out)
{
    if (bit_idx == 12U) {
        frame_out->protocol = SYN_IR_PROTO_SONY_12;
        frame_out->command = (uint32_t)(bits & 0x7FU);
        frame_out->address = (uint32_t)((bits >> 7U) & 0x1FU);
        return true;
    }
    if (bit_idx == 15U) {
        frame_out->protocol = SYN_IR_PROTO_SONY_15;
        frame_out->command = (uint32_t)(bits & 0x7FU);
        frame_out->address = (uint32_t)((bits >> 7U) & 0xFFU);
        return true;
    }
    if (bit_idx == 20U) {
        frame_out->protocol = SYN_IR_PROTO_SONY_20;
        frame_out->command = (uint32_t)(bits & 0x7FU);
        frame_out->address =
            (uint32_t)((bits >> 7U) & 0x1FU) | (((uint32_t)((bits >> 12U) & 0xFFU)) << 5U);
        return true;
    }
    return false;
}

/**
 * @brief Unpack 14-bit Philips RC5 bit stream into a decoded frame.
 * @param bit_idx Total number of decoded bits.
 * @param bits Raw MSB-first received bit pattern.
 * @param frame_out Output decoded IR frame.
 * @return true if valid 14-bit RC5 frame, false otherwise.
 */
static bool unpack_rc5(uint8_t bit_idx, uint64_t bits, SYN_IR_Frame *frame_out)
{
    if (bit_idx != 14U) {
        return false;
    }
    if (((bits >> 11U) & 0x01U) != 0U) {
        frame_out->flags |= SYN_IR_FLAG_TOGGLE;
    }
    frame_out->address = (uint32_t)((bits >> 6U) & 0x1FU);
    frame_out->command = (uint32_t)(bits & 0x3FU);
    return true;
}

/**
 * @brief Unpack 21-bit Philips RC6 Mode 0 bit stream into a decoded frame.
 * @param bit_idx Total number of decoded bits.
 * @param bits Raw MSB-first received bit pattern.
 * @param frame_out Output decoded IR frame.
 * @return true if valid 21-bit RC6 Mode 0 frame (start bit == 1, mode == 0), false otherwise.
 */
static bool unpack_rc6(uint8_t bit_idx, uint64_t bits, SYN_IR_Frame *frame_out)
{
    if (bit_idx != 21U) {
        return false;
    }
    if ((((bits >> 20U) & 0x01U) != 1U) || (((bits >> 17U) & 0x07U) != 0U)) {
        return false;
    }
    if (((bits >> 16U) & 0x01U) != 0U) {
        frame_out->flags |= SYN_IR_FLAG_TOGGLE;
    }
    frame_out->address = (uint32_t)((bits >> 8U) & 0xFFU);
    frame_out->command = (uint32_t)(bits & 0xFFU);
    return true;
}

/**
 * @brief Unpack Samsung, Kaseikyo, or Denon PDM bit stream into a decoded frame.
 * @param proto Active IR protocol identifier.
 * @param bit_idx Total number of decoded bits.
 * @param bits Raw LSB-first received bit pattern.
 * @param frame_out Output decoded IR frame.
 * @return true if bit count and protocol checks pass, false otherwise.
 */
static bool unpack_other_pdm(SYN_IR_Protocol proto, uint8_t bit_idx, uint64_t bits,
                             SYN_IR_Frame *frame_out)
{
    if (proto == SYN_IR_PROTO_SAMSUNG) {
        uint8_t cmd = (uint8_t)((bits >> 16U) & 0xFFU);
        uint8_t cmd_inv = (uint8_t)((bits >> 24U) & 0xFFU);
        if ((bit_idx != 32U) || ((cmd ^ cmd_inv) != 0xFFU)) {
            return false;
        }
        frame_out->address = (uint16_t)(bits & 0xFFFFU);
        frame_out->command = cmd;
        return true;
    }

    if (proto == SYN_IR_PROTO_KASEIKYO) {
        if (bit_idx != 48U) {
            return false;
        }
        frame_out->address = (uint16_t)(bits & 0xFFFFU);
        frame_out->command = (uint32_t)((bits >> 16U) & 0xFFFFFFFFULL);
        return true;
    }

    /* SYN_IR_PROTO_DENON (15-bit) */
    if (bit_idx != 15U) {
        return false;
    }
    frame_out->address = (uint32_t)((bits >> 10U) & 0x1FU);
    frame_out->command = (uint32_t)((bits >> 2U) & 0xFFU);
    return true;
}

/**
 * @brief Unpack raw 64-bit frame buffer into decoded SYN_IR_Frame fields.
 * @param decoder Pointer to IR decoder instance.
 * @param frame_out Pointer to output frame structure.
 * @return True on successful frame unpacking, false otherwise.
 */
static bool unpack_frame(const SYN_IR_Decoder *decoder, SYN_IR_Frame *frame_out)
{
    SYN_IR_Protocol proto = decoder->active_proto;
    const SYN_IR_ProtoDesc *desc = &proto_table[proto];

    frame_out->protocol = proto;
    frame_out->carrier_khz = desc->carrier_khz;
    frame_out->flags = SYN_IR_FLAG_NONE;
    frame_out->bit_count = decoder->bit_idx;

    uint64_t bits = decoder->bits;

    switch (proto) {
    case SYN_IR_PROTO_NEC:
    case SYN_IR_PROTO_NEC_EXTENDED:
    case SYN_IR_PROTO_APPLE:
        return unpack_nec_family(decoder->bit_idx, bits, frame_out);

    case SYN_IR_PROTO_SONY_12:
    case SYN_IR_PROTO_SONY_15:
    case SYN_IR_PROTO_SONY_20:
        return unpack_sony_family(decoder->bit_idx, bits, frame_out);

    case SYN_IR_PROTO_RC5:
        return unpack_rc5(decoder->bit_idx, bits, frame_out);

    case SYN_IR_PROTO_RC6:
        return unpack_rc6(decoder->bit_idx, bits, frame_out);

    default:
        return unpack_other_pdm(proto, decoder->bit_idx, bits, frame_out);
    }
}

/* ── Decoder FSM ─────────────────────────────────────────────────────────── */

/**
 * @brief Unpack completed bit buffer, cache repeat frame, and reset decoder state.
 * @param decoder IR decoder instance.
 * @param frame_out Output decoded IR frame.
 * @return true if a valid frame was unpacked, false otherwise.
 */
static bool ir_finish_frame(SYN_IR_Decoder *decoder, SYN_IR_Frame *frame_out)
{
    if (unpack_frame(decoder, frame_out)) {
        decoder->last_frame = *frame_out;
        decoder->have_last = true;
        reset_decoder_state(decoder);
        return true;
    }
    reset_decoder_state(decoder);
    return false;
}

/**
 * @brief Feed one half-bit level into the Manchester (RC5/RC6) decoder state machine.
 * @param decoder IR decoder instance.
 * @param is_mark true if half-bit is carrier mark, false if space.
 * @param frame_out Output decoded IR frame when final bit completes.
 * @return true if a complete frame was decoded, false otherwise.
 */
static bool ir_manchester_push_half(SYN_IR_Decoder *decoder, bool is_mark, SYN_IR_Frame *frame_out)
{
    if (!decoder->manchester_phase) {
        decoder->last_mark_us = is_mark ? 1U : 0U;
        decoder->manchester_phase = true;
        return false;
    }

    bool first_mark = (decoder->last_mark_us != 0U);
    if (first_mark == is_mark) {
        reset_decoder_state(decoder);
        return false;
    }

    bool bit_one = (decoder->active_proto == SYN_IR_PROTO_RC6) ? first_mark : is_mark;
    uint8_t bit_val = bit_one ? 1U : 0U;
    decoder->bits = (decoder->bits << 1U) | (uint64_t)bit_val;
    decoder->bit_idx++;
    decoder->manchester_phase = false;

    if (decoder->bit_idx >= decoder->expected_bits) {
        return ir_finish_frame(decoder, frame_out);
    }
    return false;
}

/**
 * @brief Feed one base timing quantum into the Manchester decoder, coalescing RC6 2T trailer bits.
 * @param decoder IR decoder instance.
 * @param is_mark true if quantum is carrier mark, false if space.
 * @param frame_out Output decoded IR frame when final bit completes.
 * @return true if a complete frame was decoded, false otherwise.
 */
static bool ir_manchester_push_quantum(SYN_IR_Decoder *decoder, bool is_mark,
                                       SYN_IR_Frame *frame_out)
{
    /* RC6 Mode 0 bit 4 (Trailer/Toggle) uses 2T half-bit width */
    if (decoder->active_proto == SYN_IR_PROTO_RC6 && decoder->bit_idx == 4U) {
        if (decoder->last_space_us == 0U) {
            decoder->last_space_us = is_mark ? 2U : 1U;
            return false;
        }
        bool prev_mark = (decoder->last_space_us == 2U);
        decoder->last_space_us = 0U;
        if (prev_mark != is_mark) {
            reset_decoder_state(decoder);
            return false;
        }
    }
    return ir_manchester_push_half(decoder, is_mark, frame_out);
}

/**
 * @brief Quantize pulse duration into Manchester half-bit units and advance RC5/RC6 decoding.
 * @param decoder IR decoder instance.
 * @param duration_us Measured pulse or space duration in microseconds.
 * @param is_mark true for mark pulse, false for space.
 * @param frame_out Output decoded IR frame when complete.
 * @return true if a complete frame was decoded, false otherwise.
 */
static bool ir_decode_manchester(SYN_IR_Decoder *decoder, uint16_t duration_us, bool is_mark,
                                 SYN_IR_Frame *frame_out)
{
    const SYN_IR_ProtoDesc *desc = &proto_table[decoder->active_proto];
    uint16_t t_us = desc->half_bit_us;
    uint16_t tol = desc->tolerance_us;
    uint8_t quanta = 0U;

    if (timing_match(duration_us, t_us, tol)) {
        quanta = 1U;
    } else if (timing_match(duration_us, (uint16_t)(t_us * 2U), (uint16_t)(tol * 2U))) {
        quanta = 2U;
    } else if (decoder->active_proto == SYN_IR_PROTO_RC6 &&
               timing_match(duration_us, (uint16_t)(t_us * 3U), (uint16_t)(tol * 2U))) {
        quanta = 3U;
    } else if (!is_mark && duration_us > (uint16_t)(t_us * 3U) && decoder->manchester_phase &&
               decoder->bit_idx + 1U == decoder->expected_bits) {
        quanta = 1U;
    } else {
        reset_decoder_state(decoder);
        return false;
    }

    for (uint8_t q = 0U; q < quanta; q++) {
        if (decoder->state != SYN_IR_STATE_DATA) {
            return false;
        }
        if (ir_manchester_push_quantum(decoder, is_mark, frame_out)) {
            return true;
        }
    }
    return false;
}

/**
 * @brief Decode one mark or space pulse in Sony SIRC pulse-width modulation mode.
 * @param decoder IR decoder instance.
 * @param desc Protocol timing descriptor.
 * @param duration_us Measured pulse duration in microseconds.
 * @param is_mark true for mark pulse, false for space.
 * @param frame_out Output decoded IR frame when complete.
 * @return true if a complete frame was decoded, false otherwise.
 */
static bool ir_decode_sony_pulse(SYN_IR_Decoder *decoder, const SYN_IR_ProtoDesc *desc,
                                 uint16_t duration_us, bool is_mark, SYN_IR_Frame *frame_out)
{
    if (is_mark) {
        uint8_t bit_val = 0U;
        if (timing_match(duration_us, desc->one_mark_us, desc->tolerance_us)) {
            bit_val = 1U;
        } else if (!timing_match(duration_us, desc->bit_mark_us, desc->tolerance_us)) {
            reset_decoder_state(decoder);
            return false;
        }

        decoder->bits |= ((uint64_t)bit_val << decoder->bit_idx);
        decoder->bit_idx++;
        if (decoder->bit_idx >= 20U) {
            return ir_finish_frame(decoder, frame_out);
        }
        return false;
    }

    /* Space in Sony PWM: 600us continues frame; inter-frame gap finalizes 12-bit or 15-bit */
    if (timing_match(duration_us, desc->zero_space_us, desc->tolerance_us)) {
        return false;
    }
    if (duration_us > (uint16_t)(desc->zero_space_us + desc->tolerance_us) &&
        (decoder->bit_idx == 12U || decoder->bit_idx == 15U)) {
        return ir_finish_frame(decoder, frame_out);
    }
    reset_decoder_state(decoder);
    return false;
}

/**
 * @brief Decode one mark/space pair in pulse-distance modulation (PDM) data state.
 * @param decoder IR decoder instance.
 * @param desc Protocol timing descriptor.
 * @param mark_us Preceding mark duration in microseconds.
 * @param space_us Measured space duration in microseconds.
 * @param frame_out Output decoded IR frame when complete.
 * @return true if a complete frame was decoded, false otherwise.
 */
static bool ir_decode_pdm_space(SYN_IR_Decoder *decoder, const SYN_IR_ProtoDesc *desc,
                                uint16_t mark_us, uint16_t space_us, SYN_IR_Frame *frame_out)
{
    if (!timing_match(mark_us, desc->bit_mark_us, desc->tolerance_us)) {
        reset_decoder_state(decoder);
        return false;
    }

    uint8_t bit_val = 0U;
    if (timing_match(space_us, desc->one_space_us, desc->tolerance_us)) {
        bit_val = 1U;
    } else if (!timing_match(space_us, desc->zero_space_us, desc->tolerance_us)) {
        reset_decoder_state(decoder);
        return false;
    }

    decoder->bits |= ((uint64_t)bit_val << decoder->bit_idx);
    decoder->bit_idx++;

    if (decoder->bit_idx >= decoder->expected_bits) {
        return ir_finish_frame(decoder, frame_out);
    }
    return false;
}

/**
 * @brief Check whether protocol uses NEC-compatible 9ms+2.25ms repeat pulses.
 * @param proto IR protocol identifier.
 * @return true if NEC, NEC_EXTENDED, or APPLE; false otherwise.
 */
static bool ir_is_nec_family(SYN_IR_Protocol proto)
{
    return (proto == SYN_IR_PROTO_NEC) || (proto == SYN_IR_PROTO_NEC_EXTENDED) ||
           (proto == SYN_IR_PROTO_APPLE);
}

/**
 * @brief Check whether mark/space matches an NEC-family repeat frame.
 * @param decoder   IR decoder instance.
 * @param mark_us   Preceding mark duration in microseconds.
 * @param space_us  Current space duration in microseconds.
 * @param frame_out Output decoded IR frame (populated on repeat).
 * @return true if repeat frame emitted, false otherwise.
 */
static bool ir_try_nec_repeat(const SYN_IR_Decoder *decoder, uint16_t mark_us, uint16_t space_us,
                              SYN_IR_Frame *frame_out)
{
    const SYN_IR_ProtoDesc *nec = &proto_table[SYN_IR_PROTO_NEC];
    if (decoder->have_last && ir_is_nec_family(decoder->last_frame.protocol) &&
        timing_match(mark_us, nec->leader_mark_us, nec->tolerance_us) &&
        timing_match(space_us, nec->repeat_space_us, nec->tolerance_us)) {
        *frame_out = decoder->last_frame;
        frame_out->flags |= SYN_IR_FLAG_REPEAT;
        return true;
    }
    return false;
}

/**
 * @brief Match leader mark/space against protocol descriptor table.
 * @param decoder  IR decoder instance.
 * @param mark_us  Preceding mark duration in microseconds.
 * @param space_us Current space duration in microseconds.
 * @return true if a protocol leader matched and transitioned state, false otherwise.
 */
static bool ir_match_leader(SYN_IR_Decoder *decoder, uint16_t mark_us, uint16_t space_us)
{
    for (int p = 1; p < SYN_IR_PROTO_COUNT; p++) {
        if ((p == SYN_IR_PROTO_SONY_12) || (p == SYN_IR_PROTO_SONY_15) ||
            (p == SYN_IR_PROTO_NEC_EXTENDED)) {
            continue;
        }

        const SYN_IR_ProtoDesc *desc = &proto_table[p];
        if ((desc->leader_mark_us > 0U) &&
            timing_match(mark_us, desc->leader_mark_us, desc->tolerance_us) &&
            timing_match(space_us, desc->leader_space_us, desc->tolerance_us)) {
            decoder->active_proto = (SYN_IR_Protocol)p;
            decoder->expected_bits = desc->total_bits;
            decoder->state = SYN_IR_STATE_DATA;
            decoder->bits = 0ULL;
            decoder->bit_idx = 0U;
            decoder->manchester_phase = false;
            decoder->last_space_us = 0U;
            return true;
        }
    }
    return false;
}

/**
 * @brief Detect protocol preamble, repeat code, or leaderless start from idle mark/space pair.
 * @param decoder IR decoder instance.
 * @param mark_us Preceding mark duration in microseconds.
 * @param space_us Measured space duration in microseconds.
 * @param frame_out Output decoded IR frame on repeat match or complete frame.
 * @return true if a frame (such as NEC repeat) is ready immediately, false otherwise.
 */
static bool ir_decode_idle_space(SYN_IR_Decoder *decoder, uint16_t mark_us, uint16_t space_us,
                                 SYN_IR_Frame *frame_out)
{
    if (ir_try_nec_repeat(decoder, mark_us, space_us, frame_out)) {
        return true;
    }
    if (ir_match_leader(decoder, mark_us, space_us)) {
        return false;
    }

    const SYN_IR_ProtoDesc *denon = &proto_table[SYN_IR_PROTO_DENON];
    if (timing_match(mark_us, denon->bit_mark_us, denon->tolerance_us) &&
        (timing_match(space_us, denon->zero_space_us, denon->tolerance_us) ||
         timing_match(space_us, denon->one_space_us, denon->tolerance_us))) {
        decoder->active_proto = SYN_IR_PROTO_DENON;
        decoder->expected_bits = denon->total_bits;
        decoder->state = SYN_IR_STATE_DATA;
        uint8_t bit = timing_match(space_us, denon->one_space_us, denon->tolerance_us) ? 1U : 0U;
        decoder->bits = (uint64_t)bit;
        decoder->bit_idx = 1U;
        return false;
    }

    const SYN_IR_ProtoDesc *rc5 = &proto_table[SYN_IR_PROTO_RC5];
    uint16_t t_rc5 = rc5->half_bit_us;
    uint16_t tol_rc5 = rc5->tolerance_us;
    bool mark_ok = timing_match(mark_us, t_rc5, tol_rc5) ||
                   timing_match(mark_us, (uint16_t)(t_rc5 * 2U), (uint16_t)(tol_rc5 * 2U));
    bool space_ok = timing_match(space_us, t_rc5, tol_rc5) ||
                    timing_match(space_us, (uint16_t)(t_rc5 * 2U), (uint16_t)(tol_rc5 * 2U));
    if (mark_ok && space_ok) {
        decoder->active_proto = SYN_IR_PROTO_RC5;
        decoder->expected_bits = rc5->total_bits;
        decoder->state = SYN_IR_STATE_DATA;
        decoder->bits = 0ULL;
        decoder->bit_idx = 0U;
        decoder->manchester_phase = true; /* Leading S1 space elapsed in idle */
        decoder->last_mark_us = 0U;
        decoder->last_space_us = 0U;
        (void)ir_decode_manchester(decoder, mark_us, true, frame_out);
        return ir_decode_manchester(decoder, space_us, false, frame_out);
    }

    return false;
}

bool syn_ir_decode_pulse(SYN_IR_Decoder *decoder, uint16_t duration_us, bool is_mark,
                         SYN_IR_Frame *frame_out)
{
    if (decoder == NULL || frame_out == NULL) {
        return false;
    }

    if (decoder->state == SYN_IR_STATE_DATA) {
        if ((int)decoder->active_proto <= 0 || decoder->active_proto >= SYN_IR_PROTO_COUNT) {
            reset_decoder_state(decoder);
            return false;
        }
        const SYN_IR_ProtoDesc *desc = &proto_table[decoder->active_proto];
        if (desc->encoding == SYN_IR_ENC_MANCHESTER) {
            return ir_decode_manchester(decoder, duration_us, is_mark, frame_out);
        }
        if (desc->encoding == SYN_IR_ENC_PWM) {
            return ir_decode_sony_pulse(decoder, desc, duration_us, is_mark, frame_out);
        }
    }

    if (is_mark) {
        decoder->last_mark_us = duration_us;
        return false;
    }

    if (decoder->last_mark_us == 0U) {
        return false;
    }

    uint16_t mark_us = decoder->last_mark_us;
    decoder->last_mark_us = 0U;
    decoder->last_space_us = duration_us;

    if (decoder->state == SYN_IR_STATE_IDLE) {
        return ir_decode_idle_space(decoder, mark_us, duration_us, frame_out);
    }

    const SYN_IR_ProtoDesc *desc = &proto_table[decoder->active_proto];
    return ir_decode_pdm_space(decoder, desc, mark_us, duration_us, frame_out);
}

bool syn_ir_decode_timeout(SYN_IR_Decoder *decoder, SYN_IR_Frame *frame_out)
{
    if (decoder == NULL || frame_out == NULL) {
        return false;
    }

    if (decoder->state == SYN_IR_STATE_DATA && (int)decoder->active_proto > 0 &&
        decoder->active_proto < SYN_IR_PROTO_COUNT) {
        const SYN_IR_ProtoDesc *desc = &proto_table[decoder->active_proto];
        if (desc->encoding == SYN_IR_ENC_MANCHESTER && decoder->manchester_phase &&
            decoder->bit_idx + 1U == decoder->expected_bits) {
            return ir_manchester_push_half(decoder, false, frame_out);
        }
        if (decoder->bit_idx > 0U) {
            return ir_finish_frame(decoder, frame_out);
        }
    }

    reset_decoder_state(decoder);
    return false;
}

/* ── Encoder ─────────────────────────────────────────────────────────────── */

/**
 * @brief Append or coalesce a mark/space pulse entry in the output pulse buffer.
 * @param buf Output pulse array.
 * @param idx Current pulse write index, updated in-place.
 * @param dur_us Pulse duration in microseconds.
 * @param is_mark true for carrier mark, false for space.
 */
static void ir_emit_pulse(SYN_IR_Pulse *buf, size_t *idx, uint16_t dur_us, bool is_mark)
{
    if (*idx == 0U && !is_mark) {
        return;
    }
    if (*idx > 0U && buf[*idx - 1U].is_mark == is_mark) {
        buf[*idx - 1U].duration_us = (uint16_t)(buf[*idx - 1U].duration_us + dur_us);
        return;
    }
    buf[*idx] = (SYN_IR_Pulse){.duration_us = dur_us, .is_mark = is_mark};
    (*idx)++;
}

/**
 * @brief Pack address, command, and protocol-specific control bits into a 64-bit word.
 * @param frame Input IR frame descriptor.
 * @return Packed raw bit pattern for protocol transmission.
 */
static uint64_t ir_pack_raw_bits(const SYN_IR_Frame *frame)
{
    switch (frame->protocol) {
    case SYN_IR_PROTO_NEC: {
        uint8_t addr = (uint8_t)(frame->address & 0xFFU);
        uint8_t cmd = (uint8_t)(frame->command & 0xFFU);
        return ((uint64_t)addr) | (((uint64_t)(~addr & 0xFFU)) << 8U) | (((uint64_t)cmd) << 16U) |
               (((uint64_t)(~cmd & 0xFFU)) << 24U);
    }
    case SYN_IR_PROTO_NEC_EXTENDED: {
        uint16_t addr = (uint16_t)(frame->address & 0xFFFFU);
        uint8_t cmd = (uint8_t)(frame->command & 0xFFU);
        return ((uint64_t)addr) | (((uint64_t)cmd) << 16U) | (((uint64_t)(~cmd & 0xFFU)) << 24U);
    }
    case SYN_IR_PROTO_APPLE: {
        uint16_t addr = (uint16_t)(frame->address & 0xFFFFU);
        uint8_t cmd = (uint8_t)(frame->command & 0xFFU);
        return ((uint64_t)addr) | (((uint64_t)cmd) << 16U) | (((uint64_t)cmd) << 24U);
    }
    case SYN_IR_PROTO_SONY_12:
        return (frame->command & 0x7FU) | ((frame->address & 0x1FU) << 7U);
    case SYN_IR_PROTO_SONY_15:
        return (frame->command & 0x7FU) | ((frame->address & 0xFFU) << 7U);
    case SYN_IR_PROTO_SONY_20:
        return (frame->command & 0x7FU) | ((frame->address & 0x1FU) << 7U) |
               (((frame->address >> 5U) & 0xFFU) << 12U);
    case SYN_IR_PROTO_RC5: {
        uint64_t toggle = ((frame->flags & SYN_IR_FLAG_TOGGLE) != 0U) ? 1ULL : 0ULL;
        return (1ULL << 13U) | (1ULL << 12U) | (toggle << 11U) |
               (((uint64_t)(frame->address & 0x1FU)) << 6U) | ((uint64_t)(frame->command & 0x3FU));
    }
    case SYN_IR_PROTO_RC6: {
        uint64_t toggle = ((frame->flags & SYN_IR_FLAG_TOGGLE) != 0U) ? 1ULL : 0ULL;
        return (1ULL << 20U) | (toggle << 16U) | (((uint64_t)(frame->address & 0xFFU)) << 8U) |
               ((uint64_t)(frame->command & 0xFFU));
    }
    case SYN_IR_PROTO_SAMSUNG: {
        uint16_t addr = (uint16_t)(frame->address & 0xFFFFU);
        uint8_t cmd = (uint8_t)(frame->command & 0xFFU);
        return ((uint64_t)addr) | (((uint64_t)cmd) << 16U) | (((uint64_t)(~cmd & 0xFFU)) << 24U);
    }
    case SYN_IR_PROTO_KASEIKYO:
        return (frame->address & 0xFFFFU) | (((uint64_t)frame->command) << 16U);
    default:
        /* SYN_IR_PROTO_DENON */
        return ((frame->address & 0x1FU) << 10U) | ((frame->command & 0xFFU) << 2U);
    }
}

/**
 * @brief Encode Manchester (RC5/RC6) bit pattern into mark/space pulses.
 * @param frame Input IR frame descriptor.
 * @param desc Protocol timing descriptor.
 * @param raw_bits Packed MSB-first raw bit word.
 * @param pulse_buf Output pulse array.
 * @param idx Current pulse write index, updated in-place.
 */
static void ir_encode_manchester(const SYN_IR_Frame *frame, const SYN_IR_ProtoDesc *desc,
                                 uint64_t raw_bits, SYN_IR_Pulse *pulse_buf, size_t *idx)
{
    bool is_rc6 = (frame->protocol == SYN_IR_PROTO_RC6);
    for (int b = (int)desc->total_bits - 1; b >= 0; b--) {
        uint8_t bit_val = (uint8_t)((raw_bits >> (uint8_t)b) & 0x01U);
        uint16_t half = desc->half_bit_us;
        if (is_rc6 && b == 16) {
            half = (uint16_t)(half * 2U); /* RC6 Mode 0 2T Trailer/Toggle bit */
        }
        bool first_mark = is_rc6 ? (bit_val == 1U) : (bit_val == 0U);
        ir_emit_pulse(pulse_buf, idx, half, first_mark);
        ir_emit_pulse(pulse_buf, idx, half, !first_mark);
    }
    if (*idx > 0U && pulse_buf[*idx - 1U].is_mark) {
        ir_emit_pulse(pulse_buf, idx, (uint16_t)(desc->half_bit_us * 4U), false);
    }
}

SYN_Status syn_ir_encode_frame(const SYN_IR_Frame *frame, SYN_IR_Pulse *pulse_buf, size_t buf_len,
                               size_t *count_out)
{
    if (frame == NULL || pulse_buf == NULL || count_out == NULL) {
        return SYN_INVALID_PARAM;
    }

    if ((int)frame->protocol <= 0 || frame->protocol >= SYN_IR_PROTO_COUNT) {
        return SYN_INVALID_PARAM;
    }

    const SYN_IR_ProtoDesc *desc = &proto_table[frame->protocol];
    size_t required_pulses = (desc->leader_mark_us > 0U) ? 2U : 0U;
    required_pulses += ((size_t)desc->total_bits) * 2U + 1U;

    if (buf_len < required_pulses) {
        return SYN_ERROR;
    }

    size_t idx = 0U;
    if (desc->leader_mark_us > 0U) {
        pulse_buf[idx++] = (SYN_IR_Pulse){.duration_us = desc->leader_mark_us, .is_mark = true};
        pulse_buf[idx++] = (SYN_IR_Pulse){.duration_us = desc->leader_space_us, .is_mark = false};
    }

    uint64_t raw_bits = ir_pack_raw_bits(frame);

    if (desc->encoding == SYN_IR_ENC_MANCHESTER) {
        ir_encode_manchester(frame, desc, raw_bits, pulse_buf, &idx);
    } else if (desc->encoding == SYN_IR_ENC_PWM) {
        for (uint8_t b = 0U; b < desc->total_bits; b++) {
            uint8_t bit_val = (uint8_t)((raw_bits >> b) & 0x01U);
            uint16_t mark = (bit_val == 1U) ? desc->one_mark_us : desc->bit_mark_us;
            uint16_t space = (b + 1U < desc->total_bits) ? desc->zero_space_us : 10000U;
            pulse_buf[idx++] = (SYN_IR_Pulse){.duration_us = mark, .is_mark = true};
            pulse_buf[idx++] = (SYN_IR_Pulse){.duration_us = space, .is_mark = false};
        }
    } else {
        for (uint8_t b = 0U; b < desc->total_bits; b++) {
            uint8_t bit_val = (uint8_t)((raw_bits >> b) & 0x01U);
            uint16_t space = (bit_val == 1U) ? desc->one_space_us : desc->zero_space_us;
            pulse_buf[idx++] = (SYN_IR_Pulse){.duration_us = desc->bit_mark_us, .is_mark = true};
            pulse_buf[idx++] = (SYN_IR_Pulse){.duration_us = space, .is_mark = false};
        }
        pulse_buf[idx++] = (SYN_IR_Pulse){.duration_us = desc->bit_mark_us, .is_mark = true};
    }

    *count_out = idx;
    return SYN_OK;
}

#endif /* SYN_USE_IR */
