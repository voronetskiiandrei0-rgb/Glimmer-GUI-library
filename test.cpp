// ════════════════════════════════════════════════════════════════════════════
// Glimmer Paint  –  a Windows-Paint-style demo using the Glimmer GUI library
// Build:  cmake --build build --config Release --target my_app
// ════════════════════════════════════════════════════════════════════════════

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include "glimmer/glimmer.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

// ─────────────────────────────────────────────────────────────────────────────
// Dynamic Window & Canvas dimensions
// ─────────────────────────────────────────────────────────────────────────────
namespace {

int g_win_w       = 1200;
int g_win_h       = 720;
constexpr int kToolW      = 220;   // left tool panel width
constexpr int kStatusH    = 24;    // bottom status bar height
constexpr int kMenuH      = 32;    // top menu/toolbar strip height

int canvas_x() { return kToolW; }
int canvas_y() { return kMenuH; }
int canvas_w() { return std::max(10, g_win_w - kToolW); }
int canvas_h() { return std::max(10, g_win_h - kMenuH - kStatusH); }

constexpr UINT_PTR kTimer = 1U;

// ─────────────────────────────────────────────────────────────────────────────
// Tool definitions
// ─────────────────────────────────────────────────────────────────────────────
enum class Tool {
    Pencil = 0,
    Brush,
    Spray,
    Eraser,
    Line,
    Rectangle,
    FilledRect,
    RoundRect,
    Circle,
    FilledCircle,
    Ellipse,
    Fill,
    ColorPicker,
    COUNT
};

const std::vector<std::string> kToolNames = {
    "Pencil", "Brush", "Spray", "Eraser",
    "Line", "Rectangle", "Filled Rect", "Round Rect",
    "Circle", "Filled Circle", "Ellipse",
    "Fill", "Color Pick",
};

// 24-colour palette (classic Paint style)
const std::vector<std::uint32_t> kPalette = {
    0xFF000000U, 0xFF808080U, 0xFFFFFFFFU, 0xFFC0C0C0U,
    0xFF800000U, 0xFFFF0000U, 0xFF804000U, 0xFFFF8040U,
    0xFF808000U, 0xFFFFFF00U, 0xFF008000U, 0xFF00FF00U,
    0xFF008080U, 0xFF00FFFFU, 0xFF000080U, 0xFF0000FFU,
    0xFF800080U, 0xFFFF00FFU, 0xFF804040U, 0xFFFF8080U,
    0xFF408080U, 0xFF80FFFFU, 0xFF004080U, 0xFF0080FFU,
};

// ─────────────────────────────────────────────────────────────────────────────
// App globals
// ─────────────────────────────────────────────────────────────────────────────
glm::Context g_ui(1200, 720);
glm::Surface g_canvas(1200 - kToolW, 720 - kMenuH - kStatusH);
glm::Surface g_preview(1200 - kToolW, 720 - kMenuH - kStatusH);

std::vector<std::uint32_t> g_dib;

// Input
int  g_mx = 0, g_my = 0;
bool g_ldown = false;

// Tool state
Tool        g_tool         = Tool::Pencil;
int         g_tool_idx     = 0;
std::uint32_t g_fg         = 0xFF000000U;
std::uint32_t g_bg         = 0xFFFFFFFFU;
std::size_t g_fg_pal       = 0U;
float       g_size_f       = 3.0f;
int         g_size         = 3;
float       g_opacity_f    = 1.0f;
int         g_ui_scale     = 1;

// Drawing state
int  g_px = -1, g_py = -1;
int  g_sx = 0,  g_sy = 0;
bool g_drawing      = false;
bool g_preview_live = false;

// Undo / redo
constexpr std::size_t kMaxUndo = 30U;
std::vector<std::vector<std::uint32_t>> g_undo, g_redo;

// Status & overlays
std::string g_status = "Ready  |  Glimmer Paint";
bool g_show_preview_panel = true;
bool g_show_grid = false;

// Recent colours
std::vector<std::uint32_t> g_recent_colors;

// ─────────────────────────────────────────────────────────────────────────────
// Helpers
// ─────────────────────────────────────────────────────────────────────────────
void push_undo() {
    const std::uint32_t* px = g_canvas.pixels();
    std::size_t n = static_cast<std::size_t>(g_canvas.width()) *
                    static_cast<std::size_t>(g_canvas.height());
    g_undo.push_back(std::vector<std::uint32_t>(px, px + n));
    if (g_undo.size() > kMaxUndo) g_undo.erase(g_undo.begin());
    g_redo.clear();
}

void do_undo() {
    if (g_undo.empty()) return;
    const std::uint32_t* px = g_canvas.pixels();
    std::size_t n = static_cast<std::size_t>(g_canvas.width()) *
                    static_cast<std::size_t>(g_canvas.height());
    g_redo.push_back(std::vector<std::uint32_t>(px, px + n));
    std::memcpy(g_canvas.pixels(), g_undo.back().data(), n * sizeof(std::uint32_t));
    g_undo.pop_back();
}

void do_redo() {
    if (g_redo.empty()) return;
    const std::uint32_t* px = g_canvas.pixels();
    std::size_t n = static_cast<std::size_t>(g_canvas.width()) *
                    static_cast<std::size_t>(g_canvas.height());
    g_undo.push_back(std::vector<std::uint32_t>(px, px + n));
    std::memcpy(g_canvas.pixels(), g_redo.back().data(), n * sizeof(std::uint32_t));
    g_redo.pop_back();
}

void blit(glm::Surface& dst, const glm::Surface& src) {
    const int copy_w = std::min(dst.width(), src.width());
    const int copy_h = std::min(dst.height(), src.height());
    for (int y = 0; y < copy_h; ++y) {
        std::memcpy(dst.pixels() + y * dst.width(),
                    src.pixels() + y * src.width(),
                    copy_w * sizeof(std::uint32_t));
    }
}

void clear_canvas(std::uint32_t color = 0xFFFFFFFFU) {
    g_canvas.fill_rect({0.0f, 0.0f,
        static_cast<float>(g_canvas.width()), static_cast<float>(g_canvas.height())}, color);
}

void record_color(std::uint32_t c) {
    g_recent_colors.erase(
        std::remove(g_recent_colors.begin(), g_recent_colors.end(), c),
        g_recent_colors.end());
    g_recent_colors.insert(g_recent_colors.begin(), c);
    if (g_recent_colors.size() > 8U) g_recent_colors.resize(8U);
}

int cx() { return g_mx - canvas_x(); }
int cy() { return g_my - canvas_y(); }
bool in_canvas() {
    return cx() >= 0 && cy() >= 0 && cx() < canvas_w() && cy() < canvas_h();
}

void pencil_stroke(glm::Surface& s, int x0, int y0, int x1, int y1, std::uint32_t c) {
    s.draw_line(x0, y0, x1, y1, c);
}

void brush_stroke(glm::Surface& s, int x0, int y0, int x1, int y1, std::uint32_t c, int r) {
    int dx = x1 - x0, dy = y1 - y0;
    int steps = std::max(1, std::max(std::abs(dx), std::abs(dy)));
    for (int i = 0; i <= steps; ++i) {
        s.draw_filled_circle(x0 + dx * i / steps, y0 + dy * i / steps, r, c);
    }
}

void spray_stroke(glm::Surface& s, int cx2, int cy2, std::uint32_t c, int r) {
    for (int i = 0; i < 20; ++i) {
        int rx = (((cx2 * 1103515245 + i * 12345 + 19) >> 4) % (2 * r + 1)) - r;
        int ry = (((cy2 * 1664525  + i * 6547  + 31) >> 3) % (2 * r + 1)) - r;
        if (rx * rx + ry * ry <= r * r) {
            s.draw_filled_circle(cx2 + rx, cy2 + ry, 0, c);
        }
    }
}

void flood_fill(int px2, int py2, std::uint32_t fill_c) {
    int w = g_canvas.width(), h = g_canvas.height();
    if (px2 < 0 || py2 < 0 || px2 >= w || py2 >= h) return;
    std::uint32_t* pix = g_canvas.pixels();
    std::uint32_t  target = pix[py2 * w + px2];
    if (target == fill_c) return;
    std::vector<int> stk;
    stk.reserve(8192);
    stk.push_back(py2 * w + px2);
    while (!stk.empty()) {
        int idx = stk.back(); stk.pop_back();
        if (idx < 0 || idx >= w * h) continue;
        if (pix[idx] != target) continue;
        pix[idx] = fill_c;
        int x2 = idx % w, y2 = idx / w;
        if (x2 > 0)      stk.push_back(idx - 1);
        if (x2 < w - 1)  stk.push_back(idx + 1);
        if (y2 > 0)      stk.push_back(idx - w);
        if (y2 < h - 1)  stk.push_back(idx + w);
    }
}

void commit_shape(int ex, int ey) {
    float sx = static_cast<float>(g_sx), sy = static_cast<float>(g_sy);
    float efx = static_cast<float>(ex), efy = static_cast<float>(ey);
    float rx = std::min(sx, efx), ry = std::min(sy, efy);
    float rw = std::abs(efx - sx), rh = std::abs(efy - sy);

    switch (g_tool) {
        case Tool::Line:
            g_canvas.draw_line(g_sx, g_sy, ex, ey, g_fg);
            break;
        case Tool::Rectangle:
            g_canvas.outline_rect({rx, ry, rw, rh}, g_fg, static_cast<float>(std::max(1, g_size)));
            break;
        case Tool::FilledRect:
            g_canvas.fill_rect({rx, ry, rw, rh}, g_fg);
            break;
        case Tool::RoundRect:
            g_canvas.draw_rounded_rect_outline({rx, ry, rw, rh}, 10.0f, g_fg,
                                               static_cast<float>(std::max(1, g_size)));
            break;
        case Tool::Circle: {
            int rad = static_cast<int>(std::sqrt((ex - g_sx)*(ex - g_sx) + (ey - g_sy)*(ey - g_sy)));
            g_canvas.draw_circle_outline(g_sx, g_sy, rad, g_fg, std::max(1, g_size));
            break;
        }
        case Tool::FilledCircle: {
            int rad = static_cast<int>(std::sqrt((ex - g_sx)*(ex - g_sx) + (ey - g_sy)*(ey - g_sy)));
            g_canvas.draw_filled_circle(g_sx, g_sy, rad, g_fg);
            break;
        }
        case Tool::Ellipse:
            g_canvas.draw_ellipse_outline({rx, ry, rw, rh}, g_fg, static_cast<float>(std::max(1, g_size)));
            break;
        default: break;
    }
}

void update_preview(int ex, int ey) {
    if (!g_preview_live) return;
    blit(g_canvas, g_preview);
    commit_shape(ex, ey);
}

// ─────────────────────────────────────────────────────────────────────────────
// Canvas mouse events
// ─────────────────────────────────────────────────────────────────────────────
void canvas_down(int x, int y) {
    g_sx = x; g_sy = y;
    g_drawing = true;
    record_color(g_fg);

    switch (g_tool) {
        case Tool::Pencil:
            push_undo();
            pencil_stroke(g_canvas, x, y, x, y, g_fg);
            break;
        case Tool::Brush:
            push_undo();
            brush_stroke(g_canvas, x, y, x, y, g_fg, g_size);
            break;
        case Tool::Spray:
            push_undo();
            spray_stroke(g_canvas, x, y, g_fg, g_size * 5);
            break;
        case Tool::Eraser:
            push_undo();
            brush_stroke(g_canvas, x, y, x, y, g_bg, g_size);
            break;
        case Tool::Fill:
            push_undo();
            flood_fill(x, y, g_fg);
            g_drawing = false;
            break;
        case Tool::ColorPicker: {
            if (x >= 0 && y >= 0 && x < g_canvas.width() && y < g_canvas.height()) {
                g_fg = g_canvas.pixels()[y * g_canvas.width() + x];
                record_color(g_fg);
            }
            g_drawing = false;
            break;
        }
        default:
            push_undo();
            blit(g_preview, g_canvas);
            g_preview_live = true;
            break;
    }
    g_px = x; g_py = y;
}

void canvas_move(int x, int y) {
    if (!g_drawing) return;
    switch (g_tool) {
        case Tool::Pencil:
            pencil_stroke(g_canvas, g_px, g_py, x, y, g_fg);
            break;
        case Tool::Brush:
            brush_stroke(g_canvas, g_px, g_py, x, y, g_fg, g_size);
            break;
        case Tool::Spray:
            spray_stroke(g_canvas, x, y, g_fg, g_size * 5);
            break;
        case Tool::Eraser:
            brush_stroke(g_canvas, g_px, g_py, x, y, g_bg, g_size);
            break;
        default:
            update_preview(x, y);
            break;
    }
    g_px = x; g_py = y;
}

void canvas_up(int x, int y) {
    if (!g_drawing) return;
    switch (g_tool) {
        case Tool::Line:
        case Tool::Rectangle:
        case Tool::FilledRect:
        case Tool::RoundRect:
        case Tool::Circle:
        case Tool::FilledCircle:
        case Tool::Ellipse:
            if (g_preview_live) {
                blit(g_canvas, g_preview);
                g_preview_live = false;
            }
            commit_shape(x, y);
            break;
        default: break;
    }
    g_drawing = false;
    g_preview_live = false;
}

void on_window_resize(int new_w, int new_h) {
    if (new_w <= 0 || new_h <= 0) return;
    g_win_w = new_w;
    g_win_h = new_h;

    // Use the library's Context::resize API!
    g_ui.resize(g_win_w, g_win_h);

    // Resize canvas surface preserving contents
    const int cw = canvas_w();
    const int ch = canvas_h();
    g_canvas.resize(cw, ch);
    g_preview.resize(cw, ch);

    g_dib.resize(static_cast<std::size_t>(g_win_w) * static_cast<std::size_t>(g_win_h));
}

// ─────────────────────────────────────────────────────────────────────────────
// Render frame
// ─────────────────────────────────────────────────────────────────────────────
void render_frame() {
    g_ui.begin(g_mx, g_my, g_ldown);

    // ── Top Menu Bar ─────────────────────────────────────────────────────────
    g_ui.window("##menubar", {0.0f, 0.0f, static_cast<float>(g_win_w), static_cast<float>(kMenuH)}, [&] {
        if (g_ui.button("Undo").clicked) do_undo();
        if (g_ui.button("Redo").clicked) do_redo();
        if (g_ui.button("Clear").clicked) {
            push_undo();
            clear_canvas(g_bg);
        }
        if (g_ui.button(g_show_grid ? "Grid: ON" : "Grid: OFF").clicked) {
            g_show_grid = !g_show_grid;
        }
        if (g_ui.button("UI Scale: 1x").clicked) {
            g_ui_scale = 1;
            g_ui.set_scale(1);
        }
        if (g_ui.button("UI Scale: 2x").clicked) {
            g_ui_scale = 2;
            g_ui.set_scale(2);
        }
    });

    // ── Left Tool Panel ──────────────────────────────────────────────────────
    g_ui.window("##tools", {0.0f, static_cast<float>(kMenuH),
                            static_cast<float>(kToolW),
                            static_cast<float>(g_win_h - kMenuH - kStatusH)}, [&] {

        g_ui.section_header("TOOLS");
        g_ui.begin_table("tools_table", 2U);
        for (int i = 0; i < static_cast<int>(Tool::COUNT); ++i) {
            bool sel = (g_tool_idx == i);
            if (g_ui.selectable(kToolNames[static_cast<std::size_t>(i)], sel).clicked) {
                g_tool_idx = i;
                g_tool = static_cast<Tool>(i);
            }
            if (i % 2 == 0 && i + 1 < static_cast<int>(Tool::COUNT)) {
                g_ui.next_table_column();
            } else if (i % 2 == 1) {
                g_ui.next_table_row();
            }
        }
        g_ui.end_table();

        g_ui.next_row(4.0f);
        g_ui.section_header("STROKE");
        g_ui.label("Brush Size: " + std::to_string(g_size) + " px");
        if (g_ui.slider("##size", g_size_f, 1.0f, 40.0f).changed) {
            g_size = static_cast<int>(g_size_f + 0.5f);
        }

        g_ui.next_row(4.0f);
        g_ui.section_header("COLOURS");
        g_ui.label("Foreground");
        g_ui.color_picker("##fgpick", g_fg);
        g_ui.label("Background");
        g_ui.color_picker("##bgpick", g_bg);

        g_ui.next_row(4.0f);
        g_ui.section_header("PALETTE");
        g_ui.color_palette("##pal", kPalette, g_fg_pal, 6U);
        if (g_fg_pal < kPalette.size()) g_fg = kPalette[g_fg_pal];

        if (!g_recent_colors.empty()) {
            g_ui.label("Recent Swatches");
            g_ui.color_palette("##recent", g_recent_colors, g_fg_pal, 8U);
        }

        g_ui.next_row(4.0f);
        g_ui.section_header("CANVAS");
        g_ui.label("Size: " + std::to_string(canvas_w()) + "x" + std::to_string(canvas_h()));
        g_ui.checkbox("Mini Preview", g_show_preview_panel);

        if (g_show_preview_panel && canvas_w() > 0) {
            g_ui.next_row(4.0f);
            g_ui.section_header("MINI PREVIEW");
            float pw = static_cast<float>(kToolW - 14);
            float ph = pw * static_cast<float>(canvas_h()) / static_cast<float>(canvas_w());
            ph = std::min(ph, 110.0f);
            glm::Rect preview_r = g_ui.row(ph);
            g_ui.image(g_canvas, preview_r);
        }
    });

    // ── Paint Canvas ──────────────────────────────────────────────────────────
    {
        glm::Rect canvas_rect{
            static_cast<float>(canvas_x()), static_cast<float>(canvas_y()),
            static_cast<float>(canvas_w()), static_cast<float>(canvas_h())
        };

        g_ui.draw_rect(canvas_rect, 0xFFFFFFFFU);
        g_ui.image(g_canvas, canvas_rect);

        // Grid overlay
        if (g_show_grid) {
            constexpr int step = 16;
            for (int gx = canvas_x(); gx < canvas_x() + canvas_w(); gx += step) {
                g_ui.draw_rect({static_cast<float>(gx), static_cast<float>(canvas_y()),
                                1.0f, static_cast<float>(canvas_h())}, 0x22000000U);
            }
            for (int gy = canvas_y(); gy < canvas_y() + canvas_h(); gy += step) {
                g_ui.draw_rect({static_cast<float>(canvas_x()), static_cast<float>(gy),
                                static_cast<float>(canvas_w()), 1.0f}, 0x22000000U);
            }
        }

        // Crosshair cursor
        if (in_canvas()) {
            g_ui.draw_rect({static_cast<float>(g_mx - 6), static_cast<float>(g_my), 12.0f, 1.0f}, 0xBB000000U);
            g_ui.draw_rect({static_cast<float>(g_mx), static_cast<float>(g_my - 6), 1.0f, 12.0f}, 0xBB000000U);
        }
    }

    // ── Status Bar ────────────────────────────────────────────────────────────
    g_ui.window("##status", {0.0f, static_cast<float>(g_win_h - kStatusH),
                            static_cast<float>(g_win_w), static_cast<float>(kStatusH)}, [&] {
        if (in_canvas()) {
            g_status = kToolNames[g_tool_idx] + "  |  " +
                       std::to_string(cx()) + ", " + std::to_string(cy()) +
                       "  |  Canvas: " + std::to_string(canvas_w()) + "x" + std::to_string(canvas_h()) +
                       "  |  Window: " + std::to_string(g_win_w) + "x" + std::to_string(g_win_h) +
                       "  |  Scale: " + std::to_string(g_ui_scale) + "x" +
                       "  |  Undo: " + std::to_string(g_undo.size());
        }
        g_ui.label(g_status);
    });

    g_ui.end();

    // DIB copy
    const std::uint32_t* src = g_ui.pixels();
    const std::size_t total_px = static_cast<std::size_t>(g_win_w) * static_cast<std::size_t>(g_win_h);
    if (g_dib.size() != total_px) g_dib.resize(total_px);
    for (std::size_t i = 0; i < total_px; ++i) {
        std::uint32_t p = src[i];
        g_dib[i] = ((p & 0xFFU) << 16U) | (p & 0xFF00U) | ((p >> 16U) & 0xFFU);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Win32 Presentation
// ─────────────────────────────────────────────────────────────────────────────
void present(HDC dc) {
    render_frame();
    BITMAPINFO bi{};
    bi.bmiHeader.biSize        = sizeof(bi.bmiHeader);
    bi.bmiHeader.biWidth       = g_win_w;
    bi.bmiHeader.biHeight      = -g_win_h;
    bi.bmiHeader.biPlanes      = 1;
    bi.bmiHeader.biBitCount    = 32;
    bi.bmiHeader.biCompression = BI_RGB;
    StretchDIBits(dc,
                  0, 0, g_win_w, g_win_h,
                  0, 0, g_win_w, g_win_h,
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
            present(dc);
            EndPaint(hwnd, &ps);
            return 0;
        }
        case WM_SIZE: {
            int new_w = LOWORD(lp);
            int new_h = HIWORD(lp);
            if (new_w > 0 && new_h > 0) {
                on_window_resize(new_w, new_h);
                InvalidateRect(hwnd, nullptr, FALSE);
            }
            return 0;
        }
        case WM_ERASEBKGND: return 1;
        case WM_TIMER:
            if (wp == kTimer) InvalidateRect(hwnd, nullptr, FALSE);
            return 0;

        case WM_MOUSEMOVE:
            update_mouse(lp);
            if (g_ldown && in_canvas()) canvas_move(cx(), cy());
            InvalidateRect(hwnd, nullptr, FALSE);
            return 0;

        case WM_LBUTTONDOWN:
            update_mouse(lp);
            g_ldown = true;
            SetCapture(hwnd);
            if (in_canvas()) canvas_down(cx(), cy());
            InvalidateRect(hwnd, nullptr, FALSE);
            return 0;

        case WM_LBUTTONUP:
            update_mouse(lp);
            if (g_ldown && in_canvas()) canvas_up(cx(), cy());
            g_ldown = false;
            ReleaseCapture();
            InvalidateRect(hwnd, nullptr, FALSE);
            return 0;

        case WM_CAPTURECHANGED:
            if (g_drawing) { g_drawing = false; g_preview_live = false; }
            g_ldown = false;
            return 0;

        case WM_MOUSEWHEEL: {
            POINT pt{
                static_cast<LONG>(static_cast<short>(LOWORD(lp))),
                static_cast<LONG>(static_cast<short>(HIWORD(lp))),
            };
            ScreenToClient(hwnd, &pt);
            g_mx = static_cast<int>(pt.x);
            g_my = static_cast<int>(pt.y);
            g_ui.feed_wheel(GET_WHEEL_DELTA_WPARAM(wp));
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
                case VK_TAB:   g_ui.feed_key(glm::Key::Tab, (GetKeyState(VK_SHIFT)&0x8000)!=0); break;
                case VK_BACK:  g_ui.feed_key(glm::Key::Backspace); break;
                case VK_DELETE:g_ui.feed_key(glm::Key::Delete);    break;
                case VK_LEFT:  g_ui.feed_key(glm::Key::Left);      break;
                case VK_RIGHT: g_ui.feed_key(glm::Key::Right);     break;
                case VK_HOME:  g_ui.feed_key(glm::Key::Home);      break;
                case VK_END:   g_ui.feed_key(glm::Key::End);       break;
                case VK_RETURN:g_ui.feed_key(glm::Key::Enter);     break;
                case VK_SPACE: g_ui.feed_key(glm::Key::Space);     break;
                case VK_UP:    g_ui.feed_key(glm::Key::Up);        break;
                case VK_DOWN:  g_ui.feed_key(glm::Key::Down);      break;
                case VK_ESCAPE:g_ui.feed_key(glm::Key::Escape);    break;
                case 'Z': if (GetKeyState(VK_CONTROL)&0x8000) do_undo(); break;
                case 'Y': if (GetKeyState(VK_CONTROL)&0x8000) do_redo(); break;
                case VK_OEM_4: // [
                    g_size_f = std::max(1.0f, g_size_f - 1.0f);
                    g_size = static_cast<int>(g_size_f);
                    break;
                case VK_OEM_6: // ]
                    g_size_f = std::min(40.0f, g_size_f + 1.0f);
                    g_size = static_cast<int>(g_size_f);
                    break;
                case 'P': g_tool = Tool::Pencil;      g_tool_idx = 0; break;
                case 'B': g_tool = Tool::Brush;       g_tool_idx = 1; break;
                case 'E': g_tool = Tool::Eraser;      g_tool_idx = 3; break;
                case 'L': g_tool = Tool::Line;        g_tool_idx = 4; break;
                case 'F': g_tool = Tool::Fill;        g_tool_idx = 11; break;
                case 'I': g_tool = Tool::ColorPicker; g_tool_idx = 12; break;
                case 'G': g_show_grid = !g_show_grid; break;
                default: return DefWindowProcW(hwnd, msg, wp, lp);
            }
            InvalidateRect(hwnd, nullptr, FALSE);
            return 0;

        case WM_DESTROY:
            KillTimer(hwnd, kTimer);
            PostQuitMessage(0);
            return 0;
        default:
            return DefWindowProcW(hwnd, msg, wp, lp);
    }
}

}  // namespace

// ─────────────────────────────────────────────────────────────────────────────
// Entry Point
// ─────────────────────────────────────────────────────────────────────────────
int WINAPI WinMain(HINSTANCE inst, HINSTANCE, LPSTR, int nShow) {
    on_window_resize(1200, 720);
    clear_canvas();
    blit(g_preview, g_canvas);

    const wchar_t* kClass = L"GlimmerPaintResizable";
    WNDCLASSW wc{};
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = inst;
    wc.hCursor       = LoadCursorW(nullptr, MAKEINTRESOURCEW(32512));
    wc.hbrBackground = nullptr;
    wc.lpszClassName = kClass;
    if (!RegisterClassW(&wc)) { MessageBoxW(nullptr, L"Register failed", L"Paint", MB_ICONERROR); return 1; }

    RECT bounds{0, 0, g_win_w, g_win_h};
    // Include WS_THICKFRAME and WS_MAXIMIZEBOX to allow user resizing & maximizing
    AdjustWindowRect(&bounds, WS_OVERLAPPEDWINDOW, FALSE);
    HWND hwnd = CreateWindowExW(0U, kClass, L"Glimmer Paint — Resizable & Scalable",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        bounds.right-bounds.left, bounds.bottom-bounds.top,
        nullptr, nullptr, inst, nullptr);
    if (!hwnd) { MessageBoxW(nullptr, L"Create failed", L"Paint", MB_ICONERROR); return 1; }

    ShowWindow(hwnd, nShow);
    UpdateWindow(hwnd);
    SetTimer(hwnd, kTimer, 16U, nullptr);  // ~60 fps

    MSG m{};
    int r = 0;
    while ((r = static_cast<int>(GetMessageW(&m, nullptr, 0U, 0U))) > 0) {
        TranslateMessage(&m);
        DispatchMessageW(&m);
    }
    return r < 0 ? 1 : static_cast<int>(m.wParam);
}
