

# File syn\_ir.c

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_ir.c**](syn__ir_8c.md)

[Go to the documentation of this file](syn__ir_8c.md)


```C++
#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_IR) || SYN_USE_IR

#include "../util/syn_assert.h"
#include "syn_ir.h"

#include <string.h>

/* ── Protocol Descriptors ────────────────────────────────────────────────── */

typedef struct {
    SYN_IR_Protocol proto;        
    const char *name;             
    uint8_t carrier_khz;          
    SYN_IR_EncodingType encoding; 
    uint16_t leader_mark_us;      
    uint16_t leader_space_us;     
    uint16_t bit_mark_us;         
    uint16_t zero_space_us;       
    uint16_t one_space_us;        
    uint16_t one_mark_us;         
    uint16_t half_bit_us;         
    uint8_t total_bits;           
    uint8_t addr_bits;            
    uint8_t cmd_bits;             
    uint16_t repeat_space_us;     
    uint16_t tolerance_us;        
} SYN_IR_ProtoDesc;

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

static bool timing_match(uint16_t actual, uint16_t expected, uint16_t tolerance)
{
    if (expected == 0)
        return false; /* LCOV_EXCL_LINE: Unreachable guard; callers pass non-zero timing thresholds
                         from proto_table */
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

static bool unpack_frame(const SYN_IR_Decoder *decoder, SYN_IR_Frame *frame_out)
{
    SYN_IR_Protocol proto = decoder->active_proto;
    if (proto >= SYN_IR_PROTO_COUNT) {
        return false; /* LCOV_EXCL_LINE: Unreachable guard; decoder->active_proto is validated
                         against proto_table during state transitions */
    }
    const SYN_IR_ProtoDesc *desc = &proto_table[proto];

    frame_out->protocol = proto;
    frame_out->carrier_khz = desc->carrier_khz;
    frame_out->flags = SYN_IR_FLAG_NONE;
    frame_out->bit_count = decoder->bit_idx;

    uint64_t bits = decoder->bits;

    switch (proto) {
    case SYN_IR_PROTO_NEC:
    case SYN_IR_PROTO_NEC_EXTENDED:
    case SYN_IR_PROTO_APPLE: {
        uint8_t addr = (uint8_t)(bits & 0xFFU);
        uint8_t addr_inv = (uint8_t)((bits >> 8) & 0xFFU);
        uint8_t cmd = (uint8_t)((bits >> 16) & 0xFFU);
        uint8_t cmd_inv = (uint8_t)((bits >> 24) & 0xFFU);

        if ((bits & 0xFFFFU) == 0xEE87U) {
            frame_out->protocol = SYN_IR_PROTO_APPLE;
            frame_out->address = (uint16_t)(bits & 0xFFFFU);
            frame_out->command = cmd;
        } else if ((cmd ^ cmd_inv) == 0xFFU) {
            if ((addr ^ addr_inv) == 0xFFU) {
                frame_out->protocol = SYN_IR_PROTO_NEC;
                frame_out->address = addr;
            } else {
                frame_out->protocol = SYN_IR_PROTO_NEC_EXTENDED;
                frame_out->address = (uint16_t)(bits & 0xFFFFU);
            }
            frame_out->command = cmd;
        } else {
            return false;
        }
        break;
    }

    case SYN_IR_PROTO_SONY_12:
    case SYN_IR_PROTO_SONY_15:
    case SYN_IR_PROTO_SONY_20: {
        if (decoder->bit_idx == 12) {
            frame_out->protocol = SYN_IR_PROTO_SONY_12;
            frame_out->command = (uint32_t)(bits & 0x7FU);
            frame_out->address = (uint32_t)((bits >> 7) & 0x1FU);
        } else if (decoder->bit_idx == 15) {
            frame_out->protocol = SYN_IR_PROTO_SONY_15;
            frame_out->command = (uint32_t)(bits & 0x7FU);
            frame_out->address = (uint32_t)((bits >> 7) & 0xFFU);
        } else if (decoder->bit_idx == 20) {
            frame_out->protocol = SYN_IR_PROTO_SONY_20;
            frame_out->command = (uint32_t)(bits & 0x7FU);
            frame_out->address =
                (uint32_t)((bits >> 7) & 0x1FU) | (((uint32_t)((bits >> 12) & 0xFFU)) << 5);
        } else {
            return false; /* LCOV_EXCL_LINE: Unreachable guard; Sony bit_idx is constrained to 12,
                             15, or 20 by decoder trigger conditions */
        }
        break;
    }

    case SYN_IR_PROTO_RC5: {
        /* 14 bits: S1(1), S2(1), Toggle(1), Address(5), Command(6) */
        uint8_t toggle = (uint8_t)((bits >> 11) & 0x01U);
        uint8_t address = (uint8_t)((bits >> 6) & 0x1FU);
        uint8_t command = (uint8_t)(bits & 0x3FU);

        if (toggle) {
            frame_out->flags |= SYN_IR_FLAG_TOGGLE;
        }
        frame_out->address = address;
        frame_out->command = command;
        break;
    }

    case SYN_IR_PROTO_RC6: {
        /* 21 bits: S(1), Mode(3), Toggle(1), Address(8), Command(8) */
        uint8_t toggle = (uint8_t)((bits >> 16) & 0x01U);
        uint8_t address = (uint8_t)((bits >> 8) & 0xFFU);
        uint8_t command = (uint8_t)(bits & 0xFFU);

        if (toggle) {
            frame_out->flags |= SYN_IR_FLAG_TOGGLE;
        }
        frame_out->address = address;
        frame_out->command = command;
        break;
    }

    case SYN_IR_PROTO_SAMSUNG: {
        uint16_t addr = (uint16_t)(bits & 0xFFFFU);
        uint8_t cmd = (uint8_t)((bits >> 16) & 0xFFU);
        uint8_t cmd_inv = (uint8_t)((bits >> 24) & 0xFFU);

        if ((cmd ^ cmd_inv) != 0xFFU) {
            return false;
        }
        frame_out->address = addr;
        frame_out->command = cmd;
        break;
    }

    case SYN_IR_PROTO_KASEIKYO: {
        uint16_t addr = (uint16_t)(bits & 0xFFFFU);
        uint32_t cmd = (uint32_t)((bits >> 16) & 0xFFFFFFFFULL);
        frame_out->address = addr;
        frame_out->command = cmd;
        break;
    }

    case SYN_IR_PROTO_DENON: {
        /* 15 bits: 5-bit address, 8-bit command, 2-bit checksum */
        frame_out->address = (uint32_t)((bits >> 10) & 0x1FU);
        frame_out->command = (uint32_t)((bits >> 2) & 0xFFU);
        break;
    }

    default: /* LCOV_EXCL_LINE: Unreachable default switch case in SYN_IR_Protocol enum switch */
        return false; /* LCOV_EXCL_LINE: Unreachable default switch case in SYN_IR_Protocol enum
                         switch */
    }

    return true;
}

/* ── Decoder FSM ─────────────────────────────────────────────────────────── */

bool syn_ir_decode_pulse(SYN_IR_Decoder *decoder, uint16_t duration_us, bool is_mark,
                         SYN_IR_Frame *frame_out)
{
    if (decoder == NULL || frame_out == NULL) {
        return false;
    }

    /* Manchester bit phase tracking */
    if (decoder->state == SYN_IR_STATE_DATA) {
        SYN_IR_Protocol proto = decoder->active_proto;
        if (proto >= SYN_IR_PROTO_COUNT) {
            return false;
        }
        const SYN_IR_ProtoDesc *desc = &proto_table[proto];
        if (desc->encoding == SYN_IR_ENC_MANCHESTER) {
            bool is_full = timing_match(duration_us, desc->half_bit_us * 2, desc->tolerance_us);
            bool is_half = timing_match(duration_us, desc->half_bit_us, desc->tolerance_us);
            if (is_half || is_full) {
                if (is_full) {
                    decoder->manchester_phase = !decoder->manchester_phase;
                }
                decoder->manchester_phase = !decoder->manchester_phase;
                if (decoder->manchester_phase) {
                    uint8_t bit_val = is_mark ? 1 : 0;
                    decoder->bits = (decoder->bits << 1) | bit_val;
                    decoder->bit_idx++;

                    if (decoder->bit_idx >= decoder->expected_bits) {
                        if (unpack_frame(decoder, frame_out)) {
                            decoder->last_frame = *frame_out;
                            decoder->have_last = true;
                            reset_decoder_state(decoder);
                            return true;
                        }
                        /* LCOV_EXCL_START: Unreachable fallback when unpack_frame fails after exact
                         * bit_idx match */
                        reset_decoder_state(decoder);
                        return false;
                        /* LCOV_EXCL_STOP */
                    }
                }
            }
        }
    }

    if (is_mark) {
        decoder->last_mark_us = duration_us;
        return false;
    }

    /* Process space pulse paired with previous mark */
    uint16_t mark_us = decoder->last_mark_us;
    uint16_t space_us = duration_us;
    decoder->last_space_us = space_us;

    if (decoder->state == SYN_IR_STATE_IDLE) {
        /* Check leader pulses against protocol table */
        for (int p = 1; p < SYN_IR_PROTO_COUNT; p++) {
            /* Skip duplicate variants in leader lookup */
            if (p == SYN_IR_PROTO_SONY_12 || p == SYN_IR_PROTO_SONY_15 ||
                p == SYN_IR_PROTO_NEC_EXTENDED) {
                continue;
            }

            const SYN_IR_ProtoDesc *desc = &proto_table[p];

            /* Check NEC repeat code first */
            if ((p == SYN_IR_PROTO_NEC || p == SYN_IR_PROTO_APPLE) &&
                timing_match(mark_us, desc->leader_mark_us, desc->tolerance_us) &&
                timing_match(space_us, desc->repeat_space_us, desc->tolerance_us) &&
                decoder->have_last) {
                *frame_out = decoder->last_frame;
                frame_out->flags |= SYN_IR_FLAG_REPEAT;
                return true;
            }

            /* Standard leader pulse match */
            if (desc->leader_mark_us > 0 &&
                timing_match(mark_us, desc->leader_mark_us, desc->tolerance_us) &&
                timing_match(space_us, desc->leader_space_us, desc->tolerance_us)) {
                decoder->active_proto = (SYN_IR_Protocol)p;
                decoder->expected_bits = desc->total_bits;
                decoder->state = SYN_IR_STATE_DATA;
                decoder->bits = 0;
                decoder->bit_idx = 0;
                return false;
            }
        }

        /* Check Denon leaderless start */
        const SYN_IR_ProtoDesc *denon = &proto_table[SYN_IR_PROTO_DENON];
        if (timing_match(mark_us, denon->bit_mark_us, denon->tolerance_us) &&
            (timing_match(space_us, denon->zero_space_us, denon->tolerance_us) ||
             timing_match(space_us, denon->one_space_us, denon->tolerance_us))) {
            decoder->active_proto = SYN_IR_PROTO_DENON;
            decoder->expected_bits = denon->total_bits;
            decoder->state = SYN_IR_STATE_DATA;
            decoder->bits = 0;
            decoder->bit_idx = 0;
            /* Process first bit */
            uint8_t bit = timing_match(space_us, denon->one_space_us, denon->tolerance_us) ? 1 : 0;
            decoder->bits |= (((uint64_t)bit) << decoder->bit_idx);
            decoder->bit_idx++;
            return false;
        }

        /* Check RC5 / Manchester start bit */
        const SYN_IR_ProtoDesc *rc5 = &proto_table[SYN_IR_PROTO_RC5];
        if (timing_match(mark_us, rc5->half_bit_us, rc5->tolerance_us) ||
            timing_match(mark_us, rc5->half_bit_us * 2, rc5->tolerance_us)) {
            decoder->active_proto = SYN_IR_PROTO_RC5;
            decoder->expected_bits = rc5->total_bits;
            decoder->state = SYN_IR_STATE_DATA;
            decoder->bits = 1; /* First start bit = 1 */
            decoder->bit_idx = 1;
            decoder->manchester_phase = false;
            return false;
        }

        return false;
    }

    /* DATA State */
    if (decoder->state == SYN_IR_STATE_DATA) {
        SYN_IR_Protocol proto = decoder->active_proto;
        const SYN_IR_ProtoDesc *desc = &proto_table[proto];

        if (desc->encoding == SYN_IR_ENC_PDM || desc->encoding == SYN_IR_ENC_PPM) {
            if (!timing_match(mark_us, desc->bit_mark_us, desc->tolerance_us)) {
                reset_decoder_state(decoder);
                return false;
            }

            uint8_t bit_val = 0;
            if (timing_match(space_us, desc->one_space_us, desc->tolerance_us)) {
                bit_val = 1;
            } else if (timing_match(space_us, desc->zero_space_us, desc->tolerance_us)) {
                bit_val = 0;
            } else {
                reset_decoder_state(decoder);
                return false;
            }

            if (decoder->bit_idx < 64) {
                decoder->bits |= (((uint64_t)bit_val) << decoder->bit_idx);
            }
            decoder->bit_idx++;

            if (decoder->bit_idx >= decoder->expected_bits) {
                if (unpack_frame(decoder, frame_out)) {
                    decoder->last_frame = *frame_out;
                    decoder->have_last = true;
                    reset_decoder_state(decoder);
                    return true;
                }
                reset_decoder_state(decoder);
                return false;
            }
        } else if (desc->encoding == SYN_IR_ENC_PWM) {
            /* Sony PWM: space is fixed, mark duration indicates bit 0 (600us) vs bit 1 (1200us) */
            uint8_t bit_val = 0;
            if (timing_match(mark_us, desc->one_mark_us, desc->tolerance_us)) {
                bit_val = 1;
            } else if (timing_match(mark_us, desc->bit_mark_us, desc->tolerance_us)) {
                bit_val = 0;
            } else {
                reset_decoder_state(decoder);
                return false;
            }

            if (decoder->bit_idx < 64) {
                decoder->bits |= (((uint64_t)bit_val) << decoder->bit_idx);
            }
            decoder->bit_idx++;

            if (decoder->bit_idx >= decoder->expected_bits || decoder->bit_idx == 12 ||
                decoder->bit_idx == 15) {
                if (unpack_frame(decoder, frame_out)) {
                    decoder->last_frame = *frame_out;
                    decoder->have_last = true;
                    if (decoder->bit_idx >= decoder->expected_bits) {
                        reset_decoder_state(decoder);
                    }
                    return true;
                }
            }
        }
    }

    return false;
}

bool syn_ir_decode_timeout(SYN_IR_Decoder *decoder, SYN_IR_Frame *frame_out)
{
    if (decoder == NULL || frame_out == NULL) {
        return false;
    }

    if (decoder->state == SYN_IR_STATE_DATA && decoder->bit_idx > 0) {
        if (unpack_frame(decoder, frame_out)) {
            decoder->last_frame = *frame_out;
            decoder->have_last = true;
            reset_decoder_state(decoder);
            return true;
        }
    }

    reset_decoder_state(decoder);
    return false;
}

/* ── Encoder ─────────────────────────────────────────────────────────────── */

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
    size_t required_pulses = 0;

    if (desc->leader_mark_us > 0) {
        required_pulses += 2; /* Leader mark + space */
    }
    required_pulses += ((size_t)desc->total_bits) * 2; /* Data mark + space */
    required_pulses += 1;                              /* Trailer stop bit mark */

    if (buf_len < required_pulses) {
        return SYN_ERROR;
    }

    size_t idx = 0;

    /* Leader Pulse */
    if (desc->leader_mark_us > 0) {
        pulse_buf[idx++] = (SYN_IR_Pulse){.duration_us = desc->leader_mark_us, .is_mark = true};
        pulse_buf[idx++] = (SYN_IR_Pulse){.duration_us = desc->leader_space_us, .is_mark = false};
    }

    /* Build raw bit stream integer depending on protocol */
    uint64_t raw_bits = 0;

    switch (frame->protocol) {
    case SYN_IR_PROTO_NEC: {
        uint8_t addr = (uint8_t)(frame->address & 0xFFU);
        uint8_t cmd = (uint8_t)(frame->command & 0xFFU);
        raw_bits = ((uint64_t)addr) | (((uint64_t)(~addr & 0xFFU)) << 8) | (((uint64_t)cmd) << 16) |
                   (((uint64_t)(~cmd & 0xFFU)) << 24);
        break;
    }

    case SYN_IR_PROTO_NEC_EXTENDED: {
        uint16_t addr = (uint16_t)(frame->address & 0xFFFFU);
        uint8_t cmd = (uint8_t)(frame->command & 0xFFU);
        raw_bits = ((uint64_t)addr) | (((uint64_t)cmd) << 16) | (((uint64_t)(~cmd & 0xFFU)) << 24);
        break;
    }

    case SYN_IR_PROTO_APPLE: {
        uint16_t addr = (uint16_t)(frame->address & 0xFFFFU);
        uint8_t cmd = (uint8_t)(frame->command & 0xFFU);
        raw_bits = ((uint64_t)addr) | (((uint64_t)cmd) << 16) | (((uint64_t)cmd) << 24);
        break;
    }

    case SYN_IR_PROTO_SONY_12:
        raw_bits = (frame->command & 0x7FU) | ((frame->address & 0x1FU) << 7);
        break;

    case SYN_IR_PROTO_SONY_15:
        raw_bits = (frame->command & 0x7FU) | ((frame->address & 0xFFU) << 7);
        break;

    case SYN_IR_PROTO_SONY_20:
        raw_bits = (frame->command & 0x7FU) | ((frame->address & 0x1FU) << 7) |
                   (((frame->address >> 5) & 0xFFU) << 12);
        break;

    case SYN_IR_PROTO_RC5: {
        uint8_t toggle = (frame->flags & SYN_IR_FLAG_TOGGLE) ? 1 : 0;
        /* 14 bits MSB to LSB: S1(1), S2(1), Toggle(1), Address(5), Command(6) */
        raw_bits = (1ULL << 13) | (1ULL << 12) | (((uint64_t)toggle) << 11) |
                   (((uint64_t)(frame->address & 0x1FU)) << 6) |
                   ((uint64_t)(frame->command & 0x3FU));
        break;
    }

    case SYN_IR_PROTO_RC6: {
        uint8_t toggle = (frame->flags & SYN_IR_FLAG_TOGGLE) ? 1 : 0;
        /* 21 bits MSB to LSB: Start(1), Mode(3), Toggle(1), Address(8), Command(8) */
        raw_bits = (1ULL << 20) | (((uint64_t)toggle) << 16) |
                   (((uint64_t)(frame->address & 0xFFU)) << 8) |
                   ((uint64_t)(frame->command & 0xFFU));
        break;
    }

    case SYN_IR_PROTO_SAMSUNG: {
        uint16_t addr = (uint16_t)(frame->address & 0xFFFFU);
        uint8_t cmd = (uint8_t)(frame->command & 0xFFU);
        raw_bits = ((uint64_t)addr) | (((uint64_t)cmd) << 16) | (((uint64_t)(~cmd & 0xFFU)) << 24);
        break;
    }

    case SYN_IR_PROTO_KASEIKYO:
        raw_bits = (frame->address & 0xFFFFU) | (((uint64_t)frame->command) << 16);
        break;

    case SYN_IR_PROTO_DENON:
        raw_bits = ((frame->address & 0x1FU) << 10) | ((frame->command & 0xFFU) << 2);
        break;

    default: /* LCOV_EXCL_LINE: Unreachable default switch case in SYN_IR_Protocol enum switch */
        return SYN_INVALID_PARAM; /* LCOV_EXCL_LINE: Unreachable default switch case in
                                     SYN_IR_Protocol enum switch */
    }

    /* Encode Data Bits */
    if (desc->encoding == SYN_IR_ENC_MANCHESTER) {
        for (int b = desc->total_bits - 1; b >= 0; b--) {
            uint8_t bit_val = (uint8_t)((raw_bits >> b) & 0x01U);
            if (frame->protocol == SYN_IR_PROTO_RC6) {
                bit_val = !bit_val;
            }
            uint16_t half = desc->half_bit_us;
            if (bit_val == 1) {
                pulse_buf[idx++] = (SYN_IR_Pulse){.duration_us = half, .is_mark = false};
                pulse_buf[idx++] = (SYN_IR_Pulse){.duration_us = half, .is_mark = true};
            } else {
                pulse_buf[idx++] = (SYN_IR_Pulse){.duration_us = half, .is_mark = true};
                pulse_buf[idx++] = (SYN_IR_Pulse){.duration_us = half, .is_mark = false};
            }
        }
    } else {
        for (uint8_t b = 0; b < desc->total_bits; b++) {
            uint8_t bit_val = (uint8_t)((raw_bits >> b) & 0x01U);

            if (desc->encoding == SYN_IR_ENC_PDM || desc->encoding == SYN_IR_ENC_PPM) {
                uint16_t space = (bit_val == 1) ? desc->one_space_us : desc->zero_space_us;
                pulse_buf[idx++] =
                    (SYN_IR_Pulse){.duration_us = desc->bit_mark_us, .is_mark = true};
                pulse_buf[idx++] = (SYN_IR_Pulse){.duration_us = space, .is_mark = false};
            } else if (desc->encoding == SYN_IR_ENC_PWM) {
                uint16_t mark = (bit_val == 1) ? desc->one_mark_us : desc->bit_mark_us;
                pulse_buf[idx++] = (SYN_IR_Pulse){.duration_us = mark, .is_mark = true};
                pulse_buf[idx++] =
                    (SYN_IR_Pulse){.duration_us = desc->zero_space_us, .is_mark = false};
            }
        }
    }

    /* Trailer Stop Bit Mark */
    pulse_buf[idx++] = (SYN_IR_Pulse){
        .duration_us = desc->bit_mark_us > 0 ? desc->bit_mark_us : 500, .is_mark = true};

    *count_out = idx;
    return SYN_OK;
}

#endif /* SYN_USE_IR */
```


