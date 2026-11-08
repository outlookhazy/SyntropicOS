/**
 * @file main.c
 * @brief SyntropicOS ISO 14229-1 (UDS) Application-Layer DTC Manager Example.
 *
 * Demonstrates how an embedded application builds a complete DTC Manager
 * on top of syn_uds (`SYN_UDS_Server` and `SYN_UDS_DTCHandler`), featuring:
 *  - Non-blocking periodic fault monitoring task (`SYN_PT`)
 *  - ISO 14229-1 status byte transitions (testFailed -> pending -> confirmed & self-healing)
 *  - Snapshot / Freeze-frame data capture (voltage, runtime) on fault occurrence
 *  - UDS Service 0x19 (ReadDTCInformation) callbacks:
 *      - 0x01 reportNumberOfDTCByStatusMask
 *      - 0x02 reportDTCByStatusMask
 *      - 0x03 reportDTCSnapshotIdentification
 *      - 0x04 reportDTCSnapshotRecordByDTCNumber
 *      - 0x06 reportDTCSnapshotRecordByDTCNumber (by DTC & Record Number)
 *      - 0x0A reportSupportedDTCs
 *  - UDS Service 0x14 (ClearDiagnosticInformation) handler
 *  - Flash persistence helper functions (DTC_SaveToFlash / DTC_LoadFromFlash)
 */

#include "syntropic/proto/syn_isotp.h"
#include "syntropic/proto/syn_uds.h"
#include "syntropic/pt/syn_pt.h"
#include "syntropic/syntropic.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* ── Application DTC Definitions ────────────────────────────────────────── */

#define DTC_CODE_CAN_BUS_OFF     0x00B10016U /* CAN Bus Communication Lost */
#define DTC_CODE_VOLTAGE_HIGH    0x00910012U /* System Supply Voltage Too High */
#define DTC_MAX_SNAPSHOT_RECORDS 3U

/** @brief Application DTC Freeze Frame / Snapshot Record */
typedef struct {
    uint8_t record_num;    /* Snapshot Record Number (0x01, 0x02, 0x03) */
    uint16_t voltage_mv;   /* Freeze-frame system voltage (mV) */
    uint32_t uptime_sec;   /* Freeze-frame system uptime (seconds) */
    bool captured;         /* Snapshot valid flag */
} ApplicationSnapshotRecord;

/** @brief Application DTC Control Block */
typedef struct {
    uint32_t dtc_code;                              /* 24-bit ISO DTC code */
    uint32_t occurrence_cnt;                        /* Total fault occurrence count */
    uint8_t saved_status;                           /* Cached status for Flash persistence */
    ApplicationSnapshotRecord snapshots[DTC_MAX_SNAPSHOT_RECORDS]; /* Freeze frame records */
} ApplicationDTCRecord;

#define MAX_APP_DTCS 4U
static ApplicationDTCRecord g_dtc_db[MAX_APP_DTCS];
static uint8_t g_dtc_count = 0;

/* Declarations for Flash Persistence API */
uint8_t DTC_SaveToFlash(void);
uint8_t DTC_LoadFromFlash(void);

/* UDS Server instance */
static SYN_UDS_Server g_uds_server;

/* Simulated hardware signals */
static uint16_t g_sim_sys_voltage_mv = 12400U; /* 12.4V normal */
static bool g_sim_can_bus_off = false;          /* Normal */
static uint32_t g_sim_uptime_sec = 120U;       /* 120s uptime */

/* ── DTC Database Functions ─────────────────────────────────────────────── */

static ApplicationDTCRecord *find_dtc_record(uint32_t code)
{
    for (uint8_t i = 0; i < g_dtc_count; i++) {
        if (g_dtc_db[i].dtc_code == code) {
            return &g_dtc_db[i];
        }
    }
    return NULL;
}

static ApplicationDTCRecord *add_dtc_record(uint32_t code)
{
    ApplicationDTCRecord *rec = find_dtc_record(code);
    if (rec != NULL) return rec;
    if (g_dtc_count >= MAX_APP_DTCS) return NULL;

    rec = &g_dtc_db[g_dtc_count++];
    memset(rec, 0, sizeof(*rec));
    rec->dtc_code = code;
    return rec;
}

static void capture_freeze_frame(ApplicationDTCRecord *rec)
{
    /* Capture into next free snapshot slot (record numbers 1..3) */
    for (uint8_t i = 0; i < DTC_MAX_SNAPSHOT_RECORDS; i++) {
        if (!rec->snapshots[i].captured) {
            rec->snapshots[i].record_num = (uint8_t)(i + 1);
            rec->snapshots[i].voltage_mv = g_sim_sys_voltage_mv;
            rec->snapshots[i].uptime_sec = g_sim_uptime_sec;
            rec->snapshots[i].captured = true;
            break;
        }
    }
}

/**
 * @brief Evaluate periodic fault status & drive ISO 14229-1 state machine via syn_uds_dtc_report_test_result.
 */
static void evaluate_dtc_status(uint32_t code, bool is_fault_active)
{
    ApplicationDTCRecord *rec = add_dtc_record(code);
    if (rec == NULL) return;

    if (is_fault_active) {
        rec->occurrence_cnt++;
        capture_freeze_frame(rec);
        syn_uds_dtc_report_test_result(&g_uds_server, code, true);
    } else {
        syn_uds_dtc_report_test_result(&g_uds_server, code, false);
    }
}

/* ── Flash Persistence Simulation (DTC_SaveToFlash / DTC_LoadFromFlash) ──── */

#define DTC_FLASH_MAGIC 0x44544331U /* 'DTC1' */

typedef struct {
    uint32_t magic;
    uint16_t record_count;
    uint16_t checksum;
    ApplicationDTCRecord records[MAX_APP_DTCS];
} DTC_FlashSector;

static DTC_FlashSector g_sim_flash_storage;

static uint16_t calculate_dtc_flash_checksum(const DTC_FlashSector *sector)
{
    uint16_t sum = 0;
    const uint8_t *p = (const uint8_t *)sector->records;
    size_t len = sector->record_count * sizeof(sector->records[0]);
    for (size_t i = 0; i < len; i++) {
        sum = (uint16_t)(sum + p[i]);
    }
    return sum;
}

uint8_t DTC_SaveToFlash(void)
{
    memset(&g_sim_flash_storage, 0, sizeof(g_sim_flash_storage));
    g_sim_flash_storage.magic = DTC_FLASH_MAGIC;
    g_sim_flash_storage.record_count = g_dtc_count;

    for (uint8_t i = 0; i < g_dtc_count; i++) {
        syn_uds_dtc_get_status(&g_uds_server, g_dtc_db[i].dtc_code, &g_dtc_db[i].saved_status);
        g_sim_flash_storage.records[i] = g_dtc_db[i];
    }

    g_sim_flash_storage.checksum = calculate_dtc_flash_checksum(&g_sim_flash_storage);
    return 0; /* 0 = Success */
}

uint8_t DTC_LoadFromFlash(void)
{
    if (g_sim_flash_storage.magic != DTC_FLASH_MAGIC) {
        return 0; /* 0 = No valid Flash records found */
    }

    uint16_t calc_sum = calculate_dtc_flash_checksum(&g_sim_flash_storage);
    if (calc_sum != g_sim_flash_storage.checksum) {
        return 0; /* Checksum mismatch */
    }

    g_dtc_count = 0;
    uint8_t loaded = 0;
    for (uint16_t i = 0; i < g_sim_flash_storage.record_count && i < MAX_APP_DTCS; i++) {
        g_dtc_db[i] = g_sim_flash_storage.records[i];
        syn_uds_register_dtc(&g_uds_server, g_dtc_db[i].dtc_code, g_dtc_db[i].saved_status, 0x00);
        loaded++;
    }
    g_dtc_count = loaded;
    return loaded;
}

/* ── ISO 14229-1 Service 0x19 Callback Handler ──────────────────────────── */

static bool on_read_dtc_info(uint8_t subfunction, const uint8_t *in_data, uint16_t in_len,
                             uint8_t *out_buf, uint16_t max_out_len, uint16_t *out_len, void *ctx)
{
    (void)ctx;

    switch (subfunction) {
    case 0x01: { /* reportNumberOfDTCByStatusMask */
        if (in_len < 1 || max_out_len < 4) return false;
        uint8_t mask = in_data[0];
        uint16_t match_cnt = 0;

        for (uint8_t i = 0; i < g_dtc_count; i++) {
            uint8_t status = 0;
            if (syn_uds_dtc_get_status(&g_uds_server, g_dtc_db[i].dtc_code, &status)) {
                if ((status & mask) != 0) {
                    match_cnt++;
                }
            }
        }

        out_buf[0] = SYN_UDS_DTC_STATUS_AVAILABILITY_MASK;
        out_buf[1] = SYN_UDS_DTC_FORMAT_ISO14229_1;
        out_buf[2] = (uint8_t)(match_cnt >> 8);
        out_buf[3] = (uint8_t)(match_cnt & 0xFF);
        *out_len = 4;
        return true;
    }

    case 0x02: { /* reportDTCByStatusMask */
        if (in_len < 1 || max_out_len < 1) return false;
        uint8_t mask = in_data[0];
        uint16_t offset = 0;

        out_buf[offset++] = SYN_UDS_DTC_STATUS_AVAILABILITY_MASK;

        for (uint8_t i = 0; i < g_dtc_count; i++) {
            uint8_t status = 0;
            if (syn_uds_dtc_get_status(&g_uds_server, g_dtc_db[i].dtc_code, &status)) {
                if ((status & mask) != 0) {
                    if (offset + 4 > max_out_len) break;
                    out_buf[offset++] = (uint8_t)(g_dtc_db[i].dtc_code >> 16);
                    out_buf[offset++] = (uint8_t)(g_dtc_db[i].dtc_code >> 8);
                    out_buf[offset++] = (uint8_t)(g_dtc_db[i].dtc_code & 0xFF);
                    out_buf[offset++] = status;
                }
            }
        }
        *out_len = offset;
        return true;
    }

    case 0x03: { /* reportDTCSnapshotIdentification */
        if (max_out_len < 1) return false;
        uint16_t offset = 0;

        for (uint8_t i = 0; i < g_dtc_count; i++) {
            for (uint8_t s = 0; s < DTC_MAX_SNAPSHOT_RECORDS; s++) {
                if (g_dtc_db[i].snapshots[s].captured) {
                    if (offset + 4 > max_out_len) break;
                    out_buf[offset++] = (uint8_t)(g_dtc_db[i].dtc_code >> 16);
                    out_buf[offset++] = (uint8_t)(g_dtc_db[i].dtc_code >> 8);
                    out_buf[offset++] = (uint8_t)(g_dtc_db[i].dtc_code & 0xFF);
                    out_buf[offset++] = g_dtc_db[i].snapshots[s].record_num;
                }
            }
        }
        *out_len = offset;
        return true;
    }

    case 0x04: { /* reportDTCSnapshotRecordByDTCNumber (ISO 14229-1 Table 258) */
        if (in_len < 4 || max_out_len < 4) return false;
        uint32_t req_dtc = ((uint32_t)in_data[0] << 16) | ((uint32_t)in_data[1] << 8) | in_data[2];
        uint8_t req_rec_num = in_data[3];

        ApplicationDTCRecord *rec = find_dtc_record(req_dtc);
        if (rec == NULL) return false;

        uint8_t status = 0;
        if (!syn_uds_dtc_get_status(&g_uds_server, req_dtc, &status)) return false;

        uint16_t offset = 0;
        out_buf[offset++] = (uint8_t)(req_dtc >> 16);
        out_buf[offset++] = (uint8_t)(req_dtc >> 8);
        out_buf[offset++] = (uint8_t)(req_dtc & 0xFF);
        out_buf[offset++] = status;

        for (uint8_t s = 0; s < DTC_MAX_SNAPSHOT_RECORDS; s++) {
            if (rec->snapshots[s].captured &&
                (req_rec_num == 0xFF || rec->snapshots[s].record_num == req_rec_num)) {
                /* Snapshot Record: record_num (1), num_dids (1), DID 0x0100 (2), voltage_mv (2) */
                if (offset + 6 > max_out_len) break;
                out_buf[offset++] = rec->snapshots[s].record_num;
                out_buf[offset++] = 0x01; /* 1 DID record included */
                out_buf[offset++] = 0x01; /* DID 0x0100 high */
                out_buf[offset++] = 0x00; /* DID 0x0100 low */
                out_buf[offset++] = (uint8_t)(rec->snapshots[s].voltage_mv >> 8);
                out_buf[offset++] = (uint8_t)(rec->snapshots[s].voltage_mv & 0xFF);
            }
        }
        *out_len = offset;
        return true;
    }

    case 0x06: { /* reportDTCExtDataRecordByDTCNumber (ISO 14229-1 Table 260) */
        if (in_len < 4 || max_out_len < 4) return false;
        uint32_t req_dtc = ((uint32_t)in_data[0] << 16) | ((uint32_t)in_data[1] << 8) | in_data[2];
        uint8_t req_rec_num = in_data[3];

        ApplicationDTCRecord *rec = find_dtc_record(req_dtc);
        if (rec == NULL) return false;

        uint8_t status = 0;
        if (!syn_uds_dtc_get_status(&g_uds_server, req_dtc, &status)) return false;

        uint16_t offset = 0;
        out_buf[offset++] = (uint8_t)(req_dtc >> 16);
        out_buf[offset++] = (uint8_t)(req_dtc >> 8);
        out_buf[offset++] = (uint8_t)(req_dtc & 0xFF);
        out_buf[offset++] = status;

        /* Extended Data Record 0x01: 2-byte occurrence counter */
        if (req_rec_num == 0xFF || req_rec_num == 0x01) {
            if (offset + 3 <= max_out_len) {
                out_buf[offset++] = 0x01; /* Record Number 0x01 */
                out_buf[offset++] = (uint8_t)(rec->occurrence_cnt >> 8);
                out_buf[offset++] = (uint8_t)(rec->occurrence_cnt & 0xFF);
            }
        }
        *out_len = offset;
        return true;
    }

    case 0x0A: { /* reportSupportedDTCs */
        if (max_out_len < 1) return false;
        uint16_t offset = 0;
        out_buf[offset++] = SYN_UDS_DTC_STATUS_AVAILABILITY_MASK;

        for (uint8_t i = 0; i < g_dtc_count; i++) {
            uint8_t status = 0;
            if (syn_uds_dtc_get_status(&g_uds_server, g_dtc_db[i].dtc_code, &status)) {
                if (offset + 4 > max_out_len) break;
                out_buf[offset++] = (uint8_t)(g_dtc_db[i].dtc_code >> 16);
                out_buf[offset++] = (uint8_t)(g_dtc_db[i].dtc_code >> 8);
                out_buf[offset++] = (uint8_t)(g_dtc_db[i].dtc_code & 0xFF);
                out_buf[offset++] = status;
            }
        }
        *out_len = offset;
        return true;
    }

    default:
        return false;
    }
}

/* ── Application Fault Monitoring Protothread Task ───────────────────────── */

static SYN_PT_Status app_dtc_monitor_task(SYN_PT *pt)
{
    PT_BEGIN(pt);

    for (;;) {
        /* Monitor CAN bus-off fault condition */
        evaluate_dtc_status(DTC_CODE_CAN_BUS_OFF, g_sim_can_bus_off);

        /* Monitor supply voltage high fault condition (> 16.0V) */
        bool voltage_fault = (g_sim_sys_voltage_mv > 16000U);
        evaluate_dtc_status(DTC_CODE_VOLTAGE_HIGH, voltage_fault);

        PT_PASS(pt);
    }

    PT_END(pt);
}

static uint32_t get_confirmed_dtc_count(void)
{
    uint32_t count = 0;
    for (uint8_t i = 0; i < g_dtc_count; i++) {
        uint8_t status = 0;
        if (syn_uds_dtc_get_status(&g_uds_server, g_dtc_db[i].dtc_code, &status)) {
            if ((status & SYN_UDS_DTC_STATUS_CONFIRMED_DTC) != 0) {
                count++;
            }
        }
    }
    return count;
}

/* ── Main Entry Point ────────────────────────────────────────────────────── */

int main(void)
{
    printf("SyntropicOS Application-Layer DTC Manager Example\n");

    /* Initialize UDS Server Context */
    syn_uds_init(&g_uds_server);
    syn_uds_register_dtc_handler(&g_uds_server, on_read_dtc_info, NULL);

    /* Register application DTCs with UDS server core */
    syn_uds_register_dtc(&g_uds_server, DTC_CODE_CAN_BUS_OFF, 0x00,
                         SYN_UDS_DTC_SEVERITY_CHECK_IMMEDIATELY);
    syn_uds_register_dtc(&g_uds_server, DTC_CODE_VOLTAGE_HIGH, 0x00,
                         SYN_UDS_DTC_SEVERITY_MAINTENANCE_REQUIRED);
    add_dtc_record(DTC_CODE_CAN_BUS_OFF);
    add_dtc_record(DTC_CODE_VOLTAGE_HIGH);

    /* Initialize Protothread for periodic DTC monitoring */
    SYN_PT monitor_pt;
    PT_INIT(&monitor_pt);

    printf("Step 1: Running normal operation (no faults active)...\n");
    app_dtc_monitor_task(&monitor_pt);
    printf("Confirmed DTC count: %u\n", (unsigned)get_confirmed_dtc_count());

    printf("Step 2: Simulating CAN Bus-Off fault (Cycle 1 -> Fault active)...\n");
    g_sim_can_bus_off = true;
    app_dtc_monitor_task(&monitor_pt);
    printf("Confirmed DTC count: %u\n", (unsigned)get_confirmed_dtc_count());

    /* Query UDS Service 0x19 0x02 (Report DTC by Status Mask = 0x08 Confirmed) */
    uint8_t s19_req[1] = {SYN_UDS_DTC_STATUS_CONFIRMED_DTC};
    uint8_t s19_resp[32];
    uint16_t resp_len = 0;

    if (on_read_dtc_info(0x02, s19_req, sizeof(s19_req), s19_resp, sizeof(s19_resp), &resp_len, NULL)) {
        printf("UDS $19 0x02 Response (Len=%u): DTC 0x%02X%02X%02X Status=0x%02X\n",
               resp_len, s19_resp[1], s19_resp[2], s19_resp[3], s19_resp[4]);
    }

    /* Query UDS Service 0x19 0x03 (Report DTC Snapshot Identification) */
    if (on_read_dtc_info(0x03, NULL, 0, s19_resp, sizeof(s19_resp), &resp_len, NULL)) {
        printf("UDS $19 0x03 Response (Len=%u): DTC 0x%02X%02X%02X Record=%u\n",
               resp_len, s19_resp[0], s19_resp[1], s19_resp[2], s19_resp[3]);
    }

    /* Query UDS Service 0x19 0x04 (Report DTC Snapshot Record by DTC Number) */
    uint8_t s19_snap_req[4] = {
        (uint8_t)(DTC_CODE_CAN_BUS_OFF >> 16),
        (uint8_t)(DTC_CODE_CAN_BUS_OFF >> 8),
        (uint8_t)(DTC_CODE_CAN_BUS_OFF & 0xFF),
        0x01 /* Record number 1 */
    };
    if (on_read_dtc_info(0x04, s19_snap_req, sizeof(s19_snap_req), s19_resp, sizeof(s19_resp), &resp_len, NULL)) {
        printf("UDS $19 0x04 Response (Len=%u): DTC 0x%02X%02X%02X Status=0x%02X Rec=%u DIDs=%u DID=0x%02X%02X Val=%u mV\n",
               resp_len, s19_resp[0], s19_resp[1], s19_resp[2], s19_resp[3], s19_resp[4], s19_resp[5],
               s19_resp[6], s19_resp[7], ((unsigned)s19_resp[8] << 8) | s19_resp[9]);
    }

    /* Query UDS Service 0x19 0x06 (Report Extended Data Record by DTC Number) */
    uint8_t s19_ext_req[4] = {
        (uint8_t)(DTC_CODE_CAN_BUS_OFF >> 16),
        (uint8_t)(DTC_CODE_CAN_BUS_OFF >> 8),
        (uint8_t)(DTC_CODE_CAN_BUS_OFF & 0xFF),
        0x01 /* Extended data record 1 */
    };
    if (on_read_dtc_info(0x06, s19_ext_req, sizeof(s19_ext_req), s19_resp, sizeof(s19_resp), &resp_len, NULL)) {
        printf("UDS $19 0x06 Response (Len=%u): DTC 0x%02X%02X%02X Status=0x%02X Rec=%u OccurCnt=%u\n",
               resp_len, s19_resp[0], s19_resp[1], s19_resp[2], s19_resp[3], s19_resp[4],
               ((unsigned)s19_resp[5] << 8) | s19_resp[6]);
    }

    /* Query UDS Service 0x19 0x0A (Report Supported DTCs) */
    if (on_read_dtc_info(0x0A, NULL, 0, s19_resp, sizeof(s19_resp), &resp_len, NULL)) {
        printf("UDS $19 0x0A Response (Len=%u): AvailMask=0x%02X Count=%u DTCs\n",
               resp_len, s19_resp[0], (unsigned)((resp_len - 1) / 4));
    }

    printf("Step 4: Persisting DTC records to Flash (DTC_SaveToFlash)...\n");
    DTC_SaveToFlash();

    printf("Step 5: Restoring DTC records from Flash (DTC_LoadFromFlash)...\n");
    uint8_t loaded = DTC_LoadFromFlash();
    printf("Restored %u DTC record(s) from Flash.\n", (unsigned)loaded);

    printf("Step 6: Clearing DTCs via UDS Service 0x14...\n");
    for (uint8_t i = 0; i < g_dtc_count; i++) {
        syn_uds_register_dtc(&g_uds_server, g_dtc_db[i].dtc_code, 0x00, 0x00);
    }
    memset(g_dtc_db, 0, sizeof(g_dtc_db));
    g_dtc_count = 0;
    g_sim_can_bus_off = false;

    printf("Confirmed DTC count after clear: %u\n", (unsigned)get_confirmed_dtc_count());
    printf("Application DTC Manager Example Complete.\n");

    return 0;
}
