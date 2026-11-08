/**
 * @file syn_usb_msc.c
 * @brief Zero-Heap USB 2.0 Mass Storage Class (MSC) Device Driver Implementation.
 * @ingroup syn_drivers
 */

#include "syn_usb_msc.h"

#include "port/syn_port_usb.h"

#include <string.h>

#define SYN_USB_MSC_CBW_SIGNATURE 0x43425355U /**< BOT CBW Magic Signature "USBC" */
#define SYN_USB_MSC_CSW_SIGNATURE 0x53425355U /**< BOT CSW Magic Signature "USBS" */
#define SYN_USB_MSC_CSW_SIZE 13U              /**< BOT CSW packet byte length */

/**
 * @brief Write a 32-bit unsigned value in little-endian order.
 * @param dst 4-byte output pointer.
 * @param val Value to serialize.
 */
static void msc_put_le32(uint8_t dst[4], uint32_t val)
{
    dst[0] = (uint8_t)(val & 0xFFU);
    dst[1] = (uint8_t)((val >> 8) & 0xFFU);
    dst[2] = (uint8_t)((val >> 16) & 0xFFU);
    dst[3] = (uint8_t)((val >> 24) & 0xFFU);
}

/**
 * @brief Read a 32-bit unsigned value in big-endian order.
 * @param src 4-byte input pointer.
 * @return Decoded 32-bit integer.
 */
static uint32_t msc_get_be32(const uint8_t src[4])
{
    return ((uint32_t)src[0] << 24) | ((uint32_t)src[1] << 16) | ((uint32_t)src[2] << 8) |
           (uint32_t)src[3];
}

/**
 * @brief Serialize and stage the 13-byte BOT Command Status Wrapper (CSW) on Bulk IN.
 * @param msc     Pointer to USB MSC instance.
 * @param status  CSW status code (0 = Passed, 1 = Failed, 2 = Phase Error).
 * @param residue Unprocessed byte count from dCBWDataTransferLength.
 */
static void msc_stage_csw(SYN_USB_MSC *msc, uint8_t status, uint32_t residue)
{
    msc->csw.dCSWSignature = SYN_USB_MSC_CSW_SIGNATURE;
    msc->csw.dCSWTag = msc->cbw.dCBWTag;
    msc->csw.dCSWDataResidue = residue;
    msc->csw.bCSWStatus = status;

    msc_put_le32(&msc->tx_buf[0], msc->csw.dCSWSignature);
    msc_put_le32(&msc->tx_buf[4], msc->csw.dCSWTag);
    msc_put_le32(&msc->tx_buf[8], msc->csw.dCSWDataResidue);
    msc->tx_buf[12] = msc->csw.bCSWStatus;
    msc->tx_len = SYN_USB_MSC_CSW_SIZE;
    msc->bot_state = SYN_USB_MSC_BOT_CSW;
    (void)syn_port_usb_ep_write(msc->ep_in, msc->tx_buf, msc->tx_len);
}

/**
 * @brief STALL the active Bulk data endpoint when failing or short-ending a CBW with data phase.
 * @param msc Pointer to USB MSC instance.
 */
static void msc_stall_cbw_pipe(const SYN_USB_MSC *msc)
{
    if (msc->cbw.dCBWDataTransferLength > 0U) {
        uint8_t ep = ((msc->cbw.bmCBWFlags & 0x80U) != 0U) ? msc->ep_in : msc->ep_out;
        (void)syn_port_usb_ep_stall(ep);
    }
}

/**
 * @brief Record SCSI sense codes, STALL pending data pipe if needed, and stage CSW.
 * @param msc        Pointer to USB MSC instance.
 * @param csw_status CSW status byte (0=Passed, 1=Failed, 2=Phase Error).
 * @param sense_key  SCSI sense key.
 * @param sense_asc  SCSI additional sense code.
 */
static void msc_fail_cbw(SYN_USB_MSC *msc, uint8_t csw_status, uint8_t sense_key, uint8_t sense_asc)
{
    msc->sense_key = sense_key;
    msc->sense_asc = sense_asc;
    msc_stall_cbw_pipe(msc);
    msc_stage_csw(msc, csw_status, msc->cbw.dCBWDataTransferLength);
}

/**
 * @brief Stage the next Bulk IN packet (up to 64B) or transition to CSW when complete.
 * @param msc Pointer to USB MSC instance.
 */
static void msc_stage_next_in(SYN_USB_MSC *msc)
{
    if (msc->sector_offset >= msc->data_in_len) {
        if (msc->xfer_blocks_rem == 0U) {
            if (msc->csw.dCSWDataResidue > 0U) {
                (void)syn_port_usb_ep_stall(msc->ep_in);
            }
            msc_stage_csw(msc, msc->csw.bCSWStatus, msc->csw.dCSWDataResidue);
            return;
        }
        if (msc->media.read_blocks(msc->xfer_lba, msc->sector_buf, 1U) != SYN_OK) {
            uint32_t rem_bytes = (uint32_t)msc->xfer_blocks_rem * SYN_USB_MSC_BLOCK_SIZE;
            msc->sense_key = SYN_SCSI_SENSE_MEDIUM_ERROR;
            msc->sense_asc = 0x11U;
            msc->xfer_blocks_rem = 0U;
            (void)syn_port_usb_ep_stall(msc->ep_in);
            msc_stage_csw(msc, 1U, msc->csw.dCSWDataResidue + rem_bytes);
            return;
        }
        msc->xfer_lba++;
        msc->xfer_blocks_rem--;
        msc->sector_offset = 0U;
        msc->data_in_len = SYN_USB_MSC_BLOCK_SIZE;
    }

    uint16_t rem = (uint16_t)(msc->data_in_len - msc->sector_offset);
    uint16_t chunk = (rem > SYN_USB_MSC_MAX_PACKET_SIZE) ? SYN_USB_MSC_MAX_PACKET_SIZE : rem;
    memcpy(msc->tx_buf, &msc->sector_buf[msc->sector_offset], chunk);
    msc->tx_len = chunk;
    msc->sector_offset = (uint16_t)(msc->sector_offset + chunk);
    msc->bot_state = SYN_USB_MSC_BOT_DATA_IN;
    (void)syn_port_usb_ep_write(msc->ep_in, msc->tx_buf, msc->tx_len);
}

/**
 * @brief Begin a short non-block SCSI DATA_IN response from msc->sector_buf.
 * @param msc      Pointer to USB MSC instance.
 * @param resp_len Total prepared response bytes in msc->sector_buf.
 */
static void msc_start_short_in(SYN_USB_MSC *msc, uint16_t resp_len)
{
    uint32_t host_len = msc->cbw.dCBWDataTransferLength;
    if (host_len == 0U) {
        msc_stage_csw(msc, 0U, 0U);
        return;
    }
    if ((msc->cbw.bmCBWFlags & 0x80U) == 0U) {
        msc_fail_cbw(msc, 2U, SYN_SCSI_SENSE_ILLEGAL_REQUEST, 0x20U);
        return;
    }

    uint16_t send_len = (host_len < (uint32_t)resp_len) ? (uint16_t)host_len : resp_len;
    msc->csw.bCSWStatus = 0U;
    msc->csw.dCSWDataResidue = host_len - (uint32_t)send_len;
    msc->xfer_blocks_rem = 0U;
    msc->sector_offset = 0U;
    msc->data_in_len = send_len;
    msc_stage_next_in(msc);
}

/**
 * @brief Validate LBA range for a SCSI READ_10 or WRITE_10 command.
 * @param msc    Pointer to USB MSC instance.
 * @param lba    Starting logical block address.
 * @param blocks Number of blocks (> 0).
 * @return true if range fits within msc->media.block_count.
 */
static bool msc_lba_range_valid(const SYN_USB_MSC *msc, uint32_t lba, uint16_t blocks)
{
    return (lba < msc->media.block_count) && ((uint32_t)blocks <= (msc->media.block_count - lba));
}

/**
 * @brief Handle SCSI READ (10) command.
 * @param msc Pointer to USB MSC instance.
 */
static void msc_handle_read10(SYN_USB_MSC *msc)
{
    uint32_t lba = msc_get_be32(&msc->cbw.CBWCB[2]);
    uint16_t blocks = (uint16_t)(((uint16_t)msc->cbw.CBWCB[7] << 8) | msc->cbw.CBWCB[8]);
    uint32_t host_len = msc->cbw.dCBWDataTransferLength;
    bool dir_in = (msc->cbw.bmCBWFlags & 0x80U) != 0U;

    if (blocks == 0U) {
        uint8_t csw_st = ((host_len > 0U) && !dir_in) ? 2U : 0U;
        msc_fail_cbw(msc, csw_st, SYN_SCSI_SENSE_NONE, 0x00U);
        return;
    }

    uint32_t expected = (uint32_t)blocks * SYN_USB_MSC_BLOCK_SIZE;
    if ((host_len < expected) || !dir_in) {
        msc_fail_cbw(msc, 2U, SYN_SCSI_SENSE_ILLEGAL_REQUEST, 0x20U);
        return;
    }

    if ((msc->media.read_blocks == NULL) || !msc_lba_range_valid(msc, lba, blocks)) {
        msc_fail_cbw(msc, 1U, SYN_SCSI_SENSE_ILLEGAL_REQUEST, 0x21U);
        return;
    }

    msc->sense_key = SYN_SCSI_SENSE_NONE;
    msc->sense_asc = 0x00U;
    msc->csw.bCSWStatus = 0U;
    msc->csw.dCSWDataResidue = host_len - expected;
    msc->xfer_lba = lba;
    msc->xfer_blocks_rem = blocks;
    msc->sector_offset = 0U;
    msc->data_in_len = 0U;
    msc_stage_next_in(msc);
}

/**
 * @brief Handle SCSI WRITE (10) command.
 * @param msc Pointer to USB MSC instance.
 */
static void msc_handle_write10(SYN_USB_MSC *msc)
{
    uint32_t lba = msc_get_be32(&msc->cbw.CBWCB[2]);
    uint16_t blocks = (uint16_t)(((uint16_t)msc->cbw.CBWCB[7] << 8) | msc->cbw.CBWCB[8]);
    uint32_t host_len = msc->cbw.dCBWDataTransferLength;
    bool dir_in = (msc->cbw.bmCBWFlags & 0x80U) != 0U;

    if (blocks == 0U) {
        uint8_t csw_st = ((host_len > 0U) && dir_in) ? 2U : 0U;
        msc_fail_cbw(msc, csw_st, SYN_SCSI_SENSE_NONE, 0x00U);
        return;
    }

    uint32_t expected = (uint32_t)blocks * SYN_USB_MSC_BLOCK_SIZE;
    if ((host_len < expected) || dir_in) {
        msc_fail_cbw(msc, 2U, SYN_SCSI_SENSE_ILLEGAL_REQUEST, 0x20U);
        return;
    }

    if ((msc->media.write_blocks == NULL) || !msc_lba_range_valid(msc, lba, blocks)) {
        msc_fail_cbw(msc, 1U, SYN_SCSI_SENSE_ILLEGAL_REQUEST, 0x21U);
        return;
    }

    msc->sense_key = SYN_SCSI_SENSE_NONE;
    msc->sense_asc = 0x00U;
    msc->csw.bCSWStatus = 0U;
    msc->csw.dCSWDataResidue = host_len - expected;
    msc->xfer_lba = lba;
    msc->xfer_blocks_rem = blocks;
    msc->sector_offset = 0U;
    msc->bot_state = SYN_USB_MSC_BOT_DATA_OUT;
}

/**
 * @brief Handle SCSI REQUEST SENSE (0x03) command with CDB allocation length clamping.
 * @param msc Pointer to USB MSC instance.
 */
static void msc_handle_request_sense(SYN_USB_MSC *msc)
{
    uint16_t alloc_len = msc->cbw.CBWCB[4];
    uint16_t resp_len = (alloc_len < 18U) ? alloc_len : 18U;
    memset(msc->sector_buf, 0, 18U);
    msc->sector_buf[0] = 0x70U;
    msc->sector_buf[2] = msc->sense_key;
    msc->sector_buf[7] = 10U;
    msc->sector_buf[12] = msc->sense_asc;
    msc->sense_key = SYN_SCSI_SENSE_NONE;
    msc->sense_asc = 0x00U;
    msc_start_short_in(msc, resp_len);
}

/**
 * @brief Handle SCSI INQUIRY (0x12) command, rejecting EVPD/PageCode and clamping alloc_len.
 * @param msc Pointer to USB MSC instance.
 */
static void msc_handle_inquiry(SYN_USB_MSC *msc)
{
    if (((msc->cbw.CBWCB[1] & 0x01U) != 0U) || (msc->cbw.CBWCB[2] != 0U)) {
        msc_fail_cbw(msc, 1U, SYN_SCSI_SENSE_ILLEGAL_REQUEST, 0x24U);
        return;
    }

    uint16_t alloc_len = (uint16_t)(((uint16_t)msc->cbw.CBWCB[3] << 8) | msc->cbw.CBWCB[4]);
    uint16_t resp_len = (alloc_len < 36U) ? alloc_len : 36U;
    memset(msc->sector_buf, 0, 36U);
    msc->sector_buf[1] = 0x80U;
    msc->sector_buf[2] = 0x04U;
    msc->sector_buf[3] = 0x02U;
    msc->sector_buf[4] = 31U;
    memcpy(&msc->sector_buf[8], "SYNTROPI", 8U);
    memcpy(&msc->sector_buf[16], "USB MSC STORAGE ", 16U);
    memcpy(&msc->sector_buf[32], "1.00", 4U);
    msc->sense_key = SYN_SCSI_SENSE_NONE;
    msc->sense_asc = 0x00U;
    msc_start_short_in(msc, resp_len);
}

/**
 * @brief Handle SCSI MODE SENSE (6) (0x1A) command with CDB allocation length clamping.
 * @param msc Pointer to USB MSC instance.
 */
static void msc_handle_mode_sense6(SYN_USB_MSC *msc)
{
    uint16_t alloc_len = msc->cbw.CBWCB[4];
    uint16_t resp_len = (alloc_len < 4U) ? alloc_len : 4U;
    msc->sector_buf[0] = 0x03U;
    msc->sector_buf[1] = 0x00U;
    msc->sector_buf[2] = (msc->media.write_blocks == NULL) ? 0x80U : 0x00U;
    msc->sector_buf[3] = 0x00U;
    msc->sense_key = SYN_SCSI_SENSE_NONE;
    msc->sense_asc = 0x00U;
    msc_start_short_in(msc, resp_len);
}

/**
 * @brief Handle SCSI READ CAPACITY (10) command.
 * @param msc Pointer to USB MSC instance.
 */
static void msc_handle_read_capacity10(SYN_USB_MSC *msc)
{
    uint32_t last_lba = (msc->media.block_count > 0U) ? (msc->media.block_count - 1U) : 0U;
    uint32_t blk_sz = (msc->media.block_size > 0U) ? (uint32_t)msc->media.block_size : 512U;
    msc->sector_buf[0] = (uint8_t)((last_lba >> 24) & 0xFFU);
    msc->sector_buf[1] = (uint8_t)((last_lba >> 16) & 0xFFU);
    msc->sector_buf[2] = (uint8_t)((last_lba >> 8) & 0xFFU);
    msc->sector_buf[3] = (uint8_t)(last_lba & 0xFFU);
    msc->sector_buf[4] = (uint8_t)((blk_sz >> 24) & 0xFFU);
    msc->sector_buf[5] = (uint8_t)((blk_sz >> 16) & 0xFFU);
    msc->sector_buf[6] = (uint8_t)((blk_sz >> 8) & 0xFFU);
    msc->sector_buf[7] = (uint8_t)(blk_sz & 0xFFU);
    msc->sense_key = SYN_SCSI_SENSE_NONE;
    msc->sense_asc = 0x00U;
    msc_start_short_in(msc, 8U);
}

/**
 * @brief Stream Bulk OUT bytes into sector_buf and commit 512B blocks via write_blocks.
 * @param msc  Pointer to USB MSC instance.
 * @param data Received Bulk OUT payload.
 * @param len  Byte length of received payload.
 */
static void msc_handle_data_out_stream(SYN_USB_MSC *msc, const uint8_t *data, uint16_t len)
{
    uint16_t pos = 0U;

    while ((pos < len) && (msc->xfer_blocks_rem > 0U)) {
        uint16_t space = (uint16_t)(SYN_USB_MSC_BLOCK_SIZE - msc->sector_offset);
        uint16_t rem = (uint16_t)(len - pos);
        uint16_t copy = (rem < space) ? rem : space;

        memcpy(&msc->sector_buf[msc->sector_offset], &data[pos], copy);
        msc->sector_offset = (uint16_t)(msc->sector_offset + copy);
        pos = (uint16_t)(pos + copy);

        if (msc->sector_offset == SYN_USB_MSC_BLOCK_SIZE) {
            msc->sector_offset = 0U;
            if (msc->media.write_blocks(msc->xfer_lba, msc->sector_buf, 1U) != SYN_OK) {
                uint32_t rem_bytes = (uint32_t)msc->xfer_blocks_rem * SYN_USB_MSC_BLOCK_SIZE;
                msc->sense_key = SYN_SCSI_SENSE_MEDIUM_ERROR;
                msc->sense_asc = 0x0CU;
                msc->xfer_blocks_rem = 0U;
                (void)syn_port_usb_ep_stall(msc->ep_out);
                msc_stage_csw(msc, 1U, msc->csw.dCSWDataResidue + rem_bytes);
                return;
            }
            msc->xfer_lba++;
            msc->xfer_blocks_rem--;
        }
    }

    if (msc->xfer_blocks_rem == 0U) {
        if (msc->csw.dCSWDataResidue > 0U) {
            (void)syn_port_usb_ep_stall(msc->ep_out);
        }
        msc_stage_csw(msc, 0U, msc->csw.dCSWDataResidue);
    }
}

/**
 * @brief Setup handler callback for USB MSC class setup requests.
 * @param ctx      Class driver context (SYN_USB_MSC pointer).
 * @param pkt      Setup packet pointer.
 * @param resp     Response payload buffer.
 * @param resp_cap Capacity of @p resp in bytes.
 * @param rlen     Output response payload byte length pointer.
 * @return SYN_OK on success, SYN_INVALID_PARAM if NULL, SYN_ERROR if response exceeds @p resp_cap.
 */
static SYN_Status usb_msc_setup_handler(void *ctx, const SYN_USB_SetupPacket *pkt, uint8_t *resp,
                                        uint16_t resp_cap, uint16_t *rlen)
{
    SYN_USB_MSC *msc = (SYN_USB_MSC *)ctx;
    if ((msc == NULL) || (pkt == NULL) || (resp == NULL) || (rlen == NULL)) {
        return SYN_INVALID_PARAM;
    }

    *rlen = 0U;
    if (pkt->bRequest == 0xFEU) {
        if (resp_cap < 1U) {
            return SYN_ERROR;
        }
        resp[0] = 0U;
        *rlen = 1U;
    } else if (pkt->bRequest == 0xFFU) {
        msc->bot_state = SYN_USB_MSC_BOT_IDLE;
        msc->tx_len = 0U;
        msc->xfer_blocks_rem = 0U;
        msc->sector_offset = 0U;
        msc->data_in_len = 0U;
    }
    return SYN_OK;
}

/**
 * @brief Data OUT handler callback for USB MSC OUT endpoint data.
 * @param ctx Class driver context (SYN_USB_MSC pointer).
 * @param ep Endpoint address.
 * @param data Received payload bytes.
 * @param len Byte length of received payload.
 */
static void usb_msc_data_out_handler(void *ctx, uint8_t ep, const uint8_t *data, uint16_t len)
{
    SYN_USB_MSC *msc = (SYN_USB_MSC *)ctx;
    if ((msc == NULL) || (data == NULL) || (ep != msc->ep_out)) {
        return;
    }

    if (msc->bot_state == SYN_USB_MSC_BOT_DATA_OUT) {
        msc_handle_data_out_stream(msc, data, len);
        return;
    }

    if (len >= 31U) {
        (void)syn_usb_msc_process_cbw(msc, data);
    }
}

/**
 * @brief Data IN handler callback for USB MSC IN endpoint completion.
 * @param ctx Class driver context (SYN_USB_MSC pointer).
 * @param ep Endpoint address.
 */
static void usb_msc_data_in_handler(void *ctx, uint8_t ep)
{
    SYN_USB_MSC *msc = (SYN_USB_MSC *)ctx;
    if ((msc == NULL) || (ep != msc->ep_in)) {
        return;
    }

    msc->tx_len = 0U;
    if (msc->bot_state == SYN_USB_MSC_BOT_DATA_IN) {
        msc_stage_next_in(msc);
    } else {
        msc->bot_state = SYN_USB_MSC_BOT_IDLE;
    }
}

/**
 * @brief Configured callback for USB MSC class interface activation.
 * @param ctx Class driver context (SYN_USB_MSC pointer).
 * @param config Configuration index.
 * @return SYN_OK on success.
 */
static SYN_Status usb_msc_configured_handler(void *ctx, uint8_t config)
{
    SYN_USB_MSC *msc = (SYN_USB_MSC *)ctx;
    (void)config;
    if (msc != NULL) {
        msc->bot_state = SYN_USB_MSC_BOT_IDLE;
        msc->tx_len = 0U;
        msc->xfer_blocks_rem = 0U;
        msc->sector_offset = 0U;
        msc->data_in_len = 0U;
    }
    return SYN_OK;
}

SYN_Status syn_usb_msc_init(SYN_USB_MSC *msc)
{
    if (msc == NULL) {
        return SYN_INVALID_PARAM;
    }
    memset(msc, 0, sizeof(*msc));
    msc->ep_in = 0x82U;
    msc->ep_out = 0x02U;
    msc->iface_num = 0U;
    msc->bot_state = SYN_USB_MSC_BOT_IDLE;
    msc->media.block_size = SYN_USB_MSC_BLOCK_SIZE;
    return SYN_OK;
}

SYN_Status syn_usb_msc_get_config_desc(const SYN_USB_MSC *msc,
                                       uint8_t desc_out[SYN_USB_MSC_DESC_SIZE])
{
    if ((msc == NULL) || (desc_out == NULL)) {
        return SYN_INVALID_PARAM;
    }

    uint8_t tmpl[SYN_USB_MSC_DESC_SIZE] = {
        /* Interface Descriptor (9B): Mass Storage, SCSI Transparent, Bulk-Only */
        0x09U, 0x04U, msc->iface_num, 0x00U, 0x02U, 0x08U, 0x06U, 0x50U, 0x00U,
        /* Bulk IN Endpoint Descriptor (7B) */
        0x07U, 0x05U, msc->ep_in, 0x02U, 0x40U, 0x00U, 0x00U,
        /* Bulk OUT Endpoint Descriptor (7B) */
        0x07U, 0x05U, msc->ep_out, 0x02U, 0x40U, 0x00U, 0x00U};

    memcpy(desc_out, tmpl, sizeof(tmpl));
    return SYN_OK;
}

SYN_Status syn_usb_msc_register(SYN_USB_Device *dev, SYN_USB_MSC *msc)
{
    if ((dev == NULL) || (msc == NULL)) {
        return SYN_INVALID_PARAM;
    }

    uint8_t msc_desc[SYN_USB_MSC_DESC_SIZE];
    (void)syn_usb_msc_get_config_desc(msc, msc_desc);

    SYN_USB_ClassDriver driver;
    memset(&driver, 0, sizeof(driver));
    driver.iface_start = msc->iface_num;
    driver.iface_count = 1U;
    driver.ctx = msc;
    driver.setup = usb_msc_setup_handler;
    driver.data_out = usb_msc_data_out_handler;
    driver.data_in = usb_msc_data_in_handler;
    driver.configured = usb_msc_configured_handler;

    SYN_Status st = syn_usb_register_class(dev, &driver, msc_desc, SYN_USB_MSC_DESC_SIZE);
    if (st == SYN_OK) {
        msc->iface_num = dev->classes[dev->class_count - 1U].iface_start;
        if (!dev->raw_config_override) {
            uint16_t base = (uint16_t)(dev->config_buf_used - SYN_USB_MSC_DESC_SIZE);
            msc->ep_in = dev->config_buf[base + 11U];
            msc->ep_out = dev->config_buf[base + 18U];
        }
    }
    return st;
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
        return SYN_ERROR;
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
    memcpy(msc->cbw.CBWCB, &cbw_raw[15], 16U);

    switch (msc->cbw.CBWCB[0]) {
    case SYN_SCSI_TEST_UNIT_READY: {
        bool bad_dir =
            (msc->cbw.dCBWDataTransferLength > 0U) && ((msc->cbw.bmCBWFlags & 0x80U) == 0U);
        msc_fail_cbw(msc, bad_dir ? 2U : 0U, SYN_SCSI_SENSE_NONE, 0x00U);
        break;
    }

    case SYN_SCSI_REQUEST_SENSE:
        msc_handle_request_sense(msc);
        break;

    case SYN_SCSI_INQUIRY:
        msc_handle_inquiry(msc);
        break;

    case SYN_SCSI_MODE_SENSE_6:
        msc_handle_mode_sense6(msc);
        break;

    case SYN_SCSI_READ_CAPACITY_10:
        msc_handle_read_capacity10(msc);
        break;

    case SYN_SCSI_READ_10:
        msc_handle_read10(msc);
        break;

    case SYN_SCSI_WRITE_10:
        msc_handle_write10(msc);
        break;

    default:
        msc_fail_cbw(msc, 1U, SYN_SCSI_SENSE_ILLEGAL_REQUEST, 0x20U);
        break;
    }

    return SYN_OK;
}
