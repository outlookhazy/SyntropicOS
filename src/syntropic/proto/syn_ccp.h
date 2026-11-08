/**
 * @file syn_ccp.h
 * @brief ASAM CAN Calibration Protocol v2.1 (CCP) Slave Implementation.
 *
 * Provides a lightweight, zero-allocation CCP v2.1 slave protocol stack for
 * ECU calibration, live parameter tuning, memory upload/download, and DAQ
 * (Data Acquisition) list streaming over CAN 2.0B.
 */

#ifndef SYN_CCP_H
#define SYN_CCP_H

#include "../common/syn_defs.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Maximum number of DAQ lists per CCP node */
#ifndef SYN_CCP_MAX_DAQ_LISTS
#define SYN_CCP_MAX_DAQ_LISTS 4U
#endif

/** @brief Maximum ODTs per DAQ list */
#ifndef SYN_CCP_MAX_ODT_PER_DAQ
#define SYN_CCP_MAX_ODT_PER_DAQ 8U
#endif

/** @brief Maximum entries per ODT */
#ifndef SYN_CCP_MAX_ENTRIES_PER_ODT
#define SYN_CCP_MAX_ENTRIES_PER_ODT 7U
#endif

/** @brief Maximum registered memory regions per CCP slave */
#ifndef SYN_CCP_MAX_MEM_REGIONS
#define SYN_CCP_MAX_MEM_REGIONS 4U
#endif

/** @name CCP Command Codes (CRO) */
/**@{*/
#define SYN_CCP_CMD_CONNECT 0x01U             /**< Establish connection */
#define SYN_CCP_CMD_EXCHANGE_ID 0x02U         /**< Exchange station ID */
#define SYN_CCP_CMD_DNLOAD 0x03U              /**< Download data block */
#define SYN_CCP_CMD_UPLOAD 0x04U              /**< Upload data block */
#define SYN_CCP_CMD_START_STOP_ALL 0x06U      /**< Start/stop all DAQ lists */
#define SYN_CCP_CMD_SET_MTA 0x08U             /**< Set memory transfer address */
#define SYN_CCP_CMD_GET_ACTIVE_CAL_PAGE 0x09U /**< Get active calibration page */
#define SYN_CCP_CMD_SHORT_UP 0x0FU            /**< Short upload */
#define SYN_CCP_CMD_BUILD_PKT 0x10U           /**< Build DAQ packet */
#define SYN_CCP_CMD_START_STOP 0x11U          /**< Start/stop single DAQ list */
#define SYN_CCP_CMD_GET_SEED 0x12U            /**< Get seed for security unlock */
#define SYN_CCP_CMD_UNLOCK 0x13U              /**< Send key to unlock resource */
#define SYN_CCP_CMD_SET_CAL_PAGE 0x14U        /**< Set calibration page */
#define SYN_CCP_CMD_SET_DAQ_SIZE 0x15U        /**< Set DAQ list size */
#define SYN_CCP_CMD_DISCONNECT 0x17U          /**< Disconnect station */
#define SYN_CCP_CMD_CLEAR_MEMORY 0x18U        /**< Clear flash/RAM memory */
#define SYN_CCP_CMD_GET_DAQ_SIZE 0x1AU        /**< Get DAQ list size */
#define SYN_CCP_CMD_GET_CCP_VERSION 0x1BU     /**< Get CCP protocol version */
#define SYN_CCP_CMD_PROGRAM 0x22U             /**< Program flash memory */
#define SYN_CCP_CMD_DNLOAD_6 0x23U            /**< Fast 6-byte download */
/**@}*/

/** @name CCP Response Packet Identifiers (DTO) */
/**@{*/
#define SYN_CCP_PID_CRM 0xFFU   /**< Command response message */
#define SYN_CCP_PID_EVENT 0xFEU /**< Event packet */
/**@}*/

/** @name CCP Error Return Codes */
/**@{*/
#define SYN_CCP_ERR_SUCCESS 0x00U            /**< Action succeeded */
#define SYN_CCP_ERR_CMD_UNKNOWN 0x01U        /**< Command unknown */
#define SYN_CCP_ERR_CMD_SYNTAX 0x02U         /**< Command syntax error */
#define SYN_CCP_ERR_PARAM_OUT_OF_RANGE 0x10U /**< Parameter out of range */
#define SYN_CCP_ERR_ACCESS_DENIED 0x11U      /**< Access denied */
#define SYN_CCP_ERR_NOT_CONNECTED 0x12U      /**< Station not connected */
#define SYN_CCP_ERR_RESOURCE_LOCKED 0x18U    /**< Resource locked */
/**@}*/

/** @name CCP Protection Resource Masks */
/**@{*/
#define SYN_CCP_RESOURCE_CAL 0x01U /**< Calibration resource mask */
#define SYN_CCP_RESOURCE_DAQ 0x02U /**< Data acquisition resource mask */
#define SYN_CCP_RESOURCE_PGM 0x40U /**< Flash programming resource mask */
/**@}*/

/**
 * @brief ODT Element mapping entry.
 */
typedef struct {
    uintptr_t address; /**< Memory target address */
    uint8_t size;      /**< Byte size (1, 2, 4 bytes) */
    bool host_valid;   /**< True if address was set via trusted host pointer */
} SYN_CCP_ODTEntry;

/**
 * @brief Registered memory region descriptor for bounds-checked MTA/DAQ access.
 */
typedef struct {
    uint32_t base_addr; /**< 32-bit CCP protocol base address */
    uint32_t size;      /**< Region byte length */
    uint8_t *host_ptr;  /**< Backing host memory pointer */
    bool writable;      /**< True if downloads/programming are permitted */
} SYN_CCP_MemRegion;

/**
 * @brief ODT (Object Descriptor Table) structure.
 */
typedef struct {
    SYN_CCP_ODTEntry entries[SYN_CCP_MAX_ENTRIES_PER_ODT]; /**< ODT entry array */
    uint8_t entry_count;                                   /**< Active entry count */
} SYN_CCP_ODT;

/**
 * @brief DAQ List structure.
 */
typedef struct {
    SYN_CCP_ODT odts[SYN_CCP_MAX_ODT_PER_DAQ]; /**< Array of ODT blocks */
    uint8_t odt_count;                         /**< Configured ODT count */
    uint8_t current_odt_idx;                   /**< Current ODT index */
    uint8_t current_entry_idx;                 /**< Current entry index */
    bool running;                              /**< DAQ list running state */
    bool pending_tx;                           /**< Pending ODT transmission for active event */
    uint8_t event_channel;                     /**< Event channel assignment */
    uint16_t prescaler;                        /**< Transmission prescaler */
    uint16_t cycle_counter;                    /**< Cycle counter */
} SYN_CCP_DAQList;

/**
 * @brief CCP Security Seed Generator callback.
 */
typedef uint32_t (*SYN_CCP_SeedFn)(uint8_t resource, void *ctx);

/**
 * @brief CCP Security Unlock Key verification callback.
 */
typedef bool (*SYN_CCP_UnlockFn)(uint8_t resource, uint32_t key, void *ctx);

/**
 * @brief CCP Memory Erase callback.
 */
typedef bool (*SYN_CCP_EraseFn)(uint32_t addr, uint32_t size, void *ctx);

/**
 * @brief CCP Slave Instance Handle.
 */
typedef struct {
    uint16_t station_addr;           /**< CCP station address */
    bool connected;                  /**< Session connection state */
    uint8_t unlocked_resources;      /**< Bitmask of unlocked resources */
    uintptr_t mta0_addr;             /**< MTA0 target address */
    uint8_t mta0_ext;                /**< MTA0 extension address */
    bool mta0_host_valid;            /**< True if MTA0 holds host pointer */
    uintptr_t mta1_addr;             /**< MTA1 target address */
    uint8_t mta1_ext;                /**< MTA1 extension address */
    bool mta1_host_valid;            /**< True if MTA1 holds host pointer */
    uint8_t active_cal_page;         /**< Currently active calibration page */
    uint8_t rr_daq_idx;              /**< Round-robin DAQ list cursor */
    bool daq_tick_drained;           /**< True once current event tick ODTs drained */
    uint8_t daq_tick_ch;             /**< Event channel of completed tick drain */
    uint8_t pending_unlock_resource; /**< Resource requested in last GET_SEED */
    SYN_CCP_MemRegion mem_regions[SYN_CCP_MAX_MEM_REGIONS]; /**< Registered memory regions */
    uint8_t region_count;                                   /**< Active memory region count */
    SYN_CCP_DAQList daq_lists[SYN_CCP_MAX_DAQ_LISTS];       /**< Configured DAQ lists */
    uint8_t selected_daq;       /**< Currently selected DAQ list index */
    uint8_t selected_odt;       /**< Currently selected ODT index */
    SYN_CCP_SeedFn seed_cb;     /**< Security seed generator callback */
    void *seed_ctx;             /**< Context for seed callback */
    SYN_CCP_UnlockFn unlock_cb; /**< Security unlock key verification callback */
    void *unlock_ctx;           /**< Context for unlock callback */
    SYN_CCP_EraseFn erase_cb;   /**< Flash/RAM memory erase callback */
    void *erase_ctx;            /**< Context for erase callback */
} SYN_CCP_Slave;

/**
 * @brief Initialize CCP slave instance.
 * @param slave Pointer to CCP slave handle.
 * @param station_addr 16-bit CCP station address.
 */
void syn_ccp_init(SYN_CCP_Slave *slave, uint16_t station_addr);

/**
 * @brief Process incoming Command Receive Object (CRO) frame.
 * @param slave Pointer to CCP slave handle.
 * @param cro 8-byte CRO input CAN payload.
 * @param dto_out 8-byte buffer to receive Command Return Message (DTO/CRM).
 * @return True if DTO response frame is ready to transmit.
 */
bool syn_ccp_process_cro(SYN_CCP_Slave *slave, const uint8_t cro[8], uint8_t dto_out[8]);

/**
 * @brief Set Memory Transfer Address (MTA0 or MTA1).
 * @param slave Pointer to CCP slave handle.
 * @param mta_num 0 for MTA0, 1 for MTA1.
 * @param ext Address extension.
 * @param addr Target memory address.
 */
void syn_ccp_set_mta(SYN_CCP_Slave *slave, uint8_t mta_num, uint8_t ext, uintptr_t addr);

/**
 * @brief Register a bounds-checked memory region for CCP MTA and DAQ accesses.
 * @param slave Pointer to CCP slave handle.
 * @param base_addr 32-bit CCP protocol base address.
 * @param host_ptr Backing host memory buffer.
 * @param size Size of region in bytes.
 * @param writable True if calibration/programming writes are allowed.
 * @return True on success, false on invalid arguments or full table.
 */
bool syn_ccp_register_mem_region(SYN_CCP_Slave *slave, uint32_t base_addr, void *host_ptr,
                                 uint32_t size, bool writable);

/**
 * @brief Advance DAQ prescalers once for an event channel tick prior to ODT drain.
 * @param slave Pointer to CCP slave handle.
 * @param event_channel Trigger event channel index.
 */
void syn_ccp_daq_event(SYN_CCP_Slave *slave, uint8_t event_channel);

/**
 * @brief Service periodic DAQ sampling tick.
 * @param slave Pointer to CCP slave handle.
 * @param event_channel Trigger event channel index.
 * @param dto_out Output buffer for DAQ DTO packet.
 * @param list_idx Output index of transmitting DAQ list.
 * @param odt_idx Output index of transmitting ODT.
 * @return True if a DAQ packet is ready to send.
 */
bool syn_ccp_service_daq(SYN_CCP_Slave *slave, uint8_t event_channel, uint8_t dto_out[8],
                         uint8_t *list_idx, uint8_t *odt_idx);

#ifdef __cplusplus
}
#endif

#endif /* SYN_CCP_H */
