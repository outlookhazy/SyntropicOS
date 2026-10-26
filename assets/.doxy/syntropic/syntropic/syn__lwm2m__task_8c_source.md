

# File syn\_lwm2m\_task.c

[**File List**](files.md) **>** [**proto**](dir_0a7333506e1b11c22e7531106607b850.md) **>** [**syn\_lwm2m\_task.c**](syn__lwm2m__task_8c.md)

[Go to the documentation of this file](syn__lwm2m__task_8c.md)


```C++

#if __has_include("syn_config.h")
#include "syn_config.h"
#endif

#if !defined(SYN_USE_LWM2M_TASK) || SYN_USE_LWM2M_TASK

#include "../port/syn_port_system.h"
#include "../util/syn_assert.h"
#include "syn_lwm2m_task.h"

#include <stdio.h>
#include <string.h>

#define SYN_LWM2M_DEFAULT_RETRY_BACKOFF_MS 5000U 
#define SYN_LWM2M_COAP_SERIALIZE_BUF_SZ 512U     
#define SYN_LWM2M_RESP_PAYLOAD_BUF_SZ 256U       
/* ── Internal State Helper ───────────────────────────────────────────────── */

static void syn_lwm2m_task_set_state(SYN_LwM2M_Task *task, SYN_LwM2M_ClientState new_state)
{
    SYN_LwM2M_ClientState old_state = task->cfg.client->state;
    if (old_state != new_state) {
        task->cfg.client->state = new_state;
        if (task->cfg.on_state != NULL) {
            task->cfg.on_state(old_state, new_state, task->cfg.user_data);
        }
    }
}

/* ── Object Command Hook Dispatcher ──────────────────────────────────────── */

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

static void syn_lwm2m_task_eval_observations(SYN_LwM2M_Task *task, uint32_t now_ms)
{
    SYN_LwM2M_Client *client = task->cfg.client;

    for (size_t i = 0; i < SYN_LWM2M_MAX_OBSERVERS; i++) {
        SYN_LwM2M_Observation *obs = &client->observations[i];
        if (!obs->active) {
            continue;
        }

        bool match_change = false;
        if (task->resource_changed) {
            if (task->changed_obj_id == obs->obj_id && task->changed_inst_id == obs->inst_id) {
                if (obs->res_id == -1 || obs->res_id == task->changed_res_id) {
                    match_change = true;
                }
            }
        }

        bool pmax_expired = false;
        if (obs->pmax > 0U && (now_ms - obs->last_notify_ms >= obs->pmax * 1000U)) {
            pmax_expired = true;
        }

        bool pmin_satisfied =
            (obs->pmin == 0U) || (now_ms - obs->last_notify_ms >= obs->pmin * 1000U);

        if ((match_change || pmax_expired) && pmin_satisfied) {
            SYN_CoapMsg notify_msg;
            SYN_CoapOption notify_opts[8];
            size_t notify_opt_cnt = 0U;
            uint8_t notify_pld[SYN_LWM2M_RESP_PAYLOAD_BUF_SZ];
            uint16_t mid = task->next_msg_id++;

            size_t pld_len = syn_lwm2m_build_notify(
                client, obs->obj_id, obs->inst_id, obs->res_id, mid, &notify_msg, notify_opts, 8U,
                &notify_opt_cnt, notify_pld, sizeof(notify_pld));

            if (pld_len > 0U || notify_msg.code == COAP_RESP_CONTENT) {
                uint8_t tx_frame[SYN_LWM2M_COAP_SERIALIZE_BUF_SZ];
                size_t frame_len = syn_coap_serialize(&notify_msg, notify_opts, notify_opt_cnt,
                                                      tx_frame, sizeof(tx_frame));
                if (frame_len > 0U) {
                    (void)syn_transport_send(task->cfg.transport, tx_frame, frame_len);
                    obs->last_notify_ms = now_ms;
                    obs->seq++;
                }
            }
        }
    }

    task->resource_changed = false;
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

    task->cfg.client->transport = cfg->transport;
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
    task->resource_changed = true;
    task->changed_obj_id = obj_id;
    task->changed_inst_id = inst_id;
    task->changed_res_id = res_id;
    return SYN_OK;
}

SYN_Status syn_lwm2m_task_step(SYN_LwM2M_Task *task, uint32_t now_ms)
{
    if (task == NULL) {
        return SYN_INVALID_PARAM;
    }

    SYN_LwM2M_Client *client = task->cfg.client;
    if (client == NULL || task->cfg.transport == NULL) {
        return SYN_INVALID_PARAM;
    }

    /* 1. If DTLS 1.3 CoAPS transport is configured, drive handshake */
    if (task->cfg.coaps_client != NULL) {
        if (!syn_coaps_client_handshake(task->cfg.coaps_client)) {
            /* Still in handshake phase or handshake failed */
            return SYN_OK;
        }
    }

    /* 2. Handle Graceful Deregistration Trigger */
    if (task->force_deregister) {
        task->force_deregister = false;
        if (client->state == SYN_LWM2M_STATE_REGISTERED ||
            client->state == SYN_LWM2M_STATE_UPDATING) {
            SYN_CoapMsg dereg_msg;
            SYN_CoapOption dereg_opts[8];
            size_t dereg_opt_cnt = 0U;
            uint16_t mid = task->next_msg_id++;

            size_t res =
                syn_lwm2m_build_deregister_request(client, mid, (const uint8_t *)&mid, sizeof(mid),
                                                   &dereg_msg, dereg_opts, 8U, &dereg_opt_cnt);
            if (res > 0U) {
                uint8_t tx_frame[SYN_LWM2M_COAP_SERIALIZE_BUF_SZ];
                size_t frame_len = syn_coap_serialize(&dereg_msg, dereg_opts, dereg_opt_cnt,
                                                      tx_frame, sizeof(tx_frame));
                if (frame_len > 0U) {
                    (void)syn_transport_send(task->cfg.transport, tx_frame, frame_len);
                }
            }
        }
        syn_lwm2m_task_set_state(task, SYN_LWM2M_STATE_DEREGISTERED);
        client->location_path[0] = '\0';
        return SYN_OK;
    }

    /* 3. Handle Autonomous Registration / Lifetime Renewal */
    switch (client->state) {
    case SYN_LWM2M_STATE_DEREGISTERED: {
        /* Check retry backoff */
        if (now_ms - task->last_action_ms >= task->cfg.retry_backoff_ms ||
            task->last_action_ms == 0U) {
            SYN_CoapMsg reg_msg;
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
                    (void)syn_transport_send(task->cfg.transport, task->cfg.tx_buf, frame_len);
                    syn_lwm2m_task_set_state(task, SYN_LWM2M_STATE_REGISTERING);
                    task->last_action_ms = now_ms;
                }
            }
        }
        break;
    }

    case SYN_LWM2M_STATE_REGISTERING:
        /* Waiting for 2.01 Created response from server (handled in RX step) */
        break;

    case SYN_LWM2M_STATE_REGISTERED: {
        /* Check if lifetime renewal is due (renew at 80% of lifetime) or forced */
        uint32_t renew_interval_ms = (client->lifetime_s * 800U); /* 80% of lifetime in ms */
        if (renew_interval_ms == 0U) {
            renew_interval_ms = 60000U;
        }

        bool need_update = task->force_update;
        if (now_ms - client->last_update_ms >= renew_interval_ms) {
            need_update = true;
        }

        if (need_update) {
            task->force_update = false;
            SYN_CoapMsg upd_msg;
            SYN_CoapOption upd_opts[8];
            size_t upd_opt_cnt = 0U;
            uint8_t upd_pld[SYN_LWM2M_RESP_PAYLOAD_BUF_SZ];
            uint16_t mid = task->next_msg_id++;

            size_t pld_len = syn_lwm2m_build_update_request(
                client, mid, (const uint8_t *)&mid, sizeof(mid), client->lifetime_s, &upd_msg,
                upd_opts, 8U, &upd_opt_cnt, upd_pld, sizeof(upd_pld));
            if (pld_len > 0U) {
                upd_msg.payload = upd_pld;
                upd_msg.payload_len = pld_len;
            }

            size_t frame_len = syn_coap_serialize(&upd_msg, upd_opts, upd_opt_cnt, task->cfg.tx_buf,
                                                  task->cfg.tx_buf_size);
            if (frame_len > 0U) {
                (void)syn_transport_send(task->cfg.transport, task->cfg.tx_buf, frame_len);
                syn_lwm2m_task_set_state(task, SYN_LWM2M_STATE_UPDATING);
                task->last_action_ms = now_ms;
            }
        }
        break;
    }

    case SYN_LWM2M_STATE_UPDATING:
        /* Waiting for 2.04 Changed response from server (handled in RX step) */
        break;

    default:
        break;
    }

    /* 4. Process Inbound CoAP Packets from Server */
    size_t rx_len = 0U;
    if (syn_transport_recv(task->cfg.transport, task->cfg.rx_buf, task->cfg.rx_buf_size, &rx_len) &&
        rx_len > 0U) {
        SYN_CoapMsg in_msg;
        SYN_CoapOption in_opts[8];
        size_t in_opt_cnt = 0U;

        SYN_Status parse_st =
            syn_coap_parse(&in_msg, in_opts, 8U, &in_opt_cnt, task->cfg.rx_buf, rx_len);
        if (parse_st == SYN_OK) {
            /* Check if Response message (Class 2, 4, 5) */
            if (in_msg.code >= COAP_RESP_CREATED) {
                if (client->state == SYN_LWM2M_STATE_REGISTERING) {
                    SYN_LwM2M_ClientState prev_st = client->state;
                    if (syn_lwm2m_handle_register_response(client, &in_msg, in_opts, in_opt_cnt)) {
                        client->last_update_ms = now_ms;
                        task->retry_count = 0U;
                        if (task->cfg.on_state != NULL && prev_st != SYN_LWM2M_STATE_REGISTERED) {
                            task->cfg.on_state(prev_st, SYN_LWM2M_STATE_REGISTERED,
                                               task->cfg.user_data);
                        }
                    } else {
                        syn_lwm2m_task_set_state(task, SYN_LWM2M_STATE_DEREGISTERED);
                        task->retry_count++;
                    }
                } else if (client->state == SYN_LWM2M_STATE_UPDATING) {
                    if (in_msg.code == COAP_RESP_CHANGED) {
                        syn_lwm2m_task_set_state(task, SYN_LWM2M_STATE_REGISTERED);
                        client->last_update_ms = now_ms;
                        task->retry_count = 0U;
                    } else {
                        syn_lwm2m_task_set_state(task, SYN_LWM2M_STATE_DEREGISTERED);
                        task->retry_count++;
                    }
                } else {
                    /* Unsolicited or late response */
                }
            }
            /* Downlink Request message (GET, POST, PUT, DELETE) */
            else if (in_msg.code >= COAP_CODE_GET && in_msg.code <= COAP_CODE_DELETE) {
                if (client->state == SYN_LWM2M_STATE_REGISTERED ||
                    client->state == SYN_LWM2M_STATE_UPDATING) {
                    SYN_CoapMsg resp_msg;
                    SYN_CoapOption resp_opts[8];
                    size_t resp_opt_cnt = 0U;
                    uint8_t resp_pld[SYN_LWM2M_RESP_PAYLOAD_BUF_SZ];

                    SYN_Status proc_st = syn_lwm2m_process_request(
                        client, &in_msg, in_opts, in_opt_cnt, &resp_msg, resp_opts, 8U,
                        &resp_opt_cnt, resp_pld, sizeof(resp_pld));

                    if (proc_st == SYN_OK) {
                        uint8_t tx_frame[SYN_LWM2M_COAP_SERIALIZE_BUF_SZ];
                        size_t frame_len = syn_coap_serialize(&resp_msg, resp_opts, resp_opt_cnt,
                                                              tx_frame, sizeof(tx_frame));
                        if (frame_len > 0U) {
                            (void)syn_transport_send(task->cfg.transport, tx_frame, frame_len);
                        }
                    }

                    /* Dispatch any triggered object execute callbacks */
                    syn_lwm2m_task_check_object_hooks(task);
                }
            } else {
                /* Empty or Ping message */
            }
        }
    }

    /* 5. Evaluate Active Observations & Notifications */
    if (client->state == SYN_LWM2M_STATE_REGISTERED) {
        syn_lwm2m_task_eval_observations(task, now_ms);
    }

    return SYN_OK;
}

SYN_PT_Status syn_lwm2m_task_pt(SYN_PT *pt, SYN_Task *task)
{
    if (pt == NULL || task == NULL || task->user_data == NULL) {
        return PT_ENDED;
    }

    SYN_LwM2M_Task *lwm2m_task = (SYN_LwM2M_Task *)task->user_data;

    PT_BEGIN(pt);

    for (;;) {
        uint32_t now = syn_port_get_tick_ms();
        if (syn_lwm2m_task_step(lwm2m_task, now) != SYN_OK) {
            break;
        }
        PT_YIELD(pt);
    }

    PT_END(pt);
}

#endif /* SYN_USE_LWM2M_TASK */
```


