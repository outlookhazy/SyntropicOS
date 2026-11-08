/**
 * @file syn_gbt27930.h
 * @brief GB/T 27930 (EV DC Fast Charging Protocol on SAE J1939 CAN Bus).
 * @ingroup syn_protocol
 *
 * Implements GB/T 27930-2015 / GB/T 27930-2024 EV DC Fast Charging Protocol
 * state machine and framing for BMS (Vehicle, 0xF4) and Charger (EVSE, 0x56) nodes.
 */

#ifndef SYN_GBT27930_H
#define SYN_GBT27930_H

#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_GBT27930) || SYN_USE_GBT27930

#include "../common/syn_defs.h"
#include "../drivers/syn_can.h"
#include "syn_j1939.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup gbt27930_addresses GB/T 27930 Source Addresses
 *  @{ */
#define SYN_GBT27930_ADDR_BMS 0xF4U     /**< BMS (Vehicle) Source Address */
#define SYN_GBT27930_ADDR_CHARGER 0x56U /**< Charger (EVSE) Source Address */
/** @} */

/** @defgroup gbt27930_pgns GB/T 27930 Parameter Group Numbers
 *  @{ */
#define SYN_GBT27930_PGN_CHM 0x002600U /**< Charger Handshake Message (CHM) */
#define SYN_GBT27930_PGN_BHM 0x002700U /**< BMS Handshake Message (BHM) */
#define SYN_GBT27930_PGN_CRM 0x000100U /**< Charger Recognition Message (CRM) */
#define SYN_GBT27930_PGN_BRM 0x000200U /**< BMS Recognition Message (BRM) */
#define SYN_GBT27930_PGN_BCP 0x000600U /**< BMS Charging Parameter (BCP) */
#define SYN_GBT27930_PGN_CTS 0x000700U /**< Charger Time Sync Message (CTS) */
#define SYN_GBT27930_PGN_CML 0x000800U /**< Charger Max Output Capability (CML) */
#define SYN_GBT27930_PGN_BRO 0x000900U /**< BMS Charging Ready State (BRO) */
#define SYN_GBT27930_PGN_CRO 0x000A00U /**< Charger Output Ready State (CRO) */
#define SYN_GBT27930_PGN_BCL 0x001000U /**< BMS Charging Demand (BCL) */
#define SYN_GBT27930_PGN_BCS 0x001100U /**< BMS Overall Charging Status (BCS) */
#define SYN_GBT27930_PGN_CCS 0x001200U /**< Charger Charging Status (CCS) */
#define SYN_GBT27930_PGN_BSM 0x001300U /**< BMS Status Message (BSM) */
#define SYN_GBT27930_PGN_BST 0x001900U /**< BMS Stop Charging Message (BST) */
#define SYN_GBT27930_PGN_CST 0x001A00U /**< Charger Stop Charging Message (CST) */
#define SYN_GBT27930_PGN_BSD 0x001C00U /**< BMS Statistics Data (BSD) */
#define SYN_GBT27930_PGN_CSD 0x001D00U /**< Charger Statistics Data (CSD) */
#define SYN_GBT27930_PGN_BEM 0x001E00U /**< BMS Error Message (BEM) */
#define SYN_GBT27930_PGN_CEM 0x001F00U /**< Charger Error Message (CEM) */
/** @} */

/**
 * @brief GB/T 27930 Node Role.
 */
typedef enum {
    SYN_GBT27930_ROLE_BMS = 0,    /**< Battery Management System (Vehicle) */
    SYN_GBT27930_ROLE_CHARGER = 1 /**< Off-board DC Charger (EVSE) */
} SYN_GBT27930_Role;

/**
 * @brief GB/T 27930 State Machine Phases.
 */
typedef enum {
    SYN_GBT27930_STATE_IDLE = 0,         /**< Idle / uninitialized phase */
    SYN_GBT27930_STATE_HANDSHAKE = 1,    /**< Handshake and recognition phase */
    SYN_GBT27930_STATE_PARAM_CONFIG = 2, /**< Parameter configuration phase */
    SYN_GBT27930_STATE_CHARGING = 3,     /**< Active power transfer phase */
    SYN_GBT27930_STATE_STOPPING = 4,     /**< Stop charging handshake phase */
    SYN_GBT27930_STATE_ERROR = 5,        /**< Communication timeout or fault phase */
    SYN_GBT27930_STATE_END = 6           /**< End of charging statistics phase (BSD / CSD) */
} SYN_GBT27930_State;

/**
 * @brief BMS Static Parameters (BRM / BCP).
 */
typedef struct {
    uint8_t battery_type;       /**< 0x01=Pb-acid, 0x02=NiMH, 0x03=LiFePO4, 0x06=NMC */
    uint16_t rated_capacity_ah; /**< Rated Capacity (0.1 Ah/bit) */
    uint16_t rated_voltage_v;   /**< Rated Total Voltage (0.1 V/bit) */
    uint16_t max_charge_volt_v; /**< Permissible Max Charge Voltage (0.1 V/bit) */
    uint16_t max_charge_curr_a; /**< Permissible Max Charge Current (0.1 A/bit, offset -400A) */
    uint16_t max_temp_c;        /**< Permissible Max Temperature (1 °C/bit, offset -50°C) */
    uint16_t max_cell_volt_v;   /**< Max Permissible Single Cell Voltage (0.01 V/bit) */
    uint8_t vin[17];            /**< 17-character Vehicle Identification Number */
} SYN_GBT27930_BMS_Config;

/**
 * @brief Charger Static Parameters (CML).
 */
typedef struct {
    uint16_t max_output_volt_v; /**< Max Output Voltage (0.1 V/bit) */
    uint16_t min_output_volt_v; /**< Min Output Voltage (0.1 V/bit) */
    uint16_t max_output_curr_a; /**< Max Output Current (0.1 A/bit, offset -400A) */
    uint16_t min_output_curr_a; /**< Min Output Current (0.1 A/bit, offset -400A) */
} SYN_GBT27930_Charger_Config;

/**
 * @brief Real-time Charging Telemetry (BCL / BCS / CCS / BSM).
 */
typedef struct {
    uint16_t volt_demand_v;       /**< Voltage Demand (0.1 V/bit) */
    uint16_t curr_demand_a;       /**< Current Demand (0.1 A/bit, offset -400A) */
    uint8_t charge_mode;          /**< 0x01=Constant Voltage, 0x02=Constant Current */
    uint16_t measured_volt_v;     /**< Measured Output Voltage (0.1 V/bit) */
    uint16_t measured_curr_a;     /**< Measured Output Current (0.1 A/bit, offset -400A) */
    uint8_t soc_percent;          /**< Battery SOC percentage (0..100 %) */
    uint16_t remaining_time_min;  /**< Remaining charging time in minutes */
    uint16_t max_cell_volt_v;     /**< Measured Max Cell Voltage (0.01 V/bit) */
    uint8_t max_cell_temp_c;      /**< Measured Max Cell Temp (1 °C/bit, offset -50°C) */
    uint8_t min_cell_temp_c;      /**< Measured Min Cell Temp (1 °C/bit, offset -50°C) */
    uint16_t cumulative_time_min; /**< Cumulative charging time in minutes (CCS/CSD) */
    uint16_t output_energy_kwh;   /**< Delivered energy (0.1 kWh/bit, CSD) */
} SYN_GBT27930_Telemetry;

/**
 * @brief GB/T 27930 Session Context.
 */
typedef struct {
    SYN_GBT27930_Role role;                  /**< Node Role (BMS or CHARGER) */
    SYN_GBT27930_State state;                /**< Current State Machine Phase */
    SYN_GBT27930_BMS_Config bms_cfg;         /**< BMS Config */
    SYN_GBT27930_Charger_Config charger_cfg; /**< Charger Config */
    SYN_GBT27930_Telemetry telemetry;        /**< Real-time telemetry */
    bool ready_for_charging;                 /**< Local ready state flag */
    bool peer_ready_for_charging;            /**< Peer ready state flag */
    bool crm00_received;                     /**< BMS received CRM(0x00) recognition prompt */
    bool brm_received;                       /**< Charger received BRM recognition data */
    bool bcp_received;                       /**< Charger received BCP battery parameters */
    bool cml_sent;                           /**< Charger transmitted CML capability frame */
    bool cml_received;                       /**< BMS received CML charger capability */
    bool cts_received;                       /**< BMS received CTS time sync frame */
    bool bro_received;                       /**< Charger received BRO (0x00 or 0xAA) from BMS */
    bool crm_aa_retransmit;                  /**< Charger queued CRM(0xAA) retransmit on BRM */
    bool ready_tx_sent;                      /**< Local BRO(0xAA)/CRO(0xAA) transmitted */
    bool stop_tx_sent;                       /**< Local BST/CST stop frame transmitted */
    bool peer_stopped;                       /**< Peer BST/CST stop frame received */
    bool stats_received;                     /**< Peer BSD/CSD statistics frame received */
    uint8_t tx_substep;                      /**< Periodic multi-PGN schedule index */
    uint32_t timer_ms;                       /**< Periodic transmit timer */
    uint32_t timeout_ms;                     /**< Phase timeout timer */
    uint32_t bcl_timeout_ms;                 /**< Charger 1000 ms BCL demand watchdog timer */
    uint8_t stop_reason;                     /**< Stop reason code */
    uint8_t fault_code;                      /**< Error/Fault code */
    uint8_t sync_time_bcd[7];                /**< Charger BCD timestamp for CTS (optional) */
} SYN_GBT27930_Session;

/**
 * @brief Initialize GB/T 27930 Charging Session.
 * @param session Pointer to session instance.
 * @param role Node role (SYN_GBT27930_ROLE_BMS or SYN_GBT27930_ROLE_CHARGER).
 */
void syn_gbt27930_init(SYN_GBT27930_Session *session, SYN_GBT27930_Role role);

/**
 * @brief Start handshake phase.
 * @param session Pointer to session instance.
 * @return SYN_OK on success.
 */
SYN_Status syn_gbt27930_start_handshake(SYN_GBT27930_Session *session);

/**
 * @brief Process incoming CAN frame on GB/T 27930 session.
 * @param session Pointer to session instance.
 * @param frame Pointer to received CAN frame.
 * @return SYN_OK if frame processed successfully.
 */
SYN_Status syn_gbt27930_process_rx_frame(SYN_GBT27930_Session *session, const SYN_CAN_Frame *frame);

/**
 * @brief Step session state machine timers and produce periodic CAN frames.
 * @param session Pointer to session instance.
 * @param dt_ms Milliseconds elapsed since last call.
 * @param tx_frame Pointer to CAN frame output buffer.
 * @return true if tx_frame contains a CAN frame to transmit, false if idle.
 */
bool syn_gbt27930_step(SYN_GBT27930_Session *session, uint32_t dt_ms, SYN_CAN_Frame *tx_frame);

/**
 * @brief Stop charging session cleanly.
 * @param session Pointer to session instance.
 * @param reason Stop reason code.
 */
void syn_gbt27930_stop_charging(SYN_GBT27930_Session *session, uint8_t reason);

#ifdef __cplusplus
}
#endif

#endif /* SYN_USE_GBT27930 */

#endif /* SYN_GBT27930_H */
