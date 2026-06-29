/**
 * @file main.c
 * @brief Native host test harness for syn_imgui / syn_canvas.
 *
 * Renders GUI pages to raw 1bpp SSD1306-format framebuffers, decodes them,
 * and writes scaled PNGs via stb_image_write. Accepts the same query
 * parameters as the ESP32 /fb HTTP handler so the same capture_fb.py
 * workflow works against a local HTTP server.
 *
 * Usage (HTTP mode, default port 8080):
 *   ./imgui_host [--port N]
 *
 * Then: python3 tools/capture_fb.py --host 127.0.0.1 --port 8080 --page 0
 */

#define _POSIX_C_SOURCE 200809L

#include "syntropic/display/syn_canvas.h"
#include "syntropic/ui/syn_imgui.h"
#include "syntropic/common/syn_defs.h"
#include "syntropic/util/syn_assert.h"

/* stb_image_write — single-header PNG writer */
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

/* POSIX sockets for the tiny HTTP server */
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>

/* ── assert_failed (required by SYN_ASSERT) ─────────────────────────────── */

void syn_assert_failed(const char *file, int line)
{
    fprintf(stderr, "ASSERT FAILED: %s:%d\n", file, line);
    abort();
}

/* ── Canvas / GUI constants ─────────────────────────────────────────────── */

#define CANVAS_W  128
#define CANVAS_H  64
#define FB_SIZE   (CANVAS_W * CANVAS_H / 8)   /* 1024 bytes, 1bpp col-major */

/* ── Persistent widget state (mirrors main.c on ESP32) ─────────────────── */

static SYN_IMGUI_Context s_gui_ctx;
static SYN_Canvas        s_canvas;
static uint8_t           s_framebuf[FB_SIZE];
static bool              s_gui_initialized = false;

static int32_t  s_page           = 0;
static int16_t  s_scroll_offset  = 0;
static int16_t  s_marquee_offset = 0;
static int32_t  s_slider_val     = 50;
static int32_t  s_spinner_val    = 10;
static bool     s_collapse1_open = false;
static bool     s_collapse2_open = false;
static int32_t  s_progress_val   = 0;
static uint32_t s_checkbox_flags = 0x05;
static bool     s_sel_states[8]  = {false};

/* ── Decode 1bpp SSD1306 framebuffer → RGBA pixels ─────────────────────── */

static void fb_to_rgba(const uint8_t *fb, uint8_t *rgba,
                       int w, int h, int scale)
{
    for (int py = 0; py < h; py++) {
        for (int px = 0; px < w; px++) {
            int page = py / 8;
            int bit  = py % 8;
            uint8_t byte = fb[page * w + px];
            uint8_t val  = ((byte >> bit) & 1) ? 255 : 0;
            /* Write scale×scale block */
            for (int sy = 0; sy < scale; sy++) {
                for (int sx = 0; sx < scale; sx++) {
                    int out_x = px * scale + sx;
                    int out_y = py * scale + sy;
                    int idx   = (out_y * w * scale + out_x) * 4;
                    rgba[idx + 0] = val;
                    rgba[idx + 1] = val;
                    rgba[idx + 2] = val;
                    rgba[idx + 3] = 255;
                }
            }
        }
    }
}

/* ── Render one frame into s_framebuf ──────────────────────────────────── */

static void render_frame(int32_t page, bool select, bool back,
                         int32_t enc_delta,
                         bool touch_down, int16_t touch_x, int16_t touch_y)
{
    if (!s_gui_initialized) {
        syn_canvas_init(&s_canvas, s_framebuf, CANVAS_W, CANVAS_H, 1, NULL, NULL);
        syn_imgui_init(&s_gui_ctx);
        s_gui_initialized = true;
    }

    s_page = page;
    s_progress_val = (s_progress_val + 5) % 105;

    syn_canvas_clear(&s_canvas);
    syn_imgui_begin(&s_gui_ctx, &s_canvas, select, back, enc_delta,
                    touch_down, touch_x, touch_y);

    if (s_page == 0) {
        syn_imgui_layout_begin(&s_gui_ctx, 0, 0, 128);
        syn_imgui_separator_text(&s_gui_ctx, "GUI Overview", 0, 0, 0);
        syn_imgui_checkbox_flags(&s_gui_ctx, "LED 1", &s_checkbox_flags, 0x01, 0, 0, 0, 0);
        syn_imgui_checkbox_flags(&s_gui_ctx, "LED 2", &s_checkbox_flags, 0x02, 0, 0, 0, 0);
        syn_imgui_slider(&s_gui_ctx, "Brightness", &s_slider_val, 0, 100, 0, 0, 0, 0);
        syn_imgui_value_int(&s_gui_ctx, "Speed", s_spinner_val, 0, 0);
        syn_imgui_layout_end(&s_gui_ctx);

    } else if (s_page == 1) {
        syn_imgui_layout_begin(&s_gui_ctx, 0, 0, 128);
        syn_imgui_text_marquee(&s_gui_ctx, "SyntropicOS Embedded UI Engine",
                               &s_marquee_offset, 0, 0, 0, 1);
        if (syn_imgui_collapsing_header(&s_gui_ctx, "System Config",
                                        &s_collapse1_open, 0, 0, 0, 0)) {
            syn_imgui_label(&s_gui_ctx, "Mode: Active", 0, 0);
            syn_imgui_label(&s_gui_ctx, "Volt: 3.3V", 0, 0);
        }
        if (syn_imgui_collapsing_header(&s_gui_ctx, "Network Status",
                                        &s_collapse2_open, 0, 0, 0, 0)) {
            syn_imgui_label(&s_gui_ctx, "IP: 127.0.0.1", 0, 0);
            syn_imgui_label(&s_gui_ctx, "SSID: host", 0, 0);
        }
        syn_imgui_progress_bar_ex(&s_gui_ctx, s_progress_val, 0, 100,
                                  "Loading...", 0, 0, 0, 0);
        syn_imgui_layout_end(&s_gui_ctx);

    } else {
        syn_imgui_scroll_begin(&s_gui_ctx, 0, 0, 128, 64, &s_scroll_offset);
        char label_buf[16];
        for (int i = 0; i < 8; i++) {
            snprintf(label_buf, sizeof(label_buf), "Item %d", i + 1);
            syn_imgui_selectable(&s_gui_ctx, label_buf, &s_sel_states[i], 0, 0, 0, 0);
        }
        syn_imgui_scroll_end(&s_gui_ctx);
    }

    syn_imgui_end(&s_gui_ctx);
}

/* ── PNG write helper ───────────────────────────────────────────────────── */

static void write_png(const char *path, int scale)
{
    int out_w = CANVAS_W * scale;
    int out_h = CANVAS_H * scale;
    uint8_t *rgba = malloc((size_t)(out_w * out_h * 4));
    if (!rgba) { perror("malloc"); return; }
    fb_to_rgba(s_framebuf, rgba, CANVAS_W, CANVAS_H, scale);
    stbi_write_png(path, out_w, out_h, 4, rgba, out_w * 4);
    free(rgba);
}

/* ── Minimal HTTP server ────────────────────────────────────────────────── */

static void parse_query(const char *query,
                        int32_t *page, int32_t *enc,
                        bool *sel, bool *back,
                        bool *touch, int16_t *tx, int16_t *ty)
{
    if (!query) return;
    const char *p;

    p = strstr(query, "page=");
    if (p) *page = atoi(p + 5);

    p = strstr(query, "enc=");
    if (p) *enc = atoi(p + 4);

    if (strstr(query, "select=1")) *sel = true;
    if (strstr(query, "back=1"))   *back = true;

    p = strstr(query, "touch=");
    if (p) {
        *touch = true;
        sscanf(p + 6, "%hd,%hd", tx, ty);
    }
}

static void send_response(int fd, int status, const char *status_text,
                          const char *content_type,
                          const uint8_t *body, size_t body_len)
{
    char header[256];
    int hlen = snprintf(header, sizeof(header),
        "HTTP/1.0 %d %s\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %zu\r\n"
        "Connection: close\r\n"
        "\r\n",
        status, status_text, content_type, body_len);
    write(fd, header, (size_t)hlen);
    if (body && body_len) write(fd, body, body_len);
}

static void handle_connection(int fd)
{
    char req_buf[1024];
    ssize_t n = recv(fd, req_buf, sizeof(req_buf) - 1, 0);
    if (n <= 0) return;
    req_buf[n] = '\0';

    /* Parse: GET /path?query HTTP/1.x */
    char method[8], uri[512];
    sscanf(req_buf, "%7s %511s", method, uri);

    char *path  = uri;
    char *query = strchr(uri, '?');
    if (query) { *query = '\0'; query++; }

    if (strcmp(path, "/fb") == 0) {
        int32_t page = s_page;
        int32_t enc  = 0;
        bool    sel  = false, bk = false, touch = false;
        int16_t tx   = 0,    ty  = 0;
        parse_query(query, &page, &enc, &sel, &bk, &touch, &tx, &ty);
        render_frame(page, sel, bk, enc, touch, tx, ty);
        send_response(fd, 200, "OK", "application/octet-stream",
                      s_framebuf, FB_SIZE);
    } else if (strcmp(path, "/png") == 0) {
        /* Convenience: return a PNG directly for browser viewing */
        int32_t page = s_page;
        int32_t enc  = 0;
        bool    sel  = false, bk = false, touch = false;
        int16_t tx   = 0,    ty  = 0;
        parse_query(query, &page, &enc, &sel, &bk, &touch, &tx, &ty);
        render_frame(page, sel, bk, enc, touch, tx, ty);

        /* Write PNG to temp file, read back, send */
        const char *tmp = "/tmp/imgui_host_frame.png";
        write_png(tmp, 4);
        FILE *f = fopen(tmp, "rb");
        if (f) {
            fseek(f, 0, SEEK_END);
            long sz = ftell(f);
            rewind(f);
            uint8_t *png = malloc((size_t)sz);
            if (png && fread(png, 1, (size_t)sz, f) == (size_t)sz) {
                send_response(fd, 200, "OK", "image/png", png, (size_t)sz);
            } else {
                send_response(fd, 500, "Error", "text/plain",
                              (const uint8_t *)"read error", 10);
            }
            free(png);
            fclose(f);
        }
    } else {
        static const char *index_html =
            "<html><body><pre>"
            "GET /fb?page=N&amp;enc=D&amp;select=1&amp;back=1  - raw 1bpp framebuffer\n"
            "GET /png?page=N&amp;enc=D&amp;select=1            - PNG (4x scale)\n"
            "</pre></body></html>";
        send_response(fd, 200, "OK", "text/html",
                      (const uint8_t *)index_html, strlen(index_html));
    }
}

/* ── main ───────────────────────────────────────────────────────────────── */

int main(int argc, char *argv[])
{
    int port = 8080;

    /* Optional: --port N or --out path (batch PNG mode) */
    const char *png_out = NULL;
    int32_t batch_page  = -1;  /* -1 = server mode */

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--port") == 0 && i + 1 < argc) {
            port = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--page") == 0 && i + 1 < argc) {
            batch_page = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--out") == 0 && i + 1 < argc) {
            png_out = argv[++i];
        } else if (strcmp(argv[i], "--enc") == 0 && i + 1 < argc) {
            /* parse silently for batch mode */
            (void)atoi(argv[++i]);
        }
    }

    /* Batch mode: render one page to PNG and exit */
    if (batch_page >= 0 && png_out) {
        int32_t enc = 0;
        for (int i = 1; i < argc; i++) {
            if (strcmp(argv[i], "--enc") == 0 && i + 1 < argc)
                enc = atoi(argv[++i]);
        }
        render_frame(batch_page, false, false, enc, false, 0, 0);
        write_png(png_out, 4);
        printf("Wrote %s\n", png_out);
        return 0;
    }

    /* HTTP server mode */
    int srv = socket(AF_INET, SOCK_STREAM, 0);
    if (srv < 0) { perror("socket"); return 1; }

    int one = 1;
    setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));

    struct sockaddr_in addr = {
        .sin_family      = AF_INET,
        .sin_port        = htons((uint16_t)port),
        .sin_addr.s_addr = INADDR_ANY,
    };
    if (bind(srv, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind"); return 1;
    }
    listen(srv, 8);
    printf("imgui_host listening on http://127.0.0.1:%d/\n", port);
    printf("  /fb?page=N    raw 1bpp framebuffer (for capture_fb.py)\n");
    printf("  /png?page=N   PNG (for browser)\n");
    fflush(stdout);

    for (;;) {
        int client = accept(srv, NULL, NULL);
        if (client < 0) { perror("accept"); continue; }
        handle_connection(client);
        close(client);
    }
}
