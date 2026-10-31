# Glimmer

Glimmer is a header-only, dependency-free C++17 immediate-mode GUI library.
It draws into a CPU RGBA pixel buffer and has no GPU or windowing dependency.
Pixels are packed as `0xAABBGGRR` (red in the least-significant byte).

## Quick start

Include `glimmer/glimmer.hpp` and link to the `glimmer` CMake interface target:

```cpp
#include "glimmer/glimmer.hpp"

int main() {
    glm::Context ui(800, 600);
    float volume = 0.5f;
    bool muted = false;

    ui.begin(0, 0, false);
    ui.window("Settings", {20, 20, 300, 240}, [&] {
        ui.label("Volume");
        ui.slider("vol", volume, 0.0f, 1.0f);
        ui.checkbox("Mute", muted);
        if (ui.button("Reset")) {
            volume = 0.5f;
        }
    });
    ui.end();
    const std::uint32_t* pixels = ui.pixels();
    (void)pixels;
}
```

Configure, build, and run tests with:

```text
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

For a multi-configuration generator such as Visual Studio, select the
configuration explicitly, for example `ctest --test-dir build -C Debug
--output-on-failure`.

`settings_panel` is an example executable that writes a PPM image named
`settings.ppm` to its current working directory.

On Windows, the optional `glimmer_live_demo` executable opens an interactive
native window. It displays a dockable CPU-rendered workspace with Controls,
Editor, Visuals, and Profiler panels. Drag a tab to the other pane to move it;
drag the divider to resize the panes. The workspace demonstrates
draggable windows, themed toggles and radio controls, progress indicators,
collapsing headers, combo popups, single- and multi-line text editing,
scrollable lists and child regions, image blitting and tinting, outlined circles
and dashed lines, filled triangles and polygons, polygon outlines, open
polylines and cubic Bézier curves, a two-handle range slider,
discrete ratings, a rotary knob, fine-grained drag values, a two-dimensional
pad, sample sparkline, histogram, pie, and donut charts, delayed tooltips, horizontal,
vertical, and four-corner RGBA gradients, plus a checkerboard transparency
preview. Use the mouse wheel over either scrolling region, and press Escape to
close an open combo popup. It also includes a caller-editable RGBA color
swatch. This example uses only the Windows API for its window and display
adapter; the Glimmer library remains independent of windowing systems. The
profiler view visualizes caller-supplied CPU timing intervals. The demo uses a
fixed sample trace for deterministic output; applications can supply measured
CPU timings. Glimmer does not measure or claim GPU timings.

Build and run the live demo from the project directory with:

```powershell
cmake -B build
cmake --build build --config Release --target glimmer_live_demo
.\build\Release\glimmer_live_demo.exe
```

In Visual Studio, use `Debug` instead of `Release` in the executable path to
launch the debug build.

## API reference

### Core concepts and terminology

| Term | Definition |
|---|---|
| Immediate-mode GUI | A user interface declared again, in order, on every frame. The application owns widget values; Glimmer draws their current values and reports input results. |
| Frame | One complete update-and-render pass: call `Context::begin`, declare the windows and widgets, then call `Context::end`. |
| Render target | The CPU-owned array of pixels that drawing updates. A `Surface` is a render target; `Context` owns one. |
| Pixel | One packed `std::uint32_t` color. Its numeric layout is `0xAABBGGRR`: bits 0–7 red, 8–15 green, 16–23 blue, and 24–31 alpha. |
| Source-over | Alpha compositing: a new drawing color (the source) is blended over the existing destination pixel rather than replacing it unconditionally. |
| Surface coordinates | Coordinates measured from the render target's upper-left corner: x grows to the right and y grows downward. |
| Rectangle (`Rect`) | `{x, y, w, h}`: upper-left position and width/height. Rectangles describe drawing geometry, clips, windows, and layout allocations. |
| Clip rectangle | A drawing boundary. Pixels outside the intersection of the active clip stack and the surface are not changed. |
| Layout cursor | The current position used by `Context` to allocate rows, columns, and widgets. Window contents normally advance vertically. |
| Widget ID | A stable identity used to retain state such as focus, pressed state, popup state, and scroll position between frames. |
| Hot | The widget currently under the pointer. |
| Active | The widget that owns an in-progress pointer action, usually a held press or drag. |
| Focused | The widget receiving keyboard navigation or activation. Tab and Shift+Tab move focus through controls declared during a frame. |
| Even-odd fill rule | A point is inside a polygon when a ray from it crosses the polygon boundary an odd number of times. This also defines fills for concave and self-intersecting contours. |

### Public data types

- **`glm::Rect`** has four `float` members: `x`, `y`, `w`, and `h`. The first
  pair gives the upper-left location and the second pair the dimensions.
- **`glm::Surface`** owns a pixel buffer and implements drawing primitives.
  Construction clamps negative width or height to zero. `width()` and
  `height()` report effective dimensions. `pixels()` has const and writable
  overloads; each points to `width() * height()` row-major pixels.
- **`glm::Context`** owns a `Surface`, theme, layout cursor, and UI interaction
  state. It is the usual API for widgets and host-forwarded input.
- **`glm::Theme`** contains packed colors named `background`, `window`,
  `title_bar`, `text`, `accent`, `button`, `button_hover`, `button_active`,
  and `border`. `padding` sets content spacing; `font_scale` sets integer text
  scale; `tooltip_delay_frames` sets the deterministic hover delay before a
  tooltip is shown. Default member values make a dark theme.
- **`glm::WindowState`** stores a draggable window's `rect`, `dragging` flag,
  and pointer offsets `drag_offset_x` and `drag_offset_y`. Keep the same
  caller-owned state object between frames.
- **`glm::DockSide`**, **`glm::DockPanel`**, and **`glm::DockspaceState`**
  define a two-group dock workspace. A panel has a stable `id`, visible
  `title`, and initial left/right side. `DockspaceState` owns the caller's
  panel assignments, active tabs, and left-pane ratio; preserve it between
  frames for layout persistence.
- **`glm::ProfileEvent`** stores a named CPU interval (`name`, `start_ms`,
  `duration_ms`) measured from the beginning of a frame. The application
  supplies timings; the library only visualizes them.
- **`glm::Key`** is the platform-independent key vocabulary for `feed_key`:
  `Backspace`, `Delete`, `Left`, `Right`, `Home`, `End`, `Enter`, `Tab`,
  `Space`, `Up`, `Down`, and `Escape`. The host translates native key events.
- **`glm::Interaction`** describes a widget's current-frame response.
  `hovered` means the pointer is over it; `active` means it owns an ongoing
  pointer action; `clicked` means it activated this frame; `changed` means a
  caller-owned value changed. Its explicit `bool` conversion is equivalent to
  `clicked`.

### Drawing and text definitions

All `Surface` drawing uses surface coordinates, respects the current clip, and
alpha blends covered pixels using source-over compositing.

- **Clipping:** `push_clip(rect)` intersects a new clip with the previous clip;
  `pop_clip()` restores the previous clip. The base clip is the surface bounds
  and cannot be removed.
- **Rectangles:** `fill_rect` paints the interior. `outline_rect` draws an
  inward border of the requested thickness. `draw_rounded_rect` fills rounded
  corners, with radius limited by half the shorter dimension;
  `draw_rounded_rect_outline` draws the rounded border.
- **Lines:** `draw_line` uses an integer Bresenham path;
  `draw_dashed_line` alternates dash and gap lengths along that path.
- **Circles and ellipses:** `draw_filled_circle` and `draw_circle_outline`
  use integer center and radius. Ellipse fill and outline use the supplied
  bounding `Rect` and test pixel centers.
- **Triangles:** `draw_filled_triangle` fills three vertices;
  `draw_triangle_outline` strokes the three edges. A degenerate fill does
  nothing.
- **Polygon paths:** `draw_filled_polygon` accepts interleaved `x, y` float
  coordinates and fills using the even-odd rule; its last point connects to
  the first. `draw_polygon_outline` strokes a closed path, while
  `draw_polyline` leaves the last point unconnected. Stroke joins blend a
  covered pixel only once.
- **Cubic Bézier:** `draw_cubic_bezier` takes two endpoints and two control
  points, samples a fixed number of segments, and strokes the resulting open
  polyline. Its default thickness is one pixel and default segment count is
  32; a zero segment count draws nothing. See the
  [Bézier example](#drawing-cubic-bézier-curves).
- **Pie chart:** `Context::pie_chart` normalizes positive finite values into
  clockwise slices starting at twelve o'clock. Palette selection follows the
  original value index and repeats when needed; invalid values are skipped.
- **Donut chart:** `Context::donut_chart` uses the same slice and palette rules
  as the pie chart, but leaves a centered hole with half the outer radius.
- **Scatter plot:** `Context::scatter_plot` maps each finite point in the
  supplied x/y ranges to a small accent-colored marker. Points outside either
  range are omitted; reversed ranges are accepted, and a zero-width range maps
  matching coordinates to the center of that axis.
- **Profiler timeline:** `Context::profiler_timeline` draws caller-provided
  named intervals against a frame duration, with timing rows, a frame-budget
  marker, and a clipped timeline area. It is visualization, not instrumentation:
  the host measures and supplies CPU timing data.
- **Gradients and patterns:** horizontal and vertical gradients interpolate
  between their endpoint colors; bilinear gradients interpolate all four
  corners. `draw_checkerboard` paints alternating square cells from the
  rectangle origin. Gradients interpolate alpha as well as color channels.
- **Images:** `draw_image` blits a caller-owned packed buffer using
  nearest-neighbor sampling. `draw_tinted_image` multiplies each packed source
  channel by the corresponding tint channel before drawing; neither modifies
  the source.
- **Bitmap text:** `draw_text` uses the embedded 8-by-16 ASCII font. Newlines
  begin new lines, and unsupported bytes render as a question mark.
  `measure_text` returns pixel dimensions in a `Rect`; `measure_text_wrapped`
  returns dimensions after wrapping to the requested width.

### Frame, input, and retained state

`Context(width, height, theme = {})` creates a transparent render target and
applies the supplied theme. The host owns the event loop: each frame, call
`begin(mouse_x, mouse_y, mouse_down)`, declare the same controls in the desired
order, then call `end()`. `begin` clears the previous frame and paints the
background; `end` finishes deferred tooltips and popups. Read the rendered
buffer with `pixels()`, its dimensions with `width()`/`height()`, and the active
settings with `theme()`.

Send host input to the context explicitly:

- `feed_char(character)` inserts a character in the focused editor.
- `feed_key(key, shift = false)` sends a `Key` action. Tab moves focus;
  Shift+Tab moves backwards. Enter/Space activate focused controls where
  applicable. Arrow and Home/End actions navigate or adjust controls;
  Backspace/Delete edit text; Escape closes a popup.
- `feed_wheel(delta)` forwards a wheel delta in 120-unit notches. Partial
  notches accumulate; positive values scroll toward earlier list entries or
  upward in child regions.
- `cursor()` reports the active text cursor position.
- `hot_id()`, `active_id()`, and `focused_id()` return the corresponding
  current IDs; zero means there is no corresponding widget.
- `mouse_down()` returns the latest pointer state supplied to `begin`.
- `child_scroll_offset(id)` reads a child region's vertical offset in the
  current ID scope, or zero when that region has no saved scroll state.

Widget state supplied by reference (for example `bool&`, `float&`, or
`std::string&`) remains owned by the application. Retained interaction state
uses the control's hashed label or explicit ID, so IDs should be stable across
frames and unique among sibling controls. Windows, open tree nodes, and tables
introduce nested ID scopes.

### Widget behavior glossary

The following describes the role of each widget. The API list below gives
signatures and additional navigation details.

- **Label:** read-only text. `label_wrapped` wraps to available content width
  and returns its allocated height.
- **Button:** momentary action, indicated by `Interaction::clicked`; its
  overload with `enabled == false` excludes it from pointer and keyboard input.
- **Checkbox, toggle, selectable:** caller-owned boolean controls. Checkbox is
  a check mark, toggle is a switch presentation, and selectable is a
  highlighted row.
- **Slider family:** `slider` edits one bounded float horizontally;
  `vertical_slider` edits it vertically; `range_slider` edits a lower/upper
  pair without allowing the values to cross.
- **Numeric controls:** `stepper` adjusts a bounded integer in increments;
  `drag_value` maps horizontal pointer movement to a float change;
  `knob` maps pointer angle to a bounded float; `xy_pad` edits two bounded
  values in a plane.
- **Choices:** `radio` selects one option, `tab_bar` selects a page,
  `combo` selects from a popup, `color_palette` selects a swatch, and
  `rating` selects a discrete value.
- **Color and status:** `color_picker` edits RGBA channels;
  `progress_bar` visualizes a clamped fraction; `sparkline`, `histogram`, and
  `scatter_plot`, `pie_chart`, and `donut_chart` visualize sample data without
  interaction.
- **Disclosure and help:** `collapsing_header` toggles a body;
  `begin_tree_node`/`end_tree_node` define nested content and ID scope;
  `tooltip` requests delayed hover help; `separator` draws a divider.
- **Text editing:** `text_input` edits a single-line caller-owned string;
  `multiline_text_input` edits line breaks in a vertically scrollable field.
  Both expose cursor position through `cursor()`.
- **Image control:** `image_button` is a focusable button that aspect-fits its
  source image. `image` and `tinted_image` simply draw images and do not
  interact.

### Layout, windows, and grouped content

- `window(title, rect, callback)` draws a titled panel and invokes the callback
  with a vertical content layout. The `WindowState` overload additionally
  allows title-bar dragging and keeps position in caller-owned state.
- `row(height)` allocates a full-content-width rectangle and advances the
  vertical cursor. `column(width)` allocates a column and advances the
  horizontal cursor. `next_row(height)` advances vertically without returning
  a rectangle.
- `begin_table(id, columns)` divides content into evenly sized columns.
  `next_table_column()` advances cells, `next_table_row()` starts a row after
  the tallest cell, and `end_table()` restores the outer layout. Tables do not
  add borders or headings.
- `begin_child(id, rect)` starts a clipped scroll region in surface
  coordinates. Call `end_child()` after drawing its contents to restore the
  parent clip and layout. Wheel and keyboard scrolling target the deepest
  applicable child region.
- `list` draws a clipped list of options; `scroll` adjusts its caller-owned
  row offset. `feed_wheel` routes wheel input to the list or child under the
  pointer.
- `begin_dockspace(state, rect)`, `dock_panel(id, callback)`, and
  `end_dockspace()` render a persistent two-group docking workspace. Declare
  all registered panels every frame using `dock_panel`; only each group's
  active tab callback runs. Drag a tab to the other pane to re-dock it, and
  drag the central splitter to resize the panes. Set `DockPanel::side` for the
  initial group and preserve `DockspaceState` between frames.

```cpp
glm::DockspaceState layout;
layout.panels = {
    {"scene", "Scene", glm::DockSide::Left},
    {"inspector", "Inspector", glm::DockSide::Right},
};

ui.begin_dockspace(layout, {8, 8, 784, 584});
ui.dock_panel("scene", [&] { ui.label("Scene contents"); });
ui.dock_panel("inspector", [&] { ui.label("Inspector contents"); });
ui.end_dockspace();
```

Feed timing data to the profiler from the host application:

```cpp
const std::vector<glm::ProfileEvent> events{
    {"Update", 0.0f, 1.2f},
    {"Raster", 1.2f, 4.8f},
};
ui.profiler_timeline(events, 16.67f, 16.67f, 120.0f);
```
- Context `image` (from either a caller-owned pixel buffer or another
  `Surface`) and `tinted_image` draw at their explicit surface-coordinate
  rectangle and do not allocate layout space. Context `gradient`,
  `vertical_gradient`, `bilinear_gradient`, and `checkerboard` are layout-aware:
  they draw across the content width and allocate a row of the requested
  height.

- `glm::Surface(width, height)` owns a transparent RGBA buffer. Use
  `fill_rect`, `draw_horizontal_gradient`, `draw_vertical_gradient`,
  `draw_bilinear_gradient`, `draw_checkerboard`, `draw_filled_circle`,
  `draw_filled_ellipse`, `draw_filled_triangle`, `draw_filled_polygon`,
  `draw_polygon_outline`, `draw_polyline`, `draw_triangle_outline`,
  `draw_cubic_bezier`, `draw_ellipse_outline`,
  `draw_circle_outline`,
  `draw_rounded_rect`, `draw_rounded_rect_outline`, `outline_rect`, `draw_line`,
  `draw_dashed_line`, `draw_text`, and the
  `push_clip`/`pop_clip` pair to draw.
  `draw_image` and `draw_tinted_image` blit packed pixel buffers with
  nearest-neighbor sampling; `measure_text` returns pixel dimensions.
- `glm::Context(width, height, theme)` owns a `Surface` and UI state.
  Call `begin(mouse_x, mouse_y, mouse_down)`, declare widgets/windows, then
  call `end`. Read the output with `pixels()`.
- `Context::gradient(left_color, right_color, height)` draws a horizontal
  gradient across the current content width and allocates a layout row.
- `Context::vertical_gradient(top_color, bottom_color, height)` draws a
  top-to-bottom gradient across the current content width and allocates a row.
- `Context::bilinear_gradient(top_left, top_right, bottom_left, bottom_right,
  height)` interpolates all four corners across a full-width layout row.
- `Context::checkerboard(first_color, second_color, height, cell_size)` draws
  an alternating square pattern across a full-width layout row.
- `window(title, rect, callback)` creates a titled panel with a vertical layout.
  Use `window(title, window_state, callback)` with `glm::WindowState` to enable
  title-bar dragging while retaining position in caller-owned state.
  `label`, `button`, `checkbox`, and `slider` return an `Interaction`;
  button results can be checked directly in an `if` or through `.clicked`.
  `button(text, enabled)` can disable pointer activation and keyboard focus
  without changing the existing `button(text)` call.
  `toggle` is a keyboard-focusable switch; `selectable` is a focusable row
  toggling caller-owned selection state. `separator` draws a themed
  non-interactive divider. Value widgets report changes through `.changed`.
- `stepper(id, value, minimum, maximum, step)` draws a bounded integer control.
  Its end buttons and focused arrow keys adjust by `step`; Home/End jump to the
  bounds.
- `vertical_slider(id, value, minimum, maximum, height)` draws a vertical
  slider with the minimum at the bottom and maximum at the top. It supports
  pointer dragging and the same focused keyboard controls as `slider`.
- `range_slider(id, lower, upper, minimum, maximum)` draws two independently
  focusable handles for selecting an interval. Drag the handles or use Tab,
  arrow keys, Home, and End to adjust them without crossing.
- `rating(id, value, maximum)` draws a themed discrete rating scale. Click a
  mark to select it; Left/Down and Right/Up adjust the value, while Home clears
  the rating and End selects the maximum.
- `knob(id, value, minimum, maximum)` draws a rotary control. Move the pointer
  around its arc to set the value; focused arrow keys adjust by one percent of
  the range and Home/End jump to its limits.
- `drag_value(id, value, minimum, maximum, sensitivity)` adjusts a value by
  horizontal pointer movement. The sensitivity is the value change per pixel;
  focused arrow keys use that same increment.
- `xy_pad(id, x, y, min_x, max_x, min_y, max_y, height)` maps pointer position
  into two bounded values. X increases to the right and Y increases upward;
  arrows adjust one axis and Home/End jump both values to their bounds.
- `sparkline(samples, minimum, maximum, height)` draws a non-interactive,
  themed sample trace, skipping non-finite samples and clipping to its row.
- `histogram(samples, minimum, maximum, bins, height)` groups finite samples
  into a bounded number of themed bars whose heights reflect bucket counts.
- `profiler_timeline(events, frame_duration_ms, frame_budget_ms, height)`
  visualizes `ProfileEvent` intervals measured from frame start. Each interval
  is clipped to the frame duration; invalid intervals are ignored. The dashed
  vertical marker shows the budget when it lies within the displayed frame.
- `scatter_plot(points, min_x, max_x, min_y, max_y, height)` plots
  `std::pair<float, float>` points within the specified data ranges. The y-axis
  is inverted for downward-growing screen coordinates; out-of-range and
  non-finite points are omitted, reversed bounds are normalized, and equal
  bounds map matching point values to the chart center. Markers are clipped to
  the chart's plot area.
- `pie_chart(values, colors, height)` visualizes positive finite values as
  clockwise slices starting at twelve o'clock. Colors are selected by value
  index and repeat when the palette is shorter than the values. Invalid,
  negative, and zero values are ignored; an empty palette or no positive
  finite values leaves the chart area blank. The chart is centered in its
  allocated row and clipped like other Context drawing.
- `donut_chart(values, colors, height)` uses the same clockwise slices,
  palette cycling, and invalid-value handling as `pie_chart`, while leaving a
  centered hole with half the chart radius. The hole shows the themed chart
  background.
- `color_picker(id, packed_color)` draws a live swatch and four focusable
  channel controls, editing red, green, blue, and alpha in `0xAABBGGRR` order.
- `begin_tree_node(label, open)` draws a hierarchical disclosure row and
  returns whether its body is open. Call `end_tree_node()` after the body to
  restore layout and scope child widget IDs to that branch.
- `radio(group, options, selected)` draws one horizontal radio option per entry;
  arrows move the selection and focus, while Enter/Space selects the focused
  option. `progress_bar(id, value)` is read-only and clamps only its display.
  `collapsing_header(text, open)` toggles caller-owned open state on click or
  Enter/Space.
- `combo(id, selected, options)` opens a popup with click or Enter, navigates
  its highlight with arrows, and selects with Enter or a popup click. Escape
  and outside clicks close it.
- `tab_bar(group, tabs, selected)` draws a tab row and updates the caller-owned
  selected index. Mouse clicks and focused Left/Right or Home/End keys change
  the active tab.
- `color_palette(id, colors, selected, columns)` lays out color swatches in a
  grid and updates the selected index on click or focused arrow/Home/End input.
- `row`, `column`, and `next_row` expose basic layout cursor operations.
- `begin_table(id, columns)`, `next_table_column()`, `next_table_row()`, and
  `end_table()` provide nested, evenly divided table cells. Each row grows to
  the tallest cell, and table IDs scope descendant widgets.
- `Surface::measure_text_wrapped(text, width, scale)` returns the actual
  dimensions after wrapping. `Context::label_wrapped(text)` wraps to the
  current content width, draws each line, and returns its allocated height.
- `begin_child(id, rect)` and `end_child()` create nested clipped regions with
  wheel and keyboard scrolling. `child_scroll_offset(id)` reads the child
  offset in the current ID scope.
- `text_input`, `feed_char`, `feed_key`, and `cursor` provide simple text editing.
  `list` draws a clipped list; `scroll` adjusts its row offset and `feed_wheel`
  routes wheel input to the visible list or child region under the pointer.
- `multiline_text_input(id, value, visible_lines, maximum_length)` edits text
  with line breaks in a vertically scrollable field. Enter inserts a newline,
  Up/Down move between lines while preserving the preferred column, and
  Home/End move to the current line boundary. The caret remains visible while
  the field follows it vertically.
- Tab and Shift+Tab move keyboard focus through interactive widgets. Enter or
  Space activates buttons and checkboxes; arrow keys and Home/End adjust focused
  sliders and navigate focused lists. The focused widget is outlined.
- `glm::Theme` controls palette colors, padding, and the integer font scale.
- `Context::image(pixels, width, height, rect)` draws a caller-owned packed
  image buffer in surface coordinates, using nearest-neighbor scaling and the
  active clip stack.
- `Context::tinted_image(pixels, width, height, rect, tint)` applies packed
  RGBA channel modulation before drawing the caller-owned image.
- `Context::image_button(id, pixels, width, height, button_height)` draws a
  focusable image button, preserving the source aspect ratio and fitting it
  centered in a full-width layout row.
- `tooltip(text)` schedules a tooltip for the immediately preceding hovered
  widget. `Theme::tooltip_delay_frames` controls the deterministic frame delay.

### Drawing cubic Bézier curves

`Surface::draw_cubic_bezier` draws a cubic curve directly into the CPU pixel
buffer. The first and last points are the endpoints; the two middle points
control the curve's tangent and bend:

```cpp
glm::Surface surface(160, 100);
surface.draw_cubic_bezier(
    12.0f, 82.0f,   // start point
    42.0f, 8.0f,    // first control point
    118.0f, 8.0f,   // second control point
    148.0f, 82.0f,  // end point
    0xFF61AFEFU,    // packed color
    2.0f,           // stroke thickness in pixels
    48U);           // line segments used to approximate the curve
```

The API signature is
`draw_cubic_bezier(x0, y0, x1, y1, x2, y2, x3, y3, color, thickness, segments)`.
Coordinates and thickness are floats in surface pixel coordinates.
`thickness` defaults to `1.0f`; `segments` defaults to `32` and must be
positive. The implementation samples uniformly spaced values of the curve
parameter from 0 to 1, then strokes those points as an open polyline. This
fixed subdivision makes output deterministic; increase `segments` for a
smoother approximation of strongly curved paths. The stroke honors the
current clip and source-over alpha blending. Non-finite points or thickness,
non-positive thickness, and zero segments are no-ops.

For custom point sequences, use `draw_polyline` (open path) or
`draw_polygon_outline` (implicitly closed path) with an interleaved array of
`x, y` float coordinates.

## Design decisions

- Mouse coordinates are integer surface coordinates; a click is reported on the
  release frame only when the pointer is still over the active widget.
- The built-in 8-by-16 font supports printable ASCII; lowercase letters share
  their uppercase bitmaps to keep the embedded font small.
- The scrollable list takes an explicit row offset; callers can update it with
  `Context::scroll` because the library does not own platform wheel events.
- `Context::feed_wheel(int)` accepts platform-independent wheel deltas: 120
  units scroll one row toward the first item, and -120 scroll one row toward
  the last in a list; child regions scroll vertically by 24 pixels per notch.
  Partial notches accumulate while a list or child under the pointer consumes
  the input; wheel input over other widgets is discarded.
- Tests fetch doctest with CMake FetchContent; the Glimmer header itself has no
  third-party dependencies.
- A window and input device are platform concerns: the optional Windows demo
  provides a native adapter, while the core library only produces pixels and
  consumes input supplied by the host application.
- Keyboard focus follows widget declaration order and wraps at either end;
  slider arrow keys adjust the value by one percent of its configured range.
- Pie-chart weights are positive finite values; invalid or non-positive entries
  are omitted, and the slice palette repeats by original value index. Slices
  start at twelve o'clock and proceed clockwise to match downward-growing
  screen coordinates.
- Row allocation advances the vertical cursor and column allocation advances
  the horizontal cursor; neither automatically wraps or constrains requested
  column widths. Returned rectangles may extend beyond the layout bounds, while
  drawing remains constrained by the active surface clip.
- Table columns divide the current content width evenly with theme padding
  between cells. Call `next_table_column()` to move across a row and
  `next_table_row()` to advance; rows use the greatest vertical extent of their
  cells, and `end_table()` restores the enclosing layout at the table's bottom.
  A zero column count is treated as one. Tables do not draw borders or headings;
  use regular widgets for cell content.
- Text inputs clip their contents and caret to the field rectangle and scroll
  the displayed text horizontally as needed to keep the caret visible.
- Empty text measures zero pixels wide and one font line high; unsupported
  non-ASCII bytes draw as question-mark glyphs and count as one glyph each.
- Draggable windows use `glm::WindowState` owned by the caller. Dragging starts
  only from the title bar and clamps its position to the render surface.
- Radio options share one horizontal row, and keyboard arrows move both the
  selection and focus to keep keyboard activation aligned with the visible
  choice. A progress bar remains read-only but participates in focus order;
  activation reports a click without changing its input value.
- Wrapped text collapses runs of ASCII spaces between words. Overlong words
  break at the glyph limit without adding hyphens; a width narrower than one
  glyph still permits one glyph per line, with drawing clipped to the layout.
  Empty wrapped text occupies one font line.
- Child rectangles use surface coordinates. Child scroll state persists inside
  the Context, keyed by its scoped ID; a wheel event is routed using the prior
  frame's hit geometry to the deepest visible child under the pointer. Keyboard
  Up/Down scroll by 24 pixels and Home/End jump to either scroll limit.
- Popup content is deferred to `Context::end()` and drawn above all regular
  widgets in the order opened. One combo popup is open at a time; pointer input
  inside it is captured so covered widgets do not receive those clicks.
- Multiline input is byte-oriented and retains the single-line editor's
  caller-owned `std::string` model. Enter inserts a newline; Up/Down preserve
  the preferred column across consecutive vertical moves, and Home/End target
  the current line. Its scroll state is managed as a nested child region and
  the caret is kept in the visible lines.
- Filled-circle centers use integer pixel coordinates and include pixels whose
  integer-coordinate distance is within the radius. Rounded-rectangle tests
  sample pixel centers; the radius is clamped to half the shorter side.
- Filled ellipses are specified by their bounding rectangle and sample pixel
  centers against the normalized ellipse equation. Non-positive or non-finite
  rectangle dimensions are a no-op.
- Filled triangles use pixel-center edge tests. The winding order does not
  affect the result; degenerate triangles and non-finite coordinates draw
  nothing.
- Filled polygons accept an interleaved array of `x, y` float coordinates and
  use the even-odd rule at pixel centers, supporting concave and self-intersecting
  contours. The closing edge is implicit; fewer than three vertices or
  non-finite coordinates draw nothing.
- Triangle outlines test pixel-center distance to each segment, with the
  thickness centered on the edge. Degenerate triangles render as a line or
  point, and each covered pixel is blended once.
- Polygon outlines accept the same interleaved `x, y` float coordinates and
  implicitly close the path. Thickness is centered on each edge; degenerate
  paths draw their segment union, and each covered pixel is blended once.
- `draw_polyline` accepts the same coordinates but leaves the path open; it
  requires at least two vertices and uses the same centered thickness and
  single-blend-per-pixel behavior as polygon outlines.
- Cubic Bézier curves take four control points and are approximated by a
  polyline. `segments` controls the deterministic subdivision count (default
  32); non-positive thickness or zero segments draws nothing.
- Ellipse outlines use the same pixel-center sampling; the inner ellipse is
  formed by insetting the bounding rectangle by `thickness` on each side.
  Non-positive or non-finite thickness draws nothing.
- Rounded-rectangle outlines use pixel-center sampling and an inner rounded
  rectangle inset by `thickness`. The radius is clamped to the outer bounds;
  non-positive radii behave like `outline_rect`, while non-finite radii or
  non-positive/non-finite thickness values draw nothing.
  Outlined rectangles default to a one-pixel, non-overlapping border.
- Circle outlines use integer pixel-coordinate distances, include the outer
  radius, and exclude the inner radius determined by the requested integer
  thickness. Thickness at least as large as the radius fills the circle; a
  zero-radius outline draws one pixel, while negative radii and non-positive
  thickness draw nothing.
- Dashed lines follow the same Bresenham pixel sequence as `draw_line`.
  `dash_length` must be positive; a non-positive gap makes the line continuous.
- Image buffers are supplied as a pointer plus source dimensions because
  `std::span` is not available in C++17. Destination pixels sample source
  pixels by nearest-neighbor scaling and use source-over alpha blending; the
  active surface clip applies without changing the source buffer.

- Tinted images multiply each source RGBA channel by its corresponding tint
  channel, round the product to the nearest byte, then use the regular
  source-over blend. An opaque white tint preserves the source image exactly.
- Horizontal gradients interpolate all four packed color channels between the
  first and last rasterized columns, round channel values to the nearest
  integer, and blend each resulting pixel source-over. A one-column gradient
  uses its left color.
- Vertical gradients use the same interpolation and blending rules from the
  first to last rasterized row; a one-row gradient uses its top color.
- Bilinear gradients interpolate each channel horizontally and vertically
  between their four corner colors; when an extent is one pixel, that axis uses
  its first edge. Channel interpolation is rounded to the nearest integer.
- Checkerboard cells are square, start at the supplied rectangle's top-left,
  and remain aligned when clipped. A non-positive cell size is treated as one
  pixel; each cell is blended source-over.
- Tooltips count consecutive frames in which `tooltip(text)` is called after
  the same hovered widget; the default delay is 30 frames and a delay of one
  shows it on the first hovered frame. Tooltip drawing is deferred to `end()`
  and follows combo popups, so tooltips occupy the topmost popup layer.
- Toggle switches update their caller-owned boolean on release or keyboard
  activation and report the change through `Interaction::changed`. A separator
  consumes a two-pixel row and draws the Theme border color without participating
  in focus or pointer interaction.
- Selectable rows toggle their caller-owned boolean on release or keyboard
  activation and report the state transition through `Interaction::changed`;
  use distinct IDs for independent selections.
- Disabled buttons draw in the Theme's window/muted-text colors, do not report
  hover or active state, and are omitted from keyboard focus and navigation.
- Image buttons are focusable and use the standard click/keyboard activation
  contract. Their images are centered and aspect-preserving; unused row space
  shows the configured button color.
- A color palette treats zero columns as one column, leaves the selection
  unchanged when no colors are available, and clamps an out-of-range selection
  when the palette is non-empty. Down from an incomplete final row selects the
  last available swatch.
- A range slider normalizes reversed bounds and clamps both caller values into
  the range. If the values arrive in reverse order, the upper value is raised
  to the lower value. Each handle is a separate Tab stop; its Home/End actions
  respect the other handle to prevent crossing.
- A rating has an inclusive value range from zero (no marks selected) to its
  maximum. Non-positive maximum values are treated as a one-mark scale so the
  control always remains usable.
- The knob uses a 270-degree arc from its lower-left endpoint, across the top,
  to its lower-right endpoint. Pointer positions in the remaining gap clamp to
  the nearest endpoint.
- Drag-value sensitivity is a non-negative value increment per horizontal
  pixel; a negative sensitivity is treated as zero. Each frame applies only
  pointer movement since the preceding frame while the control is active.
- XY-pad Y values increase upward to match vertical-slider direction. Home
  moves both axes to their minima, while End moves both to their maxima.
- Sparklines connect only adjacent finite samples. A constant range places
  samples on the centerline, and an isolated finite sample is drawn as a dot.
- Histograms clamp finite samples to the supplied range, ignore non-finite
  values, and scale bars relative to the most populated bucket. A degenerate
  range puts valid samples in the center bucket.
- Open tree nodes indent their contents and push the label onto the widget ID
  scope; close every opened node with `end_tree_node()` after drawing its body.
- The color picker edits the existing packed pixel format directly; its
  channel controls are ordered red, green, blue, alpha and increment by one.
