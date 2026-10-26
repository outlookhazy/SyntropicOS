

# File syn\_lwm2m.c

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_lwm2m.c**](syn__lwm2m_8c.md)

[Go to the documentation of this file](syn__lwm2m_8c.md)


```C++

#include "syn_lwm2m.h"

#if !defined(SYN_USE_LWM2M) || SYN_USE_LWM2M

#include "syntropic/port/syn_port_system.h"
#include "syntropic/util/syn_assert.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ── TLV (Type-Length-Value) Binary Codec ─────────────────────────────────── */

size_t syn_lwm2m_tlv_encode(const SYN_LwM2M_TLV *tlv, uint8_t *buf, size_t max_len)
{
    if (tlv == NULL || buf == NULL) {
        return 0U;
    }

    size_t id_len = (tlv->id > 0xFFU) ? 2U : 1U;
    size_t len_bytes = 0U;
    uint8_t len_type_bits = 0U;
    uint8_t direct_len_bits = 0U;

    if (tlv->len <= 7U) {
        len_bytes = 0U;
        len_type_bits = 0x00U;
        direct_len_bits = (uint8_t)tlv->len;
    } else if (tlv->len <= 0xFFU) {
        len_bytes = 1U;
        len_type_bits = 0x08U;
    } else if (tlv->len <= 0xFFFFU) {
        len_bytes = 2U;
        len_type_bits = 0x10U;
    } else {
        len_bytes = 3U;
        len_type_bits = 0x18U;
    }

    size_t total_header_len = 1U + id_len + len_bytes;
    if (total_header_len + tlv->len > max_len) {
        return 0U;
    }

    uint8_t hdr = (uint8_t)(tlv->type & 0xC0U);
    if (id_len == 2U) {
        hdr |= 0x20U;
    }
    hdr |= len_type_bits;
    hdr |= direct_len_bits;

    size_t offset = 0U;
    buf[offset++] = hdr;

    if (id_len == 1U) {
        buf[offset++] = (uint8_t)(tlv->id & 0xFFU);
    } else {
        buf[offset++] = (uint8_t)((tlv->id >> 8U) & 0xFFU);
        buf[offset++] = (uint8_t)(tlv->id & 0xFFU);
    }

    if (len_bytes == 1U) {
        buf[offset++] = (uint8_t)(tlv->len & 0xFFU);
    } else if (len_bytes == 2U) {
        buf[offset++] = (uint8_t)((tlv->len >> 8U) & 0xFFU);
        buf[offset++] = (uint8_t)(tlv->len & 0xFFU);
    } else if (len_bytes == 3U) {
        buf[offset++] = (uint8_t)((tlv->len >> 16U) & 0xFFU);
        buf[offset++] = (uint8_t)((tlv->len >> 8U) & 0xFFU);
        buf[offset++] = (uint8_t)(tlv->len & 0xFFU);
    }

    if (tlv->len > 0U && tlv->val != NULL) {
        (void)memcpy(buf + offset, tlv->val, tlv->len);
        offset += tlv->len;
    }

    return offset;
}

size_t syn_lwm2m_tlv_encode_int(uint16_t res_id, int64_t val, uint8_t *buf, size_t max_len)
{
    uint8_t val_buf[8];
    size_t val_len = 0U;

    if (val >= -128 && val <= 127) {
        val_buf[0] = (uint8_t)((int8_t)val);
        val_len = 1U;
    } else if (val >= -32768 && val <= 32767) {
        int16_t v16 = (int16_t)val;
        val_buf[0] = (uint8_t)((v16 >> 8) & 0xFF);
        val_buf[1] = (uint8_t)(v16 & 0xFF);
        val_len = 2U;
    } else if (val >= -2147483648LL && val <= 2147483647LL) {
        int32_t v32 = (int32_t)val;
        val_buf[0] = (uint8_t)((v32 >> 24) & 0xFF);
        val_buf[1] = (uint8_t)((v32 >> 16) & 0xFF);
        val_buf[2] = (uint8_t)((v32 >> 8) & 0xFF);
        val_buf[3] = (uint8_t)(v32 & 0xFF);
        val_len = 4U;
    } else {
        uint64_t u64 = (uint64_t)val;
        for (size_t i = 0U; i < 8U; i++) {
            val_buf[7U - i] = (uint8_t)((u64 >> (i * 8U)) & 0xFFU);
        }
        val_len = 8U;
    }

    SYN_LwM2M_TLV tlv = {
        .type = SYN_LWM2M_TLV_RESOURCE,
        .id = res_id,
        .val = val_buf,
        .len = val_len,
    };
    return syn_lwm2m_tlv_encode(&tlv, buf, max_len);
}

size_t syn_lwm2m_tlv_encode_float(uint16_t res_id, double val, uint8_t *buf, size_t max_len)
{
    float f32 = (float)val;
    uint8_t val_buf[4];
    uint32_t u32 = 0U;
    (void)memcpy(&u32, &f32, sizeof(u32));
    val_buf[0] = (uint8_t)((u32 >> 24) & 0xFF);
    val_buf[1] = (uint8_t)((u32 >> 16) & 0xFF);
    val_buf[2] = (uint8_t)((u32 >> 8) & 0xFF);
    val_buf[3] = (uint8_t)(u32 & 0xFF);

    SYN_LwM2M_TLV tlv = {
        .type = SYN_LWM2M_TLV_RESOURCE,
        .id = res_id,
        .val = val_buf,
        .len = sizeof(val_buf),
    };
    return syn_lwm2m_tlv_encode(&tlv, buf, max_len);
}

size_t syn_lwm2m_tlv_encode_bool(uint16_t res_id, bool val, uint8_t *buf, size_t max_len)
{
    uint8_t b_val = val ? 1U : 0U;
    SYN_LwM2M_TLV tlv = {
        .type = SYN_LWM2M_TLV_RESOURCE,
        .id = res_id,
        .val = &b_val,
        .len = 1U,
    };
    return syn_lwm2m_tlv_encode(&tlv, buf, max_len);
}

size_t syn_lwm2m_tlv_encode_string(uint16_t res_id, const char *str, uint8_t *buf, size_t max_len)
{
    if (str == NULL) {
        return 0U;
    }
    SYN_LwM2M_TLV tlv = {
        .type = SYN_LWM2M_TLV_RESOURCE,
        .id = res_id,
        .val = (const uint8_t *)str,
        .len = strlen(str),
    };
    return syn_lwm2m_tlv_encode(&tlv, buf, max_len);
}

size_t syn_lwm2m_tlv_encode_opaque(uint16_t res_id, const uint8_t *data, size_t len, uint8_t *buf,
                                   size_t max_len)
{
    SYN_LwM2M_TLV tlv = {
        .type = SYN_LWM2M_TLV_RESOURCE,
        .id = res_id,
        .val = data,
        .len = len,
    };
    return syn_lwm2m_tlv_encode(&tlv, buf, max_len);
}

size_t syn_lwm2m_tlv_encode_value(uint16_t res_id, const SYN_LwM2M_Value *val, uint8_t *buf,
                                  size_t max_len)
{
    if (val == NULL) {
        return 0U;
    }

    switch (val->type) {
    case SYN_LWM2M_TYPE_INT:
    case SYN_LWM2M_TYPE_TIME:
        return syn_lwm2m_tlv_encode_int(res_id, val->val.integer, buf, max_len);
    case SYN_LWM2M_TYPE_FLOAT:
        return syn_lwm2m_tlv_encode_float(res_id, val->val.floating, buf, max_len);
    case SYN_LWM2M_TYPE_BOOL:
        return syn_lwm2m_tlv_encode_bool(res_id, val->val.boolean, buf, max_len);
    case SYN_LWM2M_TYPE_STRING:
        return syn_lwm2m_tlv_encode_string(res_id, val->val.str, buf, max_len);
    case SYN_LWM2M_TYPE_OPAQUE:
        return syn_lwm2m_tlv_encode_opaque(res_id, val->val.opaque.data, val->val.opaque.len, buf,
                                           max_len);
    case SYN_LWM2M_TYPE_OBJLNK: {
        uint8_t link_buf[4];
        link_buf[0] = (uint8_t)((val->val.objlnk.obj_id >> 8) & 0xFF);
        link_buf[1] = (uint8_t)(val->val.objlnk.obj_id & 0xFF);
        link_buf[2] = (uint8_t)((val->val.objlnk.inst_id >> 8) & 0xFF);
        link_buf[3] = (uint8_t)(val->val.objlnk.inst_id & 0xFF);
        SYN_LwM2M_TLV tlv = {
            .type = SYN_LWM2M_TLV_RESOURCE,
            .id = res_id,
            .val = link_buf,
            .len = 4U,
        };
        return syn_lwm2m_tlv_encode(&tlv, buf, max_len);
    }
    default:
        return 0U;
    }
}

size_t syn_lwm2m_tlv_encode_instance(uint16_t inst_id, const uint8_t *tlv_payload,
                                     size_t payload_len, uint8_t *buf, size_t max_len)
{
    SYN_LwM2M_TLV tlv = {
        .type = SYN_LWM2M_TLV_OBJECT_INSTANCE,
        .id = inst_id,
        .val = tlv_payload,
        .len = payload_len,
    };
    return syn_lwm2m_tlv_encode(&tlv, buf, max_len);
}

bool syn_lwm2m_tlv_decode(const uint8_t *buf, size_t buf_len, SYN_LwM2M_TLV *tlv, size_t *consumed)
{
    if (buf == NULL || tlv == NULL || consumed == NULL || buf_len < 2U) {
        return false;
    }

    uint8_t hdr = buf[0];
    tlv->type = (SYN_LwM2M_TLV_Type)(hdr & 0xC0U);
    bool id_16bit = (hdr & 0x20U) != 0U;
    uint8_t len_type = (hdr & 0x18U) >> 3U;
    uint8_t direct_len = hdr & 0x07U;

    size_t offset = 1U;
    if (id_16bit) {
        if (offset + 2U > buf_len) {
            return false;
        }
        tlv->id = ((uint16_t)buf[offset] << 8U) | (uint16_t)buf[offset + 1U];
        offset += 2U;
    } else {
        tlv->id = (uint16_t)buf[offset++];
    }

    size_t val_len = 0U;
    if (len_type == 0U) {
        val_len = direct_len;
    } else if (len_type == 1U) {
        if (offset + 1U > buf_len) {
            return false;
        }
        val_len = (size_t)buf[offset++];
    } else if (len_type == 2U) {
        if (offset + 2U > buf_len) {
            return false;
        }
        val_len = ((size_t)buf[offset] << 8U) | (size_t)buf[offset + 1U];
        offset += 2U;
    } else {
        if (offset + 3U > buf_len) {
            return false;
        }
        val_len = ((size_t)buf[offset] << 16U) | ((size_t)buf[offset + 1U] << 8U) |
                  (size_t)buf[offset + 2U];
        offset += 3U;
    }

    if (offset + val_len > buf_len) {
        return false;
    }

    tlv->val = buf + offset;
    tlv->len = val_len;
    *consumed = offset + val_len;
    return true;
}

bool syn_lwm2m_tlv_decode_int(const SYN_LwM2M_TLV *tlv, int64_t *out_val)
{
    if (tlv == NULL || out_val == NULL || tlv->len == 0U || tlv->len > 8U || tlv->val == NULL) {
        return false;
    }

    uint64_t uval = 0U;
    for (size_t i = 0U; i < tlv->len; i++) {
        uval = (uval << 8U) | (uint64_t)tlv->val[i];
    }

    if (tlv->len < 8U && (tlv->val[0] & 0x80U) != 0U) {
        uint64_t mask = ~0ULL << (tlv->len * 8U);
        uval |= mask;
    }

    int64_t val = 0;
    (void)memcpy(&val, &uval, sizeof(val));
    *out_val = val;
    return true;
}

bool syn_lwm2m_tlv_decode_float(const SYN_LwM2M_TLV *tlv, double *out_val)
{
    if (tlv == NULL || out_val == NULL || tlv->val == NULL) {
        return false;
    }

    if (tlv->len == 4U) {
        uint32_t u32 = ((uint32_t)tlv->val[0] << 24U) | ((uint32_t)tlv->val[1] << 16U) |
                       ((uint32_t)tlv->val[2] << 8U) | (uint32_t)tlv->val[3];
        float f32 = 0.0f;
        (void)memcpy(&f32, &u32, sizeof(f32));
        *out_val = (double)f32;
        return true;
    }

    if (tlv->len == 8U) {
        uint64_t u64 = 0U;
        for (size_t i = 0U; i < 8U; i++) {
            u64 = (u64 << 8U) | (uint64_t)tlv->val[i];
        }
        double d64 = 0.0;
        (void)memcpy(&d64, &u64, sizeof(d64));
        *out_val = d64;
        return true;
    }

    return false;
}

bool syn_lwm2m_tlv_decode_bool(const SYN_LwM2M_TLV *tlv, bool *out_val)
{
    if (tlv == NULL || out_val == NULL || tlv->len != 1U || tlv->val == NULL) {
        return false;
    }
    *out_val = (tlv->val[0] != 0U);
    return true;
}

bool syn_lwm2m_tlv_decode_string(const SYN_LwM2M_TLV *tlv, char *out_str, size_t max_len)
{
    if (tlv == NULL || out_str == NULL || max_len == 0U) {
        return false;
    }
    if (tlv->len >= max_len) {
        return false;
    }
    if (tlv->len > 0U && tlv->val != NULL) {
        (void)memcpy(out_str, tlv->val, tlv->len);
    }
    out_str[tlv->len] = '\0';
    return true;
}

bool syn_lwm2m_tlv_decode_value(const SYN_LwM2M_TLV *tlv, SYN_LwM2M_ValType expected_type,
                                SYN_LwM2M_Value *out_val)
{
    if (tlv == NULL || out_val == NULL) {
        return false;
    }
    out_val->type = expected_type;

    switch (expected_type) {
    case SYN_LWM2M_TYPE_INT:
    case SYN_LWM2M_TYPE_TIME:
        return syn_lwm2m_tlv_decode_int(tlv, &out_val->val.integer);
    case SYN_LWM2M_TYPE_FLOAT:
        return syn_lwm2m_tlv_decode_float(tlv, &out_val->val.floating);
    case SYN_LWM2M_TYPE_BOOL:
        return syn_lwm2m_tlv_decode_bool(tlv, &out_val->val.boolean);
    case SYN_LWM2M_TYPE_OPAQUE:
        out_val->val.opaque.data = tlv->val;
        out_val->val.opaque.len = tlv->len;
        return true;
    case SYN_LWM2M_TYPE_OBJLNK:
        if (tlv->len == 4U && tlv->val != NULL) {
            out_val->val.objlnk.obj_id = ((uint16_t)tlv->val[0] << 8U) | (uint16_t)tlv->val[1];
            out_val->val.objlnk.inst_id = ((uint16_t)tlv->val[2] << 8U) | (uint16_t)tlv->val[3];
            return true;
        }
        return false;
    default:
        return false;
    }
}

/* ── Core Link Format & Registration ─────────────────────────────────────── */

size_t syn_lwm2m_generate_link_format(const SYN_LwM2M_Client *client, char *buf, size_t max_len)
{
    if (client == NULL || buf == NULL || max_len == 0U) {
        return 0U;
    }

    size_t written = 0U;
    buf[0] = '\0';

    for (size_t i = 0U; i < client->object_count; i++) {
        const SYN_LwM2M_Object *obj = client->objects[i];
        if (obj == NULL) {
            continue;
        }

        for (size_t j = 0U; j < obj->instance_count; j++) {
            uint16_t inst_id = (obj->instances != NULL) ? obj->instances[j] : (uint16_t)j;
            char link[32];
            int n = 0;
            if (written == 0U) {
                n = snprintf(link, sizeof(link), "</%u/%u>", obj->id, inst_id);
            } else {
                n = snprintf(link, sizeof(link), ",</%u/%u>", obj->id, inst_id);
            }

            if (n < 0 || (size_t)n >= sizeof(link)) {
                return 0U;
            }

            if (written + (size_t)n >= max_len) {
                return 0U;
            }

            (void)memcpy(buf + written, link, (size_t)n);
            written += (size_t)n;
            buf[written] = '\0';
        }
    }

    return written;
}

bool syn_lwm2m_client_init(SYN_LwM2M_Client *client, const char *endpoint_name, uint32_t lifetime_s,
                           SYN_Transport *transport)
{
    if (client == NULL || endpoint_name == NULL || transport == NULL || lifetime_s == 0U) {
        return false;
    }

    (void)memset(client, 0, sizeof(*client));
    client->endpoint_name = endpoint_name;
    client->lifetime_s = lifetime_s;
    client->binding = "U";
    client->transport = transport;
    client->state = SYN_LWM2M_STATE_DEREGISTERED;

    return true;
}

bool syn_lwm2m_register_object(SYN_LwM2M_Client *client, const SYN_LwM2M_Object *obj)
{
    if (client == NULL || obj == NULL) {
        return false;
    }
    if (client->object_count >= SYN_LWM2M_MAX_OBJECTS) {
        return false;
    }

    client->objects[client->object_count++] = obj;
    return true;
}

size_t syn_lwm2m_build_register_request(const SYN_LwM2M_Client *client, uint16_t msg_id,
                                        const uint8_t *token, uint8_t token_len, SYN_CoapMsg *req,
                                        SYN_CoapOption *req_opts, size_t max_req_opts,
                                        size_t *req_opt_cnt, uint8_t *payload_buf,
                                        size_t payload_buf_sz)
{
    if (client == NULL || req == NULL || req_opts == NULL || req_opt_cnt == NULL ||
        payload_buf == NULL || max_req_opts < 6U || token_len > 8U) {
        return 0U;
    }

    size_t link_len = syn_lwm2m_generate_link_format(client, (char *)payload_buf, payload_buf_sz);
    if (link_len == 0U) {
        return 0U;
    }

    req->type = COAP_TYPE_CON;
    req->code = COAP_CODE_POST;
    req->msg_id = msg_id;
    req->token_len = token_len;
    if (token_len > 0U && token != NULL) {
        (void)memcpy(req->token, token, token_len);
    }
    req->payload = payload_buf;
    req->payload_len = link_len;

    /* Options: Uri-Path: "rd", Uri-Query: "ep=...", Uri-Query: "lt=...", Uri-Query: "lwm2m=1.1",
     * Uri-Query: "b=U", Content-Format: 40 */
    static char query_ep[64];
    static char query_lt[32];
    (void)snprintf(query_ep, sizeof(query_ep), "ep=%s", client->endpoint_name);
    (void)snprintf(query_lt, sizeof(query_lt), "lt=%u", client->lifetime_s);

    static const uint8_t fmt_link = SYN_LWM2M_FORMAT_LINK_FORMAT;

    req_opts[0].num = COAP_OPT_URI_PATH;
    req_opts[0].val = (const uint8_t *)"rd";
    req_opts[0].len = 2U;

    req_opts[1].num = COAP_OPT_CONTENT_FORMAT;
    req_opts[1].val = &fmt_link;
    req_opts[1].len = 1U;

    req_opts[2].num = COAP_OPT_URI_QUERY;
    req_opts[2].val = (const uint8_t *)query_ep;
    req_opts[2].len = strlen(query_ep);

    req_opts[3].num = COAP_OPT_URI_QUERY;
    req_opts[3].val = (const uint8_t *)query_lt;
    req_opts[3].len = strlen(query_lt);

    req_opts[4].num = COAP_OPT_URI_QUERY;
    req_opts[4].val = (const uint8_t *)"lwm2m=1.1";
    req_opts[4].len = 9U;

    req_opts[5].num = COAP_OPT_URI_QUERY;
    req_opts[5].val = (const uint8_t *)"b=U";
    req_opts[5].len = 3U;

    *req_opt_cnt = 6U;
    return link_len;
}

size_t syn_lwm2m_build_update_request(const SYN_LwM2M_Client *client, uint16_t msg_id,
                                      const uint8_t *token, uint8_t token_len, uint32_t lifetime_s,
                                      SYN_CoapMsg *req, SYN_CoapOption *req_opts,
                                      size_t max_req_opts, size_t *req_opt_cnt,
                                      uint8_t *payload_buf, size_t payload_buf_sz)
{
    (void)payload_buf;
    (void)payload_buf_sz;

    if (client == NULL || req == NULL || req_opts == NULL || req_opt_cnt == NULL ||
        max_req_opts < 3U || token_len > 8U || client->location_path[0] == '\0') {
        return 0U;
    }

    req->type = COAP_TYPE_CON;
    req->code = COAP_CODE_POST;
    req->msg_id = msg_id;
    req->token_len = token_len;
    if (token_len > 0U && token != NULL) {
        (void)memcpy(req->token, token, token_len);
    }
    req->payload = NULL;
    req->payload_len = 0U;

    const char *loc = client->location_path;
    if (loc[0] == '/') {
        loc++;
    }

    req_opts[0].num = COAP_OPT_URI_PATH;
    req_opts[0].val = (const uint8_t *)"rd";
    req_opts[0].len = 2U;

    const char *slash = strchr(loc, '/');
    const char *id_part = (slash != NULL) ? (slash + 1) : loc;

    req_opts[1].num = COAP_OPT_URI_PATH;
    req_opts[1].val = (const uint8_t *)id_part;
    req_opts[1].len = strlen(id_part);

    static char query_lt[32];
    uint32_t lt = (lifetime_s > 0U) ? lifetime_s : client->lifetime_s;
    (void)snprintf(query_lt, sizeof(query_lt), "lt=%u", lt);

    req_opts[2].num = COAP_OPT_URI_QUERY;
    req_opts[2].val = (const uint8_t *)query_lt;
    req_opts[2].len = strlen(query_lt);

    *req_opt_cnt = 3U;
    return 1U;
}

size_t syn_lwm2m_build_deregister_request(const SYN_LwM2M_Client *client, uint16_t msg_id,
                                          const uint8_t *token, uint8_t token_len, SYN_CoapMsg *req,
                                          SYN_CoapOption *req_opts, size_t max_req_opts,
                                          size_t *req_opt_cnt)
{
    if (client == NULL || req == NULL || req_opts == NULL || req_opt_cnt == NULL ||
        max_req_opts < 2U || token_len > 8U || client->location_path[0] == '\0') {
        return 0U;
    }

    req->type = COAP_TYPE_CON;
    req->code = COAP_CODE_DELETE;
    req->msg_id = msg_id;
    req->token_len = token_len;
    if (token_len > 0U && token != NULL) {
        (void)memcpy(req->token, token, token_len);
    }
    req->payload = NULL;
    req->payload_len = 0U;

    const char *loc = client->location_path;
    if (loc[0] == '/') {
        loc++;
    }

    req_opts[0].num = COAP_OPT_URI_PATH;
    req_opts[0].val = (const uint8_t *)"rd";
    req_opts[0].len = 2U;

    const char *slash = strchr(loc, '/');
    const char *id_part = (slash != NULL) ? (slash + 1) : loc;

    req_opts[1].num = COAP_OPT_URI_PATH;
    req_opts[1].val = (const uint8_t *)id_part;
    req_opts[1].len = strlen(id_part);

    *req_opt_cnt = 2U;
    return 1U;
}

bool syn_lwm2m_handle_register_response(SYN_LwM2M_Client *client, const SYN_CoapMsg *resp,
                                        const SYN_CoapOption *resp_opts, size_t resp_opt_cnt)
{
    if (client == NULL || resp == NULL) {
        return false;
    }

    if (resp->code != COAP_RESP_CREATED) {
        return false;
    }

    /* Extract Location-Path options */
    char loc_buf[SYN_LWM2M_MAX_URI_PATH_LEN] = {0};
    size_t loc_len = 0U;

    if (resp_opts != NULL) {
        for (size_t i = 0U; i < resp_opt_cnt; i++) {
            if (resp_opts[i].num == COAP_OPT_LOCATION_PATH && resp_opts[i].val != NULL) {
                if (loc_len > 0U && loc_len + 1U < sizeof(loc_buf)) {
                    loc_buf[loc_len++] = '/';
                }
                size_t copy_len = resp_opts[i].len;
                if (loc_len + copy_len < sizeof(loc_buf)) {
                    (void)memcpy(loc_buf + loc_len, resp_opts[i].val, copy_len);
                    loc_len += copy_len;
                }
            }
        }
    }

    if (loc_len > 0U) {
        loc_buf[loc_len] = '\0';
        (void)strncpy(client->location_path, loc_buf, sizeof(client->location_path) - 1U);
    } else {
        (void)strncpy(client->location_path, "rd/0", sizeof(client->location_path) - 1U);
    }

    client->state = SYN_LWM2M_STATE_REGISTERED;
    client->last_update_ms = syn_port_get_tick_ms();
    return true;
}

/* ── Observation Table ───────────────────────────────────────────────────── */

bool syn_lwm2m_observe_add(SYN_LwM2M_Client *client, uint16_t obj_id, uint16_t inst_id,
                           int32_t res_id, const uint8_t *token, uint8_t token_len, uint32_t pmin,
                           uint32_t pmax)
{
    if (client == NULL || token_len > 8U) {
        return false;
    }

    /* Search for existing matching observation to update */
    int free_slot = -1;
    for (size_t i = 0U; i < SYN_LWM2M_MAX_OBSERVERS; i++) {
        if (client->observations[i].active) {
            if (client->observations[i].token_len == token_len &&
                memcmp(client->observations[i].token, token, token_len) == 0) {
                free_slot = (int)i;
                break;
            }
        } else if (free_slot < 0) {
            free_slot = (int)i;
        }
    }

    if (free_slot < 0) {
        return false;
    }

    SYN_LwM2M_Observation *obs = &client->observations[free_slot];
    obs->active = true;
    obs->obj_id = obj_id;
    obs->inst_id = inst_id;
    obs->res_id = res_id;
    obs->token_len = token_len;
    if (token_len > 0U && token != NULL) {
        (void)memcpy(obs->token, token, token_len);
    }
    obs->pmin = pmin;
    obs->pmax = (pmax > 0U) ? pmax : 60000U;
    obs->last_notify_ms = syn_port_get_tick_ms();
    obs->seq = 0U;

    return true;
}

bool syn_lwm2m_observe_remove(SYN_LwM2M_Client *client, const uint8_t *token, uint8_t token_len)
{
    if (client == NULL || token_len > 8U) {
        return false;
    }

    for (size_t i = 0U; i < SYN_LWM2M_MAX_OBSERVERS; i++) {
        if (client->observations[i].active && client->observations[i].token_len == token_len) {
            if (token_len == 0U || memcmp(client->observations[i].token, token, token_len) == 0) {
                client->observations[i].active = false;
                return true;
            }
        }
    }

    return false;
}

size_t syn_lwm2m_build_notify(SYN_LwM2M_Client *client, uint16_t obj_id, uint16_t inst_id,
                              int32_t res_id, uint16_t msg_id, SYN_CoapMsg *resp,
                              SYN_CoapOption *resp_opts, size_t max_resp_opts, size_t *resp_opt_cnt,
                              uint8_t *resp_buf, size_t resp_buf_sz)
{
    if (client == NULL || resp == NULL || resp_opts == NULL || resp_opt_cnt == NULL ||
        resp_buf == NULL || max_resp_opts < 2U || resp_buf_sz == 0U) {
        return 0U;
    }

    /* Find matching observation */
    SYN_LwM2M_Observation *obs = NULL;
    for (size_t i = 0U; i < SYN_LWM2M_MAX_OBSERVERS; i++) {
        if (client->observations[i].active && client->observations[i].obj_id == obj_id &&
            client->observations[i].inst_id == inst_id &&
            client->observations[i].res_id == res_id) {
            obs = &client->observations[i];
            break;
        }
    }

    if (obs == NULL) {
        return 0U;
    }

    /* Find target object and resource */
    const SYN_LwM2M_Object *obj = NULL;
    for (size_t i = 0U; i < client->object_count; i++) {
        if (client->objects[i] != NULL && client->objects[i]->id == obj_id) {
            obj = client->objects[i];
            break;
        }
    }

    if (obj == NULL || obj->read == NULL || res_id < 0) {
        return 0U;
    }

    SYN_LwM2M_Value val = {0};
    if (obj->read(inst_id, (uint16_t)res_id, &val, obj->user_ctx) != SYN_OK) {
        return 0U;
    }

    size_t payload_len = syn_lwm2m_tlv_encode_value((uint16_t)res_id, &val, resp_buf, resp_buf_sz);
    if (payload_len == 0U) {
        return 0U;
    }

    resp->type = COAP_TYPE_NON;
    resp->code = COAP_RESP_CONTENT;
    resp->msg_id = msg_id;
    resp->token_len = obs->token_len;
    (void)memcpy(resp->token, obs->token, obs->token_len);
    resp->payload = resp_buf;
    resp->payload_len = payload_len;

    static uint8_t obs_seq_buf[3];
    obs->seq++;
    obs_seq_buf[0] = (uint8_t)((obs->seq >> 16) & 0xFF);
    obs_seq_buf[1] = (uint8_t)((obs->seq >> 8) & 0xFF);
    obs_seq_buf[2] = (uint8_t)(obs->seq & 0xFF);

    static const uint8_t fmt_tlv[2] = {(uint8_t)(SYN_LWM2M_FORMAT_TLV >> 8),
                                       (uint8_t)(SYN_LWM2M_FORMAT_TLV & 0xFF)};

    resp_opts[0].num = COAP_OPT_OBSERVE;
    resp_opts[0].val = obs_seq_buf;
    resp_opts[0].len = 3U;

    resp_opts[1].num = COAP_OPT_CONTENT_FORMAT;
    resp_opts[1].val = fmt_tlv;
    resp_opts[1].len = 2U;

    *resp_opt_cnt = 2U;
    obs->last_notify_ms = syn_port_get_tick_ms();

    return payload_len;
}

/* ── Request Dispatcher ──────────────────────────────────────────────────── */

static const SYN_LwM2M_Object *find_object(const SYN_LwM2M_Client *client, uint16_t obj_id)
{
    for (size_t i = 0U; i < client->object_count; i++) {
        if (client->objects[i] != NULL && client->objects[i]->id == obj_id) {
            return client->objects[i];
        }
    }
    return NULL;
}

static bool validate_instance(const SYN_LwM2M_Object *obj, uint16_t inst_id)
{
    if (obj->instances == NULL) {
        return (inst_id < obj->instance_count);
    }
    for (size_t i = 0U; i < obj->instance_count; i++) {
        if (obj->instances[i] == inst_id) {
            return true;
        }
    }
    return false;
}

static const SYN_LwM2M_ResourceDesc *find_resource(const SYN_LwM2M_Object *obj, uint16_t res_id)
{
    if (obj->resources == NULL) {
        return NULL;
    }
    for (size_t i = 0U; i < obj->resource_count; i++) {
        if (obj->resources[i].id == res_id) {
            return &obj->resources[i];
        }
    }
    return NULL;
}

SYN_Status syn_lwm2m_process_request(SYN_LwM2M_Client *client, const SYN_CoapMsg *req,
                                     const SYN_CoapOption *req_opts, size_t req_opt_cnt,
                                     SYN_CoapMsg *resp, SYN_CoapOption *resp_opts,
                                     size_t max_resp_opts, size_t *resp_opt_cnt, uint8_t *resp_buf,
                                     size_t resp_buf_sz)
{
    if (client == NULL || req == NULL || resp == NULL || resp_opts == NULL ||
        resp_opt_cnt == NULL || resp_buf == NULL || max_resp_opts < 2U || resp_buf_sz == 0U) {
        return SYN_ERROR;
    }

    /* Initialize response */
    resp->type = (req->type == COAP_TYPE_CON) ? COAP_TYPE_ACK : COAP_TYPE_NON;
    resp->msg_id = req->msg_id;
    resp->token_len = req->token_len;
    if (req->token_len > 0U) {
        (void)memcpy(resp->token, req->token, req->token_len);
    }
    resp->payload = NULL;
    resp->payload_len = 0U;
    *resp_opt_cnt = 0U;

    /* Parse URI Path segments (1, 2, or 3 segments) */
    size_t path_segs = 0U;
    uint16_t obj_id = 0U;
    uint16_t inst_id = 0U;
    uint16_t res_id = 0U;
    bool has_observe = false;

    if (req_opts != NULL) {
        for (size_t i = 0U; i < req_opt_cnt; i++) {
            if (req_opts[i].num == COAP_OPT_URI_PATH && req_opts[i].val != NULL) {
                char seg[16] = {0};
                size_t c_len =
                    (req_opts[i].len < sizeof(seg) - 1U) ? req_opts[i].len : sizeof(seg) - 1U;
                (void)memcpy(seg, req_opts[i].val, c_len);
                seg[c_len] = '\0';
                unsigned int num = (unsigned int)strtoul(seg, NULL, 10);

                if (path_segs == 0U) {
                    obj_id = (uint16_t)num;
                } else if (path_segs == 1U) {
                    inst_id = (uint16_t)num;
                } else if (path_segs == 2U) {
                    res_id = (uint16_t)num;
                }
                path_segs++;
            } else if (req_opts[i].num == COAP_OPT_OBSERVE) {
                has_observe = true;
            }
        }
    }

    if (path_segs == 0U) {
        resp->code = COAP_RESP_BAD_REQ;
        return SYN_OK;
    }

    const SYN_LwM2M_Object *obj = find_object(client, obj_id);
    if (obj == NULL) {
        resp->code = COAP_RESP_NOT_FOUND;
        return SYN_OK;
    }

    if (path_segs >= 2U && !validate_instance(obj, inst_id)) {
        resp->code = COAP_RESP_NOT_FOUND;
        return SYN_OK;
    }

    static const uint8_t fmt_tlv[2] = {(uint8_t)(SYN_LWM2M_FORMAT_TLV >> 8),
                                       (uint8_t)(SYN_LWM2M_FORMAT_TLV & 0xFF)};

    /* ── GET (Read / Observe / Discover) ─────────────────────────────────── */
    if (req->code == COAP_CODE_GET) {
        /* Single resource read */
        if (path_segs == 3U) {
            const SYN_LwM2M_ResourceDesc *desc = find_resource(obj, res_id);
            if (desc == NULL) {
                resp->code = COAP_RESP_NOT_FOUND;
                return SYN_OK;
            }
            if ((desc->operations & SYN_LWM2M_OP_R) == 0U || obj->read == NULL) {
                resp->code = COAP_RESP_METHOD_NA;
                return SYN_OK;
            }

            SYN_LwM2M_Value val = {0};
            if (obj->read(inst_id, res_id, &val, obj->user_ctx) != SYN_OK) {
                resp->code = COAP_RESP_INTERNAL;
                return SYN_OK;
            }

            size_t enc_len = syn_lwm2m_tlv_encode_value(res_id, &val, resp_buf, resp_buf_sz);
            if (enc_len == 0U) {
                resp->code = COAP_RESP_INTERNAL;
                return SYN_OK;
            }

            resp->code = COAP_RESP_CONTENT;
            resp->payload = resp_buf;
            resp->payload_len = enc_len;

            size_t opt_i = 0U;
            if (has_observe) {
                syn_lwm2m_observe_add(client, obj_id, inst_id, (int32_t)res_id, req->token,
                                      req->token_len, 0U, 60000U);
                static const uint8_t obs_val = 0U;
                resp_opts[opt_i].num = COAP_OPT_OBSERVE;
                resp_opts[opt_i].val = &obs_val;
                resp_opts[opt_i].len = 1U;
                opt_i++;
            }

            resp_opts[opt_i].num = COAP_OPT_CONTENT_FORMAT;
            resp_opts[opt_i].val = fmt_tlv;
            resp_opts[opt_i].len = 2U;
            opt_i++;
            *resp_opt_cnt = opt_i;
            return SYN_OK;
        }

        /* Object instance read (whole instance) */
        if (path_segs == 2U) {
            if (obj->read == NULL || obj->resources == NULL) {
                resp->code = COAP_RESP_NOT_FOUND;
                return SYN_OK;
            }

            size_t total_written = 0U;
            for (size_t i = 0U; i < obj->resource_count; i++) {
                const SYN_LwM2M_ResourceDesc *r_desc = &obj->resources[i];
                if ((r_desc->operations & SYN_LWM2M_OP_R) != 0U) {
                    SYN_LwM2M_Value r_val = {0};
                    if (obj->read(inst_id, r_desc->id, &r_val, obj->user_ctx) == SYN_OK) {
                        size_t w =
                            syn_lwm2m_tlv_encode_value(r_desc->id, &r_val, resp_buf + total_written,
                                                       resp_buf_sz - total_written);
                        total_written += w;
                    }
                }
            }

            resp->code = COAP_RESP_CONTENT;
            resp->payload = resp_buf;
            resp->payload_len = total_written;

            size_t opt_i = 0U;
            if (has_observe) {
                syn_lwm2m_observe_add(client, obj_id, inst_id, -1, req->token, req->token_len, 0U,
                                      60000U);
                static const uint8_t obs_val = 0U;
                resp_opts[opt_i].num = COAP_OPT_OBSERVE;
                resp_opts[opt_i].val = &obs_val;
                resp_opts[opt_i].len = 1U;
                opt_i++;
            }

            resp_opts[opt_i].num = COAP_OPT_CONTENT_FORMAT;
            resp_opts[opt_i].val = fmt_tlv;
            resp_opts[opt_i].len = 2U;
            opt_i++;
            *resp_opt_cnt = opt_i;
            return SYN_OK;
        }

        resp->code = COAP_RESP_BAD_REQ;
        return SYN_OK;
    }

    /* ── PUT / POST (Write) ──────────────────────────────────────────────── */
    if (req->code == COAP_CODE_PUT || (req->code == COAP_CODE_POST && path_segs == 3U)) {
        const SYN_LwM2M_ResourceDesc *desc = find_resource(obj, res_id);
        if (desc == NULL) {
            resp->code = COAP_RESP_NOT_FOUND;
            return SYN_OK;
        }

        /* Check if this is an Execute call on an executable-only resource */
        if ((desc->operations & SYN_LWM2M_OP_E) != 0U &&
            (desc->operations & SYN_LWM2M_OP_W) == 0U) {
            if (obj->exec == NULL) {
                resp->code = COAP_RESP_METHOD_NA;
                return SYN_OK;
            }
            if (obj->exec(inst_id, res_id, req->payload, req->payload_len, obj->user_ctx) ==
                SYN_OK) {
                resp->code = COAP_RESP_CHANGED;
            } else {
                resp->code = COAP_RESP_INTERNAL;
            }
            return SYN_OK;
        }

        if ((desc->operations & SYN_LWM2M_OP_W) == 0U || obj->write == NULL) {
            resp->code = COAP_RESP_METHOD_NA;
            return SYN_OK;
        }

        /* Decode value from payload (supports TLV or plain text) */
        SYN_LwM2M_Value in_val = {0};
        bool decoded = false;

        if (req->payload_len > 0U && req->payload != NULL) {
            SYN_LwM2M_TLV tlv;
            size_t consumed = 0U;
            if (syn_lwm2m_tlv_decode(req->payload, req->payload_len, &tlv, &consumed)) {
                decoded = syn_lwm2m_tlv_decode_value(&tlv, desc->type, &in_val);
            }

            if (!decoded) {
                /* Text fallback */
                if (desc->type == SYN_LWM2M_TYPE_STRING) {
                    char str_buf[64] = {0};
                    size_t c_len = (req->payload_len < sizeof(str_buf) - 1U) ? req->payload_len
                                                                             : sizeof(str_buf) - 1U;
                    (void)memcpy(str_buf, req->payload, c_len);
                    str_buf[c_len] = '\0';
                    in_val.type = SYN_LWM2M_TYPE_STRING;
                    in_val.val.str = str_buf;
                    decoded = true;
                } else if (desc->type == SYN_LWM2M_TYPE_INT || desc->type == SYN_LWM2M_TYPE_TIME) {
                    in_val.type = desc->type;
                    in_val.val.integer = (int64_t)strtoll((const char *)req->payload, NULL, 10);
                    decoded = true;
                } else if (desc->type == SYN_LWM2M_TYPE_OPAQUE) {
                    in_val.type = SYN_LWM2M_TYPE_OPAQUE;
                    in_val.val.opaque.data = req->payload;
                    in_val.val.opaque.len = req->payload_len;
                    decoded = true;
                }
            }
        }

        if (!decoded) {
            resp->code = COAP_RESP_BAD_REQ;
            return SYN_OK;
        }

        if (obj->write(inst_id, res_id, &in_val, obj->user_ctx) == SYN_OK) {
            resp->code = COAP_RESP_CHANGED;
        } else {
            resp->code = COAP_RESP_INTERNAL;
        }
        return SYN_OK;
    }

    /* ── DELETE ─────────────────────────────────────────────────────────── */
    if (req->code == COAP_CODE_DELETE) {
        resp->code = COAP_RESP_DELETED;
        return SYN_OK;
    }

    resp->code = COAP_RESP_METHOD_NA;
    return SYN_OK;
}

/* ── Standard Standard Object Handlers ───────────────────────────────────── */

static const uint16_t s_single_inst[1] = {0U};

/* ── Object 3 (Device) ── */

static const SYN_LwM2M_ResourceDesc s_device_resources[] = {
    {0U, SYN_LWM2M_OP_R, SYN_LWM2M_TYPE_STRING},   /* Manufacturer */
    {1U, SYN_LWM2M_OP_R, SYN_LWM2M_TYPE_STRING},   /* Model Number */
    {2U, SYN_LWM2M_OP_R, SYN_LWM2M_TYPE_STRING},   /* Serial Number */
    {3U, SYN_LWM2M_OP_R, SYN_LWM2M_TYPE_STRING},   /* Firmware Version */
    {4U, SYN_LWM2M_OP_E, SYN_LWM2M_TYPE_NONE},     /* Reboot */
    {5U, SYN_LWM2M_OP_E, SYN_LWM2M_TYPE_NONE},     /* Factory Reset */
    {9U, SYN_LWM2M_OP_R, SYN_LWM2M_TYPE_INT},      /* Battery Level */
    {10U, SYN_LWM2M_OP_R, SYN_LWM2M_TYPE_INT},     /* Memory Free */
    {11U, SYN_LWM2M_OP_R, SYN_LWM2M_TYPE_INT},     /* Error Code */
    {13U, SYN_LWM2M_OP_RW, SYN_LWM2M_TYPE_TIME},   /* Current Time */
    {14U, SYN_LWM2M_OP_RW, SYN_LWM2M_TYPE_STRING}, /* UTC Offset */
    {15U, SYN_LWM2M_OP_RW, SYN_LWM2M_TYPE_STRING}, /* Timezone */
};

static SYN_Status device_read_cb(uint16_t instance_id, uint16_t resource_id,
                                 SYN_LwM2M_Value *out_val, void *user_ctx)
{
    (void)instance_id;
    SYN_LwM2M_DeviceContext *ctx = (SYN_LwM2M_DeviceContext *)user_ctx;
    if (ctx == NULL || out_val == NULL) {
        return SYN_ERROR;
    }

    switch (resource_id) {
    case 0U:
        out_val->type = SYN_LWM2M_TYPE_STRING;
        out_val->val.str = (ctx->manufacturer != NULL) ? ctx->manufacturer : "Syntropic";
        return SYN_OK;
    case 1U:
        out_val->type = SYN_LWM2M_TYPE_STRING;
        out_val->val.str = (ctx->model_number != NULL) ? ctx->model_number : "SyntropicOS-Node";
        return SYN_OK;
    case 2U:
        out_val->type = SYN_LWM2M_TYPE_STRING;
        out_val->val.str = (ctx->serial_number != NULL) ? ctx->serial_number : "SYN-001";
        return SYN_OK;
    case 3U:
        out_val->type = SYN_LWM2M_TYPE_STRING;
        out_val->val.str = (ctx->firmware_ver != NULL) ? ctx->firmware_ver : "1.0.0";
        return SYN_OK;
    case 9U:
        out_val->type = SYN_LWM2M_TYPE_INT;
        out_val->val.integer = ctx->battery_level;
        return SYN_OK;
    case 10U:
        out_val->type = SYN_LWM2M_TYPE_INT;
        out_val->val.integer = ctx->memory_free_kb;
        return SYN_OK;
    case 11U:
        out_val->type = SYN_LWM2M_TYPE_INT;
        out_val->val.integer = ctx->error_code;
        return SYN_OK;
    case 13U:
        out_val->type = SYN_LWM2M_TYPE_TIME;
        out_val->val.integer = ctx->current_time;
        return SYN_OK;
    case 14U:
        out_val->type = SYN_LWM2M_TYPE_STRING;
        out_val->val.str = (ctx->utc_offset != NULL) ? ctx->utc_offset : "+00:00";
        return SYN_OK;
    case 15U:
        out_val->type = SYN_LWM2M_TYPE_STRING;
        out_val->val.str = (ctx->timezone != NULL) ? ctx->timezone : "UTC";
        return SYN_OK;
    default:
        return SYN_ERROR;
    }
}

static SYN_Status device_write_cb(uint16_t instance_id, uint16_t resource_id,
                                  const SYN_LwM2M_Value *in_val, void *user_ctx)
{
    (void)instance_id;
    SYN_LwM2M_DeviceContext *ctx = (SYN_LwM2M_DeviceContext *)user_ctx;
    if (ctx == NULL || in_val == NULL) {
        return SYN_ERROR;
    }

    if (resource_id == 13U &&
        (in_val->type == SYN_LWM2M_TYPE_INT || in_val->type == SYN_LWM2M_TYPE_TIME)) {
        ctx->current_time = in_val->val.integer;
        return SYN_OK;
    }

    return SYN_ERROR;
}

static SYN_Status device_exec_cb(uint16_t instance_id, uint16_t resource_id, const uint8_t *args,
                                 size_t args_len, void *user_ctx)
{
    (void)instance_id;
    (void)args;
    (void)args_len;
    SYN_LwM2M_DeviceContext *ctx = (SYN_LwM2M_DeviceContext *)user_ctx;
    if (ctx == NULL) {
        return SYN_ERROR;
    }

    if (resource_id == 4U) {
        ctx->reboot_requested = true;
        return SYN_OK;
    }
    if (resource_id == 5U) {
        ctx->factory_reset_requested = true;
        return SYN_OK;
    }

    return SYN_ERROR;
}

SYN_LwM2M_Object syn_lwm2m_make_device_object(SYN_LwM2M_DeviceContext *ctx)
{
    SYN_LwM2M_Object obj = {
        .id = SYN_LWM2M_OBJ_DEVICE,
        .instance_count = 1U,
        .instances = s_single_inst,
        .resource_count = sizeof(s_device_resources) / sizeof(s_device_resources[0]),
        .resources = s_device_resources,
        .read = device_read_cb,
        .write = device_write_cb,
        .exec = device_exec_cb,
        .user_ctx = ctx,
    };
    return obj;
}

/* ── Object 5 (Firmware Update) ── */

static const SYN_LwM2M_ResourceDesc s_fw_resources[] = {
    {1U, SYN_LWM2M_OP_RW, SYN_LWM2M_TYPE_STRING}, /* Package URI */
    {2U, SYN_LWM2M_OP_E, SYN_LWM2M_TYPE_NONE},    /* Update */
    {3U, SYN_LWM2M_OP_R, SYN_LWM2M_TYPE_INT},     /* State */
    {5U, SYN_LWM2M_OP_R, SYN_LWM2M_TYPE_INT},     /* Update Result */
    {6U, SYN_LWM2M_OP_R, SYN_LWM2M_TYPE_STRING},  /* Package Name */
    {7U, SYN_LWM2M_OP_R, SYN_LWM2M_TYPE_STRING},  /* Package Version */
};

static SYN_Status fw_read_cb(uint16_t instance_id, uint16_t resource_id, SYN_LwM2M_Value *out_val,
                             void *user_ctx)
{
    (void)instance_id;
    SYN_LwM2M_FirmwareContext *ctx = (SYN_LwM2M_FirmwareContext *)user_ctx;
    if (ctx == NULL || out_val == NULL) {
        return SYN_ERROR;
    }

    switch (resource_id) {
    case 1U:
        out_val->type = SYN_LWM2M_TYPE_STRING;
        out_val->val.str = ctx->package_uri;
        return SYN_OK;
    case 3U:
        out_val->type = SYN_LWM2M_TYPE_INT;
        out_val->val.integer = (int64_t)ctx->state;
        return SYN_OK;
    case 5U:
        out_val->type = SYN_LWM2M_TYPE_INT;
        out_val->val.integer = (int64_t)ctx->result;
        return SYN_OK;
    case 6U:
        out_val->type = SYN_LWM2M_TYPE_STRING;
        out_val->val.str = ctx->pkg_name;
        return SYN_OK;
    case 7U:
        out_val->type = SYN_LWM2M_TYPE_STRING;
        out_val->val.str = ctx->pkg_version;
        return SYN_OK;
    default:
        return SYN_ERROR;
    }
}

static SYN_Status fw_write_cb(uint16_t instance_id, uint16_t resource_id,
                              const SYN_LwM2M_Value *in_val, void *user_ctx)
{
    (void)instance_id;
    SYN_LwM2M_FirmwareContext *ctx = (SYN_LwM2M_FirmwareContext *)user_ctx;
    if (ctx == NULL || in_val == NULL) {
        return SYN_ERROR;
    }

    if (resource_id == 1U && in_val->type == SYN_LWM2M_TYPE_STRING && in_val->val.str != NULL) {
        (void)strncpy(ctx->package_uri, in_val->val.str, sizeof(ctx->package_uri) - 1U);
        ctx->state = SYN_LWM2M_FW_STATE_DOWNLOADED;
        return SYN_OK;
    }

    return SYN_ERROR;
}

static SYN_Status fw_exec_cb(uint16_t instance_id, uint16_t resource_id, const uint8_t *args,
                             size_t args_len, void *user_ctx)
{
    (void)instance_id;
    (void)args;
    (void)args_len;
    SYN_LwM2M_FirmwareContext *ctx = (SYN_LwM2M_FirmwareContext *)user_ctx;
    if (ctx == NULL) {
        return SYN_ERROR;
    }

    if (resource_id == 2U) {
        ctx->update_requested = true;
        ctx->state = SYN_LWM2M_FW_STATE_UPDATING;
        ctx->result = SYN_LWM2M_FW_RESULT_SUCCESS;
        return SYN_OK;
    }

    return SYN_ERROR;
}

SYN_LwM2M_Object syn_lwm2m_make_firmware_object(SYN_LwM2M_FirmwareContext *ctx)
{
    SYN_LwM2M_Object obj = {
        .id = SYN_LWM2M_OBJ_FIRMWARE_UPDATE,
        .instance_count = 1U,
        .instances = s_single_inst,
        .resource_count = sizeof(s_fw_resources) / sizeof(s_fw_resources[0]),
        .resources = s_fw_resources,
        .read = fw_read_cb,
        .write = fw_write_cb,
        .exec = fw_exec_cb,
        .user_ctx = ctx,
    };
    return obj;
}

/* ── Object 3303 (IPSO Temperature Sensor) ── */

static const SYN_LwM2M_ResourceDesc s_temp_resources[] = {
    {5700U, SYN_LWM2M_OP_R, SYN_LWM2M_TYPE_FLOAT},  /* Sensor Value */
    {5701U, SYN_LWM2M_OP_R, SYN_LWM2M_TYPE_STRING}, /* Sensor Unit */
    {5601U, SYN_LWM2M_OP_R, SYN_LWM2M_TYPE_FLOAT},  /* Min Measured Value */
    {5602U, SYN_LWM2M_OP_R, SYN_LWM2M_TYPE_FLOAT},  /* Max Measured Value */
};

static SYN_Status temp_read_cb(uint16_t instance_id, uint16_t resource_id, SYN_LwM2M_Value *out_val,
                               void *user_ctx)
{
    (void)instance_id;
    SYN_LwM2M_SensorContext *ctx = (SYN_LwM2M_SensorContext *)user_ctx;
    if (ctx == NULL || out_val == NULL) {
        return SYN_ERROR;
    }

    switch (resource_id) {
    case 5700U:
        out_val->type = SYN_LWM2M_TYPE_FLOAT;
        out_val->val.floating = ctx->sensor_value;
        return SYN_OK;
    case 5701U:
        out_val->type = SYN_LWM2M_TYPE_STRING;
        out_val->val.str = (ctx->unit != NULL) ? ctx->unit : "Cel";
        return SYN_OK;
    case 5601U:
        out_val->type = SYN_LWM2M_TYPE_FLOAT;
        out_val->val.floating = ctx->min_measured_val;
        return SYN_OK;
    case 5602U:
        out_val->type = SYN_LWM2M_TYPE_FLOAT;
        out_val->val.floating = ctx->max_measured_val;
        return SYN_OK;
    default:
        return SYN_ERROR;
    }
}

SYN_LwM2M_Object syn_lwm2m_make_temperature_object(SYN_LwM2M_SensorContext *ctx)
{
    SYN_LwM2M_Object obj = {
        .id = SYN_LWM2M_OBJ_TEMPERATURE,
        .instance_count = 1U,
        .instances = s_single_inst,
        .resource_count = sizeof(s_temp_resources) / sizeof(s_temp_resources[0]),
        .resources = s_temp_resources,
        .read = temp_read_cb,
        .write = NULL,
        .exec = NULL,
        .user_ctx = ctx,
    };
    return obj;
}

#endif /* SYN_USE_LWM2M */
```


