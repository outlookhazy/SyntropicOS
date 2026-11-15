

# File syn\_modbus\_master.c

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_modbus\_master.c**](syn__modbus__master_8c.md)

[Go to the documentation of this file](syn__modbus__master_8c.md)


```C++


#include "syn_modbus_master.h"

#if SYN_MODBUS_MASTER_ENABLED

#include "../util/syn_assert.h"
#include "../util/syn_crc.h"
#include "../util/syn_pack.h"

#include <string.h>

static inline void write_u16_be(uint8_t *p, uint16_t val)
{
    size_t pos = 0;
    syn_pack_u16(p, &pos, val);
}

static inline uint16_t read_u16_be(const uint8_t *p)
{
    return syn_peek_u16(p, 0);
}

void syn_modbus_master_init(SYN_ModbusMaster *m, uint32_t timeout_ms)
{
    SYN_ASSERT(m != NULL);
    memset(m, 0, sizeof(*m));
    m->timeout_ms = (timeout_ms > 0) ? timeout_ms : 500;
    m->state = SYN_MB_MASTER_STATE_IDLE;
}

static SYN_Status send_request(SYN_ModbusMaster *m, uint8_t slave_addr, uint8_t fc, uint16_t addr,
                               uint16_t count, const uint16_t *write_vals)
{
    if (m->state == SYN_MB_MASTER_STATE_WAITING_RESPONSE) {
        return SYN_BUSY;
    }

    m->slave_addr = slave_addr;
    m->func_code = fc;
    m->start_addr = addr;
    m->count = count;
    m->rx_len = 0;
    m->read_count = 0;
    m->exception_code = 0;
    m->request_tick_ms = syn_port_get_tick_ms();

    m->buf[0] = slave_addr;
    m->buf[1] = fc;
    write_u16_be(&m->buf[2], addr);

    if (fc == SYN_MB_FC_WRITE_MULTIPLE) {
        write_u16_be(&m->buf[4], count);
        m->buf[6] = (uint8_t)(count * 2U);
        for (uint16_t i = 0; i < count; i++) {
            write_u16_be(&m->buf[7U + i * 2U], write_vals[i]);
        }
        uint16_t len = (uint16_t)(7U + count * 2U);
        syn_poke_u16_le(syn_crc16_modbus(m->buf, len), m->buf, len);
        m->tx_len = (uint16_t)(len + 2U);
    } else {
        write_u16_be(&m->buf[4], count);
        syn_poke_u16_le(syn_crc16_modbus(m->buf, 6), m->buf, 6);
        m->tx_len = 8U;
    }

    m->state = SYN_MB_MASTER_STATE_WAITING_RESPONSE;
    return SYN_OK;
}

SYN_Status syn_modbus_master_read_holding(SYN_ModbusMaster *m, uint8_t slave_addr,
                                          uint16_t start_addr, uint16_t count)
{
    if (m == NULL || slave_addr == 0 || count == 0 || count > 125)
        return SYN_INVALID_PARAM;
    return send_request(m, slave_addr, SYN_MB_FC_READ_HOLDING, start_addr, count, NULL);
}

SYN_Status syn_modbus_master_read_input(SYN_ModbusMaster *m, uint8_t slave_addr,
                                        uint16_t start_addr, uint16_t count)
{
    if (m == NULL || slave_addr == 0 || count == 0 || count > 125)
        return SYN_INVALID_PARAM;
    return send_request(m, slave_addr, SYN_MB_FC_READ_INPUT, start_addr, count, NULL);
}

SYN_Status syn_modbus_master_write_single(SYN_ModbusMaster *m, uint8_t slave_addr,
                                          uint16_t reg_addr, uint16_t value)
{
    if (m == NULL || slave_addr == 0)
        return SYN_INVALID_PARAM;
    return send_request(m, slave_addr, SYN_MB_FC_WRITE_SINGLE, reg_addr, value, NULL);
}

SYN_Status syn_modbus_master_write_multiple(SYN_ModbusMaster *m, uint8_t slave_addr,
                                            uint16_t start_addr, uint16_t count,
                                            const uint16_t *values)
{
    if (m == NULL || slave_addr == 0 || count == 0 || count > 123 || values == NULL)
        return SYN_INVALID_PARAM;
    return send_request(m, slave_addr, SYN_MB_FC_WRITE_MULTIPLE, start_addr, count, values);
}

SYN_Status syn_modbus_master_read_coils(SYN_ModbusMaster *m, uint8_t slave_addr,
                                        uint16_t start_addr, uint16_t count)
{
    if (m == NULL || slave_addr == 0 || count == 0 || count > 2000)
        return SYN_INVALID_PARAM;
    return send_request(m, slave_addr, SYN_MB_FC_READ_COILS, start_addr, count, NULL);
}

SYN_Status syn_modbus_master_read_discrete_inputs(SYN_ModbusMaster *m, uint8_t slave_addr,
                                                  uint16_t start_addr, uint16_t count)
{
    if (m == NULL || slave_addr == 0 || count == 0 || count > 2000)
        return SYN_INVALID_PARAM;
    return send_request(m, slave_addr, SYN_MB_FC_READ_DISCRETE_INPUTS, start_addr, count, NULL);
}

SYN_Status syn_modbus_master_write_single_coil(SYN_ModbusMaster *m, uint8_t slave_addr,
                                               uint16_t coil_addr, bool state)
{
    if (m == NULL || slave_addr == 0)
        return SYN_INVALID_PARAM;
    uint16_t val = state ? 0xFF00U : 0x0000U;
    return send_request(m, slave_addr, SYN_MB_FC_WRITE_SINGLE_COIL, coil_addr, val, NULL);
}

SYN_Status syn_modbus_master_write_multiple_coils(SYN_ModbusMaster *m, uint8_t slave_addr,
                                                  uint16_t start_addr, uint16_t count,
                                                  const uint8_t *coil_bytes)
{
    if (m == NULL || slave_addr == 0 || count == 0 || count > 1968 || coil_bytes == NULL)
        return SYN_INVALID_PARAM;

    if (m->state == SYN_MB_MASTER_STATE_WAITING_RESPONSE)
        return SYN_BUSY;

    m->slave_addr = slave_addr;
    m->func_code = SYN_MB_FC_WRITE_MULTIPLE_COILS;
    m->start_addr = start_addr;
    m->count = count;
    m->rx_len = 0;
    m->read_count = 0;
    m->exception_code = 0;
    m->request_tick_ms = syn_port_get_tick_ms();

    m->buf[0] = slave_addr;
    m->buf[1] = SYN_MB_FC_WRITE_MULTIPLE_COILS;
    write_u16_be(&m->buf[2], start_addr);
    write_u16_be(&m->buf[4], count);

    uint8_t num_bytes = (uint8_t)((count + 7U) / 8U);
    m->buf[6] = num_bytes;
    memcpy(&m->buf[7], coil_bytes, num_bytes);

    uint16_t len = (uint16_t)(7 + num_bytes);
    syn_poke_u16_le(syn_crc16_modbus(m->buf, len), m->buf, len);
    m->tx_len = (uint16_t)(len + 2);

    m->state = SYN_MB_MASTER_STATE_WAITING_RESPONSE;
    return SYN_OK;
}

SYN_Status syn_modbus_master_mask_write_register(SYN_ModbusMaster *m, uint8_t slave_addr,
                                                 uint16_t reg_addr, uint16_t and_mask,
                                                 uint16_t or_mask)
{
    if (m == NULL || slave_addr == 0) {
        return SYN_INVALID_PARAM;
    }

    if (m->state == SYN_MB_MASTER_STATE_WAITING_RESPONSE) {
        return SYN_BUSY;
    }

    m->slave_addr = slave_addr;
    m->func_code = SYN_MB_FC_MASK_WRITE_REGISTER;
    m->start_addr = reg_addr;
    m->count = 1;
    m->rx_len = 0;
    m->read_count = 0;
    m->exception_code = 0;
    m->request_tick_ms = syn_port_get_tick_ms();

    m->buf[0] = slave_addr;
    m->buf[1] = SYN_MB_FC_MASK_WRITE_REGISTER;
    write_u16_be(&m->buf[2], reg_addr);
    write_u16_be(&m->buf[4], and_mask);
    write_u16_be(&m->buf[6], or_mask);

    syn_poke_u16_le(syn_crc16_modbus(m->buf, 8), m->buf, 8);
    m->tx_len = 10;

    m->state = SYN_MB_MASTER_STATE_WAITING_RESPONSE;
    return SYN_OK;
}

SYN_Status syn_modbus_master_read_fifo_queue(SYN_ModbusMaster *m, uint8_t slave_addr,
                                             uint16_t fifo_addr)
{
    if (m == NULL || slave_addr == 0) {
        return SYN_INVALID_PARAM;
    }

    if (m->state == SYN_MB_MASTER_STATE_WAITING_RESPONSE) {
        return SYN_BUSY;
    }

    m->slave_addr = slave_addr;
    m->func_code = SYN_MB_FC_READ_FIFO_QUEUE;
    m->start_addr = fifo_addr;
    m->count = 0;
    m->rx_len = 0;
    m->read_count = 0;
    m->exception_code = 0;
    m->request_tick_ms = syn_port_get_tick_ms();

    m->buf[0] = slave_addr;
    m->buf[1] = SYN_MB_FC_READ_FIFO_QUEUE;
    write_u16_be(&m->buf[2], fifo_addr);

    syn_poke_u16_le(syn_crc16_modbus(m->buf, 4), m->buf, 4);
    m->tx_len = 6;

    m->state = SYN_MB_MASTER_STATE_WAITING_RESPONSE;
    return SYN_OK;
}

SYN_Status syn_modbus_master_report_server_id(SYN_ModbusMaster *m, uint8_t slave_addr)
{
    if (m == NULL || slave_addr == 0) {
        return SYN_INVALID_PARAM;
    }

    if (m->state == SYN_MB_MASTER_STATE_WAITING_RESPONSE) {
        return SYN_BUSY;
    }

    m->slave_addr = slave_addr;
    m->func_code = SYN_MB_FC_REPORT_SERVER_ID;
    m->start_addr = 0;
    m->count = 0;
    m->rx_len = 0;
    m->read_count = 0;
    m->exception_code = 0;
    m->request_tick_ms = syn_port_get_tick_ms();

    m->buf[0] = slave_addr;
    m->buf[1] = SYN_MB_FC_REPORT_SERVER_ID;

    syn_poke_u16_le(syn_crc16_modbus(m->buf, 2), m->buf, 2);
    m->tx_len = 4;

    m->state = SYN_MB_MASTER_STATE_WAITING_RESPONSE;
    return SYN_OK;
}

void syn_modbus_master_feed(SYN_ModbusMaster *m, uint8_t byte)
{
    SYN_ASSERT(m != NULL);
    if (m->state == SYN_MB_MASTER_STATE_WAITING_RESPONSE) {
        if (m->rx_len < sizeof(m->buf)) {
            m->buf[m->rx_len++] = byte;
        }
    }
}

static uint32_t mb_master_expected_rx_len(const SYN_ModbusMaster *m, uint8_t fc)
{
    switch (fc) {
    case SYN_MB_FC_READ_COILS:
    case SYN_MB_FC_READ_DISCRETE_INPUTS:
    case SYN_MB_FC_READ_HOLDING:
    case SYN_MB_FC_READ_INPUT:
    case SYN_MB_FC_REPORT_SERVER_ID:
    case SYN_MB_FC_READ_WRITE_MULTIPLE:
        return 5U + (uint32_t)m->buf[2];

    case SYN_MB_FC_WRITE_SINGLE_COIL:
    case SYN_MB_FC_WRITE_SINGLE:
    case SYN_MB_FC_WRITE_MULTIPLE_COILS:
    case SYN_MB_FC_WRITE_MULTIPLE:
        return 8U;

    case SYN_MB_FC_MASK_WRITE_REGISTER:
        return 10U;

    case SYN_MB_FC_READ_FIFO_QUEUE:
        if (m->rx_len < 8U) {
            return 8U;
        }
        return 8U + ((uint32_t)read_u16_be(&m->buf[4]) * 2U);

    default:
        return 5U;
    }
}

static void mb_master_parse_payload(SYN_ModbusMaster *m, uint8_t fc)
{
    if (fc == SYN_MB_FC_READ_HOLDING || fc == SYN_MB_FC_READ_INPUT) {
        uint8_t byte_count = m->buf[2];
        uint16_t words = (uint16_t)(byte_count / 2U);
        /* LCOV_EXCL_START: Defensive response words count clamp */
        if (words > 125U)
            words = 125U;
        /* LCOV_EXCL_STOP */
        for (uint16_t i = 0U; i < words; i++) {
            m->read_data[i] = read_u16_be(&m->buf[3U + i * 2U]);
        }
        m->read_count = words;
        m->state = SYN_MB_MASTER_STATE_COMPLETE;
    } else if (fc == SYN_MB_FC_READ_COILS || fc == SYN_MB_FC_READ_DISCRETE_INPUTS ||
               fc == SYN_MB_FC_REPORT_SERVER_ID) {
        uint8_t byte_count = m->buf[2];
        /* LCOV_EXCL_START: Defensive response byte_count clamp */
        if (byte_count > 250U)
            byte_count = 250U;
        /* LCOV_EXCL_STOP */
        for (uint16_t i = 0U; i < byte_count; i++) {
            m->read_data[i] = m->buf[3U + i];
        }
        m->read_count = byte_count;
        m->state = SYN_MB_MASTER_STATE_COMPLETE;
    } else if (fc == SYN_MB_FC_READ_FIFO_QUEUE) {
        uint16_t fifo_count = read_u16_be(&m->buf[4]);
        /* LCOV_EXCL_START: Defensive response fifo_count clamp */
        if (fifo_count > 31U)
            fifo_count = 31U;
        /* LCOV_EXCL_STOP */
        for (uint16_t i = 0U; i < fifo_count; i++) {
            m->read_data[i] = read_u16_be(&m->buf[6U + i * 2U]);
        }
        m->read_count = fifo_count;
        m->state = SYN_MB_MASTER_STATE_COMPLETE;
    } else if (fc == SYN_MB_FC_WRITE_SINGLE || fc == SYN_MB_FC_WRITE_MULTIPLE ||
               fc == SYN_MB_FC_WRITE_SINGLE_COIL || fc == SYN_MB_FC_WRITE_MULTIPLE_COILS ||
               fc == SYN_MB_FC_MASK_WRITE_REGISTER) {
        m->state = SYN_MB_MASTER_STATE_COMPLETE;
    }
}

SYN_ModbusMaster_State syn_modbus_master_process(SYN_ModbusMaster *m, uint32_t current_tick_ms)
{
    SYN_ASSERT(m != NULL);

    if (m->state != SYN_MB_MASTER_STATE_WAITING_RESPONSE) {
        return m->state;
    }

    if (m->request_tick_ms == 0U) {
        m->request_tick_ms = current_tick_ms;
    }

    /* Check response timeout */
    if ((current_tick_ms - m->request_tick_ms) >= m->timeout_ms) {
        m->state = SYN_MB_MASTER_STATE_TIMEOUT;
        return m->state;
    }

    /* Minimum frame length is 5 bytes (for exception responses) */
    if (m->rx_len < 5U) {
        return m->state;
    }

    /* Verify response CRC */
    uint16_t crc_calc = syn_crc16_modbus(m->buf, (uint16_t)(m->rx_len - 2U));
    uint16_t crc_rx = syn_peek_u16_le(m->buf, (size_t)(m->rx_len - 2U));
    if (crc_calc != crc_rx) {
        return m->state; /* Incomplete or corrupted frame — keep waiting or timeout */
    }

    if (m->buf[0] != m->slave_addr) {
        m->state = SYN_MB_MASTER_STATE_ERROR;
        return m->state;
    }

    uint8_t fc = m->buf[1];
    m->response_fc = fc;

    /* Exception response check */
    if (fc == (uint8_t)(m->func_code | 0x80U)) {
        m->exception_code = m->buf[2];
        m->state = SYN_MB_MASTER_STATE_ERROR;
        return m->state;
    }

    if (fc != m->func_code) {
        m->state = SYN_MB_MASTER_STATE_ERROR;
        return m->state;
    }

    uint32_t expected_len = mb_master_expected_rx_len(m, fc);
    if ((uint32_t)m->rx_len < expected_len) {
        return m->state;
    }

    mb_master_parse_payload(m, fc);
    return m->state;
}

void syn_modbus_master_queue_init(SYN_ModbusMasterQueue *q, uint8_t max_retries)
{
    if (q == NULL) {
        return;
    }
    (void)memset(q, 0, sizeof(*q));
    q->max_retries = max_retries;
}

SYN_Status syn_modbus_master_queue_push(SYN_ModbusMasterQueue *q,
                                        const SYN_ModbusMasterQuery *query)
{
    if (q == NULL || query == NULL) {
        return SYN_INVALID_PARAM;
    }

    if (q->count >= SYN_MODBUS_QUEUE_SIZE) {
        return SYN_ERROR; /* Queue full */
    }

    q->queries[q->tail] = *query;
    q->tail = (uint8_t)((q->tail + 1U) % SYN_MODBUS_QUEUE_SIZE);
    q->count++;

    return SYN_OK;
}

static SYN_Status mb_master_dispatch_queued(SYN_ModbusMaster *m, const SYN_ModbusMasterQuery *qry)
{
    switch (qry->func_code) {
    case SYN_MB_FC_READ_HOLDING:
        return syn_modbus_master_read_holding(m, qry->slave_addr, qry->start_addr, qry->count);
    case SYN_MB_FC_READ_INPUT:
        return syn_modbus_master_read_input(m, qry->slave_addr, qry->start_addr, qry->count);
    case SYN_MB_FC_WRITE_SINGLE:
        return syn_modbus_master_write_single(m, qry->slave_addr, qry->start_addr,
                                              qry->write_value);
    case SYN_MB_FC_READ_COILS:
        return syn_modbus_master_read_coils(m, qry->slave_addr, qry->start_addr, qry->count);
    case SYN_MB_FC_READ_DISCRETE_INPUTS:
        return syn_modbus_master_read_discrete_inputs(m, qry->slave_addr, qry->start_addr,
                                                      qry->count);
    default:
        return SYN_INVALID_PARAM;
    }
}

static void mb_master_pop_queue(SYN_ModbusMasterQueue *q)
{
    q->head = (uint8_t)((q->head + 1U) % SYN_MODBUS_QUEUE_SIZE);
    q->count--;
    q->retry_count = 0U;
}

SYN_Status syn_modbus_master_queue_step(SYN_ModbusMaster *m, SYN_ModbusMasterQueue *q,
                                        uint32_t now_ms)
{
    if (m == NULL || q == NULL) {
        return SYN_INVALID_PARAM;
    }

    SYN_ModbusMaster_State state = syn_modbus_master_process(m, now_ms);

    if (state == SYN_MB_MASTER_STATE_IDLE && q->count > 0U) {
        SYN_ModbusMasterQuery *qry = &q->queries[q->head];
        SYN_Status st = mb_master_dispatch_queued(m, qry);

        if (st == SYN_OK) {
            m->request_tick_ms = now_ms;
            (void)syn_modbus_master_process(m, now_ms); /* transition state to WAITING_RESPONSE */
        } else {
            if (qry->callback != NULL) {
                qry->callback(qry->slave_addr, qry->func_code, NULL, 0U, st, qry->user_ctx);
            }
            mb_master_pop_queue(q);
        }
    } else if (state == SYN_MB_MASTER_STATE_COMPLETE && q->count > 0U) {
        SYN_ModbusMasterQuery *qry = &q->queries[q->head];
        if (qry->callback != NULL) {
            qry->callback(qry->slave_addr, qry->func_code, m->read_data, m->read_count, SYN_OK,
                          qry->user_ctx);
        }
        mb_master_pop_queue(q);
        m->state = SYN_MB_MASTER_STATE_IDLE;
    } else if ((state == SYN_MB_MASTER_STATE_TIMEOUT || state == SYN_MB_MASTER_STATE_ERROR) &&
               q->count > 0U) {
        SYN_ModbusMasterQuery *qry = &q->queries[q->head];
        if (q->retry_count < q->max_retries) {
            q->retry_count++;
            m->state = SYN_MB_MASTER_STATE_IDLE; /* Retry query */
        } else {
            if (qry->callback != NULL) {
                qry->callback(qry->slave_addr, qry->func_code, NULL, 0U, SYN_ERROR, qry->user_ctx);
            }
            mb_master_pop_queue(q);
            m->state = SYN_MB_MASTER_STATE_IDLE;
        }
    }

    return SYN_OK;
}

#endif /* SYN_MODBUS_MASTER_ENABLED */
```


