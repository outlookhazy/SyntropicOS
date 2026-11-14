

# File syn\_netbuf.c

[**File List**](files.md) **>** [**net**](dir_1b04293f300841df63a8ab6b5fd3200c.md) **>** [**syn\_netbuf.c**](syn__netbuf_8c.md)

[Go to the documentation of this file](syn__netbuf_8c.md)


```C++


#include "syn_netbuf.h"

#include "../port/syn_port_system.h"

#include <string.h>

SYN_Status syn_netbuf_pool_init(SYN_NetBufPool *pool, SYN_NetBuf *bufs, uint8_t *raw_mem,
                                size_t buf_count, size_t buf_size)
{
    if (!pool || !bufs || !raw_mem || buf_count == 0U || buf_size == 0U ||
        buf_count > (SIZE_MAX / buf_size)) {
        return SYN_INVALID_PARAM;
    }

    syn_port_enter_critical();
    memset(pool, 0, sizeof(*pool));
    pool->bufs = bufs;
    pool->raw_mem = raw_mem;
    pool->buf_count = buf_count;
    pool->buf_size = buf_size;
    pool->free_count = buf_count;
    pool->free_list = NULL;

    for (size_t i = 0U; i < buf_count; i++) {
        SYN_NetBuf *b = &bufs[i];
        uint8_t *mem = raw_mem + (i * buf_size);
        b->head = mem;
        b->data = mem;
        b->tail = mem;
        b->end = mem + buf_size;
        b->ref_count = 0U;
        b->flags = 0U;
        b->pool = pool;
        b->next_free = pool->free_list;
        pool->free_list = b;
    }
    syn_port_exit_critical();

    return SYN_OK;
}

SYN_NetBuf *syn_netbuf_alloc(SYN_NetBufPool *pool, size_t headroom)
{
    if (pool == NULL) {
        return NULL;
    }

    syn_port_enter_critical();
    if (pool->free_list == NULL || pool->free_count == 0U) {
        syn_port_exit_critical();
        return NULL;
    }

    SYN_NetBuf *b = pool->free_list;
    pool->free_list = b->next_free;
    b->next_free = NULL;

    if (headroom > pool->buf_size) {
        headroom = 0U;
    }

    b->data = b->head + headroom;
    b->tail = b->data;
    b->ref_count = 1U;
    b->flags = 0U;
    pool->free_count--;
    syn_port_exit_critical();

    return b;
}

void syn_netbuf_ref(SYN_NetBuf *buf)
{
    if (buf == NULL) {
        return;
    }
    syn_port_enter_critical();
    if (buf->ref_count < UINT16_MAX) {
        buf->ref_count++;
    }
    syn_port_exit_critical();
}

void syn_netbuf_free(SYN_NetBufPool *pool, SYN_NetBuf *buf)
{
    if (buf == NULL) {
        return;
    }
    SYN_NetBufPool *owner = (SYN_NetBufPool *)buf->pool;
    if (owner == NULL || (pool != NULL && pool != owner)) {
        return;
    }

    syn_port_enter_critical();
    if (buf->ref_count > 0U) {
        buf->ref_count--;
        if (buf->ref_count == 0U) {
            buf->data = buf->head;
            buf->tail = buf->head;
            buf->flags = 0U;
            buf->next_free = owner->free_list;
            owner->free_list = buf;
            owner->free_count++;
        }
    }
    syn_port_exit_critical();
}

uint8_t *syn_netbuf_push(SYN_NetBuf *buf, size_t len)
{
    if (!buf)
        return NULL;
    if ((size_t)(buf->data - buf->head) < len)
        return NULL;

    buf->data -= len;
    return buf->data;
}

uint8_t *syn_netbuf_pull(SYN_NetBuf *buf, size_t len)
{
    if (!buf)
        return NULL;
    if ((size_t)(buf->tail - buf->data) < len)
        return NULL;

    buf->data += len;
    return buf->data;
}

uint8_t *syn_netbuf_put(SYN_NetBuf *buf, size_t len)
{
    if (!buf)
        return NULL;
    if ((size_t)(buf->end - buf->tail) < len)
        return NULL;

    uint8_t *ret = buf->tail;
    buf->tail += len;
    return ret;
}

size_t syn_netbuf_len(const SYN_NetBuf *buf)
{
    if (!buf || buf->tail < buf->data)
        return 0;
    return (size_t)(buf->tail - buf->data);
}
```


