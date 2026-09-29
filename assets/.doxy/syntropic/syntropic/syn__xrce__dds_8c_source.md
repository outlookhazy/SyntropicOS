

# File syn\_xrce\_dds.c

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_xrce\_dds.c**](syn__xrce__dds_8c.md)

[Go to the documentation of this file](syn__xrce__dds_8c.md)


```C++

#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_XRCE_DDS) || SYN_USE_XRCE_DDS

#include "../port/syn_port_system.h"
#include "syn_xrce_dds.h"

#include <string.h>

#define SYN_XRCE_DEFAULT_HEARTBEAT_MS 5000U
#define SYN_XRCE_FLAG_LITTLE_ENDIAN 0x01U
#define SYN_XRCE_REPRESENTATION_BY_REF 0x01U
#define SYN_XRCE_REPRESENTATION_AS_STRING 0x02U
#define SYN_XRCE_DATA_FORMAT_DATA 0x00U

/* ── Internal Helper: CDR Alignment ──────────────────────────────────────── */

static size_t cdr_pad_writer(SYN_CDR_Writer *w, size_t align)
{
    size_t remainder = w->pos % align;
    if (remainder != 0U) {
        size_t pad = align - remainder;
        if (w->pos + pad > w->size) {
            w->error = true;
            return w->pos;
        }
        (void)memset(&w->data[w->pos], 0, pad);
        w->pos += pad;
    }
    return w->pos;
}

static size_t cdr_pad_reader(SYN_CDR_Reader *r, size_t align)
{
    size_t remainder = r->pos % align;
    if (remainder != 0U) {
        size_t pad = align - remainder;
        if (r->pos + pad > r->size) {
            r->error = true;
            return r->pos;
        }
        r->pos += pad;
    }
    return r->pos;
}

/* ── CDR Writer Implementation ───────────────────────────────────────────── */

void syn_cdr_writer_init(SYN_CDR_Writer *w, uint8_t *buf, size_t size)
{
    if (w == NULL) {
        return;
    }
    w->data = buf;
    w->size = size;
    w->pos = 0U;
    w->error = (buf == NULL || size == 0U);
}

bool syn_cdr_write_u8(SYN_CDR_Writer *w, uint8_t v)
{
    if (w == NULL || w->error || w->pos + 1U > w->size) {
        if (w != NULL) {
            w->error = true;
        }
        return false;
    }
    w->data[w->pos++] = v;
    return true;
}

bool syn_cdr_write_u16(SYN_CDR_Writer *w, uint16_t v)
{
    if (w == NULL || w->error) {
        return false;
    }
    (void)cdr_pad_writer(w, 2U);
    if (w->error || w->pos + 2U > w->size) {
        w->error = true;
        return false;
    }
    w->data[w->pos++] = (uint8_t)(v & 0xFFU);
    w->data[w->pos++] = (uint8_t)((v >> 8U) & 0xFFU);
    return true;
}

bool syn_cdr_write_u32(SYN_CDR_Writer *w, uint32_t v)
{
    if (w == NULL || w->error) {
        return false;
    }
    (void)cdr_pad_writer(w, 4U);
    if (w->error || w->pos + 4U > w->size) {
        w->error = true;
        return false;
    }
    w->data[w->pos++] = (uint8_t)(v & 0xFFU);
    w->data[w->pos++] = (uint8_t)((v >> 8U) & 0xFFU);
    w->data[w->pos++] = (uint8_t)((v >> 16U) & 0xFFU);
    w->data[w->pos++] = (uint8_t)((v >> 24U) & 0xFFU);
    return true;
}

bool syn_cdr_write_u64(SYN_CDR_Writer *w, uint64_t v)
{
    if (w == NULL || w->error) {
        return false;
    }
    (void)cdr_pad_writer(w, 8U);
    if (w->error || w->pos + 8U > w->size) {
        w->error = true;
        return false;
    }
    for (size_t i = 0U; i < 8U; i++) {
        w->data[w->pos++] = (uint8_t)((v >> (i * 8U)) & 0xFFU);
    }
    return true;
}

bool syn_cdr_write_i8(SYN_CDR_Writer *w, int8_t v)
{
    return syn_cdr_write_u8(w, (uint8_t)v);
}

bool syn_cdr_write_i16(SYN_CDR_Writer *w, int16_t v)
{
    return syn_cdr_write_u16(w, (uint16_t)v);
}

bool syn_cdr_write_i32(SYN_CDR_Writer *w, int32_t v)
{
    return syn_cdr_write_u32(w, (uint32_t)v);
}

bool syn_cdr_write_i64(SYN_CDR_Writer *w, int64_t v)
{
    return syn_cdr_write_u64(w, (uint64_t)v);
}

bool syn_cdr_write_float(SYN_CDR_Writer *w, float v)
{
    uint32_t u;
    (void)memcpy(&u, &v, sizeof(u));
    return syn_cdr_write_u32(w, u);
}

bool syn_cdr_write_double(SYN_CDR_Writer *w, double v)
{
    uint64_t u;
    (void)memcpy(&u, &v, sizeof(u));
    return syn_cdr_write_u64(w, u);
}

bool syn_cdr_write_bool(SYN_CDR_Writer *w, bool v)
{
    return syn_cdr_write_u8(w, v ? 1U : 0U);
}

bool syn_cdr_write_string(SYN_CDR_Writer *w, const char *str)
{
    if (w == NULL || str == NULL) {
        if (w != NULL) {
            w->error = true;
        }
        return false;
    }
    size_t str_len = strlen(str) + 1U; /* Length including null terminator */
    if (!syn_cdr_write_u32(w, (uint32_t)str_len)) {
        return false;
    }
    if (w->pos + str_len > w->size) {
        w->error = true;
        return false;
    }
    (void)memcpy(&w->data[w->pos], str, str_len);
    w->pos += str_len;
    return true;
}

bool syn_cdr_write_bytes(SYN_CDR_Writer *w, const uint8_t *bytes, size_t len)
{
    if (w == NULL || w->error || (bytes == NULL && len > 0U)) {
        if (w != NULL) {
            w->error = true;
        }
        return false;
    }
    if (w->pos + len > w->size) {
        w->error = true;
        return false;
    }
    if (len > 0U) {
        (void)memcpy(&w->data[w->pos], bytes, len);
        w->pos += len;
    }
    return true;
}

/* ── CDR Reader Implementation ───────────────────────────────────────────── */

void syn_cdr_reader_init(SYN_CDR_Reader *r, const uint8_t *buf, size_t size)
{
    if (r == NULL) {
        return;
    }
    r->data = buf;
    r->size = size;
    r->pos = 0U;
    r->error = (buf == NULL || size == 0U);
}

bool syn_cdr_read_u8(SYN_CDR_Reader *r, uint8_t *out)
{
    if (r == NULL || r->error || out == NULL || r->pos + 1U > r->size) {
        if (r != NULL) {
            r->error = true;
        }
        return false;
    }
    *out = r->data[r->pos++];
    return true;
}

bool syn_cdr_read_u16(SYN_CDR_Reader *r, uint16_t *out)
{
    if (r == NULL || r->error || out == NULL) {
        return false;
    }
    (void)cdr_pad_reader(r, 2U);
    if (r->error || r->pos + 2U > r->size) {
        r->error = true;
        return false;
    }
    *out = (uint16_t)r->data[r->pos] | ((uint16_t)r->data[r->pos + 1U] << 8U);
    r->pos += 2U;
    return true;
}

bool syn_cdr_read_u32(SYN_CDR_Reader *r, uint32_t *out)
{
    if (r == NULL || r->error || out == NULL) {
        return false;
    }
    (void)cdr_pad_reader(r, 4U);
    if (r->error || r->pos + 4U > r->size) {
        r->error = true;
        return false;
    }
    *out = (uint32_t)r->data[r->pos] | ((uint32_t)r->data[r->pos + 1U] << 8U) |
           ((uint32_t)r->data[r->pos + 2U] << 16U) | ((uint32_t)r->data[r->pos + 3U] << 24U);
    r->pos += 4U;
    return true;
}

bool syn_cdr_read_u64(SYN_CDR_Reader *r, uint64_t *out)
{
    if (r == NULL || r->error || out == NULL) {
        return false;
    }
    (void)cdr_pad_reader(r, 8U);
    if (r->error || r->pos + 8U > r->size) {
        r->error = true;
        return false;
    }
    uint64_t v = 0U;
    for (size_t i = 0U; i < 8U; i++) {
        v |= ((uint64_t)r->data[r->pos + i] << (i * 8U));
    }
    *out = v;
    r->pos += 8U;
    return true;
}

bool syn_cdr_read_i8(SYN_CDR_Reader *r, int8_t *out)
{
    return syn_cdr_read_u8(r, (uint8_t *)out);
}

bool syn_cdr_read_i16(SYN_CDR_Reader *r, int16_t *out)
{
    return syn_cdr_read_u16(r, (uint16_t *)out);
}

bool syn_cdr_read_i32(SYN_CDR_Reader *r, int32_t *out)
{
    return syn_cdr_read_u32(r, (uint32_t *)out);
}

bool syn_cdr_read_i64(SYN_CDR_Reader *r, int64_t *out)
{
    return syn_cdr_read_u64(r, (uint64_t *)out);
}

bool syn_cdr_read_float(SYN_CDR_Reader *r, float *out)
{
    uint32_t u;
    if (!syn_cdr_read_u32(r, &u) || out == NULL) {
        return false;
    }
    (void)memcpy(out, &u, sizeof(u));
    return true;
}

bool syn_cdr_read_double(SYN_CDR_Reader *r, double *out)
{
    uint64_t u;
    if (!syn_cdr_read_u64(r, &u) || out == NULL) {
        return false;
    }
    (void)memcpy(out, &u, sizeof(u));
    return true;
}

bool syn_cdr_read_bool(SYN_CDR_Reader *r, bool *out)
{
    uint8_t u;
    if (!syn_cdr_read_u8(r, &u) || out == NULL) {
        return false;
    }
    *out = (u != 0U);
    return true;
}

bool syn_cdr_read_string(SYN_CDR_Reader *r, char *out_buf, size_t max_len)
{
    if (r == NULL || r->error || out_buf == NULL || max_len == 0U) {
        if (r != NULL) {
            r->error = true;
        }
        return false;
    }
    uint32_t str_len = 0U;
    if (!syn_cdr_read_u32(r, &str_len)) {
        return false;
    }
    if (str_len == 0U || r->pos + str_len > r->size) {
        r->error = true;
        return false;
    }
    size_t copy_len = (str_len < max_len) ? str_len : max_len - 1U;
    (void)memcpy(out_buf, &r->data[r->pos], copy_len);
    out_buf[copy_len] = '\0';
    r->pos += str_len;
    return true;
}

bool syn_cdr_read_bytes(SYN_CDR_Reader *r, uint8_t *out_buf, size_t len)
{
    if (r == NULL || r->error || (out_buf == NULL && len > 0U)) {
        if (r != NULL) {
            r->error = true;
        }
        return false;
    }
    if (r->pos + len > r->size) {
        r->error = true;
        return false;
    }
    if (len > 0U) {
        (void)memcpy(out_buf, &r->data[r->pos], len);
        r->pos += len;
    }
    return true;
}

/* ── Internal Framing Helpers ────────────────────────────────────────────── */

static void xrce_write_header(SYN_CDR_Writer *w, uint8_t session_id, uint8_t stream_id,
                              uint16_t seq_num, uint32_t client_key)
{
    (void)syn_cdr_write_u8(w, session_id);
    (void)syn_cdr_write_u8(w, stream_id);
    (void)syn_cdr_write_u16(w, seq_num);
    if (session_id == 0x00U || session_id >= 0x80U) {
        (void)syn_cdr_write_u32(w, client_key);
    }
}

static size_t xrce_start_submessage(SYN_CDR_Writer *w, uint8_t submsg_id, uint8_t flags)
{
    (void)syn_cdr_write_u8(w, submsg_id);
    (void)syn_cdr_write_u8(w, flags | SYN_XRCE_FLAG_LITTLE_ENDIAN);
    size_t len_pos = w->pos;
    (void)syn_cdr_write_u16(w, 0U); /* Placeholder for submessage payload length */
    return len_pos;
}

static void xrce_finish_submessage(SYN_CDR_Writer *w, size_t len_pos)
{
    if (w->error || len_pos + 2U > w->pos) {
        return;
    }
    uint16_t submsg_len = (uint16_t)(w->pos - (len_pos + 2U));
    w->data[len_pos] = (uint8_t)(submsg_len & 0xFFU);
    w->data[len_pos + 1U] = (uint8_t)((submsg_len >> 8U) & 0xFFU);
}

/* ── Public XRCE Client API ──────────────────────────────────────────────── */

SYN_Status syn_xrce_client_init(SYN_XRCE_Client *client, const SYN_XRCE_Config *cfg)
{
    if (client == NULL || cfg == NULL) {
        return SYN_INVALID_PARAM;
    }
    if (cfg->transport == NULL || cfg->rx_buf == NULL || cfg->tx_buf == NULL) {
        return SYN_INVALID_PARAM;
    }
    if (cfg->rx_buf_size < 128U || cfg->tx_buf_size < 128U) {
        return SYN_INVALID_PARAM;
    }

    (void)memset(client, 0, sizeof(*client));
    client->cfg = *cfg;
    if (client->cfg.client_key == 0U) {
        client->cfg.client_key = SYN_XRCE_CLIENT_KEY_DEFAULT;
    }
    if (client->cfg.session_id == 0U) {
        client->cfg.session_id = SYN_XRCE_SESSION_ID_DEFAULT;
    }
    if (client->cfg.heartbeat_period_ms == 0U) {
        client->cfg.heartbeat_period_ms = SYN_XRCE_DEFAULT_HEARTBEAT_MS;
    }

    client->state = SYN_XRCE_STATE_DISCONNECTED;
    client->next_req_id = 1U;
    client->stream_seq_num = 1U;
    return SYN_OK;
}

SYN_Status syn_xrce_client_create_session(SYN_XRCE_Client *client)
{
    if (client == NULL || client->cfg.transport == NULL) {
        return SYN_INVALID_PARAM;
    }

    SYN_CDR_Writer w;
    syn_cdr_writer_init(&w, client->cfg.tx_buf, client->cfg.tx_buf_size);

    /* Sessionless Header */
    xrce_write_header(&w, 0x00U, 0x00U, 0U, client->cfg.client_key);

    /* CREATE_CLIENT Submessage */
    size_t len_pos = xrce_start_submessage(&w, SYN_XRCE_SUBMSG_CREATE_CLIENT, 0U);
    (void)syn_cdr_write_u32(&w, client->cfg.client_key);
    (void)syn_cdr_write_u8(&w, client->cfg.session_id);
    (void)syn_cdr_write_u16(&w, (uint16_t)client->cfg.tx_buf_size);
    xrce_finish_submessage(&w, len_pos);

    if (w.error || !syn_transport_send(client->cfg.transport, w.data, w.pos)) {
        client->state = SYN_XRCE_STATE_ERROR;
        return SYN_ERROR;
    }

    client->state = SYN_XRCE_STATE_CONNECTING;
    return SYN_OK;
}

static SYN_Status xrce_register_local_object(SYN_XRCE_Client *client, uint16_t id, uint8_t kind,
                                             uint16_t parent)
{
    if (client->object_count >= SYN_XRCE_MAX_OBJECTS) {
        return SYN_ERROR;
    }
    SYN_XRCE_Object *obj = &client->objects[client->object_count++];
    obj->id = id;
    obj->kind = kind;
    obj->parent = parent;
    obj->configured = true;
    return SYN_OK;
}

SYN_Status syn_xrce_client_create_participant(SYN_XRCE_Client *client, uint16_t participant_id)
{
    if (client == NULL || client->cfg.transport == NULL) {
        return SYN_INVALID_PARAM;
    }

    SYN_CDR_Writer w;
    syn_cdr_writer_init(&w, client->cfg.tx_buf, client->cfg.tx_buf_size);

    xrce_write_header(&w, client->cfg.session_id, SYN_XRCE_STREAM_BEST_EFFORT,
                      client->stream_seq_num++, client->cfg.client_key);

    size_t len_pos = xrce_start_submessage(&w, SYN_XRCE_SUBMSG_CREATE, 0U);
    uint16_t req_id = client->next_req_id++;
    (void)syn_cdr_write_u16(&w, req_id);
    (void)syn_cdr_write_u16(&w, participant_id);
    (void)syn_cdr_write_u8(&w, SYN_XRCE_OBJKIND_PARTICIPANT);
    (void)syn_cdr_write_u8(&w, SYN_XRCE_REPRESENTATION_BY_REF);
    (void)syn_cdr_write_string(&w, "default_participant");
    xrce_finish_submessage(&w, len_pos);

    if (w.error || !syn_transport_send(client->cfg.transport, w.data, w.pos)) {
        return SYN_ERROR;
    }

    return xrce_register_local_object(client, participant_id, SYN_XRCE_OBJKIND_PARTICIPANT, 0U);
}

SYN_Status syn_xrce_client_create_topic(SYN_XRCE_Client *client, uint16_t topic_id,
                                        uint16_t participant_id, const char *topic_name,
                                        const char *type_name)
{
    if (client == NULL || topic_name == NULL || type_name == NULL) {
        return SYN_INVALID_PARAM;
    }

    SYN_CDR_Writer w;
    syn_cdr_writer_init(&w, client->cfg.tx_buf, client->cfg.tx_buf_size);

    xrce_write_header(&w, client->cfg.session_id, SYN_XRCE_STREAM_BEST_EFFORT,
                      client->stream_seq_num++, client->cfg.client_key);

    size_t len_pos = xrce_start_submessage(&w, SYN_XRCE_SUBMSG_CREATE, 0U);
    uint16_t req_id = client->next_req_id++;
    (void)syn_cdr_write_u16(&w, req_id);
    (void)syn_cdr_write_u16(&w, topic_id);
    (void)syn_cdr_write_u8(&w, SYN_XRCE_OBJKIND_TOPIC);
    (void)syn_cdr_write_u8(&w, SYN_XRCE_REPRESENTATION_AS_STRING);
    (void)syn_cdr_write_u16(&w, participant_id);
    (void)syn_cdr_write_string(&w, topic_name);
    (void)syn_cdr_write_string(&w, type_name);
    xrce_finish_submessage(&w, len_pos);

    if (w.error || !syn_transport_send(client->cfg.transport, w.data, w.pos)) {
        return SYN_ERROR;
    }

    return xrce_register_local_object(client, topic_id, SYN_XRCE_OBJKIND_TOPIC, participant_id);
}

SYN_Status syn_xrce_client_create_publisher(SYN_XRCE_Client *client, uint16_t publisher_id,
                                            uint16_t participant_id)
{
    if (client == NULL) {
        return SYN_INVALID_PARAM;
    }

    SYN_CDR_Writer w;
    syn_cdr_writer_init(&w, client->cfg.tx_buf, client->cfg.tx_buf_size);

    xrce_write_header(&w, client->cfg.session_id, SYN_XRCE_STREAM_BEST_EFFORT,
                      client->stream_seq_num++, client->cfg.client_key);

    size_t len_pos = xrce_start_submessage(&w, SYN_XRCE_SUBMSG_CREATE, 0U);
    uint16_t req_id = client->next_req_id++;
    (void)syn_cdr_write_u16(&w, req_id);
    (void)syn_cdr_write_u16(&w, publisher_id);
    (void)syn_cdr_write_u8(&w, SYN_XRCE_OBJKIND_PUBLISHER);
    (void)syn_cdr_write_u8(&w, SYN_XRCE_REPRESENTATION_BY_REF);
    (void)syn_cdr_write_u16(&w, participant_id);
    xrce_finish_submessage(&w, len_pos);

    if (w.error || !syn_transport_send(client->cfg.transport, w.data, w.pos)) {
        return SYN_ERROR;
    }

    return xrce_register_local_object(client, publisher_id, SYN_XRCE_OBJKIND_PUBLISHER,
                                      participant_id);
}

SYN_Status syn_xrce_client_create_subscriber(SYN_XRCE_Client *client, uint16_t subscriber_id,
                                             uint16_t participant_id)
{
    if (client == NULL) {
        return SYN_INVALID_PARAM;
    }

    SYN_CDR_Writer w;
    syn_cdr_writer_init(&w, client->cfg.tx_buf, client->cfg.tx_buf_size);

    xrce_write_header(&w, client->cfg.session_id, SYN_XRCE_STREAM_BEST_EFFORT,
                      client->stream_seq_num++, client->cfg.client_key);

    size_t len_pos = xrce_start_submessage(&w, SYN_XRCE_SUBMSG_CREATE, 0U);
    uint16_t req_id = client->next_req_id++;
    (void)syn_cdr_write_u16(&w, req_id);
    (void)syn_cdr_write_u16(&w, subscriber_id);
    (void)syn_cdr_write_u8(&w, SYN_XRCE_OBJKIND_SUBSCRIBER);
    (void)syn_cdr_write_u8(&w, SYN_XRCE_REPRESENTATION_BY_REF);
    (void)syn_cdr_write_u16(&w, participant_id);
    xrce_finish_submessage(&w, len_pos);

    if (w.error || !syn_transport_send(client->cfg.transport, w.data, w.pos)) {
        return SYN_ERROR;
    }

    return xrce_register_local_object(client, subscriber_id, SYN_XRCE_OBJKIND_SUBSCRIBER,
                                      participant_id);
}

SYN_Status syn_xrce_client_create_datawriter(SYN_XRCE_Client *client, uint16_t writer_id,
                                             uint16_t publisher_id, uint16_t topic_id)
{
    if (client == NULL) {
        return SYN_INVALID_PARAM;
    }

    SYN_CDR_Writer w;
    syn_cdr_writer_init(&w, client->cfg.tx_buf, client->cfg.tx_buf_size);

    xrce_write_header(&w, client->cfg.session_id, SYN_XRCE_STREAM_BEST_EFFORT,
                      client->stream_seq_num++, client->cfg.client_key);

    size_t len_pos = xrce_start_submessage(&w, SYN_XRCE_SUBMSG_CREATE, 0U);
    uint16_t req_id = client->next_req_id++;
    (void)syn_cdr_write_u16(&w, req_id);
    (void)syn_cdr_write_u16(&w, writer_id);
    (void)syn_cdr_write_u8(&w, SYN_XRCE_OBJKIND_DATAWRITER);
    (void)syn_cdr_write_u8(&w, SYN_XRCE_REPRESENTATION_BY_REF);
    (void)syn_cdr_write_u16(&w, publisher_id);
    (void)syn_cdr_write_u16(&w, topic_id);
    xrce_finish_submessage(&w, len_pos);

    if (w.error || !syn_transport_send(client->cfg.transport, w.data, w.pos)) {
        return SYN_ERROR;
    }

    return xrce_register_local_object(client, writer_id, SYN_XRCE_OBJKIND_DATAWRITER, publisher_id);
}

SYN_Status syn_xrce_client_create_datareader(SYN_XRCE_Client *client, uint16_t reader_id,
                                             uint16_t subscriber_id, uint16_t topic_id)
{
    if (client == NULL) {
        return SYN_INVALID_PARAM;
    }

    SYN_CDR_Writer w;
    syn_cdr_writer_init(&w, client->cfg.tx_buf, client->cfg.tx_buf_size);

    xrce_write_header(&w, client->cfg.session_id, SYN_XRCE_STREAM_BEST_EFFORT,
                      client->stream_seq_num++, client->cfg.client_key);

    size_t len_pos = xrce_start_submessage(&w, SYN_XRCE_SUBMSG_CREATE, 0U);
    uint16_t req_id = client->next_req_id++;
    (void)syn_cdr_write_u16(&w, req_id);
    (void)syn_cdr_write_u16(&w, reader_id);
    (void)syn_cdr_write_u8(&w, SYN_XRCE_OBJKIND_DATAREADER);
    (void)syn_cdr_write_u8(&w, SYN_XRCE_REPRESENTATION_BY_REF);
    (void)syn_cdr_write_u16(&w, subscriber_id);
    (void)syn_cdr_write_u16(&w, topic_id);
    xrce_finish_submessage(&w, len_pos);

    if (w.error || !syn_transport_send(client->cfg.transport, w.data, w.pos)) {
        return SYN_ERROR;
    }

    return xrce_register_local_object(client, reader_id, SYN_XRCE_OBJKIND_DATAREADER,
                                      subscriber_id);
}

SYN_Status syn_xrce_client_write_data(SYN_XRCE_Client *client, uint16_t writer_id,
                                      const uint8_t *data, size_t len)
{
    if (client == NULL || (data == NULL && len > 0U)) {
        return SYN_INVALID_PARAM;
    }

    SYN_CDR_Writer w;
    syn_cdr_writer_init(&w, client->cfg.tx_buf, client->cfg.tx_buf_size);

    xrce_write_header(&w, client->cfg.session_id, SYN_XRCE_STREAM_BEST_EFFORT,
                      client->stream_seq_num++, client->cfg.client_key);

    size_t len_pos = xrce_start_submessage(&w, SYN_XRCE_SUBMSG_WRITE_DATA, 0U);
    uint16_t req_id = client->next_req_id++;
    (void)syn_cdr_write_u16(&w, req_id);
    (void)syn_cdr_write_u16(&w, writer_id);
    (void)syn_cdr_write_u8(&w, SYN_XRCE_DATA_FORMAT_DATA);
    (void)syn_cdr_write_bytes(&w, data, len);
    xrce_finish_submessage(&w, len_pos);

    if (w.error || !syn_transport_send(client->cfg.transport, w.data, w.pos)) {
        return SYN_ERROR;
    }

    return SYN_OK;
}

SYN_Status syn_xrce_client_read_data(SYN_XRCE_Client *client, uint16_t reader_id)
{
    if (client == NULL) {
        return SYN_INVALID_PARAM;
    }

    SYN_CDR_Writer w;
    syn_cdr_writer_init(&w, client->cfg.tx_buf, client->cfg.tx_buf_size);

    xrce_write_header(&w, client->cfg.session_id, SYN_XRCE_STREAM_BEST_EFFORT,
                      client->stream_seq_num++, client->cfg.client_key);

    size_t len_pos = xrce_start_submessage(&w, SYN_XRCE_SUBMSG_READ_DATA, 0U);
    uint16_t req_id = client->next_req_id++;
    (void)syn_cdr_write_u16(&w, req_id);
    (void)syn_cdr_write_u16(&w, reader_id);
    (void)syn_cdr_write_u16(&w, 1U); /* max_messages: 1 */
    (void)syn_cdr_write_u8(&w, SYN_XRCE_DATA_FORMAT_DATA);
    xrce_finish_submessage(&w, len_pos);

    if (w.error || !syn_transport_send(client->cfg.transport, w.data, w.pos)) {
        return SYN_ERROR;
    }

    return SYN_OK;
}

SYN_Status syn_xrce_client_step(SYN_XRCE_Client *client, uint32_t now_ms)
{
    if (client == NULL || client->cfg.transport == NULL) {
        return SYN_INVALID_PARAM;
    }

    /* 1. Receive incoming frame from transport */
    size_t rx_len = 0U;
    if (syn_transport_recv(client->cfg.transport, client->cfg.rx_buf, client->cfg.rx_buf_size,
                           &rx_len) &&
        rx_len >= 4U) {
        SYN_CDR_Reader r;
        syn_cdr_reader_init(&r, client->cfg.rx_buf, rx_len);

        uint8_t session_id = 0U;
        uint8_t stream_id = 0U;
        uint16_t seq_num = 0U;
        (void)syn_cdr_read_u8(&r, &session_id);
        (void)syn_cdr_read_u8(&r, &stream_id);
        (void)syn_cdr_read_u16(&r, &seq_num);

        if (session_id == 0x00U || session_id >= 0x80U) {
            uint32_t client_key = 0U;
            (void)syn_cdr_read_u32(&r, &client_key);
        }

        /* Parse Submessages in payload */
        while (r.pos + 4U <= r.size && !r.error) {
            uint8_t submsg_id = 0U;
            uint8_t flags = 0U;
            uint16_t submsg_len = 0U;
            (void)syn_cdr_read_u8(&r, &submsg_id);
            (void)syn_cdr_read_u8(&r, &flags);
            (void)syn_cdr_read_u16(&r, &submsg_len);

            size_t submsg_start = r.pos;

            if (submsg_id == SYN_XRCE_SUBMSG_STATUS) {
                uint16_t req_id = 0U;
                uint16_t obj_id = 0U;
                uint8_t status = 0U;
                uint8_t impl_status = 0U;
                (void)syn_cdr_read_u16(&r, &req_id);
                (void)syn_cdr_read_u16(&r, &obj_id);
                (void)syn_cdr_read_u8(&r, &status);
                (void)syn_cdr_read_u8(&r, &impl_status);

                if (client->state == SYN_XRCE_STATE_CONNECTING && status == SYN_XRCE_STATUS_OK) {
                    client->state = SYN_XRCE_STATE_CONNECTED;
                }

                if (client->cfg.on_status != NULL) {
                    client->cfg.on_status(req_id, status, client->cfg.user_data);
                }
            } else if (submsg_id == SYN_XRCE_SUBMSG_DATA) {
                uint16_t req_id = 0U;
                uint16_t obj_id = 0U;
                uint8_t data_fmt = 0U;
                (void)syn_cdr_read_u16(&r, &req_id);
                (void)syn_cdr_read_u16(&r, &obj_id);
                (void)syn_cdr_read_u8(&r, &data_fmt);

                size_t pld_len = (r.size > r.pos) ? r.size - r.pos : 0U;
                if (client->cfg.on_data != NULL && pld_len > 0U) {
                    client->cfg.on_data(obj_id, &r.data[r.pos], pld_len, client->cfg.user_data);
                }
            } else {
                /* Skip other submessages */
            }

            r.pos = submsg_start + submsg_len;
        }
    }

    /* 2. Check periodic heartbeat / keep-alive */
    if (client->state == SYN_XRCE_STATE_CONNECTED &&
        (now_ms - client->last_heartbeat_ms >= client->cfg.heartbeat_period_ms ||
         client->last_heartbeat_ms == 0U)) {
        SYN_CDR_Writer w;
        syn_cdr_writer_init(&w, client->cfg.tx_buf, client->cfg.tx_buf_size);

        xrce_write_header(&w, client->cfg.session_id, SYN_XRCE_STREAM_RELIABLE,
                          client->stream_seq_num++, client->cfg.client_key);

        size_t len_pos = xrce_start_submessage(&w, SYN_XRCE_SUBMSG_HEARTBEAT, 0U);
        (void)syn_cdr_write_u16(&w, 1U);
        (void)syn_cdr_write_u16(&w, client->stream_seq_num);
        (void)syn_cdr_write_u8(&w, SYN_XRCE_STREAM_RELIABLE);
        xrce_finish_submessage(&w, len_pos);

        if (!w.error) {
            (void)syn_transport_send(client->cfg.transport, w.data, w.pos);
            client->last_heartbeat_ms = now_ms;
        }
    }

    return SYN_OK;
}

SYN_PT_Status syn_xrce_client_pt(SYN_PT *pt, SYN_Task *task)
{
    if (pt == NULL || task == NULL || task->user_data == NULL) {
        return PT_ENDED;
    }

    SYN_XRCE_Client *client = (SYN_XRCE_Client *)task->user_data;

    PT_BEGIN(pt);

    for (;;) {
        uint32_t now = syn_port_get_tick_ms();
        if (syn_xrce_client_step(client, now) != SYN_OK) {
            break;
        }
        PT_YIELD(pt);
    }

    PT_END(pt);
}

#endif /* SYN_USE_XRCE_DDS */
```


