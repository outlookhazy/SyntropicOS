/**
 * @file main.c
 * @brief SyntropicOS ESP32 OTA example.
 *
 * Demonstrates:
 *   - SyntropicOS CLI over serial for WiFi configuration
 *   - WiFi connection using credentials set via CLI
 *   - HTTP GET request to verify networking
 *   - Foundation for OTA firmware updates
 *
 * CLI commands:
 *   wifi ssid <name>    — Set WiFi SSID
 *   wifi pass <pass>    — Set WiFi password
 *   wifi connect        — Connect to WiFi
 *   wifi status         — Show connection status
 *   http get <host> [path] — Fetch a URL
 *   version             — Show firmware version
 *   reboot              — Reboot the device
 */

#include "syntropic/syntropic.h"
#include "syntropic/display/syn_canvas.h"
#include "syntropic/ui/syn_imgui.h"
#include "syntropic/cli/syn_cli.h"
#include "syntropic/net/syn_http.h"
#include "syntropic/net/syn_httpd.h"
#include "syntropic/net/syn_dns.h"
#include "syntropic/net/syn_websocket.h"
#include "syntropic/net/syn_mqtt.h"
#include "syntropic/util/syn_json_write.h"
#include "syntropic/system/syn_version.h"
#include "syntropic/storage/syn_param.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static const char *TAG = "syntropic";

/* ── Scheduler & Task pool ───────────────────────────────────────────────── */
enum {
    TASK_CLI,
    TASK_HTTPD,
    TASK_MDNS,
    TASK_MQTT,
    TASK_HTTP_CLIENT,
    TASK_DNS,
    TASK_COUNT
};

static SYN_Task s_tasks[TASK_COUNT];
static SYN_Sched s_sched;

/* ── WiFi state ─────────────────────────────────────────────────────────── */

/* syn_param partition lives at 0x310000 (see partitions.csv), 2 sectors */
#define PARAM_FLASH_BASE   0x310000u
#define PARAM_SECTOR_COUNT 2

typedef struct {
    char ssid[33];
    char pass[65];
} WifiParams;

static SYN_ParamStore param_store;
static WifiParams     wifi_params;

static char *wifi_ssid = wifi_params.ssid;
static char *wifi_pass = wifi_params.pass;
static bool wifi_connected = false;
static EventGroupHandle_t s_wifi_events;
#define WIFI_CONNECTED_BIT  BIT0

static void wifi_creds_save(void)
{
    syn_param_save(&param_store, &wifi_params);
}

static bool wifi_creds_load(void)
{
    if (syn_param_load(&param_store, &wifi_params) != SYN_OK) {
        memset(&wifi_params, 0, sizeof(wifi_params));
        return false;
    }
    return wifi_params.ssid[0] != '\0';
}

static void wifi_event_handler(void *arg, esp_event_base_t base,
                                int32_t id, void *data)
{
    (void)arg;
    if (base == WIFI_EVENT && id == WIFI_EVENT_STA_DISCONNECTED) {
        wifi_connected = false;
        xEventGroupClearBits(s_wifi_events, WIFI_CONNECTED_BIT);
        ESP_LOGI(TAG, "WiFi disconnected, reconnecting...");
        esp_wifi_connect();
    } else if (base == IP_EVENT && id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)data;
        ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
        wifi_connected = true;
        xEventGroupSetBits(s_wifi_events, WIFI_CONNECTED_BIT);
    }
}

static void wifi_do_connect(void)
{
    if (wifi_ssid[0] == '\0') {
        printf("Error: SSID not set. Use: wifi ssid <name>\r\n");
        return;
    }

    ESP_LOGI(TAG, "Connecting to '%s'...", wifi_ssid);

    wifi_config_t cfg = {0};
    strncpy((char *)cfg.sta.ssid, wifi_ssid, sizeof(cfg.sta.ssid) - 1);
    strncpy((char *)cfg.sta.password, wifi_pass, sizeof(cfg.sta.password) - 1);

    esp_wifi_set_config(WIFI_IF_STA, &cfg);
    esp_wifi_connect();

    /* Wait up to 10 seconds */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_events, WIFI_CONNECTED_BIT,
                                            pdFALSE, pdTRUE,
                                            pdMS_TO_TICKS(10000));
    if (bits & WIFI_CONNECTED_BIT) {
        printf("WiFi connected!\r\n");
    } else {
        printf("WiFi connection timeout.\r\n");
    }
}

static void wifi_init_sta(void)
{
    s_wifi_events = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                               wifi_event_handler, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                               wifi_event_handler, NULL);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
}

/* ── CLI commands ───────────────────────────────────────────────────────── */

static int cmd_wifi(int argc, char *argv[])
{
    if (argc < 2) {
        printf("Usage:\r\n");
        printf("  wifi ssid <name>   Set SSID\r\n");
        printf("  wifi pass <pass>   Set password\r\n");
        printf("  wifi connect       Connect\r\n");
        printf("  wifi status        Show status\r\n");
        return 0;
    }

    if (strcmp(argv[1], "ssid") == 0) {
        if (argc >= 3) {
            strncpy(wifi_ssid, argv[2], sizeof(wifi_params.ssid) - 1);
            wifi_params.ssid[sizeof(wifi_params.ssid) - 1] = '\0';
            wifi_creds_save();
            printf("SSID set to: %s (saved)\r\n", wifi_ssid);
        } else {
            printf("SSID: %s\r\n", wifi_ssid[0] ? wifi_ssid : "(not set)");
        }

    } else if (strcmp(argv[1], "pass") == 0) {
        if (argc >= 3) {
            strncpy(wifi_pass, argv[2], sizeof(wifi_params.pass) - 1);
            wifi_params.pass[sizeof(wifi_params.pass) - 1] = '\0';
            wifi_creds_save();
            printf("Password set (%d chars, saved)\r\n", (int)strlen(wifi_pass));
        } else {
            printf("Password: %s\r\n", wifi_pass[0] ? "(set)" : "(not set)");
        }

    } else if (strcmp(argv[1], "connect") == 0) {
        wifi_do_connect();

    } else if (strcmp(argv[1], "status") == 0) {
        printf("SSID: %s\r\n", wifi_ssid[0] ? wifi_ssid : "(not set)");
        printf("Connected: %s\r\n", wifi_connected ? "yes" : "no");
        if (wifi_connected) {
            esp_netif_ip_info_t ip_info;
            esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
            if (netif && esp_netif_get_ip_info(netif, &ip_info) == ESP_OK) {
                printf("IP: " IPSTR "\r\n", IP2STR(&ip_info.ip));
            }
        }

    } else {
        printf("Unknown wifi subcommand: %s\r\n", argv[1]);
    }

    return 0;
}

/* ── HTTP GET command ───────────────────────────────────────────────────── */

static size_t s_body_total;

static bool http_body_print(const uint8_t *data, size_t len, void *ctx)
{
    (void)ctx;
    /* Print body data to console (truncate at 512 bytes for readability) */
    if (s_body_total + len > 512) {
        size_t remaining = 512 - s_body_total;
        if (remaining > 0) {
            printf("%.*s", (int)remaining, (const char *)data);
        }
        if (s_body_total < 512) {
            printf("\r\n... (truncated)\r\n");
        }
        s_body_total += len;
        return true;
    }
    printf("%.*s", (int)len, (const char *)data);
    s_body_total += len;
    return true;
}

static SYN_CLI cli;

static SYN_PT_Status cli_sched_task(SYN_PT *pt, SYN_Task *task)
{
    (void)task;
    PT_BEGIN(pt);
    for (;;) {
        int c = getchar();
        if (c != EOF) {
            syn_cli_process_char(&cli, (char)c);
        }
        PT_YIELD(pt);
    }
    PT_END(pt);
}

static int cmd_http(int argc, char *argv[])
{
    if (!wifi_connected) {
        printf("Error: WiFi not connected.\r\n");
        return -1;
    }

    if (argc < 3 || strcmp(argv[1], "get") != 0) {
        printf("Usage: http get <host> [path]\r\n");
        printf("  e.g.: http get example.com /\r\n");
        return -1;
    }

    const char *host = argv[2];
    const char *path = (argc >= 4) ? argv[3] : "/";
    uint16_t port = (argc >= 5) ? (uint16_t)atoi(argv[4]) : 80;

    printf("GET http://%s:%u%s\r\n", host, port, path);
    printf("Resolving %s...\r\n", host);
    fflush(stdout);

    static uint8_t work_buf[1024];
    s_body_total = 0;

    static SYN_HttpClient http_client_inst;
    syn_http_client_init(&http_client_inst, "GET", host, port, path,
                          NULL, NULL, 0, NULL, 0,
                          http_body_print, NULL,
                          work_buf, sizeof(work_buf));

    s_tasks[TASK_HTTP_CLIENT].user_data = &http_client_inst;

    syn_task_suspend(&s_tasks[TASK_CLI]);
    syn_task_restart(&s_tasks[TASK_HTTP_CLIENT]);

    while (syn_task_is_alive(&s_tasks[TASK_HTTP_CLIENT])) {
        syn_sched_run(&s_sched);
        taskYIELD();
    }

    syn_task_resume(&s_tasks[TASK_CLI]);

    SYN_HttpResponse resp = http_client_inst.resp;
    SYN_Status st = http_client_inst.status;

    printf("\r\n---\r\n");
    if (st == SYN_OK) {
        printf("HTTP %d, %lu bytes received\r\n",
               resp.status_code, (unsigned long)s_body_total);
    } else {
        printf("HTTP request failed (connect or parse error)\r\n");
    }

    return (st == SYN_OK) ? 0 : -1;
}

/* ── Version command ───────────────────────────────────────────────────── */

static int cmd_version(int argc, char *argv[])
{
    (void)argc; (void)argv;
    const SYN_Version *v = syn_version();
    printf("%s v%u.%u.%u (%s %s)\r\n",
           v->app_name, v->major, v->minor, v->patch,
           v->date, v->time);
    return 0;
}

/* ── OTA firmware update ───────────────────────────────────────────────── */

#include "esp_ota_ops.h"
#include "esp_app_format.h"

static esp_ota_handle_t s_ota_handle;
static size_t s_ota_written;

static bool ota_body_cb(const uint8_t *data, size_t len, void *ctx)
{
    (void)ctx;
    esp_err_t err = esp_ota_write(s_ota_handle, data, len);
    if (err != ESP_OK) {
        printf("\r\nOTA write failed: %s\r\n", esp_err_to_name(err));
        return false;
    }
    s_ota_written += len;
    /* Progress every 64KB */
    if ((s_ota_written % (64 * 1024)) < len) {
        printf("  %lu KB...\r\n", (unsigned long)(s_ota_written / 1024));
    }
    return true;
}

static int cmd_ota(int argc, char *argv[])
{
    if (!wifi_connected) {
        printf("Error: WiFi not connected.\r\n");
        return -1;
    }

    if (argc < 3) {
        printf("Usage: ota <host> <path>\r\n");
        printf("  e.g.: ota 192.168.76.100 /firmware.bin\r\n");
        return -1;
    }

    const char *host = argv[1];
    const char *path = argv[2];
    uint16_t port = (argc >= 4) ? (uint16_t)atoi(argv[3]) : 80;

    /* Find the next OTA partition */
    const esp_partition_t *update_part = esp_ota_get_next_update_partition(NULL);
    if (update_part == NULL) {
        printf("Error: No OTA partition available.\r\n");
        return -1;
    }

    const esp_partition_t *running = esp_ota_get_running_partition();
    printf("Running from: %s\r\n", running->label);
    printf("Update target: %s (0x%lx, %lu KB)\r\n",
           update_part->label,
           (unsigned long)update_part->address,
           (unsigned long)update_part->size / 1024);
    printf("Downloading http://%s:%u%s...\r\n", host, port, path);
    fflush(stdout);

    /* Begin OTA */
    esp_err_t err = esp_ota_begin(update_part, OTA_WITH_SEQUENTIAL_WRITES,
                                   &s_ota_handle);
    if (err != ESP_OK) {
        printf("esp_ota_begin failed: %s\r\n", esp_err_to_name(err));
        return -1;
    }

    s_ota_written = 0;

    static uint8_t work_buf[1024];

    static SYN_HttpClient ota_client_inst;
    syn_http_client_init(&ota_client_inst, "GET", host, port, path,
                          NULL, NULL, 0, NULL, 0,
                          ota_body_cb, NULL,
                          work_buf, sizeof(work_buf));

    s_tasks[TASK_HTTP_CLIENT].user_data = &ota_client_inst;

    syn_task_suspend(&s_tasks[TASK_CLI]);
    syn_task_restart(&s_tasks[TASK_HTTP_CLIENT]);

    while (syn_task_is_alive(&s_tasks[TASK_HTTP_CLIENT])) {
        syn_sched_run(&s_sched);
        taskYIELD();
    }

    syn_task_resume(&s_tasks[TASK_CLI]);

    SYN_HttpResponse resp = ota_client_inst.resp;
    SYN_Status st = ota_client_inst.status;

    if (st != SYN_OK) {
        printf("Download failed (syn_http error, %lu bytes received)\r\n",
               (unsigned long)s_ota_written);
        esp_ota_abort(s_ota_handle);
        return -1;
    }
    if (resp.status_code != 200) {
        printf("Server returned HTTP %d\r\n", resp.status_code);
        esp_ota_abort(s_ota_handle);
        return -1;
    }

    printf("Downloaded %lu bytes.\r\n", (unsigned long)s_ota_written);

    /* End OTA */
    err = esp_ota_end(s_ota_handle);
    if (err != ESP_OK) {
        printf("esp_ota_end failed: %s\r\n", esp_err_to_name(err));
        return -1;
    }

    /* Set boot partition */
    err = esp_ota_set_boot_partition(update_part);
    if (err != ESP_OK) {
        printf("esp_ota_set_boot_partition failed: %s\r\n",
               esp_err_to_name(err));
        return -1;
    }

    printf("OTA success! Rebooting into %s...\r\n", update_part->label);
    fflush(stdout);
    vTaskDelay(pdMS_TO_TICKS(500));
    esp_restart();
    return 0;  /* unreachable */
}

static int cmd_reboot(int argc, char *argv[])
{
    (void)argc; (void)argv;
    printf("Rebooting...\r\n");
    vTaskDelay(pdMS_TO_TICKS(100));
    esp_restart();
    return 0; /* unreachable */
}

/* ── HTTP Server ────────────────────────────────────────────────────────── */

static SYN_Httpd httpd;
static uint8_t httpd_buf[1024];
static bool httpd_running = false;

static SYN_WebsocketSession ws_session;

static void handle_ws_message(const uint8_t *payload, size_t len, uint8_t opcode, void *ctx)
{
    (void)ctx;
    printf("\r\n[WS] Received msg (op=0x%02X, len=%d): %.*s\r\n", 
           opcode, (int)len, (int)len, (const char *)payload);
    printf("syntropic> ");
    fflush(stdout);

    /* Echo text or binary frames back to the client */
    if (opcode == 0x01 || opcode == 0x02) {
        syn_websocket_send(&ws_session, opcode, payload, len);
    }
}

static void handle_ws_route(const SYN_HttpdRequest *req,
                            SYN_HttpdResponse *resp, void *ctx)
{
    (void)ctx;
    printf("WS upgrade requested\r\n");
    SYN_Status st = syn_websocket_upgrade(req, resp, &ws_session, handle_ws_message, NULL);
    if (st == SYN_OK) {
        printf("WS upgrade successful!\r\n");
    } else {
        printf("WS upgrade failed!\r\n");
    }
}

/* ── GUI state for Live Testing ─────────────────────────────────────────── */
static SYN_IMGUI_Context s_gui_ctx;
static SYN_Canvas        s_canvas;
static uint8_t           s_framebuf[128 * 64 / 8];
static bool              s_gui_initialized = false;

/* Widget states */
static int32_t s_page = 0;
static int16_t s_scroll_offset = 0;
static int16_t s_marquee_offset = 0;
static int32_t s_slider_val = 50;
static int32_t s_spinner_val = 10;
static bool    s_collapse1_open = false;
static bool    s_collapse2_open = false;
static int32_t s_progress_val = 0;
static uint32_t s_checkbox_flags = 0x05;

static void handle_fb(const SYN_HttpdRequest *req,
                      SYN_HttpdResponse *resp, void *ctx)
{
    (void)ctx;

    // Initialize GUI context on first call
    if (!s_gui_initialized) {
        syn_canvas_init(&s_canvas, s_framebuf, 128, 64, 1, NULL, NULL);
        syn_imgui_init(&s_gui_ctx);
        s_gui_initialized = true;
    }

    // Parse input query parameters
    bool select = false;
    bool back = false;
    int32_t enc_delta = 0;
    bool touch_down = false;
    int16_t touch_x = 0;
    int16_t touch_y = 0;

    if (req->query) {
        if (strstr(req->query, "select=1")) {
            select = true;
        }
        if (strstr(req->query, "back=1")) {
            back = true;
        }
        const char *p_enc = strstr(req->query, "enc=");
        if (p_enc) {
            enc_delta = atoi(p_enc + 4);
        }
        const char *p_touch = strstr(req->query, "touch=");
        if (p_touch) {
            touch_down = true;
            sscanf(p_touch + 6, "%hd,%hd", &touch_x, &touch_y);
        }
        const char *p_page = strstr(req->query, "page=");
        if (p_page) {
            s_page = atoi(p_page + 5);
        }
    }

    // Process background state updates (e.g. progress bar auto-advance)
    s_progress_val = (s_progress_val + 5) % 105;

    // Clear canvas
    syn_canvas_clear(&s_canvas);

    // Begin IMGUI frame
    syn_imgui_begin(&s_gui_ctx, &s_canvas, select, back, enc_delta, touch_down, touch_x, touch_y);

    // Render based on s_page
    if (s_page == 0) {
        // Page 0: Widgets Overview — layout starts at y=0 so header+4 items fit in 64px
        syn_imgui_layout_begin(&s_gui_ctx, 0, 0, 128);
        syn_imgui_separator_text(&s_gui_ctx, "GUI Overview", 0, 0, 0);

        // Checkbox flags (e.g. status LEDs)
        syn_imgui_checkbox_flags(&s_gui_ctx, "LED 1", &s_checkbox_flags, 0x01, 0, 0, 0, 0);
        syn_imgui_checkbox_flags(&s_gui_ctx, "LED 2", &s_checkbox_flags, 0x02, 0, 0, 0, 0);

        // Slider
        syn_imgui_slider(&s_gui_ctx, "Brightness", &s_slider_val, 0, 100, 0, 0, 0, 0);

        // Value Int display
        syn_imgui_value_int(&s_gui_ctx, "Speed", s_spinner_val, 0, 0);

        syn_imgui_layout_end(&s_gui_ctx);
    } else if (s_page == 1) {
        // Page 1: Advanced widgets — layout starts at y=0
        syn_imgui_layout_begin(&s_gui_ctx, 0, 0, 128);

        // Marquee text (scrolling title)
        syn_imgui_text_marquee(&s_gui_ctx, "SyntropicOS Embedded UI Engine", &s_marquee_offset, 0, 0, 128, 1);

        // Collapsing headers
        if (syn_imgui_collapsing_header(&s_gui_ctx, "System Config", &s_collapse1_open, 0, 0, 0, 0)) {
            syn_imgui_label(&s_gui_ctx, "Mode: Active", 0, 0);
            syn_imgui_label(&s_gui_ctx, "Volt: 3.3V", 0, 0);
        }

        if (syn_imgui_collapsing_header(&s_gui_ctx, "Network Status", &s_collapse2_open, 0, 0, 0, 0)) {
            syn_imgui_label(&s_gui_ctx, "IP: 192.168.76.114", 0, 0);
            syn_imgui_label(&s_gui_ctx, "SSID: Olimex", 0, 0);
        }

        // Progress bar (only visible when headers collapsed, otherwise off-screen)
        syn_imgui_progress_bar_ex(&s_gui_ctx, s_progress_val, 0, 100, "Loading...", 0, 0, 0, 0);

        syn_imgui_layout_end(&s_gui_ctx);
    } else {
        // Page 2: Scroll region and selectables — full-width, full-height
        syn_imgui_scroll_begin(&s_gui_ctx, 0, 0, 128, 64, &s_scroll_offset);

        static bool sel_states[8] = {false};
        char label_buf[16];
        for (int i = 0; i < 8; i++) {
            snprintf(label_buf, sizeof(label_buf), "Item %d", i + 1);
            syn_imgui_selectable(&s_gui_ctx, label_buf, &sel_states[i], 0, 0, 0, 0);
        }

        syn_imgui_scroll_end(&s_gui_ctx);
    }

    // End IMGUI frame
    syn_imgui_end(&s_gui_ctx);

    // Send HTTP response
    syn_httpd_status(resp, 200, "OK");
    syn_httpd_header(resp, "Content-Type", "application/octet-stream");
    char len_str[32];
    sprintf(len_str, "%u", (unsigned int)sizeof(s_framebuf));
    syn_httpd_header(resp, "Content-Length", len_str);
    syn_httpd_body(resp, s_framebuf, sizeof(s_framebuf));
}

static void handle_root(const SYN_HttpdRequest *req,
                         SYN_HttpdResponse *resp, void *ctx)
{
    (void)req; (void)ctx;
    static const char html[] =
        "<!DOCTYPE html><html><head>"
        "<title>SyntropicOS</title>"
        "<style>body{font-family:monospace;background:#1a1a2e;color:#e0e0e0;"
        "max-width:600px;margin:40px auto;padding:20px}"
        "h1{color:#0ff}a{color:#0af}</style>"
        "</head><body>"
        "<h1>SyntropicOS</h1>"
        "<p>Device is running.</p>"
        "<p><a href=\"/api/status\">/api/status</a> |"
        " <a href=\"/api/version\">/api/version</a></p>"
        "</body></html>";

    syn_httpd_status(resp, 200, "OK");
    syn_httpd_header(resp, "Content-Type", "text/html");
    syn_httpd_body_str(resp, html);
}

static void handle_api_status(const SYN_HttpdRequest *req,
                               SYN_HttpdResponse *resp, void *ctx)
{
    (void)req; (void)ctx;
    static char json_buf[256];
    SYN_JsonWriter w;
    syn_json_init(&w, json_buf, sizeof(json_buf));
    syn_json_obj_open(&w);
    syn_json_key_str(&w, "device", "esp32-poe");
    syn_json_key_uint(&w, "uptime_ms", syn_port_get_tick_ms());
    syn_json_key_bool(&w, "wifi", wifi_connected);
    syn_json_key_str(&w, "ssid", wifi_ssid[0] ? wifi_ssid : "");
    syn_json_obj_close(&w);

    syn_httpd_status(resp, 200, "OK");
    syn_httpd_header(resp, "Content-Type", "application/json");
    syn_httpd_body_str(resp, syn_json_str(&w));
}

static void handle_api_version(const SYN_HttpdRequest *req,
                                SYN_HttpdResponse *resp, void *ctx)
{
    (void)req; (void)ctx;
    const SYN_Version *v = syn_version();
    static char json_buf[256];
    SYN_JsonWriter w;
    syn_json_init(&w, json_buf, sizeof(json_buf));
    syn_json_obj_open(&w);
    syn_json_key_str(&w, "app", v->app_name);
    syn_json_key_uint(&w, "major", v->major);
    syn_json_key_uint(&w, "minor", v->minor);
    syn_json_key_uint(&w, "patch", v->patch);
    syn_json_key_str(&w, "date", v->date);
    syn_json_key_str(&w, "time", v->time);
    syn_json_obj_close(&w);

    syn_httpd_status(resp, 200, "OK");
    syn_httpd_header(resp, "Content-Type", "application/json");
    syn_httpd_body_str(resp, syn_json_str(&w));
}

static const SYN_HttpdRoute httpd_routes[] = {
    { SYN_HTTP_GET, "/",            handle_root,        NULL },
    { SYN_HTTP_GET, "/api/status",  handle_api_status,  NULL },
    { SYN_HTTP_GET, "/api/version", handle_api_version, NULL },
    { SYN_HTTP_GET, "/ws",          handle_ws_route,    NULL },
    { SYN_HTTP_GET, "/fb",          handle_fb,          NULL },
};

static SYN_PT_Status httpd_sched_task(SYN_PT *pt, SYN_Task *task)
{
    (void)task;
    static SYN_PT ws_pt;
    static SYN_Task ws_task;

    PT_BEGIN(pt);

    PT_INIT(&ws_pt);
    ws_task.user_data = &ws_session;

    for (;;) {
        if (!httpd_running) {
            break;
        }
        syn_httpd_step(&httpd);
        if (ws_session.state == SYN_WS_STATE_CONNECTED) {
            syn_websocket_task(&ws_pt, &ws_task);
        } else {
            PT_INIT(&ws_pt);
        }
        PT_YIELD(pt);
    }

    syn_httpd_stop(&httpd);
    ESP_LOGI(TAG, "HTTP server stopped");
    PT_END(pt);
}

static int cmd_httpd(int argc, char *argv[])
{
    if (argc >= 2 && strcmp(argv[1], "stop") == 0) {
        if (httpd_running) {
            httpd_running = false;
            printf("HTTP server stopping...\r\n");
        } else {
            printf("HTTP server not running.\r\n");
        }
        return 0;
    }

    if (httpd_running) {
        printf("HTTP server already running.\r\n");
        return 0;
    }

    if (!wifi_connected) {
        printf("Error: WiFi not connected.\r\n");
        return -1;
    }

    ws_session.state = SYN_WS_STATE_CLOSED;

    SYN_Status st = syn_httpd_init(&httpd, 80,
                                    httpd_routes, SYN_ARRAY_SIZE(httpd_routes),
                                    httpd_buf, sizeof(httpd_buf));
    if (st != SYN_OK) {
        printf("HTTP server init FAILED — bind/listen error on port 80\r\n");
        return -1;
    }

    ESP_LOGI(TAG, "HTTP server listening on port 80 (fd=%d)", (int)httpd.listener);

    httpd_running = true;
    syn_task_restart(&s_tasks[TASK_HTTPD]);
    printf("HTTP server started on port 80.\r\n");
    return 0;
}

/* ── DNS command ────────────────────────────────────────────────────────── */

static int cmd_dns(int argc, char *argv[])
{
    if (argc < 3 || strcmp(argv[1], "resolve") != 0) {
        printf("Usage: dns resolve <hostname>\r\n");
        return -1;
    }
    if (!wifi_connected) {
        printf("Error: WiFi not connected.\r\n");
        return -1;
    }
    const char *host = argv[2];
    printf("Resolving '%s' via DNS (cooperatively)...\r\n", host);
    fflush(stdout);

    static SYN_DnsResolver dns_resolver;
    static SYN_SockAddr addr;
    dns_resolver.dns_server = NULL;
    dns_resolver.hostname = host;
    dns_resolver.addr_out = &addr;
    dns_resolver.timeout_ms = 5000;

    s_tasks[TASK_DNS].user_data = &dns_resolver;

    syn_task_suspend(&s_tasks[TASK_CLI]);
    syn_task_restart(&s_tasks[TASK_DNS]);

    while (syn_task_is_alive(&s_tasks[TASK_DNS])) {
        syn_sched_run(&s_sched);
        taskYIELD();
    }

    syn_task_resume(&s_tasks[TASK_CLI]);

    SYN_Status st = dns_resolver.status;
    if (st == SYN_OK) {
        printf("Resolved: %d.%d.%d.%d\r\n", addr.ip[0], addr.ip[1], addr.ip[2], addr.ip[3]);
    } else if (st == SYN_TIMEOUT) {
        printf("Resolution timed out\r\n");
    } else {
        printf("Resolution failed\r\n");
    }
    return (st == SYN_OK) ? 0 : -1;
}

/* ── mDNS command ───────────────────────────────────────────────────────── */

static SYN_Mdns mdns_inst;
static int cmd_mdns(int argc, char *argv[])
{
    if (argc < 2 || strcmp(argv[1], "start") != 0) {
        printf("Usage: mdns start [hostname]\r\n");
        return -1;
    }
    if (!wifi_connected) {
        printf("Error: WiFi not connected.\r\n");
        return -1;
    }
    if (s_tasks[TASK_MDNS].state != (uint8_t)SYN_TASK_SUSPENDED) {
        printf("mDNS responder already running.\r\n");
        return 0;
    }
    
    const char *hostname = (argc >= 3) ? argv[2] : "syntropic";
    
    esp_netif_ip_info_t ip_info;
    esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    if (!netif || esp_netif_get_ip_info(netif, &ip_info) != ESP_OK) {
        printf("Error: failed to get IP address.\r\n");
        return -1;
    }
    
    uint8_t ip[4];
    memcpy(ip, &ip_info.ip.addr, 4);
    
    SYN_Status st = syn_mdns_init(&mdns_inst, hostname, ip);
    if (st != SYN_OK) {
        printf("Error: mDNS init failed.\r\n");
        return -1;
    }
    
    syn_task_restart(&s_tasks[TASK_MDNS]);
    printf("mDNS responder started for '%s.local' (%d.%d.%d.%d)\r\n", 
           hostname, ip[0], ip[1], ip[2], ip[3]);
    return 0;
}

/* ── MQTT command ───────────────────────────────────────────────────────── */

static uint8_t mqtt_rx_buf[512];
static uint8_t mqtt_tx_buf[512];
static SYN_MqttClient mqtt_client;
static void mqtt_message_cb(const char *topic, const uint8_t *payload, size_t len, void *ctx)
{
    (void)ctx;
    printf("\r\n[MQTT] Message on %s (%d bytes): %.*s\r\n", 
           topic, (int)len, (int)len, (const char *)payload);
    printf("syntropic> ");
    fflush(stdout);
}

static int cmd_mqtt(int argc, char *argv[])
{
    if (argc < 2) {
        printf("Usage:\r\n");
        printf("  mqtt connect <host> [port] [username] [password]\r\n");
        printf("  mqtt publish <topic> <msg> [qos]\r\n");
        printf("  mqtt subscribe <topic> [qos]\r\n");
        printf("  mqtt status\r\n");
        printf("  mqtt disconnect\r\n");
        return 0;
    }

    if (strcmp(argv[1], "connect") == 0) {
        if (argc < 3) {
            printf("Usage: mqtt connect <host> [port] [username] [password]\r\n");
            return -1;
        }
        if (!wifi_connected) {
            printf("Error: WiFi not connected.\r\n");
            return -1;
        }
        if (s_tasks[TASK_MQTT].state != (uint8_t)SYN_TASK_SUSPENDED) {
            printf("MQTT client already connected/connecting.\r\n");
            return 0;
        }

        const char *host = argv[2];
        uint16_t port = (argc >= 4) ? (uint16_t)atoi(argv[3]) : 1883;
        const char *username = (argc >= 5) ? argv[4] : NULL;
        const char *password = (argc >= 6) ? argv[5] : NULL;

        syn_mqtt_init(&mqtt_client, host, port, "syntropic-device",
                      username, password, 60,
                      mqtt_rx_buf, sizeof(mqtt_rx_buf),
                      mqtt_tx_buf, sizeof(mqtt_tx_buf));
        mqtt_client.on_message = mqtt_message_cb;

        syn_task_restart(&s_tasks[TASK_MQTT]);
        printf("MQTT client started (connecting to %s:%d...)\r\n", host, port);

    } else if (strcmp(argv[1], "publish") == 0) {
        if (argc < 4) {
            printf("Usage: mqtt publish <topic> <msg> [qos]\r\n");
            return -1;
        }
        if (s_tasks[TASK_MQTT].state == (uint8_t)SYN_TASK_SUSPENDED || mqtt_client.state != SYN_MQTT_CONNECTED) {
            printf("Error: MQTT client not connected.\r\n");
            return -1;
        }
        const char *topic = argv[2];
        const char *msg = argv[3];
        uint8_t qos = (argc >= 5) ? (uint8_t)atoi(argv[4]) : 0;
        
        SYN_Status st = syn_mqtt_publish(&mqtt_client, topic, msg, strlen(msg), qos, false);
        if (st == SYN_OK) {
            printf("Published message to '%s' (QoS %d)\r\n", topic, qos);
        } else {
            printf("Error: publish failed.\r\n");
        }

    } else if (strcmp(argv[1], "subscribe") == 0) {
        if (argc < 3) {
            printf("Usage: mqtt subscribe <topic> [qos]\r\n");
            return -1;
        }
        if (s_tasks[TASK_MQTT].state == (uint8_t)SYN_TASK_SUSPENDED || mqtt_client.state != SYN_MQTT_CONNECTED) {
            printf("Error: MQTT client not connected.\r\n");
            return -1;
        }
        const char *topic = argv[2];
        uint8_t qos = (argc >= 4) ? (uint8_t)atoi(argv[3]) : 0;

        SYN_Status st = syn_mqtt_subscribe(&mqtt_client, topic, qos);
        if (st == SYN_OK) {
            printf("Subscribed to '%s' (QoS %d)\r\n", topic, qos);
        } else {
            printf("Error: subscribe failed.\r\n");
        }

    } else if (strcmp(argv[1], "status") == 0) {
        printf("MQTT state: ");
        if (s_tasks[TASK_MQTT].state == (uint8_t)SYN_TASK_SUSPENDED) {
            printf("stopped\r\n");
        } else if (mqtt_client.state == SYN_MQTT_DISCONNECTED) {
            printf("disconnected / connecting...\r\n");
        } else if (mqtt_client.state == SYN_MQTT_CONNECTING) {
            printf("connecting...\r\n");
        } else if (mqtt_client.state == SYN_MQTT_CONNECTED) {
            printf("connected\r\n");
        }
        
    } else if (strcmp(argv[1], "disconnect") == 0) {
        if (s_tasks[TASK_MQTT].state == (uint8_t)SYN_TASK_SUSPENDED) {
            printf("MQTT client not running.\r\n");
            return 0;
        }
        syn_task_suspend(&s_tasks[TASK_MQTT]);
        if (mqtt_client.state != SYN_MQTT_DISCONNECTED) {
            syn_port_sock_close(mqtt_client.sock);
            mqtt_client.sock = SYN_SOCKET_INVALID;
            mqtt_client.state = SYN_MQTT_DISCONNECTED;
        }
        printf("MQTT client stopped.\r\n");
    } else {
        printf("Unknown mqtt subcommand: %s\r\n", argv[1]);
    }
    return 0;
}

/* ── WebSocket command ──────────────────────────────────────────────────── */

static int cmd_websocket(int argc, char *argv[])
{
    if (argc < 2) {
        printf("Usage:\r\n");
        printf("  websocket send <message>   Send a text message to client\r\n");
        printf("  websocket status           Show status of WebSocket session\r\n");
        return 0;
    }
    
    if (strcmp(argv[1], "send") == 0) {
        if (argc < 3) {
            printf("Usage: websocket send <message>\r\n");
            return -1;
        }
        if (ws_session.state != SYN_WS_STATE_CONNECTED) {
            printf("Error: WebSocket client not connected.\r\n");
            return -1;
        }
        const char *msg = argv[2];
        SYN_Status st = syn_websocket_send(&ws_session, 0x01, msg, strlen(msg));
        if (st == SYN_OK) {
            printf("Sent message: %s\r\n", msg);
        } else {
            printf("Error: failed to send message.\r\n");
        }
    } else if (strcmp(argv[1], "status") == 0) {
        printf("WebSocket state: %s\r\n", 
               (ws_session.state == SYN_WS_STATE_CONNECTED) ? "connected" : "closed");
    } else {
        printf("Unknown websocket subcommand: %s\r\n", argv[1]);
    }
    return 0;
}

/* ── CLI setup ──────────────────────────────────────────────────────────── */

static void cli_putchar(char ch)
{
    putchar(ch);
    fflush(stdout);
}

static const SYN_CLI_Command cli_commands[] = {
    { "wifi",      "WiFi configuration",    cmd_wifi      },
    { "http",      "HTTP requests",         cmd_http      },
    { "httpd",     "HTTP server",           cmd_httpd     },
    { "ota",       "OTA firmware update",   cmd_ota       },
    { "dns",       "DNS resolver",          cmd_dns       },
    { "mdns",      "mDNS responder",        cmd_mdns      },
    { "mqtt",      "MQTT client",           cmd_mqtt      },
    { "websocket", "WebSocket control",     cmd_websocket },
    { "version",   "Show firmware version", cmd_version   },
    { "reboot",    "Reboot device",         cmd_reboot    },
};

/* ── Main ───────────────────────────────────────────────────────────────── */

void app_main(void)
{
    /* Lower task priority to 0 to share time-slice round-robin with idle task */
    vTaskPrioritySet(NULL, tskIDLE_PRIORITY);

    /* Initialize NVS — required by WiFi */
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
        ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }

    printf("\r\n");
    printf("====================================\r\n");
    printf("  SyntropicOS ESP32 OTA Example\r\n");
    printf("  v%d.%d.%d\r\n",
           SYN_VERSION_MAJOR, SYN_VERSION_MINOR, SYN_VERSION_PATCH);
    printf("====================================\r\n");
    printf("\r\n");

    /* Init WiFi subsystem (doesn't connect yet) */
    wifi_init_sta();

    /* Init parameter storage and load saved WiFi credentials */
    syn_param_init(&param_store, PARAM_FLASH_BASE,
                   PARAM_SECTOR_COUNT, sizeof(WifiParams));

    if (wifi_creds_load()) {
        printf("Loaded saved WiFi credentials (SSID: %s)\r\n", wifi_ssid);
        wifi_do_connect();
    }

    /* Init CLI */
    syn_cli_init(&cli, cli_commands, SYN_ARRAY_SIZE(cli_commands),
                 cli_putchar, "syntropic> ");

    printf("Type 'help' for commands.\r\n");
    syn_cli_print_prompt(&cli);

    /* Initialize all cooperative tasks */
    syn_task_create(&s_tasks[TASK_CLI], "cli", cli_sched_task, 3, NULL);
    syn_task_create(&s_tasks[TASK_HTTPD], "httpd", httpd_sched_task, 1, NULL);
    syn_task_create(&s_tasks[TASK_MDNS], "mdns", syn_mdns_task, 2, &mdns_inst);
    syn_task_create(&s_tasks[TASK_MQTT], "mqtt", syn_mqtt_task, 1, &mqtt_client);
    syn_task_create(&s_tasks[TASK_HTTP_CLIENT], "http_client", syn_http_client_task, 0, NULL);
    syn_task_create(&s_tasks[TASK_DNS], "dns_client", syn_dns_resolve_task, 1, NULL);

    /* Suspend the background/client ones initially */
    syn_task_suspend(&s_tasks[TASK_HTTPD]);
    syn_task_suspend(&s_tasks[TASK_MDNS]);
    syn_task_suspend(&s_tasks[TASK_MQTT]);
    syn_task_suspend(&s_tasks[TASK_HTTP_CLIENT]);
    syn_task_suspend(&s_tasks[TASK_DNS]);

    /* Init scheduler */
    syn_sched_init(&s_sched, s_tasks, TASK_COUNT);
    syn_cli_set_scheduler(&s_sched);

    /* Main loop: pump the SyntropicOS scheduler */
    for (;;) {
        syn_sched_run(&s_sched);
        taskYIELD();
    }
}
