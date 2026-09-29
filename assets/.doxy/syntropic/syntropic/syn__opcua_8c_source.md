

# File syn\_opcua.c

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_opcua.c**](syn__opcua_8c.md)

[Go to the documentation of this file](syn__opcua_8c.md)


```C++

#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_OPCUA) || SYN_USE_OPCUA

#include "../port/syn_port_system.h"
#include "syn_opcua.h"

#include <string.h>

#define SYN_OPCUA_REQ_CREATE_SESSION 461U
#define SYN_OPCUA_RESP_CREATE_SESSION 464U
#define SYN_OPCUA_REQ_ACTIVATE_SESSION 467U
#define SYN_OPCUA_RESP_ACTIVATE_SESSION 470U
#define SYN_OPCUA_REQ_CLOSE_SESSION 473U
#define SYN_OPCUA_RESP_CLOSE_SESSION 476U
#define SYN_OPCUA_REQ_READ 631U
#define SYN_OPCUA_RESP_READ 634U
#define SYN_OPCUA_REQ_WRITE 673U
#define SYN_OPCUA_RESP_WRITE 676U
#define SYN_OPCUA_REQ_BROWSE 527U
#define SYN_OPCUA_RESP_BROWSE 530U

/* ── Byte Writer Helper ─────────────────────────────────────────────────── */

typedef struct {
    uint8_t *data; 
    size_t size;   
    size_t pos;    
    bool error;    
} OPCUA_Writer;

static void opcua_writer_init(OPCUA_Writer *w, uint8_t *buf, size_t size)
{
    w->data = buf;
    w->size = size;
    w->pos = 0U;
    w->error = (buf == NULL || size == 0U);
}

static bool opcua_write_u8(OPCUA_Writer *w, uint8_t v)
{
    if (w->error || w->pos + 1U > w->size) {
        w->error = true;
        return false;
    }
    w->data[w->pos++] = v;
    return true;
}

static bool opcua_write_u16(OPCUA_Writer *w, uint16_t v)
{
    if (w->error || w->pos + 2U > w->size) {
        w->error = true;
        return false;
    }
    w->data[w->pos++] = (uint8_t)(v & 0xFFU);
    w->data[w->pos++] = (uint8_t)((v >> 8U) & 0xFFU);
    return true;
}

static bool opcua_write_u32(OPCUA_Writer *w, uint32_t v)
{
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

static bool opcua_write_u64(OPCUA_Writer *w, uint64_t v)
{
    if (w->error || w->pos + 8U > w->size) {
        w->error = true;
        return false;
    }
    for (size_t i = 0U; i < 8U; i++) {
        w->data[w->pos++] = (uint8_t)((v >> (i * 8U)) & 0xFFU);
    }
    return true;
}

static bool opcua_write_bytes(OPCUA_Writer *w, const uint8_t *buf, size_t len)
{
    if (w->error || w->pos + len > w->size) {
        w->error = true;
        return false;
    }
    if (len > 0U && buf != NULL) {
        (void)memcpy(&w->data[w->pos], buf, len);
        w->pos += len;
    }
    return true;
}

static bool opcua_write_string(OPCUA_Writer *w, const char *str)
{
    if (str == NULL) {
        return opcua_write_u32(w, 0xFFFFFFFFU); /* Null string (-1) */
    }
    uint32_t len = (uint32_t)strlen(str);
    if (!opcua_write_u32(w, len)) {
        return false;
    }
    return opcua_write_bytes(w, (const uint8_t *)str, len);
}

static bool opcua_write_nodeid_num(OPCUA_Writer *w, uint16_t ns, uint32_t num)
{
    if (ns == 0U && num <= 0xFFU) {
        (void)opcua_write_u8(w, 0x00U); /* TwoByte NodeId */
        return opcua_write_u8(w, (uint8_t)num);
    }
    if (ns <= 0xFFU && num <= 0xFFFFU) {
        (void)opcua_write_u8(w, 0x01U); /* FourByte NodeId */
        (void)opcua_write_u8(w, (uint8_t)ns);
        return opcua_write_u16(w, (uint16_t)num);
    }
    (void)opcua_write_u8(w, 0x02U); /* Numeric NodeId */
    (void)opcua_write_u16(w, ns);
    return opcua_write_u32(w, num);
}

static bool opcua_write_variant(OPCUA_Writer *w, const SYN_OPCUA_Variant *var)
{
    if (var == NULL || var->type == SYN_OPCUA_TYPE_NULL) {
        return opcua_write_u8(w, 0x00U);
    }
    (void)opcua_write_u8(w, (uint8_t)var->type);
    switch (var->type) {
    case SYN_OPCUA_TYPE_BOOLEAN:
        return opcua_write_u8(w, var->val.boolean ? 1U : 0U);
    case SYN_OPCUA_TYPE_SBYTE:
    case SYN_OPCUA_TYPE_BYTE:
        return opcua_write_u8(w, var->val.byte);
    case SYN_OPCUA_TYPE_INT16:
    case SYN_OPCUA_TYPE_UINT16:
        return opcua_write_u16(w, var->val.uint16);
    case SYN_OPCUA_TYPE_INT32:
    case SYN_OPCUA_TYPE_UINT32:
        return opcua_write_u32(w, var->val.uint32);
    case SYN_OPCUA_TYPE_FLOAT: {
        uint32_t u;
        (void)memcpy(&u, &var->val.float_val, sizeof(u));
        return opcua_write_u32(w, u);
    }
    case SYN_OPCUA_TYPE_INT64:
    case SYN_OPCUA_TYPE_UINT64:
    case SYN_OPCUA_TYPE_DATETIME:
        return opcua_write_u64(w, var->val.uint64);
    case SYN_OPCUA_TYPE_DOUBLE: {
        uint64_t u;
        (void)memcpy(&u, &var->val.double_val, sizeof(u));
        return opcua_write_u64(w, u);
    }
    case SYN_OPCUA_TYPE_STRING:
        return opcua_write_string(w, var->val.string);
    default:
        return false;
    }
}

static bool opcua_write_datavalue(OPCUA_Writer *w, const SYN_OPCUA_DataValue *dv)
{
    uint8_t mask = 0x01U; /* Has Value */
    if (dv->status_code != SYN_OPCUA_STATUS_GOOD) {
        mask |= 0x02U; /* Has Status */
    }
    if (dv->source_ts_ms != 0U) {
        mask |= 0x04U; /* Has SourceTimestamp */
    }
    (void)opcua_write_u8(w, mask);
    (void)opcua_write_variant(w, &dv->value);
    if ((mask & 0x02U) != 0U) {
        (void)opcua_write_u32(w, dv->status_code);
    }
    if ((mask & 0x04U) != 0U) {
        (void)opcua_write_u64(w, dv->source_ts_ms);
    }
    return !w->error;
}

/* ── Byte Reader Helper ─────────────────────────────────────────────────── */

typedef struct {
    const uint8_t *data; 
    size_t size;         
    size_t pos;          
    bool error;          
} OPCUA_Reader;

static void opcua_reader_init(OPCUA_Reader *r, const uint8_t *buf, size_t size)
{
    r->data = buf;
    r->size = size;
    r->pos = 0U;
    r->error = (buf == NULL || size == 0U);
}

static bool opcua_read_u8(OPCUA_Reader *r, uint8_t *out)
{
    if (r->error || r->pos + 1U > r->size) {
        r->error = true;
        return false;
    }
    *out = r->data[r->pos++];
    return true;
}

static bool opcua_read_u16(OPCUA_Reader *r, uint16_t *out)
{
    if (r->error || r->pos + 2U > r->size) {
        r->error = true;
        return false;
    }
    *out = (uint16_t)r->data[r->pos] | ((uint16_t)r->data[r->pos + 1U] << 8U);
    r->pos += 2U;
    return true;
}

static bool opcua_read_u32(OPCUA_Reader *r, uint32_t *out)
{
    if (r->error || r->pos + 4U > r->size) {
        r->error = true;
        return false;
    }
    *out = (uint32_t)r->data[r->pos] | ((uint32_t)r->data[r->pos + 1U] << 8U) |
           ((uint32_t)r->data[r->pos + 2U] << 16U) | ((uint32_t)r->data[r->pos + 3U] << 24U);
    r->pos += 4U;
    return true;
}

static bool opcua_read_nodeid_num(OPCUA_Reader *r, uint16_t *out_ns, uint32_t *out_num)
{
    uint8_t enc = 0U;
    if (!opcua_read_u8(r, &enc)) {
        return false;
    }
    if (enc == 0x00U) { /* TwoByte */
        uint8_t id = 0U;
        if (!opcua_read_u8(r, &id)) {
            return false;
        }
        *out_ns = 0U;
        *out_num = id;
        return true;
    }
    if (enc == 0x01U) { /* FourByte */
        uint8_t ns = 0U;
        uint16_t id = 0U;
        if (!opcua_read_u8(r, &ns) || !opcua_read_u16(r, &id)) {
            return false;
        }
        *out_ns = ns;
        *out_num = id;
        return true;
    }
    if (enc == 0x02U) { /* Numeric */
        uint16_t ns = 0U;
        uint32_t id = 0U;
        if (!opcua_read_u16(r, &ns) || !opcua_read_u32(r, &id)) {
            return false;
        }
        *out_ns = ns;
        *out_num = id;
        return true;
    }
    return false;
}

/* ── Standard Address Space Setup ────────────────────────────────────────── */

static void opcua_populate_standard_nodes(SYN_OPCUA_Server *srv)
{
    SYN_OPCUA_Node root = {
        .node_id = {.ns_index = SYN_OPCUA_NS_STANDARD,
                    .id_type = SYN_OPCUA_NODEID_NUMERIC,
                    .id = {.num = SYN_OPCUA_NODEID_ROOT}},
        .node_class = SYN_OPCUA_NODECLASS_OBJECT,
        .browse_name = "Root",
        .display_name = "Root",
        .access_level = 0x01U,
    };
    (void)syn_opcua_server_register_node(srv, &root);

    SYN_OPCUA_Node objects = {
        .node_id = {.ns_index = SYN_OPCUA_NS_STANDARD,
                    .id_type = SYN_OPCUA_NODEID_NUMERIC,
                    .id = {.num = SYN_OPCUA_NODEID_OBJECTS_FOLDER}},
        .parent_id = {.ns_index = SYN_OPCUA_NS_STANDARD,
                      .id_type = SYN_OPCUA_NODEID_NUMERIC,
                      .id = {.num = SYN_OPCUA_NODEID_ROOT}},
        .node_class = SYN_OPCUA_NODECLASS_OBJECT,
        .browse_name = "Objects",
        .display_name = "Objects",
        .access_level = 0x01U,
    };
    (void)syn_opcua_server_register_node(srv, &objects);

    SYN_OPCUA_Node server_node = {
        .node_id = {.ns_index = SYN_OPCUA_NS_STANDARD,
                    .id_type = SYN_OPCUA_NODEID_NUMERIC,
                    .id = {.num = SYN_OPCUA_NODEID_SERVER}},
        .parent_id = {.ns_index = SYN_OPCUA_NS_STANDARD,
                      .id_type = SYN_OPCUA_NODEID_NUMERIC,
                      .id = {.num = SYN_OPCUA_NODEID_OBJECTS_FOLDER}},
        .node_class = SYN_OPCUA_NODECLASS_OBJECT,
        .browse_name = "Server",
        .display_name = "Server",
        .access_level = 0x01U,
    };
    (void)syn_opcua_server_register_node(srv, &server_node);

    SYN_OPCUA_Node server_status = {
        .node_id = {.ns_index = SYN_OPCUA_NS_STANDARD,
                    .id_type = SYN_OPCUA_NODEID_NUMERIC,
                    .id = {.num = SYN_OPCUA_NODEID_SERVER_SERVERSTATUS}},
        .parent_id = {.ns_index = SYN_OPCUA_NS_STANDARD,
                      .id_type = SYN_OPCUA_NODEID_NUMERIC,
                      .id = {.num = SYN_OPCUA_NODEID_SERVER}},
        .node_class = SYN_OPCUA_NODECLASS_VARIABLE,
        .browse_name = "ServerStatus",
        .display_name = "ServerStatus",
        .data_type = SYN_OPCUA_TYPE_INT32,
        .value = {.value = {.type = SYN_OPCUA_TYPE_INT32, .val = {.int32 = 0}},
                  .status_code = SYN_OPCUA_STATUS_GOOD},
        .access_level = 0x01U,
    };
    (void)syn_opcua_server_register_node(srv, &server_status);
}

/* ── Public API Implementation ───────────────────────────────────────────── */

SYN_Status syn_opcua_server_init(SYN_OPCUA_Server *srv, const SYN_OPCUA_Config *cfg)
{
    if (srv == NULL || cfg == NULL) {
        return SYN_INVALID_PARAM;
    }
    if (cfg->rx_buf == NULL || cfg->tx_buf == NULL || cfg->rx_buf_size < 128U ||
        cfg->tx_buf_size < 128U) {
        return SYN_INVALID_PARAM;
    }

    (void)memset(srv, 0, sizeof(*srv));
    srv->cfg = *cfg;
    if (srv->cfg.endpoint_url == NULL) {
        srv->cfg.endpoint_url = "opc.tcp://0.0.0.0:4840";
    }
    if (srv->cfg.server_name == NULL) {
        srv->cfg.server_name = "SyntropicOS OPC UA Server";
    }

    srv->state = SYN_OPCUA_STATE_CLOSED;
    srv->secure_channel_id = 1U;
    srv->security_token_id = 1U;
    srv->sequence_number = 1U;

    opcua_populate_standard_nodes(srv);
    return SYN_OK;
}

SYN_Status syn_opcua_server_register_node(SYN_OPCUA_Server *srv, const SYN_OPCUA_Node *node)
{
    if (srv == NULL || node == NULL) {
        return SYN_INVALID_PARAM;
    }
    if (srv->node_count >= SYN_OPCUA_MAX_NODES) {
        return SYN_ERROR;
    }
    srv->nodes[srv->node_count++] = *node;
    return SYN_OK;
}

SYN_OPCUA_Node *syn_opcua_server_find_node_num(SYN_OPCUA_Server *srv, uint16_t ns, uint32_t num_id)
{
    if (srv == NULL) {
        return NULL;
    }
    for (size_t i = 0; i < srv->node_count; i++) {
        SYN_OPCUA_Node *n = &srv->nodes[i];
        if (n->node_id.ns_index == ns && n->node_id.id_type == SYN_OPCUA_NODEID_NUMERIC &&
            n->node_id.id.num == num_id) {
            return n;
        }
    }
    return NULL;
}

SYN_Status syn_opcua_server_write_variable(SYN_OPCUA_Server *srv, uint16_t ns, uint32_t num_id,
                                           const SYN_OPCUA_Variant *val)
{
    if (srv == NULL || val == NULL) {
        return SYN_INVALID_PARAM;
    }
    SYN_OPCUA_Node *node = syn_opcua_server_find_node_num(srv, ns, num_id);
    if (node == NULL || node->node_class != SYN_OPCUA_NODECLASS_VARIABLE) {
        return SYN_ERROR;
    }
    node->value.value = *val;
    node->value.status_code = SYN_OPCUA_STATUS_GOOD;
    node->value.source_ts_ms = syn_port_get_tick_ms();

    if (node->on_write != NULL) {
        return node->on_write(srv, node, &node->value, node->user_data);
    }
    return SYN_OK;
}

SYN_Status syn_opcua_server_read_variable(const SYN_OPCUA_Server *srv, uint16_t ns, uint32_t num_id,
                                          SYN_OPCUA_Variant *out_val)
{
    if (srv == NULL || out_val == NULL) {
        return SYN_INVALID_PARAM;
    }
    for (size_t i = 0; i < srv->node_count; i++) {
        const SYN_OPCUA_Node *n = &srv->nodes[i];
        if (n->node_id.ns_index == ns && n->node_id.id_type == SYN_OPCUA_NODEID_NUMERIC &&
            n->node_id.id.num == num_id) {
            *out_val = n->value.value;
            return SYN_OK;
        }
    }
    return SYN_ERROR;
}

/* ── Service Processing ──────────────────────────────────────────────────── */

SYN_Status syn_opcua_server_process_message(SYN_OPCUA_Server *srv, const uint8_t *rx, size_t rx_len,
                                            uint8_t *tx, size_t max_tx, size_t *out_len)
{
    if (srv == NULL || rx == NULL || tx == NULL || out_len == NULL || rx_len < 8U) {
        return SYN_INVALID_PARAM;
    }

    *out_len = 0U;
    OPCUA_Reader r;
    opcua_reader_init(&r, rx, rx_len);

    char msg_type[4] = {0};
    msg_type[0] = (char)rx[0];
    msg_type[1] = (char)rx[1];
    msg_type[2] = (char)rx[2];
    msg_type[3] = (char)rx[3]; /* IsFinal: 'F' */

    uint32_t message_size = 0U;
    r.pos = 4U;
    (void)opcua_read_u32(&r, &message_size);

    OPCUA_Writer w;
    opcua_writer_init(&w, tx, max_tx);

    /* 1. Handle Hello Message (HEL) */
    if (strncmp(msg_type, "HELF", 4) == 0) {
        (void)opcua_write_bytes(&w, (const uint8_t *)"ACKF", 4);
        (void)opcua_write_u32(&w, 28U); /* Length */
        (void)opcua_write_u32(&w, SYN_OPCUA_PROTOCOL_VERSION);
        (void)opcua_write_u32(&w, (uint32_t)srv->cfg.rx_buf_size);
        (void)opcua_write_u32(&w, (uint32_t)srv->cfg.tx_buf_size);
        (void)opcua_write_u32(&w, SYN_OPCUA_DEFAULT_BUFFER_SIZE);
        (void)opcua_write_u32(&w, 1U); /* MaxChunkCount */

        if (w.error) {
            return SYN_ERROR;
        }

        srv->state = SYN_OPCUA_STATE_HELLO_RECEIVED;
        *out_len = w.pos;
        return SYN_OK;
    }

    /* 2. Handle OpenSecureChannel (OPN) */
    if (strncmp(msg_type, "OPNF", 4) == 0) {
        uint32_t channel_id = 0U;
        (void)opcua_read_u32(&r, &channel_id);

        (void)opcua_write_bytes(&w, (const uint8_t *)"OPNF", 4);
        size_t len_pos = w.pos;
        (void)opcua_write_u32(&w, 0U); /* Length placeholder */
        (void)opcua_write_u32(&w, srv->secure_channel_id);
        (void)opcua_write_u32(&w, srv->security_token_id);
        (void)opcua_write_u32(&w, srv->sequence_number++);
        (void)opcua_write_u32(&w, srv->request_id++);

        /* Response TypeId: OpenSecureChannelResponse */
        (void)opcua_write_nodeid_num(&w, 0U, 446U);
        /* ResponseHeader: timestamp, requestHandle, serviceResult */
        (void)opcua_write_u64(&w, syn_port_get_tick_ms());
        (void)opcua_write_u32(&w, 0U);
        (void)opcua_write_u32(&w, SYN_OPCUA_STATUS_GOOD);

        /* ServerProtocolVersion */
        (void)opcua_write_u32(&w, SYN_OPCUA_PROTOCOL_VERSION);
        /* SecurityToken: ChannelId, TokenId, CreatedAt, RevisedLifetime */
        (void)opcua_write_u32(&w, srv->secure_channel_id);
        (void)opcua_write_u32(&w, srv->security_token_id);
        (void)opcua_write_u64(&w, syn_port_get_tick_ms());
        (void)opcua_write_u32(&w, 3600000U); /* 1 hour lifetime */

        if (w.error) {
            return SYN_ERROR;
        }

        /* Patch length */
        uint32_t total_len = (uint32_t)w.pos;
        w.data[len_pos] = (uint8_t)(total_len & 0xFFU);
        w.data[len_pos + 1U] = (uint8_t)((total_len >> 8U) & 0xFFU);
        w.data[len_pos + 2U] = (uint8_t)((total_len >> 16U) & 0xFFU);
        w.data[len_pos + 3U] = (uint8_t)((total_len >> 24U) & 0xFFU);

        srv->state = SYN_OPCUA_STATE_SECURE_CHANNEL;
        *out_len = w.pos;
        return SYN_OK;
    }

    /* 3. Handle Service Messages (MSG) */
    if (strncmp(msg_type, "MSGF", 4) == 0) {
        uint32_t channel_id = 0U;
        uint32_t token_id = 0U;
        uint32_t seq_num = 0U;
        uint32_t req_id = 0U;
        (void)opcua_read_u32(&r, &channel_id);
        (void)opcua_read_u32(&r, &token_id);
        (void)opcua_read_u32(&r, &seq_num);
        (void)opcua_read_u32(&r, &req_id);

        uint16_t req_ns = 0U;
        uint32_t req_type_id = 0U;
        (void)opcua_read_nodeid_num(&r, &req_ns, &req_type_id);

        (void)opcua_write_bytes(&w, (const uint8_t *)"MSGF", 4);
        size_t len_pos = w.pos;
        (void)opcua_write_u32(&w, 0U); /* Length placeholder */
        (void)opcua_write_u32(&w, srv->secure_channel_id);
        (void)opcua_write_u32(&w, srv->security_token_id);
        (void)opcua_write_u32(&w, srv->sequence_number++);
        (void)opcua_write_u32(&w, req_id);

        /* Read Request Dispatch */
        if (req_type_id == SYN_OPCUA_REQ_READ) {
            (void)opcua_write_nodeid_num(&w, 0U, SYN_OPCUA_RESP_READ);
            /* ResponseHeader */
            (void)opcua_write_u64(&w, syn_port_get_tick_ms());
            (void)opcua_write_u32(&w, req_id);
            (void)opcua_write_u32(&w, SYN_OPCUA_STATUS_GOOD);

            /* Skip RequestHeader and read maxAge / timestampsToReturn */
            r.pos += 16U;

            uint32_t count = 0U;
            (void)opcua_read_u32(&r, &count);
            if (count > 16U) {
                count = 16U;
            }

            (void)opcua_write_u32(&w, count);
            for (uint32_t i = 0; i < count; i++) {
                uint16_t ns = 0U;
                uint32_t num = 0U;
                (void)opcua_read_nodeid_num(&r, &ns, &num);
                uint32_t attr_id = 0U;
                (void)opcua_read_u32(&r, &attr_id);

                SYN_OPCUA_Node *node = syn_opcua_server_find_node_num(srv, ns, num);
                if (node != NULL) {
                    if (node->on_read != NULL) {
                        (void)node->on_read(srv, node, &node->value, node->user_data);
                    }
                    (void)opcua_write_datavalue(&w, &node->value);
                } else {
                    SYN_OPCUA_DataValue bad = {.value = {.type = SYN_OPCUA_TYPE_NULL},
                                               .status_code = SYN_OPCUA_STATUS_BAD_NODE_ID_UNKNOWN};
                    (void)opcua_write_datavalue(&w, &bad);
                }
            }
        }
        /* Write Request Dispatch */
        else if (req_type_id == SYN_OPCUA_REQ_WRITE) {
            (void)opcua_write_nodeid_num(&w, 0U, SYN_OPCUA_RESP_WRITE);
            /* ResponseHeader */
            (void)opcua_write_u64(&w, syn_port_get_tick_ms());
            (void)opcua_write_u32(&w, req_id);
            (void)opcua_write_u32(&w, SYN_OPCUA_STATUS_GOOD);

            /* Skip RequestHeader */
            r.pos += 16U;

            uint32_t count = 0U;
            (void)opcua_read_u32(&r, &count);
            if (count > 16U) {
                count = 16U;
            }

            (void)opcua_write_u32(&w, count);
            for (uint32_t i = 0; i < count; i++) {
                uint16_t ns = 0U;
                uint32_t num = 0U;
                (void)opcua_read_nodeid_num(&r, &ns, &num);
                uint32_t attr_id = 0U;
                (void)opcua_read_u32(&r, &attr_id);

                SYN_OPCUA_Node *node = syn_opcua_server_find_node_num(srv, ns, num);
                if (node != NULL && (node->access_level & 0x02U) != 0U) {
                    (void)opcua_write_u32(&w, SYN_OPCUA_STATUS_GOOD);
                } else if (node != NULL) {
                    (void)opcua_write_u32(&w, SYN_OPCUA_STATUS_BAD_NOT_WRITABLE);
                } else {
                    (void)opcua_write_u32(&w, SYN_OPCUA_STATUS_BAD_NODE_ID_UNKNOWN);
                }
            }
        }
        /* Browse Request Dispatch */
        else if (req_type_id == SYN_OPCUA_REQ_BROWSE) {
            (void)opcua_write_nodeid_num(&w, 0U, SYN_OPCUA_RESP_BROWSE);
            /* ResponseHeader */
            (void)opcua_write_u64(&w, syn_port_get_tick_ms());
            (void)opcua_write_u32(&w, req_id);
            (void)opcua_write_u32(&w, SYN_OPCUA_STATUS_GOOD);

            /* Results count */
            (void)opcua_write_u32(&w, 1U);
            (void)opcua_write_u32(&w, SYN_OPCUA_STATUS_GOOD);
            (void)opcua_write_string(&w, NULL); /* ContinuationPoint = Null */

            /* Return list of child nodes */
            uint32_t ref_count = 0U;
            for (size_t i = 0; i < srv->node_count; i++) {
                if (srv->nodes[i].node_id.id.num != SYN_OPCUA_NODEID_ROOT) {
                    ref_count++;
                }
            }
            (void)opcua_write_u32(&w, ref_count);
            for (size_t i = 0; i < srv->node_count; i++) {
                const SYN_OPCUA_Node *n = &srv->nodes[i];
                if (n->node_id.id.num == SYN_OPCUA_NODEID_ROOT) {
                    continue;
                }
                (void)opcua_write_nodeid_num(&w, 0U, 47U); /* HasComponent ReferenceType */
                (void)opcua_write_u8(&w, 0U);              /* IsForward = true */
                (void)opcua_write_nodeid_num(&w, n->node_id.ns_index, n->node_id.id.num);
                (void)opcua_write_string(&w, n->browse_name);
                (void)opcua_write_string(&w, n->display_name);
                (void)opcua_write_u32(&w, (uint32_t)n->node_class);
                (void)opcua_write_nodeid_num(&w, 0U, 0U); /* TypeDefinition */
            }
        }
        /* Create Session Request Dispatch */
        else if (req_type_id == SYN_OPCUA_REQ_CREATE_SESSION) {
            (void)opcua_write_nodeid_num(&w, 0U, SYN_OPCUA_RESP_CREATE_SESSION);
            (void)opcua_write_u64(&w, syn_port_get_tick_ms());
            (void)opcua_write_u32(&w, req_id);
            (void)opcua_write_u32(&w, SYN_OPCUA_STATUS_GOOD);

            (void)opcua_write_nodeid_num(&w, 1U, 1001U); /* SessionId */
            (void)opcua_write_nodeid_num(&w, 1U, 1002U); /* AuthenticationToken */
            (void)opcua_write_u64(&w, 3600000U);         /* RevisedSessionTimeout */

            srv->state = SYN_OPCUA_STATE_SESSION_ACTIVE;
        }
        /* Activate Session Request Dispatch */
        else if (req_type_id == SYN_OPCUA_REQ_ACTIVATE_SESSION) {
            (void)opcua_write_nodeid_num(&w, 0U, SYN_OPCUA_RESP_ACTIVATE_SESSION);
            (void)opcua_write_u64(&w, syn_port_get_tick_ms());
            (void)opcua_write_u32(&w, req_id);
            (void)opcua_write_u32(&w, SYN_OPCUA_STATUS_GOOD);
        }
        /* Close Session Request Dispatch */
        else if (req_type_id == SYN_OPCUA_REQ_CLOSE_SESSION) {
            (void)opcua_write_nodeid_num(&w, 0U, SYN_OPCUA_RESP_CLOSE_SESSION);
            (void)opcua_write_u64(&w, syn_port_get_tick_ms());
            (void)opcua_write_u32(&w, req_id);
            (void)opcua_write_u32(&w, SYN_OPCUA_STATUS_GOOD);

            srv->state = SYN_OPCUA_STATE_CLOSED;
        } else {
            /* Unsupported Service */
            (void)opcua_write_nodeid_num(&w, 0U, req_type_id + 3U);
            (void)opcua_write_u64(&w, syn_port_get_tick_ms());
            (void)opcua_write_u32(&w, req_id);
            (void)opcua_write_u32(&w, SYN_OPCUA_STATUS_BAD_SERVICE_UNSUPPORTED);
        }

        if (w.error) {
            return SYN_ERROR;
        }

        /* Patch Total Length */
        uint32_t total_len = (uint32_t)w.pos;
        w.data[len_pos] = (uint8_t)(total_len & 0xFFU);
        w.data[len_pos + 1U] = (uint8_t)((total_len >> 8U) & 0xFFU);
        w.data[len_pos + 2U] = (uint8_t)((total_len >> 16U) & 0xFFU);
        w.data[len_pos + 3U] = (uint8_t)((total_len >> 24U) & 0xFFU);

        *out_len = w.pos;
        return SYN_OK;
    }

    return SYN_ERROR;
}

SYN_Status syn_opcua_server_step(SYN_OPCUA_Server *srv, uint32_t now_ms)
{
    (void)now_ms;
    if (srv == NULL || srv->cfg.transport == NULL) {
        return SYN_INVALID_PARAM;
    }

    size_t rx_len = 0U;
    if (syn_transport_recv(srv->cfg.transport, srv->cfg.rx_buf, srv->cfg.rx_buf_size, &rx_len) &&
        rx_len >= 8U) {
        size_t tx_len = 0U;
        SYN_Status st = syn_opcua_server_process_message(
            srv, srv->cfg.rx_buf, rx_len, srv->cfg.tx_buf, srv->cfg.tx_buf_size, &tx_len);
        if (st == SYN_OK && tx_len > 0U) {
            (void)syn_transport_send(srv->cfg.transport, srv->cfg.tx_buf, tx_len);
        }
    }
    return SYN_OK;
}

SYN_PT_Status syn_opcua_server_pt(SYN_PT *pt, SYN_Task *task)
{
    if (pt == NULL || task == NULL || task->user_data == NULL) {
        return PT_ENDED;
    }

    SYN_OPCUA_Server *srv = (SYN_OPCUA_Server *)task->user_data;

    PT_BEGIN(pt);

    for (;;) {
        uint32_t now = syn_port_get_tick_ms();
        if (syn_opcua_server_step(srv, now) != SYN_OK) {
            break;
        }
        PT_YIELD(pt);
    }

    PT_END(pt);
}

#endif /* SYN_USE_OPCUA */
```


