

# File syn\_port\_serial.h

[**File List**](files.md) **>** [**port**](dir_df8aee55eb103ebca23cd86ea02a32de.md) **>** [**syn\_port\_serial.h**](syn__port__serial_8h.md)

[Go to the documentation of this file](syn__port__serial_8h.md)


```C++

#ifndef SYN_PORT_SERIAL_H
#define SYN_PORT_SERIAL_H

#include "../common/syn_defs.h"
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

SYN_Status syn_port_serial_init(uint32_t baudrate);

int syn_port_serial_write(const uint8_t *data, size_t len);

int syn_port_serial_read(uint8_t *buf, size_t max_len);

/* ── Inline convenience helpers (not port obligations) ─────────────────── */

static inline int syn_port_serial_write_byte(uint8_t byte)
{
    return syn_port_serial_write(&byte, 1);
}

static inline int syn_port_serial_read_byte(uint8_t *byte)
{
    return syn_port_serial_read(byte, 1);
}

#ifdef __cplusplus
}
#endif

#endif /* SYN_PORT_SERIAL_H */
```


