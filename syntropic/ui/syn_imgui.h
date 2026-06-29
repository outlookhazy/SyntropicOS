/**
 * @file syn_imgui.h
 * @brief Lightweight, zero-allocation Immediate Mode GUI (IMGUI) for embedded systems.
 *
 * Supports both button/encoder physical inputs and touch-screen inputs.
 * @ingroup syn_display
 */

#ifndef SYN_IMGUI_H
#define SYN_IMGUI_H

#include "../common/syn_defs.h"
#include "../display/syn_gfx.h"

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Style / theme ──────────────────────────────────────────────────────── */

/**
 * @brief Visual style parameters for IMGUI widgets.
 *
 * Set via syn_imgui_set_style().  syn_imgui_init() loads the defaults
 * (monochrome-friendly: white-on-black, matching the existing hardcoded
 * colors) so existing code is unaffected.
 */
typedef struct {
    uint16_t fg;        /**< Foreground: text, outlines              */
    uint16_t bg;        /**< Background fill for unfocused widgets   */
    uint16_t highlight; /**< Focused widget fill color               */
    uint16_t fg_inv;    /**< Text drawn on a highlighted background  */
    uint16_t accent;    /**< Slider fill, progress bar, gauge needle */
    uint16_t disabled;  /**< Disabled widget color                   */
    int16_t  padding;   /**< Inner padding in pixels (default 2)     */
    int16_t  spacing;   /**< Inter-widget gap for auto-layout (default 3) */
} SYN_IMGUI_Style;

/* ── Layout cursor ───────────────────────────────────────────────────────── */

/**
 * @brief Maximum number of columns in a layout_row call.
 */
#define SYN_IMGUI_MAX_ROW_COLS 8

/**
 * @brief Auto-layout state.  Managed by syn_imgui_layout_begin/end.
 *
 * When active (in_layout == true) and a widget receives x=0 y=0 w=0 h=0,
 * it uses the cursor position and default sizes from this struct instead.
 * Widgets that use the cursor advance it automatically.
 */
typedef struct {
    bool    in_layout; /**< True while between layout_begin / layout_end   */
    int16_t cx;        /**< Cursor X (left edge of next widget)            */
    int16_t cy;        /**< Cursor Y (top edge of next widget)             */
    int16_t origin_x;  /**< Left margin for new rows                       */
    int16_t width;     /**< Default widget width (set in layout_begin)     */
    int16_t row_h;     /**< Height of tallest widget in the current row    */
    bool    same_line; /**< True: next widget appends right, not new row   */
    int16_t row_x;         /**< X position of end of last resolved widget + spacing */
    int16_t row_y;         /**< Y position of current row start                     */
    int16_t prev_row_h;    /**< Height of current row before layout advancement     */
    /* Multi-column layout row state */
    int     row_items;     /**< Number of columns in current row (0 = default) */
    int     row_item_idx;  /**< Index of next column to consume                */
    int16_t row_height;    /**< Forced row height (0 = auto)                   */
    int16_t row_widths[SYN_IMGUI_MAX_ROW_COLS]; /**< Column widths            */
} SYN_IMGUI_Layout;

/* ── Scroll region state ────────────────────────────────────────────────── */

/** @brief Scroll region state for scrollable IMGUI panels. */
typedef struct {
    bool    in_scroll;     /**< True while between scroll_begin / scroll_end */
    int16_t vp_x;          /**< Viewport left edge                          */
    int16_t vp_y;          /**< Viewport top edge                           */
    int16_t vp_w;          /**< Viewport width                              */
    int16_t vp_h;          /**< Viewport height                             */
    int16_t content_start; /**< Layout cursor Y at scroll_begin              */
    int16_t *scroll_p;     /**< Pointer to user-owned scroll offset          */
    uint16_t focus_y;      /**< Y position of focused widget (for auto-scroll) */
    uint16_t focus_h;      /**< Height of focused widget                     */
} SYN_IMGUI_Scroll;

/** @brief Immediate-mode GUI context — inputs, navigation, style, layout. */
typedef struct {
    SYN_GfxContext gfx;  /**< Graphics context used for drawing */

    /* Physical button/encoder inputs for the current frame */
    bool btn_select;   /**< OK / Enter button pressed */
    bool btn_back;     /**< Cancel / Back button pressed */
    int32_t enc_delta; /**< Rotary encoder delta or +/- navigation count */

    /* Touch screen inputs for the current frame */
    bool touch_down;   /**< True if screen is touched */
    int16_t touch_x;   /**< Touched X coordinate */
    int16_t touch_y;   /**< Touched Y coordinate */

    /* Internal Navigation State */
    uint16_t next_id;       /**< Transient counter for widgets in current frame */
    uint16_t focused_id;    /**< ID of currently highlighted/focused widget */
    uint16_t active_id;     /**< ID of widget currently in "active/editing" mode */
    uint16_t last_max_id;   /**< Total count of widgets from the previous frame */

    /* Style and layout */
    SYN_IMGUI_Style  style;  /**< Visual style — set via syn_imgui_set_style() */
    SYN_IMGUI_Layout layout; /**< Auto-layout cursor state                     */
    SYN_IMGUI_Scroll scroll; /**< Scroll region state                          */
    uint8_t          disabled_depth; /**< >0: widgets skip input, draw dimmed  */
    bool             updated_focus;  /**< Set when focused widget is visited   */
} SYN_IMGUI_Context;

/**
 * @brief Initialize the IMGUI context.
 *
 * @param ctx Context to initialize.
 */
void syn_imgui_init(SYN_IMGUI_Context *ctx);

/**
 * @brief Override the active style.
 *
 * @param ctx    Context.
 * @param style  Style to copy into the context.
 */
void syn_imgui_set_style(SYN_IMGUI_Context *ctx, const SYN_IMGUI_Style *style);

/**
 * @brief Return the default monochrome style (white-on-black).
 *
 * The returned struct may be modified and passed to syn_imgui_set_style().
 *
 * @return Default style struct.
 */
SYN_IMGUI_Style syn_imgui_default_style(void);

/**
 * @brief Begin a new IMGUI frame.
 *
 * Resets the transient ID counter, processes physical navigation inputs,
 * and resets active states.
 *
 * @param ctx         Context.
 * @param gfx         Graphics context to draw on.
 * @param select      True if select button was pressed this frame.
 * @param back        True if back button was pressed this frame.
 * @param enc_delta   Encoder rotation value for this frame.
 * @param touch_down  True if touch screen is active.
 * @param touch_x     Touch X coordinate.
 * @param touch_y     Touch Y coordinate.
 */
void syn_imgui_begin(SYN_IMGUI_Context *ctx, SYN_GfxContext gfx,
                      bool select, bool back, int32_t enc_delta,
                      bool touch_down, int16_t touch_x, int16_t touch_y);

/**
 * @brief End the IMGUI frame.
 *
 * Finalizes widget count tracking.
 *
 * @param ctx Context.
 */
void syn_imgui_end(SYN_IMGUI_Context *ctx);

/* ── Auto-layout ─────────────────────────────────────────────────────────── */

/**
 * @brief Begin an auto-layout region.
 *
 * Widgets within this region may pass x=0, y=0, w=0, h=0 to use the
 * layout cursor.  Widgets that use the cursor advance it down by
 * (widget_height + style.spacing) after each call.
 *
 * @param ctx  Context.
 * @param x    Left edge of the layout region.
 * @param y    Top edge.
 * @param w    Default widget width (used when widget's w parameter is 0).
 */
void syn_imgui_layout_begin(SYN_IMGUI_Context *ctx,
                             int16_t x, int16_t y, int16_t w);

/**
 * @brief End the auto-layout region.
 *
 * @param ctx  Context.
 */
void syn_imgui_layout_end(SYN_IMGUI_Context *ctx);

/**
 * @brief Place the next widget on the same row as the previous one.
 *
 * Call immediately before the next widget.  The next widget's x is set
 * to (previous widget right edge + style.spacing).
 *
 * @param ctx  Context.
 */
void syn_imgui_same_line(SYN_IMGUI_Context *ctx);

/**
 * @brief Insert vertical spacing (moves the layout cursor down).
 *
 * @param ctx     Context.
 * @param pixels  Number of pixels to advance downward.
 */
void syn_imgui_spacing(SYN_IMGUI_Context *ctx, int16_t pixels);

/* ── Simple display widgets ─────────────────────────────────────────────── */

/**
 * @brief Draw a static text label (no navigation ID, no interaction).
 *
 * Uses style.fg for color.  When inside a layout region with x=0, y=0,
 * the label is drawn at the current cursor and the cursor advances.
 *
 * @param ctx   Context.
 * @param text  Null-terminated string.
 * @param x     X coordinate (0 = use layout cursor).
 * @param y     Y coordinate (0 = use layout cursor).
 */
void syn_imgui_label(SYN_IMGUI_Context *ctx, const char *text,
                      int16_t x, int16_t y);

/**
 * @brief Draw a horizontal separator line.
 *
 * When inside a layout region, the line spans the full layout width and
 * the cursor advances by 1 + style.spacing.
 *
 * @param ctx  Context.
 * @param x    X start (0 = use layout cursor).
 * @param y    Y position (0 = use layout cursor).
 * @param w    Width in pixels (0 = use layout width).
 */
void syn_imgui_separator(SYN_IMGUI_Context *ctx,
                          int16_t x, int16_t y, int16_t w);

/**
 * @brief Integer numeric spinner widget.
 *
 * Renders `Label  [< value >]`.  When focused, encoder delta increments
 * the value by @p step.  Returns true on the frame the value changes.
 *
 * @param ctx    Context.
 * @param label  Display label.
 * @param value  Pointer to the current value (modified in place).
 * @param min    Minimum value (clamped, wraps to max).
 * @param max    Maximum value (clamped, wraps to min).
 * @param step   Increment / decrement amount per encoder tick.
 * @param x      X coordinate (0 = use layout cursor).
 * @param y      Y coordinate (0 = use layout cursor).
 * @param w      Width  (0 = use layout width).
 * @param h      Height (0 = font height + 2*padding).
 * @return true if the value changed this frame.
 */
bool syn_imgui_spinner(SYN_IMGUI_Context *ctx, const char *label,
                        int32_t *value, int32_t min, int32_t max, int32_t step,
                        int16_t x, int16_t y, int16_t w, int16_t h);

/**
 * @brief Draw and handle a button.
 *
 * @param ctx   Context.
 * @param label Button text.
 * @param x     X coordinate.
 * @param y     Y coordinate.
 * @param w     Width.
 * @param h     Height.
 * @return true if the button was selected/clicked this frame.
 */
bool syn_imgui_button(SYN_IMGUI_Context *ctx, const char *label,
                       int16_t x, int16_t y, int16_t w, int16_t h);

/**
 * @brief Draw and handle a checkbox.
 *
 * @param ctx     Context.
 * @param label   Checkbox label.
 * @param checked Pointer to boolean state.
 * @param x       X coordinate.
 * @param y       Y coordinate.
 * @param w       Width.
 * @param h       Height.
 * @return true if the checkbox state was toggled this frame.
 */
bool syn_imgui_checkbox(SYN_IMGUI_Context *ctx, const char *label,
                         bool *checked, int16_t x, int16_t y, int16_t w, int16_t h);

/**
 * @brief Draw and handle an integer value slider.
 *
 * When clicked or touched, this widget goes into "edit mode". While in edit mode,
 * encoder rotation or direct touch drags change the value.
 *
 * @param ctx   Context.
 * @param label Slider label.
 * @param value Pointer to integer value.
 * @param min   Minimum allowed value.
 * @param max   Maximum allowed value.
 * @param x     X coordinate.
 * @param y     Y coordinate.
 * @param w     Width.
 * @param h     Height.
 * @return true if the value was modified this frame.
 */
bool syn_imgui_slider(SYN_IMGUI_Context *ctx, const char *label,
                       int32_t *value, int32_t min, int32_t max,
                       int16_t x, int16_t y, int16_t w, int16_t h);

/**
 * @brief Draw and handle a radio button option.
 *
 * Sets the selection variable to the target button value if clicked or touched.
 *
 * @param ctx       Context.
 * @param label     Radio button label.
 * @param selection Pointer to active selection variable.
 * @param button_val Specific option value this button represents.
 * @param x         X coordinate.
 * @param y         Y coordinate.
 * @param w         Width.
 * @param h         Height.
 * @return true if the selection changed to this button's value this frame.
 */
bool syn_imgui_radio(SYN_IMGUI_Context *ctx, const char *label,
                      int32_t *selection, int32_t button_val,
                      int16_t x, int16_t y, int16_t w, int16_t h);

/**
 * @brief Renders a non-interactive progress bar.
 *
 * @param ctx   Context.
 * @param value Current status value.
 * @param min   Minimum possible value.
 * @param max   Maximum possible value.
 * @param x     X coordinate.
 * @param y     Y coordinate.
 * @param w     Width.
 * @param h     Height.
 */
void syn_imgui_progress_bar(SYN_IMGUI_Context *ctx, int32_t value, int32_t min, int32_t max,
                             int16_t x, int16_t y, int16_t w, int16_t h);

/**
 * @brief Renders and handles an inline option selection dropdown/selector.
 *
 * @param ctx          Context.
 * @param label        Selector label.
 * @param options      Array of option string labels.
 * @param count        Number of option strings in the array.
 * @param selected     Pointer to active selected index.
 * @param x            X coordinate.
 * @param y            Y coordinate.
 * @param w            Width.
 * @param h            Height.
 * @return true if selection changed this frame.
 */
bool syn_imgui_combo(SYN_IMGUI_Context *ctx, const char *label,
                      const char **options, size_t count, int32_t *selected,
                      int16_t x, int16_t y, int16_t w, int16_t h);

/**
 * @brief Renders a line telemetry chart.
 *
 * @param ctx     Context.
 * @param title   Chart title label.
 * @param data    Pointer to array of data points.
 * @param count   Number of data points.
 * @param min_val Minimum scale limit.
 * @param max_val Maximum scale limit.
 * @param x       X coordinate.
 * @param y       Y coordinate.
 * @param w       Width.
 * @param h       Height.
 */
void syn_imgui_graph(SYN_IMGUI_Context *ctx, const char *title,
                      const int32_t *data, size_t count,
                      int32_t min_val, int32_t max_val,
                      int16_t x, int16_t y, int16_t w, int16_t h);

/**
 * @brief Renders an analog meter gauge with a needle indicator.
 *
 * Uses lightweight fixed-point integer mathematics for vector coordinates.
 *
 * @param ctx    Context.
 * @param label  Gauge label.
 * @param value  Current value to point needle at.
 * @param min    Minimum bounds.
 * @param max    Maximum bounds.
 * @param cx     Center X coordinate.
 * @param cy     Center Y coordinate.
 * @param radius Radius of gauge boundary.
 */
void syn_imgui_gauge(SYN_IMGUI_Context *ctx, const char *label,
                      int32_t value, int32_t min, int32_t max,
                      int16_t cx, int16_t cy, int16_t radius);

/**
 * @brief Renders a modal confirmation dialog overlay.
 *
 * Captures all navigation and touch focus until dismissed.
 *
 * @param ctx        Context.
 * @param message    Display text.
 * @param ok_clicked Pointer to check trigger results (updated when OK is clicked).
 * @param x          X coordinate.
 * @param y          Y coordinate.
 * @param w          Width.
 * @param h          Height.
 * @return true if the modal dialog has been dismissed (OK or CANCEL selected).
 */
bool syn_imgui_dialog(SYN_IMGUI_Context *ctx, const char *message,
                       bool *ok_clicked, int16_t x, int16_t y, int16_t w, int16_t h);

/* ── Scroll region ──────────────────────────────────────────────────────── */

/**
 * @brief Begin a scrollable region.
 *
 * Widgets between scroll_begin and scroll_end are clipped to the viewport
 * and offset by the scroll position.  The scroll offset is auto-adjusted
 * to keep the focused widget visible.
 *
 * @param ctx     Context.
 * @param x       Viewport left edge.
 * @param y       Viewport top edge.
 * @param w       Viewport width.
 * @param h       Viewport height.
 * @param scroll  Pointer to user-owned scroll offset (persists between frames).
 */
void syn_imgui_scroll_begin(SYN_IMGUI_Context *ctx,
                             int16_t x, int16_t y, int16_t w, int16_t h,
                             int16_t *scroll);

/**
 * @brief End the scrollable region and draw scroll indicator.
 *
 * @param ctx  Context.
 */
void syn_imgui_scroll_end(SYN_IMGUI_Context *ctx);

/* ── Toggle switch ──────────────────────────────────────────────────────── */

/**
 * @brief ON/OFF toggle switch.
 *
 * @param ctx    Context.
 * @param label  Label text.
 * @param state  Pointer to boolean state.
 * @param x,y,w,h  Bounds (0 = auto-layout).
 * @return true if state was toggled this frame.
 */
bool syn_imgui_toggle(SYN_IMGUI_Context *ctx, const char *label,
                       bool *state, int16_t x, int16_t y, int16_t w, int16_t h);

/* ── Disabled state ─────────────────────────────────────────────────────── */

/**
 * @brief Begin a disabled region.  Widgets draw dimmed and skip input.
 *
 * Calls may be nested; the region is re-enabled only when every
 * begin_disabled has a matching end_disabled.
 *
 * @param ctx  Context.
 */
void syn_imgui_begin_disabled(SYN_IMGUI_Context *ctx);

/**
 * @brief End a disabled region.
 *
 * @param ctx  Context.
 */
void syn_imgui_end_disabled(SYN_IMGUI_Context *ctx);

/* ── Text alignment helpers ─────────────────────────────────────────────── */

/**
 * @brief Draw a label in an arbitrary color.
 *
 * @param ctx    Context.
 * @param text   Null-terminated string.
 * @param color  16-bit color value.
 * @param x,y    Position.
 */
void syn_imgui_label_colored(SYN_IMGUI_Context *ctx, const char *text,
                              uint16_t color, int16_t x, int16_t y);

/**
 * @brief Draw a right-aligned label within a given width.
 *
 * @param ctx   Context.
 * @param text  Null-terminated string.
 * @param x,y   Left edge and vertical position.
 * @param w     Width to right-align within.
 */
void syn_imgui_label_right(SYN_IMGUI_Context *ctx, const char *text,
                            int16_t x, int16_t y, int16_t w);

/**
 * @brief Draw a centered label within a given width.
 *
 * @param ctx   Context.
 * @param text  Null-terminated string.
 * @param x,y   Left edge and vertical position.
 * @param w     Width to center within.
 */
void syn_imgui_label_centered(SYN_IMGUI_Context *ctx, const char *text,
                               int16_t x, int16_t y, int16_t w);

/* ── Group box ──────────────────────────────────────────────────────────── */

/**
 * @brief Begin a visual group box with an optional title.
 *
 * @param ctx    Context.
 * @param title  Title text (NULL for no title).
 * @param x,y,w,h  Bounds.
 */
void syn_imgui_group_begin(SYN_IMGUI_Context *ctx, const char *title,
                            int16_t x, int16_t y, int16_t w, int16_t h);

/**
 * @brief End the group box.
 *
 * @param ctx  Context.
 */
void syn_imgui_group_end(SYN_IMGUI_Context *ctx);

/* ── Tab bar ────────────────────────────────────────────────────────────── */

/**
 * @brief Horizontal tab selector.
 *
 * Renders a row of labeled tabs.  Consumes one navigation ID.
 * When focused, encoder cycles through tabs.
 *
 * @param ctx     Context.
 * @param labels  Array of tab label strings.
 * @param count   Number of tabs.
 * @param active  Pointer to active tab index.
 * @param x,y,w   Position and total width.
 * @return true if the active tab changed.
 */
bool syn_imgui_tabs(SYN_IMGUI_Context *ctx, const char **labels,
                    size_t count, int32_t *active,
                    int16_t x, int16_t y, int16_t w);

/* ── Bar chart ──────────────────────────────────────────────────────────── */

/**
 * @brief Vertical bar chart.
 *
 * @param ctx      Context.
 * @param title    Chart title (drawn top-left).
 * @param data     Data values.
 * @param count    Number of values.
 * @param min_val  Minimum axis value.
 * @param max_val  Maximum axis value.
 * @param x,y,w,h  Bounds.
 */
void syn_imgui_bar_chart(SYN_IMGUI_Context *ctx, const char *title,
                          const int32_t *data, size_t count,
                          int32_t min_val, int32_t max_val,
                          int16_t x, int16_t y, int16_t w, int16_t h);

/* ── Icon button ────────────────────────────────────────────────────────── */

/**
 * @brief Button that displays a monochrome icon bitmap.
 *
 * @param ctx           Context.
 * @param icon          Bitmap data (1bpp, row-major, MSB first).
 * @param icon_w,icon_h Icon dimensions in pixels.
 * @param x,y,w,h       Button bounds (0 = auto-layout).
 * @return true if clicked this frame.
 */
bool syn_imgui_icon_button(SYN_IMGUI_Context *ctx,
                            const uint8_t *icon, int16_t icon_w, int16_t icon_h,
                            int16_t x, int16_t y, int16_t w, int16_t h);

/* ── Status bar ─────────────────────────────────────────────────────────── */

/**
 * @brief Draw a single-line status bar with a separator above it.
 * @param ctx   IMGUI context.
 * @param text  Status text to display.
 * @param x     X position.
 * @param y     Y position.
 * @param w     Width in pixels.
 */
void syn_imgui_status_bar(SYN_IMGUI_Context *ctx, const char *text,
                           int16_t x, int16_t y, int16_t w);

/* ── Separator text ─────────────────────────────────────────────────────── */

/**
 * @brief Draw a labeled separator:  ── Title ──────────
 *
 * @param ctx   Context.
 * @param text  Section title text.
 * @param x     X coordinate.
 * @param y     Y coordinate.
 * @param w     Total width.
 */
void syn_imgui_separator_text(SYN_IMGUI_Context *ctx, const char *text,
                                int16_t x, int16_t y, int16_t w);

/* ── Checkbox flags ─────────────────────────────────────────────────────── */

/**
 * @brief Checkbox that toggles a bitmask in a flags word.
 *
 * @param ctx    Context.
 * @param label  Display label.
 * @param flags  Pointer to the flags word (modified in place).
 * @param mask   Bitmask to toggle.
 * @param x,y,w,h  Bounds (0 = auto-layout).
 * @return true if the value changed this frame.
 */
bool syn_imgui_checkbox_flags(SYN_IMGUI_Context *ctx, const char *label,
                                uint32_t *flags, uint32_t mask,
                                int16_t x, int16_t y, int16_t w, int16_t h);

/* ── Value display ──────────────────────────────────────────────────────── */

/**
 * @brief Draw a non-interactive "label: value" pair.
 *
 * @param ctx    Context.
 * @param label  Key text.
 * @param value  Integer value.
 * @param x,y    Position.
 */
void syn_imgui_value_int(SYN_IMGUI_Context *ctx, const char *label,
                           int32_t value, int16_t x, int16_t y);

/* ── Progress bar with overlay text ─────────────────────────────────────── */

/**
 * @brief Enhanced progress bar with overlay text and indeterminate mode.
 *
 * @param ctx      Context.
 * @param value    Current value. If value < min, renders indeterminate animation.
 * @param min,max  Value range.
 * @param overlay  Overlay text (NULL = auto "XX%", "" = no text).
 * @param x,y,w,h  Bounds.
 */
void syn_imgui_progress_bar_ex(SYN_IMGUI_Context *ctx, int32_t value,
                                 int32_t min, int32_t max,
                                 const char *overlay,
                                 int16_t x, int16_t y, int16_t w, int16_t h);

/* ── Selectable ─────────────────────────────────────────────────────────── */

/**
 * @brief Full-width clickable row (like a borderless toggleable button).
 *
 * @param ctx       Context.
 * @param label     Display text.
 * @param selected  Pointer to selection state (toggled on click).
 * @param x,y,w,h   Bounds (0 = auto-layout).
 * @return true if clicked this frame.
 */
bool syn_imgui_selectable(SYN_IMGUI_Context *ctx, const char *label,
                            bool *selected,
                            int16_t x, int16_t y, int16_t w, int16_t h);

/* ── Collapsing header ──────────────────────────────────────────────────── */

/**
 * @brief Collapsible section header.
 *
 * Renders "+ Label" (collapsed) or "- Label" (expanded).
 * Toggles *expanded on click.
 *
 * @param ctx       Context.
 * @param label     Section title.
 * @param expanded  Pointer to expansion state (toggled on click).
 * @param x,y,w,h   Bounds (0 = auto-layout).
 * @return true if the expanded state changed this frame.
 */
bool syn_imgui_collapsing_header(SYN_IMGUI_Context *ctx, const char *label,
                                   bool *expanded,
                                   int16_t x, int16_t y, int16_t w, int16_t h);

/* ── Text word-wrap ─────────────────────────────────────────────────────── */

/**
 * @brief Render word-wrapped text within a width.
 *
 * @param ctx   Context.
 * @param text  Null-terminated string.
 * @param x,y   Top-left corner.
 * @param w     Maximum width (wraps at word boundaries).
 */
void syn_imgui_text_wrapped(SYN_IMGUI_Context *ctx, const char *text,
                              int16_t x, int16_t y, int16_t w);

/* ── Layout row (multi-column) ──────────────────────────────────────────── */

/**
 * @brief Define a multi-column layout row.
 *
 * Subsequent auto-layout widgets fill columns left-to-right.
 * Positive width = fixed pixels. Negative width = fill remainder.
 *
 * @param ctx      Context (must be inside a layout region).
 * @param items    Number of columns (max SYN_IMGUI_MAX_ROW_COLS).
 * @param widths   Array of column widths (NULL for equal split).
 * @param height   Row height (0 = auto).
 */
void syn_imgui_layout_row(SYN_IMGUI_Context *ctx, int items,
                            const int16_t *widths, int16_t height);

/* ── Visibility culling ─────────────────────────────────────────────────── */

/**
 * @brief Check if a widget at (y, h) is visible in the scroll viewport.
 *
 * Returns true if not in a scroll region or if any part is visible.
 * Widgets that return false should still call next_id++ for focus.
 *
 * @param ctx  Context.
 * @param y    Widget top edge.
 * @param h    Widget height.
 * @return true if any part of the widget is visible (or no scroll active).
 */
bool syn_imgui_widget_visible(const SYN_IMGUI_Context *ctx,
                                int16_t y, int16_t h);

/* ── Text clipped ───────────────────────────────────────────────────────── */

/**
 * @brief Draw text clipped to a bounding rectangle.
 *
 * Uses the canvas clip rect to prevent text overflow.
 *
 * @param ctx          Context.
 * @param text         Null-terminated string.
 * @param x,y          Text draw position.
 * @param clip_x,clip_y,clip_w,clip_h  Clipping rectangle.
 */
void syn_imgui_text_clipped(SYN_IMGUI_Context *ctx, const char *text,
                              int16_t x, int16_t y,
                              int16_t clip_x, int16_t clip_y,
                              int16_t clip_w, int16_t clip_h);

/* ── Text marquee ───────────────────────────────────────────────────────── */

/**
 * @brief Scrolling text label with pause at each end.
 *
 * If the text fits within @p w, it's drawn statically.
 * Otherwise it scrolls left, pauses, scrolls right, pauses, and repeats.
 * The caller owns the scroll state (just declare `static int16_t off = 0;`).
 *
 * @param ctx     Context.
 * @param text    Null-terminated string.
 * @param offset  Pointer to caller-owned scroll offset (auto-advanced).
 * @param x,y     Top-left corner.
 * @param w       Available width (clips text beyond this).
 * @param speed   Pixels per frame (1–2 typical).
 */
void syn_imgui_text_marquee(SYN_IMGUI_Context *ctx, const char *text,
                              int16_t *offset,
                              int16_t x, int16_t y, int16_t w,
                              int16_t speed);

#ifdef __cplusplus
}
#endif

#endif /* SYN_IMGUI_H */
