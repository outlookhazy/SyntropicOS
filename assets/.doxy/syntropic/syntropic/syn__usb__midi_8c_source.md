

# File syn\_usb\_midi.c

[**File List**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_usb\_midi.c**](syn__usb__midi_8c.md)

[Go to the documentation of this file](syn__usb__midi_8c.md)


```C++

#include "syn_usb_midi.h"

#include <string.h>

static SYN_Status usb_midi_setup_handler(void *ctx, const SYN_USB_SetupPacket *pkt, uint8_t *resp,
                                         uint16_t *rlen)
{
    (void)ctx;
    (void)pkt;
    (void)resp;
    (void)rlen;
    return SYN_OK;
}

static void usb_midi_data_out_handler(void *ctx, uint8_t ep, const uint8_t *data, uint16_t len)
{
    SYN_USB_MIDI *midi = (SYN_USB_MIDI *)ctx;
    if ((midi == NULL) || (data == NULL) || (len == 0U)) {
        return;
    }
    if (ep == midi->ep_out) {
        uint16_t copy_len = len > SYN_USB_MIDI_MAX_PACKET_SIZE ? SYN_USB_MIDI_MAX_PACKET_SIZE : len;
        memcpy(midi->rx_buf, data, copy_len);
        midi->rx_len = copy_len;
    }
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
    (void)ctx;
    (void)config;
    return SYN_OK;
}

SYN_Status syn_usb_midi_init(SYN_USB_MIDI *midi)
{
    if (midi == NULL) {
        return SYN_INVALID_PARAM;
    }
    memset(midi, 0, sizeof(*midi));
    midi->ep_in = 0x81U;  /* Bulk IN Endpoint 1 */
    midi->ep_out = 0x01U; /* Bulk OUT Endpoint 1 */
    midi->iface_num = 0U;
    midi->cable_num = 0U;
    return SYN_OK;
}

SYN_Status syn_usb_midi_register(SYN_USB_Device *dev, SYN_USB_MIDI *midi)
{
    if ((dev == NULL) || (midi == NULL)) {
        return SYN_INVALID_PARAM;
    }

    SYN_USB_ClassDriver driver;
    memset(&driver, 0, sizeof(driver));
    driver.iface_start = midi->iface_num;
    driver.iface_count = 2U; /* Control + Streaming interface */
    driver.ctx = midi;
    driver.setup = usb_midi_setup_handler;
    driver.data_out = usb_midi_data_out_handler;
    driver.data_in = usb_midi_data_in_handler;
    driver.configured = usb_midi_configured_handler;

    static const uint8_t midi_desc[9] = {0x09, 0x04, 0x00, 0x00, 0x02, 0x01, 0x03, 0x00, 0x00};
    return syn_usb_register_class(dev, &driver, midi_desc, (uint16_t)sizeof(midi_desc));
}

static SYN_Status queue_midi_packet(SYN_USB_MIDI *midi, uint8_t cin, uint8_t status, uint8_t param1,
                                    uint8_t param2)
{
    if (midi == NULL) {
        return SYN_INVALID_PARAM;
    }
    if (midi->tx_len + 4U > SYN_USB_MIDI_MAX_PACKET_SIZE) {
        return SYN_ERROR; /* Buffer full */
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
```


