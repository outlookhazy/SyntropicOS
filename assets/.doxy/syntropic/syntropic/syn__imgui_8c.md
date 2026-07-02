

# File syn\_imgui.c



[**FileList**](files.md) **>** [**src**](dir_68267d1309a1af8e8297ef4c3efbcdba.md) **>** [**syntropic**](dir_d9e327fea148db8a6993543717219dae.md) **>** [**ui**](dir_5167a572f0687ac609ba099b941e0ec0.md) **>** [**syn\_imgui.c**](syn__imgui_8c.md)

[Go to the source code of this file](syn__imgui_8c_source.md)

_Immediate Mode GUI (IMGUI) implementation._ 

* `#include "syn_imgui.h"`
* `#include "../util/syn_assert.h"`
* `#include <string.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**syn\_imgui\_bar\_chart**](#function-syn_imgui_bar_chart) ([**SYN\_IMGUI\_Context**](structSYN__IMGUI__Context.md) \* ctx, const char \* title, const int32\_t \* data, size\_t count, int32\_t min\_val, int32\_t max\_val, int16\_t x, int16\_t y, int16\_t w, int16\_t h) <br>_Vertical bar chart._  |
|  void | [**syn\_imgui\_begin**](#function-syn_imgui_begin) ([**SYN\_IMGUI\_Context**](structSYN__IMGUI__Context.md) \* ctx, [**SYN\_GfxContext**](syn__gfx_8h.md#typedef-syn_gfxcontext) gfx, [**bool**](syn__defs_8h.md#enum-bool) select, [**bool**](syn__defs_8h.md#enum-bool) back, int32\_t enc\_delta, [**bool**](syn__defs_8h.md#enum-bool) touch\_down, int16\_t touch\_x, int16\_t touch\_y) <br>_Begin a new IMGUI frame._  |
|  void | [**syn\_imgui\_begin\_disabled**](#function-syn_imgui_begin_disabled) ([**SYN\_IMGUI\_Context**](structSYN__IMGUI__Context.md) \* ctx) <br>_Begin a disabled region. Widgets draw dimmed and skip input._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_imgui\_button**](#function-syn_imgui_button) ([**SYN\_IMGUI\_Context**](structSYN__IMGUI__Context.md) \* ctx, const char \* label, int16\_t x, int16\_t y, int16\_t w, int16\_t h) <br>_Draw and handle a button._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_imgui\_checkbox**](#function-syn_imgui_checkbox) ([**SYN\_IMGUI\_Context**](structSYN__IMGUI__Context.md) \* ctx, const char \* label, [**bool**](syn__defs_8h.md#enum-bool) \* checked, int16\_t x, int16\_t y, int16\_t w, int16\_t h) <br>_Draw and handle a checkbox._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_imgui\_checkbox\_flags**](#function-syn_imgui_checkbox_flags) ([**SYN\_IMGUI\_Context**](structSYN__IMGUI__Context.md) \* ctx, const char \* label, uint32\_t \* flags, uint32\_t mask, int16\_t x, int16\_t y, int16\_t w, int16\_t h) <br>_Checkbox that toggles a bitmask in a flags word._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_imgui\_collapsing\_header**](#function-syn_imgui_collapsing_header) ([**SYN\_IMGUI\_Context**](structSYN__IMGUI__Context.md) \* ctx, const char \* label, [**bool**](syn__defs_8h.md#enum-bool) \* expanded, int16\_t x, int16\_t y, int16\_t w, int16\_t h) <br>_Collapsible section header._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_imgui\_combo**](#function-syn_imgui_combo) ([**SYN\_IMGUI\_Context**](structSYN__IMGUI__Context.md) \* ctx, const char \* label, const char \*\* options, size\_t count, int32\_t \* selected, int16\_t x, int16\_t y, int16\_t w, int16\_t h) <br>_Renders and handles an inline option selection dropdown/selector._  |
|  [**SYN\_IMGUI\_Style**](structSYN__IMGUI__Style.md) | [**syn\_imgui\_default\_style**](#function-syn_imgui_default_style) (void) <br>_Return the default monochrome style (white-on-black)._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_imgui\_dialog**](#function-syn_imgui_dialog) ([**SYN\_IMGUI\_Context**](structSYN__IMGUI__Context.md) \* ctx, const char \* message, [**bool**](syn__defs_8h.md#enum-bool) \* ok\_clicked, int16\_t x, int16\_t y, int16\_t w, int16\_t h) <br>_Renders a modal confirmation dialog overlay._  |
|  void | [**syn\_imgui\_end**](#function-syn_imgui_end) ([**SYN\_IMGUI\_Context**](structSYN__IMGUI__Context.md) \* ctx) <br>_End the IMGUI frame._  |
|  void | [**syn\_imgui\_end\_disabled**](#function-syn_imgui_end_disabled) ([**SYN\_IMGUI\_Context**](structSYN__IMGUI__Context.md) \* ctx) <br>_End a disabled region._  |
|  void | [**syn\_imgui\_gauge**](#function-syn_imgui_gauge) ([**SYN\_IMGUI\_Context**](structSYN__IMGUI__Context.md) \* ctx, const char \* label, int32\_t value, int32\_t min, int32\_t max, int16\_t cx, int16\_t cy, int16\_t radius) <br>_Renders an analog meter gauge with a needle indicator._  |
|  void | [**syn\_imgui\_graph**](#function-syn_imgui_graph) ([**SYN\_IMGUI\_Context**](structSYN__IMGUI__Context.md) \* ctx, const char \* title, const int32\_t \* data, size\_t count, int32\_t min\_val, int32\_t max\_val, int16\_t x, int16\_t y, int16\_t w, int16\_t h) <br>_Renders a line telemetry chart._  |
|  void | [**syn\_imgui\_group\_begin**](#function-syn_imgui_group_begin) ([**SYN\_IMGUI\_Context**](structSYN__IMGUI__Context.md) \* ctx, const char \* title, int16\_t x, int16\_t y, int16\_t w, int16\_t h) <br>_Begin a visual group box with an optional title._  |
|  void | [**syn\_imgui\_group\_end**](#function-syn_imgui_group_end) ([**SYN\_IMGUI\_Context**](structSYN__IMGUI__Context.md) \* ctx) <br>_End the group box._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_imgui\_icon\_button**](#function-syn_imgui_icon_button) ([**SYN\_IMGUI\_Context**](structSYN__IMGUI__Context.md) \* ctx, const uint8\_t \* icon, int16\_t icon\_w, int16\_t icon\_h, int16\_t x, int16\_t y, int16\_t w, int16\_t h) <br>_Button that displays a monochrome icon bitmap._  |
|  void | [**syn\_imgui\_init**](#function-syn_imgui_init) ([**SYN\_IMGUI\_Context**](structSYN__IMGUI__Context.md) \* ctx) <br>_Initialize the IMGUI context._  |
|  void | [**syn\_imgui\_label**](#function-syn_imgui_label) ([**SYN\_IMGUI\_Context**](structSYN__IMGUI__Context.md) \* ctx, const char \* text, int16\_t x, int16\_t y) <br>_Draw a static text label (no navigation ID, no interaction)._  |
|  void | [**syn\_imgui\_label\_centered**](#function-syn_imgui_label_centered) ([**SYN\_IMGUI\_Context**](structSYN__IMGUI__Context.md) \* ctx, const char \* text, int16\_t x, int16\_t y, int16\_t w) <br>_Draw a centered label within a given width._  |
|  void | [**syn\_imgui\_label\_colored**](#function-syn_imgui_label_colored) ([**SYN\_IMGUI\_Context**](structSYN__IMGUI__Context.md) \* ctx, const char \* text, uint16\_t color, int16\_t x, int16\_t y) <br>_Draw a label in an arbitrary color._  |
|  void | [**syn\_imgui\_label\_right**](#function-syn_imgui_label_right) ([**SYN\_IMGUI\_Context**](structSYN__IMGUI__Context.md) \* ctx, const char \* text, int16\_t x, int16\_t y, int16\_t w) <br>_Draw a right-aligned label within a given width._  |
|  void | [**syn\_imgui\_layout\_begin**](#function-syn_imgui_layout_begin) ([**SYN\_IMGUI\_Context**](structSYN__IMGUI__Context.md) \* ctx, int16\_t x, int16\_t y, int16\_t w) <br>_Begin an auto-layout region._  |
|  void | [**syn\_imgui\_layout\_end**](#function-syn_imgui_layout_end) ([**SYN\_IMGUI\_Context**](structSYN__IMGUI__Context.md) \* ctx) <br>_End the auto-layout region._  |
|  void | [**syn\_imgui\_layout\_row**](#function-syn_imgui_layout_row) ([**SYN\_IMGUI\_Context**](structSYN__IMGUI__Context.md) \* ctx, int items, const int16\_t \* widths, int16\_t height) <br>_Define a multi-column layout row._  |
|  void | [**syn\_imgui\_progress\_bar**](#function-syn_imgui_progress_bar) ([**SYN\_IMGUI\_Context**](structSYN__IMGUI__Context.md) \* ctx, int32\_t value, int32\_t min, int32\_t max, int16\_t x, int16\_t y, int16\_t w, int16\_t h) <br>_Renders a non-interactive progress bar._  |
|  void | [**syn\_imgui\_progress\_bar\_ex**](#function-syn_imgui_progress_bar_ex) ([**SYN\_IMGUI\_Context**](structSYN__IMGUI__Context.md) \* ctx, int32\_t value, int32\_t min, int32\_t max, const char \* overlay, int16\_t x, int16\_t y, int16\_t w, int16\_t h) <br>_Enhanced progress bar with overlay text and indeterminate mode._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_imgui\_radio**](#function-syn_imgui_radio) ([**SYN\_IMGUI\_Context**](structSYN__IMGUI__Context.md) \* ctx, const char \* label, int32\_t \* selection, int32\_t button\_val, int16\_t x, int16\_t y, int16\_t w, int16\_t h) <br>_Draw and handle a radio button option._  |
|  void | [**syn\_imgui\_same\_line**](#function-syn_imgui_same_line) ([**SYN\_IMGUI\_Context**](structSYN__IMGUI__Context.md) \* ctx) <br>_Place the next widget on the same row as the previous one._  |
|  void | [**syn\_imgui\_scroll\_begin**](#function-syn_imgui_scroll_begin) ([**SYN\_IMGUI\_Context**](structSYN__IMGUI__Context.md) \* ctx, int16\_t x, int16\_t y, int16\_t w, int16\_t h, int16\_t \* scroll) <br>_Begin a scrollable region._  |
|  void | [**syn\_imgui\_scroll\_end**](#function-syn_imgui_scroll_end) ([**SYN\_IMGUI\_Context**](structSYN__IMGUI__Context.md) \* ctx) <br>_End the scrollable region and draw scroll indicator._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_imgui\_selectable**](#function-syn_imgui_selectable) ([**SYN\_IMGUI\_Context**](structSYN__IMGUI__Context.md) \* ctx, const char \* label, [**bool**](syn__defs_8h.md#enum-bool) \* selected, int16\_t x, int16\_t y, int16\_t w, int16\_t h) <br>_Full-width clickable row (like a borderless toggleable button)._  |
|  void | [**syn\_imgui\_separator**](#function-syn_imgui_separator) ([**SYN\_IMGUI\_Context**](structSYN__IMGUI__Context.md) \* ctx, int16\_t x, int16\_t y, int16\_t w) <br>_Draw a horizontal separator line._  |
|  void | [**syn\_imgui\_separator\_text**](#function-syn_imgui_separator_text) ([**SYN\_IMGUI\_Context**](structSYN__IMGUI__Context.md) \* ctx, const char \* text, int16\_t x, int16\_t y, int16\_t w) <br>_Draw a labeled separator: ── Title ──────────_  |
|  void | [**syn\_imgui\_set\_style**](#function-syn_imgui_set_style) ([**SYN\_IMGUI\_Context**](structSYN__IMGUI__Context.md) \* ctx, const [**SYN\_IMGUI\_Style**](structSYN__IMGUI__Style.md) \* style) <br>_Override the active style._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_imgui\_slider**](#function-syn_imgui_slider) ([**SYN\_IMGUI\_Context**](structSYN__IMGUI__Context.md) \* ctx, const char \* label, int32\_t \* value, int32\_t min, int32\_t max, int16\_t x, int16\_t y, int16\_t w, int16\_t h) <br>_Draw and handle an integer value slider._  |
|  void | [**syn\_imgui\_spacing**](#function-syn_imgui_spacing) ([**SYN\_IMGUI\_Context**](structSYN__IMGUI__Context.md) \* ctx, int16\_t pixels) <br>_Insert vertical spacing (moves the layout cursor down)._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_imgui\_spinner**](#function-syn_imgui_spinner) ([**SYN\_IMGUI\_Context**](structSYN__IMGUI__Context.md) \* ctx, const char \* label, int32\_t \* value, int32\_t min, int32\_t max, int32\_t step, int16\_t x, int16\_t y, int16\_t w, int16\_t h) <br>_Integer numeric spinner widget._  |
|  void | [**syn\_imgui\_status\_bar**](#function-syn_imgui_status_bar) ([**SYN\_IMGUI\_Context**](structSYN__IMGUI__Context.md) \* ctx, const char \* text, int16\_t x, int16\_t y, int16\_t w) <br>_Draw a single-line status bar with a separator above it._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_imgui\_tabs**](#function-syn_imgui_tabs) ([**SYN\_IMGUI\_Context**](structSYN__IMGUI__Context.md) \* ctx, const char \*\* labels, size\_t count, int32\_t \* active, int16\_t x, int16\_t y, int16\_t w) <br>_Horizontal tab selector._  |
|  void | [**syn\_imgui\_text\_clipped**](#function-syn_imgui_text_clipped) ([**SYN\_IMGUI\_Context**](structSYN__IMGUI__Context.md) \* ctx, const char \* text, int16\_t x, int16\_t y, int16\_t clip\_x, int16\_t clip\_y, int16\_t clip\_w, int16\_t clip\_h) <br>_Draw text clipped to a bounding rectangle._  |
|  void | [**syn\_imgui\_text\_marquee**](#function-syn_imgui_text_marquee) ([**SYN\_IMGUI\_Context**](structSYN__IMGUI__Context.md) \* ctx, const char \* text, int16\_t \* offset, int16\_t x, int16\_t y, int16\_t w, int16\_t speed) <br>_Scrolling text label with pause at each end._  |
|  void | [**syn\_imgui\_text\_wrapped**](#function-syn_imgui_text_wrapped) ([**SYN\_IMGUI\_Context**](structSYN__IMGUI__Context.md) \* ctx, const char \* text, int16\_t x, int16\_t y, int16\_t w) <br>_Render word-wrapped text within a width._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_imgui\_toggle**](#function-syn_imgui_toggle) ([**SYN\_IMGUI\_Context**](structSYN__IMGUI__Context.md) \* ctx, const char \* label, [**bool**](syn__defs_8h.md#enum-bool) \* state, int16\_t x, int16\_t y, int16\_t w, int16\_t h) <br>_ON/OFF toggle switch._  |
|  void | [**syn\_imgui\_value\_int**](#function-syn_imgui_value_int) ([**SYN\_IMGUI\_Context**](structSYN__IMGUI__Context.md) \* ctx, const char \* label, int32\_t value, int16\_t x, int16\_t y) <br>_Draw a non-interactive "label: value" pair._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**syn\_imgui\_widget\_visible**](#function-syn_imgui_widget_visible) (const [**SYN\_IMGUI\_Context**](structSYN__IMGUI__Context.md) \* ctx, int16\_t y, int16\_t h) <br>_Check if a widget at (y, h) is visible in the scroll viewport._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  void | [**draw\_semi\_circle**](#function-draw_semi_circle) ([**SYN\_GfxContext**](syn__gfx_8h.md#typedef-syn_gfxcontext) c, int16\_t cx, int16\_t cy, int16\_t r, uint16\_t color) <br>_Draw a semicircle (top half) using Bresenham's algorithm._  |
|  [**bool**](syn__defs_8h.md#enum-bool) | [**is\_hit\_test**](#function-is_hit_test) (const [**SYN\_IMGUI\_Context**](structSYN__IMGUI__Context.md) \* ctx, int16\_t x, int16\_t y, int16\_t w, int16\_t h) <br>_Test if touch coordinates fall within a rectangle._  |
|  void | [**layout\_resolve**](#function-layout_resolve) ([**SYN\_IMGUI\_Context**](structSYN__IMGUI__Context.md) \* ctx, int16\_t \* x, int16\_t \* y, int16\_t \* w, int16\_t \* h, int16\_t default\_h) <br>_Resolve widget geometry from the layout cursor._  |
|  void | [**track\_focus**](#function-track_focus) ([**SYN\_IMGUI\_Context**](structSYN__IMGUI__Context.md) \* ctx) <br>_Track whether the focused widget was visited this frame._  |


























## Public Functions Documentation




### function syn\_imgui\_bar\_chart 

_Vertical bar chart._ 
```C++
void syn_imgui_bar_chart (
    SYN_IMGUI_Context * ctx,
    const char * title,
    const int32_t * data,
    size_t count,
    int32_t min_val,
    int32_t max_val,
    int16_t x,
    int16_t y,
    int16_t w,
    int16_t h
) 
```





**Parameters:**


* `ctx` Context. 
* `title` Chart title (drawn top-left). 
* `data` Data values. 
* `count` Number of values. 
* `min_val` Minimum axis value. 
* `max_val` Maximum axis value. 
* `x` Bounds. 




        

<hr>



### function syn\_imgui\_begin 

_Begin a new IMGUI frame._ 
```C++
void syn_imgui_begin (
    SYN_IMGUI_Context * ctx,
    SYN_GfxContext gfx,
    bool select,
    bool back,
    int32_t enc_delta,
    bool touch_down,
    int16_t touch_x,
    int16_t touch_y
) 
```



Resets the transient ID counter, processes physical navigation inputs, and resets active states.




**Parameters:**


* `ctx` Context. 
* `gfx` Graphics context to draw on. 
* `select` True if select button was pressed this frame. 
* `back` True if back button was pressed this frame. 
* `enc_delta` Encoder rotation value for this frame. 
* `touch_down` True if touch screen is active. 
* `touch_x` Touch X coordinate. 
* `touch_y` Touch Y coordinate. 




        

<hr>



### function syn\_imgui\_begin\_disabled 

_Begin a disabled region. Widgets draw dimmed and skip input._ 
```C++
void syn_imgui_begin_disabled (
    SYN_IMGUI_Context * ctx
) 
```



Calls may be nested; the region is re-enabled only when every begin\_disabled has a matching end\_disabled.




**Parameters:**


* `ctx` Context. 




        

<hr>



### function syn\_imgui\_button 

_Draw and handle a button._ 
```C++
bool syn_imgui_button (
    SYN_IMGUI_Context * ctx,
    const char * label,
    int16_t x,
    int16_t y,
    int16_t w,
    int16_t h
) 
```





**Parameters:**


* `ctx` Context. 
* `label` Button text. 
* `x` X coordinate. 
* `y` Y coordinate. 
* `w` Width. 
* `h` Height. 



**Returns:**

true if the button was selected/clicked this frame. 





        

<hr>



### function syn\_imgui\_checkbox 

_Draw and handle a checkbox._ 
```C++
bool syn_imgui_checkbox (
    SYN_IMGUI_Context * ctx,
    const char * label,
    bool * checked,
    int16_t x,
    int16_t y,
    int16_t w,
    int16_t h
) 
```





**Parameters:**


* `ctx` Context. 
* `label` Checkbox label. 
* `checked` Pointer to boolean state. 
* `x` X coordinate. 
* `y` Y coordinate. 
* `w` Width. 
* `h` Height. 



**Returns:**

true if the checkbox state was toggled this frame. 





        

<hr>



### function syn\_imgui\_checkbox\_flags 

_Checkbox that toggles a bitmask in a flags word._ 
```C++
bool syn_imgui_checkbox_flags (
    SYN_IMGUI_Context * ctx,
    const char * label,
    uint32_t * flags,
    uint32_t mask,
    int16_t x,
    int16_t y,
    int16_t w,
    int16_t h
) 
```





**Parameters:**


* `ctx` Context. 
* `label` Display label. 
* `flags` Pointer to the flags word (modified in place). 
* `mask` Bitmask to toggle. 
* `x` Bounds (0 = auto-layout). 



**Returns:**

true if the value changed this frame. 





        

<hr>



### function syn\_imgui\_collapsing\_header 

_Collapsible section header._ 
```C++
bool syn_imgui_collapsing_header (
    SYN_IMGUI_Context * ctx,
    const char * label,
    bool * expanded,
    int16_t x,
    int16_t y,
    int16_t w,
    int16_t h
) 
```



Renders "+ Label" (collapsed) or "- Label" (expanded). Toggles \*expanded on click.




**Parameters:**


* `ctx` Context. 
* `label` Section title. 
* `expanded` Pointer to expansion state (toggled on click). 
* `x` Bounds (0 = auto-layout). 



**Returns:**

true if the expanded state changed this frame. 





        

<hr>



### function syn\_imgui\_combo 

_Renders and handles an inline option selection dropdown/selector._ 
```C++
bool syn_imgui_combo (
    SYN_IMGUI_Context * ctx,
    const char * label,
    const char ** options,
    size_t count,
    int32_t * selected,
    int16_t x,
    int16_t y,
    int16_t w,
    int16_t h
) 
```





**Parameters:**


* `ctx` Context. 
* `label` Selector label. 
* `options` Array of option string labels. 
* `count` Number of option strings in the array. 
* `selected` Pointer to active selected index. 
* `x` X coordinate. 
* `y` Y coordinate. 
* `w` Width. 
* `h` Height. 



**Returns:**

true if selection changed this frame. 





        

<hr>



### function syn\_imgui\_default\_style 

_Return the default monochrome style (white-on-black)._ 
```C++
SYN_IMGUI_Style syn_imgui_default_style (
    void
) 
```



The returned struct may be modified and passed to [**syn\_imgui\_set\_style()**](syn__imgui_8h.md#function-syn_imgui_set_style).




**Returns:**

Default style struct. 





        

<hr>



### function syn\_imgui\_dialog 

_Renders a modal confirmation dialog overlay._ 
```C++
bool syn_imgui_dialog (
    SYN_IMGUI_Context * ctx,
    const char * message,
    bool * ok_clicked,
    int16_t x,
    int16_t y,
    int16_t w,
    int16_t h
) 
```



Captures all navigation and touch focus until dismissed.




**Parameters:**


* `ctx` Context. 
* `message` Display text. 
* `ok_clicked` Pointer to check trigger results (updated when OK is clicked). 
* `x` X coordinate. 
* `y` Y coordinate. 
* `w` Width. 
* `h` Height. 



**Returns:**

true if the modal dialog has been dismissed (OK or CANCEL selected). 





        

<hr>



### function syn\_imgui\_end 

_End the IMGUI frame._ 
```C++
void syn_imgui_end (
    SYN_IMGUI_Context * ctx
) 
```



Finalizes widget count tracking.




**Parameters:**


* `ctx` Context. 




        

<hr>



### function syn\_imgui\_end\_disabled 

_End a disabled region._ 
```C++
void syn_imgui_end_disabled (
    SYN_IMGUI_Context * ctx
) 
```





**Parameters:**


* `ctx` Context. 




        

<hr>



### function syn\_imgui\_gauge 

_Renders an analog meter gauge with a needle indicator._ 
```C++
void syn_imgui_gauge (
    SYN_IMGUI_Context * ctx,
    const char * label,
    int32_t value,
    int32_t min,
    int32_t max,
    int16_t cx,
    int16_t cy,
    int16_t radius
) 
```



Uses lightweight fixed-point integer mathematics for vector coordinates.




**Parameters:**


* `ctx` Context. 
* `label` Gauge label. 
* `value` Current value to point needle at. 
* `min` Minimum bounds. 
* `max` Maximum bounds. 
* `cx` Center X coordinate. 
* `cy` Center Y coordinate. 
* `radius` Radius of gauge boundary. 




        

<hr>



### function syn\_imgui\_graph 

_Renders a line telemetry chart._ 
```C++
void syn_imgui_graph (
    SYN_IMGUI_Context * ctx,
    const char * title,
    const int32_t * data,
    size_t count,
    int32_t min_val,
    int32_t max_val,
    int16_t x,
    int16_t y,
    int16_t w,
    int16_t h
) 
```





**Parameters:**


* `ctx` Context. 
* `title` Chart title label. 
* `data` Pointer to array of data points. 
* `count` Number of data points. 
* `min_val` Minimum scale limit. 
* `max_val` Maximum scale limit. 
* `x` X coordinate. 
* `y` Y coordinate. 
* `w` Width. 
* `h` Height. 




        

<hr>



### function syn\_imgui\_group\_begin 

_Begin a visual group box with an optional title._ 
```C++
void syn_imgui_group_begin (
    SYN_IMGUI_Context * ctx,
    const char * title,
    int16_t x,
    int16_t y,
    int16_t w,
    int16_t h
) 
```





**Parameters:**


* `ctx` Context. 
* `title` Title text (NULL for no title). 
* `x` Bounds. 




        

<hr>



### function syn\_imgui\_group\_end 

_End the group box._ 
```C++
void syn_imgui_group_end (
    SYN_IMGUI_Context * ctx
) 
```





**Parameters:**


* `ctx` Context. 




        

<hr>



### function syn\_imgui\_icon\_button 

_Button that displays a monochrome icon bitmap._ 
```C++
bool syn_imgui_icon_button (
    SYN_IMGUI_Context * ctx,
    const uint8_t * icon,
    int16_t icon_w,
    int16_t icon_h,
    int16_t x,
    int16_t y,
    int16_t w,
    int16_t h
) 
```





**Parameters:**


* `ctx` Context. 
* `icon` Bitmap data (1bpp, row-major, MSB first). 
* `icon_w` Icon dimensions in pixels. 
* `x` Button bounds (0 = auto-layout). 



**Returns:**

true if clicked this frame. 





        

<hr>



### function syn\_imgui\_init 

_Initialize the IMGUI context._ 
```C++
void syn_imgui_init (
    SYN_IMGUI_Context * ctx
) 
```





**Parameters:**


* `ctx` Context to initialize. 




        

<hr>



### function syn\_imgui\_label 

_Draw a static text label (no navigation ID, no interaction)._ 
```C++
void syn_imgui_label (
    SYN_IMGUI_Context * ctx,
    const char * text,
    int16_t x,
    int16_t y
) 
```



Uses style.fg for color. When inside a layout region with x=0, y=0, the label is drawn at the current cursor and the cursor advances.




**Parameters:**


* `ctx` Context. 
* `text` Null-terminated string. 
* `x` X coordinate (0 = use layout cursor). 
* `y` Y coordinate (0 = use layout cursor). 




        

<hr>



### function syn\_imgui\_label\_centered 

_Draw a centered label within a given width._ 
```C++
void syn_imgui_label_centered (
    SYN_IMGUI_Context * ctx,
    const char * text,
    int16_t x,
    int16_t y,
    int16_t w
) 
```





**Parameters:**


* `ctx` Context. 
* `text` Null-terminated string. 
* `x` Left edge and vertical position. 
* `w` Width to center within. 




        

<hr>



### function syn\_imgui\_label\_colored 

_Draw a label in an arbitrary color._ 
```C++
void syn_imgui_label_colored (
    SYN_IMGUI_Context * ctx,
    const char * text,
    uint16_t color,
    int16_t x,
    int16_t y
) 
```





**Parameters:**


* `ctx` Context. 
* `text` Null-terminated string. 
* `color` 16-bit color value. 
* `x` Position. 




        

<hr>



### function syn\_imgui\_label\_right 

_Draw a right-aligned label within a given width._ 
```C++
void syn_imgui_label_right (
    SYN_IMGUI_Context * ctx,
    const char * text,
    int16_t x,
    int16_t y,
    int16_t w
) 
```





**Parameters:**


* `ctx` Context. 
* `text` Null-terminated string. 
* `x` Left edge and vertical position. 
* `w` Width to right-align within. 




        

<hr>



### function syn\_imgui\_layout\_begin 

_Begin an auto-layout region._ 
```C++
void syn_imgui_layout_begin (
    SYN_IMGUI_Context * ctx,
    int16_t x,
    int16_t y,
    int16_t w
) 
```



Widgets within this region may pass x=0, y=0, w=0, h=0 to use the layout cursor. Widgets that use the cursor advance it down by (widget\_height + style.spacing) after each call.




**Parameters:**


* `ctx` Context. 
* `x` Left edge of the layout region. 
* `y` Top edge. 
* `w` Default widget width (used when widget's w parameter is 0). 




        

<hr>



### function syn\_imgui\_layout\_end 

_End the auto-layout region._ 
```C++
void syn_imgui_layout_end (
    SYN_IMGUI_Context * ctx
) 
```





**Parameters:**


* `ctx` Context. 




        

<hr>



### function syn\_imgui\_layout\_row 

_Define a multi-column layout row._ 
```C++
void syn_imgui_layout_row (
    SYN_IMGUI_Context * ctx,
    int items,
    const int16_t * widths,
    int16_t height
) 
```



Subsequent auto-layout widgets fill columns left-to-right. Positive width = fixed pixels. Negative width = fill remainder.




**Parameters:**


* `ctx` Context (must be inside a layout region). 
* `items` Number of columns (max SYN\_IMGUI\_MAX\_ROW\_COLS). 
* `widths` Array of column widths (NULL for equal split). 
* `height` Row height (0 = auto). 




        

<hr>



### function syn\_imgui\_progress\_bar 

_Renders a non-interactive progress bar._ 
```C++
void syn_imgui_progress_bar (
    SYN_IMGUI_Context * ctx,
    int32_t value,
    int32_t min,
    int32_t max,
    int16_t x,
    int16_t y,
    int16_t w,
    int16_t h
) 
```





**Parameters:**


* `ctx` Context. 
* `value` Current status value. 
* `min` Minimum possible value. 
* `max` Maximum possible value. 
* `x` X coordinate. 
* `y` Y coordinate. 
* `w` Width. 
* `h` Height. 




        

<hr>



### function syn\_imgui\_progress\_bar\_ex 

_Enhanced progress bar with overlay text and indeterminate mode._ 
```C++
void syn_imgui_progress_bar_ex (
    SYN_IMGUI_Context * ctx,
    int32_t value,
    int32_t min,
    int32_t max,
    const char * overlay,
    int16_t x,
    int16_t y,
    int16_t w,
    int16_t h
) 
```





**Parameters:**


* `ctx` Context. 
* `value` Current value. If value &lt; min, renders indeterminate animation. 
* `min` Value range. 
* `overlay` Overlay text (NULL = auto "XX%", "" = no text). 
* `x` Bounds. 




        

<hr>



### function syn\_imgui\_radio 

_Draw and handle a radio button option._ 
```C++
bool syn_imgui_radio (
    SYN_IMGUI_Context * ctx,
    const char * label,
    int32_t * selection,
    int32_t button_val,
    int16_t x,
    int16_t y,
    int16_t w,
    int16_t h
) 
```



Sets the selection variable to the target button value if clicked or touched.




**Parameters:**


* `ctx` Context. 
* `label` Radio button label. 
* `selection` Pointer to active selection variable. 
* `button_val` Specific option value this button represents. 
* `x` X coordinate. 
* `y` Y coordinate. 
* `w` Width. 
* `h` Height. 



**Returns:**

true if the selection changed to this button's value this frame. 





        

<hr>



### function syn\_imgui\_same\_line 

_Place the next widget on the same row as the previous one._ 
```C++
void syn_imgui_same_line (
    SYN_IMGUI_Context * ctx
) 
```



Call immediately before the next widget. The next widget's x is set to (previous widget right edge + style.spacing).




**Parameters:**


* `ctx` Context. 




        

<hr>



### function syn\_imgui\_scroll\_begin 

_Begin a scrollable region._ 
```C++
void syn_imgui_scroll_begin (
    SYN_IMGUI_Context * ctx,
    int16_t x,
    int16_t y,
    int16_t w,
    int16_t h,
    int16_t * scroll
) 
```



Widgets between scroll\_begin and scroll\_end are clipped to the viewport and offset by the scroll position. The scroll offset is auto-adjusted to keep the focused widget visible.




**Parameters:**


* `ctx` Context. 
* `x` Viewport left edge. 
* `y` Viewport top edge. 
* `w` Viewport width. 
* `h` Viewport height. 
* `scroll` Pointer to user-owned scroll offset (persists between frames). 




        

<hr>



### function syn\_imgui\_scroll\_end 

_End the scrollable region and draw scroll indicator._ 
```C++
void syn_imgui_scroll_end (
    SYN_IMGUI_Context * ctx
) 
```





**Parameters:**


* `ctx` Context. 




        

<hr>



### function syn\_imgui\_selectable 

_Full-width clickable row (like a borderless toggleable button)._ 
```C++
bool syn_imgui_selectable (
    SYN_IMGUI_Context * ctx,
    const char * label,
    bool * selected,
    int16_t x,
    int16_t y,
    int16_t w,
    int16_t h
) 
```





**Parameters:**


* `ctx` Context. 
* `label` Display text. 
* `selected` Pointer to selection state (toggled on click). 
* `x` Bounds (0 = auto-layout). 



**Returns:**

true if clicked this frame. 





        

<hr>



### function syn\_imgui\_separator 

_Draw a horizontal separator line._ 
```C++
void syn_imgui_separator (
    SYN_IMGUI_Context * ctx,
    int16_t x,
    int16_t y,
    int16_t w
) 
```



When inside a layout region, the line spans the full layout width and the cursor advances by 1 + style.spacing.




**Parameters:**


* `ctx` Context. 
* `x` X start (0 = use layout cursor). 
* `y` Y position (0 = use layout cursor). 
* `w` Width in pixels (0 = use layout width). 




        

<hr>



### function syn\_imgui\_separator\_text 

_Draw a labeled separator: ── Title ──────────_ 
```C++
void syn_imgui_separator_text (
    SYN_IMGUI_Context * ctx,
    const char * text,
    int16_t x,
    int16_t y,
    int16_t w
) 
```





**Parameters:**


* `ctx` Context. 
* `text` Section title text. 
* `x` X coordinate. 
* `y` Y coordinate. 
* `w` Total width. 




        

<hr>



### function syn\_imgui\_set\_style 

_Override the active style._ 
```C++
void syn_imgui_set_style (
    SYN_IMGUI_Context * ctx,
    const SYN_IMGUI_Style * style
) 
```





**Parameters:**


* `ctx` Context. 
* `style` Style to copy into the context. 




        

<hr>



### function syn\_imgui\_slider 

_Draw and handle an integer value slider._ 
```C++
bool syn_imgui_slider (
    SYN_IMGUI_Context * ctx,
    const char * label,
    int32_t * value,
    int32_t min,
    int32_t max,
    int16_t x,
    int16_t y,
    int16_t w,
    int16_t h
) 
```



When clicked or touched, this widget goes into "edit mode". While in edit mode, encoder rotation or direct touch drags change the value.




**Parameters:**


* `ctx` Context. 
* `label` Slider label. 
* `value` Pointer to integer value. 
* `min` Minimum allowed value. 
* `max` Maximum allowed value. 
* `x` X coordinate. 
* `y` Y coordinate. 
* `w` Width. 
* `h` Height. 



**Returns:**

true if the value was modified this frame. 





        

<hr>



### function syn\_imgui\_spacing 

_Insert vertical spacing (moves the layout cursor down)._ 
```C++
void syn_imgui_spacing (
    SYN_IMGUI_Context * ctx,
    int16_t pixels
) 
```





**Parameters:**


* `ctx` Context. 
* `pixels` Number of pixels to advance downward. 




        

<hr>



### function syn\_imgui\_spinner 

_Integer numeric spinner widget._ 
```C++
bool syn_imgui_spinner (
    SYN_IMGUI_Context * ctx,
    const char * label,
    int32_t * value,
    int32_t min,
    int32_t max,
    int32_t step,
    int16_t x,
    int16_t y,
    int16_t w,
    int16_t h
) 
```



Renders `Label [< value >]`. When focused, encoder delta increments the value by `step`. Returns true on the frame the value changes.




**Parameters:**


* `ctx` Context. 
* `label` Display label. 
* `value` Pointer to the current value (modified in place). 
* `min` Minimum value (clamped, wraps to max). 
* `max` Maximum value (clamped, wraps to min). 
* `step` Increment / decrement amount per encoder tick. 
* `x` X coordinate (0 = use layout cursor). 
* `y` Y coordinate (0 = use layout cursor). 
* `w` Width (0 = use layout width). 
* `h` Height (0 = font height + 2\*padding). 



**Returns:**

true if the value changed this frame. 





        

<hr>



### function syn\_imgui\_status\_bar 

_Draw a single-line status bar with a separator above it._ 
```C++
void syn_imgui_status_bar (
    SYN_IMGUI_Context * ctx,
    const char * text,
    int16_t x,
    int16_t y,
    int16_t w
) 
```





**Parameters:**


* `ctx` IMGUI context. 
* `text` Status text to display. 
* `x` X position. 
* `y` Y position. 
* `w` Width in pixels. 




        

<hr>



### function syn\_imgui\_tabs 

_Horizontal tab selector._ 
```C++
bool syn_imgui_tabs (
    SYN_IMGUI_Context * ctx,
    const char ** labels,
    size_t count,
    int32_t * active,
    int16_t x,
    int16_t y,
    int16_t w
) 
```



Renders a row of labeled tabs. Consumes one navigation ID. When focused, encoder cycles through tabs.




**Parameters:**


* `ctx` Context. 
* `labels` Array of tab label strings. 
* `count` Number of tabs. 
* `active` Pointer to active tab index. 
* `x` Position and total width. 



**Returns:**

true if the active tab changed. 





        

<hr>



### function syn\_imgui\_text\_clipped 

_Draw text clipped to a bounding rectangle._ 
```C++
void syn_imgui_text_clipped (
    SYN_IMGUI_Context * ctx,
    const char * text,
    int16_t x,
    int16_t y,
    int16_t clip_x,
    int16_t clip_y,
    int16_t clip_w,
    int16_t clip_h
) 
```



Uses the canvas clip rect to prevent text overflow.




**Parameters:**


* `ctx` Context. 
* `text` Null-terminated string. 
* `x` Text draw position. 
* `clip_x` Clipping rectangle. 




        

<hr>



### function syn\_imgui\_text\_marquee 

_Scrolling text label with pause at each end._ 
```C++
void syn_imgui_text_marquee (
    SYN_IMGUI_Context * ctx,
    const char * text,
    int16_t * offset,
    int16_t x,
    int16_t y,
    int16_t w,
    int16_t speed
) 
```



If the text fits within `w`, it's drawn statically. Otherwise it scrolls left, pauses, scrolls right, pauses, and repeats. The caller owns the scroll state (just declare `static int16_t off = 0;`).




**Parameters:**


* `ctx` Context. 
* `text` Null-terminated string. 
* `offset` Pointer to caller-owned scroll offset (auto-advanced). 
* `x` Top-left corner. 
* `w` Available width (clips text beyond this). 
* `speed` Pixels per frame (1–2 typical). 




        

<hr>



### function syn\_imgui\_text\_wrapped 

_Render word-wrapped text within a width._ 
```C++
void syn_imgui_text_wrapped (
    SYN_IMGUI_Context * ctx,
    const char * text,
    int16_t x,
    int16_t y,
    int16_t w
) 
```





**Parameters:**


* `ctx` Context. 
* `text` Null-terminated string. 
* `x` Top-left corner. 
* `w` Maximum width (wraps at word boundaries). 




        

<hr>



### function syn\_imgui\_toggle 

_ON/OFF toggle switch._ 
```C++
bool syn_imgui_toggle (
    SYN_IMGUI_Context * ctx,
    const char * label,
    bool * state,
    int16_t x,
    int16_t y,
    int16_t w,
    int16_t h
) 
```





**Parameters:**


* `ctx` Context. 
* `label` Label text. 
* `state` Pointer to boolean state. 
* `x` Bounds (0 = auto-layout). 



**Returns:**

true if state was toggled this frame. 





        

<hr>



### function syn\_imgui\_value\_int 

_Draw a non-interactive "label: value" pair._ 
```C++
void syn_imgui_value_int (
    SYN_IMGUI_Context * ctx,
    const char * label,
    int32_t value,
    int16_t x,
    int16_t y
) 
```





**Parameters:**


* `ctx` Context. 
* `label` Key text. 
* `value` Integer value. 
* `x` Position. 




        

<hr>



### function syn\_imgui\_widget\_visible 

_Check if a widget at (y, h) is visible in the scroll viewport._ 
```C++
bool syn_imgui_widget_visible (
    const SYN_IMGUI_Context * ctx,
    int16_t y,
    int16_t h
) 
```



Returns true if not in a scroll region or if any part is visible. Widgets that return false should still call next\_id++ for focus.




**Parameters:**


* `ctx` Context. 
* `y` Widget top edge. 
* `h` Widget height. 



**Returns:**

true if any part of the widget is visible (or no scroll active). 





        

<hr>
## Public Static Functions Documentation




### function draw\_semi\_circle 

_Draw a semicircle (top half) using Bresenham's algorithm._ 
```C++
static void draw_semi_circle (
    SYN_GfxContext c,
    int16_t cx,
    int16_t cy,
    int16_t r,
    uint16_t color
) 
```





**Parameters:**


* `c` Graphics context. 
* `cx` Center X. 
* `cy` Center Y. 
* `r` Radius. 
* `color` Pixel color. 




        

<hr>



### function is\_hit\_test 

_Test if touch coordinates fall within a rectangle._ 
```C++
static bool is_hit_test (
    const SYN_IMGUI_Context * ctx,
    int16_t x,
    int16_t y,
    int16_t w,
    int16_t h
) 
```





**Parameters:**


* `ctx` IMGUI context. 
* `x` Widget X. 
* `y` Widget Y. 
* `w` Widget width. 
* `h` Widget height. 



**Returns:**

true if touch is inside. 





        

<hr>



### function layout\_resolve 

_Resolve widget geometry from the layout cursor._ 
```C++
static void layout_resolve (
    SYN_IMGUI_Context * ctx,
    int16_t * x,
    int16_t * y,
    int16_t * w,
    int16_t * h,
    int16_t default_h
) 
```



If the widget is inside a layout region AND all four geometry params are 0, fill them from the cursor and default width/height.




**Parameters:**


* `ctx` IMGUI context. 
* `x` [in/out] Widget X. 
* `y` [in/out] Widget Y. 
* `w` [in/out] Widget width. 
* `h` [in/out] Widget height. 
* `default_h` Default height if not specified. 




        

<hr>



### function track\_focus 

_Track whether the focused widget was visited this frame._ 
```C++
static inline void track_focus (
    SYN_IMGUI_Context * ctx
) 
```





**Parameters:**


* `ctx` IMGUI context. 




        

<hr>

------------------------------
The documentation for this class was generated from the following file `src/syntropic/ui/syn_imgui.c`

