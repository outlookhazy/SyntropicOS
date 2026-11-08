/**
 * @file syn_ccp.c
 * @brief ASAM CAN Calibration Protocol v2.1 (CCP) Slave Implementation.
 */

#include "syn_ccp.h"

#include "../util/syn_assert.h"
#include "../util/syn_pack.h"

#include <string.h>

void syn_ccp_init(SYN_CCP_Slave *slave, uint16_t station_addr)
{
    SYN_ASSERT(slave != NULL);

    memset(slave, 0, sizeof(*slave));
    slave->station_addr = station_addr;
    slave->connected = false;
    slave->unlocked_resources = SYN_CCP_RESOURCE_CAL | SYN_CCP_RESOURCE_DAQ | SYN_CCP_RESOURCE_PGM;
}

bool syn_ccp_register_mem_region(SYN_CCP_Slave *slave, uint32_t base_addr, void *host_ptr,
                                 uint32_t size, bool writable)
{
    if (slave == NULL || host_ptr == NULL || size == 0U ||
        slave->region_count >= SYN_CCP_MAX_MEM_REGIONS) {
        return false;
    }
    SYN_CCP_MemRegion *reg = &slave->mem_regions[slave->region_count];
    reg->base_addr = base_addr;
    reg->size = size;
    reg->host_ptr = (uint8_t *)host_ptr;
    reg->writable = writable;
    slave->region_count++;
    return true;
}

void syn_ccp_set_mta(SYN_CCP_Slave *slave, uint8_t mta_num, uint8_t ext, uintptr_t addr)
{
    if (slave == NULL) {
        return;
    }
    if (mta_num == 0U) {
        slave->mta0_ext = ext;
        slave->mta0_addr = addr;
        slave->mta0_host_valid = (addr != 0U);
    } else if (mta_num == 1U) {
        slave->mta1_ext = ext;
        slave->mta1_addr = addr;
        slave->mta1_host_valid = (addr != 0U);
    }
}

/**
 * @brief Check whether a memory access falls within a registered CCP region.
 * @param reg      Memory region descriptor.
 * @param addr     Requested protocol or host address.
 * @param size     Access byte length.
 * @param is_write true if write access is required.
 * @param out_ptr  Output host byte pointer when matched and permitted.
 * @return 1 if valid match, -1 if out-of-bounds or write-protected, 0 if no match.
 */
static int ccp_match_region(const SYN_CCP_MemRegion *reg, uintptr_t addr, uint32_t size,
                            bool is_write, uint8_t **out_ptr)
{
    uint64_t addr64 = (uint64_t)addr;
    uint64_t proto_base = (uint64_t)reg->base_addr;
    uint64_t host_base = (uint64_t)(uintptr_t)reg->host_ptr;
    uint64_t reg_size = (uint64_t)reg->size;
    uint32_t offset = 0U;
    if ((addr64 >= proto_base) && ((addr64 - proto_base) < reg_size)) {
        offset = (uint32_t)(addr64 - proto_base);
    } else if ((addr64 >= host_base) && ((addr64 - host_base) < reg_size)) {
        offset = (uint32_t)(addr64 - host_base);
    } else {
        return 0;
    }

    if ((uint64_t)size > (reg_size - (uint64_t)offset)) {
        return -1;
    }
    if (is_write && !reg->writable) {
        return -1;
    }
    *out_ptr = &reg->host_ptr[offset];
    return 1;
}

/**
 * @brief Resolve a protocol or host MTA address to a validated host memory pointer.
 * @param slave      CCP slave instance.
 * @param addr       Requested address.
 * @param size       Access byte length.
 * @param is_write   true if write access is required.
 * @param host_valid true if addr was set directly as a valid host pointer.
 * @param out_ptr    Output resolved host byte pointer.
 * @return true if access is permitted, false otherwise.
 */
static bool ccp_resolve_mem(const SYN_CCP_Slave *slave, uintptr_t addr, uint32_t size,
                            bool is_write, bool host_valid, uint8_t **out_ptr)
{
    if (size == 0U) {
        *out_ptr = NULL;
        return true;
    }
    for (uint8_t i = 0U; i < slave->region_count; i++) {
        int rc = ccp_match_region(&slave->mem_regions[i], addr, size, is_write, out_ptr);
        if (rc != 0) {
            return (rc > 0);
        }
    }
    if (slave->region_count == 0U && host_valid && addr != 0U) {
        *out_ptr = (uint8_t *)addr;
        return true;
    }
    return false;
}

/**
 * @brief Verify that required CCP resource protection mask is unlocked.
 * @param slave   CCP slave instance.
 * @param mask    Required resource bitmask.
 * @param dto_out Outgoing 8-byte CRM DTO frame (error code populated on failure).
 * @return true if resource is unlocked, false otherwise.
 */
static bool ccp_require_resource(const SYN_CCP_Slave *slave, uint8_t mask, uint8_t dto_out[8])
{
    if ((slave->unlocked_resources & mask) == 0U) {
        dto_out[1] = SYN_CCP_ERR_RESOURCE_LOCKED;
        return false;
    }
    return true;
}

/**
 * @brief Handle CCP DISCONNECT command and stop all running DAQ lists.
 * @param slave CCP slave instance.
 * @param cro   Incoming 8-byte CRO payload.
 */
static void ccp_handle_disconnect(SYN_CCP_Slave *slave, const uint8_t cro[8])
{
    uint8_t mode = cro[2];
    uint16_t target_addr = syn_peek_u16_le(cro, 3);
    if (((target_addr == slave->station_addr) || (target_addr == 0x0000U)) &&
        ((mode == 0x00U) || (mode == 0x01U))) {
        slave->connected = false;
        slave->daq_tick_drained = false;
        for (uint8_t i = 0U; i < SYN_CCP_MAX_DAQ_LISTS; i++) {
            slave->daq_lists[i].running = false;
            slave->daq_lists[i].pending_tx = false;
            slave->daq_lists[i].cycle_counter = 0U;
            slave->daq_lists[i].current_odt_idx = 0U;
        }
    }
}

/**
 * @brief Handle session management, version, ID exchange, and seed/key unlock CROs.
 * @param slave   CCP slave instance.
 * @param cmd     CCP command code.
 * @param cro     Incoming 8-byte CRO payload.
 * @param dto_out Outgoing 8-byte CRM DTO payload.
 */
static void ccp_handle_session_sec(SYN_CCP_Slave *slave, uint8_t cmd, const uint8_t cro[8],
                                   uint8_t dto_out[8])
{
    if (cmd == SYN_CCP_CMD_DISCONNECT) {
        ccp_handle_disconnect(slave, cro);
        return;
    }
    if (cmd == SYN_CCP_CMD_GET_CCP_VERSION) {
        dto_out[3] = 0x02U;
        dto_out[4] = 0x01U;
        return;
    }
    uint8_t avail = SYN_CCP_RESOURCE_CAL | SYN_CCP_RESOURCE_DAQ | SYN_CCP_RESOURCE_PGM;
    if (cmd == SYN_CCP_CMD_EXCHANGE_ID) {
        dto_out[3] = 0x08U;
        dto_out[4] = 0x00U;
        dto_out[5] = avail;
        dto_out[6] = (uint8_t)(avail & (uint8_t)~slave->unlocked_resources);
        return;
    }
    if (cmd == SYN_CCP_CMD_GET_SEED) {
        uint8_t req_resource = cro[2];
        bool is_unlocked = (slave->unlocked_resources & req_resource) != 0U;
        dto_out[3] = is_unlocked ? 0x00U : 0x01U;
        slave->pending_unlock_resource = req_resource;
        uint32_t seed =
            (slave->seed_cb != NULL) ? slave->seed_cb(req_resource, slave->seed_ctx) : 0x5A5A5A5AU;
        syn_poke_u32_le(seed, dto_out, 4);
        return;
    }
    uint8_t resource = (cro[2] != 0U) ? cro[2] : slave->pending_unlock_resource;
    uint32_t key = syn_peek_u32_le(cro, 4);
    bool valid =
        (slave->unlock_cb != NULL) ? slave->unlock_cb(resource, key, slave->unlock_ctx) : true;
    if (valid) {
        slave->unlocked_resources |= resource;
    } else {
        slave->unlocked_resources = (uint8_t)(slave->unlocked_resources & (uint8_t)~resource);
        dto_out[1] = SYN_CCP_ERR_ACCESS_DENIED;
    }
    dto_out[3] = (uint8_t)(avail & (uint8_t)~slave->unlocked_resources);
}

/**
 * @brief Handle SET_MTA command to update MTA0 or MTA1 pointer and extension.
 * @param slave CCP slave instance.
 * @param cro   Incoming 8-byte CRO payload.
 */
static void ccp_handle_set_mta(SYN_CCP_Slave *slave, const uint8_t cro[8])
{
    uint8_t mta_num = cro[2];
    uint8_t ext = cro[3];
    uint32_t addr = syn_peek_u32_le(cro, 4);
    if (mta_num == 0U) {
        slave->mta0_ext = ext;
        if (addr != 0U) {
            slave->mta0_addr = (uintptr_t)addr;
            slave->mta0_host_valid = false;
        }
    } else if (mta_num == 1U) {
        slave->mta1_ext = ext;
        if (addr != 0U) {
            slave->mta1_addr = (uintptr_t)addr;
            slave->mta1_host_valid = false;
        }
    }
}

/**
 * @brief Handle DNLOAD and DNLOAD_6 calibration write commands.
 * @param slave   CCP slave instance.
 * @param cmd     CCP command code.
 * @param cro     Incoming 8-byte CRO payload.
 * @param dto_out Outgoing 8-byte CRM DTO payload.
 */
static void ccp_handle_dnload(SYN_CCP_Slave *slave, uint8_t cmd, const uint8_t cro[8],
                              uint8_t dto_out[8])
{
    if (!ccp_require_resource(slave, SYN_CCP_RESOURCE_CAL, dto_out)) {
        return;
    }
    uint8_t size = 6U;
    uint8_t src_off = 2U;
    if (cmd == SYN_CCP_CMD_DNLOAD) {
        size = cro[2];
        src_off = 3U;
        if (size > 5U) {
            dto_out[1] = SYN_CCP_ERR_PARAM_OUT_OF_RANGE;
            return;
        }
    }
    uint8_t *ptr = NULL;
    if (!ccp_resolve_mem(slave, slave->mta0_addr, size, true, slave->mta0_host_valid, &ptr)) {
        dto_out[1] = SYN_CCP_ERR_ACCESS_DENIED;
        return;
    }
    if (ptr != NULL) {
        memcpy(ptr, &cro[src_off], size);
        slave->mta0_addr += size;
    }
    dto_out[3] = slave->mta0_ext;
    syn_poke_u32_le((uint32_t)slave->mta0_addr, dto_out, 4);
}

/**
 * @brief Handle UPLOAD and SHORT_UP memory read commands.
 * @param slave   CCP slave instance.
 * @param cmd     CCP command code.
 * @param cro     Incoming 8-byte CRO payload.
 * @param dto_out Outgoing 8-byte CRM DTO payload.
 */
static void ccp_handle_upload(SYN_CCP_Slave *slave, uint8_t cmd, const uint8_t cro[8],
                              uint8_t dto_out[8])
{
    uint8_t size = cro[2];
    if (size > 5U) {
        dto_out[1] = SYN_CCP_ERR_PARAM_OUT_OF_RANGE;
        return;
    }
    if (cmd == SYN_CCP_CMD_SHORT_UP) {
        uint32_t addr = syn_peek_u32_le(cro, 4);
        slave->mta0_ext = cro[3];
        if (addr != 0U) {
            slave->mta0_addr = (uintptr_t)addr;
            slave->mta0_host_valid = false;
        }
    }
    uint8_t *ptr = NULL;
    if (!ccp_resolve_mem(slave, slave->mta0_addr, size, false, slave->mta0_host_valid, &ptr)) {
        dto_out[1] = SYN_CCP_ERR_ACCESS_DENIED;
        return;
    }
    if (ptr != NULL) {
        memcpy(&dto_out[3], ptr, size);
        slave->mta0_addr += size;
    }
}

/**
 * @brief Handle calibration page switching and flash erase/program commands.
 * @param slave   CCP slave instance.
 * @param cmd     CCP command code.
 * @param cro     Incoming 8-byte CRO payload.
 * @param dto_out Outgoing 8-byte CRM DTO payload.
 */
static void ccp_handle_cal_pgm(SYN_CCP_Slave *slave, uint8_t cmd, const uint8_t cro[8],
                               uint8_t dto_out[8])
{
    if (cmd == SYN_CCP_CMD_SET_CAL_PAGE) {
        if (!ccp_require_resource(slave, SYN_CCP_RESOURCE_CAL, dto_out)) {
            return;
        }
        slave->active_cal_page = (uint8_t)(syn_peek_u32_le(cro, 2) & 0xFFU);
        return;
    }
    if (cmd == SYN_CCP_CMD_GET_ACTIVE_CAL_PAGE) {
        dto_out[3] = 0x00U;
        syn_poke_u32_le((uint32_t)slave->active_cal_page, dto_out, 4);
        return;
    }
    if (!ccp_require_resource(slave, SYN_CCP_RESOURCE_PGM, dto_out)) {
        return;
    }
    if (cmd == SYN_CCP_CMD_CLEAR_MEMORY) {
        uint32_t size = syn_peek_u32_le(cro, 2);
        if (slave->erase_cb != NULL &&
            !slave->erase_cb((uint32_t)slave->mta0_addr, size, slave->erase_ctx)) {
            dto_out[1] = SYN_CCP_ERR_ACCESS_DENIED;
        }
        return;
    }
    uint8_t size = cro[2];
    if (size > 5U) {
        dto_out[1] = SYN_CCP_ERR_PARAM_OUT_OF_RANGE;
        return;
    }
    uint8_t *ptr = NULL;
    if (!ccp_resolve_mem(slave, slave->mta0_addr, size, true, slave->mta0_host_valid, &ptr)) {
        dto_out[1] = SYN_CCP_ERR_ACCESS_DENIED;
        return;
    }
    if (ptr != NULL) {
        memcpy(ptr, &cro[3], size);
        slave->mta0_addr += size;
    }
    dto_out[3] = slave->mta0_ext;
    syn_poke_u32_le((uint32_t)slave->mta0_addr, dto_out, 4);
}

/**
 * @brief Handle START_STOP and START_STOP_ALL DAQ execution control commands.
 * @param slave   CCP slave instance.
 * @param cmd     CCP command code.
 * @param cro     Incoming 8-byte CRO payload.
 * @param dto_out Outgoing 8-byte CRM DTO payload.
 */
static void ccp_handle_daq_run(SYN_CCP_Slave *slave, uint8_t cmd, const uint8_t cro[8],
                               uint8_t dto_out[8])
{
    if (cmd == SYN_CCP_CMD_START_STOP) {
        uint8_t daq_num = cro[3];
        uint16_t prescaler = syn_peek_u16_le(cro, 6);
        if (daq_num >= SYN_CCP_MAX_DAQ_LISTS) {
            dto_out[1] = SYN_CCP_ERR_PARAM_OUT_OF_RANGE;
            return;
        }
        SYN_CCP_DAQList *daq = &slave->daq_lists[daq_num];
        daq->event_channel = cro[5];
        daq->prescaler = (prescaler == 0U) ? 1U : prescaler;
        daq->cycle_counter = 0U;
        daq->current_odt_idx = 0U;
        daq->pending_tx = false;
        daq->running = (cro[2] == 0x01U);
        slave->daq_tick_drained = false;
        return;
    }
    slave->daq_tick_drained = false;
    for (uint8_t i = 0U; i < SYN_CCP_MAX_DAQ_LISTS; i++) {
        slave->daq_lists[i].running = (cro[2] == 0x01U);
        slave->daq_lists[i].pending_tx = false;
        slave->daq_lists[i].cycle_counter = 0U;
        slave->daq_lists[i].current_odt_idx = 0U;
    }
}

/**
 * @brief Handle DAQ list sizing, ODT entry assignment, and start/stop commands.
 * @param slave   CCP slave instance.
 * @param cmd     CCP command code.
 * @param cro     Incoming 8-byte CRO payload.
 * @param dto_out Outgoing 8-byte CRM DTO payload.
 */
static void ccp_handle_daq_cmd(SYN_CCP_Slave *slave, uint8_t cmd, const uint8_t cro[8],
                               uint8_t dto_out[8])
{
    if (!ccp_require_resource(slave, SYN_CCP_RESOURCE_DAQ, dto_out)) {
        return;
    }
    if (cmd == SYN_CCP_CMD_GET_DAQ_SIZE) {
        uint8_t daq_num = cro[2];
        if (daq_num >= SYN_CCP_MAX_DAQ_LISTS) {
            dto_out[1] = SYN_CCP_ERR_PARAM_OUT_OF_RANGE;
            return;
        }
        slave->selected_daq = daq_num;
        dto_out[3] = SYN_CCP_MAX_ODT_PER_DAQ;
        dto_out[4] = (uint8_t)(daq_num * SYN_CCP_MAX_ODT_PER_DAQ);
        return;
    }
    if (cmd == SYN_CCP_CMD_SET_DAQ_SIZE) {
        uint8_t daq_num = cro[2];
        uint8_t count = cro[4];
        if (daq_num >= SYN_CCP_MAX_DAQ_LISTS || count > SYN_CCP_MAX_ODT_PER_DAQ) {
            dto_out[1] = SYN_CCP_ERR_PARAM_OUT_OF_RANGE;
            return;
        }
        SYN_CCP_DAQList *daq = &slave->daq_lists[daq_num];
        memset(daq->odts, 0, sizeof(daq->odts));
        daq->odt_count = count;
        daq->current_odt_idx = 0U;
        daq->current_entry_idx = 0U;
        daq->cycle_counter = 0U;
        daq->running = false;
        daq->pending_tx = false;
        dto_out[3] = count;
        dto_out[4] = (uint8_t)(daq_num * SYN_CCP_MAX_ODT_PER_DAQ);
        return;
    }
    if (cmd == SYN_CCP_CMD_BUILD_PKT) {
        uint8_t daq_num = cro[2];
        uint8_t odt_num = cro[3];
        uint8_t elem_idx = cro[4];
        if (daq_num >= SYN_CCP_MAX_DAQ_LISTS || odt_num >= SYN_CCP_MAX_ODT_PER_DAQ ||
            elem_idx >= SYN_CCP_MAX_ENTRIES_PER_ODT) {
            dto_out[1] = SYN_CCP_ERR_PARAM_OUT_OF_RANGE;
            return;
        }
        SYN_CCP_ODT *odt = &slave->daq_lists[daq_num].odts[odt_num];
        odt->entries[elem_idx].address = slave->mta0_addr;
        odt->entries[elem_idx].size = cro[5];
        odt->entries[elem_idx].host_valid = slave->mta0_host_valid;
        if ((uint8_t)(elem_idx + 1U) > odt->entry_count) {
            odt->entry_count = (uint8_t)(elem_idx + 1U);
        }
        return;
    }
    ccp_handle_daq_run(slave, cmd, cro, dto_out);
}

/**
 * @brief Dispatch memory transfer, calibration page, and programming CRO commands.
 * @param slave   CCP slave instance.
 * @param cmd     CCP command code.
 * @param cro     Incoming 8-byte CRO payload.
 * @param dto_out Outgoing 8-byte CRM DTO payload.
 * @return true if cmd was handled by this dispatcher, false otherwise.
 */
static bool ccp_dispatch_mem_cro(SYN_CCP_Slave *slave, uint8_t cmd, const uint8_t cro[8],
                                 uint8_t dto_out[8])
{
    switch (cmd) {
    case SYN_CCP_CMD_SET_MTA:
        ccp_handle_set_mta(slave, cro);
        return true;
    case SYN_CCP_CMD_DNLOAD:
    case SYN_CCP_CMD_DNLOAD_6:
        ccp_handle_dnload(slave, cmd, cro, dto_out);
        return true;
    case SYN_CCP_CMD_UPLOAD:
    case SYN_CCP_CMD_SHORT_UP:
        ccp_handle_upload(slave, cmd, cro, dto_out);
        return true;
    case SYN_CCP_CMD_SET_CAL_PAGE:
    case SYN_CCP_CMD_GET_ACTIVE_CAL_PAGE:
    case SYN_CCP_CMD_CLEAR_MEMORY:
    case SYN_CCP_CMD_PROGRAM:
        ccp_handle_cal_pgm(slave, cmd, cro, dto_out);
        return true;
    default:
        return false;
    }
}

/**
 * @brief Dispatch any CRO command received while the slave is in connected state.
 * @param slave   CCP slave instance.
 * @param cmd     CCP command code.
 * @param cro     Incoming 8-byte CRO payload.
 * @param dto_out Outgoing 8-byte CRM DTO payload.
 */
static void ccp_dispatch_connected_cro(SYN_CCP_Slave *slave, uint8_t cmd, const uint8_t cro[8],
                                       uint8_t dto_out[8])
{
    if (ccp_dispatch_mem_cro(slave, cmd, cro, dto_out)) {
        return;
    }

    switch (cmd) {
    case SYN_CCP_CMD_DISCONNECT:
    case SYN_CCP_CMD_GET_CCP_VERSION:
    case SYN_CCP_CMD_EXCHANGE_ID:
    case SYN_CCP_CMD_GET_SEED:
    case SYN_CCP_CMD_UNLOCK:
        ccp_handle_session_sec(slave, cmd, cro, dto_out);
        break;
    case SYN_CCP_CMD_GET_DAQ_SIZE:
    case SYN_CCP_CMD_SET_DAQ_SIZE:
    case SYN_CCP_CMD_BUILD_PKT:
    case SYN_CCP_CMD_START_STOP:
    case SYN_CCP_CMD_START_STOP_ALL:
        ccp_handle_daq_cmd(slave, cmd, cro, dto_out);
        break;
    default:
        dto_out[1] = SYN_CCP_ERR_CMD_UNKNOWN;
        break;
    }
}

bool syn_ccp_process_cro(SYN_CCP_Slave *slave, const uint8_t cro[8], uint8_t dto_out[8])
{
    if (slave == NULL || cro == NULL || dto_out == NULL) {
        return false;
    }

    uint8_t cmd = cro[0];
    memset(dto_out, 0, 8U);
    dto_out[0] = SYN_CCP_PID_CRM;
    dto_out[1] = SYN_CCP_ERR_SUCCESS;
    dto_out[2] = cro[1];

    if (cmd == SYN_CCP_CMD_CONNECT) {
        uint16_t target_addr = syn_peek_u16_le(cro, 2);
        if (target_addr != slave->station_addr && target_addr != 0x0000U) {
            return false;
        }
        slave->connected = true;
        return true;
    }

    if (!slave->connected) {
        dto_out[1] = SYN_CCP_ERR_NOT_CONNECTED;
        return true;
    }

    ccp_dispatch_connected_cro(slave, cmd, cro, dto_out);
    return true;
}

/**
 * @brief Check whether any running DAQ list on the event channel has pending ODTs.
 * @param slave         CCP slave instance.
 * @param event_channel Event channel index.
 * @return true if at least one DAQ list already has pending_tx set.
 */
static bool ccp_has_pending_event(const SYN_CCP_Slave *slave, uint8_t event_channel)
{
    for (uint8_t i = 0U; i < SYN_CCP_MAX_DAQ_LISTS; i++) {
        const SYN_CCP_DAQList *daq = &slave->daq_lists[i];
        if (daq->running && daq->event_channel == event_channel && daq->pending_tx) {
            return true;
        }
    }
    return false;
}

/**
 * @brief Advance prescaler counters and arm transmission for matching DAQ lists.
 * @param slave         CCP slave instance.
 * @param event_channel Triggered event channel index.
 */
static void ccp_step_event_counters(SYN_CCP_Slave *slave, uint8_t event_channel)
{
    for (uint8_t i = 0U; i < SYN_CCP_MAX_DAQ_LISTS; i++) {
        SYN_CCP_DAQList *daq = &slave->daq_lists[i];
        if (!daq->running || daq->event_channel != event_channel || daq->odt_count == 0U) {
            continue;
        }
        uint16_t prescaler = (daq->prescaler == 0U) ? 1U : daq->prescaler;
        daq->cycle_counter++;
        if (daq->cycle_counter >= prescaler) {
            daq->cycle_counter = 0U;
            daq->pending_tx = true;
            if (daq->current_odt_idx >= daq->odt_count) {
                daq->current_odt_idx = 0U;
            }
        }
    }
}

/**
 * @brief Sample mapped memory entries and pack a single DAQ ODT DTO frame.
 * @param slave   CCP slave instance.
 * @param odt     Descriptor of ODT to sample.
 * @param pid     Packet identifier byte for DTO[0].
 * @param dto_out Outgoing 8-byte DAQ DTO frame.
 */
static void ccp_pack_odt(const SYN_CCP_Slave *slave, const SYN_CCP_ODT *odt, uint8_t pid,
                         uint8_t dto_out[8])
{
    memset(dto_out, 0, 8U);
    dto_out[0] = pid;
    uint8_t payload_pos = 1U;
    for (uint8_t e = 0U; e < odt->entry_count; e++) {
        const SYN_CCP_ODTEntry *entry = &odt->entries[e];
        uint8_t avail = (uint8_t)(8U - payload_pos);
        if (entry->size > avail) {
            break;
        }
        uint8_t *src = NULL;
        if (entry->size > 0U &&
            ccp_resolve_mem(slave, entry->address, entry->size, false, entry->host_valid, &src) &&
            src != NULL) {
            memcpy(&dto_out[payload_pos], src, entry->size);
            payload_pos = (uint8_t)(payload_pos + entry->size);
        }
    }
}

void syn_ccp_daq_event(SYN_CCP_Slave *slave, uint8_t event_channel)
{
    if (slave == NULL || !slave->connected ||
        (slave->unlocked_resources & SYN_CCP_RESOURCE_DAQ) == 0U) {
        return;
    }
    ccp_step_event_counters(slave, event_channel);
    slave->daq_tick_drained = true;
    slave->daq_tick_ch = event_channel;
}

/**
 * @brief Step DAQ event counters once per tick if not already stepped or draining.
 * @param slave         CCP slave instance.
 * @param event_channel Event channel index.
 */
static void ccp_prepare_daq_event(SYN_CCP_Slave *slave, uint8_t event_channel)
{
    if (ccp_has_pending_event(slave, event_channel)) {
        return;
    }
    if (slave->daq_tick_drained && slave->daq_tick_ch == event_channel) {
        slave->daq_tick_drained = false;
        return;
    }
    slave->daq_tick_drained = false;
    ccp_step_event_counters(slave, event_channel);
}

bool syn_ccp_service_daq(SYN_CCP_Slave *slave, uint8_t event_channel, uint8_t dto_out[8],
                         uint8_t *list_idx, uint8_t *odt_idx)
{
    if (slave == NULL || dto_out == NULL || list_idx == NULL || odt_idx == NULL) {
        return false;
    }
    if (!slave->connected || (slave->unlocked_resources & SYN_CCP_RESOURCE_DAQ) == 0U) {
        return false;
    }

    ccp_prepare_daq_event(slave, event_channel);

    for (uint8_t k = 0U; k < SYN_CCP_MAX_DAQ_LISTS; k++) {
        uint8_t i = (uint8_t)((slave->rr_daq_idx + k) % SYN_CCP_MAX_DAQ_LISTS);
        SYN_CCP_DAQList *daq = &slave->daq_lists[i];
        if (!daq->running || daq->event_channel != event_channel || !daq->pending_tx ||
            daq->odt_count == 0U) {
            continue;
        }

        uint8_t current_odt = daq->current_odt_idx;
        uint8_t pid = (uint8_t)(i * SYN_CCP_MAX_ODT_PER_DAQ + current_odt);
        ccp_pack_odt(slave, &daq->odts[current_odt], pid, dto_out);

        *list_idx = i;
        *odt_idx = current_odt;

        daq->current_odt_idx++;
        if (daq->current_odt_idx >= daq->odt_count) {
            daq->current_odt_idx = 0U;
            daq->pending_tx = false;
            if (!ccp_has_pending_event(slave, event_channel)) {
                slave->daq_tick_drained = true;
                slave->daq_tick_ch = event_channel;
            }
        }
        slave->rr_daq_idx = (uint8_t)((i + 1U) % SYN_CCP_MAX_DAQ_LISTS);
        return true;
    }

    return false;
}
