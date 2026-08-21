

# File syn\_port\_ble\_hci.h

[**File List**](files.md) **>** [**port**](dir_df8aee55eb103ebca23cd86ea02a32de.md) **>** [**syn\_port\_ble\_hci.h**](syn__port__ble__hci_8h.md)

[Go to the documentation of this file](syn__port__ble__hci_8h.md)


```C++

#ifndef SYN_PORT_BLE_HCI_H
#define SYN_PORT_BLE_HCI_H

#include "../common/syn_defs.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

SYN_Status syn_port_ble_hci_tx(const uint8_t *data, uint16_t len);

SYN_Status syn_port_ble_hci_reset(void);

#ifdef __cplusplus
}
#endif

#endif /* SYN_PORT_BLE_HCI_H */
```


