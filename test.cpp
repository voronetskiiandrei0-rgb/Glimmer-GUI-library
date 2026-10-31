// Paint Tool - Windows Paint-like application using the Glimmer GUI library.
//copyright (c) 2026 Glimmer Developers. All rights reserved.
// Compiled as my_app via CMakeLists.txt:
//   cmake --build build --config Release --target my_app

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include "glimmer/glimmer.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

// ─────────────────────────────────────────────────────────────────────────────
// Constants
// ─────────────────────────────────────────────────────────────────────────────
namespace {

constexpr int kWinWidth  = 1100;
constexpr int kWinHeight = 700;

constexpr int kToolPanelW = 180;
constexpr int kCanvasX    = kToolPanelW;
constexpr int kCanvasY    = 0;
constexpr int kCanvasW    = kWinWidth  - kToolPanelW;
constexpr int kCanvasH    = kWinHeight;

// Frame timer id
constexpr UINT_PTR kFrameTimer = 1U;

// ─────────────────────────────────────────────────────────────────────────────
// Tool enum
// ─────────────────────────────────────────────────────────────────────────────
enum class Tool {
    Pencil = 0,
    Brush,
    Eraser,
    Line,
    Rectangle,
    FilledRectangle,
    Circle,
    FilledCircle,
    Fill,
    ColorPicker,
    COUNT
};

const std::vector<std::string> kToolNames{
    "Pencil",
    "Brush",
    "Eraser",
    "Line",
    "Rectangle",
    "Filled Rect",
    "Circle",
    "Filled Circle",
    "Fill",
    "Color Picker",
};

// ─────────────────────────────────────────────────────────────────────────────
// Color palette
// ─────────────────────────────────────────────────────────────────────────────
const std::vector<std::uint32_t> kPalette{
    0xFF000000U, 0xFF808080U, 0xFFFFFFFFU, 0xFFC0C0C0U,
    0xFF800000U, 0xFFFF0000U, 0xFF804000U, 0xFFFF8040U,
    0xFF808000U, 0xFFFFFF00U, 0xFF008000U, 0xFF00FF00U,
    0xFF008080U, 0xFF00FFFFU, 0xFF000080U, 0xFF0000FFU,
    0xFF800080U, 0xFFFF00FFU, 0xFF804040U, 0xFFFF8080U,
    0xFF408080U, 0xFF80FFFFU, 0xFF004080U, 0xFF0080FFU,
};

// ─────────────────────────────────────────────────────────────────────────────
// Application state
// ─────────────────────────────────────────────────────────────────────────────

// Glimmer UI context (full window)
glm::Context g_ui(kWinWidth, kWinHeight);

// The paint canvas – a separate Surface we draw into
glm::Surface g_canvas(kCanvasW, kCanvasH);

// DIB pixel buffer for Win32 StretchDIBits
std::vector<std::uint32_t> g_dib(static_cast<std::size_t>(kWinWidth) *
                                  static_cast<std::size_t>(kWinHeight));

// Input state forwarded from Win32
int  g_mx       = 0;
int  g_my       = 0;
bool g_ldown    = false;

// Tool state
Tool g_tool           = Tool::Pencil;
int  g_selected_tool  = 0;
std::uint32_t g_fg_color = 0xFF000000U; // foreground (packed AABBGGRR)
std::uint32_t g_bg_color = 0xFFFFFFFFU; // background
std::size_t   g_fg_palette_sel = 0U;
std::size_t   g_bg_palette_sel = 2U;
float         g_brush_size     = 4.0f;  // slider value 1-30
int           g_brush_size_int = 4;

// Canvas interaction – previous mouse pos for stroke
int  g_prev_cx = -1;
int  g_prev_cy = -1;
bool g_drawing = false;

// For shape tools: start point
int  g_shape_sx = 0;
int  g_shape_sy = 0;

// Preview canvas (used to show shape preview without committing)
glm::Surface g_preview_canvas(kCanvasW, kCanvasH);
bool         g_preview_valid = false;

// Undo stack (simple: store raw pixel buffers)
constexpr std::size_t kMaxUndo = 20U;
std::vector<std::vector<std::uint32_t>> g_undo_stack;
std::vector<std::vector<std::uint32_t>> g_redo_stack;

// Status bar text
std::string g_status = "Ready";

// ─────────────────────────────────────────────────────────────────────────────
// Helpers
// ─────────────────────────────────────────────────────────────────────────────

// Push current canvas pixels onto the undo stack
void push_undo() {
    const std::uint32_t* px = g_canvas.pixels();
    std::size_t n = static_cast<std::size_t>(g_canvas.width()) *
                    static_cast<std::size_t>(g_canvas.height());
    g_undo_stack.push_back(std::vector<std::uint32_t>(px, px + n));
    if (g_undo_stack.size() > kMaxUndo) {
        g_undo_stack.erase(g_undo_stack.begin());
    }
    g_redo_stack.clear();
}

void do_undo() {
    if (g_undo_stack.empty()) return;
    // Save current for redo
    const std::uint32_t* px = g_canvas.pixels();
    std::size_t n = static_cast<std::size_t>(g_canvas.width()) *
                    static_cast<std::size_t>(g_canvas.height());
    g_redo_stack.push_back(std::vector<std::uint32_t>(px, px + n));

    // Restore last undo state
    auto& state = g_undo_stack.back();
    std::uint32_t* dst = g_canvas.pixels();
    std::memcpy(dst, state.data(), n * sizeof(std::uint32_t));
    g_undo_stack.pop_back();
}

void do_redo() {
    if (g_redo_stack.empty()) return;
    const std::uint32_t* px = g_canvas.pixels();
    std::size_t n = static_cast<std::size_t>(g_canvas.width()) *
                    static_cast<std::size_t>(g_canvas.height());
    g_undo_stack.push_back(std::vector<std::uint32_t>(px, px + n));

    auto& state = g_redo_stack.back();
    std::uint32_t* dst = g_canvas.pixels();
    std::memcpy(dst, state.data(), n * sizeof(std::uint32_t));
    g_redo_stack.pop_back();
}

// Flood fill using the canvas pixel buffer directly
void flood_fill(int cx, int cy, std::uint32_t fill_color) {
    int w = g_canvas.width();
    int h = g_canvas.height();
    if (cx < 0 || cy < 0 || cx >= w || cy >= h) return;

    std::uint32_t* px = g_canvas.pixels();
    std::uint32_t  target = px[cy * w + cx];
    if (target == fill_color) return;

    // Iterative BFS
    std::vector<int> stack;
    stack.reserve(4096);
    stack.push_back(cy * w + cx);

    while (!stack.empty()) {
        int idx = stack.back();
        stack.pop_back();
        if (idx < 0 || idx >= w * h) continue;
        if (px[idx] != target) continue;
        px[idx] = fill_color;

        int x = idx % w;
        int y = idx / w;
        if (x > 0)     stack.push_back(idx - 1);
        if (x < w - 1) stack.push_back(idx + 1);
        if (y > 0)     stack.push_back(idx - w);
        if (y < h - 1) stack.push_back(idx + w);
    }
}

// Erase color = background color on the canvas (fully opaque white by default)
std::uint32_t erase_color() { return g_bg_color; }

// Canvas-local mouse coordinates (subtract tool panel offset)
int canvas_mx() { return g_mx - kCanvasX; }
int canvas_my() { return g_my - kCanvasY; }

// Is the mouse inside the canvas area?
bool in_canvas() {
    int cx = canvas_mx(), cy = canvas_my();
    return cx >= 0 && cy >= 0 && cx < kCanvasW && cy < kCanvasH;
}

// Brush draw: a filled circle stroke on the canvas
void draw_brush_stroke(glm::Surface& surf, int x0, int y0, int x1, int y1,
                        std::uint32_t color, int radius) {
    // Draw filled circles along the line from (x0,y0) to (x1,y1)
    int dx = x1 - x0, dy = y1 - y0;
    int steps = std::max(1, std::max(std::abs(dx), std::abs(dy)));
    for (int i = 0; i <= steps; ++i) {
        int bx = x0 + dx * i / steps;
        int by = y0 + dy * i / steps;
        surf.draw_filled_circle(bx, by, radius, color);
    }
}

// Thin pencil line
void draw_pencil_stroke(glm::Surface& surf, int x0, int y0, int x1, int y1,
                         std::uint32_t color) {
    surf.draw_line(x0, y0, x1, y1, color);
}

// Copy pixels from src surface to dst surface
void blit_surface(glm::Surface& dst, const glm::Surface& src) {
    std::memcpy(dst.pixels(),
                src.pixels(),
                static_cast<std::size_t>(src.width()) *
                static_cast<std::size_t>(src.height()) *
                sizeof(std::uint32_t));
}

// Initialize the canvas to the background color
void clear_canvas() {
    g_canvas.fill_rect({0.0f, 0.0f,
                         static_cast<float>(kCanvasW),
                         static_cast<float>(kCanvasH)},
                        0xFFFFFFFFU);
}

// ─────────────────────────────────────────────────────────────────────────────
// Handle canvas mouse interaction
// ─────────────────────────────────────────────────────────────────────────────
void on_canvas_down(int cx, int cy) {
    g_shape_sx = cx;
    g_shape_sy = cy;
    g_drawing  = true;

    switch (g_tool) {
        case Tool::Pencil:
            push_undo();
            draw_pencil_stroke(g_canvas, cx, cy, cx, cy, g_fg_color);
            break;
        case Tool::Brush:
            push_undo();
            draw_brush_stroke(g_canvas, cx, cy, cx, cy, g_fg_color, g_brush_size_int);
            break;
        case Tool::Eraser:
            push_undo();
            draw_brush_stroke(g_canvas, cx, cy, cx, cy, erase_color(), g_brush_size_int);
            break;
        case Tool::Fill:
            push_undo();
            flood_fill(cx, cy, g_fg_color);
            break;
        case Tool::ColorPicker: {
            int w = g_canvas.width();
            if (cx >= 0 && cy >= 0 && cx < w && cy < g_canvas.height()) {
                g_fg_color = g_canvas.pixels()[cy * w + cx];
            }
            g_drawing = false;
            g_status  = "Color picked";
            break;
        }
        default:
            // Shape tools: snapshot canvas for preview
            push_undo();
            blit_surface(g_preview_canvas, g_canvas);
            g_preview_valid = true;
            break;
    }
    g_prev_cx = cx;
    g_prev_cy = cy;
}

void on_canvas_move(int cx, int cy) {
    if (!g_drawing) return;

    switch (g_tool) {
        case Tool::Pencil:
            draw_pencil_stroke(g_canvas, g_prev_cx, g_prev_cy, cx, cy, g_fg_color);
            break;
        case Tool::Brush:
            draw_brush_stroke(g_canvas, g_prev_cx, g_prev_cy, cx, cy, g_fg_color, g_brush_size_int);
            break;
        case Tool::Eraser:
            draw_brush_stroke(g_canvas, g_prev_cx, g_prev_cy, cx, cy, erase_color(), g_brush_size_int);
            break;
        case Tool::Line:
        case Tool::Rectangle:
        case Tool::FilledRectangle:
        case Tool::Circle:
        case Tool::FilledCircle:
            // Restore base and draw preview shape
            if (g_preview_valid) {
                blit_surface(g_canvas, g_preview_canvas);
                float sx = static_cast<float>(g_shape_sx);
                float sy = static_cast<float>(g_shape_sy);
                float ex = static_cast<float>(cx);
                float ey = static_cast<float>(cy);
                if (g_tool == Tool::Line) {
                    g_canvas.draw_line(g_shape_sx, g_shape_sy, cx, cy, g_fg_color);
                } else if (g_tool == Tool::Rectangle) {
                    float rx = std::min(sx, ex);
                    float ry = std::min(sy, ey);
                    float rw = std::abs(ex - sx);
                    float rh = std::abs(ey - sy);
                    g_canvas.outline_rect({rx, ry, rw, rh}, g_fg_color, 2.0f);
                } else if (g_tool == Tool::FilledRectangle) {
                    float rx = std::min(sx, ex);
                    float ry = std::min(sy, ey);
                    float rw = std::abs(ex - sx);
                    float rh = std::abs(ey - sy);
                    g_canvas.fill_rect({rx, ry, rw, rh}, g_fg_color);
                } else if (g_tool == Tool::Circle) {
                    int rad = static_cast<int>(std::sqrt(
                        static_cast<float>((cx - g_shape_sx) * (cx - g_shape_sx) +
                                           (cy - g_shape_sy) * (cy - g_shape_sy))));
                    g_canvas.draw_circle_outline(g_shape_sx, g_shape_sy, rad, g_fg_color, 2);
                } else if (g_tool == Tool::FilledCircle) {
                    int rad = static_cast<int>(std::sqrt(
                        static_cast<float>((cx - g_shape_sx) * (cx - g_shape_sx) +
                                           (cy - g_shape_sy) * (cy - g_shape_sy))));
                    g_canvas.draw_filled_circle(g_shape_sx, g_shape_sy, rad, g_fg_color);
                }
            }
            break;
        default:
            break;
    }

    g_prev_cx = cx;
    g_prev_cy = cy;
}

void on_canvas_up(int cx, int cy) {
    if (!g_drawing) return;

    // Finalize shape tools
    switch (g_tool) {
        case Tool::Line:
            g_canvas.draw_line(g_shape_sx, g_shape_sy, cx, cy, g_fg_color);
            break;
        case Tool::Rectangle: {
            float sx = static_cast<float>(g_shape_sx);
            float sy = static_cast<float>(g_shape_sy);
            float ex = static_cast<float>(cx);
            float ey = static_cast<float>(cy);
            float rx = std::min(sx, ex), ry = std::min(sy, ey);
            float rw = std::abs(ex - sx), rh = std::abs(ey - sy);
            g_canvas.outline_rect({rx, ry, rw, rh}, g_fg_color, 2.0f);
            break;
        }
        case Tool::FilledRectangle: {
            float sx = static_cast<float>(g_shape_sx);
            float sy = static_cast<float>(g_shape_sy);
            float ex = static_cast<float>(cx);
            float ey = static_cast<float>(cy);
            float rx = std::min(sx, ex), ry = std::min(sy, ey);
            float rw = std::abs(ex - sx), rh = std::abs(ey - sy);
            g_canvas.fill_rect({rx, ry, rw, rh}, g_fg_color);
            break;
        }
        case Tool::Circle: {
            int rad = static_cast<int>(std::sqrt(
                static_cast<float>((cx - g_shape_sx) * (cx - g_shape_sx) +
                                   (cy - g_shape_sy) * (cy - g_shape_sy))));
            if (g_preview_valid) blit_surface(g_canvas, g_preview_canvas);
            g_canvas.draw_circle_outline(g_shape_sx, g_shape_sy, rad, g_fg_color, 2);
            break;
        }
        case Tool::FilledCircle: {
            int rad = static_cast<int>(std::sqrt(
                static_cast<float>((cx - g_shape_sx) * (cx - g_shape_sx) +
                                   (cy - g_shape_sy) * (cy - g_shape_sy))));
            if (g_preview_valid) blit_surface(g_canvas, g_preview_canvas);
            g_canvas.draw_filled_circle(g_shape_sx, g_shape_sy, rad, g_fg_color);
            break;
        }
        default:
            break;
    }

    g_drawing       = false;
    g_preview_valid = false;
}

// ─────────────────────────────────────────────────────────────────────────────
// Render one frame
// ─────────────────────────────────────────────────────────────────────────────
void render_frame() {
    g_ui.begin(g_mx, g_my, g_ldown);

    // ── Tool panel (left side) ────────────────────────────────────────────────
    g_ui.window("Paint Tools", {0.0f, 0.0f,
                                  static_cast<float>(kToolPanelW),
                                  static_cast<float>(kWinHeight)}, [&] {

        g_ui.label("Tools");
        g_ui.separator();

        // Radio-style tool selection via selectable rows
        for (int i = 0; i < static_cast<int>(Tool::COUNT); ++i) {
            bool sel = (g_selected_tool == i);
            if (g_ui.selectable(kToolNames[static_cast<std::size_t>(i)], sel).clicked) {
                g_selected_tool = i;
                g_tool = static_cast<Tool>(i);
            }
        }

        g_ui.separator();
        g_ui.label("Brush size");
        if (g_ui.slider("BrushSz", g_brush_size, 1.0f, 30.0f).changed) {
            g_brush_size_int = static_cast<int>(g_brush_size + 0.5f);
        }
        g_ui.label("Size: " + std::to_string(g_brush_size_int));

        g_ui.separator();
        g_ui.label("Foreground");
        {
            // Draw the foreground color swatch
            glm::Rect swatch = g_ui.row(24.0f);
            g_ui.image(&g_fg_color, 1, 1, swatch);
        }
        g_ui.color_picker("FgColor", g_fg_color);

        g_ui.separator();
        g_ui.label("Background");
        {
            glm::Rect swatch = g_ui.row(24.0f);
            g_ui.image(&g_bg_color, 1, 1, swatch);
        }
        g_ui.color_picker("BgColor", g_bg_color);

        g_ui.separator();
        g_ui.label("Palette");
        g_ui.color_palette("Palette", kPalette, g_fg_palette_sel, 4U);
        if (g_fg_palette_sel < kPalette.size()) {
            g_fg_color = kPalette[g_fg_palette_sel];
        }

        g_ui.separator();
        if (g_ui.button("Clear Canvas").clicked) {
            push_undo();
            clear_canvas();
        }
        if (g_ui.button("Undo").clicked) { do_undo(); }
        if (g_ui.button("Redo").clicked) { do_redo(); }

        g_ui.separator();
        g_ui.label(g_status);
    });

    // ── Canvas area (right side) ──────────────────────────────────────────────
    // We blit the canvas surface into the UI context at the canvas rectangle.
    // The canvas is a separate Surface; we draw into it directly and blit each frame.
    glm::Rect canvas_rect{
        static_cast<float>(kCanvasX), static_cast<float>(kCanvasY),
        static_cast<float>(kCanvasW), static_cast<float>(kCanvasH)
    };
    // Draw a white background behind the canvas
    g_ui.image(g_canvas, canvas_rect);

    // Update status bar with mouse position
    if (in_canvas()) {
        int cx = canvas_mx(), cy = canvas_my();
        g_status = "(" + std::to_string(cx) + ", " + std::to_string(cy) + ")  " +
                   kToolNames[g_selected_tool];
    } else {
        g_status = kToolNames[g_selected_tool];
    }

    g_ui.end();

    // ── Compose UI + canvas → DIB ─────────────────────────────────────────────
    // The g_ui.pixels() buffer contains the rendered UI frame.
    // Convert from AABBGGRR → GDI BBGGRR (top-down).
    const std::uint32_t* src = g_ui.pixels();
    for (std::size_t i = 0; i < g_dib.size(); ++i) {
        std::uint32_t p     = src[i];
        std::uint32_t r     = (p      ) & 0xFFU;
        std::uint32_t gr    = (p >>  8) & 0xFFU;
        std::uint32_t b     = (p >> 16) & 0xFFU;
        g_dib[i] = (r << 16) | (gr << 8) | b;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Win32 glue
// ─────────────────────────────────────────────────────────────────────────────
void present_frame(HDC dc) {
    render_frame();
    BITMAPINFO bi{};
    bi.bmiHeader.biSize        = sizeof(bi.bmiHeader);
    bi.bmiHeader.biWidth       = kWinWidth;
    bi.bmiHeader.biHeight      = -kWinHeight;   // top-down
    bi.bmiHeader.biPlanes      = 1;
    bi.bmiHeader.biBitCount    = 32;
    bi.bmiHeader.biCompression = BI_RGB;
    StretchDIBits(dc,
                  0, 0, kWinWidth, kWinHeight,
                  0, 0, kWinWidth, kWinHeight,
                  g_dib.data(), &bi, DIB_RGB_COLORS, SRCCOPY);
}

void update_mouse(LPARAM lp) {
    g_mx = static_cast<int>(static_cast<short>(LOWORD(lp)));
    g_my = static_cast<int>(static_cast<short>(HIWORD(lp)));
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
        case WM_PAINT: {
            PAINTSTRUCT ps{};
            HDC dc = BeginPaint(hwnd, &ps);
            present_frame(dc);
            EndPaint(hwnd, &ps);
            return 0;
        }
        case WM_ERASEBKGND:
            return 1;
        case WM_TIMER:
            if (wp == kFrameTimer) InvalidateRect(hwnd, nullptr, FALSE);
            return 0;
        case WM_MOUSEMOVE:
            update_mouse(lp);
            if (g_ldown && in_canvas()) {
                on_canvas_move(canvas_mx(), canvas_my());
                InvalidateRect(hwnd, nullptr, FALSE);
            }
            return 0;
        case WM_LBUTTONDOWN:
            update_mouse(lp);
            g_ldown = true;
            SetCapture(hwnd);
            if (in_canvas()) {
                on_canvas_down(canvas_mx(), canvas_my());
            }
            InvalidateRect(hwnd, nullptr, FALSE);
            return 0;
        case WM_LBUTTONUP:
            update_mouse(lp);
            if (g_ldown && in_canvas()) {
                on_canvas_up(canvas_mx(), canvas_my());
            }
            g_ldown = false;
            ReleaseCapture();
            InvalidateRect(hwnd, nullptr, FALSE);
            return 0;
        case WM_CAPTURECHANGED:
            g_ldown = false;
            return 0;
        case WM_MOUSEWHEEL: {
            int delta = GET_WHEEL_DELTA_WPARAM(wp);
            POINT pt{
                static_cast<LONG>(static_cast<short>(LOWORD(lp))),
                static_cast<LONG>(static_cast<short>(HIWORD(lp))),
            };
            ScreenToClient(hwnd, &pt);
            g_mx = static_cast<int>(pt.x);
            g_my = static_cast<int>(pt.y);
            g_ui.feed_wheel(delta);
            InvalidateRect(hwnd, nullptr, FALSE);
            return 0;
        }
        case WM_CHAR:
            if (wp >= 32U && wp <= 126U) {
                g_ui.feed_char(static_cast<char>(wp));
                InvalidateRect(hwnd, nullptr, FALSE);
            }
            return 0;
        case WM_KEYDOWN:
            switch (wp) {
                case VK_TAB:
                    g_ui.feed_key(glm::Key::Tab, (GetKeyState(VK_SHIFT) & 0x8000) != 0);
                    break;
                case VK_BACK:   g_ui.feed_key(glm::Key::Backspace); break;
                case VK_DELETE: g_ui.feed_key(glm::Key::Delete);    break;
                case VK_LEFT:   g_ui.feed_key(glm::Key::Left);      break;
                case VK_RIGHT:  g_ui.feed_key(glm::Key::Right);     break;
                case VK_HOME:   g_ui.feed_key(glm::Key::Home);      break;
                case VK_END:    g_ui.feed_key(glm::Key::End);       break;
                case VK_RETURN: g_ui.feed_key(glm::Key::Enter);     break;
                case VK_SPACE:  g_ui.feed_key(glm::Key::Space);     break;
                case VK_UP:     g_ui.feed_key(glm::Key::Up);        break;
                case VK_DOWN:   g_ui.feed_key(glm::Key::Down);      break;
                case VK_ESCAPE: g_ui.feed_key(glm::Key::Escape);    break;
                // Ctrl+Z = undo, Ctrl+Y = redo
                case 'Z':
                    if (GetKeyState(VK_CONTROL) & 0x8000) { do_undo(); }
                    break;
                case 'Y':
                    if (GetKeyState(VK_CONTROL) & 0x8000) { do_redo(); }
                    break;
                default:
                    return DefWindowProcW(hwnd, msg, wp, lp);
            }
            InvalidateRect(hwnd, nullptr, FALSE);
            return 0;
        case WM_DESTROY:
            KillTimer(hwnd, kFrameTimer);
            PostQuitMessage(0);
            return 0;
        default:
            return DefWindowProcW(hwnd, msg, wp, lp);
    }
}

}  // namespace

// ─────────────────────────────────────────────────────────────────────────────
// Entry point
// ─────────────────────────────────────────────────────────────────────────────
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nShow) {
    // Fill canvas with white
    clear_canvas();
    // Init preview canvas too
    g_preview_canvas.fill_rect(
        {0.0f, 0.0f,
         static_cast<float>(kCanvasW),
         static_cast<float>(kCanvasH)},
        0xFFFFFFFFU);

    const wchar_t* kClass = L"GlimmerPaintWindow";
    WNDCLASSW wc{};
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInst;
    wc.hCursor       = LoadCursorW(nullptr, MAKEINTRESOURCEW(32512));
    wc.hbrBackground = nullptr;
    wc.lpszClassName = kClass;
    if (RegisterClassW(&wc) == 0U) {
        MessageBoxW(nullptr, L"Window class registration failed.",
                    L"Paint", MB_OK | MB_ICONERROR);
        return 1;
    }

    RECT bounds{0, 0, kWinWidth, kWinHeight};
    AdjustWindowRect(&bounds,
                     WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
                     FALSE);

    HWND hwnd = CreateWindowExW(
        0U, kClass, L"Glimmer Paint",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT,
        bounds.right  - bounds.left,
        bounds.bottom - bounds.top,
        nullptr, nullptr, hInst, nullptr);

    if (!hwnd) {
        MessageBoxW(nullptr, L"Window creation failed.",
                    L"Paint", MB_OK | MB_ICONERROR);
        return 1;
    }

    ShowWindow(hwnd, nShow);
    UpdateWindow(hwnd);

    // ~30 fps timer
    if (SetTimer(hwnd, kFrameTimer, 33U, nullptr) == 0U) {
        MessageBoxW(hwnd, L"Could not start frame timer.",
                    L"Paint", MB_OK | MB_ICONERROR);
        DestroyWindow(hwnd);
        return 1;
    }

    MSG m{};
    int res = 0;
    while ((res = static_cast<int>(GetMessageW(&m, nullptr, 0U, 0U))) > 0) {
        TranslateMessage(&m);
        DispatchMessageW(&m);
    }
    return res < 0 ? 1 : static_cast<int>(m.wParam);
}
