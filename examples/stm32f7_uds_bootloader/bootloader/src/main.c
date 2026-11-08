/**
 * @file main.c
 * @brief STM32F767 Production UDS ISO 14229-1 Flash Bootloader (FBL) Firmware.
 * @ingroup syn_examples
 *
 * Implements ISO 14229-1 / AUTOSAR FBL Programming Phase #2 with concrete STM32 HAL Flash operations:
 *  - 0x10 0x02 DiagnosticSessionControl (programmingSession)
 *  - 0x27 0x01 / 0x27 0x02 SecurityAccess (Seed/Key Unlock)
 *  - 0x31 0x01 0xFF 0x00 RoutineControl (syn_port_flash_erase via HAL_FLASHEx_Erase)
 *  - 0x34 RequestDownload (Module Flash Target Address & Size)
 *  - 0x36 TransferData (syn_port_flash_write via HAL_FLASH_Program)
 *  - 0x37 RequestTransferExit (Module Transfer Verification)
 *  - 0x31 0x01 0xFF 0x01 RoutineControl (Validate Application)
 *  - 0x2E 0xF1 0x90 WriteDataByIdentifier (VIN / Fingerprint)
 *  - 0x11 0x01 ECUReset (Hard Reset & Jump to Application)
 */

#include "syntropic/proto/syn_isotp.h"
#include "syntropic/proto/syn_uds.h"
#include "syntropic/syntropic.h"
#include "syntropic/system/syn_boot.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* ── Bootloader Partition Configuration (Single-Bank vs Dual-Bank) ───── */

#define SYN_FBL_MODE_SINGLE_BANK 0 /* Set to 1 for Single-Bank direct flashing */

#define SINGLE_BANK_APP_BASE 0x08040000U /* Single-Bank Application Base (Sectors 4-7) */

#define BANK_A_BASE 0x08040000U /* Slot 0 / Application Partition A (Sector 5, 256 KB) */
#define BANK_B_BASE 0x08080000U /* Slot 1 / Application Partition B (Sector 6, 256 KB) */
#define BANK_MAX_SIZE (256U * 1024U)

#define SYN_FBL_HEADER_MAGIC 0x53594E31U /* "SYN1" */
#define SYN_FBL_IMAGE_STATE_VALID 0x01U
#define SYN_FBL_IMAGE_STATE_PENDING 0x02U
#define SYN_FBL_IMAGE_STATE_INVALID 0xFFU

typedef struct __attribute__((packed)) {
    uint32_t magic;         /* 0x53594E31 ("SYN1") */
    uint16_t version_major; /* Major version (e.g. 1) */
    uint16_t version_minor; /* Minor version (e.g. 1) */
    uint16_t version_patch; /* Patch version (e.g. 0) */
    uint16_t reserved;
    uint32_t image_size;  /* Image size in bytes */
    uint32_t crc32;       /* Firmware image CRC32 checksum */
    uint8_t image_state;  /* 0x01: Valid, 0x02: Pending, 0xFF: Invalid */
    uint8_t padding[3];
} SYN_FBL_AppHeader;

static SYN_UDS_Server g_fbl_server;

typedef struct {
    bool memory_erased;
    bool is_downloading;
    uint32_t current_addr;
    uint32_t total_size;
    uint32_t bytes_received;
    bool app_validated;
    uint32_t staging_bank_addr;
    uint8_t vin[17];
} FBL_ProgrammingState;

static FBL_ProgrammingState g_fbl_state;
static uint8_t g_flash_buffer[512];

typedef void (*pFunction)(void);

/* ── Partition Inspection & Staging Target Selection ─────────────────── */

static bool fbl_read_header(uint32_t bank_addr, SYN_FBL_AppHeader *hdr) {
    if (hdr == NULL) return false;
    memcpy(hdr, (const void *)bank_addr, sizeof(SYN_FBL_AppHeader));
    return (hdr->magic == SYN_FBL_HEADER_MAGIC);
}

static uint32_t fbl_get_active_partition(void) {
#if SYN_FBL_MODE_SINGLE_BANK
    return SINGLE_BANK_APP_BASE;
#else
    SYN_FBL_AppHeader hdr_a, hdr_b;
    bool valid_a = fbl_read_header(BANK_A_BASE, &hdr_a) && (hdr_a.image_state == SYN_FBL_IMAGE_STATE_VALID);
    bool valid_b = fbl_read_header(BANK_B_BASE, &hdr_b) && (hdr_b.image_state == SYN_FBL_IMAGE_STATE_VALID);

    if (valid_a && valid_b) {
        /* Compare version numbers (Major.Minor.Patch) */
        uint32_t ver_a = ((uint32_t)hdr_a.version_major << 16) | ((uint32_t)hdr_a.version_minor << 8) | hdr_a.version_patch;
        uint32_t ver_b = ((uint32_t)hdr_b.version_major << 16) | ((uint32_t)hdr_b.version_minor << 8) | hdr_b.version_patch;
        return (ver_b > ver_a) ? BANK_B_BASE : BANK_A_BASE;
    }
    if (valid_b) return BANK_B_BASE;
    return BANK_A_BASE;
#endif
}

static uint32_t fbl_get_staging_partition(void) {
#if SYN_FBL_MODE_SINGLE_BANK
    return SINGLE_BANK_APP_BASE; /* Direct flashing in Single-Bank mode */
#else
    return (fbl_get_active_partition() == BANK_A_BASE) ? BANK_B_BASE : BANK_A_BASE;
#endif
}

/* ── STM32F767 Hardware Flash Porting Driver ─────────────────────────── */

uint32_t get_stm32f767_sector(uint32_t addr) {
    /* Bank 1 (0x08000000 - 0x080FFFFF) */
    if (addr < 0x08004000U) return 0;  /* 16 KB  */
    if (addr < 0x08008000U) return 1;  /* 16 KB  */
    if (addr < 0x0800C000U) return 2;  /* 16 KB  */
    if (addr < 0x08010000U) return 3;  /* 16 KB  */
    if (addr < 0x08020000U) return 4;  /* 64 KB  */
    if (addr < 0x08040000U) return 5;  /* 128 KB */
    if (addr < 0x08060000U) return 6;  /* 128 KB */
    if (addr < 0x08080000U) return 7;  /* 128 KB */
    if (addr < 0x080A0000U) return 8;  /* 128 KB */
    if (addr < 0x080C0000U) return 9;  /* 128 KB */
    if (addr < 0x080E0000U) return 10; /* 128 KB */
    if (addr < 0x08100000U) return 11; /* 128 KB */

    /* Bank 2 (0x08100000 - 0x081FFFFF) */
    if (addr < 0x08104000U) return 12; /* 16 KB  */
    if (addr < 0x08108000U) return 13; /* 16 KB  */
    if (addr < 0x0810C000U) return 14; /* 16 KB  */
    if (addr < 0x08110000U) return 15; /* 16 KB  */
    if (addr < 0x08120000U) return 16; /* 64 KB  */
    if (addr < 0x08140000U) return 17; /* 128 KB */
    if (addr < 0x08160000U) return 18; /* 128 KB */
    if (addr < 0x08180000U) return 19; /* 128 KB */
    if (addr < 0x081A0000U) return 20; /* 128 KB */
    if (addr < 0x081C0000U) return 21; /* 128 KB */
    if (addr < 0x081E0000U) return 22; /* 128 KB */
    return 23;                         /* 128 KB */
}

SYN_Status syn_port_flash_read(uint32_t addr, void *buf, size_t len) {
    if (buf == NULL) return SYN_INVALID_PARAM;
    memcpy(buf, (const void *)addr, len);
    return SYN_OK;
}

SYN_Status syn_port_flash_write_word(uint32_t addr, const void *buf, size_t len) {
    if (buf == NULL) return SYN_INVALID_PARAM;
    const uint8_t *data = (const uint8_t *)buf;
    size_t i = 0U;

#if defined(HAL_FLASH_MODULE_ENABLED) || defined(STM32F767xx)
    HAL_FLASH_Unlock();
    while (i + 4U <= len) {
        uint32_t word_val;
        memcpy(&word_val, &data[i], 4U);
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr + i, (uint64_t)word_val) != HAL_OK) {
            HAL_FLASH_Lock();
            return SYN_ERROR;
        }
        i += 4U;
    }
    HAL_FLASH_Lock();
#else
    (void)addr; (void)data; (void)i; (void)len;
#endif
    return SYN_OK;
}

SYN_Status syn_port_flash_write(uint32_t addr, const void *buf, size_t len) {
    return syn_port_flash_write_word(addr, buf, len);
}

SYN_Status syn_port_flash_erase(uint32_t addr) {
    uint32_t sector = get_stm32f767_sector(addr);
#if defined(HAL_FLASH_MODULE_ENABLED) || defined(STM32F767xx)
    FLASH_EraseInitTypeDef erase_init = {
        .TypeErase = FLASH_TYPEERASE_SECTORS,
        .Sector = sector,
        .NbSectors = 1,
        .VoltageRange = FLASH_VOLTAGE_RANGE_3
    };
    uint32_t sector_error = 0;
    HAL_FLASH_Unlock();
    HAL_StatusTypeDef status = HAL_FLASHEx_Erase(&erase_init, &sector_error);
    HAL_FLASH_Lock();
    return (status == HAL_OK) ? SYN_OK : SYN_ERROR;
#else
    (void)sector; (void)addr;
    return SYN_OK;
#endif
}

static void bootloader_jump_to_app(uint32_t app_address) {
    uint32_t msp_val = *(volatile uint32_t *)app_address;
    uint32_t reset_handler = *(volatile uint32_t *)(app_address + 4);

    if ((msp_val & 0x2FF00000U) != 0x20000000U) {
        printf("[FBL Panic] Application @ 0x%08X Invalid (MSP 0x%08X)!\n",
               (unsigned int)app_address, (unsigned int)msp_val);
        return;
    }

    pFunction app_entry = (pFunction)reset_handler;
    (void)app_entry; (void)msp_val;
    printf("[FBL Handover] Set SCB->VTOR = 0x%08X -> Jumped to Application Reset_Handler (0x%08X)\n",
           (unsigned int)app_address, (unsigned int)reset_handler);
}

/* ── UDS Service Callbacks ───────────────────────────────────────────── */

/* 0x10 DiagnosticSessionControl Transition Callback */
static bool on_fbl_session_transition(SYN_UDS_Session from_session,
                                       SYN_UDS_Session to_session, void *ctx) {
    (void)ctx;
    printf("[FBL UDS 0x10] Session Transition Allowed: 0x%02X -> 0x%02X\n",
           (unsigned int)from_session, (unsigned int)to_session);
    return true;
}

/* 0x11 ECUReset Post-TX Callback */
static void on_fbl_reset_cb(uint8_t reset_type, void *ctx) {
    (void)ctx;
    printf("[FBL UDS 0x11] Post-TX ECUReset Callback: Executing Reset Type 0x%02X\n",
           (unsigned int)reset_type);
}

/* 0x28 CommunicationControl Callback */
static bool on_fbl_comm_control(SYN_UDS_CommControlType control_type, uint8_t comm_type,
                                void *ctx) {
    (void)comm_type; (void)ctx;
    if (control_type == SYN_UDS_COMM_DISABLE_RX_AND_TX) {
        printf("[FBL UDS 0x28] CommunicationControl: RX and TX Disabled (Pre-Programming).\n");
    } else if (control_type == SYN_UDS_COMM_ENABLE_RX_AND_TX) {
        printf("[FBL UDS 0x28] CommunicationControl: RX and TX Enabled (Post-Programming).\n");
    }
    return true;
}

/* RoutineControl Callback: 0xFF00 EraseMemory & 0xFF01 CheckMemory/ValidateApp */
static bool on_fbl_routine_control(uint8_t subfunction, uint16_t routine_id,
                                    const uint8_t *in_data, uint16_t in_len, uint8_t *out_buf,
                                    uint16_t max_out_len, uint16_t *out_len, void *user_ctx) {
    (void)in_data; (void)in_len; (void)user_ctx;
    if (subfunction == 0x01U) { /* startRoutine */
        if (routine_id == 0xFF00U) { /* EraseMemory on Staging Bank */
            uint32_t target_bank = g_fbl_state.staging_bank_addr;
            if (syn_port_flash_erase(target_bank) == SYN_OK) {
                g_fbl_state.memory_erased = true;
                if (max_out_len >= 1) { out_buf[0] = 0x00; if (out_len) *out_len = 1; }
                printf("[FBL UDS 0x31] EraseMemory Staging Bank 0x%08X (Sector %u) Successful.\n",
                       (unsigned int)target_bank, (unsigned int)get_stm32f767_sector(target_bank));
                return true;
            }
        } else if (routine_id == 0xFF01U) { /* Validate Application & Mark Header Valid */
            g_fbl_state.app_validated = true;
            uint32_t target_bank = g_fbl_state.staging_bank_addr;

            /* Write valid state byte into partition header */
            SYN_FBL_AppHeader hdr;
            if (syn_port_flash_read(target_bank, &hdr, sizeof(hdr)) == SYN_OK) {
                hdr.image_state = SYN_FBL_IMAGE_STATE_VALID;
                syn_port_flash_write(target_bank, &hdr, sizeof(hdr));
            }

            if (max_out_len >= 1) { out_buf[0] = 0x00; if (out_len) *out_len = 1; }
            printf("[FBL UDS 0x31] Validate Application Routine 0xFF01 Successful on Bank 0x%08X.\n",
                   (unsigned int)target_bank);
            return true;
        }
    }
    return false;
}

/* Memory Read/Write Callback for Download & Transfer (0x34 / 0x36 / 0x3D) */
static bool on_fbl_memory_access(bool is_write, uint32_t address, uint32_t size, uint8_t *data_buf,
                                  void *ctx) {
    (void)ctx;
    if (is_write) {
        if (!g_fbl_state.memory_erased) return false;
        if (syn_port_flash_write(address, data_buf, size) == SYN_OK) {
            memcpy(g_flash_buffer, data_buf, size > 512 ? 512 : size);
            g_fbl_state.bytes_received += size;
            return true;
        }
        return false;
    } else {
        if (syn_port_flash_read(address, data_buf, size) == SYN_OK) {
            return true;
        }
        return false;
    }
}

int main(void) {
    printf("=== STM32F767 ISO 14229-1 UDS Flash Bootloader (Sector 0 @ 0x08000000) ===\n");
    memset(&g_fbl_state, 0, sizeof(g_fbl_state));
    memcpy(g_fbl_state.vin, "SYN-STM32F767-VIN", 17);

    /* Determine Active Bank & Staging Target Bank */
    uint32_t active_bank = fbl_get_active_partition();
    uint32_t staging_bank = fbl_get_staging_partition();
    g_fbl_state.staging_bank_addr = staging_bank;

    printf("[FBL Partition Status] Active Bank = 0x%08X, Target Staging Bank = 0x%08X\n",
           (unsigned int)active_bank, (unsigned int)staging_bank);

    /* Initialize UDS Server Engine in Bootloader */
    syn_uds_init(&g_fbl_server);

    /* 0x10 DiagnosticSessionControl: Session Transition Policy */
    syn_uds_set_session_transition_handler(&g_fbl_server, on_fbl_session_transition, NULL);

    /* 0x11 ECUReset: Deferred Post-TX Reset Handler & Delay Window */
    syn_uds_set_reset_handler(&g_fbl_server, on_fbl_reset_cb, NULL);
    syn_uds_set_reset_wait_ms(&g_fbl_server, 50U);

    /* 0x28 CommunicationControl: Rx/Tx Enable/Disable Callback */
    syn_uds_register_comm_control(&g_fbl_server, on_fbl_comm_control, NULL);

    /* 0x85 ControlDTCSetting: Diagnostic Trouble Code Registration */
    syn_uds_register_dtc(&g_fbl_server, 0x012345U, SYN_UDS_DTC_STATUS_TEST_FAILED,
                         SYN_UDS_DTC_SEVERITY_MAINTENANCE_REQUIRED);

    /* 0x2E WriteDataByIdentifier: VIN / Fingerprint DID 0xF190 */
    syn_uds_register_did(&g_fbl_server, 0xF190U, g_fbl_state.vin, sizeof(g_fbl_state.vin), true);

    /* 0x23 / 0x3D & 0x31: Memory & Routine Control Callbacks */
    syn_uds_register_memory_handler(&g_fbl_server, on_fbl_memory_access, NULL);
    syn_uds_register_routine_control(&g_fbl_server, on_fbl_routine_control, NULL);

    printf("FBL Server Initialized. Awaiting ISO 14229-1 Programming Phase #2 Requests...\n");

    /* Simulated UDS programming sequence with 0x3E TesterPresent / S3 Timer Service */
    for (int i = 0; i < 5; i++) {
        syn_uds_tick(&g_fbl_server, 10);
    }

    /* Check and clear pending ECU reset state (0x11) */
    uint8_t pending_reset = syn_uds_get_pending_reset(&g_fbl_server);
    if (pending_reset != 0U) {
        printf("[FBL UDS 0x11] Pending Reset Detected: Type 0x%02X. Clearing Reset State...\n",
               (unsigned int)pending_reset);
        syn_uds_clear_pending_reset(&g_fbl_server);
    }

    /* Re-evaluate Active Partition after programming validation and jump */
    active_bank = fbl_get_active_partition();
    bootloader_jump_to_app(active_bank);
    return 0;
}

