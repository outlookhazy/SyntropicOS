# Display & UI

## Display Canvas

| Module | Header | Config |
|---|---|---|
| Canvas | `display/syn_canvas.h` | `SYN_USE_CANVAS` |
| Graphics | `display/syn_gfx.h` | `SYN_USE_CANVAS` |

Hardware-independent framebuffer canvas supporting 1bpp (monochrome) and 16bpp (RGB565) pixel formats. Drawing primitives include:

- Lines, rectangles, rounded rectangles
- Circles and filled circles
- Bitmap blitting
- Text rendering (built-in font)

The graphics backend is configurable:

- `SYN_GFX_BACKEND_CANVAS` (default) — renders to a framebuffer that you flush to your display
- `SYN_GFX_BACKEND_DIRECT` — direct-draw mode with no framebuffer (lower RAM, higher bus traffic)

## Immediate-Mode GUI

| Module | Header | Config |
|---|---|---|
| IMGUI | `ui/syn_imgui.h` | `SYN_USE_IMGUI` |

A zero-allocation immediate-mode GUI framework. Widgets are drawn and input-tested in a single pass each frame — no widget tree, no heap.

**Widget types:** buttons, sliders, checkboxes, progress bars, gauges, real-time graphs, text labels, scrolling marquee text, and modal dialog overlays.

**Input sources:** button/encoder navigation, touchscreen coordinates.

## Menu

| Module | Header | Config |
|---|---|---|
| Menu | `ui/syn_menu.h` | `SYN_USE_MENU` |

Hierarchical static list menu layout for encoder/button-driven interfaces.
