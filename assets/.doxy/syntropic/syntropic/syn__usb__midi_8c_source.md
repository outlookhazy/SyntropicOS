

# File syn\_usb\_midi.c

[**File List**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_usb\_midi.c**](syn__usb__midi_8c.md)

[Go to the documentation of this file](syn__usb__midi_8c.md)


```C++


#include "syn_usb_midi.h"

#include "syntropic/port/syn_port_system.h"

#include <string.h>

static const uint8_t MIDI_INTERFACE_DESC[SYN_USB_MIDI_DESC_SIZE] = {
    /* Interface 0: Standard Audio Control Interface (0 endpoints) */
    0x09U, 0x04U, 0x00U, 0x00U, 0x00U, 0x01U, 0x01U, 0x00U, 0x00U,
    /* CS_INTERFACE: Audio Control Header (Streaming interface = 1) */
    0x09U, 0x24U, 0x01U, 0x00U, 0x01U, 0x09U, 0x00U, 0x01U, 0x01U,
    /* Interface 1: Standard MIDI Streaming Interface (2 endpoints) */
    0x09U, 0x04U, 0x01U, 0x00U, 0x02U, 0x01U, 0x03U, 0x00U, 0x00U,
    /* CS_INTERFACE: MIDI Streaming Header (wTotalLength = 22) */
    0x07U, 0x24U, 0x01U, 0x00U, 0x01U, 0x16U, 0x00U,
    /* MIDI IN Jack Descriptor (Embedded, Jack ID 1) */
    0x06U, 0x24U, 0x02U, 0x01U, 0x01U, 0x00U,
    /* MIDI OUT Jack Descriptor (Embedded, Jack ID 2, Source = 1) */
    0x09U, 0x24U, 0x03U, 0x01U, 0x02U, 0x01U, 0x01U, 0x01U, 0x00U,
    /* Bulk OUT Endpoint Descriptor (offset 49, bEndpointAddress at 51) */
    0x07U, 0x05U, 0x01U, 0x02U, 0x40U, 0x00U, 0x00U,
    /* CS_ENDPOINT: MS General (1 Embedded MIDI IN Jack = ID 1) */
    0x05U, 0x25U, 0x01U, 0x01U, 0x01U,
    /* Bulk IN Endpoint Descriptor (offset 61, bEndpointAddress at 63) */
    0x07U, 0x05U, 0x81U, 0x02U, 0x40U, 0x00U, 0x00U,
    /* CS_ENDPOINT: MS General (1 Embedded MIDI OUT Jack = ID 2) */
    0x05U, 0x25U, 0x01U, 0x01U, 0x02U};

static SYN_Status usb_midi_setup_handler(void *ctx, const SYN_USB_SetupPacket *pkt, uint8_t *resp,
                                         uint16_t resp_cap, uint16_t *rlen)
{
    (void)ctx;
    (void)pkt;
    (void)resp;
    (void)resp_cap;
    if (rlen != NULL) {
        *rlen = 0U;
    }
    return SYN_OK;
}

static void usb_midi_data_out_handler(void *ctx, uint8_t ep, const uint8_t *data, uint16_t len)
{
    SYN_USB_MIDI *midi = (SYN_USB_MIDI *)ctx;
    if ((midi == NULL) || (data == NULL) || (len == 0U) || (ep != midi->ep_out)) {
        return;
    }

    syn_port_enter_critical();
    uint16_t space = (uint16_t)(sizeof(midi->rx_buf) - midi->rx_len);
    uint16_t copy_len = (len < space) ? len : space;
    if (copy_len > 0U) {
        memcpy(&midi->rx_buf[midi->rx_len], data, copy_len);
        midi->rx_len = (uint16_t)(midi->rx_len + copy_len);
    }
    syn_port_exit_critical();
}

static void usb_midi_data_in_handler(void *ctx, uint8_t ep)
{
    SYN_USB_MIDI *midi = (SYN_USB_MIDI *)ctx;
    if (midi == NULL) {
        return;
    }
    if (ep == midi->ep_in) {
        midi->tx_len = 0U;
    }
}

static SYN_Status usb_midi_configured_handler(void *ctx, uint8_t config)
{
    SYN_USB_MIDI *midi = (SYN_USB_MIDI *)ctx;
    (void)config;
    if (midi != NULL) {
        midi->tx_len = 0U;
    }
    return SYN_OK;
}

SYN_Status syn_usb_midi_init(SYN_USB_MIDI *midi)
{
    if (midi == NULL) {
        return SYN_INVALID_PARAM;
    }
    memset(midi, 0, sizeof(*midi));
    midi->ep_in = 0x81U;
    midi->ep_out = 0x01U;
    midi->iface_num = 0U;
    midi->cable_num = 0U;
    return SYN_OK;
}

SYN_Status syn_usb_midi_register(SYN_USB_Device *dev, SYN_USB_MIDI *midi)
{
    if ((dev == NULL) || (midi == NULL)) {
        return SYN_INVALID_PARAM;
    }

    uint8_t desc_buf[SYN_USB_MIDI_DESC_SIZE];
    memcpy(desc_buf, MIDI_INTERFACE_DESC, sizeof(MIDI_INTERFACE_DESC));
    desc_buf[51] = midi->ep_out;
    desc_buf[63] = midi->ep_in;

    SYN_USB_ClassDriver driver;
    memset(&driver, 0, sizeof(driver));
    driver.iface_start = midi->iface_num;
    driver.iface_count = 2U;
    driver.ctx = midi;
    driver.setup = usb_midi_setup_handler;
    driver.data_out = usb_midi_data_out_handler;
    driver.data_in = usb_midi_data_in_handler;
    driver.configured = usb_midi_configured_handler;

    SYN_Status st = syn_usb_register_class(dev, &driver, desc_buf, SYN_USB_MIDI_DESC_SIZE);
    if (st == SYN_OK) {
        midi->iface_num = dev->classes[dev->class_count - 1U].iface_start;
        if (!dev->raw_config_override) {
            uint16_t base = (uint16_t)(dev->config_buf_used - SYN_USB_MIDI_DESC_SIZE);
            midi->ep_out = dev->config_buf[base + 51U];
            midi->ep_in = dev->config_buf[base + 63U];
        }
    }
    return st;
}

static SYN_Status queue_midi_packet(SYN_USB_MIDI *midi, uint8_t cin, uint8_t status, uint8_t param1,
                                    uint8_t param2)
{
    if (midi == NULL) {
        return SYN_INVALID_PARAM;
    }
    if ((midi->tx_len + 4U) > SYN_USB_MIDI_MAX_PACKET_SIZE) {
        return SYN_BUSY;
    }

    uint8_t header = (uint8_t)(((midi->cable_num & 0x0FU) << 4) | (cin & 0x0FU));
    midi->tx_buf[midi->tx_len++] = header;
    midi->tx_buf[midi->tx_len++] = status;
    midi->tx_buf[midi->tx_len++] = param1;
    midi->tx_buf[midi->tx_len++] = param2;

    return SYN_OK;
}

SYN_Status syn_usb_midi_send_note_on(SYN_USB_MIDI *midi, uint8_t channel, uint8_t note,
                                     uint8_t velocity)
{
    if ((channel > 15U) || (note > 127U) || (velocity > 127U)) {
        return SYN_INVALID_PARAM;
    }
    uint8_t status = (uint8_t)(0x90U | channel);
    return queue_midi_packet(midi, SYN_USB_MIDI_CIN_NOTE_ON, status, note, velocity);
}

SYN_Status syn_usb_midi_send_note_off(SYN_USB_MIDI *midi, uint8_t channel, uint8_t note,
                                      uint8_t velocity)
{
    if ((channel > 15U) || (note > 127U) || (velocity > 127U)) {
        return SYN_INVALID_PARAM;
    }
    uint8_t status = (uint8_t)(0x80U | channel);
    return queue_midi_packet(midi, SYN_USB_MIDI_CIN_NOTE_OFF, status, note, velocity);
}

SYN_Status syn_usb_midi_send_cc(SYN_USB_MIDI *midi, uint8_t channel, uint8_t controller,
                                uint8_t value)
{
    if ((channel > 15U) || (controller > 127U) || (value > 127U)) {
        return SYN_INVALID_PARAM;
    }
    uint8_t status = (uint8_t)(0xB0U | channel);
    return queue_midi_packet(midi, SYN_USB_MIDI_CIN_CONTROL_CHANGE, status, controller, value);
}

SYN_Status syn_usb_midi_send_pitch_bend(SYN_USB_MIDI *midi, uint8_t channel, int16_t value)
{
    if ((channel > 15U) || (value < -8192) || (value > 8191)) {
        return SYN_INVALID_PARAM;
    }
    uint16_t unsigned_val = (uint16_t)(value + 8192);
    uint8_t lsb = (uint8_t)(unsigned_val & 0x7FU);
    uint8_t msb = (uint8_t)((unsigned_val >> 7) & 0x7FU);
    uint8_t status = (uint8_t)(0xE0U | channel);

    return queue_midi_packet(midi, SYN_USB_MIDI_CIN_PITCH_BEND, status, lsb, msb);
}

SYN_Status syn_usb_midi_parse_packet(const uint8_t raw_bytes[4], SYN_USB_MIDI_Packet *packet)
{
    if ((raw_bytes == NULL) || (packet == NULL)) {
        return SYN_INVALID_PARAM;
    }
    packet->header = raw_bytes[0];
    packet->midi0 = raw_bytes[1];
    packet->midi1 = raw_bytes[2];
    packet->midi2 = raw_bytes[3];
    return SYN_OK;
}

SYN_Status syn_usb_midi_read(SYN_USB_MIDI *midi, void *buf, size_t max_len, size_t *out_len)
{
    if ((midi == NULL) || (buf == NULL) || (out_len == NULL)) {
        return SYN_INVALID_PARAM;
    }

    syn_port_enter_critical();
    uint16_t available = midi->rx_len;
    if (available == 0U) {
        syn_port_exit_critical();
        *out_len = 0U;
        return SYN_OK;
    }

    if (max_len < available) {
        available = (uint16_t)max_len;
    }

    memcpy(buf, midi->rx_buf, available);
    if (available < midi->rx_len) {
        memmove(midi->rx_buf, &midi->rx_buf[available], (size_t)(midi->rx_len - available));
        midi->rx_len = (uint16_t)(midi->rx_len - available);
    } else {
        midi->rx_len = 0U;
    }
    syn_port_exit_critical();

    *out_len = available;
    return SYN_OK;
}

bool syn_usb_midi_rx_available(const SYN_USB_MIDI *midi)
{
    return (midi != NULL) && (midi->rx_len > 0U);
}
```


