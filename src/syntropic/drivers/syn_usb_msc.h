/**
 * @file syn_usb_msc.h
 * @brief Zero-Heap USB 2.0 Mass Storage Class (MSC) Device Driver.
 * @ingroup syn_drivers
 *
 * Implements USB Mass Storage Class Bulk-Only Transport (BOT) v1.0 with
 * SCSI Transparent Command Set to expose SD Card or Flash storage as a USB drive.
 */

#ifndef SYN_USB_MSC_H
#define SYN_USB_MSC_H

#include "syntropic/common/syn_defs.h"
#include "syntropic/drivers/syn_usb.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SYN_USB_MSC_BLOCK_SIZE 512U     /**< Standard SCSI block sector size in bytes */
#define SYN_USB_MSC_MAX_PACKET_SIZE 64U /**< Maximum USB endpoint packet size */
#define SYN_USB_MSC_DESC_SIZE 23U       /**< Interface (9B) + Bulk IN (7B) + Bulk OUT (7B) */

/** SCSI Command Opcodes */
#define SYN_SCSI_TEST_UNIT_READY 0x00U  /**< SCSI Test Unit Ready opcode (0x00) */
#define SYN_SCSI_REQUEST_SENSE 0x03U    /**< SCSI Request Sense opcode (0x03) */
#define SYN_SCSI_INQUIRY 0x12U          /**< SCSI Inquiry opcode (0x12) */
#define SYN_SCSI_MODE_SENSE_6 0x1AU     /**< SCSI Mode Sense (6) opcode (0x1A) */
#define SYN_SCSI_READ_CAPACITY_10 0x25U /**< SCSI Read Capacity (10) opcode (0x25) */
#define SYN_SCSI_READ_10 0x28U          /**< SCSI Read (10) opcode (0x28) */
#define SYN_SCSI_WRITE_10 0x2AU         /**< SCSI Write (10) opcode (0x2A) */

/** SCSI Sense Key Codes */
#define SYN_SCSI_SENSE_NONE 0x00U            /**< No Sense (0x00) */
#define SYN_SCSI_SENSE_MEDIUM_ERROR 0x03U    /**< Medium Error (0x03) */
#define SYN_SCSI_SENSE_ILLEGAL_REQUEST 0x05U /**< Illegal Request (0x05) */

/** Bulk-Only Transport (BOT) Protocol States */
#define SYN_USB_MSC_BOT_IDLE 0U     /**< Waiting for 31-byte CBW on Bulk OUT */
#define SYN_USB_MSC_BOT_DATA_IN 1U  /**< Streaming IN payload to host */
#define SYN_USB_MSC_BOT_DATA_OUT 2U /**< Receiving OUT payload from host */
#define SYN_USB_MSC_BOT_CSW 3U      /**< 13-byte CSW staged on Bulk IN */

/** Command Block Wrapper (CBW) per USB BOT Spec §5.1 */
typedef struct {
    uint32_t dCBWSignature;          /**< 0x43425355 ("USBC") */
    uint32_t dCBWTag;                /**< Command Block Tag */
    uint32_t dCBWDataTransferLength; /**< Payload transfer length */
    uint8_t bmCBWFlags;              /**< Direction flag (0x80 IN, 0x00 OUT) */
    uint8_t bCBWLUN;                 /**< Logical Unit Number */
    uint8_t bCBWCBLength;            /**< SCSI Command length */
    uint8_t CBWCB[16];               /**< Raw SCSI Command block */
} SYN_USB_MSC_CBW;

/** Command Status Wrapper (CSW) per USB BOT Spec §5.2 */
typedef struct {
    uint32_t dCSWSignature;   /**< 0x53425355 ("USBS") */
    uint32_t dCSWTag;         /**< Matches dCBWTag */
    uint32_t dCSWDataResidue; /**< Difference in expected/actual bytes */
    uint8_t bCSWStatus;       /**< 0=Success, 1=Failed, 2=Phase Error */
} SYN_USB_MSC_CSW;

/** USB MSC Block Storage Callbacks */
typedef struct {
    uint32_t block_count; /**< Total sectors */
    uint16_t block_size;  /**< Sector size in bytes (512) */
    SYN_Status (*read_blocks)(uint32_t lba, uint8_t *buf,
                              uint16_t count); /**< Block read callback */
    SYN_Status (*write_blocks)(uint32_t lba, const uint8_t *buf,
                               uint16_t count); /**< Block write callback */
} SYN_USB_MSC_Media;

/** USB MSC Instance Context */
typedef struct {
    uint8_t ep_in;                               /**< Bulk IN Endpoint address */
    uint8_t ep_out;                              /**< Bulk OUT Endpoint address */
    uint8_t iface_num;                           /**< Assigned interface index */
    uint8_t bot_state;                           /**< Active BOT state (SYN_USB_MSC_BOT_*) */
    uint8_t sense_key;                           /**< Pending SCSI Sense Key */
    uint8_t sense_asc;                           /**< Pending SCSI Additional Sense Code */
    uint16_t xfer_blocks_rem;                    /**< Remaining sectors in READ_10/WRITE_10 */
    uint16_t sector_offset;                      /**< Byte offset within sector_buf */
    uint16_t data_in_len;                        /**< Staged byte count in sector_buf */
    uint32_t xfer_lba;                           /**< Next LBA for active block transfer */
    SYN_USB_MSC_Media media;                     /**< Bound storage media device */
    SYN_USB_MSC_CBW cbw;                         /**< Active SCSI CBW command */
    SYN_USB_MSC_CSW csw;                         /**< Pending SCSI CSW status */
    uint8_t tx_buf[SYN_USB_MSC_MAX_PACKET_SIZE]; /**< IN packet buffer */
    uint16_t tx_len;                             /**< Pending IN byte length */
    uint8_t sector_buf[SYN_USB_MSC_BLOCK_SIZE];  /**< Single-sector BOT staging buffer */
} SYN_USB_MSC;

/**
 * @brief Initialize USB MSC Instance.
 * @param msc Pointer to USB MSC instance.
 * @return SYN_OK on success, SYN_INVALID_PARAM if NULL.
 */
SYN_Status syn_usb_msc_init(SYN_USB_MSC *msc);

/**
 * @brief Build the 23-byte MSC Interface + Bulk IN + Bulk OUT descriptor block.
 * @param msc      Pointer to USB MSC instance.
 * @param desc_out Output 23-byte buffer.
 * @return SYN_OK on success, SYN_INVALID_PARAM if NULL.
 */
SYN_Status syn_usb_msc_get_config_desc(const SYN_USB_MSC *msc,
                                       uint8_t desc_out[SYN_USB_MSC_DESC_SIZE]);

/**
 * @brief Register USB MSC class driver with USB core.
 * @param dev Pointer to USB device context.
 * @param msc Pointer to USB MSC instance.
 * @return SYN_OK on success, SYN_INVALID_PARAM if NULL.
 */
SYN_Status syn_usb_msc_register(SYN_USB_Device *dev, SYN_USB_MSC *msc);

/**
 * @brief Bind block storage media callbacks.
 * @param msc Pointer to USB MSC instance.
 * @param media Pointer to storage media interface.
 * @return SYN_OK on success, SYN_INVALID_PARAM if NULL.
 */
SYN_Status syn_usb_msc_set_media(SYN_USB_MSC *msc, const SYN_USB_MSC_Media *media);

/**
 * @brief Process incoming BOT Command Block Wrapper (CBW).
 * @param msc Pointer to USB MSC instance.
 * @param cbw_raw Pointer to 31-byte raw CBW buffer.
 * @return SYN_OK on success, SYN_INVALID_PARAM if NULL, SYN_ERROR if signature invalid.
 */
SYN_Status syn_usb_msc_process_cbw(SYN_USB_MSC *msc, const uint8_t cbw_raw[31]);

#ifdef __cplusplus
}
#endif

#endif /* SYN_USB_MSC_H */
