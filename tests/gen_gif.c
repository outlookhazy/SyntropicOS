/**
 * @file gen_gif.c
 * @brief Renders a multi-frame sequence of SyntropicOS IMGUI to simulate interactive screen and values.
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
    SYN_IMGUI_Context ctx;

    /* Initialize IMGUI context */
    syn_imgui_init(&ctx);

    /* Setup state variables */
    int32_t page = 0;                     /* Page selection (0 = Dashboard, 1 = Stats) */
    int32_t mode = 1;                     /* Radio button state (1=Auto, 2=Manual) */
    bool led_state = true;                /* Checkbox state */
    int32_t speed = 50;                   /* Slider speed */
    int32_t temp_data[10] = { 20, 24, 30, 42, 35, 45, 55, 60, 58, 62 }; /* Graph telemetry */
    int32_t load_value = 50;              /* Gauge load percentage */
    bool show_dialog = false;
    bool dialog_ok = false;

    FILE *f = fopen("tests/simulation_fb.bin", "wb");
    if (!f) {
        printf("Error: Could not open tests/simulation_fb.bin for writing.\n");
        return 1;
    }

    const int total_frames = 100;
    for (int frame = 0; frame < total_frames; frame++) {
        /* Clear canvas */
        syn_canvas_init(&canvas, fb, 128, 64, 16, NULL, NULL);
        syn_canvas_clear(&canvas);

        /* Set up standard input defaults for this frame */
        bool select = false;
        bool back = false;
        int32_t enc_delta = 0;
        bool touch_down = false;
        int16_t touch_x = 0;
        int16_t touch_y = 0;

        /* ------------------ Simulation Scenario Timeline ------------------ */

        /* Background: Always sweep telemetry data */
        if (frame % 2 == 0) {
            for (int i = 0; i < 9; i++) {
                temp_data[i] = temp_data[i + 1];
            }
            int osc = (frame % 10) * 5;
            temp_data[9] = 30 + osc;
        }
        if (frame < 50) {
            load_value = 30 + (frame % 10) * 6;
        } else {
            load_value = 90 - ((frame - 50) % 10) * 6;
        }

        /* 1. Dashboard (Page 0) - Slider Ramping (Frames 0-19) */
        if (frame >= 0 && frame < 20) {
            page = 0;
            if (frame < 10) {
                speed = 50 + frame * 3;       /* 50 -> 80 */
            } else {
                speed = 80 - (frame - 10) * 3; /* 80 -> 50 */
            }
        }

        /* 2. Navigation on Page 0 (Frames 20-34) */
        else if (frame >= 20 && frame < 35) {
            page = 0;
            if (frame == 20) {
                ctx.focused_id = 3; /* Focus on Auto radio button */
            } else if (frame == 22) {
                enc_delta = 1;      /* Move focus to LED checkbox (id = 4) */
            } else if (frame == 24) {
                select = true;      /* Toggle LED checkbox state */
            } else if (frame == 26) {
                enc_delta = 1;      /* Move focus to Speed slider (id = 5) */
            } else if (frame == 28) {
                select = true;      /* Enter Slider Edit Mode */
            } else if (frame >= 29 && frame < 33) {
                enc_delta = 6;      /* Increase Slider value */
            } else if (frame == 33) {
                select = true;      /* Exit Slider Edit Mode */
            }
        }

        /* 3. Shift focus to Stats Tab & switch page (Frames 35-44) */
        else if (frame >= 35 && frame < 45) {
            page = 0;
            if (frame == 35) {
                enc_delta = -1; /* Speed -> LED (id = 4) */
            } else if (frame == 37) {
                enc_delta = -1; /* LED -> Auto (id = 3) */
            } else if (frame == 39) {
                enc_delta = -1; /* Auto -> Stats Tab Button (id = 2) */
            } else if (frame == 41) {
                select = true;  /* Click Stats Tab Button (switches page to 1) */
            }
        }

        /* 4. Page 1 active - Telemetry monitoring (Frames 45-69) */
        else if (frame >= 45 && frame < 70) {
            page = 1;
            /* Navigate between Dash (id=1) and Stats (id=2) tabs */
            if (frame == 50) {
                enc_delta = -1; /* Focus moves to Dash Tab (id=1) */
            } else if (frame == 55) {
                enc_delta = 1;  /* Focus moves back to Stats Tab (id=2) */
            }
        }

        /* 5. Navigate back to Page 0 via tabs (Frames 70-74) */
        else if (frame >= 70 && frame < 75) {
            if (frame == 70) {
                page = 1;
                enc_delta = -1; /* Stats -> Dash Tab (id=1) */
            } else if (frame == 72) {
                page = 1;
                select = true;  /* Click Dash Tab Button (switches page to 0) */
            } else {
                page = 0;
            }
        }

        /* 6. Direct touch screen interactions (Frames 75-84) */
        else if (frame >= 75 && frame < 85) {
            /* Switch to Stats page by touching the Stats tab button */
            if (frame >= 76 && frame <= 78) {
                touch_down = true;
                touch_x = 90;
                touch_y = 18;
            }
            /* Release and view Stats page */
            else if (frame > 78 && frame <= 80) {
                page = 1;
            }
            /* Switch back to Dashboard page by touching the Dash tab button */
            else if (frame >= 81 && frame <= 83) {
                touch_down = true;
                touch_x = 30;
                touch_y = 18;
            }
            else {
                page = 0;
            }
        }

        /* 7. Modal Dialog Overlay (Frames 85-99) */
        else if (frame >= 85 && frame < 100) {
            page = 0;
            show_dialog = true;

            if (frame == 89) {
                enc_delta = 1;  /* Focus to Cancel button */
            } else if (frame == 93) {
                enc_delta = -1; /* Focus to OK button */
            } else if (frame == 97) {
                select = true;  /* Click OK to dismiss */
            }
        }

        /* ------------------ Render GUI Layout ------------------ */

        /* Draw title header & separator */
        syn_canvas_rect_fill(&canvas, 0, 0, 128, 10, syn_rgb565(0, 50, 180));
        syn_canvas_text(&canvas, 6, 1, "SyntropicOS IMGUI DASHBOARD", syn_rgb565(255, 255, 255));
        syn_canvas_line(&canvas, 0, 10, 127, 10, syn_rgb565(220, 220, 0));

        /* Begin IMGUI frame */
        syn_imgui_begin(&ctx, &canvas, select, back, enc_delta, touch_down, touch_x, touch_y);

        /* Render Persistent top navigation tabs */
        bool click_dash = syn_imgui_button(&ctx, "Dash", 2, 12, 60, 12);
        bool click_stats = syn_imgui_button(&ctx, "Stats", 66, 12, 60, 12);

        if (click_dash) page = 0;
        if (click_stats) page = 1;

        /* Highlight active tab manually by drawing an underline */
        if (page == 0) {
            syn_canvas_line(&canvas, 4, 23, 60, 23, syn_rgb565(0, 255, 0));
        } else {
            syn_canvas_line(&canvas, 68, 23, 124, 23, syn_rgb565(0, 255, 0));
        }

        /* Render Page Content */
        if (page == 0) {
            syn_imgui_radio(&ctx, "Auto", &mode, 1, 2, 27, 50, 13);
            syn_imgui_checkbox(&ctx, "LED", &led_state, 66, 27, 60, 13);
            syn_imgui_slider(&ctx, "Speed", &speed, 0, 100, 2, 44, 124, 13);
        } else {
            syn_imgui_gauge(&ctx, "Load", load_value, 0, 100, 30, 52, 10);
            syn_imgui_graph(&ctx, "Temp", temp_data, 10, 0, 100, 66, 26, 60, 36);

            /* Overwrite active gauge needle in red */
            int32_t val = load_value;
            if (val < 0) val = 0;
            if (val > 100) val = 100;
            int32_t val_percent = (val * 1000) / 100;
            int32_t temp = (1000 - val_percent) * 8;
            int32_t idx = temp / 1000;
            int32_t rem = temp % 1000;

            static const int16_t sin_tbl[9] = { 0, 98, 181, 236, 256, 236, 181, 98, 0 };
            static const int16_t cos_tbl[9] = { 256, 236, 181, 98, 0, -98, -181, -236, -256 };

            int32_t sin_val = (idx >= 8) ? sin_tbl[8] : sin_tbl[idx] + (rem * (sin_tbl[idx + 1] - sin_tbl[idx])) / 1000;
            int32_t cos_val = (idx >= 8) ? cos_tbl[8] : cos_tbl[idx] + (rem * (cos_tbl[idx + 1] - cos_tbl[idx])) / 1000;

            int16_t nx = 30 + (int16_t)((cos_val * 6) / 256);
            int16_t ny = 52 - (int16_t)((sin_val * 6) / 256);

            syn_canvas_line(&canvas, 30, 52, nx, ny, syn_rgb565(255, 0, 0));
        }

        /* Render Dialog Overlay if active */
        if (show_dialog) {
            bool dialog_dismissed = syn_imgui_dialog(&ctx, "Confirm?", &dialog_ok, 14, 15, 100, 36);
            if (dialog_dismissed) {
                show_dialog = false;
                /* Toggle LED state to visually confirm */
                if (dialog_ok) {
                    led_state = !led_state;
                }
            }
        }

        syn_imgui_end(&ctx);

        /* Write this frame's raw bytes to output file */
        fwrite(fb, 1, sizeof(fb), f);
    }

    fclose(f);
    printf("Successfully wrote %d frames of raw framebuffer to tests/simulation_fb.bin\n", total_frames);
    return 0;
}
