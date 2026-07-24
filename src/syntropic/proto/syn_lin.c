/**
 * @file syn_lin.c
 * @brief LIN Protocol Engine implementation.
 * @ingroup syn_proto
 */

#include "syn_lin.h"

#if defined(SYN_USE_LIN) && SYN_USE_LIN

/* ── PID Parity Calculation ──────────────────────────────────────────────── */

uint8_t syn_lin_calc_pid(uint8_t id)
{
    uint8_t id_clean = id & SYN_LIN_ID_MAX;
    uint8_t id0 = (id_clean >> 0) & 0x01;
    uint8_t id1 = (id_clean >> 1) & 0x01;
    uint8_t id2 = (id_clean >> 2) & 0x01;
    uint8_t id3 = (id_clean >> 3) & 0x01;
    uint8_t id4 = (id_clean >> 4) & 0x01;
    uint8_t id5 = (id_clean >> 5) & 0x01;

    uint8_t p0 = id0 ^ id1 ^ id2 ^ id4;
    uint8_t p1 = !(id1 ^ id3 ^ id4 ^ id5);

    return (uint8_t)((p1 << 7) | (p0 << 6) | id_clean);
}

bool syn_lin_verify_pid(uint8_t pid)
{
    uint8_t expected_pid = syn_lin_calc_pid(pid & SYN_LIN_ID_MAX);
    return (pid == expected_pid);
}

/* ── LIN Checksum Calculation ────────────────────────────────────────────── */

uint8_t syn_lin_calc_checksum(uint8_t pid, const uint8_t *data, uint8_t len, SYN_LIN_ChecksumMode mode)
{
    if (data == NULL || len == 0 || len > SYN_LIN_DATA_MAX) {
        return 0;
    }

    uint8_t id = pid & SYN_LIN_ID_MAX;
    /* Diagnostic frames 0x3C and 0x3D always use Classic checksum per LIN 2.1/ISO 17987 */
    if (id == SYN_LIN_ID_MASTER_REQ || id == SYN_LIN_ID_SLAVE_RESP) {
        mode = SYN_LIN_CHECKSUM_CLASSIC;
    }

    uint16_t sum = 0;

    if (mode == SYN_LIN_CHECKSUM_ENHANCED) {
        sum += pid;
        if (sum > 0xFF) {
            sum -= 0xFF;
        }
    }

    for (uint8_t i = 0; i < len; i++) {
        sum += data[i];
        if (sum > 0xFF) {
            sum -= 0xFF;
        }
    }

    return (uint8_t)(~sum & 0xFF);
}

/* ── LIN Master Implementation ───────────────────────────────────────────── */

SYN_Status syn_lin_master_init(SYN_LIN_Master *master, const SYN_LIN_ScheduleSlot *slots, size_t count)
{
    if (master == NULL || slots == NULL || count == 0) {
        return SYN_INVALID_PARAM;
    }

    master->slots = slots;
    master->slot_count = count;
    master->current_slot = 0;
    master->slot_timer = 0;
    master->running = true;

    return SYN_OK;
}

bool syn_lin_master_step(SYN_LIN_Master *master, uint32_t dt_ms, const SYN_LIN_ScheduleSlot **active_slot)
{
    if (master == NULL || !master->running || master->slots == NULL || master->slot_count == 0) {
        if (active_slot != NULL) {
            *active_slot = NULL;
        }
        return false;
    }

    master->slot_timer += dt_ms;

    const SYN_LIN_ScheduleSlot *current = &master->slots[master->current_slot];

    if (master->slot_timer >= current->delay_ms) {
        master->slot_timer = 0;
        master->current_slot = (master->current_slot + 1) % master->slot_count;
        if (active_slot != NULL) {
            *active_slot = &master->slots[master->current_slot];
        }
        return true;
    }

    if (active_slot != NULL) {
        *active_slot = current;
    }
    return false;
}

/* ── LIN Slave Implementation ────────────────────────────────────────────── */

SYN_Status syn_lin_slave_init(SYN_LIN_Slave *slave, uint8_t nad)
{
    if (slave == NULL) {
        return SYN_INVALID_PARAM;
    }

    slave->nad = nad;
    slave->frame_count = 0;
    slave->state = SYN_LIN_STATE_IDLE;
    slave->rx_idx = 0;
    slave->expected_len = 0;
    slave->expected_checksum_mode = SYN_LIN_CHECKSUM_CLASSIC;

    for (size_t i = 0; i < SYN_LIN_SLAVE_MAX_FRAMES; i++) {
        slave->frames[i].active = false;
    }

    return SYN_OK;
}

SYN_Status syn_lin_slave_add_frame(SYN_LIN_Slave *slave, uint8_t id, uint8_t len, SYN_LIN_SlotDirection dir, SYN_LIN_ChecksumMode mode)
{
    if (slave == NULL || id > SYN_LIN_ID_MAX || len == 0 || len > SYN_LIN_DATA_MAX) {
        return SYN_INVALID_PARAM;
    }

    if (slave->frame_count >= SYN_LIN_SLAVE_MAX_FRAMES) {
        return SYN_ERROR;
    }

    /* Check if frame ID is already registered */
    for (size_t i = 0; i < slave->frame_count; i++) {
        if (slave->frames[i].active && slave->frames[i].id == id) {
            slave->frames[i].len = len;
            slave->frames[i].dir = dir;
            slave->frames[i].checksum_mode = mode;
            return SYN_OK;
        }
    }

    SYN_LIN_SlaveFrame *f = &slave->frames[slave->frame_count++];
    f->id = id;
    f->len = len;
    f->dir = dir;
    f->checksum_mode = mode;
    f->active = true;

    for (uint8_t j = 0; j < SYN_LIN_DATA_MAX; j++) {
        f->data[j] = 0x00;
    }

    return SYN_OK;
}

SYN_Status syn_lin_slave_set_publish_data(SYN_LIN_Slave *slave, uint8_t id, const uint8_t *data, uint8_t len)
{
    if (slave == NULL || data == NULL || len == 0 || len > SYN_LIN_DATA_MAX || id > SYN_LIN_ID_MAX) {
        return SYN_INVALID_PARAM;
    }

    for (size_t i = 0; i < slave->frame_count; i++) {
        if (slave->frames[i].active && slave->frames[i].id == id && slave->frames[i].dir == SYN_LIN_SLOT_PUBLISH) {
            if (len > slave->frames[i].len) {
                len = slave->frames[i].len;
            }
            for (uint8_t j = 0; j < len; j++) {
                slave->frames[i].data[j] = data[j];
            }
            return SYN_OK;
        }
    }

    return SYN_ERROR;
}

bool syn_lin_slave_process_byte(SYN_LIN_Slave *slave, uint8_t byte, SYN_LIN_Frame *out_frame)
{
    if (slave == NULL) {
        return false;
    }

    switch (slave->state) {
        case SYN_LIN_STATE_IDLE:
            if (byte == SYN_LIN_SYNC_BYTE) {
                slave->state = SYN_LIN_STATE_PID;
            }
            break;

        case SYN_LIN_STATE_SYNC:
            if (byte == SYN_LIN_SYNC_BYTE) {
                slave->state = SYN_LIN_STATE_PID;
            } else {
                slave->state = SYN_LIN_STATE_IDLE;
            }
            break;

        case SYN_LIN_STATE_PID: {
            slave->rx_frame.pid = byte;
            slave->rx_frame.id = byte & SYN_LIN_ID_MAX;
            slave->rx_frame.valid_pid = syn_lin_verify_pid(byte);
            slave->rx_idx = 0;

            if (!slave->rx_frame.valid_pid) {
                slave->state = SYN_LIN_STATE_IDLE;
                break;
            }

            /* Look up expected frame in configuration table */
            bool found = false;
            for (size_t i = 0; i < slave->frame_count; i++) {
                if (slave->frames[i].active && slave->frames[i].id == slave->rx_frame.id) {
                    slave->expected_len = slave->frames[i].len;
                    slave->expected_checksum_mode = slave->frames[i].checksum_mode;
                    found = true;
                    break;
                }
            }

            if (!found) {
                /* Unknown frame ID; default to 8 bytes classic checksum */
                slave->expected_len = 8;
                slave->expected_checksum_mode = SYN_LIN_CHECKSUM_CLASSIC;
            }

            slave->state = SYN_LIN_STATE_DATA;
            break;
        }

        case SYN_LIN_STATE_DATA:
            slave->rx_frame.data[slave->rx_idx++] = byte;
            if (slave->rx_idx >= slave->expected_len) {
                slave->rx_frame.len = slave->rx_idx;
                slave->state = SYN_LIN_STATE_CHECKSUM;
            }
            break;

        case SYN_LIN_STATE_CHECKSUM: {
            slave->rx_frame.checksum = byte;
            uint8_t calc_cs = syn_lin_calc_checksum(slave->rx_frame.pid,
                                                    slave->rx_frame.data,
                                                    slave->rx_frame.len,
                                                    slave->expected_checksum_mode);
            slave->rx_frame.valid_checksum = (byte == calc_cs);
            slave->state = SYN_LIN_STATE_IDLE;

            if (out_frame != NULL) {
                *out_frame = slave->rx_frame;
            }
            return slave->rx_frame.valid_pid && slave->rx_frame.valid_checksum;
        }

        default:
            slave->state = SYN_LIN_STATE_IDLE;
            break;
    }

    return false;
}

#endif /* SYN_USE_LIN */
