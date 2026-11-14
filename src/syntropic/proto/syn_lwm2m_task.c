/**
 * @file syn_lwm2m_task.c
 * @brief Autonomous OMA LwM2M Client Supervisor Task & Engine Implementation.
 */

#include "syn_lwm2m_task.h"

#if SYN_LWM2M_TASK_ENABLED

#include "../port/syn_port_system.h"
#include "../sched/syn_sched.h"
#include "../util/syn_assert.h"

#include <stdio.h>
#include <string.h>

#define SYN_LWM2M_DEFAULT_RETRY_BACKOFF_MS 5000U /**< Default network retry backoff (ms) */
#define SYN_LWM2M_RESP_PAYLOAD_BUF_SZ 256U       /**< Response payload scratch size */

/* ── Internal State & Transport Helpers ─────────────────────────────────── */

/**
 * @brief Resolve active datagram transport (DTLS 1.3 channel when configured, else raw UDP).
 * @param task LwM2M supervisor task instance.
 * @return Active transport pointer.
 */
static SYN_Transport *lwm2m_active_transport(const SYN_LwM2M_Task *task)
{
#if SYN_DTLS_ENABLED
    if (task->cfg.coaps_client != NULL) {
        return &task->cfg.coaps_client->dtls_transport;
    }
#endif
    return task->cfg.transport;
}

/**
 * @brief Update client state and fire state change callback if transition occurs.
 * @param task      LwM2M supervisor task instance.
 * @param new_state Target client registration state.
 */
static void syn_lwm2m_task_set_state(SYN_LwM2M_Task *task, SYN_LwM2M_ClientState new_state)
{
    SYN_LwM2M_Client *client = task->cfg.client;
    if (new_state == SYN_LWM2M_STATE_DEREGISTERED) {
        (void)memset(client->observations, 0, sizeof(client->observations));
        (void)memset(task->pending_notify, 0, sizeof(task->pending_notify));
    }
    SYN_LwM2M_ClientState old_state = client->state;
    if (old_state != new_state) {
        client->state = new_state;
        if (task->cfg.on_state != NULL) {
            task->cfg.on_state(old_state, new_state, task->cfg.user_data);
        }
    }
}

/* ── Object Command Hook Dispatcher ──────────────────────────────────────── */

/**
 * @brief Check registered objects for pending command flags (reboot, reset, fw update).
 * @param task LwM2M supervisor task instance.
 */
static void syn_lwm2m_task_check_object_hooks(SYN_LwM2M_Task *task)
{
    SYN_LwM2M_Client *client = task->cfg.client;
    for (size_t i = 0; i < client->object_count; i++) {
        const SYN_LwM2M_Object *obj = client->objects[i];
        if (obj == NULL || obj->instance_count == 0U) {
            continue;
        }

        /* Check Device Object (Obj 3) */
        if (obj->id == SYN_LWM2M_OBJ_DEVICE) {
            SYN_LwM2M_DeviceContext *dev_ctx = (SYN_LwM2M_DeviceContext *)obj->user_ctx;
            if (dev_ctx != NULL) {
                if (dev_ctx->reboot_requested) {
                    dev_ctx->reboot_requested = false;
                    if (task->cfg.on_reboot != NULL) {
                        task->cfg.on_reboot(task->cfg.user_data);
                    }
                }
                if (dev_ctx->factory_reset_requested) {
                    dev_ctx->factory_reset_requested = false;
                    if (task->cfg.on_reset != NULL) {
                        task->cfg.on_reset(task->cfg.user_data);
                    }
                }
            }
        }
        /* Check Firmware Update Object (Obj 5) */
        else if (obj->id == SYN_LWM2M_OBJ_FIRMWARE_UPDATE) {
            SYN_LwM2M_FirmwareContext *fw_ctx = (SYN_LwM2M_FirmwareContext *)obj->user_ctx;
            if (fw_ctx != NULL && fw_ctx->update_requested) {
                fw_ctx->update_requested = false;
                if (task->cfg.on_fw_update != NULL) {
                    task->cfg.on_fw_update(fw_ctx->package_uri, task->cfg.user_data);
                }
            }
        } else {
            /* Other objects do not have standard execute hooks */
        }
    }
}

/* ── Observation Evaluator ───────────────────────────────────────────────── */

/**
 * @brief Convert an observation period from seconds to milliseconds with saturation.
 * @param sec Period in seconds.
 * @return Period in milliseconds, clamped to UINT32_MAX.
 */
static uint32_t lwm2m_sec_to_ms_clamped(uint32_t sec)
{
    return (sec > (UINT32_MAX / 1000U)) ? UINT32_MAX : (sec * 1000U);
}

/**
 * @brief Check whether a pending resource change matches an observation slot.
 * @param task LwM2M supervisor task instance.
 * @param obs  Active observation slot.
 * @return true if the resource change triggers this observation.
 */
static bool lwm2m_obs_matches_change(const SYN_LwM2M_Task *task, const SYN_LwM2M_Observation *obs)
{
    for (size_t i = 0U; i < task->dirty_count; i++) {
        const SYN_LwM2M_DirtyEntry *d = &task->dirty_queue[i];
        if (d->obj_id == obs->obj_id && d->inst_id == obs->inst_id &&
            (obs->res_id == -1 || d->res_id == -1 || obs->res_id == d->res_id)) {
            return true;
        }
    }
    return false;
}

/**
 * @brief Build and transmit a single CoAP Observe notification for @p obs.
 * @param task         LwM2M supervisor task instance.
 * @param obs          Active observation entry.
 * @param now_ms       Current system timestamp in milliseconds.
 * @param pending_flag [in,out] Latched dirty flag cleared after evaluation.
 */
static void lwm2m_task_send_obs_notify(SYN_LwM2M_Task *task, SYN_LwM2M_Observation *obs,
                                       uint32_t now_ms, bool *pending_flag)
{
    SYN_LwM2M_Client *client = task->cfg.client;
    SYN_CoapMsg notify_msg = {0};
    SYN_CoapOption notify_opts[8];
    size_t notify_opt_cnt = 0U;
    uint8_t notify_pld[SYN_LWM2M_RESP_PAYLOAD_BUF_SZ];
    uint16_t mid = task->next_msg_id++;

    *pending_flag = false;
    size_t pld_len =
        syn_lwm2m_build_notify(client, obs->obj_id, obs->inst_id, obs->res_id, mid, &notify_msg,
                               notify_opts, 8U, &notify_opt_cnt, notify_pld, sizeof(notify_pld));

    if (pld_len > 0U || notify_msg.code == COAP_RESP_CONTENT) {
        size_t frame_len = syn_coap_serialize(&notify_msg, notify_opts, notify_opt_cnt,
                                              task->cfg.tx_buf, task->cfg.tx_buf_size);
        if (frame_len > 0U) {
            (void)syn_transport_send(lwm2m_active_transport(task), task->cfg.tx_buf, frame_len);
            obs->last_notify_ms = now_ms;
        }
    }
}

/**
 * @brief Evaluate active observations against resource changes and pmax intervals.
 * @param task   LwM2M supervisor task instance.
 * @param now_ms Current system timestamp in milliseconds.
 */
static void syn_lwm2m_task_eval_observations(SYN_LwM2M_Task *task, uint32_t now_ms)
{
    SYN_LwM2M_Client *client = task->cfg.client;

    for (size_t i = 0; i < SYN_LWM2M_MAX_OBSERVERS; i++) {
        SYN_LwM2M_Observation *obs = &client->observations[i];
        if (!obs->active) {
            task->pending_notify[i] = false;
            continue;
        }

        if (lwm2m_obs_matches_change(task, obs)) {
            task->pending_notify[i] = true;
        }
        uint32_t pmax_ms = lwm2m_sec_to_ms_clamped(obs->pmax);
        bool pmax_expired = (obs->pmax > 0U) && ((now_ms - obs->last_notify_ms) >= pmax_ms);
        uint32_t pmin_ms = lwm2m_sec_to_ms_clamped(obs->pmin);
        bool pmin_satisfied = (obs->pmin == 0U) || ((now_ms - obs->last_notify_ms) >= pmin_ms);

        if ((task->pending_notify[i] || pmax_expired) && pmin_satisfied) {
            lwm2m_task_send_obs_notify(task, obs, now_ms, &task->pending_notify[i]);
        }
    }

    task->dirty_count = 0U;
}

/* ── Public API ─────────────────────────────────────────────────────────── */

SYN_Status syn_lwm2m_task_init(SYN_LwM2M_Task *task, const SYN_LwM2M_TaskConfig *cfg)
{
    if (task == NULL || cfg == NULL) {
        return SYN_INVALID_PARAM;
    }
    if (cfg->client == NULL || cfg->transport == NULL) {
        return SYN_INVALID_PARAM;
    }
    if (cfg->rx_buf == NULL || cfg->rx_buf_size < 64U) {
        return SYN_INVALID_PARAM;
    }
    if (cfg->tx_buf == NULL || cfg->tx_buf_size < 64U) {
        return SYN_INVALID_PARAM;
    }

    (void)memset(task, 0, sizeof(SYN_LwM2M_Task));
    task->cfg = *cfg;

    if (task->cfg.retry_backoff_ms == 0U) {
        task->cfg.retry_backoff_ms = SYN_LWM2M_DEFAULT_RETRY_BACKOFF_MS;
    }

    task->cfg.client->transport = lwm2m_active_transport(task);
    task->next_msg_id = 0x1000U;

    return SYN_OK;
}

SYN_Status syn_lwm2m_task_trigger_update(SYN_LwM2M_Task *task)
{
    if (task == NULL) {
        return SYN_INVALID_PARAM;
    }
    task->force_update = true;
    return SYN_OK;
}

SYN_Status syn_lwm2m_task_deregister(SYN_LwM2M_Task *task)
{
    if (task == NULL) {
        return SYN_INVALID_PARAM;
    }
    task->force_deregister = true;
    return SYN_OK;
}

SYN_Status syn_lwm2m_task_notify_changed(SYN_LwM2M_Task *task, uint16_t obj_id, uint16_t inst_id,
                                         int32_t res_id)
{
    if (task == NULL) {
        return SYN_INVALID_PARAM;
    }
    for (size_t i = 0U; i < task->dirty_count; i++) {
        if (task->dirty_queue[i].obj_id == obj_id && task->dirty_queue[i].inst_id == inst_id &&
            task->dirty_queue[i].res_id == res_id) {
            return SYN_OK;
        }
    }
    if (task->dirty_count >= SYN_LWM2M_MAX_DIRTY_QUEUE) {
        return SYN_BUSY;
    }
    task->dirty_queue[task->dirty_count].obj_id = obj_id;
    task->dirty_queue[task->dirty_count].inst_id = inst_id;
    task->dirty_queue[task->dirty_count].res_id = res_id;
    task->dirty_count++;
    return SYN_OK;
}

/**
 * @brief Send graceful deregistration request if currently registered and reset client state.
 * @param task LwM2M supervisor task instance.
 */
static void lwm2m_task_handle_deregister(SYN_LwM2M_Task *task)
{
    SYN_LwM2M_Client *client = task->cfg.client;
    task->force_deregister = false;
    if (client->state == SYN_LWM2M_STATE_REGISTERED || client->state == SYN_LWM2M_STATE_UPDATING) {
        SYN_CoapMsg dereg_msg = {0};
        SYN_CoapOption dereg_opts[8];
        size_t dereg_opt_cnt = 0U;
        uint16_t mid = task->next_msg_id++;

        size_t res =
            syn_lwm2m_build_deregister_request(client, mid, (const uint8_t *)&mid, sizeof(mid),
                                               &dereg_msg, dereg_opts, 8U, &dereg_opt_cnt);
        if (res > 0U) {
            size_t frame_len = syn_coap_serialize(&dereg_msg, dereg_opts, dereg_opt_cnt,
                                                  task->cfg.tx_buf, task->cfg.tx_buf_size);
            if (frame_len > 0U) {
                (void)syn_transport_send(lwm2m_active_transport(task), task->cfg.tx_buf, frame_len);
            }
        }
    }
    syn_lwm2m_task_set_state(task, SYN_LWM2M_STATE_DEREGISTERED);
    client->location_path[0] = '\0';
}

/**
 * @brief Build and transmit a Registration Update request.
 * @param task   LwM2M supervisor task instance.
 * @param now_ms Current system timestamp in milliseconds.
 */
static void lwm2m_task_send_update(SYN_LwM2M_Task *task, uint32_t now_ms)
{
    SYN_LwM2M_Client *client = task->cfg.client;
    SYN_CoapMsg upd_msg = {0};
    SYN_CoapOption upd_opts[8];
    size_t upd_opt_cnt = 0U;
    uint8_t upd_pld[SYN_LWM2M_RESP_PAYLOAD_BUF_SZ];
    uint16_t mid = task->next_msg_id++;

    size_t upd_len = syn_lwm2m_build_update_request(client, mid, (const uint8_t *)&mid, sizeof(mid),
                                                    client->lifetime_s, &upd_msg, upd_opts, 8U,
                                                    &upd_opt_cnt, upd_pld, sizeof(upd_pld));
    if (upd_msg.code == COAP_CODE_POST) {
        upd_msg.payload = (upd_len > 0U) ? upd_pld : NULL;
        upd_msg.payload_len = upd_len;

        size_t frame_len = syn_coap_serialize(&upd_msg, upd_opts, upd_opt_cnt, task->cfg.tx_buf,
                                              task->cfg.tx_buf_size);
        if (frame_len > 0U) {
            (void)syn_transport_send(lwm2m_active_transport(task), task->cfg.tx_buf, frame_len);
            task->pending_msg_id = mid;
            syn_lwm2m_task_set_state(task, SYN_LWM2M_STATE_UPDATING);
            task->last_action_ms = now_ms;
            task->has_acted = true;
        }
    }
}

/**
 * @brief Drive autonomous server registration and 80% lifetime renewal timers.
 * @param task   LwM2M supervisor task instance.
 * @param now_ms Current system timestamp in milliseconds.
 */
static void lwm2m_task_step_registration(SYN_LwM2M_Task *task, uint32_t now_ms)
{
    SYN_LwM2M_Client *client = task->cfg.client;

    switch (client->state) {
    case SYN_LWM2M_STATE_DEREGISTERED: {
        if ((!task->has_acted && task->last_action_ms == 0U) ||
            (now_ms - task->last_action_ms) >= task->cfg.retry_backoff_ms) {
            SYN_CoapMsg reg_msg = {0};
            SYN_CoapOption reg_opts[8];
            size_t reg_opt_cnt = 0U;
            uint8_t reg_pld[SYN_LWM2M_RESP_PAYLOAD_BUF_SZ];
            uint16_t mid = task->next_msg_id++;

            size_t pld_len = syn_lwm2m_build_register_request(
                client, mid, (const uint8_t *)&mid, sizeof(mid), &reg_msg, reg_opts, 8U,
                &reg_opt_cnt, reg_pld, sizeof(reg_pld));
            if (pld_len > 0U) {
                reg_msg.payload = reg_pld;
                reg_msg.payload_len = pld_len;

                size_t frame_len = syn_coap_serialize(&reg_msg, reg_opts, reg_opt_cnt,
                                                      task->cfg.tx_buf, task->cfg.tx_buf_size);
                if (frame_len > 0U) {
                    (void)syn_transport_send(lwm2m_active_transport(task), task->cfg.tx_buf,
                                             frame_len);
                    task->pending_msg_id = mid;
                    syn_lwm2m_task_set_state(task, SYN_LWM2M_STATE_REGISTERING);
                    task->last_action_ms = now_ms;
                    task->has_acted = true;
                }
            }
        }
        break;
    }

    case SYN_LWM2M_STATE_REGISTERING:
    case SYN_LWM2M_STATE_UPDATING:
        if ((now_ms - task->last_action_ms) >= task->cfg.retry_backoff_ms) {
            syn_lwm2m_task_set_state(task, SYN_LWM2M_STATE_DEREGISTERED);
            task->retry_count++;
            task->last_action_ms = now_ms;
            task->has_acted = true;
        }
        break;

    case SYN_LWM2M_STATE_REGISTERED: {
        /* Check if lifetime renewal is due (renew at 80% of lifetime) or forced */
        uint64_t renew_ms64 = (uint64_t)client->lifetime_s * 800ULL;
        uint32_t renew_interval_ms =
            (renew_ms64 > (uint64_t)UINT32_MAX) ? UINT32_MAX : (uint32_t)renew_ms64;
        if (renew_interval_ms == 0U) {
            renew_interval_ms = 60000U;
        }

        bool need_update = task->force_update;
        if (now_ms - client->last_update_ms >= renew_interval_ms) {
            need_update = true;
        }

        if (need_update) {
            task->force_update = false;
            lwm2m_task_send_update(task, now_ms);
        }
        break;
    }

    default:
        break;
    }
}

/**
 * @brief Handle inbound CoAP response packets for registration and lifetime update.
 * @param task       LwM2M supervisor task instance.
 * @param in_msg     Parsed inbound CoAP message.
 * @param in_opts    Parsed CoAP options array.
 * @param in_opt_cnt Number of parsed CoAP options.
 * @param now_ms     Current system timestamp in milliseconds.
 */
static void lwm2m_task_handle_response(SYN_LwM2M_Task *task, const SYN_CoapMsg *in_msg,
                                       const SYN_CoapOption *in_opts, size_t in_opt_cnt,
                                       uint32_t now_ms)
{
    SYN_LwM2M_Client *client = task->cfg.client;

    if (client->state != SYN_LWM2M_STATE_REGISTERING && client->state != SYN_LWM2M_STATE_UPDATING) {
        return;
    }
    if (in_msg->msg_id != task->pending_msg_id) {
        return;
    }

    if (client->state == SYN_LWM2M_STATE_REGISTERING) {
        SYN_LwM2M_ClientState prev_st = client->state;
        if (syn_lwm2m_handle_register_response(client, in_msg, in_opts, in_opt_cnt)) {
            client->last_update_ms = now_ms;
            task->retry_count = 0U;
            if (task->cfg.on_state != NULL && prev_st != SYN_LWM2M_STATE_REGISTERED) {
                task->cfg.on_state(prev_st, SYN_LWM2M_STATE_REGISTERED, task->cfg.user_data);
            }
        } else {
            syn_lwm2m_task_set_state(task, SYN_LWM2M_STATE_DEREGISTERED);
            task->retry_count++;
        }
    } else {
        if (in_msg->code == COAP_RESP_CHANGED) {
            syn_lwm2m_task_set_state(task, SYN_LWM2M_STATE_REGISTERED);
            client->last_update_ms = now_ms;
            task->retry_count = 0U;
        } else {
            syn_lwm2m_task_set_state(task, SYN_LWM2M_STATE_DEREGISTERED);
            task->retry_count++;
        }
    }
}

/**
 * @brief Receive and process at most one inbound CoAP datagram from the server.
 * @param task   LwM2M supervisor task instance.
 * @param now_ms Current system timestamp in milliseconds.
 * @return true if a non-empty datagram was consumed from the transport.
 */
static bool lwm2m_task_process_inbound(SYN_LwM2M_Task *task, uint32_t now_ms)
{
    SYN_LwM2M_Client *client = task->cfg.client;
    SYN_Transport *active_tr = lwm2m_active_transport(task);
    size_t rx_len = 0U;

    if (!syn_transport_recv(active_tr, task->cfg.rx_buf, task->cfg.rx_buf_size, &rx_len) ||
        rx_len == 0U) {
        return false;
    }

    SYN_CoapMsg in_msg = {0};
    SYN_CoapOption in_opts[8];
    size_t in_opt_cnt = 0U;

    if (syn_coap_parse(&in_msg, in_opts, 8U, &in_opt_cnt, task->cfg.rx_buf, rx_len) == SYN_OK) {
        if (in_msg.code >= COAP_RESP_CREATED) {
            lwm2m_task_handle_response(task, &in_msg, in_opts, in_opt_cnt, now_ms);
        } else if (in_msg.code >= COAP_CODE_GET && in_msg.code <= COAP_CODE_DELETE) {
            if (client->state == SYN_LWM2M_STATE_REGISTERED ||
                client->state == SYN_LWM2M_STATE_UPDATING) {
                SYN_CoapMsg resp_msg = {0};
                SYN_CoapOption resp_opts[8];
                size_t resp_opt_cnt = 0U;
                uint8_t resp_pld[SYN_LWM2M_RESP_PAYLOAD_BUF_SZ];

                SYN_Status proc_st = syn_lwm2m_process_request(
                    client, &in_msg, in_opts, in_opt_cnt, &resp_msg, resp_opts, 8U, &resp_opt_cnt,
                    resp_pld, sizeof(resp_pld));

                if (proc_st == SYN_OK) {
                    size_t frame_len = syn_coap_serialize(&resp_msg, resp_opts, resp_opt_cnt,
                                                          task->cfg.tx_buf, task->cfg.tx_buf_size);
                    if (frame_len > 0U) {
                        (void)syn_transport_send(active_tr, task->cfg.tx_buf, frame_len);
                    }
                }

                syn_lwm2m_task_check_object_hooks(task);
            }
        } else {
            /* Empty or Ping message */
        }
    }

    return true;
}

/**
 * @brief Advance the LwM2M supervisor state machine and report inbound packet progress.
 * @param task            LwM2M supervisor task instance.
 * @param now_ms          Current system timestamp in milliseconds.
 * @param out_rx_consumed [out] Optional flag set true if an inbound datagram was consumed.
 * @return SYN_OK on normal operation, error status on invalid args or DTLS handshake failure.
 */
static SYN_Status lwm2m_task_step_ex(SYN_LwM2M_Task *task, uint32_t now_ms, bool *out_rx_consumed)
{
    if (out_rx_consumed != NULL) {
        *out_rx_consumed = false;
    }
    if (task == NULL) {
        return SYN_INVALID_PARAM;
    }

    SYN_LwM2M_Client *client = task->cfg.client;
    if (client == NULL || task->cfg.transport == NULL) {
        return SYN_INVALID_PARAM;
    }

#if SYN_DTLS_ENABLED
    if (task->cfg.coaps_client != NULL &&
        task->cfg.coaps_client->dtls.state != SYN_DTLS_STATE_ESTABLISHED) {
        if (!syn_coaps_client_handshake(task->cfg.coaps_client)) {
            return SYN_ERROR;
        }
        if (task->cfg.coaps_client->dtls.state != SYN_DTLS_STATE_ESTABLISHED) {
            return SYN_OK;
        }
    }
#else
    if (task->cfg.coaps_client != NULL) {
        return SYN_ERROR;
    }
#endif

    if (task->force_deregister) {
        lwm2m_task_handle_deregister(task);
        return SYN_OK;
    }

    lwm2m_task_step_registration(task, now_ms);

    bool rx_consumed = lwm2m_task_process_inbound(task, now_ms);
    if (out_rx_consumed != NULL) {
        *out_rx_consumed = rx_consumed;
    }

    if (client->state == SYN_LWM2M_STATE_REGISTERED) {
        syn_lwm2m_task_eval_observations(task, now_ms);
    }

    return SYN_OK;
}

SYN_Status syn_lwm2m_task_step(SYN_LwM2M_Task *task, uint32_t now_ms)
{
    return lwm2m_task_step_ex(task, now_ms, NULL);
}

#if SYN_DTLS_ENABLED
/**
 * @brief Check whether the LwM2M supervisor task must run a cooperative DTLS 1.3 handshake first.
 * @param task LwM2M supervisor task instance.
 * @return true if transport/client are valid and a bound CoAPs session is not yet established.
 */
static bool lwm2m_needs_dtls_handshake(const SYN_LwM2M_Task *task)
{
    return (task->cfg.client != NULL) && (task->cfg.transport != NULL) &&
           (task->cfg.coaps_client != NULL) &&
           (task->cfg.coaps_client->dtls.state != SYN_DTLS_STATE_ESTABLISHED);
}
#endif

SYN_PT_Status syn_lwm2m_task_pt(SYN_PT *pt, SYN_Task *task)
{
    if (pt == NULL || task == NULL || task->user_data == NULL) {
        return PT_EXITED;
    }

    SYN_LwM2M_Task *lwm2m_task = (SYN_LwM2M_Task *)task->user_data;

    PT_BEGIN(pt);

    for (;;) {
#if SYN_DTLS_ENABLED
        if (lwm2m_needs_dtls_handshake(lwm2m_task)) {
            PT_SPAWN_IF_NEEDED(pt, task, &lwm2m_task->cfg.coaps_client->dtls.hs_pt,
                               syn_dtls_handshake_pt(&lwm2m_task->cfg.coaps_client->dtls.hs_pt,
                                                     &lwm2m_task->cfg.coaps_client->dtls));
            if (lwm2m_task->cfg.coaps_client->dtls.state != SYN_DTLS_STATE_ESTABLISHED) {
                break;
            }
            PT_CONTINUE_IF_NEEDED(pt, task);
        }
#endif
        bool rx_consumed = false;
        uint32_t now = syn_port_get_tick_ms();
        if (lwm2m_task_step_ex(lwm2m_task, now, &rx_consumed) != SYN_OK) {
            break;
        }
        SYN_Transport *active_tr = lwm2m_active_transport(lwm2m_task);
        if (rx_consumed && active_tr != NULL && active_tr->has_data != NULL &&
            syn_transport_has_data(active_tr)) {
            PT_CONTINUE_IF_NEEDED(pt, task);
        } else {
            PT_PASS(pt);
        }
    }

    PT_END(pt);
}

#endif /* SYN_LWM2M_TASK_ENABLED */
