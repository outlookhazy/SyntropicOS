/**
 * @file gen_screenshot.c
 * @brief Renders a mock dashboard in 16bpp color mode and writes the framebuffer to disk.
 */

#include "syntropic/syntropic.h"
#include "syntropic/display/syn_canvas.h"
#include "syntropic/ui/syn_imgui.h"

#include <stdio.h>
#include <string.h>

int main(void)
{
    /* 16bpp RGB565: 128 * 64 * 2 = 16384 bytes */
    static uint8_t fb[128 * 64 * 2];
    SYN_Canvas canvas;
    syn_canvas_init(&canvas, fb, 128, 64, 16, NULL, NULL);

    SYN_IMGUI_Context ctx;
    syn_imgui_init(&ctx);

    /* Setup state variables */
    int32_t mode = 1;         /* Auto mode selected */
    bool led_state = true;    /* LED status checkbox checked */
    int32_t speed = 70;       /* Speed slider set to 70% */
    int32_t temp_data[10] = { 20, 24, 30, 42, 35, 45, 55, 60, 58, 62 }; /* Graph telemetry */
    (void)mode;
    (void)led_state;
    (void)speed;

    /* Mock focus state: we want to highlight the Speed slider */
    ctx.last_max_id = 4;
    ctx.focused_id = 3; /* Widget 3 is the slider */

    /* Render Frame */
    syn_canvas_clear(&canvas);

    /* 1. Header Title & Divider (Color Mode) */
    /* Draw solid blue header bar */
    syn_canvas_rect_fill(&canvas, 0, 0, 128, 10, syn_rgb565(0, 50, 180));
    syn_canvas_text(&canvas, 6, 1, "SyntropicOS IMGUI DASHBOARD", syn_rgb565(255, 255, 255));
    /* Draw bright yellow divider line */
    syn_canvas_line(&canvas, 0, 10, 127, 10, syn_rgb565(220, 220, 0));

    syn_imgui_begin(&ctx, &canvas, false, false, 0, false, 0, 0);

    /* 2. persistent Tabs */
    syn_imgui_button(&ctx, "Dash", 2, 12, 60, 12);
    syn_imgui_button(&ctx, "Stats", 66, 12, 60, 12);

    /* 3. Page 1 Content (Telemetry / Stats) */
    syn_imgui_gauge(&ctx, "Load", 60, 0, 100, 30, 52, 10);
    syn_imgui_graph(&ctx, "Temp", temp_data, 10, 0, 100, 66, 26, 60, 36);

    syn_imgui_end(&ctx);

    /* Overwrite the gauge needle in red to show off color capability! */
    syn_canvas_line(&canvas, 30, 52, 32, 45, syn_rgb565(255, 0, 0));

    /* Write framebuffer bytes to file */
    FILE *f = fopen("tests/framebuffer.bin", "wb");
    if (!f) {
        printf("Error: Could not open output file.\n");
        return 1;
    }
    fwrite(fb, 1, sizeof(fb), f);
    fclose(f);

    printf("Color framebuffer generated successfully.\n");
    return 0;
}
