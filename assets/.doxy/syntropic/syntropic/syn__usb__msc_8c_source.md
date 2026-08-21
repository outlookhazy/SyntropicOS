

# File syn\_usb\_msc.c

[**File List**](files.md) **>** [**drivers**](dir_5897a976633784d8820096ec35a94718.md) **>** [**syn\_usb\_msc.c**](syn__usb__msc_8c.md)

[Go to the documentation of this file](syn__usb__msc_8c.md)


```C++

#include "syn_usb_msc.h"

#include <string.h>

#define SYN_USB_MSC_CBW_SIGNATURE 0x43425355U 
#define SYN_USB_MSC_CSW_SIGNATURE 0x53425355U 
static SYN_Status usb_msc_setup_handler(void *ctx, const SYN_USB_SetupPacket *pkt, uint8_t *resp,
                                        uint16_t *rlen)
{
    if ((ctx == NULL) || (pkt == NULL) || (resp == NULL) || (rlen == NULL)) {
        return SYN_INVALID_PARAM;
    }
    /* GET_MAX_LUN request (0xFE) */
    if (pkt->bRequest == 0xFEU) {
        resp[0] = 0; /* 0 LUNs (single drive) */
        *rlen = 1;
        return SYN_OK;
    }
    return SYN_OK;
}

static void usb_msc_data_out_handler(void *ctx, uint8_t ep, const uint8_t *data, uint16_t len)
{
    SYN_USB_MSC *msc = (SYN_USB_MSC *)ctx;
    if ((msc == NULL) || (data == NULL)) {
        return;
    }
    if (ep == msc->ep_out && len >= 31U) {
        syn_usb_msc_process_cbw(msc, data);
    }
}

static void usb_msc_data_in_handler(void *ctx, uint8_t ep)
{
    SYN_USB_MSC *msc = (SYN_USB_MSC *)ctx;
    if (msc == NULL) {
        return;
    }
    if (ep == msc->ep_in) {
        msc->tx_len = 0U;
    }
}

static SYN_Status usb_msc_configured_handler(void *ctx, uint8_t config)
{
    (void)ctx;
    (void)config;
    return SYN_OK;
}

SYN_Status syn_usb_msc_init(SYN_USB_MSC *msc)
{
    if (msc == NULL) {
        return SYN_INVALID_PARAM;
    }
    memset(msc, 0, sizeof(*msc));
    msc->ep_in = 0x82U;  /* Bulk IN Endpoint 2 */
    msc->ep_out = 0x02U; /* Bulk OUT Endpoint 2 */
    msc->iface_num = 0U;
    msc->media.block_size = SYN_USB_MSC_BLOCK_SIZE;
    return SYN_OK;
}

SYN_Status syn_usb_msc_register(SYN_USB_Device *dev, SYN_USB_MSC *msc)
{
    if ((dev == NULL) || (msc == NULL)) {
        return SYN_INVALID_PARAM;
    }

    SYN_USB_ClassDriver driver;
    memset(&driver, 0, sizeof(driver));
    driver.iface_start = msc->iface_num;
    driver.iface_count = 1U;
    driver.ctx = msc;
    driver.setup = usb_msc_setup_handler;
    driver.data_out = usb_msc_data_out_handler;
    driver.data_in = usb_msc_data_in_handler;
    driver.configured = usb_msc_configured_handler;

    static const uint8_t msc_desc[9] = {0x09, 0x04, 0x00, 0x00, 0x02, 0x08, 0x06, 0x50, 0x00};
    return syn_usb_register_class(dev, &driver, msc_desc, (uint16_t)sizeof(msc_desc));
}

SYN_Status syn_usb_msc_set_media(SYN_USB_MSC *msc, const SYN_USB_MSC_Media *media)
{
    if ((msc == NULL) || (media == NULL)) {
        return SYN_INVALID_PARAM;
    }
    msc->media = *media;
    return SYN_OK;
}

SYN_Status syn_usb_msc_process_cbw(SYN_USB_MSC *msc, const uint8_t cbw_raw[31])
{
    if ((msc == NULL) || (cbw_raw == NULL)) {
        return SYN_INVALID_PARAM;
    }

    uint32_t sig = ((uint32_t)cbw_raw[0]) | (((uint32_t)cbw_raw[1]) << 8) |
                   (((uint32_t)cbw_raw[2]) << 16) | (((uint32_t)cbw_raw[3]) << 24);

    if (sig != SYN_USB_MSC_CBW_SIGNATURE) {
        return SYN_ERROR; /* Invalid CBW signature */
    }

    msc->cbw.dCBWSignature = sig;
    msc->cbw.dCBWTag = ((uint32_t)cbw_raw[4]) | (((uint32_t)cbw_raw[5]) << 8) |
                       (((uint32_t)cbw_raw[6]) << 16) | (((uint32_t)cbw_raw[7]) << 24);
    msc->cbw.dCBWDataTransferLength = ((uint32_t)cbw_raw[8]) | (((uint32_t)cbw_raw[9]) << 8) |
                                      (((uint32_t)cbw_raw[10]) << 16) |
                                      (((uint32_t)cbw_raw[11]) << 24);
    msc->cbw.bmCBWFlags = cbw_raw[12];
    msc->cbw.bCBWLUN = cbw_raw[13];
    msc->cbw.bCBWCBLength = cbw_raw[14];
    memcpy(msc->cbw.CBWCB, &cbw_raw[15], 16);

    /* Construct matching CSW response */
    msc->csw.dCSWSignature = SYN_USB_MSC_CSW_SIGNATURE;
    msc->csw.dCSWTag = msc->cbw.dCBWTag;
    msc->csw.dCSWDataResidue = 0U;
    msc->csw.bCSWStatus = 0U; /* Success */

    uint8_t opcode = msc->cbw.CBWCB[0];

    switch (opcode) {
    case SYN_SCSI_TEST_UNIT_READY:
    case SYN_SCSI_REQUEST_SENSE:
    case SYN_SCSI_INQUIRY:
    case SYN_SCSI_READ_CAPACITY_10:
    case SYN_SCSI_READ_10:
    case SYN_SCSI_WRITE_10:
        msc->csw.bCSWStatus = 0U;
        break;
    default:
        msc->csw.bCSWStatus = 1U; /* Unsupported opcode -> Command Failed */
        break;
    }

    return SYN_OK;
}
```


