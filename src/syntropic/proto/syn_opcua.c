/**
 * @file syn_opcua.c
 * @brief Micro OPC UA (IEC 62541) Binary Protocol Engine & Embedded Server Implementation.
 */

#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_OPCUA) || SYN_USE_OPCUA

#include "../port/syn_port_system.h"
#include "../sched/syn_sched.h"
#include "syn_opcua.h"

#include <string.h>

/** @brief OPC UA ServiceFault NodeId */
#define SYN_OPCUA_RESP_SERVICE_FAULT 397U
/** @brief OPC UA CreateSessionRequest NodeId */
#define SYN_OPCUA_REQ_CREATE_SESSION 461U
/** @brief OPC UA CreateSessionResponse NodeId */
#define SYN_OPCUA_RESP_CREATE_SESSION 464U
/** @brief OPC UA ActivateSessionRequest NodeId */
#define SYN_OPCUA_REQ_ACTIVATE_SESSION 467U
/** @brief OPC UA ActivateSessionResponse NodeId */
#define SYN_OPCUA_RESP_ACTIVATE_SESSION 470U
/** @brief OPC UA CloseSessionRequest NodeId */
#define SYN_OPCUA_REQ_CLOSE_SESSION 473U
/** @brief OPC UA CloseSessionResponse NodeId */
#define SYN_OPCUA_RESP_CLOSE_SESSION 476U
/** @brief OPC UA ReadRequest NodeId */
#define SYN_OPCUA_REQ_READ 631U
/** @brief OPC UA ReadResponse NodeId */
#define SYN_OPCUA_RESP_READ 634U
/** @brief OPC UA WriteRequest NodeId */
#define SYN_OPCUA_REQ_WRITE 673U
/** @brief OPC UA WriteResponse NodeId */
#define SYN_OPCUA_RESP_WRITE 676U
/** @brief OPC UA BrowseRequest NodeId */
#define SYN_OPCUA_REQ_BROWSE 527U
/** @brief OPC UA BrowseResponse NodeId */
#define SYN_OPCUA_RESP_BROWSE 530U

/* ── Byte Writer Helper ─────────────────────────────────────────────────── */

/**
 * @brief Binary serializer context for OPC UA messages.
 */
typedef struct {
    uint8_t *data; /**< Destination buffer pointer */
    size_t size;   /**< Total buffer capacity */
    size_t pos;    /**< Current write position */
    bool error;    /**< Error flag on overflow */
} OPCUA_Writer;

/**
 * @brief Initialize OPC UA binary writer.
 * @param w Writer context.
 * @param buf Output buffer.
 * @param size Output buffer size.
 */
static void opcua_writer_init(OPCUA_Writer *w, uint8_t *buf, size_t size)
{
    w->data = buf;
    w->size = size;
    w->pos = 0U;
    w->error = (buf == NULL || size == 0U);
}

/**
 * @brief Write 8-bit unsigned integer.
 * @param w Writer context.
 * @param v Value to write.
 * @return True on success, false on overflow.
 */
static bool opcua_write_u8(OPCUA_Writer *w, uint8_t v)
{
    if (w->error || w->pos + 1U > w->size) {
        w->error = true;
        return false;
    }
    w->data[w->pos++] = v;
    return true;
}

/**
 * @brief Write 16-bit little-endian unsigned integer.
 * @param w Writer context.
 * @param v Value to write.
 * @return True on success, false on overflow.
 */
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

/**
 * @brief Write 32-bit little-endian unsigned integer.
 * @param w Writer context.
 * @param v Value to write.
 * @return True on success, false on overflow.
 */
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

/**
 * @brief Write 64-bit little-endian unsigned integer.
 * @param w Writer context.
 * @param v Value to write.
 * @return True on success, false on overflow.
 */
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

/**
 * @brief Write raw byte buffer.
 * @param w Writer context.
 * @param buf Source bytes.
 * @param len Number of bytes.
 * @return True on success, false on overflow.
 */
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

/**
 * @brief Write length-prefixed OPC UA string.
 * @param w Writer context.
 * @param str Null-terminated string or NULL for null string.
 * @return True on success, false on overflow.
 */
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

/**
 * @brief Write numeric OPC UA NodeId.
 * @param w Writer context.
 * @param ns Namespace index.
 * @param num Numeric identifier.
 * @return True on success, false on overflow.
 */
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

/**
 * @brief Write integer or boolean OPC UA variant payload.
 * @param w         Writer context.
 * @param var       Variant structure pointer.
 * @param out_wrote Set true if var->type was handled by this helper.
 * @return True on success, false on serialization overflow.
 */
static bool opcua_write_variant_int(OPCUA_Writer *w, const SYN_OPCUA_Variant *var, bool *out_wrote)
{
    *out_wrote = true;
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
    case SYN_OPCUA_TYPE_INT64:
    case SYN_OPCUA_TYPE_UINT64:
    case SYN_OPCUA_TYPE_DATETIME:
        return opcua_write_u64(w, var->val.uint64);
    default:
        *out_wrote = false;
        return false;
    }
}

/**
 * @brief Write OPC UA variant value.
 * @param w Writer context.
 * @param var Variant structure pointer.
 * @return True on success, false on serialization error.
 */
static bool opcua_write_variant(OPCUA_Writer *w, const SYN_OPCUA_Variant *var)
{
    if (var == NULL || var->type == SYN_OPCUA_TYPE_NULL) {
        return opcua_write_u8(w, 0x00U);
    }
    (void)opcua_write_u8(w, (uint8_t)var->type);
    bool wrote = false;
    bool ok = opcua_write_variant_int(w, var, &wrote);
    if (wrote) {
        return ok;
    }
    if (var->type == SYN_OPCUA_TYPE_FLOAT) {
        uint32_t u;
        (void)memcpy(&u, &var->val.float_val, sizeof(u));
        return opcua_write_u32(w, u);
    }
    if (var->type == SYN_OPCUA_TYPE_DOUBLE) {
        uint64_t u;
        (void)memcpy(&u, &var->val.double_val, sizeof(u));
        return opcua_write_u64(w, u);
    }
    if (var->type == SYN_OPCUA_TYPE_STRING) {
        return opcua_write_string(w, var->val.string);
    }
    return false;
}

/**
 * @brief Write OPC UA DataValue structure.
 * @param w Writer context.
 * @param dv DataValue structure pointer.
 * @return True on success, false on error.
 */
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

/**
 * @brief Binary deserializer context for OPC UA messages.
 */
typedef struct {
    const uint8_t *data; /**< Source buffer pointer */
    size_t size;         /**< Total buffer size */
    size_t pos;          /**< Current read position */
    bool error;          /**< Error flag on underflow */
} OPCUA_Reader;

/**
 * @brief Initialize OPC UA binary reader.
 * @param r Reader context.
 * @param buf Input buffer.
 * @param size Input buffer size.
 */
static void opcua_reader_init(OPCUA_Reader *r, const uint8_t *buf, size_t size)
{
    r->data = buf;
    r->size = size;
    r->pos = 0U;
    r->error = (buf == NULL || size == 0U);
}

/**
 * @brief Read 8-bit unsigned integer.
 * @param r Reader context.
 * @param out Pointer to receive value.
 * @return True on success, false on underflow.
 */
static bool opcua_read_u8(OPCUA_Reader *r, uint8_t *out)
{
    if (r->error || r->pos + 1U > r->size) {
        r->error = true;
        return false;
    }
    *out = r->data[r->pos++];
    return true;
}

/**
 * @brief Read 16-bit little-endian unsigned integer.
 * @param r Reader context.
 * @param out Pointer to receive value.
 * @return True on success, false on underflow.
 */
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

/**
 * @brief Read 32-bit little-endian unsigned integer.
 * @param r Reader context.
 * @param out Pointer to receive value.
 * @return True on success, false on underflow.
 */
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

/**
 * @brief Read 64-bit little-endian unsigned integer.
 * @param r Reader context.
 * @param out Pointer to receive value.
 * @return True on success, false on underflow.
 */
static bool opcua_read_u64(OPCUA_Reader *r, uint64_t *out)
{
    uint32_t lo = 0U;
    uint32_t hi = 0U;
    if (!opcua_read_u32(r, &lo) || !opcua_read_u32(r, &hi)) {
        return false;
    }
    *out = (uint64_t)lo | ((uint64_t)hi << 32U);
    return true;
}

/**
 * @brief Advance reader by count bytes with bounds checking.
 * @param r     Reader context.
 * @param count Number of bytes to skip.
 * @return True on success, false on underflow.
 */
static bool opcua_skip(OPCUA_Reader *r, size_t count)
{
    if (r->error || count > r->size - r->pos) {
        r->error = true;
        return false;
    }
    r->pos += count;
    return true;
}

/**
 * @brief Read numeric OPC UA NodeId.
 * @param r Reader context.
 * @param out_ns Pointer to receive namespace index.
 * @param out_num Pointer to receive numeric identifier.
 * @return True on success, false on invalid format or underflow.
 */
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

/**
 * @brief Populate default standard OPC UA address space folders and server status node.
 * @param srv Server instance pointer.
 */
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
    SYN_OPCUA_Node *dst = &srv->nodes[srv->node_count++];
    *dst = *node;
    if ((dst->node_class == SYN_OPCUA_NODECLASS_VARIABLE) &&
        (dst->value.value.type == SYN_OPCUA_TYPE_NULL) && (dst->data_type != SYN_OPCUA_TYPE_NULL)) {
        dst->value.value.type = dst->data_type;
    }
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

/**
 * @brief Patch 32-bit little-endian total message size into UASC frame header.
 * @param w       Writer context.
 * @param len_pos Offset of 4-byte length field inside w->data.
 */
static void opcua_patch_u32_len(OPCUA_Writer *w, size_t len_pos)
{
    uint32_t total_len = (uint32_t)w->pos;
    w->data[len_pos] = (uint8_t)(total_len & 0xFFU);
    w->data[len_pos + 1U] = (uint8_t)((total_len >> 8U) & 0xFFU);
    w->data[len_pos + 2U] = (uint8_t)((total_len >> 16U) & 0xFFU);
    w->data[len_pos + 3U] = (uint8_t)((total_len >> 24U) & 0xFFU);
}

/**
 * @brief Handle UACP Hello (HELF) message and encode ACKF response.
 * @param srv     Server instance.
 * @param w       Response writer.
 * @param out_len Output response length.
 * @return SYN_OK on success, SYN_ERROR on buffer overflow.
 */
static SYN_Status opcua_handle_hel(SYN_OPCUA_Server *srv, OPCUA_Writer *w, size_t *out_len)
{
    (void)opcua_write_bytes(w, (const uint8_t *)"ACKF", 4U);
    (void)opcua_write_u32(w, 28U);
    (void)opcua_write_u32(w, SYN_OPCUA_PROTOCOL_VERSION);
    (void)opcua_write_u32(w, (uint32_t)srv->cfg.rx_buf_size);
    (void)opcua_write_u32(w, (uint32_t)srv->cfg.tx_buf_size);
    (void)opcua_write_u32(w, SYN_OPCUA_DEFAULT_BUFFER_SIZE);
    (void)opcua_write_u32(w, 1U);

    if (w->error) {
        return SYN_ERROR;
    }

    srv->state = SYN_OPCUA_STATE_HELLO_RECEIVED;
    *out_len = w->pos;
    return SYN_OK;
}

/**
 * @brief Handle UASC OpenSecureChannel (OPNF) request and encode OPNF response.
 * @param srv     Server instance.
 * @param r       Request reader.
 * @param w       Response writer.
 * @param out_len Output response length.
 * @return SYN_OK on success, SYN_ERROR on buffer overflow.
 */
static SYN_Status opcua_handle_opn(SYN_OPCUA_Server *srv, OPCUA_Reader *r, OPCUA_Writer *w,
                                   size_t *out_len)
{
    uint32_t channel_id = 0U;
    (void)opcua_read_u32(r, &channel_id);

    (void)opcua_write_bytes(w, (const uint8_t *)"OPNF", 4U);
    size_t len_pos = w->pos;
    (void)opcua_write_u32(w, 0U);
    (void)opcua_write_u32(w, srv->secure_channel_id);
    (void)opcua_write_u32(w, srv->security_token_id);
    (void)opcua_write_u32(w, srv->sequence_number++);
    (void)opcua_write_u32(w, srv->request_id++);

    (void)opcua_write_nodeid_num(w, 0U, 446U);
    (void)opcua_write_u64(w, syn_port_get_tick_ms());
    (void)opcua_write_u32(w, 0U);
    (void)opcua_write_u32(w, SYN_OPCUA_STATUS_GOOD);

    (void)opcua_write_u32(w, SYN_OPCUA_PROTOCOL_VERSION);
    (void)opcua_write_u32(w, srv->secure_channel_id);
    (void)opcua_write_u32(w, srv->security_token_id);
    (void)opcua_write_u64(w, syn_port_get_tick_ms());
    (void)opcua_write_u32(w, 3600000U);

    if (w->error) {
        return SYN_ERROR;
    }

    opcua_patch_u32_len(w, len_pos);
    srv->state = SYN_OPCUA_STATE_SECURE_CHANNEL;
    *out_len = w->pos;
    return SYN_OK;
}

/**
 * @brief Encode ReadResponse payload for an inbound ReadRequest.
 * @param srv    Server instance.
 * @param r      Request reader positioned after RequestTypeId.
 * @param w      Response writer.
 * @param req_id Request identifier.
 */
static void opcua_service_read(SYN_OPCUA_Server *srv, OPCUA_Reader *r, OPCUA_Writer *w,
                               uint32_t req_id)
{
    (void)opcua_write_nodeid_num(w, 0U, SYN_OPCUA_RESP_READ);
    (void)opcua_write_u64(w, syn_port_get_tick_ms());
    (void)opcua_write_u32(w, req_id);
    (void)opcua_write_u32(w, SYN_OPCUA_STATUS_GOOD);

    /* Skip RequestHeader / maxAge / timestampsToReturn with bounds check */
    (void)opcua_skip(r, 16U);

    uint32_t count = 0U;
    (void)opcua_read_u32(r, &count);
    if (count > 16U) {
        count = 16U;
    }

    (void)opcua_write_u32(w, count);
    for (uint32_t i = 0U; i < count; i++) {
        uint16_t ns = 0U;
        uint32_t num = 0U;
        (void)opcua_read_nodeid_num(r, &ns, &num);
        uint32_t attr_id = 0U;
        (void)opcua_read_u32(r, &attr_id);

        SYN_OPCUA_Node *node = syn_opcua_server_find_node_num(srv, ns, num);
        if (node != NULL) {
            if (node->on_read != NULL) {
                (void)node->on_read(srv, node, &node->value, node->user_data);
            }
            (void)opcua_write_datavalue(w, &node->value);
        } else {
            SYN_OPCUA_DataValue bad = {.value = {.type = SYN_OPCUA_TYPE_NULL},
                                       .status_code = SYN_OPCUA_STATUS_BAD_NODE_ID_UNKNOWN};
            (void)opcua_write_datavalue(w, &bad);
        }
    }
}

/**
 * @brief Decoded single WriteValue entry from a WriteRequest.
 */
typedef struct {
    uint16_t ns;                            /**< Target namespace index */
    uint32_t num;                           /**< Target numeric identifier */
    uint32_t attr_id;                       /**< Target AttributeId */
    bool has_index_range;                   /**< Non-empty IndexRange present */
    uint8_t dv_mask;                        /**< DataValue encoding mask */
    SYN_OPCUA_Variant var;                  /**< Decoded Variant value */
    uint32_t status_code;                   /**< Optional DataValue statusCode */
    uint64_t source_ts_ms;                  /**< Optional DataValue sourceTimestamp */
    char str_buf[SYN_OPCUA_MAX_STRING_LEN]; /**< Decoded string buffer */
    bool str_is_null;                       /**< True if decoded string was null (-1) */
} OPCUA_WriteItem;

/**
 * @brief Read integer or boolean OPC UA Variant payload.
 * @param r Binary stream reader.
 * @param type OPC UA built-in data type identifier.
 * @param out Output Variant structure.
 * @param out_handled Set true if type belongs to integer/boolean family, false otherwise.
 * @return true on successful decode, false on stream underflow or unhandled type.
 */
static bool opcua_read_variant_int(OPCUA_Reader *r, SYN_OPCUA_DataType type, SYN_OPCUA_Variant *out,
                                   bool *out_handled)
{
    *out_handled = true;
    switch (type) {
    case SYN_OPCUA_TYPE_BOOLEAN: {
        uint8_t b = 0U;
        if (!opcua_read_u8(r, &b)) {
            return false;
        }
        out->val.boolean = (b != 0U);
        return true;
    }
    case SYN_OPCUA_TYPE_SBYTE:
    case SYN_OPCUA_TYPE_BYTE:
        return opcua_read_u8(r, &out->val.byte);
    case SYN_OPCUA_TYPE_INT16:
    case SYN_OPCUA_TYPE_UINT16:
        return opcua_read_u16(r, &out->val.uint16);
    case SYN_OPCUA_TYPE_INT32:
    case SYN_OPCUA_TYPE_UINT32:
        return opcua_read_u32(r, &out->val.uint32);
    case SYN_OPCUA_TYPE_INT64:
    case SYN_OPCUA_TYPE_UINT64:
    case SYN_OPCUA_TYPE_DATETIME:
        return opcua_read_u64(r, &out->val.uint64);
    default:
        *out_handled = false;
        return false;
    }
}

/**
 * @brief Read OPC UA String payload into bounded scratch buffer.
 * @param r Binary stream reader.
 * @param buf Destination NUL-terminated scratch character buffer.
 * @param out_is_null Set true if wire string length was -1 (null string).
 * @return true on success, false on stream underflow or length overflow.
 */
static bool opcua_read_string_buf(OPCUA_Reader *r, char buf[SYN_OPCUA_MAX_STRING_LEN],
                                  bool *out_is_null)
{
    uint32_t slen = 0U;
    if (!opcua_read_u32(r, &slen)) {
        return false;
    }
    if (slen == 0xFFFFFFFFU) {
        buf[0] = '\0';
        *out_is_null = true;
        return true;
    }
    *out_is_null = false;
    if ((slen >= SYN_OPCUA_MAX_STRING_LEN) || (slen > (r->size - r->pos))) {
        r->error = true;
        return false;
    }
    if (slen > 0U) {
        (void)memcpy(buf, &r->data[r->pos], slen);
        r->pos += slen;
    }
    buf[slen] = '\0';
    return true;
}

/**
 * @brief Read typed OPC UA Variant from binary stream.
 * @param r Binary stream reader.
 * @param item Output decoded WriteValue item receiving the Variant value.
 * @return true on valid scalar Variant decode, false otherwise.
 */
static bool opcua_read_variant(OPCUA_Reader *r, OPCUA_WriteItem *item)
{
    uint8_t enc = 0U;
    if (!opcua_read_u8(r, &enc) || ((enc & 0xC0U) != 0U)) {
        r->error = true;
        return false;
    }
    SYN_OPCUA_DataType type = (SYN_OPCUA_DataType)(enc & 0x3FU);
    item->var.type = type;
    if (type == SYN_OPCUA_TYPE_NULL) {
        return true;
    }
    bool handled = false;
    bool ok = opcua_read_variant_int(r, type, &item->var, &handled);
    if (handled) {
        return ok;
    }
    if (type == SYN_OPCUA_TYPE_FLOAT) {
        uint32_t raw = 0U;
        if (!opcua_read_u32(r, &raw)) {
            return false;
        }
        (void)memcpy(&item->var.val.float_val, &raw, sizeof(raw));
        return true;
    }
    if (type == SYN_OPCUA_TYPE_DOUBLE) {
        uint64_t raw = 0U;
        if (!opcua_read_u64(r, &raw)) {
            return false;
        }
        (void)memcpy(&item->var.val.double_val, &raw, sizeof(raw));
        return true;
    }
    if (type == SYN_OPCUA_TYPE_STRING) {
        return opcua_read_string_buf(r, item->str_buf, &item->str_is_null);
    }
    r->error = true;
    return false;
}

/**
 * @brief Decode DataValue trailing fields for a WriteValue item.
 * @param r Binary stream reader.
 * @param item Output WriteValue item receiving DataValue fields.
 * @return true on valid DataValue decode, false on stream error.
 */
static bool opcua_decode_write_datavalue(OPCUA_Reader *r, OPCUA_WriteItem *item)
{
    if (!opcua_read_u8(r, &item->dv_mask)) {
        return false;
    }
    if (((item->dv_mask & 0x01U) != 0U) && !opcua_read_variant(r, item)) {
        return false;
    }
    if (((item->dv_mask & 0x02U) != 0U) && !opcua_read_u32(r, &item->status_code)) {
        return false;
    }
    if (((item->dv_mask & 0x04U) != 0U) && !opcua_read_u64(r, &item->source_ts_ms)) {
        return false;
    }
    size_t extra_ts = 0U;
    if ((item->dv_mask & 0x10U) != 0U) {
        extra_ts += 2U;
    }
    if ((item->dv_mask & 0x08U) != 0U) {
        extra_ts += 8U;
    }
    if ((item->dv_mask & 0x20U) != 0U) {
        extra_ts += 2U;
    }
    return (extra_ts == 0U) || opcua_skip(r, extra_ts);
}

/**
 * @brief Decode one WriteValue item from a WriteRequest stream.
 * @param r Binary stream reader.
 * @param item Output decoded WriteValue item.
 * @return true on valid decode, false on malformed or truncated stream.
 */
static bool opcua_decode_write_item(OPCUA_Reader *r, OPCUA_WriteItem *item)
{
    (void)memset(item, 0, sizeof(*item));
    if (!opcua_read_nodeid_num(r, &item->ns, &item->num) || !opcua_read_u32(r, &item->attr_id)) {
        return false;
    }
    uint32_t idx_len = 0U;
    if (!opcua_read_u32(r, &idx_len)) {
        return false;
    }
    if ((idx_len != 0xFFFFFFFFU) && (idx_len > 0U)) {
        item->has_index_range = true;
        if (!opcua_skip(r, idx_len)) {
            return false;
        }
    }
    return opcua_decode_write_datavalue(r, item);
}

/**
 * @brief Commit a validated WriteValue item to a Variable node and invoke on_write.
 * @param srv OPC UA server instance.
 * @param node Target Variable node in server address space.
 * @param item Validated WriteValue item to commit.
 * @return OPC UA StatusCode (SYN_OPCUA_STATUS_GOOD or error code).
 */
static uint32_t opcua_commit_write_item(SYN_OPCUA_Server *srv, SYN_OPCUA_Node *node,
                                        const OPCUA_WriteItem *item)
{
    SYN_OPCUA_DataValue prev_dv = node->value;
    SYN_OPCUA_Variant committed = item->var;
    if (committed.type == SYN_OPCUA_TYPE_STRING) {
        if (item->str_is_null) {
            committed.val.string = NULL;
        } else {
            (void)memcpy(node->string_buf, item->str_buf, sizeof(node->string_buf));
            committed.val.string = node->string_buf;
        }
    }
    node->value.value = committed;
    node->value.status_code =
        ((item->dv_mask & 0x02U) != 0U) ? item->status_code : SYN_OPCUA_STATUS_GOOD;
    node->value.source_ts_ms =
        ((item->dv_mask & 0x04U) != 0U) ? item->source_ts_ms : syn_port_get_tick_ms();

    if ((node->on_write != NULL) &&
        (node->on_write(srv, node, &node->value, node->user_data) != SYN_OK)) {
        node->value = prev_dv;
        return SYN_OPCUA_STATUS_BAD_INTERNAL_ERROR;
    }
    return SYN_OPCUA_STATUS_GOOD;
}

/**
 * @brief Validate and apply a decoded WriteValue item to the server address space.
 * @param srv OPC UA server instance.
 * @param item Decoded WriteValue item.
 * @return OPC UA StatusCode result for this write operation.
 */
static uint32_t opcua_apply_write_item(SYN_OPCUA_Server *srv, const OPCUA_WriteItem *item)
{
    SYN_OPCUA_Node *node = syn_opcua_server_find_node_num(srv, item->ns, item->num);
    if (node == NULL) {
        return SYN_OPCUA_STATUS_BAD_NODE_ID_UNKNOWN;
    }
    if (item->attr_id != SYN_OPCUA_ATTR_VALUE) {
        return SYN_OPCUA_STATUS_BAD_NOT_WRITABLE;
    }
    if (item->has_index_range) {
        return SYN_OPCUA_STATUS_BAD_INDEX_RANGE_INVALID;
    }
    if ((node->node_class != SYN_OPCUA_NODECLASS_VARIABLE) ||
        ((node->access_level & 0x02U) == 0U)) {
        return SYN_OPCUA_STATUS_BAD_NOT_WRITABLE;
    }
    SYN_OPCUA_DataType expected_type =
        (node->value.value.type != SYN_OPCUA_TYPE_NULL) ? node->value.value.type : node->data_type;
    if (((item->dv_mask & 0x01U) == 0U) || (item->var.type != expected_type)) {
        return SYN_OPCUA_STATUS_BAD_TYPE_MISMATCH;
    }
    return opcua_commit_write_item(srv, node, item);
}

/**
 * @brief Encode OPC UA ServiceFault response body.
 * @param w Binary response writer.
 * @param req_id Client request identifier.
 * @param status_code Fault OPC UA StatusCode.
 */
static void opcua_write_service_fault(OPCUA_Writer *w, uint32_t req_id, uint32_t status_code)
{
    (void)opcua_write_nodeid_num(w, 0U, SYN_OPCUA_RESP_SERVICE_FAULT);
    (void)opcua_write_u64(w, syn_port_get_tick_ms());
    (void)opcua_write_u32(w, req_id);
    (void)opcua_write_u32(w, status_code);
}

/**
 * @brief Encode WriteResponse payload for an inbound WriteRequest.
 * @param srv    Server instance.
 * @param r      Request reader positioned after RequestTypeId.
 * @param w      Response writer.
 * @param req_id Request identifier.
 */
static void opcua_service_write(SYN_OPCUA_Server *srv, OPCUA_Reader *r, OPCUA_Writer *w,
                                uint32_t req_id)
{
    uint32_t count = 0U;
    if (!opcua_skip(r, 16U) || !opcua_read_u32(r, &count)) {
        opcua_write_service_fault(w, req_id, SYN_OPCUA_STATUS_BAD_DECODING_ERROR);
        return;
    }
    if (count == 0xFFFFFFFFU) {
        count = 0U;
    } else if (count > 16U) {
        count = 16U;
    }

    OPCUA_Reader scan = *r;
    OPCUA_WriteItem item;
    for (uint32_t i = 0U; i < count; i++) {
        if (!opcua_decode_write_item(&scan, &item)) {
            opcua_write_service_fault(w, req_id, SYN_OPCUA_STATUS_BAD_DECODING_ERROR);
            return;
        }
    }

    (void)opcua_write_nodeid_num(w, 0U, SYN_OPCUA_RESP_WRITE);
    (void)opcua_write_u64(w, syn_port_get_tick_ms());
    (void)opcua_write_u32(w, req_id);
    (void)opcua_write_u32(w, SYN_OPCUA_STATUS_GOOD);
    (void)opcua_write_u32(w, count);

    for (uint32_t i = 0U; i < count; i++) {
        (void)opcua_decode_write_item(r, &item);
        (void)opcua_write_u32(w, opcua_apply_write_item(srv, &item));
    }
}

/**
 * @brief Write numeric or string OPC UA NodeId.
 * @param w  Writer context.
 * @param id NodeId descriptor to encode.
 * @return True on success, false on overflow.
 */
static bool opcua_write_nodeid(OPCUA_Writer *w, const SYN_OPCUA_NodeId *id)
{
    if (id->id_type == SYN_OPCUA_NODEID_STRING) {
        (void)opcua_write_u8(w, 0x03U);
        (void)opcua_write_u16(w, id->ns_index);
        return opcua_write_string(w, id->id.str);
    }
    return opcua_write_nodeid_num(w, id->ns_index, id->id.num);
}

/**
 * @brief Check whether a NodeId identifies the standard RootFolder node.
 * @param id NodeId descriptor to inspect.
 * @return true if numeric RootFolder NodeId, false otherwise.
 */
static bool opcua_is_root_node(const SYN_OPCUA_NodeId *id)
{
    return (id->id_type == SYN_OPCUA_NODEID_NUMERIC) && (id->id.num == SYN_OPCUA_NODEID_ROOT);
}

/**
 * @brief Encode BrowseResponse listing registered non-root nodes.
 * @param srv    Server instance.
 * @param w      Response writer.
 * @param req_id Request identifier.
 */
static void opcua_service_browse(const SYN_OPCUA_Server *srv, OPCUA_Writer *w, uint32_t req_id)
{
    (void)opcua_write_nodeid_num(w, 0U, SYN_OPCUA_RESP_BROWSE);
    (void)opcua_write_u64(w, syn_port_get_tick_ms());
    (void)opcua_write_u32(w, req_id);
    (void)opcua_write_u32(w, SYN_OPCUA_STATUS_GOOD);

    (void)opcua_write_u32(w, 1U);
    (void)opcua_write_u32(w, SYN_OPCUA_STATUS_GOOD);
    (void)opcua_write_string(w, NULL);

    uint32_t ref_count = 0U;
    for (size_t i = 0U; i < srv->node_count; i++) {
        if (!opcua_is_root_node(&srv->nodes[i].node_id)) {
            ref_count++;
        }
    }
    (void)opcua_write_u32(w, ref_count);
    for (size_t i = 0U; i < srv->node_count; i++) {
        const SYN_OPCUA_Node *n = &srv->nodes[i];
        if (opcua_is_root_node(&n->node_id)) {
            continue;
        }
        (void)opcua_write_nodeid_num(w, 0U, 47U);
        (void)opcua_write_u8(w, 0U);
        (void)opcua_write_nodeid(w, &n->node_id);
        (void)opcua_write_string(w, n->browse_name);
        (void)opcua_write_string(w, n->display_name);
        (void)opcua_write_u32(w, (uint32_t)n->node_class);
        (void)opcua_write_nodeid_num(w, 0U, 0U);
    }
}

/**
 * @brief Dispatch session-level OPC UA service after SecureChannel validation.
 * @param srv OPC UA server instance.
 * @param req_type_id Numeric NodeId of the requested service type.
 * @param req_id Client request identifier.
 * @param r Request payload reader positioned after RequestTypeId.
 * @param w Binary response writer.
 */
static void opcua_dispatch_service(SYN_OPCUA_Server *srv, uint32_t req_type_id, uint32_t req_id,
                                   OPCUA_Reader *r, OPCUA_Writer *w)
{
    if (req_type_id == SYN_OPCUA_REQ_CREATE_SESSION) {
        (void)opcua_write_nodeid_num(w, 0U, SYN_OPCUA_RESP_CREATE_SESSION);
        (void)opcua_write_u64(w, syn_port_get_tick_ms());
        (void)opcua_write_u32(w, req_id);
        (void)opcua_write_u32(w, SYN_OPCUA_STATUS_GOOD);
        (void)opcua_write_nodeid_num(w, 1U, 1001U);
        (void)opcua_write_nodeid_num(w, 1U, 1002U);
        (void)opcua_write_u64(w, 3600000U);
        srv->state = SYN_OPCUA_STATE_SESSION_CREATED;
        return;
    }
    if (req_type_id == SYN_OPCUA_REQ_ACTIVATE_SESSION) {
        if (srv->state < SYN_OPCUA_STATE_SESSION_CREATED) {
            opcua_write_service_fault(w, req_id, SYN_OPCUA_STATUS_BAD_SESSION_ID_INVALID);
            return;
        }
        (void)opcua_write_nodeid_num(w, 0U, SYN_OPCUA_RESP_ACTIVATE_SESSION);
        (void)opcua_write_u64(w, syn_port_get_tick_ms());
        (void)opcua_write_u32(w, req_id);
        (void)opcua_write_u32(w, SYN_OPCUA_STATUS_GOOD);
        srv->state = SYN_OPCUA_STATE_SESSION_ACTIVE;
        return;
    }
    if (req_type_id == SYN_OPCUA_REQ_CLOSE_SESSION) {
        if (srv->state < SYN_OPCUA_STATE_SESSION_CREATED) {
            opcua_write_service_fault(w, req_id, SYN_OPCUA_STATUS_BAD_SESSION_ID_INVALID);
            return;
        }
        (void)opcua_write_nodeid_num(w, 0U, SYN_OPCUA_RESP_CLOSE_SESSION);
        (void)opcua_write_u64(w, syn_port_get_tick_ms());
        (void)opcua_write_u32(w, req_id);
        (void)opcua_write_u32(w, SYN_OPCUA_STATUS_GOOD);
        srv->state = SYN_OPCUA_STATE_SECURE_CHANNEL;
        return;
    }
    if ((req_type_id == SYN_OPCUA_REQ_READ) || (req_type_id == SYN_OPCUA_REQ_WRITE) ||
        (req_type_id == SYN_OPCUA_REQ_BROWSE)) {
        if (srv->state != SYN_OPCUA_STATE_SESSION_ACTIVE) {
            opcua_write_service_fault(w, req_id, SYN_OPCUA_STATUS_BAD_SESSION_NOT_ACTIVATED);
            return;
        }
        if (req_type_id == SYN_OPCUA_REQ_READ) {
            opcua_service_read(srv, r, w, req_id);
        } else if (req_type_id == SYN_OPCUA_REQ_WRITE) {
            opcua_service_write(srv, r, w, req_id);
        } else {
            opcua_service_browse(srv, w, req_id);
        }
        return;
    }
    (void)opcua_write_nodeid_num(w, 0U, req_type_id + 3U);
    (void)opcua_write_u64(w, syn_port_get_tick_ms());
    (void)opcua_write_u32(w, req_id);
    (void)opcua_write_u32(w, SYN_OPCUA_STATUS_BAD_SERVICE_UNSUPPORTED);
}

/**
 * @brief Handle UASC Service Message (MSGF) and encode matching service response.
 * @param srv     Server instance.
 * @param r       Request reader.
 * @param w       Response writer.
 * @param out_len Output response length.
 * @return SYN_OK on success, SYN_ERROR on invalid channel/token or buffer overflow.
 */
static SYN_Status opcua_handle_msg(SYN_OPCUA_Server *srv, OPCUA_Reader *r, OPCUA_Writer *w,
                                   size_t *out_len)
{
    uint32_t channel_id = 0U;
    uint32_t token_id = 0U;
    uint32_t seq_num = 0U;
    uint32_t req_id = 0U;
    uint16_t req_ns = 0U;
    uint32_t req_type_id = 0U;
    if (!opcua_read_u32(r, &channel_id) || !opcua_read_u32(r, &token_id) ||
        !opcua_read_u32(r, &seq_num) || !opcua_read_u32(r, &req_id) ||
        !opcua_read_nodeid_num(r, &req_ns, &req_type_id)) {
        return SYN_ERROR;
    }
    (void)seq_num;
    (void)req_ns;

    if ((srv->state < SYN_OPCUA_STATE_SECURE_CHANNEL) || (channel_id != srv->secure_channel_id) ||
        (token_id != srv->security_token_id)) {
        return SYN_ERROR;
    }

    (void)opcua_write_bytes(w, (const uint8_t *)"MSGF", 4U);
    size_t len_pos = w->pos;
    (void)opcua_write_u32(w, 0U);
    (void)opcua_write_u32(w, srv->secure_channel_id);
    (void)opcua_write_u32(w, srv->security_token_id);
    (void)opcua_write_u32(w, srv->sequence_number++);
    (void)opcua_write_u32(w, req_id);

    opcua_dispatch_service(srv, req_type_id, req_id, r, w);

    if (w->error) {
        return SYN_ERROR;
    }

    opcua_patch_u32_len(w, len_pos);
    *out_len = w->pos;
    return SYN_OK;
}

SYN_Status syn_opcua_server_process_message(SYN_OPCUA_Server *srv, const uint8_t *rx, size_t rx_len,
                                            uint8_t *tx, size_t max_tx, size_t *out_len)
{
    if (srv == NULL || rx == NULL || tx == NULL || out_len == NULL || rx_len < 8U) {
        return SYN_INVALID_PARAM;
    }

    *out_len = 0U;
    OPCUA_Reader r;
    opcua_reader_init(&r, rx, rx_len);
    r.pos = 4U;

    uint32_t message_size = 0U;
    (void)opcua_read_u32(&r, &message_size);
    if (message_size < 8U || (size_t)message_size > rx_len) {
        return SYN_INVALID_PARAM;
    }
    r.size = (size_t)message_size;

    OPCUA_Writer w;
    opcua_writer_init(&w, tx, max_tx);

    if (memcmp(rx, "HELF", 4U) == 0) {
        return opcua_handle_hel(srv, &w, out_len);
    }
    if (memcmp(rx, "OPNF", 4U) == 0) {
        return opcua_handle_opn(srv, &r, &w, out_len);
    }
    if (memcmp(rx, "MSGF", 4U) == 0) {
        return opcua_handle_msg(srv, &r, &w, out_len);
    }
    if (memcmp(rx, "CLOF", 4U) == 0) {
        srv->state = SYN_OPCUA_STATE_CLOSED;
        return SYN_OK;
    }

    return SYN_ERROR;
}

/**
 * @brief Dispatch all complete OPC UA frames buffered in srv->cfg.rx_buf and retain partial tail.
 * @param srv OPC UA server instance.
 */
static void opcua_drain_rx_frames(SYN_OPCUA_Server *srv)
{
    size_t offset = 0U;
    while (srv->rx_pending - offset >= 8U) {
        const uint8_t *frame = &srv->cfg.rx_buf[offset];
        uint32_t msg_size = (uint32_t)frame[4] | ((uint32_t)frame[5] << 8U) |
                            ((uint32_t)frame[6] << 16U) | ((uint32_t)frame[7] << 24U);
        if (msg_size < 8U || (size_t)msg_size > srv->cfg.rx_buf_size) {
            offset = srv->rx_pending;
            break;
        }
        if ((size_t)msg_size > srv->rx_pending - offset) {
            break;
        }

        size_t tx_len = 0U;
        SYN_Status st = syn_opcua_server_process_message(
            srv, frame, (size_t)msg_size, srv->cfg.tx_buf, srv->cfg.tx_buf_size, &tx_len);
        if (st == SYN_OK && tx_len > 0U) {
            (void)syn_transport_send(srv->cfg.transport, srv->cfg.tx_buf, tx_len);
        }
        offset += (size_t)msg_size;
    }

    if (offset > 0U) {
        size_t rem = srv->rx_pending - offset;
        if (rem > 0U) {
            (void)memmove(srv->cfg.rx_buf, &srv->cfg.rx_buf[offset], rem);
        }
        srv->rx_pending = rem;
    }
}

/**
 * @brief Step the OPC UA server once and report whether an inbound packet was consumed.
 * @param srv             OPC UA server instance.
 * @param now_ms          Current system timestamp in milliseconds.
 * @param out_rx_consumed [out] Optional flag set true if a non-empty packet was read.
 * @return SYN_OK on success, SYN_INVALID_PARAM on NULL server or transport.
 */
static SYN_Status opcua_server_step_ex(SYN_OPCUA_Server *srv, uint32_t now_ms,
                                       bool *out_rx_consumed)
{
    (void)now_ms;
    if (out_rx_consumed != NULL) {
        *out_rx_consumed = false;
    }
    if (srv == NULL || srv->cfg.transport == NULL) {
        return SYN_INVALID_PARAM;
    }

    size_t avail = srv->cfg.rx_buf_size - srv->rx_pending;
    size_t rx_len = 0U;
    if (syn_transport_recv(srv->cfg.transport, &srv->cfg.rx_buf[srv->rx_pending], avail, &rx_len) &&
        rx_len > 0U) {
        if (out_rx_consumed != NULL) {
            *out_rx_consumed = true;
        }
        srv->rx_pending += rx_len;
        opcua_drain_rx_frames(srv);
    }
    return SYN_OK;
}

SYN_Status syn_opcua_server_step(SYN_OPCUA_Server *srv, uint32_t now_ms)
{
    return opcua_server_step_ex(srv, now_ms, NULL);
}

SYN_PT_Status syn_opcua_server_pt(SYN_PT *pt, SYN_Task *task)
{
    if (pt == NULL || task == NULL || task->user_data == NULL) {
        return PT_EXITED;
    }

    SYN_OPCUA_Server *srv = (SYN_OPCUA_Server *)task->user_data;

    PT_BEGIN(pt);

    for (;;) {
        bool rx_consumed = false;
        uint32_t now = syn_port_get_tick_ms();
        if (opcua_server_step_ex(srv, now, &rx_consumed) != SYN_OK) {
            break;
        }
        if (rx_consumed && srv->cfg.transport != NULL && srv->cfg.transport->has_data != NULL &&
            syn_transport_has_data(srv->cfg.transport)) {
            PT_CONTINUE_IF_NEEDED(pt, task);
        } else {
            PT_PASS(pt);
        }
    }

    PT_END(pt);
}

#endif /* SYN_USE_OPCUA */
