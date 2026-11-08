/**
 * @file syn_usb_cdc.c
 * @brief Zero-Heap USB 2.0 CDC ACM Virtual COM Port Class Engine Implementation.
 */

#include "syntropic/drivers/syn_usb_cdc.h"

#include "syntropic/port/syn_port_system.h"

#include <string.h>

/** Standard CDC Interface Descriptor Assembly (2 interfaces, 3 endpoints, 66 bytes) */
static const uint8_t CDC_INTERFACE_DESC[66] = {
    /* Interface Association Descriptor (IAD) */
    0x08U, 0x0BU, 0x00U, 0x02U, 0x02U, 0x02U, 0x01U, 0x00U,

    /* Interface 0: Communication Interface Class */
    0x09U, 0x04U, 0x00U, 0x00U, 0x01U, 0x02U, 0x02U, 0x01U, 0x00U,
    /* Header Functional Descriptor */
    0x05U, 0x24U, 0x00U, 0x10U, 0x01U,
    /* Call Management Functional Descriptor */
    0x05U, 0x24U, 0x01U, 0x00U, 0x01U,
    /* ACM Functional Descriptor */
    0x04U, 0x24U, 0x02U, 0x02U,
    /* Union Functional Descriptor */
    0x05U, 0x24U, 0x06U, 0x00U, 0x01U,
    /* Endpoint 0x82: Interrupt IN */
    0x07U, 0x05U, 0x82U, 0x03U, 0x08U, 0x00U, 0x10U,

    /* Interface 1: Data Interface Class */
    0x09U, 0x04U, 0x01U, 0x00U, 0x02U, 0x0AU, 0x00U, 0x00U, 0x00U,
    /* Endpoint 0x01: Bulk OUT */
    0x07U, 0x05U, 0x01U, 0x02U, 0x40U, 0x00U, 0x00U,
    /* Endpoint 0x81: Bulk IN */
    0x07U, 0x05U, 0x81U, 0x02U, 0x40U, 0x00U, 0x00U};

/**
 * @brief Setup request callback wrapper for CDC class requests.
 *
 * @param ctx      Pointer to SYN_USB_CDC instance.
 * @param pkt      Pointer to received Setup Packet.
 * @param resp     Output buffer for data stage response.
 * @param resp_cap Capacity of @p resp in bytes.
 * @param rlen     Pointer to receive response length.
 * @return SYN_OK on success, SYN_ERROR if response exceeds @p resp_cap.
 */
static SYN_Status cdc_class_setup(void *ctx, const SYN_USB_SetupPacket *pkt, uint8_t *resp,
                                  uint16_t resp_cap, uint16_t *rlen)
{
    SYN_USB_CDC *cdc = (SYN_USB_CDC *)ctx;
    size_t size_rlen = 0U;
    SYN_Status st =
        ((pkt != NULL) && (pkt->bRequest == SYN_USB_CDC_GET_LINE_CODING) && (resp_cap < 7U))
            ? SYN_ERROR
            : syn_usb_cdc_handle_setup(cdc, pkt, resp, &size_rlen);
    if (rlen != NULL) {
        *rlen = (uint16_t)size_rlen;
    }
    return st;
}

/**
 * @brief Data OUT handler callback for CDC Bulk OUT endpoint.
 *
 * @param ctx  Pointer to SYN_USB_CDC instance.
 * @param ep   Endpoint address.
 * @param data Received payload bytes.
 * @param len  Byte length of received payload.
 */
static void cdc_class_data_out(void *ctx, uint8_t ep, const uint8_t *data, uint16_t len)
{
    SYN_USB_CDC *cdc = (SYN_USB_CDC *)ctx;
    if ((cdc == NULL) || (data == NULL) || (len == 0U) || (ep != cdc->ep_out)) {
        return;
    }
    syn_port_enter_critical();
    uint16_t space = (uint16_t)(sizeof(cdc->rx_buf) - cdc->rx_len);
    uint16_t copy = (len < space) ? len : space;
    if (copy > 0U) {
        memcpy(&cdc->rx_buf[cdc->rx_len], data, copy);
        cdc->rx_len = (uint16_t)(cdc->rx_len + copy);
    }
    syn_port_exit_critical();
}

/**
 * @brief Data IN completion callback for CDC Bulk IN endpoint.
 *
 * @param ctx Pointer to SYN_USB_CDC instance.
 * @param ep  Endpoint address.
 */
static void cdc_class_data_in(void *ctx, uint8_t ep)
{
    SYN_USB_CDC *cdc = (SYN_USB_CDC *)ctx;
    if ((cdc != NULL) && (ep == cdc->ep_in)) {
        cdc->tx_len = 0U;
    }
}

/**
 * @brief Configured callback wrapper for CDC class.
 *
 * @param ctx Pointer to SYN_USB_CDC instance.
 * @param config Configuration index (0 = unconfigured).
 * @return SYN_OK on success.
 */
static SYN_Status cdc_class_configured(void *ctx, uint8_t config)
{
    SYN_USB_CDC *cdc = (SYN_USB_CDC *)ctx;
    if (cdc != NULL) {
        cdc->configured = (config != 0U);
        cdc->tx_len = 0U;
    }
    return SYN_OK;
}

SYN_Status syn_usb_cdc_init(SYN_USB_CDC *cdc)
{
    if (cdc == NULL) {
        return SYN_INVALID_PARAM;
    }
    memset(cdc, 0, sizeof(*cdc));

    cdc->ep_in = 0x81U;
    cdc->ep_out = 0x01U;
    cdc->ep_cmd = 0x82U;
    cdc->configured = false;

    cdc->line_coding.baudrate = 115200U;
    cdc->line_coding.stop_bits = 0U;
    cdc->line_coding.parity = 0U;
    cdc->line_coding.data_bits = 8U;

    return SYN_OK;
}

SYN_Status syn_usb_cdc_register(SYN_USB_Device *dev, SYN_USB_CDC *cdc)
{
    if ((dev == NULL) || (cdc == NULL)) {
        return SYN_INVALID_PARAM;
    }

    uint8_t desc_buf[sizeof(CDC_INTERFACE_DESC)];
    memcpy(desc_buf, CDC_INTERFACE_DESC, sizeof(CDC_INTERFACE_DESC));
    desc_buf[38] = cdc->ep_cmd;
    desc_buf[54] = cdc->ep_out;
    desc_buf[61] = cdc->ep_in;

    SYN_USB_ClassDriver cls;
    memset(&cls, 0, sizeof(cls));
    cls.iface_start = cdc->iface_num;
    cls.iface_count = 2U;
    cls.ctx = cdc;
    cls.setup = cdc_class_setup;
    cls.data_out = cdc_class_data_out;
    cls.data_in = cdc_class_data_in;
    cls.configured = cdc_class_configured;

    SYN_Status st = syn_usb_register_class(dev, &cls, desc_buf, (uint16_t)sizeof(desc_buf));
    if (st == SYN_OK) {
        cdc->iface_num = dev->classes[dev->class_count - 1U].iface_start;
        if (!dev->raw_config_override) {
            uint16_t base = (uint16_t)(dev->config_buf_used - sizeof(desc_buf));
            cdc->ep_cmd = dev->config_buf[base + 38U];
            cdc->ep_out = dev->config_buf[base + 54U];
            cdc->ep_in = dev->config_buf[base + 61U];
        }
    }
    return st;
}

SYN_Status syn_usb_cdc_handle_setup(SYN_USB_CDC *cdc, const SYN_USB_SetupPacket *setup,
                                    uint8_t *resp, size_t *rlen)
{
    if ((cdc == NULL) || (setup == NULL) || (resp == NULL) || (rlen == NULL)) {
        return SYN_INVALID_PARAM;
    }

    *rlen = 0U;

    switch (setup->bRequest) {
    case SYN_USB_REQ_SET_ADDRESS:
        cdc->dev_address = (uint8_t)(setup->wValue & 0x7FU);
        return SYN_OK;

    case SYN_USB_REQ_SET_CONFIGURATION:
        cdc->configured = (setup->wValue != 0U);
        return SYN_OK;

    case SYN_USB_CDC_SET_LINE_CODING:
        return SYN_OK;

    case SYN_USB_CDC_GET_LINE_CODING:
        resp[0] = (uint8_t)(cdc->line_coding.baudrate & 0xFFU);
        resp[1] = (uint8_t)((cdc->line_coding.baudrate >> 8) & 0xFFU);
        resp[2] = (uint8_t)((cdc->line_coding.baudrate >> 16) & 0xFFU);
        resp[3] = (uint8_t)((cdc->line_coding.baudrate >> 24) & 0xFFU);
        resp[4] = cdc->line_coding.stop_bits;
        resp[5] = cdc->line_coding.parity;
        resp[6] = cdc->line_coding.data_bits;
        *rlen = 7U;
        return SYN_OK;

    case SYN_USB_CDC_SET_CONTROL_LINE_STATE:
        return SYN_OK;

    default:
        return SYN_OK;
    }
}

SYN_Status syn_usb_cdc_write(SYN_USB_CDC *cdc, const void *data, size_t len)
{
    if ((cdc == NULL) || (data == NULL) || (len == 0U)) {
        return SYN_INVALID_PARAM;
    }

    if (cdc->tx_len > 0U) {
        return SYN_BUSY;
    }

    if (len > sizeof(cdc->tx_buf)) {
        len = sizeof(cdc->tx_buf);
    }

    memcpy(cdc->tx_buf, data, len);
    cdc->tx_len = (uint16_t)len;

    return SYN_OK;
}

SYN_Status syn_usb_cdc_read(SYN_USB_CDC *cdc, void *buf, size_t max_len, size_t *out_len)
{
    if ((cdc == NULL) || (buf == NULL) || (out_len == NULL)) {
        return SYN_INVALID_PARAM;
    }

    syn_port_enter_critical();
    uint16_t available = cdc->rx_len;
    if (available == 0U) {
        syn_port_exit_critical();
        *out_len = 0U;
        return SYN_OK;
    }

    if (max_len < available) {
        available = (uint16_t)max_len;
    }

    memcpy(buf, cdc->rx_buf, available);
    if (available < cdc->rx_len) {
        memmove(cdc->rx_buf, &cdc->rx_buf[available], (size_t)(cdc->rx_len - available));
        cdc->rx_len = (uint16_t)(cdc->rx_len - available);
    } else {
        cdc->rx_len = 0U;
    }
    syn_port_exit_critical();

    *out_len = available;
    return SYN_OK;
}

bool syn_usb_cdc_rx_available(const SYN_USB_CDC *cdc)
{
    return (cdc != NULL) && (cdc->rx_len > 0U);
}

bool syn_usb_cdc_tx_ready(const SYN_USB_CDC *cdc)
{
    return (cdc != NULL) && (cdc->tx_len == 0U);
}
