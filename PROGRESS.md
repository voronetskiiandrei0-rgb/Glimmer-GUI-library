# Glimmer implementation progress

## Item 1: Layout tests (M5)

- **Status:** Done. Added exact geometry coverage for padded row and column
  allocation, cursor spacing, overflow without automatic wrapping, nested
  window content coordinates, and parent cursor restoration.
- **Files changed:** `tests/test_glimmer.cpp` (322 lines),
  `README.md` (101 lines), `PROGRESS.md` (27 lines).
- **Tests:** MSVC Debug: 10/10 cases, 215/215 assertions passed.
  MSVC Release: 10/10 cases, 215/215 assertions passed. Clang 19 strict
  warning-as-error build: 10/10 cases, 215/215 assertions passed.
- **Compilers:** MSVC 19.52, Clang 19.1.5. GCC installation using winget is
  still in progress; no GCC result is claimed.
- **Design decisions:** Rows advance vertically by row height plus theme
  padding; columns advance horizontally by requested width plus padding.
  Allocation does not wrap or clamp column widths; drawing remains clipped.
  The first nested-layout test accidentally allocated its columns after a row
  had advanced their shared vertical cursor. The test setup was corrected to
  measure row and column allocation independently from the nested content
  origin; no expected coordinates were changed.
- **API changes:** None.
- **Conflict with original spec:** None.
- **Known issues / unverified:** Window dragging remains optional and
  unimplemented. GCC had not yet been installed at this milestone; its later
  installation is recorded below and its frontend remained unusable.

## Item 2: Mouse wheel input for lists (M7)

- **Status:** Done. Wheel deltas are routed only to a non-empty visible list
  under the current pointer, offsets clamp at both ends, and focus on other
  widgets is preserved. Partial notches combine across list-consumed events;
  events not consumed by a list are discarded at frame end.
- **Files changed:** `include/glimmer/glimmer.hpp` (937 lines),
  `tests/test_glimmer.cpp` (437 lines), `README.md` (89 lines), and
  `PROGRESS.md`.
- **Tests:** MSVC Debug: 13/13 cases, 225/225 assertions passed.
  MSVC Release: 13/13 cases, 225/225 assertions passed.
  Clang 19 with `-Wall -Wextra -Wpedantic -Werror`: 13/13 cases,
  225/225 assertions passed. GCC was installed through winget, but the
  compiler frontend `cc1plus.exe` exits with status `0xC0E90002`; the GCC
  CMake compiler probe and strict test compilation therefore remain
  unverified. No GCC pass is claimed.
- **Compilers:** MSVC 19.52, Clang 19.1.5, GCC 16.2.0 driver (frontend
  unusable in this environment).
- **Design decisions:** Added `Context::feed_wheel(int wheel_delta)` rather than
  changing `begin(int, int, bool)`. Deltas use platform-independent 120-unit
  wheel notches; positive scrolls toward the first item. Fractional notches
  persist only after a visible list consumes the event. Wheel input over other
  controls is discarded.
- **API changes:** Added `void Context::feed_wheel(int wheel_delta)`.
  The existing `begin(int, int, bool)` signature is unchanged. No example
  changes were needed.
- **Conflict with original spec:** None.
- **Known issues / unverified:** GCC strict compilation is unverified because
  its installed frontend cannot run. A list is matched against its rectangle
  and active ancestor clip; overlapping lists use declaration/render order,
  so the first list that consumes the event receives it.

## Item 3: Text input clipping (M7)

- **Status:** Done. Text contents and caret are clipped to the widget rectangle;
  the displayed substring scrolls horizontally to keep the caret visible at
  the end of long values. Tests compare all pixels outside the field against an
  otherwise identical window without a text input.
- **Files changed:** `include/glimmer/glimmer.hpp` (937 lines),
  `tests/test_glimmer.cpp` (437 lines), `README.md` (89 lines), and
  `PROGRESS.md`.
- **Tests:** MSVC Debug: 15/15 cases, 3251/3251 assertions passed.
  MSVC Release: 15/15 cases, 3251/3251 assertions passed. Clang 19 with
  `-Wall -Wextra -Wpedantic -Werror`: 15/15 cases, 3251/3251 assertions
  passed. GCC 16.2.0 strict compilation was attempted but its
  `cc1plus.exe` frontend exits with status `0xC0E90002`; no GCC pass is claimed.
- **Compilers:** MSVC 19.52 Debug and Release, Clang 19.1.5 strict
  warning-as-error build, GCC 16.2.0 driver (frontend unusable).
- **Design decisions:** The field rectangle is the clip boundary, including
  its padding; text begins after the left padding. When the cursor advances
  beyond the visible range, earlier text is hidden as needed to keep the caret
  visible. No public API change was needed.
- **API changes:** None.
- **Conflict with original spec:** None.
- **Known issues / unverified:** GCC strict compilation remains unverified
  because the installed frontend cannot run. Horizontal scrolling is
  byte-oriented, consistent with the existing simple ASCII text editor.

## Item 4: Text measurement and clipping audit (M2)

- **Status:** Done. Added coverage for empty text dimensions and no-op drawing,
  glyphs partly outside a clip rectangle, and UTF-8 bytes falling back to
  question-mark glyphs without out-of-range font access. Existing behavior
  passed; no implementation change was needed.
- **Files changed:** `tests/test_glimmer.cpp` (473 lines), `README.md`
  (92 lines), and `PROGRESS.md`.
- **Tests:** MSVC Debug: 16/16 cases, 3704/3704 assertions passed.
  MSVC Release: 16/16 cases, 3704/3704 assertions passed. Clang 19 with
  `-Wall -Wextra -Wpedantic -Werror`: 16/16 cases, 3704/3704 assertions
  passed. GCC 16.2.0 strict compilation remains unverified because its
  `cc1plus.exe` frontend exits with status `0xC0E90002`.
- **Compilers:** MSVC 19.52 Debug and Release, Clang 19.1.5 strict
  warning-as-error build; GCC driver attempted but frontend unusable.
- **Design decisions:** Empty strings have zero width and one font line of
  height, consistent with the existing line-count rule. Each unsupported
  non-ASCII byte maps to one `?` glyph; no UTF-8 decoding is implied.
- **API changes:** None.
- **Conflict with original spec:** None.
- **Known issues / unverified:** GCC compilation remains unverified. UTF-8
  multibyte sequences display one fallback glyph per encoded byte, in keeping
  with the library's ASCII-only font and byte-oriented text input.

## Item 5: Window dragging (M5, optional)

- **Status:** Done. Added caller-owned window position/drag state, title-bar
  drag initiation, pointer-offset movement, release handling, and screen-edge
  clamping. Existing fixed-rectangle `window` calls retain their behavior.
- **Files changed:** `include/glimmer/glimmer.hpp` (977 lines),
  `tests/test_glimmer.cpp` (522 lines), `README.md` (100 lines), and
  `PROGRESS.md` (131 lines).
- **Tests:** MSVC Debug: 18/18 cases, 3716/3716 assertions passed.
  MSVC Release: 18/18 cases, 3716/3716 assertions passed. Clang 19 with
  `-Wall -Wextra -Wpedantic -Werror`: 18/18 cases, 3716/3716 assertions
  passed. GCC 16.2.0 strict compilation remains unverified because its
  `cc1plus.exe` frontend exits with status `0xC0E90002`.
- **Compilers:** MSVC 19.52 Debug and Release, Clang 19.1.5 strict
  warning-as-error build; GCC driver attempted but frontend unusable.
- **Design decisions:** Added `glm::WindowState` and a compatible
  `window(title, state, callback)` overload instead of changing the existing
  `window(title, rect, callback)` signature. Dragging starts only on a press
  in the title bar; caller state stores the drag offset. The full window width
  is clamped horizontally when it fits the surface; the title bar remains
  vertically visible. No demo or example changes were needed.
- **API changes:** Added public `glm::WindowState` with `rect`, `dragging`,
  `drag_offset_x`, and `drag_offset_y`; added
  `Context::window(const std::string&, WindowState&, Callback&&)`. The existing
  fixed-rectangle overload is unchanged.
- **Conflict with original spec:** None.
- **Known issues / unverified:** GCC strict compilation remains unverified
  because the installed frontend cannot run. A window wider than the surface
  cannot fit entirely on-screen; its left edge is clamped to zero. With the
  Visual Studio multi-configuration generator, CTest requires `-C Debug` (or
  another configuration); that configured run passed. README now documents
  this requirement.

## Focused GCC strict-warning verification (2026-10-31)

- **Status:** Done. GCC UCRT64 16.2.0 configured, built the library consumers
  and tests with `-Wall -Wextra -Wpedantic -Werror`, and passed the test suite.
  No library or test warning required a fix; behavior and public API are
  unchanged.
- **GCC setup attempts:**
  - WSL check: exact command `wsl -l -v`. It failed with
    `Class not registered` and `Error code: Wsl/CallMsi/Install/REGDB_E_CLASSNOTREG`;
    no WSL distribution could be used.
  - MSYS2 initial package command: `pacman -S mingw-w64-ucrt-x86_64-gcc cmake`.
    It displayed the confirmation prompt and exited without installing when
    no interactive response was available.
  - MSYS2 noninteractive install: `C:\msys64\usr\bin\pacman.exe -S
    --noconfirm mingw-w64-ucrt-x86_64-gcc cmake`. The package download
    displayed mirror timeouts for GCC, CMake, headers, binutils, and CRT;
    pacman nevertheless completed package processing. Version check:
    `C:\msys64\ucrt64\bin\g++.exe --version` reported
    `g++.exe (Rev4, Built by MSYS2 project) 16.2.0`.
  - Build-tool setup: `C:\msys64\usr\bin\pacman.exe -S --noconfirm
    mingw-w64-ucrt-x86_64-make` installed MinGW Make 4.4.1. The MinGW
    Makefiles generator was not available in MSYS2 CMake and MinGW make could
    not consume its `/c/...` source paths. Then
    `C:\msys64\usr\bin\pacman.exe -S --noconfirm make` installed MSYS Make
    4.4.1; its post-transaction hook reported an error, but `make.exe` was
    present and runnable.
  - The earlier WinLibs GCC attempt from the previous session had failed in
    `cc1plus.exe` with status `0xC0E90002`. It is superseded by the working
    MSYS2 compiler; no WinLibs code failure is inferred.
- **GCC version and successful build commands:** GCC
  `g++.exe (Rev4, Built by MSYS2 project) 16.2.0`.
  MSYS2 CMake required its `Unix Makefiles` generator and MSYS make; the
  successful configure command was:
  `C:\msys64\usr\bin\cmake.exe -G "Unix Makefiles" -B build-gcc
  -DCMAKE_CXX_COMPILER=g++ -DCMAKE_MAKE_PROGRAM=C:/msys64/usr/bin/make.exe
  -DCMAKE_CXX_FLAGS="-Wall -Wextra -Wpedantic -Werror"`.
  Build: `C:\msys64\usr\bin\cmake.exe --build build-gcc`.
  Test: `C:\msys64\usr\bin\ctest.exe --test-dir build-gcc --output-on-failure`.
  The configure and build emitted no warnings or errors; CTest passed 1/1
  registered test target. Running `build-gcc\glimmer_tests.exe` directly
  reported 18/18 cases and 3716/3716 assertions passed.
- **Failed generator probes:** The requested initial Windows CMake configure
  selected an unavailable WinLibs Ninja executable and failed with
  `no such file or directory`. MSYS2 CMake rejected `MinGW Makefiles` as an
  unknown generator. `Unix Makefiles` paired with MinGW make configured but
  failed to locate source paths in `/c/...` format. Switching to MSYS make
  resolved the setup issue; no source workaround was made.
- **Warnings fixed:** 0. No warning category or source line required a change.
- **Regression tests:** MSVC 19.52 Debug: 18/18 cases and 3716/3716
  assertions passed. MSVC 19.52 Release: 18/18 cases and 3716/3716
  assertions passed. Clang 19.1.5 with
  `-Wall -Wextra -Wpedantic -Werror`:
  18/18 cases and 3716/3716 assertions passed. GCC 16.2.0 with the same
  strict warning flags: 18/18 cases and 3716/3716 assertions passed.
- **Files changed:** `PROGRESS.md` only. No library, tests, CMake configuration,
  examples, or public API signatures were changed. The request to use library
  features in an example was not applied because this session's explicit scope
  prohibits example edits except for a necessary compilation fix.
- **Golden images:** No golden-image differences; the packed-pixel golden
  test passed under GCC along with the full suite.
- **Remaining unverified:** None for GCC strict compilation/testing on this
  Windows MSYS2 UCRT64 toolchain. Invoke CTest with
  `C:\msys64\ucrt64\bin` and `C:\msys64\usr\bin` on `PATH` so the executable
  can load its runtime DLLs; an invocation without that environment failed to
  launch the test, while the correctly configured rerun passed. WSL remains
  unavailable; no Linux GCC result was obtained.

## Expansion Item 1: Small widgets

- **Status:** Done. Added radio groups, read-only progress bars, and
  caller-owned collapsing-header state, with mouse interaction and keyboard
  focus/activation.
- **Files changed:** `include/glimmer/glimmer.hpp` (1086 lines),
  `tests/test_glimmer.cpp` (615 lines), `README.md` (108 lines), and
  `PROGRESS.md`.
- **Tests:** MSVC Debug: 20/20 cases, 3747/3747 assertions passed. MSVC
  Release: 20/20 cases, 3747/3747 assertions passed. Clang 19.1.5 with
  `-Wall -Wextra -Wpedantic -Werror`: 20/20 cases, 3747/3747 assertions
  passed. GCC 16.2.0 with the same strict warning flags: 20/20 cases,
  3747/3747 assertions passed.
- **Compilers:** MSVC 19.52 Debug and Release, Clang 19.1.5 strict,
  GCC 16.2.0 strict (MSYS2 UCRT64).
- **New public API:** `Interaction Context::radio(const std::string&,
  const std::vector<std::string>&, int&)`; `Interaction
  Context::progress_bar(const std::string&, float)`; and `Interaction
  Context::collapsing_header(const std::string&, bool&)`.
- **Design decisions:** Radio choices share one horizontal layout row; arrows
  move selection and focus together, and activation selects the focused choice.
  An out-of-range selection is clamped before rendering. Progress bars accept
  values by value and clamp only the displayed fill, preserving caller state;
  they are focusable and activation reports a click without mutating the value.
  Empty radio option lists draw nothing and do not register focus.
- **Known issues / unverified:** None for this item. No existing public
  signature changed and no example was edited.

## Expansion Item 2: Text layout

- **Status:** Done. Added wrapped text measurement and a wrapped Context label
  that uses available content width, clips its drawing to the allocated
  height, and returns that height.
- **Files changed:** `include/glimmer/glimmer.hpp` (1177 lines),
  `tests/test_glimmer.cpp` (641 lines), `README.md` (116 lines), and
  `PROGRESS.md`.
- **Tests:** MSVC Debug: 22/22 cases, 3759/3759 assertions passed. MSVC
  Release: 22/22 cases, 3759/3759 assertions passed. Clang 19.1.5 with
  `-Wall -Wextra -Wpedantic -Werror`: 22/22 cases, 3759/3759 assertions
  passed. GCC 16.2.0 with the same strict warning flags: 22/22 cases,
  3759/3759 assertions passed.
- **Compilers:** MSVC 19.52 Debug and Release, Clang 19.1.5 strict,
  GCC 16.2.0 strict (MSYS2 UCRT64).
- **New public API:** `Rect Surface::measure_text_wrapped(const
  std::string&, float maximum_width, int scale = 1) const`; `float
  Context::label_wrapped(const std::string&)`.
- **Design decisions:** Wrapping collapses repeated ASCII spaces between
  words, respects explicit newline breaks, and splits words longer than the
  available glyph count without hyphenation. For a width narrower than one
  glyph, the minimum capacity is one glyph and drawing remains clipped. Empty
  text has one font-line height. The existing `label(const std::string&)`
  signature and behavior remain unchanged; wrapped labels use the additive
  `label_wrapped` name and return their allocated height.
- **Known issues / unverified:** None for this item. Non-ASCII text follows the
  existing byte-oriented fallback glyph behavior.

## Expansion Item 3: Scrollable child regions

- **Status:** Done. Added nested clipping/layout regions, persistent vertical
  scroll state, wheel routing to the deepest visible child, keyboard scrolling,
  clamping at both ends, and release/focus behavior for the child region.
- **Files changed:** `include/glimmer/glimmer.hpp` (1318 lines),
  `tests/test_glimmer.cpp` (731 lines), `README.md` (123 lines), and
  `PROGRESS.md`.
- **Tests:** MSVC Debug: 25/25 cases, 9167/9167 assertions passed. MSVC
  Release: 25/25 cases, 9167/9167 assertions passed. Clang 19.1.5 with
  `-Wall -Wextra -Wpedantic -Werror`: 25/25 cases, 9167/9167 assertions
  passed. GCC 16.2.0 with the same strict warning flags: 25/25 cases,
  9167/9167 assertions passed.
- **Compilers:** MSVC 19.52 Debug and Release, Clang 19.1.5 strict,
  GCC 16.2.0 strict (MSYS2 UCRT64).
- **New public API:** `void Context::begin_child(const std::string&, Rect)`;
  `void Context::end_child()`; and `float
  Context::child_scroll_offset(const std::string&) const`.
- **Design decisions:** Child rectangles are in surface coordinates. Scroll
  state is retained internally by scoped child ID so the simple begin/end
  surface does not require caller-owned state. Scroll extent is measured from
  the prior content layout; wheel targeting uses the prior frame's visible
  child geometry to identify the innermost child before applying the event.
  Keyboard Up/Down moves 24 pixels and Home/End jumps to the scroll limits.
- **Known issues / unverified:** None for this item. A child must have been
  declared in a previous frame before its first wheel event can be routed;
  layout-dependent content height is measured as the child is declared.

## Expansion Item 4: Popup layer and combo box

- **Status:** Done. Added deferred popup rendering after regular widgets,
  combo mouse and keyboard interaction, selection and dismissal handling,
  keyboard highlight movement, and input capture over the popup.
- **Files changed:** `include/glimmer/glimmer.hpp` (1528 lines),
  `tests/test_glimmer.cpp` (805 lines), `README.md` (129 lines), and
  `PROGRESS.md`.
- **Tests:** MSVC Debug: 27/27 cases, 9179/9179 assertions passed. MSVC
  Release: 27/27 cases, 9179/9179 assertions passed. Clang 19.1.5 with
  `-Wall -Wextra -Wpedantic -Werror`: 27/27 cases, 9179/9179 assertions
  passed. GCC 16.2.0 with the same strict warning flags: 27/27 cases,
  9179/9179 assertions passed.
- **Compilers:** MSVC 19.52 Debug and Release, Clang 19.1.5 strict,
  GCC 16.2.0 strict (MSYS2 UCRT64).
- **New public API:** `Interaction Context::combo(const std::string&,
  std::size_t&, const std::vector<std::string>&)`; added `Key::Escape`.
- **Design decisions:** Popup drawing is deferred to `Context::end()` and
  overlaid after regular widgets in combo declaration/open order. Only one
  combo popup is open at a time. Input is blocked using the popup geometry
  recorded in the previous frame, while the current frame popup is drawn
  last. Enter/Space opens a closed combo and selects the highlighted item in
  an open combo; Escape closes without changing selection.
- **Known issues / unverified:** None for this item. No existing public
  function signature changed; no example was edited.

## Expansion Item 5: Multi-line text input

- **Status:** Done. Added line-aware editing and cursor navigation with
  vertically scrollable content that keeps the caret in the visible viewport.
- **Files changed:** `include/glimmer/glimmer.hpp` (1803 lines),
  `tests/test_glimmer.cpp` (1019 lines), `README.md` (147 lines), and
  `PROGRESS.md`.
- **Tests:** MSVC Debug: 30/30 cases, 9194/9194 assertions passed. MSVC
  Release: 30/30 cases, 9194/9194 assertions passed. Clang 19.1.5 with
  `-Wall -Wextra -Wpedantic -Werror`: 30/30 cases, 9194/9194 assertions
  passed. GCC 16.2.0 with the same strict warning flags: 30/30 cases,
  9194/9194 assertions passed.
- **Compilers:** MSVC 19.52 Debug and Release, Clang 19.1.5 strict,
  GCC 16.2.0 strict (MSYS2 UCRT64).
- **New public API:** `Interaction Context::multiline_text_input(const
  std::string&, std::string&, std::size_t visible_lines = 4U,
  std::size_t maximum_length = 4096U)`. Existing single-line input signature
  is unchanged.
- **Design decisions:** The existing byte-oriented text model is preserved.
  Enter inserts a newline; Home/End target the current line; Up/Down preserve
  the initial preferred column through successive moves. Vertical scroll uses
  the existing child-region mechanism and follows the caret when the field is
  focused. The caret is represented as a linear byte offset to remain
  compatible with `Context::cursor()`.
- **Known issues / unverified:** None for this item. Mouse-wheel scrolling is
  tested while focus is outside the editor; no example was edited.

## Expansion Item 6: Drawing primitives and images

- **Status:** Done. Added filled circles, rounded filled rectangles, outlined
  rectangles, and clipped nearest-neighbor image blitting from pixel buffers
  or another `Surface`.
- **Files changed:** `include/glimmer/glimmer.hpp` (1933 lines),
  `tests/test_glimmer.cpp` (1109 lines), `README.md` (170 lines), and
  `PROGRESS.md`.
- **Tests:** MSVC Debug: 33/33 cases, 9294/9294 assertions passed. MSVC
  Release: 33/33 cases, 9294/9294 assertions passed. Clang 19.1.5 with
  `-Wall -Wextra -Wpedantic -Werror`: 33/33 cases, 9294/9294 assertions
  passed. GCC 16.2.0 with the same strict warning flags: 33/33 cases,
  9294/9294 assertions passed.
- **Compilers:** MSVC 19.52 Debug and Release, Clang 19.1.5 strict,
  GCC 16.2.0 strict (MSYS2 UCRT64).
- **New public API:** `Surface::draw_filled_circle(int center_x, int
  center_y, int radius, std::uint32_t color)`; `Surface::draw_rounded_rect(
  Rect, float radius, std::uint32_t color)`; `Surface::outline_rect(
  Rect, std::uint32_t color, float thickness = 1.0f)`; `Surface::draw_image(
  const std::uint32_t*, int source_width, int source_height, Rect)`; and
  `Context::image` overloads for a pixel pointer with dimensions and for a
  `const Surface&`.
- **Design decisions:** Circle centers use integer pixel coordinates and
  include pixels at distance less than or equal to the integer radius.
  Rounded rectangles sample pixel centers and clamp radius to half the
  shorter side. Outlines default to a one-pixel border and draw non-overlapping
  bands. Images accept either a packed pointer plus dimensions (C++17 has no
  `std::span`) or a source `Surface`; nearest-neighbor sampling and source-over
  blending apply within the active clip stack.
- **Known issues / unverified:** None for this item. The Clang test executable
  was compiled with the strict flags and run directly; CTest was used for the
  MSVC and GCC builds. No examples or CMake files were edited.

## Expansion Item 7: Tooltips

- **Status:** Done. Added hover-frame tracking, configurable delay, and
  deferred tooltip rendering above ordinary widgets and combo popups.
- **Files changed:** `include/glimmer/glimmer.hpp` (2014 lines),
  `tests/test_glimmer.cpp` (1146 lines), `README.md` (176 lines), and
  `PROGRESS.md`.
- **Tests:** MSVC Debug: 35/35 cases, 9298/9298 assertions passed. MSVC
  Release: 35/35 cases, 9298/9298 assertions passed. Clang 19.1.5 with
  `-Wall -Wextra -Wpedantic -Werror`: 35/35 cases, 9298/9298 assertions
  passed. GCC 16.2.0 with the same strict warning flags: 35/35 cases,
  9298/9298 assertions passed.
- **Compilers:** MSVC 19.52 Debug and Release, Clang 19.1.5 strict,
  GCC 16.2.0 strict (MSYS2 UCRT64).
- **New public API:** `Theme::tooltip_delay_frames` (default `30U`) and
  `void Context::tooltip(const std::string&)`.
- **Design decisions:** The call must immediately follow the widget to which
  the tooltip belongs and be repeated every frame for continuous hover
  tracking. Frame counting is deterministic; delay one displays on the first
  hovered frame. Tooltip geometry is placed by the pointer and clamped to the
  render surface. Tooltips are deferred until `end()` and drawn after combo
  popups, on the topmost popup layer.
- **Known issues / unverified:** None. No API signatures were changed, and
  no examples or CMake files were edited.

## Expansion queue summary

- **Completed:** Items 1–7 (small widgets, wrapped text, scrollable child
  regions, combo popups, multiline text input, primitives and images, and
  tooltips).
- **Partial / not started:** None.
- **Final verification:** MSVC Debug and Release, Clang 19.1.5 strict, and
  GCC 16.2.0 strict each pass 35 doctest cases and 9298 assertions.
- **Unverified toolchains:** None among the requested toolchains.
- **Review decisions:** Combo popups render above ordinary widgets at frame
  end; tooltip delay defaults to 30 hovered frames and tooltips draw after
  combo popups; multiline input uses Enter for newline, Up/Down for vertical
  caret movement, and line-local Home/End.

## Follow-up: toggle controls and current-feature live demo

- **Status:** Done. Added a focusable toggle switch and themed divider, and
  updated the Windows live demo to showcase the newer widget, text, image,
  popup, tooltip, and scrolling APIs.
- **Files changed:** `include/glimmer/glimmer.hpp` (2052 lines),
  `tests/test_glimmer.cpp` (1203 lines),
  `examples/live_demo_win32.cpp` (310 lines), `README.md` (184 lines), and
  `PROGRESS.md`.
- **Tests:** MSVC Debug: 38/38 cases, 9310/9310 assertions passed. MSVC
  Release: 38/38 cases, 9310/9310 assertions passed. Clang 19.1.5 with
  `-Wall -Wextra -Wpedantic -Werror`: 38/38 cases, 9310/9310 assertions
  passed. GCC 16.2.0 compiled the strict-warning test target, but CTest could
  not launch the binary: `Process not started ... glimmer_tests.exe
  [permission denied]`; direct launch was blocked by the Windows Application
  Control policy. Thus the current GCC test run is unverified.
- **Builds:** The Windows live demo compiled in MSVC Debug and Release. The
  strict Clang run compiled and tested the portable library/tests; GCC
  compiled the target, but the host blocked test execution. The Windows-only
  demo is not built by those non-Windows toolchains.
- **New public API:** `Interaction Context::toggle(const std::string&,
  bool&)` and `void Context::separator()`.
- **Demo updates:** Added `toggle`, `radio`, `progress_bar`,
  `collapsing_header`, `combo`, wrapped labels, single- and multi-line text
  input, a scrolling list and child, image rendering, tooltips, and draggable
  windows. The Windows wheel message now calls `Context::feed_wheel` and
  supplies client-space pointer coordinates; Escape is forwarded to close
  combo popups.
- **Design decisions:** Toggle reuses the established `Interaction` contract:
  it changes caller-owned state on mouse release or keyboard activation and
  reports the change. The divider allocates a two-pixel row and uses the
  Theme border color; it is non-interactive. Tooltip attachment after a radio
  group treats the complete option row as one hovered widget.
- **Known issues / unverified:** The native demo was compiled but not launched
  for interactive visual inspection in this session. GCC's current test
  executable could not run under the host's Application Control policy. No
  API signature was changed.

## Follow-up: selectable rows

- **Status:** Done. Added a focusable selectable row that toggles caller-owned
  state by mouse or keyboard activation, and included pinned-profile choices
  in the Windows demo.
- **Files changed:** `include/glimmer/glimmer.hpp` (2073 lines),
  `tests/test_glimmer.cpp` (1240 lines),
  `examples/live_demo_win32.cpp` (318 lines), `README.md` (188 lines), and
  `PROGRESS.md`.
- **Tests:** MSVC Debug: 40/40 cases, 9320/9320 assertions passed. MSVC
  Release: 40/40 cases, 9320/9320 assertions passed. Clang 19.1.5 with
  `-Wall -Wextra -Wpedantic -Werror`: 40/40 cases, 9320/9320 assertions
  passed. GCC 16.2.0 with the same strict warning flags: 40/40 cases,
  9320/9320 assertions passed.
- **Builds:** The Windows live demo compiled in MSVC Debug and Release. GCC
  and Clang verified the portable tests; neither builds the Windows-only
  target on this host.
- **New public API:** `Interaction Context::selectable(const std::string&,
  bool&)`.
- **Design decisions:** A selectable row represents a persistent independent
  boolean choice and toggles on click release or Enter/Space activation, using
  the existing `Interaction` result and Theme colors. Separate selections
  should use distinct IDs; the widget does not impose radio-group semantics.
- **Known issues / unverified:** The Windows demo was compiled, but not
  launched for interactive visual inspection. No existing signature changed.

## Follow-up: tab bar

- **Status:** Done. Added a selectable tab row with caller-owned selection,
  pointer clicks, keyboard focus, arrow navigation, and Home/End navigation;
  integrated it into the demo's profile, notes, and preset pages.
- **Files changed:** `include/glimmer/glimmer.hpp` (2152 lines),
  `tests/test_glimmer.cpp` (1289 lines),
  `examples/live_demo_win32.cpp` (324 lines), `README.md` (191 lines), and
  `PROGRESS.md`.
- **Tests:** MSVC Debug and Release: 42/42 cases, 9333/9333 assertions passed.
  Clang 19.1.5 with `-Wall -Wextra -Wpedantic -Werror`: 42/42 cases,
  9333/9333 assertions passed. GCC 16.2.0 compiled the strict-warning test
  target, but the host's Application Control policy blocked launching its
  test executable; current GCC test execution remains unverified.
- **Builds:** The Windows demo compiled in MSVC Debug and Release.
- **New public API:** `Interaction Context::tab_bar(const std::string&,
  const std::vector<std::string>&, std::size_t&)`.
- **Design decisions:** Tabs are evenly divided across one allocated layout
  row. The selected index is clamped for non-empty options; empty options
  leave caller state unchanged. Left/Right and Up/Down move selection and
  focus together; Home/End jump to the first/last tab. The tab bar does not
  own the page content; callers conditionally declare it after reading the
  selected index.
- **Known issues / unverified:** Demo compiled but was not interactively
  launched. GCC's current executable launch is blocked by host policy. No
  existing public signature changed.

## Follow-up: image button

- **Status:** Done. Added an interactive image button with keyboard focus and
  activation; the Windows preview uses it to cycle the selected output mode.
- **Files changed:** `include/glimmer/glimmer.hpp` (2180 lines),
  `tests/test_glimmer.cpp` (1334 lines),
  `examples/live_demo_win32.cpp` (327 lines), `README.md` (197 lines), and
  `PROGRESS.md`.
- **Tests:** MSVC Debug and Release: 44/44 cases, 9344/9344 assertions passed.
  Clang 19.1.5 with `-Wall -Wextra -Wpedantic -Werror`: 44/44 cases,
  9344/9344 assertions passed. GCC 16.2.0 with the same strict warning flags:
  44/44 cases, 9344/9344 assertions passed.
- **Builds:** The Windows demo compiled in MSVC Debug and Release. GCC and
  Clang builds verify the portable library and tests.
- **New public API:** `Interaction Context::image_button(const std::string&,
  const std::uint32_t*, int source_width, int source_height,
  float height = 64.0f)`.
- **Design decisions:** Image buttons occupy the available content width and
  caller-selected row height; source pixels are centered and scaled
  proportionally with nearest-neighbor sampling. Empty or invalid image input
  leaves the themed button background intact while preserving interaction.
  Image pixels are drawn over the button background; hover/active feedback
  remains visible on the outline.
- **Known issues / unverified:** The demo compiled but was not launched for
  interactive visual inspection. No existing public signature changed.

## Follow-up: bounded integer stepper

- **Status:** Done. Added an integer stepper with decrement/increment buttons,
  keyboard adjustment, Home/End bounds, and caller-defined step size; the live
  demo uses it for output copies.
- **Files changed:** `include/glimmer/glimmer.hpp` (2266 lines),
  `tests/test_glimmer.cpp` (1400 lines),
  `examples/live_demo_win32.cpp` (330 lines), `README.md` (200 lines), and
  `PROGRESS.md`.
- **Tests:** MSVC Debug and Release: 46/46 cases, 9361/9361 assertions passed.
  Clang 19.1.5 with `-Wall -Wextra -Wpedantic -Werror`: 46/46 cases,
  9361/9361 assertions passed. GCC 16.2.0 with the same strict warning flags:
  46/46 cases, 9361/9361 assertions passed.
- **Builds:** The Windows live demo compiled in MSVC Debug and Release.
- **New public API:** `Interaction Context::stepper(const std::string&, int&,
  int minimum, int maximum, int step = 1)`.
- **Design decisions:** Reversed bounds are normalized to ascending order and
  the initial value is clamped. A non-positive step is treated as one. Left
  and right row buttons decrement/increment; focused Left/Down and Right/Up
  do the same, while Home/End set the bounds. Arithmetic is widened before
  applying step changes to avoid signed integer overflow.
- **Known issues / unverified:** The live demo compiled but was not launched
  for interactive visual inspection. No existing public signature changed.

## Follow-up: disabled buttons

- **Status:** Done. Added an additive enabled/disabled button overload and
  used it to disable profile application when the name is empty.
- **Files changed:** `include/glimmer/glimmer.hpp` (2297 lines),
  `tests/test_glimmer.cpp` (1452 lines),
  `examples/live_demo_win32.cpp` (335 lines), `README.md` (204 lines), and
  `PROGRESS.md`.
- **Tests:** MSVC Debug and Release: 48/48 cases, 9374/9374 assertions passed.
  Clang 19.1.5 with `-Wall -Wextra -Wpedantic -Werror`: 48/48 cases,
  9374/9374 assertions passed. GCC 16.2.0 with the same strict warning flags:
  48/48 cases, 9374/9374 assertions passed.
- **Builds:** The Windows live demo compiled in MSVC Debug and Release.
- **New public API:** `Interaction Context::button(const std::string&,
  bool enabled)`. Existing `button(const std::string&)` remains unchanged and
  delegates to the enabled overload.
- **Design decisions:** Disabled buttons do not report hover, active, or
  clicked state and are omitted from focus order. If a button becomes disabled
  while focused or active, its focus, activation, and pending navigation are
  cleared immediately. Disabled rendering uses the Theme window and
  button-hover colors for its background and text.
- **Known issues / unverified:** Demo compiled but was not launched for
  interactive visual inspection. No existing public signature changed.

## Follow-up: vertical slider

- **Status:** Done. Added a vertical slider with pointer dragging and keyboard
  navigation, and placed it in the live demo preview panel.
- **Files changed:** `include/glimmer/glimmer.hpp` (2359 lines),
  `tests/test_glimmer.cpp` (1509 lines),
  `examples/live_demo_win32.cpp` (337 lines), `README.md` (207 lines), and
  `PROGRESS.md`.
- **Tests:** MSVC Debug and Release: 50/50 cases, 9388/9388 assertions passed.
  Clang 19.1.5 with `-Wall -Wextra -Wpedantic -Werror`: 50/50 cases,
  9388/9388 assertions passed. GCC 16.2.0 with the same strict warning flags:
  50/50 cases, 9388/9388 assertions passed.
- **Builds:** The Windows live demo compiled in MSVC Debug and Release.
- **New public API:** `Interaction Context::vertical_slider(const
  std::string&, float&, float minimum, float maximum,
  float height = 120.0f)`.
- **Design decisions:** The slider uses a full-width allocated layout row with
  the track centered; the minimum is at the bottom and maximum at the top.
  Pointer and keyboard behavior match the existing slider: dragging adjusts
  continuously, Left/Down decrement, Right/Up increment, and Home/End jump to
  bounds. Reversed bounds are normalized; an equal bound fixes the value.
- **Known issues / unverified:** The live demo compiled but was not launched
  for interactive visual inspection. No existing public signature changed.
## Follow-up: color palette

- **Status:** Done. Added a themed, keyboard-navigable color palette widget and
  added it to the interactive Windows showcase.
- **Files changed:** `include/glimmer/glimmer.hpp` (2465 lines),
  `tests/test_glimmer.cpp` (1567 lines),
  `examples/live_demo_win32.cpp` (347 lines), `README.md` (213 lines), and
  `PROGRESS.md` (631 lines).
- **Tests:** MSVC Debug and Release: 52/52 cases, 9403/9403 assertions passed.
  Clang 19.1.5 with `-Wall -Wextra -Wpedantic -Werror`: 52/52 cases,
  9403/9403 assertions passed. GCC 16.2.0 with
  `-Wall -Wextra -Wpedantic -Werror`: 52/52 cases, 9403/9403 assertions
  passed.
- **Builds:** The Windows live demo compiled with MSVC Debug and Release.
  GCC and Clang compiled and tested the portable library suite.
- **New public API:** `Interaction Context::color_palette(const std::string&,
  const std::vector<std::uint32_t>&, std::size_t& selected,
  std::size_t columns = 4U)`.
- **Design decisions:** A zero column count becomes one column; empty palettes
  leave the caller's selection untouched; non-empty palettes clamp an invalid
  selection. Down from an incomplete final row selects the last available
  swatch. Arrow keys move by grid position and Home/End jump to the first/last
  available colors.
- **Known issues / unverified:** The live demo compiled but was not launched
  for interactive visual inspection. No existing public signature changed.
## Follow-up: range slider

- **Status:** Done. Added an interval slider with independently focusable and
  keyboard-adjustable handles, mouse dragging, themed hover/active feedback,
  and protection against the handles crossing. The live demo now showcases it.
- **Files changed:** `include/glimmer/glimmer.hpp` (2589 lines),
  `tests/test_glimmer.cpp` (1649 lines),
  `examples/live_demo_win32.cpp` (352 lines), `README.md` (221 lines), and
  `PROGRESS.md` (657 lines).
- **Tests:** MSVC Debug: 54/54 cases, 9422/9422 assertions passed. MSVC
  Release: 54/54 cases, 9422/9422 assertions passed. Clang 19.1.5 with
  `-Wall -Wextra -Wpedantic -Werror`: 54/54 cases, 9422/9422 assertions
  passed. GCC 16.2.0 with `-Wall -Wextra -Wpedantic -Werror`: 54/54 cases,
  9422/9422 assertions passed.
- **Builds:** The Windows live demo compiled in MSVC Debug and Release. GCC
  and Clang compiled and tested the portable library suite.
- **New public API:** `Interaction Context::range_slider(const std::string&,
  float& lower_value, float& upper_value, float minimum, float maximum)`.
  Existing signatures are unchanged.
- **Design decisions:** Reversed bounds are normalized. Each input value is
  clamped into the range; when values are reversed, the upper value is raised
  to the lower value. The lower and upper handles are separate Tab stops.
  Arrow keys adjust by one percent of the range; Home/End jump to the available
  bound without crossing the other handle.
- **Known issues / unverified:** Interactive visual behavior was compile-tested
  but not manually inspected in a running Windows demo.

## Follow-up: four-corner bilinear gradients

- **Status:** Done. Added bilinear packed-color interpolation on Surface and a
  full-width Context row; the live demo shows a four-color blend.
- **Files changed:** `include/glimmer/glimmer.hpp` (3353 lines),
  `tests/test_glimmer.cpp` (2395 lines),
  `examples/live_demo_win32.cpp` (400 lines), `README.md` (291 lines), and
  `PROGRESS.md`.
- **Tests:** MSVC Debug and Release: 82/82 cases and 9611/9611 assertions
  passed. The Windows live demo compiled in both configurations with `/W4
  /WX`.
- **New public API:** `Surface::draw_bilinear_gradient(rect, top_left,
  top_right, bottom_left, bottom_right)` and
  `Context::bilinear_gradient(top_left, top_right, bottom_left, bottom_right,
  height)`. Existing APIs are unchanged.
- **Design decisions:** Each of the four packed RGBA channels is interpolated
  across both axes and rounded to the nearest integer before source-over
  blending. A one-pixel extent uses the first edge on that axis.
- **Known issues / unverified:** GCC and Clang were not available in this
  environment. The Windows demo compiled but was not manually opened.

## Follow-up: vertical RGBA gradients

- **Status:** Done. Added a top-to-bottom Surface gradient and a full-width
  Context layout widget; the live demo shows the same colors in both directions.
- **Files changed:** `include/glimmer/glimmer.hpp` (3284 lines),
  `tests/test_glimmer.cpp` (2339 lines),
  `examples/live_demo_win32.cpp` (398 lines), `README.md` (286 lines), and
  `PROGRESS.md`.
- **Tests:** MSVC Debug and Release: 79/79 cases and 9592/9592 assertions
  passed. The Windows live demo compiled in both configurations with `/W4
  /WX`.
- **New public API:** `Surface::draw_vertical_gradient(rect, top_color,
  bottom_color)` and `Context::vertical_gradient(top_color, bottom_color,
  height)`. Existing APIs are unchanged.
- **Design decisions:** The top and bottom colors are interpolated across
  rasterized rows using the same four-channel rounding, clipping, and
  source-over blending behavior as horizontal gradients. One-row gradients use
  their top color.
- **Known issues / unverified:** GCC and Clang were not available in this
  environment. The Windows demo compiled but was not manually opened.

## Follow-up: discrete rating

- **Status:** Done. Added a themed rating scale with mouse selection and
  keyboard adjustment; the Windows live demo now includes a profile rating.
- **Files changed:** `include/glimmer/glimmer.hpp` (2669 lines),
  `tests/test_glimmer.cpp` (1721 lines),
  `examples/live_demo_win32.cpp` (355 lines), `README.md` (227 lines), and
  `PROGRESS.md` (683 lines).
- **Tests:** MSVC Debug: 56/56 cases, 9441/9441 assertions passed. MSVC
  Release-equivalent build (`/O2 /DNDEBUG /W4 /WX`): 56/56 cases,
  9441/9441 assertions passed. Clang 19.1.5 with
  `-Wall -Wextra -Wpedantic -Werror`: 56/56 cases, 9441/9441 assertions
  passed. GCC 16.2.0 with the same strict warning flags: 56/56 cases,
  9441/9441 assertions passed.
- **Builds:** Windows live demo compiled in MSVC Debug and Release. The
  standard MSVC Release test executable was blocked from launch by Windows
  Application Control; a fresh Release-equivalent executable compiled with
  MSVC and ran the full suite successfully.
- **New public API:** `Interaction Context::rating(const std::string&,
  int& value, int maximum = 5)`. Existing signatures are unchanged.
- **Design decisions:** Zero means no marks selected; clicks select a mark,
  Left/Down decrement, Right/Up increment, Home clears, and End selects the
  maximum. Values clamp to the range. A non-positive maximum is treated as one
  to keep the control usable.
- **Known issues / unverified:** Interactive visual behavior was compile-tested
  but not manually inspected in a running Windows demo.

## Follow-up: nested table layout

- **Status:** Done. Added an evenly divided, nested table layout and a two-column
  showcase to the live demo's Profile tab.
- **Files changed:** `include/glimmer/glimmer.hpp` (3217 lines),
  `tests/test_glimmer.cpp` (2240 lines),
  `examples/live_demo_win32.cpp` (395 lines), `README.md` (273 lines), and
  `PROGRESS.md`.
- **Tests:** MSVC Debug and Release: 73/73 cases and 9555/9555 assertions
  passed. The full test executable and Windows demo compiled in both
  configurations with `/W4 /WX`.
- **New public API:** `begin_table(id, columns)`, `next_table_column()`,
  `next_table_row()`, and `end_table()`. Existing APIs are unchanged.
- **Design decisions:** Columns split the current content width evenly with
  theme padding between them. Rows advance to the maximum vertical cursor of
  their cells; zero columns means one. Each cell scopes descendant IDs using
  its row and column. Tables are unadorned layout only; callers provide cell
  content using regular widgets.
- **Known issues / unverified:** The live demo compiles, but its interactive
  visual behavior was not manually inspected.

## Follow-up: horizontal RGBA gradients

- **Status:** Done. Added a clipped horizontal gradient primitive and a
  full-width Context layout widget; the live demo shows a teal-to-coral gradient.
- **Files changed:** `include/glimmer/glimmer.hpp` (3268 lines),
  `tests/test_glimmer.cpp` (2289 lines),
  `examples/live_demo_win32.cpp` (397 lines), `README.md` (281 lines), and
  `PROGRESS.md`.
- **Tests:** MSVC Debug and Release: 76/76 cases and 9571/9571 assertions
  passed. The Windows live demo compiled in both configurations with `/W4
  /WX`.
- **New public API:** `Surface::draw_horizontal_gradient(rect, left_color,
  right_color)` and `Context::gradient(left_color, right_color, height)`.
  Existing APIs are unchanged.
- **Design decisions:** All four channels in packed `0xAABBGGRR` colors are
  linearly interpolated from the first rasterized column to the last and
  rounded to the nearest integer. Pixels use the existing source-over blend;
  a one-column gradient uses its left endpoint.
- **Known issues / unverified:** GCC and Clang were not available in this
  environment. The Windows demo compiled but was not manually opened.
## Follow-up: rotary knob

- **Status:** Done. Added a themed rotary numeric control with pointer-angle
  adjustment and keyboard navigation. The live demo includes a balance knob.
- **Files changed:** `include/glimmer/glimmer.hpp` (2761 lines),
  `tests/test_glimmer.cpp` (1787 lines),
  `examples/live_demo_win32.cpp` (359 lines), `README.md` (233 lines), and
  `PROGRESS.md` (707 lines).
- **Tests:** MSVC Debug: 58/58 cases, 9458/9458 assertions passed. MSVC
  Release: 58/58 cases, 9458/9458 assertions passed. Clang 19.1.5 with
  `-Wall -Wextra -Wpedantic -Werror`: 58/58 cases, 9458/9458 assertions
  passed. GCC 16.2.0 with the same strict warning flags: 58/58 cases,
  9458/9458 assertions passed.
- **Builds:** The Windows live demo compiled in MSVC Debug and Release. The
  MSVC Release test suite was compiled as a fresh `/O2 /DNDEBUG /W4 /WX`
  executable and passed. GCC and Clang compiled and tested the portable suite.
- **New public API:** `Interaction Context::knob(const std::string&, float&
  value, float minimum, float maximum)`. Existing signatures are unchanged.
- **Design decisions:** The knob renders a 270-degree arc from its lower-left
  endpoint, across the top, to its lower-right endpoint. Pointer positions in
  the remaining gap clamp to the nearest endpoint. Focused arrow keys adjust
  by one percent of the normalized range; Home/End jump to its limits.
- **Known issues / unverified:** Interactive visual behavior was compile-tested
  but not manually inspected in a running Windows demo.
## Follow-up: horizontal drag value

- **Status:** Done. Added a fine-grained float control using horizontal pointer
  deltas and keyboard input. The live demo includes a fine-tuning control.
- **Files changed:** `include/glimmer/glimmer.hpp` (2830 lines),
  `tests/test_glimmer.cpp` (1862 lines),
  `examples/live_demo_win32.cpp` (362 lines), `README.md` (240 lines), and
  `PROGRESS.md` (732 lines).
- **Tests:** MSVC Debug: 60/60 cases, 9475/9475 assertions passed. MSVC
  Release: 60/60 cases, 9475/9475 assertions passed. Clang 19.1.5 with
  `-Wall -Wextra -Wpedantic -Werror`: 60/60 cases, 9475/9475 assertions
  passed. GCC 16.2.0 with the same strict warning flags: 60/60 cases,
  9475/9475 assertions passed.
- **Builds:** The Windows live demo compiled in MSVC Debug and Release. The
  MSVC Release test suite was built as a fresh `/O2 /DNDEBUG /W4 /WX`
  executable and passed. GCC and Clang compiled and tested the portable suite.
- **New public API:** `Interaction Context::drag_value(const std::string&,
  float& value, float minimum, float maximum, float sensitivity = 0.01f)`.
  Existing signatures are unchanged.
- **Design decisions:** Sensitivity is a non-negative value increment per
  horizontal pixel; negative values are treated as zero. Keyboard arrows use
  the same increment. Pointer changes use only the delta since the prior frame
  while the widget remains active. Values clamp to normalized bounds.
- **Known issues / unverified:** Interactive visual behavior was compile-tested
  but not manually inspected in a running Windows demo.
## Follow-up: two-dimensional XY pad

- **Status:** Done. Added an XY value pad with pointer mapping, keyboard
  adjustment, themed grid/cursor rendering, and normalized bounds. The Notes
  tab in the live demo now showcases it.
- **Files changed:** `include/glimmer/glimmer.hpp` (2932 lines),
  `tests/test_glimmer.cpp` (1927 lines),
  `examples/live_demo_win32.cpp` (368 lines), `README.md` (245 lines), and
  `PROGRESS.md` (758 lines).
- **Tests:** MSVC Debug: 62/62 cases, 9493/9493 assertions passed. MSVC
  Release: 62/62 cases, 9493/9493 assertions passed. Clang 19.1.5 with
  `-Wall -Wextra -Wpedantic -Werror`: 62/62 cases, 9493/9493 assertions
  passed. GCC 16.2.0 with the same strict warning flags: 62/62 cases,
  9493/9493 assertions passed.
- **Builds:** The Windows live demo compiled in MSVC Debug and Release. The
  MSVC Release test suite was built as a fresh `/O2 /DNDEBUG /W4 /WX`
  executable and passed. GCC and Clang compiled and tested the portable suite.
- **New public API:** `Interaction Context::xy_pad(const std::string&,
  float& x_value, float& y_value, float minimum_x, float maximum_x,
  float minimum_y, float maximum_y, float height = 120.0f)`. Existing
  signatures are unchanged.
- **Design decisions:** Positive Y points upward, matching the vertical
  slider. Arrow keys adjust the corresponding axis by one percent of that
  axis's range; Home/End set both axes to their respective minima/maxima.
- **Known issues / unverified:** Interactive visual behavior was compile-tested
  but not manually inspected in a running Windows demo.
## Follow-up: sparkline chart

- **Status:** Done. Added a non-interactive themed sparkline, with a fixed
  sample history shown in the live demo.
- **Files changed:** `include/glimmer/glimmer.hpp` (2984 lines),
  `tests/test_glimmer.cpp` (1977 lines),
  `examples/live_demo_win32.cpp` (373 lines), `README.md` (249 lines), and
  `PROGRESS.md` (782 lines).
- **Tests:** MSVC Debug: 64/64 cases, 9505/9505 assertions passed. MSVC
  Release: 64/64 cases, 9505/9505 assertions passed. Clang 19.1.5 with
  `-Wall -Wextra -Wpedantic -Werror`: 64/64 cases, 9505/9505 assertions
  passed. GCC 16.2.0 with the same strict warning flags: 64/64 cases,
  9505/9505 assertions passed.
- **Builds:** The Windows live demo compiled in MSVC Debug and Release. The
  MSVC Release test suite was built as a fresh `/O2 /DNDEBUG /W4 /WX`
  executable and passed. GCC and Clang compiled and tested the portable suite.
- **New public API:** `void Context::sparkline(const
  std::vector<float>& samples, float minimum, float maximum,
  float height = 36.0f)`. Existing signatures are unchanged.
- **Design decisions:** Only adjacent finite samples are connected. Samples
  clamp to normalized bounds; a constant range is drawn along the centerline.
  An isolated finite value is a dot, and non-finite samples break the path.
- **Known issues / unverified:** Interactive visual behavior was compile-tested
  but not manually inspected in a running Windows demo.
## Follow-up: histogram chart

- **Status:** Done. Added a themed histogram for caller-provided samples and
  added it to the live demo alongside the sparkline.
- **Files changed:** `include/glimmer/glimmer.hpp` (3041 lines),
  `tests/test_glimmer.cpp` (2023 lines),
  `examples/live_demo_win32.cpp` (374 lines), `README.md` (254 lines), and
  `PROGRESS.md` (807 lines).
- **Tests:** MSVC Debug: 66/66 cases, 9514/9514 assertions passed. MSVC
  Release: 66/66 cases, 9514/9514 assertions passed. Clang 19.1.5 with
  `-Wall -Wextra -Wpedantic -Werror`: 66/66 cases, 9514/9514 assertions
  passed. GCC 16.2.0 with the same strict warning flags: 66/66 cases,
  9514/9514 assertions passed.
- **Builds:** The Windows live demo compiled in MSVC Debug and Release. The
  MSVC Release test suite was built as a fresh `/O2 /DNDEBUG /W4 /WX`
  executable and passed. GCC and Clang compiled and tested the portable suite.
- **New public API:** `void Context::histogram(const std::vector<float>&
  samples, float minimum, float maximum, std::size_t bins,
  float height = 48.0f)`. Existing signatures are unchanged.
- **Design decisions:** Finite sample values clamp to the normalized supplied
  range; non-finite values are ignored. Bars scale against the largest bucket.
  A degenerate range places finite samples in the center bucket. Bin count is
  limited by available pixel width.
- **Known issues / unverified:** Interactive visual behavior was compile-tested
  but not manually inspected in a running Windows demo.
## Follow-up: hierarchical tree nodes

- **Status:** Done. Added caller-state disclosure nodes with nested layout and
  child-ID scoping; the live demo shows a metadata tree in the Notes tab.
- **Files changed:** `include/glimmer/glimmer.hpp` (3099 lines),
  `tests/test_glimmer.cpp` (2092 lines),
  `examples/live_demo_win32.cpp` (380 lines), `README.md` (259 lines), and
  `PROGRESS.md` (832 lines).
- **Tests:** MSVC Debug: 68/68 cases, 9530/9530 assertions passed. MSVC
  Release: 68/68 cases, 9530/9530 assertions passed. Clang 19.1.5 with
  `-Wall -Wextra -Wpedantic -Werror`: 68/68 cases, 9530/9530 assertions
  passed. GCC 16.2.0 with the same strict warning flags: 68/68 cases,
  9530/9530 assertions passed.
- **Builds:** The Windows live demo compiled in MSVC Debug and Release. The
  MSVC Release test suite was built as a fresh `/O2 /DNDEBUG /W4 /WX`
  executable and passed. GCC and Clang compiled and tested the portable suite.
- **New public API:** `bool Context::begin_tree_node(const std::string&,
  bool& open)` and `void Context::end_tree_node()`. Existing signatures are
  unchanged.
- **Design decisions:** The caller supplies and retains each node's open state.
  `end_tree_node()` is called only when `begin_tree_node()` returns true.
  Expanded content indents by 12 pixels and hashes the node label into the
  widget ID scope, allowing repeated descendant labels in separate branches.
- **Known issues / unverified:** Interactive visual behavior was compile-tested
  but not manually inspected in a running Windows demo.
## Follow-up: packed RGBA color picker

- **Status:** Done. Added a live packed-color swatch and four focusable channel
  controls. The Notes tab in the Windows demo includes the editor.
- **Files changed:** `include/glimmer/glimmer.hpp` (3132 lines),
  `tests/test_glimmer.cpp` (2156 lines),
  `examples/live_demo_win32.cpp` (384 lines), `README.md` (264 lines), and
  `PROGRESS.md` (859 lines).
- **Tests:** MSVC Debug: 70/70 cases, 9546/9546 assertions passed by direct
  executable run. MSVC Release: 70/70 cases, 9546/9546 assertions passed.
  Clang 19.1.5 with `-Wall -Wextra -Wpedantic -Werror`: 70/70 cases,
  9546/9546 assertions passed. GCC 16.2.0 with the same strict warning flags:
  70/70 cases, 9546/9546 assertions passed.
- **Builds:** The Windows live demo compiled in MSVC Debug and Release.
  MSVC Debug CTest could not start the executable on the final run due to a
  Windows process-launch error; direct execution of the same freshly built
  Debug test binary passed. Release tests ran from a fresh `/O2 /DNDEBUG /W4
  /WX` executable. GCC and Clang compiled and tested the portable suite.
- **New public API:** `Interaction Context::color_picker(const
  std::string&, std::uint32_t& packed_color)`. Existing signatures are
  unchanged.
- **Design decisions:** The swatch and channel controls use the existing
  `0xAABBGGRR` packing. Red, green, blue, and alpha channels are focusable in
  that order and adjust by one per keyboard step; channel IDs are scoped by the
  picker label to allow multiple editors.
- **Known issues / unverified:** Interactive visual behavior was compile-tested
  but not manually inspected in a running Windows demo.

## Follow-up: checkerboard transparency preview

- **Status:** Done. Added a clipped alternating-cell fill and a full-width
  Context layout widget; the live demo shows a transparency-preview pattern.
- **Files changed:** `include/glimmer/glimmer.hpp` (3388 lines),
  `tests/test_glimmer.cpp` (2444 lines),
  `examples/live_demo_win32.cpp` (402 lines), `README.md` (296 lines), and
  `PROGRESS.md`.
- **Tests:** MSVC Debug and Release: 85/85 cases and 9647/9647 assertions
  passed. The Windows live demo compiled in both configurations with `/W4
  /WX`.
- **New public API:** `Surface::draw_checkerboard(rect, first_color,
  second_color, cell_size)` and `Context::checkerboard(first_color,
  second_color, height, cell_size)`. Existing APIs are unchanged.
- **Design decisions:** The square pattern begins at the drawing rectangle's
  top-left, so clipping does not reset its phase. Non-positive cell sizes clamp
  to one pixel, and both colors use source-over blending.
- **Known issues / unverified:** GCC and Clang were not available in this
  environment. The Windows demo compiled but was not manually opened.

## Follow-up: tinted image blitting

- **Status:** Done. Added packed RGBA tinting for nearest-neighbor image blits
  and a tinted preview in the Windows live demo.
- **Files changed:** `include/glimmer/glimmer.hpp` (3412 lines),
  `tests/test_glimmer.cpp` (2473 lines),
  `examples/live_demo_win32.cpp` (405 lines), `README.md` (301 lines), and
  `PROGRESS.md`.
- **Tests:** MSVC Debug and Release: 87/87 cases and 9654/9654 assertions
  passed. The Windows live demo compiled in both configurations with `/W4
  /WX`.
- **New public API:** `Surface::draw_tinted_image(source, width, height, rect,
  tint)` and `Context::tinted_image(source, width, height, rect, tint)`.
  Existing `draw_image` and `image` signatures remain unchanged.
- **Design decisions:** Each source channel is multiplied by its corresponding
  `0xAABBGGRR` tint channel, rounded to the nearest byte, then composited with
  the existing source-over blend. Opaque white tint preserves the source.
- **Known issues / unverified:** GCC and Clang were not available in this
  environment. The Windows demo compiled but was not manually opened.

## Follow-up: outlined circles

- **Status:** Done. Added a clipped integer-coordinate circle outline and
  showcased the outline in the Windows live demo.
- **Files changed:** `include/glimmer/glimmer.hpp` (3462 lines),
  `tests/test_glimmer.cpp` (2511 lines),
  `examples/live_demo_win32.cpp` (412 lines), `README.md` (307 lines), and
  `PROGRESS.md`.
- **Tests:** MSVC Debug and Release: 89/89 cases and 9686/9686 assertions
  passed. The Windows live demo compiled in both configurations with `/W4
  /WX`.
- **New public API:** `Surface::draw_circle_outline(center_x, center_y, radius,
  color, thickness = 1)`. Existing APIs are unchanged.
- **Design decisions:** Pixels are selected using integer-coordinate squared
  distances. Thickness at least as large as the radius draws a filled disk;
  radius zero draws one pixel. Negative radii and non-positive thickness are
  no-ops.
- **Known issues / unverified:** GCC and Clang were not available in this
  environment. The Windows demo compiled but was not manually opened.

## Follow-up: dashed line rendering

- **Status:** Done. Added deterministic dashed Bresenham lines and a line sample
  to the Windows live demo.
- **Files changed:** `include/glimmer/glimmer.hpp` (3498 lines),
  `tests/test_glimmer.cpp` (2550 lines),
  `examples/live_demo_win32.cpp` (419 lines), `README.md` (310 lines), and
  `PROGRESS.md`.
- **Tests:** MSVC Debug and Release: 91/91 cases and 9709/9709 assertions
  passed. The Windows live demo compiled in both configurations with `/W4
  /WX`.
- **New public API:** `Surface::draw_dashed_line(x0, y0, x1, y1, color,
  dash_length = 4, gap_length = 4)`. Existing APIs are unchanged.
- **Design decisions:** The line uses the same integer Bresenham sequence as
  `draw_line`; dash and gap lengths count pixels in that sequence. A non-positive
  dash draws nothing, while a non-positive gap makes it continuous.
- **Known issues / unverified:** GCC and Clang were not available in this
  environment. The Windows demo compiled but was not manually opened.

## Follow-up: filled ellipse rendering

- **Status:** Done. Added deterministic filled ellipses and a sample to the
  Windows live demo.
- **Files changed:** `include/glimmer/glimmer.hpp`, `tests/test_glimmer.cpp`,
  `examples/live_demo_win32.cpp`, `README.md`, and `PROGRESS.md`.
- **Tests:** MSVC Debug and Release builds succeeded; Release CTest passed and
  the suite reports 93/93 cases and 9730/9730 assertions. Both builds include
  the demo compiled with `/W4 /WX`. After the final Debug rebuild, Windows
  Application Control blocked the Debug test executable; the Debug CTest run
  had passed earlier in this task before that rebuild.
- **New public API:** `Surface::draw_filled_ellipse(Rect, color)`. Existing APIs
  are unchanged.
- **Design decisions:** Ellipses are specified by their bounding rectangles and
  rasterized by testing pixel centers against the normalized ellipse equation.
  Non-finite coordinates and non-positive dimensions draw nothing. This also
  ensures clipping and source-over blending match the other rectangular
  drawing primitives.
- **Known issues / unverified:** GCC and Clang were not available in `PATH`;
  the existing GCC build cache also pointed to a different workspace path.
  The Windows demo compiled but was not manually opened.

## Follow-up: outlined ellipse rendering

- **Status:** Done. Added deterministic ellipse borders and a preview to the
  Windows live demo.
- **Feature:** Added deterministic ellipse borders to `Surface` and an outline
  preview to the Windows live demo.
- **Rasterization:** The argument is a surface-coordinate bounding rectangle.
  The inner ellipse is formed by insetting that rectangle by the requested
  thickness on each side; pixel centers inside the outer ellipse but outside
  the inner ellipse are drawn.
- **Invalid input:** Non-finite coordinates, dimensions, or thickness, as well
  as non-positive dimensions or thickness, draw nothing.
- **Files changed:** `include/glimmer/glimmer.hpp`, `tests/test_glimmer.cpp`,
  `examples/live_demo_win32.cpp`, `README.md`, and `PROGRESS.md`.
- **Tests:** Debug and Release CTest passed; the Release suite reports 95/95
  cases and 9739/9739 assertions. The Debug build succeeded. In Release, the
  demo source compiled, but linking its executable was blocked because an
  existing `glimmer_live_demo.exe` process is running. The Release test target
  built successfully.
- **New public API:** `Surface::draw_ellipse_outline(Rect, color,
  thickness = 1.0f)`. Existing APIs are unchanged.
- **Known issues / unverified:** GCC and Clang were not available in `PATH`.

## Follow-up: cubic Bézier drawing

- **Status:** Done. Added deterministic cubic Bézier strokes and a preview to
  the Windows live demo.
- **Feature:** Added deterministic cubic Bézier strokes to `Surface` and a
  curve preview to the Windows live demo.
- **Rasterization:** Evaluates the cubic at a fixed subdivision count, then
  uses the shared open-polyline stroke rasterizer for thickness, clipping, and
  alpha blending.
- **Invalid input:** Non-finite control points or thickness, non-positive
  thickness, and zero segments draw nothing.
- **Files changed:** `include/glimmer/glimmer.hpp`, `tests/test_glimmer.cpp`,
  `examples/live_demo_win32.cpp`, `README.md`, and `PROGRESS.md`.
- **Tests:** MSVC Debug build and Release test-target build succeeded. Debug
  and Release CTest passed; each suite reports 110/110 cases and 9910/9910
  assertions. The Windows demo compiled as part of the Debug build.
- **New public API:** `Surface::draw_cubic_bezier(x0, y0, x1, y1, x2, y2, x3,
  y3, color, thickness = 1.0f, segments = 32)`.
- **Design decisions:** The curve is evaluated at uniformly spaced parameter
  values and stroked as an open polyline. A configurable segment count keeps
  rendering deterministic while letting callers trade smoothness for work.
- **Known issues / unverified:** GCC and Clang were not available in `PATH`.

## Follow-up: open polyline rendering

- **Status:** Done. Added open polyline strokes and a zig-zag preview to the
  Windows live demo.
- **Feature:** Added open polyline strokes to `Surface` and an open zig-zag
  preview to the Windows live demo.
- **Rasterization:** Uses the shared path-stroke rasterizer with an open final
  endpoint; pixel centers are tested against each segment and blended once.
  Polygon outlines remain implicitly closed.
- **Invalid input:** Null buffers, fewer than two vertices, non-finite
  coordinates, invalid clips, and non-positive/non-finite thickness draw
  nothing.
- **Files changed:** `include/glimmer/glimmer.hpp`, `tests/test_glimmer.cpp`,
  `examples/live_demo_win32.cpp`, `README.md`, and `PROGRESS.md`.
- **Tests:** MSVC Debug build and Release test-target build succeeded. Debug
  and Release CTest passed; each suite reports 108/108 cases and 9899/9899
  assertions. The Windows demo compiled as part of the Debug build.
- **New public API:** `Surface::draw_polyline(const float* vertices,
  std::size_t vertex_count, color, thickness = 1.0f)`. Vertices are supplied as
  interleaved `x, y` float values; unlike polygon outlines, the final vertex is
  not joined back to the first.
- **Design decisions:** Reused the same internal path-stroke rasterizer as
  closed polygon outlines to keep clipping, thickness, and blending behavior
  consistent.
- **Known issues / unverified:** GCC and Clang were not available in `PATH`.

## Follow-up: polygon outline rendering

- **Status:** Done. Added deterministic closed polygon outlines and a preview
  to the Windows live demo.
- **Feature:** Added closed polygon outlines to `Surface` and an outlined
  concave-polygon sample to the Windows live demo. Triangle outlines now use
  the shared polygon-outline implementation.
- **Rasterization:** Samples pixel centers by distance to every segment,
  including the implicit closing edge. Each covered pixel is blended once.
- **Invalid input:** Null buffers, fewer than two vertices, non-finite
  coordinates, invalid clips, and non-positive/non-finite thickness draw
  nothing.
- **Files changed:** `include/glimmer/glimmer.hpp`, `tests/test_glimmer.cpp`,
  `examples/live_demo_win32.cpp`, `README.md`, and `PROGRESS.md`.
- **Tests:** MSVC Debug build and Release test-target build succeeded. Debug
  and Release CTest passed; each suite reports 106/106 cases and 9892/9892
  assertions. The Windows demo compiled as part of the Debug build.
- **New public API:** `Surface::draw_polygon_outline(const float* vertices,
  std::size_t vertex_count, color, thickness = 1.0f)`. Vertices are supplied as
  interleaved `x, y` float values; the path closes automatically. The existing
  triangle-outline API now delegates to this implementation.
- **Design decisions:** Segment-distance tests use a centered thickness and
  blend each pixel once, including at joints. Two vertices form a closed line
  segment; fewer than two vertices draw nothing.
- **Known issues / unverified:** GCC and Clang were not available in `PATH`.

## Follow-up: triangle outline rendering

- **Status:** Done. Added deterministic triangle outlines and an outlined
  triangle sample to the Windows live demo.
- **Feature:** Added deterministic triangle outlines to `Surface` and an
  outlined-triangle sample to the Windows live demo.
- **Rasterization:** Samples pixel centers by distance to each of the three
  segments. Thickness is centered on the edges; each pixel is blended only
  once even where edges meet.
- **Degenerate input:** Collinear vertices render as their segment union, and
  coincident vertices render as a point. Non-finite coordinates or
  non-positive/non-finite thickness draw nothing.
- **Files changed:** `include/glimmer/glimmer.hpp`, `tests/test_glimmer.cpp`,
  `examples/live_demo_win32.cpp`, `README.md`, and `PROGRESS.md`.
- **Tests:** MSVC Debug build and Debug CTest passed: 101/101 cases and
  9843/9843 assertions. The Release test target built, but Release CTest could
  not launch the executable because Windows Application Control blocked it.
- **New public API:** `Surface::draw_triangle_outline(x0, y0, x1, y1, x2, y2,
  color, thickness = 1.0f)`. Existing APIs are unchanged.
- **Design decisions:** Thickness is centered on each segment and applied as
  distance-to-segment coverage. Degenerate triangles render their segment
  union; each pixel is blended once to avoid darker translucent joints.
- **Known issues / unverified:** GCC and Clang were unavailable in `PATH`.

## Follow-up: filled triangle rendering

- **Status:** Done. Added deterministic filled triangles and a preview to the
  Windows live demo.
- **Feature:** Added a deterministic filled-triangle primitive to `Surface` and
  added a triangle preview to the Windows live demo.
- **Rasterization:** Samples pixel centers against the three oriented edges;
  either winding direction produces the same filled region. Existing clipping
  and source-over blending are applied.
- **Invalid input:** Degenerate triangles and triangles with non-finite vertex
  coordinates draw nothing.
- **Files changed:** `include/glimmer/glimmer.hpp`, `tests/test_glimmer.cpp`,
  `examples/live_demo_win32.cpp`, `README.md`, and `PROGRESS.md`.
- **Tests:** MSVC Debug build and Release test-target build succeeded. Debug
  and Release CTest passed; each suite reports 99/99 cases and 9834/9834
  assertions. The Windows demo compiled as part of the Debug build.
- **New public API:** `Surface::draw_filled_triangle(x0, y0, x1, y1, x2, y2,
  color)`. Existing APIs are unchanged.
- **Design decisions:** Boundary pixels are included by non-strict edge tests.
  Degenerate triangles are no-ops rather than being rasterized as lines.
- **Known issues / unverified:** GCC and Clang were not available in `PATH`.
  The running demo process was left untouched.

## Follow-up: filled polygon rendering

- **Status:** Done. Added deterministic filled polygons and a concave polygon
  preview to the Windows live demo.
- **Feature:** Added a general filled-polygon primitive to `Surface` and a
  concave polygon sample to the Windows live demo.
- **Rasterization:** Uses pixel-center even-odd coverage. The closing edge is
  implicit, and winding direction does not affect the result. Existing clip
  rectangles and source-over blending are respected.
- **Invalid input:** Null buffers, fewer than three vertices, non-finite
  coordinates, invalid clips, and off-screen bounds draw nothing.
- **Files changed:** `include/glimmer/glimmer.hpp`, `tests/test_glimmer.cpp`,
  `examples/live_demo_win32.cpp`, `README.md`, and `PROGRESS.md`.
- **Tests:** MSVC Debug build and Release test-target build succeeded. Debug
  and Release CTest passed; each suite reports 104/104 cases and 9883/9883
  assertions. The Windows demo compiled as part of the Debug build.
- **New public API:** `Surface::draw_filled_polygon(const float* vertices,
  std::size_t vertex_count, color)`. Vertices are supplied as interleaved
  `x, y` float values; existing APIs are unchanged.
- **Design decisions:** The even-odd rule was selected as a simple rule that
  supports concave and self-intersecting contours. Polygon boundary pixels
  are included.
- **Known issues / unverified:** GCC and Clang were not available in `PATH`.

## Follow-up: rounded-rectangle outlines

- **Status:** Done. Added deterministic rounded-rectangle borders and a preview
  to the Windows live demo.
- **Feature:** Added deterministic rounded-rectangle borders to `Surface` and
  added a rounded outline preview to the Windows live demo.
- **Rasterization:** Samples pixel centers inside the rounded outer shape and
  outside an inner rounded rectangle inset by the requested thickness.
- **Fallback:** Non-positive radii use the existing rectangular outline
  implementation; positive radii are clamped to half the shorter dimension.
- **Invalid input:** Non-finite coordinates, dimensions, radius, or thickness,
  and non-positive dimensions or thickness, draw nothing.
- **Files changed:** `include/glimmer/glimmer.hpp`, `tests/test_glimmer.cpp`,
  `examples/live_demo_win32.cpp`, `README.md`, and `PROGRESS.md`.
- **Tests:** MSVC Debug build and Release test-target build succeeded. Debug
  and Release CTest passed; each suite reports 97/97 cases and 9798/9798
  assertions. The Windows demo compiled as part of the Debug build.
- **New public API:** `Surface::draw_rounded_rect_outline(Rect, radius, color,
  thickness = 1.0f)`. Existing APIs are unchanged.
- **Design decisions:** The border is formed by subtracting an inward-inset
  rounded rectangle. Non-positive radii fall back to `outline_rect`, matching
  the filled rounded rectangle's square-corner behavior.
- **Known issues / unverified:** GCC and Clang were not available in `PATH`.

## Follow-up: pie chart widget

- **Status:** Done. Added a deterministic, non-interactive pie chart widget and
  added a category-distribution sample to the Windows live demo.
- **Behavior:** Positive finite values determine proportional clockwise slices,
  starting at twelve o'clock. Non-positive and non-finite values are ignored;
  palette colors repeat when there are fewer colors than values. The chart is
  centered in a clipped, themed layout row.
- **Files changed:** `include/glimmer/glimmer.hpp`, `tests/test_glimmer.cpp`,
  `examples/live_demo_win32.cpp`, `README.md`, and `PROGRESS.md`.
- **Tests:** MSVC Debug build and test executable passed: 112/112 cases and
  9919/9919
  assertions. The Windows live demo and settings example also built
  successfully.
- **New public API:** `Context::pie_chart(const std::vector<float>& values,
  const std::vector<std::uint32_t>& colors, float height = 72.0f)`.
- **Design decisions:** The drawing starts at twelve o'clock and proceeds
  clockwise to match screen coordinates. Invalid values are skipped rather
  than invalidating the complete chart; the color for each slice is indexed by
  its original value position.
- **Known issues / unverified:** Cross-compiler builds have not yet been run
  for this milestone.

## Follow-up: scatter plot chart

- **Status:** Done. Added a non-interactive scatter plot chart with range
  mapping, finite-point filtering, and clipping to its plot area.
- **Behavior:** Point coordinates map into caller-supplied x/y ranges, with
  screen y inverted. Reversed ranges are normalized; equal bounds map matching
  coordinates to the axis center. Non-finite points, non-finite bounds, and
  points outside either range are omitted.
- **Files changed:** `include/glimmer/glimmer.hpp`, `tests/test_glimmer.cpp`,
  `examples/live_demo_win32.cpp`, `README.md`, and `PROGRESS.md`.
- **Tests:** MSVC Debug build and CTest passed: 117/117 cases and 9933/9933
  assertions. The Windows live demo and settings example also built
  successfully.
- **New public API:** `Context::scatter_plot(
  const std::vector<std::pair<float, float>>& points, float minimum_x,
  float maximum_x, float minimum_y, float maximum_y, float height = 48.0f)`.
- **Design decisions:** Out-of-range points are omitted instead of clamped to
  plot edges, keeping markers faithful to the requested data domain. Marker
  radius is one pixel and uses the theme accent color.
- **Known issues / unverified:** Cross-compiler builds have not yet been run
  for this milestone.

## Follow-up: donut chart widget

- **Status:** Done. Added a donut-style companion to the pie chart, sharing its
  slice rasterization, palette cycling, clipping, and invalid-value handling.
- **Behavior:** The ring leaves a centered hole with an inner radius equal to
  half the outer radius. The hole displays the chart's themed background.
- **Files changed:** `include/glimmer/glimmer.hpp`, `tests/test_glimmer.cpp`,
  `examples/live_demo_win32.cpp`, `README.md`, and `PROGRESS.md`.
- **Tests:** MSVC Debug build and CTest passed: 114/114 cases and 9924/9924
  assertions. The Windows live demo and settings example also built
  successfully.
- **New public API:** `Context::donut_chart(const std::vector<float>& values,
  const std::vector<std::uint32_t>& colors, float height = 72.0f)`.
- **Design decisions:** The hole is fixed at half the outer radius to keep the
  API small and maintain a consistent ring appearance. Slice direction,
  ordering, palette mapping, and invalid-value behavior match `pie_chart`.
- **Known issues / unverified:** Cross-compiler builds have not yet been run
  for this milestone.

## Expansion: dockable workspace and profiler timeline

- **Status:** Done. Added a resizable dockspace with two tabbed groups,
  drag-to-dock between groups, and a CPU profiler timeline visualizer.
- **Docking behavior:** `DockspaceState` preserves panel placement and active
  tabs between frames. The splitter ratio persists and is clamped to keep both
  panes usable. Each frame, callers register panel content callbacks; only the
  active panel in each group is drawn and receives input.
- **Profiler behavior:** `ProfileEvent` provides a named start/duration interval
  in milliseconds. `profiler_timeline` draws clipped frame-relative intervals
  and a configurable frame-budget marker. Timings are supplied by the
  application; the library does not use a clock or add nondeterministic
  instrumentation. The Windows demo uses fixed sample data.
- **Files changed:** `include/glimmer/glimmer.hpp`, `tests/test_glimmer.cpp`,
  `examples/live_demo_win32.cpp`, `README.md`, and `PROGRESS.md`.
- **Tests:** MSVC Debug build and CTest passed: 122/122 cases and 9947/9947
  assertions. The Windows live demo and settings example built successfully.
- **New public API:** `DockSide`, `DockPanel`, `DockspaceState`,
  `Context::begin_dockspace`, `Context::dock_panel`, `Context::end_dockspace`,
  `ProfileEvent`, and `Context::profiler_timeline`.
- **Design decisions:** Docking uses two resizable groups rather than an
  unrestricted split tree; this keeps the first dock manager small while
  providing common tab, drag-to-dock, and resize workflows. The profile view
  accepts host-measured CPU intervals and makes no GPU timing claims.
- **Known issues / unverified:** This milestone was built and tested with MSVC
  only; GCC and Clang validation was not run.
