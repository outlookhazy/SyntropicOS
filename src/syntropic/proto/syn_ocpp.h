/**
 * @file syn_ocpp.h
 * @brief Open Charge Point Protocol over JSON (OCPP-J 1.6 / 2.0.1) Dual-Role Engine.
 * @ingroup syn_proto
 *
 * Implements a lightweight, zero-allocation OCPP-J protocol engine supporting both
 * Charge Point (EVSE Client) and Central Management System (CSMS Server) roles.
 */

#ifndef SYN_OCPP_H
#define SYN_OCPP_H

#include "../common/syn_defs.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @name OCPP-J Message Type Identifiers */
/**@{*/
#define SYN_OCPP_MSG_TYPE_CALL 2U       /**< Client/Server Request Call */
#define SYN_OCPP_MSG_TYPE_CALLRESULT 3U /**< Success Response */
#define SYN_OCPP_MSG_TYPE_CALLERROR 4U  /**< Error Response */
/**@}*/

/** @name OCPP-J Standard RPC Error Codes (Specification Section 4.2.3 Table 7) */
/**@{*/
#define SYN_OCPP_ERROR_NOT_IMPLEMENTED "NotImplemented"         /**< Action not implemented */
#define SYN_OCPP_ERROR_NOT_SUPPORTED "NotSupported"             /**< Action not supported */
#define SYN_OCPP_ERROR_INTERNAL_ERROR "InternalError"           /**< Internal error */
#define SYN_OCPP_ERROR_PROTOCOL_ERROR "ProtocolError"           /**< Protocol error */
#define SYN_OCPP_ERROR_SECURITY_ERROR "SecurityError"           /**< Security error */
#define SYN_OCPP_ERROR_FORMATION_VIOLATION "FormationViolation" /**< Syntax formation violation */
#define SYN_OCPP_ERROR_PROPERTY_CONSTRAINT_VIOLATION \
    "PropertyConstraintViolation" /**< Field property constraint violation */
#define SYN_OCPP_ERROR_OCCURRENCE_CONSTRAINT_VIOLATION \
    "OccurenceConstraintViolation" /**< Field occurrence violation */
#define SYN_OCPP_ERROR_TYPE_CONSTRAINT_VIOLATION \
    "TypeConstraintViolation"                       /**< Data type constraint violation */
#define SYN_OCPP_ERROR_GENERIC_ERROR "GenericError" /**< Generic error */
/**@}*/

/** @name OCPP WebSocket Subprotocol Identifiers (OCPP 1.6 / 2.0.1 / 2.1) */
/**@{*/
#define SYN_OCPP_SUBPROTOCOL_2_1 "ocpp2.1"     /**< Official OCPP 2.1 WebSocket Subprotocol */
#define SYN_OCPP_SUBPROTOCOL_2_0_1 "ocpp2.0.1" /**< Official OCPP 2.0.1 WebSocket Subprotocol */
#define SYN_OCPP_SUBPROTOCOL_1_6 "ocpp1.6"     /**< Official OCPP 1.6-J WebSocket Subprotocol */
/**@}*/

/** @brief Supported OCPP Protocol Versions. */
typedef enum {
    SYN_OCPP_VERSION_1_6 = 0, /**< OCPP 1.6-J */
    SYN_OCPP_VERSION_2_0_1,   /**< OCPP 2.0.1 */
    SYN_OCPP_VERSION_2_1      /**< OCPP 2.1 Edition 1 */
} SYN_OCPP_Version;

/** @brief OCPP 2.1 ISO 15118-20 Bidirectional V2G Energy Transfer Control Mode. */
typedef enum {
    SYN_OCPP_V2G_SCHEDULED = 0, /**< ISO 15118-20 Scheduled Control Mode */
    SYN_OCPP_V2G_DYNAMIC        /**< ISO 15118-20 Dynamic Control Mode */
} SYN_OCPP_V2GMode;

/** @brief OCPP 2.1 Customer Display Message Structure. */
typedef struct {
    uint32_t message_id;     /**< Unique display message ID */
    const char *header;      /**< Optional header string */
    const char *content;     /**< Display text content */
    uint32_t display_time_s; /**< Display duration in seconds */
} SYN_OCPP_DisplayMessage;

/** @brief OCPP Connector Status Enumeration (OCPP 1.6 / 2.0.1 / 2.1). */
typedef enum {
    SYN_OCPP_STATUS_AVAILABLE = 0,  /**< Connector available for charging */
    SYN_OCPP_STATUS_PREPARING,      /**< EV connected, preparing session */
    SYN_OCPP_STATUS_CHARGING,       /**< Active charging session */
    SYN_OCPP_STATUS_SUSPENDED_EV,   /**< Suspended by EV */
    SYN_OCPP_STATUS_SUSPENDED_EVSE, /**< Suspended by EVSE station */
    SYN_OCPP_STATUS_FINISHING,      /**< Session ending */
    SYN_OCPP_STATUS_RESERVED,       /**< Reserved connector */
    SYN_OCPP_STATUS_UNAVAILABLE,    /**< Connector unavailable */
    SYN_OCPP_STATUS_FAULTED         /**< Fault condition */
} SYN_OCPP_ChargePointStatus;

/** @brief OCPP Registration Status Enumeration. */
typedef enum {
    SYN_OCPP_REGISTRATION_ACCEPTED = 0, /**< Registration accepted */
    SYN_OCPP_REGISTRATION_PENDING,      /**< Registration pending */
    SYN_OCPP_REGISTRATION_REJECTED      /**< Registration rejected */
} SYN_OCPP_RegistrationStatus;

/** @brief OCPP Authorization Status Enumeration. */
typedef enum {
    SYN_OCPP_AUTH_ACCEPTED = 0, /**< IdTag authorized */
    SYN_OCPP_AUTH_BLOCKED,      /**< IdTag blocked */
    SYN_OCPP_AUTH_EXPIRED,      /**< IdTag expired */
    SYN_OCPP_AUTH_INVALID       /**< IdTag invalid */
} SYN_OCPP_AuthorizationStatus;

/** @brief Charge Point information for BootNotification. */
typedef struct {
    const char *charge_point_vendor; /**< Vendor identifier string */
    const char *charge_point_model;  /**< Model identifier string */
    const char *serial_number;       /**< Charge point serial number */
    const char *firmware_version;    /**< Firmware version string */
} SYN_OCPP_ChargePointInfo;

/** @brief Meter Values reading structure. */
typedef struct {
    uint32_t energy_wh;  /**< Total active energy in Wh */
    uint16_t voltage_v;  /**< RMS Voltage in V */
    uint16_t current_a;  /**< RMS Current in A (deci-Amperes, 0.1A) */
    uint16_t power_kw;   /**< Active Power in W */
    uint8_t soc_percent; /**< State of Charge (0..100%) */
} SYN_OCPP_MeterValues;

/* ── Client Callbacks ── */

/** @brief Remote Start Transaction handler callback. */
typedef bool (*SYN_OCPP_RemoteStartHandler)(uint32_t connector_id, const char *id_tag,
                                            void *user_ctx);

/** @brief Remote Stop Transaction handler callback. */
typedef bool (*SYN_OCPP_RemoteStopHandler)(int32_t transaction_id, void *user_ctx);

/** @brief Registration response callback. */
typedef void (*SYN_OCPP_RegistrationHandler)(SYN_OCPP_RegistrationStatus status,
                                             uint32_t heartbeat_interval_sec, void *user_ctx);

/** @brief Authorization response callback. */
typedef void (*SYN_OCPP_AuthorizationHandler)(const char *id_tag,
                                              SYN_OCPP_AuthorizationStatus status, void *user_ctx);

/** @brief Start Transaction response callback. */
typedef void (*SYN_OCPP_StartTxHandler)(int32_t transaction_id, SYN_OCPP_AuthorizationStatus status,
                                        void *user_ctx);

/** @brief OCPP Client Pending Outbound Action Enumeration. */
typedef enum {
    SYN_OCPP_ACTION_NONE = 0,          /**< No outbound Call awaiting CallResult */
    SYN_OCPP_ACTION_BOOT_NOTIFICATION, /**< Awaiting BootNotification.conf */
    SYN_OCPP_ACTION_HEARTBEAT,         /**< Awaiting Heartbeat.conf */
    SYN_OCPP_ACTION_STATUS_NOTIFY,     /**< Awaiting StatusNotification.conf */
    SYN_OCPP_ACTION_AUTHORIZE,         /**< Awaiting Authorize.conf */
    SYN_OCPP_ACTION_START_TRANSACTION, /**< Awaiting StartTransaction.conf */
    SYN_OCPP_ACTION_STOP_TRANSACTION,  /**< Awaiting StopTransaction.conf */
    SYN_OCPP_ACTION_METER_VALUES,      /**< Awaiting MeterValues.conf */
    SYN_OCPP_ACTION_DISPLAY_MESSAGE,   /**< Awaiting DisplayMessage.conf */
    SYN_OCPP_ACTION_V2G_TRANSFER       /**< Awaiting V2GEnergyTransfer.conf */
} SYN_OCPP_PendingAction;

/** @brief Maximum concurrently tracked outbound OCPP Calls per client. */
#ifndef SYN_OCPP_MAX_PENDING
#define SYN_OCPP_MAX_PENDING 4U
#endif

/** @brief In-flight OCPP outbound Call tracking entry. */
typedef struct {
    SYN_OCPP_PendingAction action; /**< Pending OCPP Call action */
    char unique_id[16];            /**< UniqueId of outbound Call */
} SYN_OCPP_PendingCall;

/** @brief OCPP Client instance state (EVSE Role). */
typedef struct {
    SYN_OCPP_Version version;  /**< Negotiated OCPP version (1.6 / 2.0.1 / 2.1) */
    SYN_OCPP_V2GMode v2g_mode; /**< ISO 15118-20 V2G mode (Scheduled / Dynamic) */
    SYN_OCPP_RegistrationStatus registration_status; /**< Central System registration state */
    uint32_t heartbeat_interval_sec;                 /**< Heartbeat interval in seconds */
    uint32_t heartbeat_timer_ms;                     /**< Heartbeat countdown timer ms */

    uint32_t connector_id;                       /**< Active connector ID (e.g. 1) */
    SYN_OCPP_ChargePointStatus connector_status; /**< Current connector status */

    int32_t active_transaction_id; /**< Active transaction ID (-1 if none) */
    char active_id_tag[32];        /**< Active authorized RFID tag */
    uint32_t tx_start_energy_wh;   /**< Meter Wh at transaction start */

    uint32_t message_counter;              /**< Monotonic message ID counter */
    SYN_OCPP_PendingAction pending_action; /**< Action of most recent outbound Call */
    char pending_unique_id[32];            /**< UniqueId of most recent outbound Call */
    SYN_OCPP_PendingCall pending_calls[SYN_OCPP_MAX_PENDING]; /**< Concurrent in-flight Calls */

    SYN_OCPP_RegistrationHandler reg_cb;         /**< Registration response callback */
    SYN_OCPP_AuthorizationHandler auth_cb;       /**< Authorization response callback */
    SYN_OCPP_StartTxHandler start_tx_cb;         /**< StartTx response callback */
    SYN_OCPP_RemoteStartHandler remote_start_cb; /**< RemoteStart command callback */
    SYN_OCPP_RemoteStopHandler remote_stop_cb;   /**< RemoteStop command callback */
    void *user_ctx;                              /**< User context pointer */
} SYN_OCPP_Client;

/* ── Server Callbacks ── */

/** @brief CSMS Server station boot callback. */
typedef SYN_OCPP_RegistrationStatus (*SYN_OCPP_ServerBootHandler)(
    const SYN_OCPP_ChargePointInfo *info, uint32_t *heartbeat_sec, void *user_ctx);

/** @brief CSMS Server station authorize callback. */
typedef SYN_OCPP_AuthorizationStatus (*SYN_OCPP_ServerAuthorizeHandler)(const char *id_tag,
                                                                        void *user_ctx);

/** @brief CSMS Server station start transaction callback. */
typedef int32_t (*SYN_OCPP_ServerStartTxHandler)(uint32_t connector_id, const char *id_tag,
                                                 uint32_t meter_start_wh, void *user_ctx);

/** @brief OCPP Server instance state (CSMS Central System Role). */
typedef struct {
    uint32_t message_counter;                  /**< Monotonic server message counter */
    int32_t next_transaction_id;               /**< Auto-incrementing transaction ID */
    SYN_OCPP_ServerBootHandler boot_cb;        /**< Station boot registration callback */
    SYN_OCPP_ServerAuthorizeHandler auth_cb;   /**< Station RFID authorization callback */
    SYN_OCPP_ServerStartTxHandler start_tx_cb; /**< Station start transaction callback */
    void *user_ctx;                            /**< User context pointer */
} SYN_OCPP_Server;

/* ── Client API Declarations ─────────────────────────────────────────── */

/**
 * @brief Initialize an OCPP-J client instance.
 * @param client Pointer to client instance.
 * @return SYN_OK on success, SYN_INVALID_PARAM if client is NULL.
 */
SYN_Status syn_ocpp_init(SYN_OCPP_Client *client);

/**
 * @brief Set event and remote command callbacks for OCPP client.
 * @param client Pointer to client instance.
 * @param reg_cb Registration response callback.
 * @param auth_cb Authorization response callback.
 * @param start_tx_cb StartTransaction response callback.
 * @param remote_start_cb RemoteStart command callback.
 * @param remote_stop_cb RemoteStop command callback.
 * @param user_ctx User context pointer.
 * @return SYN_OK on success, SYN_INVALID_PARAM if client is NULL.
 */
SYN_Status syn_ocpp_set_callbacks(SYN_OCPP_Client *client, SYN_OCPP_RegistrationHandler reg_cb,
                                  SYN_OCPP_AuthorizationHandler auth_cb,
                                  SYN_OCPP_StartTxHandler start_tx_cb,
                                  SYN_OCPP_RemoteStartHandler remote_start_cb,
                                  SYN_OCPP_RemoteStopHandler remote_stop_cb, void *user_ctx);

/**
 * @brief Format an OCPP-J BootNotification.req Call frame.
 * @param client Pointer to client instance.
 * @param info Charge point info.
 * @param out_buf Output text buffer.
 * @param max_len Buffer capacity.
 * @param out_len Pointer to store length.
 * @return SYN_OK on success, error code on failure.
 */
SYN_Status syn_ocpp_format_boot_notification(SYN_OCPP_Client *client,
                                             const SYN_OCPP_ChargePointInfo *info, char *out_buf,
                                             size_t max_len, size_t *out_len);

/**
 * @brief Format an OCPP-J Heartbeat.req Call frame.
 * @param client Pointer to client instance.
 * @param out_buf Output text buffer.
 * @param max_len Buffer capacity.
 * @param out_len Pointer to store length.
 * @return SYN_OK on success, error code on failure.
 */
SYN_Status syn_ocpp_format_heartbeat(SYN_OCPP_Client *client, char *out_buf, size_t max_len,
                                     size_t *out_len);

/**
 * @brief Format an OCPP-J StatusNotification.req Call frame.
 * @param client Pointer to client instance.
 * @param connector_id Connector ID.
 * @param status Status enum.
 * @param error_code Error code string.
 * @param out_buf Output text buffer.
 * @param max_len Buffer capacity.
 * @param out_len Pointer to store length.
 * @return SYN_OK on success, error code on failure.
 */
SYN_Status syn_ocpp_format_status_notification(SYN_OCPP_Client *client, uint32_t connector_id,
                                               SYN_OCPP_ChargePointStatus status,
                                               const char *error_code, char *out_buf,
                                               size_t max_len, size_t *out_len);

/**
 * @brief Format an OCPP-J Authorize.req Call frame.
 * @param client Pointer to client instance.
 * @param id_tag RFID tag string.
 * @param out_buf Output text buffer.
 * @param max_len Buffer capacity.
 * @param out_len Pointer to store length.
 * @return SYN_OK on success, error code on failure.
 */
SYN_Status syn_ocpp_format_authorize(SYN_OCPP_Client *client, const char *id_tag, char *out_buf,
                                     size_t max_len, size_t *out_len);

/**
 * @brief Format an OCPP-J StartTransaction.req Call frame.
 * @param client Pointer to client instance.
 * @param connector_id Connector ID.
 * @param id_tag RFID tag string.
 * @param meter_start_wh Meter reading in Wh.
 * @param out_buf Output text buffer.
 * @param max_len Buffer capacity.
 * @param out_len Pointer to store length.
 * @return SYN_OK on success, error code on failure.
 */
SYN_Status syn_ocpp_format_start_transaction(SYN_OCPP_Client *client, uint32_t connector_id,
                                             const char *id_tag, uint32_t meter_start_wh,
                                             char *out_buf, size_t max_len, size_t *out_len);

/**
 * @brief Format an OCPP-J StopTransaction.req Call frame.
 * @param client Pointer to client instance.
 * @param transaction_id Active transaction ID.
 * @param meter_stop_wh Meter reading in Wh.
 * @param reason Reason string.
 * @param out_buf Output text buffer.
 * @param max_len Buffer capacity.
 * @param out_len Pointer to store length.
 * @return SYN_OK on success, error code on failure.
 */
SYN_Status syn_ocpp_format_stop_transaction(SYN_OCPP_Client *client, int32_t transaction_id,
                                            uint32_t meter_stop_wh, const char *reason,
                                            char *out_buf, size_t max_len, size_t *out_len);

/**
 * @brief Format an OCPP-J MeterValues.req Call frame.
 * @param client Pointer to client instance.
 * @param connector_id Connector ID.
 * @param values Meter readings pointer.
 * @param out_buf Output text buffer.
 * @param max_len Buffer capacity.
 * @param out_len Pointer to store length.
 * @return SYN_OK on success, error code on failure.
 */
SYN_Status syn_ocpp_format_meter_values(SYN_OCPP_Client *client, uint32_t connector_id,
                                        const SYN_OCPP_MeterValues *values, char *out_buf,
                                        size_t max_len, size_t *out_len);

/**
 * @brief Format an OCPP 2.1 Customer DisplayMessage Call frame.
 * @param client Pointer to client instance.
 * @param msg Pointer to DisplayMessage structure.
 * @param out_buf Output text buffer.
 * @param max_len Buffer capacity.
 * @param out_len Pointer to store length.
 * @return SYN_OK on success, error code on failure.
 */
SYN_Status syn_ocpp_format_display_message(SYN_OCPP_Client *client,
                                           const SYN_OCPP_DisplayMessage *msg, char *out_buf,
                                           size_t max_len, size_t *out_len);

/**
 * @brief Format an OCPP 2.1 ISO 15118-20 V2G Energy Transfer Request Call frame.
 * @param client Pointer to client instance.
 * @param mode V2G control mode (Scheduled / Dynamic).
 * @param power_limit_w Power limit in Watts (positive for charging, negative for discharging).
 * @param out_buf Output text buffer.
 * @param max_len Buffer capacity.
 * @param out_len Pointer to store length.
 * @return SYN_OK on success, error code on failure.
 */
SYN_Status syn_ocpp_format_v2g_energy_transfer(SYN_OCPP_Client *client, SYN_OCPP_V2GMode mode,
                                               int32_t power_limit_w, char *out_buf, size_t max_len,
                                               size_t *out_len);

/**
 * @brief Process an incoming OCPP-J JSON frame.
 * @param client Pointer to client instance.
 * @param in_buf Incoming text frame.
 * @param in_len Frame length.
 * @param out_resp Optional response buffer.
 * @param max_resp_len Response capacity.
 * @param out_resp_len Pointer to store response length.
 * @return SYN_OK on success, error code on failure.
 */
SYN_Status syn_ocpp_process_message(SYN_OCPP_Client *client, const char *in_buf, size_t in_len,
                                    char *out_resp, size_t max_resp_len, size_t *out_resp_len);

/**
 * @brief Tick periodic Heartbeat timer for OCPP client.
 * @param client Pointer to client instance.
 * @param dt_ms Elapsed time step in ms.
 * @param out_hb_buf Output heartbeat buffer.
 * @param max_len Buffer capacity.
 * @param out_len Pointer to store length.
 */
void syn_ocpp_tick(SYN_OCPP_Client *client, uint32_t dt_ms, char *out_hb_buf, size_t max_len,
                   size_t *out_len);

/* ── Server API Declarations (CSMS Role) ─────────────────────────────── */

/**
 * @brief Initialize an OCPP CSMS Server instance.
 * @param server Pointer to server instance.
 * @return SYN_OK on success, SYN_INVALID_PARAM if server is NULL.
 */
SYN_Status syn_ocpp_server_init(SYN_OCPP_Server *server);

/**
 * @brief Set event callbacks for OCPP CSMS Server.
 * @param server Pointer to server instance.
 * @param boot_cb Station registration callback.
 * @param auth_cb RFID authorization callback.
 * @param start_tx_cb StartTransaction callback.
 * @param user_ctx User context pointer.
 * @return SYN_OK on success, SYN_INVALID_PARAM if server is NULL.
 */
SYN_Status syn_ocpp_server_set_callbacks(SYN_OCPP_Server *server,
                                         SYN_OCPP_ServerBootHandler boot_cb,
                                         SYN_OCPP_ServerAuthorizeHandler auth_cb,
                                         SYN_OCPP_ServerStartTxHandler start_tx_cb, void *user_ctx);

/**
 * @brief Format a RemoteStartTransaction.req Call frame from CSMS server to station.
 * @param server Pointer to server instance.
 * @param connector_id Target connector ID.
 * @param id_tag Target RFID tag string.
 * @param out_buf Output text buffer for JSON payload.
 * @param max_len Capacity of output buffer.
 * @param out_len Pointer to store output byte count.
 * @return SYN_OK on success, SYN_INVALID_PARAM or SYN_ERROR on failure.
 */
SYN_Status syn_ocpp_server_format_remote_start(SYN_OCPP_Server *server, uint32_t connector_id,
                                               const char *id_tag, char *out_buf, size_t max_len,
                                               size_t *out_len);

/**
 * @brief Format a RemoteStopTransaction.req Call frame from CSMS server to station.
 * @param server Pointer to server instance.
 * @param transaction_id Active transaction ID to stop.
 * @param out_buf Output text buffer for JSON payload.
 * @param max_len Capacity of output buffer.
 * @param out_len Pointer to store output byte count.
 * @return SYN_OK on success, SYN_INVALID_PARAM or SYN_ERROR on failure.
 */
SYN_Status syn_ocpp_server_format_remote_stop(SYN_OCPP_Server *server, int32_t transaction_id,
                                              char *out_buf, size_t max_len, size_t *out_len);

/**
 * @brief Process incoming station request frame on CSMS server and generate response CallResult.
 * @param server Pointer to server instance.
 * @param in_buf Incoming JSON frame from station.
 * @param in_len Length of incoming JSON frame.
 * @param out_resp Output buffer for CallResult response frame.
 * @param max_resp_len Capacity of output response buffer.
 * @param out_resp_len Pointer to store response byte count.
 * @return SYN_OK on success, SYN_INVALID_PARAM or SYN_ERROR on failure.
 */
SYN_Status syn_ocpp_server_process_message(SYN_OCPP_Server *server, const char *in_buf,
                                           size_t in_len, char *out_resp, size_t max_resp_len,
                                           size_t *out_resp_len);

/**
 * @brief Format an OCPP-J CallError (MessageTypeId 4) response frame.
 * @param msg_id Unique message identifier string to match Call request.
 * @param error_code Error code string (e.g. SYN_OCPP_ERROR_NOT_IMPLEMENTED).
 * @param error_description Optional human-readable error description string.
 * @param out_buf Output text buffer.
 * @param max_len Buffer capacity.
 * @param out_len Pointer to store length.
 * @return SYN_OK on success, SYN_INVALID_PARAM or SYN_ERROR on failure.
 */
SYN_Status syn_ocpp_format_call_error(const char *msg_id, const char *error_code,
                                      const char *error_description, char *out_buf, size_t max_len,
                                      size_t *out_len);

#ifdef __cplusplus
}
#endif

#endif /* SYN_OCPP_H */
