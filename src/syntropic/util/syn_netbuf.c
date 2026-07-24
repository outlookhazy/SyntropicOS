/**
 * @file syn_netbuf.c
 * @brief Zero-Copy Packet Buffer Pool implementation.
 */

#include "syn_netbuf.h"
#include <string.h>

SYN_Status syn_netbuf_pool_init(SYN_NetBufPool *pool, SYN_NetBuf *bufs, uint8_t *raw_mem, size_t buf_count, size_t buf_size)
{
    if (!pool || !bufs || !raw_mem || buf_count == 0 || buf_size == 0) return SYN_INVALID_PARAM;

    memset(pool, 0, sizeof(*pool));
    pool->bufs       = bufs;
    pool->raw_mem    = raw_mem;
    pool->buf_count  = buf_count;
    pool->buf_size   = buf_size;
    pool->free_count = buf_count;
    pool->free_list  = NULL;

    for (size_t i = 0; i < buf_count; i++) {
        SYN_NetBuf *b = &bufs[i];
        uint8_t *mem = raw_mem + (i * buf_size);
        b->head      = mem;
        b->data      = mem;
        b->tail      = mem;
        b->end       = mem + buf_size;
        b->ref_count = 0;
        b->flags     = 0;
        b->pool      = pool;
        b->next_free = pool->free_list;
        pool->free_list = b;
    }

    return SYN_OK;
}

SYN_NetBuf *syn_netbuf_alloc(SYN_NetBufPool *pool, size_t headroom)
{
    if (!pool || !pool->free_list || pool->free_count == 0) return NULL;

    SYN_NetBuf *b = pool->free_list;
    pool->free_list = b->next_free;
    b->next_free = NULL;

    if (headroom > pool->buf_size) headroom = 0;

    b->data      = b->head + headroom;
    b->tail      = b->data;
    b->ref_count = 1;
    pool->free_count--;

    return b;
}

void syn_netbuf_ref(SYN_NetBuf *buf)
{
    if (!buf) return;
    buf->ref_count++;
}

void syn_netbuf_free(SYN_NetBufPool *pool, SYN_NetBuf *buf)
{
    if (!buf || buf->ref_count == 0) return;
    if (!pool && buf->pool) {
        pool = (SYN_NetBufPool *)buf->pool;
    }
    if (!pool) return;

    buf->ref_count--;
    if (buf->ref_count == 0) {
        buf->data = buf->head;
        buf->tail = buf->head;
        buf->next_free = pool->free_list;
        pool->free_list = buf;
        pool->free_count++;
    }
}

uint8_t *syn_netbuf_push(SYN_NetBuf *buf, size_t len)
{
    if (!buf) return NULL;
    if ((size_t)(buf->data - buf->head) < len) return NULL;

    buf->data -= len;
    return buf->data;
}

uint8_t *syn_netbuf_pull(SYN_NetBuf *buf, size_t len)
{
    if (!buf) return NULL;
    if ((size_t)(buf->tail - buf->data) < len) return NULL;

    buf->data += len;
    return buf->data;
}

uint8_t *syn_netbuf_put(SYN_NetBuf *buf, size_t len)
{
    if (!buf) return NULL;
    if ((size_t)(buf->end - buf->tail) < len) return NULL;

    uint8_t *ret = buf->tail;
    buf->tail += len;
    return ret;
}

size_t syn_netbuf_len(const SYN_NetBuf *buf)
{
    if (!buf || buf->tail < buf->data) return 0;
    return (size_t)(buf->tail - buf->data);
}
