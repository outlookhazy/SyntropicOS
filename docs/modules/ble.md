# Bluetooth Low Energy (BLE) Host Stack

The SyntropicOS BLE module (`syn_ble`) provides a zero-heap, deterministic Bluetooth Low Energy (BLE) host stack for microcontrollers paired with external BLE controllers over UART (H:4) or SPI.

## Architecture

```
┌─────────────────────────────────────────────────────────┐
│                 BLE Application / Profile               │
├─────────────────────────────────────────────────────────┤
│                   GATT Server (syn_ble_gatt)            │ Static attribute table
├─────────────────────────────────────────────────────────┤
│                   GAP Layer (syn_ble_gap)               │ Adv, scan, connect, disconnect
├─────────────────────────────────────────────────────────┤
│                   ATT Protocol (syn_ble_att)            │ PDU encoding & decoding
├─────────────────────────────────────────────────────────┤
│                  L2CAP Channel (syn_ble_l2cap)          │ CID 0x0004 (ATT), reassembly
├─────────────────────────────────────────────────────────┤
│                HCI Transport Driver (syn_ble_hci)       │ H:4 UART framing (Cmd, Evt, ACL)
├─────────────────────────────────────────────────────────┤
│                HCI Port Adapter (syn_port_ble_hci)      │ Hardware UART / SPI transport
└─────────────────────────────────────────────────────────┘
```

## Header Reference

| Header | Layer | Description |
|---|---|---|
| `ble/syn_ble_hci.h` | HCI Transport | H:4 UART/SPI framing for HCI Command (`0x01`), ACL Data (`0x02`), and Event (`0x04`) packets (`SYN_BLE_HCI`, `syn_ble_hci_init`, `syn_ble_hci_rx_byte`) |
| `ble/syn_ble_l2cap.h` | L2CAP | Connection-oriented CID `0x0004` (ATT) fragmentation and reassembly (`SYN_BLE_L2CAP`, `syn_ble_l2cap_init`) |
| `ble/syn_ble_att.h` | ATT | Attribute Protocol PDU encoder and decoder (`SYN_BLE_ATT_*`, Read/Write/FindInfo/ReadByGroupType/HandleValueNotification) |
| `ble/syn_ble_gap.h` | GAP | Peripheral advertising, central scanning, and connection lifecycle (`SYN_BLE_GAP`, `syn_ble_gap_init`, `syn_ble_gap_set_adv_data`) |
| `ble/syn_ble_gatt.h` | GATT Server | Static attribute table dispatch, per-connection CCCD state, and outbound notifications (`SYN_BLE_GATT`, `syn_ble_gatt_init`, `syn_ble_gatt_notify`) |
| `port/syn_port_ble_hci.h` | Port HAL | Platform controller transport (`syn_port_ble_hci_tx`, `syn_port_ble_hci_reset`) |

## Key Features

- **Zero Dynamic Memory**: All HCI RX reassembly buffers, L2CAP connection tables, and GATT attribute entries are statically allocated.
- **H:4 Transport Layer**: Compliance with Bluetooth Core Specification Vol 4, Part A.
- **GATT Server**: Static attribute tables for custom or standard SIG services and characteristics, with independent per-connection CCCD tracking.
- **GAP Peripheral & Central**: Connectable and non-connectable advertising, active/passive scanning, and connection parameter updates.

## Usage Example

```c
#include <syntropic/syntropic.h>

static SYN_BLE_HCI hci;
static SYN_BLE_GAP gap;
static SYN_BLE_GATT gatt;

/* Define GATT attribute table */
static const SYN_BLE_GATT_Attr gatt_table[] = {
    { 0x0001, SYN_BLE_UUID_PRIMARY_SERVICE, SYN_BLE_PROP_READ, NULL, NULL, (const uint8_t *)"\x09\x18", 2 },
    { 0x0002, 0x2A6E, SYN_BLE_PROP_READ, temp_read_cb, NULL, NULL, 0 }
};

void ble_init(SYN_BLE_HCI_EventCb hci_evt_cb, SYN_BLE_HCI_AclCb hci_acl_cb,
              SYN_BLE_GAP_EventCb gap_evt_cb) {
    SYN_BLE_HCI_Config hci_cfg = {
        .evt_cb = hci_evt_cb,
        .acl_cb = hci_acl_cb,
        .user_data = NULL,
    };
    syn_ble_hci_init(&hci, &hci_cfg);
    syn_ble_gap_init(&gap, &hci, gap_evt_cb, NULL);
    syn_ble_gatt_init(&gatt, gatt_table, 2, NULL);
}
```

## Configuration Defines

| Define | Default | Description |
|---|---|---|
| `SYN_USE_BLE` | `1` | Enable BLE host stack headers in `syntropic.h` |
| `SYN_BLE_MAX_CONNECTIONS` | `2U` | Maximum simultaneous active BLE connections |
| `SYN_BLE_MAX_ATTRIBUTES` | `32U` | Maximum GATT attribute table capacity |
| `SYN_BLE_MAX_CCCDS` | `8U` | Maximum independent CCCD descriptors tracked per connection |
| `SYN_BLE_HCI_RX_BUF_SIZE` | `260U` | HCI RX packet buffer size in bytes (4 B ACL header + 256 B L2CAP payload) |
| `SYN_BLE_HCI_TX_BUF_SIZE` | `261U` | HCI TX staging size in bytes (1 B H:4 indicator + 4 B ACL header + 256 B payload) |
