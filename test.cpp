// ════════════════════════════════════════════════════════════════════════════
// Modern Windows 11 Fluent Paint Showcase
// Built with Glimmer Immediate-Mode GUI Engine
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

namespace {

// ─────────────────────────────────────────────────────────────────────────────
// Window & Layout Metrics (Matches Windows 11 Paint Reference)
// ─────────────────────────────────────────────────────────────────────────────
int g_win_w = 1280;
int g_win_h = 760;

constexpr float kTitleBarH  = 34.0f;
constexpr float kRibbonH    = 108.0f;
constexpr float kStatusBarH = 28.0f;
constexpr float kLeftRailW  = 44.0f;
constexpr UINT_PTR kTimer   = 1U;

// Default Canvas metrics
int g_doc_w = 1152;
int g_doc_h = 648;
float g_zoom_pct = 100.0f;
float g_scroll_x = 0.0f;
float g_scroll_y = 0.0f;

// Active Canvas Surface & Preview
glm::Surface g_canvas(g_doc_w, g_doc_h);
glm::Surface g_preview(g_doc_w, g_doc_h);

// Glimmer UI context
glm::Context g_ui(1280, 760, glm::Theme::make_fluent_light());
std::vector<std::uint32_t> g_dib;

// Win32 Mouse
int g_mx = 0, g_my = 0;
bool g_ldown = false;

// ─────────────────────────────────────────────────────────────────────────────
// Tools & State
// ─────────────────────────────────────────────────────────────────────────────
enum class ActiveTool {
    Selection = 0,
    Crop,
    Pencil,
    Eraser,
    Fill,
    ColorPicker,
    Text,
    Magnifier,
    Brush,
    Shape,
    COUNT
};

ActiveTool g_tool = ActiveTool::Pencil;
int g_shape_type = 0; // 0=Line, 1=Circle, 2=Rect, 3=RoundRect, 4=Triangle, 5=Diamond, 6=Arrow, 7=Star, 8=Bubble, 9=Heart
int g_brush_type = 0;
int g_brush_size = 3;

std::uint32_t g_color1 = 0xFF000000U; // Primary Black
std::uint32_t g_color2 = 0xFFFFFFFFU; // Secondary White
bool g_picking_color2 = false;

// Windows 11 Paint Modern 30-Color Palette (Top 15, Bottom 15)
const std::vector<std::uint32_t> kWin11Palette = {
    // Row 1 (core)
    0xFF000000U, 0xFF7F7F7FU, 0xFF880015U, 0xFFED1C24U, 0xFFFF7F27U,
    0xFFFFF200U, 0xFF22B14CU, 0xFF00A2E8U, 0xFF3F48CCU, 0xFFA349A9U,
    0xFF585858U, 0xFFB97A57U, 0xFFFFAEC9U, 0xFFB5E61DU, 0xFF7092BEU,
    // Row 2 (pastel)
    0xFFFFFFFFU, 0xFFC3C3C3U, 0xFFB5E61DU, 0xFF99D9EAU, 0xFFC8BFE7U,
    0xFFEFE4B0U, 0xFFE36C0AU, 0xFF00B050U, 0xFF002060U, 0xFF7030A0U,
    0xFFD9D9D9U, 0xFF8C564BU, 0xFFF781BFU, 0xFFCCEBC5U, 0xFFE0E0E0U
};

// Undo / Redo
constexpr std::size_t kMaxUndo = 30U;
std::vector<std::vector<std::uint32_t>> g_undo, g_redo;

// Drawing interaction
bool g_drawing = false;
bool g_preview_live = false;
int g_start_cx = 0, g_start_cy = 0;
int g_prev_cx = -1, g_prev_cy = -1;

// Left Rail Sliders (Brush Size & Opacity)
float g_rail_size_pct = 0.15f;    // 1 to 40 px
float g_rail_opacity_pct = 1.0f; // 0 to 1

// Modern Layers Panel & Overlays
bool g_layers_open = false;
std::string g_status_msg = "Ready";

// Selection Rect
bool g_has_selection = false;
glm::Rect g_sel_rect{0, 0, 0, 0};

// ─────────────────────────────────────────────────────────────────────────────
// Canvas Projection & Transform Helpers
// ─────────────────────────────────────────────────────────────────────────────
glm::Rect get_viewport_rect() {
    float vx = kLeftRailW;
    float vy = kTitleBarH + kRibbonH;
    float vw = std::max(10.0f, static_cast<float>(g_win_w) - vx);
    float vh = std::max(10.0f, static_cast<float>(g_win_h) - vy - kStatusBarH);
    return {vx, vy, vw, vh};
}

glm::Rect get_canvas_screen_rect() {
    glm::Rect vp = get_viewport_rect();
    float scale = g_zoom_pct / 100.0f;
    float disp_w = static_cast<float>(g_doc_w) * scale;
    float disp_h = static_cast<float>(g_doc_h) * scale;
    float cx = vp.x + (vp.w - disp_w) * 0.5f + g_scroll_x;
    float cy = vp.y + (vp.h - disp_h) * 0.5f + g_scroll_y;
    return {cx, cy, disp_w, disp_h};
}

bool screen_to_canvas(int sx, int sy, int& out_cx, int& out_cy) {
    glm::Rect cr = get_canvas_screen_rect();
    if (sx < cr.x || sy < cr.y || sx >= cr.x + cr.w || sy >= cr.y + cr.h) return false;
    float scale = g_zoom_pct / 100.0f;
    out_cx = static_cast<int>((static_cast<float>(sx) - cr.x) / scale);
    out_cy = static_cast<int>((static_cast<float>(sy) - cr.y) / scale);
    out_cx = std::clamp(out_cx, 0, g_doc_w - 1);
    out_cy = std::clamp(out_cy, 0, g_doc_h - 1);
    return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// Canvas Undo / Redo & Primitives
// ─────────────────────────────────────────────────────────────────────────────
void push_undo() {
    const std::uint32_t* px = g_canvas.pixels();
    std::size_t n = static_cast<std::size_t>(g_canvas.width()) * static_cast<std::size_t>(g_canvas.height());
    g_undo.push_back(std::vector<std::uint32_t>(px, px + n));
    if (g_undo.size() > kMaxUndo) g_undo.erase(g_undo.begin());
    g_redo.clear();
}

void do_undo() {
    if (g_undo.empty()) return;
    const std::uint32_t* px = g_canvas.pixels();
    std::size_t n = static_cast<std::size_t>(g_canvas.width()) * static_cast<std::size_t>(g_canvas.height());
    g_redo.push_back(std::vector<std::uint32_t>(px, px + n));
    std::memcpy(g_canvas.pixels(), g_undo.back().data(), n * sizeof(std::uint32_t));
    g_undo.pop_back();
}

void do_redo() {
    if (g_redo.empty()) return;
    const std::uint32_t* px = g_canvas.pixels();
    std::size_t n = static_cast<std::size_t>(g_canvas.width()) * static_cast<std::size_t>(g_canvas.height());
    g_undo.push_back(std::vector<std::uint32_t>(px, px + n));
    std::memcpy(g_canvas.pixels(), g_redo.back().data(), n * sizeof(std::uint32_t));
    g_redo.pop_back();
}

void copy_surface_full(glm::Surface& dst, const glm::Surface& src) {
    std::memcpy(dst.pixels(), src.pixels(), static_cast<std::size_t>(src.width() * src.height()) * sizeof(std::uint32_t));
}

void clear_canvas_white() {
    g_canvas.clear(0xFFFFFFFFU);
}

void flood_fill(int x, int y, std::uint32_t fill_c) {
    int w = g_canvas.width(), h = g_canvas.height();
    if (x < 0 || y < 0 || x >= w || y >= h) return;
    std::uint32_t* pix = g_canvas.pixels();
    std::uint32_t target = pix[y * w + x];
    if (target == fill_c) return;
    std::vector<int> stk;
    stk.reserve(16384);
    stk.push_back(y * w + x);
    while (!stk.empty()) {
        int idx = stk.back(); stk.pop_back();
        if (idx < 0 || idx >= w * h) continue;
        if (pix[idx] != target) continue;
        pix[idx] = fill_c;
        int px = idx % w, py = idx / w;
        if (px > 0)     stk.push_back(idx - 1);
        if (px < w - 1) stk.push_back(idx + 1);
        if (py > 0)     stk.push_back(idx - w);
        if (py < h - 1) stk.push_back(idx + w);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Shape Rasterizer
// ─────────────────────────────────────────────────────────────────────────────
void commit_shape(glm::Surface& surf, int sx, int sy, int ex, int ey, std::uint32_t color, int stroke_sz) {
    float x0 = static_cast<float>(sx), y0 = static_cast<float>(sy);
    float x1 = static_cast<float>(ex), y1 = static_cast<float>(ey);
    float rx = std::min(x0, x1), ry = std::min(y0, y1);
    float rw = std::abs(x1 - x0), rh = std::abs(y1 - y0);
    glm::Rect r{rx, ry, rw, rh};

    switch (g_shape_type) {
        case 0: // Line
            surf.draw_thick_line(x0, y0, x1, y1, static_cast<float>(stroke_sz), color);
            break;
        case 1: // Circle / Ellipse
            surf.draw_ellipse_outline(r, color, static_cast<float>(stroke_sz));
            break;
        case 2: // Rectangle
            surf.outline_rect(r, color, static_cast<float>(stroke_sz));
            break;
        case 3: // Rounded Rectangle
            surf.draw_rounded_rect_outline(r, 12.0f, color, static_cast<float>(stroke_sz));
            break;
        case 4: // Triangle
            surf.draw_triangle_outline(rx + rw * 0.5f, ry, rx + rw, ry + rh, rx, ry + rh, color, static_cast<float>(stroke_sz));
            break;
        case 5: // Diamond
            surf.draw_diamond_outline(r, color, static_cast<float>(stroke_sz));
            break;
        case 6: // Arrow Right
            surf.draw_arrow_outline(r, 0, color, static_cast<float>(stroke_sz));
            break;
        case 7: // 5-Point Star
            surf.draw_star_outline(rx + rw * 0.5f, ry + rh * 0.5f, rw * 0.5f, rw * 0.25f, color, static_cast<float>(stroke_sz));
            break;
        case 8: // Speech Bubble
            surf.draw_speech_bubble(r, color, true, static_cast<float>(stroke_sz));
            break;
        case 9: // Heart
            surf.draw_filled_heart(r, color);
            break;
        default: break;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Mouse Input Handler for Canvas
// ─────────────────────────────────────────────────────────────────────────────
void on_canvas_down(int cx, int cy) {
    g_start_cx = cx; g_start_cy = cy;
    g_prev_cx = cx; g_prev_cy = cy;
    g_drawing = true;

    int eff_size = std::max(1, static_cast<int>(g_rail_size_pct * 30.0f));

    switch (g_tool) {
        case ActiveTool::Pencil:
            push_undo();
            g_canvas.draw_line(cx, cy, cx, cy, g_color1);
            break;
        case ActiveTool::Brush:
            push_undo();
            g_canvas.draw_filled_circle(cx, cy, eff_size, g_color1);
            break;
        case ActiveTool::Eraser:
            push_undo();
            g_canvas.draw_filled_circle(cx, cy, eff_size * 2, g_color2);
            break;
        case ActiveTool::Fill:
            push_undo();
            flood_fill(cx, cy, g_color1);
            g_drawing = false;
            break;
        case ActiveTool::ColorPicker: {
            if (cx >= 0 && cy >= 0 && cx < g_doc_w && cy < g_doc_h) {
                if (g_picking_color2) g_color2 = g_canvas.get_pixel(cx, cy);
                else g_color1 = g_canvas.get_pixel(cx, cy);
            }
            g_drawing = false;
            break;
        }
        case ActiveTool::Selection:
            g_has_selection = true;
            g_sel_rect = {static_cast<float>(cx), static_cast<float>(cy), 0.0f, 0.0f};
            break;
        case ActiveTool::Shape:
            push_undo();
            copy_surface_full(g_preview, g_canvas);
            g_preview_live = true;
            break;
        default: break;
    }
}

void on_canvas_move(int cx, int cy) {
    if (!g_drawing) return;
    int eff_size = std::max(1, static_cast<int>(g_rail_size_pct * 30.0f));

    switch (g_tool) {
        case ActiveTool::Pencil:
            g_canvas.draw_line(g_prev_cx, g_prev_cy, cx, cy, g_color1);
            break;
        case ActiveTool::Brush: {
            int dx = cx - g_prev_cx, dy = cy - g_prev_cy;
            int steps = std::max(1, std::max(std::abs(dx), std::abs(dy)));
            for (int i = 0; i <= steps; ++i) {
                g_canvas.draw_filled_circle(g_prev_cx + dx * i / steps, g_prev_cy + dy * i / steps, eff_size, g_color1);
            }
            break;
        }
        case ActiveTool::Eraser: {
            int dx = cx - g_prev_cx, dy = cy - g_prev_cy;
            int steps = std::max(1, std::max(std::abs(dx), std::abs(dy)));
            for (int i = 0; i <= steps; ++i) {
                g_canvas.draw_filled_circle(g_prev_cx + dx * i / steps, g_prev_cy + dy * i / steps, eff_size * 2, g_color2);
            }
            break;
        }
        case ActiveTool::Selection:
            g_sel_rect.x = static_cast<float>(std::min(g_start_cx, cx));
            g_sel_rect.y = static_cast<float>(std::min(g_start_cy, cy));
            g_sel_rect.w = static_cast<float>(std::abs(cx - g_start_cx));
            g_sel_rect.h = static_cast<float>(std::abs(cy - g_start_cy));
            break;
        case ActiveTool::Shape:
            if (g_preview_live) {
                copy_surface_full(g_canvas, g_preview);
                commit_shape(g_canvas, g_start_cx, g_start_cy, cx, cy, g_color1, eff_size);
            }
            break;
        default: break;
    }

    g_prev_cx = cx; g_prev_cy = cy;
}

void on_canvas_up(int cx, int cy) {
    if (!g_drawing) return;
    int eff_size = std::max(1, static_cast<int>(g_rail_size_pct * 30.0f));
    if (g_tool == ActiveTool::Shape && g_preview_live) {
        copy_surface_full(g_canvas, g_preview);
        commit_shape(g_canvas, g_start_cx, g_start_cy, cx, cy, g_color1, eff_size);
        g_preview_live = false;
    }
    g_drawing = false;
}

// ─────────────────────────────────────────────────────────────────────────────
// Render Windows 11 Fluent Ribbon Header
// ─────────────────────────────────────────────────────────────────────────────
void render_fluent_ribbon(glm::Context& ui) {
    // 1. Title bar (Window Title & Quick Access)
    {
        ui.menu_bar_begin(kTitleBarH);
        ui.surface().draw_icon_dropper({10.0f, 8.0f, 18.0f, 18.0f}, 0xFF0067C0U);
        ui.surface().draw_text(34, 9, "Untitled - Paint", 0xFF1A1A1AU, 1);

        // Menu items
        float menu_x = 180.0f;
        auto draw_menu_tab = [&](const std::string& name) {
            ui.surface().draw_text(static_cast<int>(menu_x + 6), 9, name, 0xFF1A1A1AU, 1);
            menu_x += 48.0f;
        };
        draw_menu_tab("File");
        draw_menu_tab("Edit");
        draw_menu_tab("View");

        // Quick Undo / Redo in title bar
        if (ui.button_primary("Undo").clicked) do_undo();
        if (ui.button_secondary("Redo").clicked) do_redo();
        ui.menu_bar_end();
    }

    // 2. Command Ribbon Container
    glm::Rect rib_rect{0.0f, kTitleBarH, static_cast<float>(g_win_w), kRibbonH};
    ui.draw_rect(rib_rect, 0xFFF7F7F7U);
    ui.surface().draw_line(0, static_cast<int>(kTitleBarH + kRibbonH), g_win_w, static_cast<int>(kTitleBarH + kRibbonH), 0xFFE0E0E0U);

    float card_x = 8.0f;
    float card_y = kTitleBarH + 6.0f;
    float card_h = 92.0f;

    // CARD 1: SELECTION
    {
        float card_w = 64.0f;
        glm::Rect cr{card_x, card_y, card_w, card_h};
        ui.draw_panel(cr, 5.0f);
        bool sel = (g_tool == ActiveTool::Selection);
        if (ui.tool_button("##btn_sel", "Select", sel, 36.0f).clicked) g_tool = ActiveTool::Selection;
        ui.surface().draw_text(static_cast<int>(card_x + 8), static_cast<int>(card_y + card_h - 14.0f), "Selection", 0xFF8A8A8AU, 1);
        card_x += card_w + 6.0f;
    }

    // CARD 2: IMAGE (Crop, Flip, Rotate)
    {
        float card_w = 68.0f;
        glm::Rect cr{card_x, card_y, card_w, card_h};
        ui.draw_panel(cr, 5.0f);
        if (ui.tool_button("##crop", "Crop", false, 24.0f).clicked) {
            if (g_has_selection && g_sel_rect.w > 4 && g_sel_rect.h > 4) {
                push_undo();
                g_canvas = g_canvas.crop(g_sel_rect);
                g_doc_w = g_canvas.width(); g_doc_h = g_canvas.height();
                g_has_selection = false;
            }
        }
        if (ui.tool_button("##flip", "Flip", false, 24.0f).clicked) {
            push_undo();
            g_canvas.flip_horizontal();
        }
        ui.surface().draw_text(static_cast<int>(card_x + 16), static_cast<int>(card_y + card_h - 14.0f), "Image", 0xFF8A8A8AU, 1);
        card_x += card_w + 6.0f;
    }

    // CARD 3: TOOLS (Pencil, Eraser, Fill, Dropper, Text, Magnifier)
    {
        float card_w = 94.0f;
        glm::Rect cr{card_x, card_y, card_w, card_h};
        ui.draw_panel(cr, 5.0f);

        auto tool_toggle = [&](ActiveTool t, const std::string& name, float tx, float ty) {
            glm::Rect r{tx, ty, 26.0f, 26.0f};
            std::uint64_t id = static_cast<std::uint64_t>(t) + 1000U;
            bool clicked = false;
            if (g_mx >= r.x && g_my >= r.y && g_mx < r.x + r.w && g_my < r.y + r.h && g_ldown) {
                clicked = true;
            }
            if (clicked) g_tool = t;
            bool active = (g_tool == t);
            if (active) ui.surface().draw_rounded_rect(r, 4.0f, 0xFFE0EEFAU);
            if (name == "pencil") ui.surface().draw_icon_pencil(r, active ? 0xFF0067C0U : 0xFF1A1A1AU);
            else if (name == "fill") ui.surface().draw_icon_bucket(r, active ? 0xFF0067C0U : 0xFF1A1A1AU);
            else if (name == "text") ui.surface().draw_text(static_cast<int>(r.x + 8), static_cast<int>(r.y + 5), "A", active ? 0xFF0067C0U : 0xFF1A1A1AU, 1);
            else if (name == "eraser") ui.surface().draw_rounded_rect_outline({r.x + 5, r.y + 6, 16, 14}, 2.0f, active ? 0xFF0067C0U : 0xFF1A1A1AU, 1.0f);
            else if (name == "dropper") ui.surface().draw_icon_dropper(r, active ? 0xFF0067C0U : 0xFF1A1A1AU);
            else if (name == "zoom") ui.surface().draw_icon_magnifier(r, active ? 0xFF0067C0U : 0xFF1A1A1AU);
        };

        tool_toggle(ActiveTool::Pencil,      "pencil",  card_x + 6, card_y + 8);
        tool_toggle(ActiveTool::Fill,        "fill",    card_x + 34, card_y + 8);
        tool_toggle(ActiveTool::Text,        "text",    card_x + 62, card_y + 8);
        tool_toggle(ActiveTool::Eraser,      "eraser",  card_x + 6, card_y + 38);
        tool_toggle(ActiveTool::ColorPicker, "dropper", card_x + 34, card_y + 38);
        tool_toggle(ActiveTool::Magnifier,   "zoom",    card_x + 62, card_y + 38);

        ui.surface().draw_text(static_cast<int>(card_x + 30), static_cast<int>(card_y + card_h - 14.0f), "Tools", 0xFF8A8A8AU, 1);
        card_x += card_w + 6.0f;
    }

    // CARD 4: BRUSHES (Drop-down large button)
    {
        float card_w = 64.0f;
        glm::Rect cr{card_x, card_y, card_w, card_h};
        ui.draw_panel(cr, 5.0f);
        bool is_brush = (g_tool == ActiveTool::Brush);
        if (ui.tool_button("##brush_main", "Brush", is_brush, 36.0f).clicked) g_tool = ActiveTool::Brush;
        ui.surface().draw_text(static_cast<int>(card_x + 12), static_cast<int>(card_y + card_h - 14.0f), "Brushes", 0xFF8A8A8AU, 1);
        card_x += card_w + 6.0f;
    }

    // CARD 5: SHAPES GRID (Line, Circle, Rect, Star, Bubble, Heart, etc.)
    {
        float card_w = 160.0f;
        glm::Rect cr{card_x, card_y, card_w, card_h};
        ui.draw_panel(cr, 5.0f);

        int chosen_shape = ui.shape_picker_grid("##rib_shapes", (g_tool == ActiveTool::Shape) ? g_shape_type : -1, 10, 5);
        if (chosen_shape >= 0) {
            g_shape_type = chosen_shape;
            g_tool = ActiveTool::Shape;
        }

        ui.surface().draw_text(static_cast<int>(card_x + 60), static_cast<int>(card_y + card_h - 14.0f), "Shapes", 0xFF8A8A8AU, 1);
        card_x += card_w + 6.0f;
    }

    // CARD 6: SIZE / STROKE PRESETS
    {
        float card_w = 54.0f;
        glm::Rect cr{card_x, card_y, card_w, card_h};
        ui.draw_panel(cr, 5.0f);
        int new_sz = ui.brush_size_presets("##rib_sizes", g_brush_size);
        if (new_sz != g_brush_size) {
            g_brush_size = new_sz;
            g_rail_size_pct = static_cast<float>(new_sz) / 30.0f;
        }
        ui.surface().draw_text(static_cast<int>(card_x + 14), static_cast<int>(card_y + card_h - 14.0f), "Size", 0xFF8A8A8AU, 1);
        card_x += card_w + 6.0f;
    }

    // CARD 7: COLOURS (Color 1, Color 2, 30 Modern Swatches & Color Wheel)
    {
        float card_w = 340.0f;
        glm::Rect cr{card_x, card_y, card_w, card_h};
        ui.draw_panel(cr, 5.0f);

        // Color 1 (Primary)
        glm::Rect c1_r{card_x + 10.0f, card_y + 12.0f, 32.0f, 32.0f};
        if (g_mx >= c1_r.x && g_my >= c1_r.y && g_mx < c1_r.x + c1_r.w && g_my < c1_r.y + c1_r.h && g_ldown) {
            g_picking_color2 = false;
        }
        ui.surface().draw_filled_circle(static_cast<int>(c1_r.x + 16), static_cast<int>(c1_r.y + 16), 15, g_color1);
        ui.surface().draw_circle_outline(static_cast<int>(c1_r.x + 16), static_cast<int>(c1_r.y + 16), 15, !g_picking_color2 ? 0xFF0067C0U : 0xFFC0C0C0U, 2);
        ui.surface().draw_text(static_cast<int>(c1_r.x - 2), static_cast<int>(c1_r.y + 36), "Color 1", 0xFF1A1A1AU, 1);

        // Color 2 (Secondary)
        glm::Rect c2_r{card_x + 48.0f, card_y + 12.0f, 26.0f, 26.0f};
        if (g_mx >= c2_r.x && g_my >= c2_r.y && g_mx < c2_r.x + c2_r.w && g_my < c2_r.y + c2_r.h && g_ldown) {
            g_picking_color2 = true;
        }
        ui.surface().draw_filled_circle(static_cast<int>(c2_r.x + 13), static_cast<int>(c2_r.y + 13), 12, g_color2);
        ui.surface().draw_circle_outline(static_cast<int>(c2_r.x + 13), static_cast<int>(c2_r.y + 13), 12, g_picking_color2 ? 0xFF0067C0U : 0xFFC0C0C0U, 2);
        ui.surface().draw_text(static_cast<int>(c2_r.x - 2), static_cast<int>(c2_r.y + 36), "Color 2", 0xFF1A1A1AU, 1);

        // 30-Color Palette Matrix (15 columns x 2 rows)
        float pal_start_x = card_x + 88.0f;
        for (std::size_t i = 0; i < kWin11Palette.size(); ++i) {
            float px = pal_start_x + static_cast<float>(i % 15) * 15.0f;
            float py = card_y + 12.0f + static_cast<float>(i / 15) * 16.0f;
            glm::Rect pr{px, py, 13.0f, 13.0f};
            if (g_mx >= pr.x && g_my >= pr.y && g_mx < pr.x + pr.w && g_my < pr.y + pr.h && g_ldown) {
                if (g_picking_color2) g_color2 = kWin11Palette[i];
                else g_color1 = kWin11Palette[i];
            }
            ui.surface().draw_filled_circle(static_cast<int>(px + 6), static_cast<int>(py + 6), 6, kWin11Palette[i]);
            ui.surface().draw_circle_outline(static_cast<int>(px + 6), static_cast<int>(py + 6), 6, 0xFFE0E0E0U, 1);
        }

        // Color wheel trigger
        glm::Rect wheel_r{pal_start_x + 15.0f * 15.0f + 6.0f, card_y + 16.0f, 22.0f, 22.0f};
        ui.surface().draw_color_wheel(static_cast<int>(wheel_r.x + 11), static_cast<int>(wheel_r.y + 11), 10);
        std::uint32_t chosen = g_color1;
        if (ui.color_picker_radial(static_cast<int>(wheel_r.x + 11), static_cast<int>(wheel_r.y + 11), 30, chosen)) {
            if (g_picking_color2) g_color2 = chosen;
            else g_color1 = chosen;
        }

        ui.surface().draw_text(static_cast<int>(card_x + 150), static_cast<int>(card_y + card_h - 14.0f), "Colours", 0xFF8A8A8AU, 1);
        card_x += card_w + 6.0f;
    }

    // CARD 8: LAYERS & COPILOT
    {
        float card_w = 64.0f;
        glm::Rect cr{card_x, card_y, card_w, card_h};
        ui.draw_panel(cr, 5.0f);
        if (ui.tool_button("##layers_btn", "Layers", g_layers_open, 36.0f).clicked) g_layers_open = !g_layers_open;
        ui.surface().draw_text(static_cast<int>(card_x + 14), static_cast<int>(card_y + card_h - 14.0f), "Layers", 0xFF8A8A8AU, 1);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Render Left Rail Sliders (Exact Visual Match to Picture)
// ─────────────────────────────────────────────────────────────────────────────
void render_left_rail(glm::Context& ui) {
    glm::Rect rail_rect{0.0f, kTitleBarH + kRibbonH, kLeftRailW, static_cast<float>(g_win_h) - kTitleBarH - kRibbonH - kStatusBarH};
    ui.draw_rect(rail_rect, 0xFFF7F7F7U);

    // Top Vertical Slider: Brush Size / Thickness
    {
        float track_x = 20.0f;
        float track_y = rail_rect.y + 30.0f;
        float track_h = 160.0f;
        glm::Rect track_rect{track_x - 1.0f, track_y, 2.0f, track_h};
        ui.surface().fill_rect(track_rect, 0xFFD0D0D0U);

        float thumb_y = track_y + (1.0f - g_rail_size_pct) * track_h;
        if (g_mx >= track_x - 12.0f && g_mx <= track_x + 12.0f && g_my >= track_y && g_my <= track_y + track_h && g_ldown) {
            float my = std::clamp(static_cast<float>(g_my) - track_y, 0.0f, track_h);
            g_rail_size_pct = 1.0f - (my / track_h);
        }
        ui.surface().draw_filled_circle(static_cast<int>(track_x), static_cast<int>(thumb_y), 5, 0xFF0067C0U);
        ui.surface().draw_circle_outline(static_cast<int>(track_x), static_cast<int>(thumb_y), 5, 0xFFFFFFFFU, 1);
    }

    // Bottom Vertical Slider: Opacity
    {
        float track_x = 20.0f;
        float track_y = rail_rect.y + 240.0f;
        float track_h = 180.0f;
        glm::Rect track_rect{track_x - 1.0f, track_y, 2.0f, track_h};
        ui.surface().fill_rect(track_rect, 0xFFD0D0D0U);

        float thumb_y = track_y + (1.0f - g_rail_opacity_pct) * track_h;
        if (g_mx >= track_x - 12.0f && g_mx <= track_x + 12.0f && g_my >= track_y && g_my <= track_y + track_h && g_ldown) {
            float my = std::clamp(static_cast<float>(g_my) - track_y, 0.0f, track_h);
            g_rail_opacity_pct = 1.0f - (my / track_h);
        }
        ui.surface().draw_filled_circle(static_cast<int>(track_x), static_cast<int>(thumb_y), 5, 0xFF0067C0U);
        ui.surface().draw_circle_outline(static_cast<int>(track_x), static_cast<int>(thumb_y), 5, 0xFFFFFFFFU, 1);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Render Bottom Status Bar
// ─────────────────────────────────────────────────────────────────────────────
void render_status_bar(glm::Context& ui) {
    float bar_y = static_cast<float>(g_win_h) - kStatusBarH;
    glm::Rect bar_rect{0.0f, bar_y, static_cast<float>(g_win_w), kStatusBarH};
    ui.draw_rect(bar_rect, 0xFFF7F7F7U);
    ui.surface().draw_line(0, static_cast<int>(bar_y), g_win_w, static_cast<int>(bar_y), 0xFFE0E0E0U);

    // Left status: Canvas Dimensions
    std::string dim_str = std::to_string(g_doc_w) + " x " + std::to_string(g_doc_h) + "px";
    ui.surface().outline_rect({16.0f, bar_y + 6.0f, 12.0f, 12.0f}, 0xFF7A7A7AU, 1.0f);
    ui.surface().draw_text(34, static_cast<int>(bar_y + 6.0f), dim_str, 0xFF1A1A1AU, 1);

    // Right status: Zoom Slider & Percentage (100%)
    float right_x = static_cast<float>(g_win_w) - 240.0f;
    ui.surface().draw_text(static_cast<int>(right_x), static_cast<int>(bar_y + 6.0f), std::to_string(static_cast<int>(g_zoom_pct)) + "%", 0xFF1A1A1AU, 1);

    // Mini Zoom Track
    float z_track_x = right_x + 50.0f;
    float z_track_w = 120.0f;
    ui.surface().draw_line(static_cast<int>(z_track_x), static_cast<int>(bar_y + 14.0f),
                           static_cast<int>(z_track_x + z_track_w), static_cast<int>(bar_y + 14.0f), 0xFFD0D0D0U);

    float z_thumb_x = z_track_x + (std::clamp((g_zoom_pct - 25.0f) / 375.0f, 0.0f, 1.0f)) * z_track_w;
    if (g_mx >= z_track_x && g_mx <= z_track_x + z_track_w && g_my >= bar_y && g_my <= bar_y + kStatusBarH && g_ldown) {
        float rel = std::clamp((static_cast<float>(g_mx) - z_track_x) / z_track_w, 0.0f, 1.0f);
        g_zoom_pct = 25.0f + rel * 375.0f;
    }
    ui.surface().draw_filled_circle(static_cast<int>(z_thumb_x), static_cast<int>(bar_y + 14.0f), 4, 0xFF0067C0U);
}

// ─────────────────────────────────────────────────────────────────────────────
// Main Frame Presentation
// ─────────────────────────────────────────────────────────────────────────────
void render_frame() {
    g_ui.begin(g_mx, g_my, g_ldown);

    // 1. Draw Viewport Background (Slate Grey #F3F3F3)
    glm::Rect vp = get_viewport_rect();
    g_ui.draw_rect(vp, 0xFFF3F3F3U);

    // 2. Draw Paint Document Canvas with Drop Shadow
    glm::Rect cr = get_canvas_screen_rect();
    g_ui.surface().draw_drop_shadow(cr, 4, 0x1A000000U);
    g_ui.draw_rect(cr, 0xFFFFFFFFU);
    g_ui.surface().draw_image(g_canvas.pixels(), g_canvas.width(), g_canvas.height(), cr);

    // 3. Document Resize Grippers (Right, Bottom, Bottom-Right)
    auto draw_gripper = [&](float gx, float gy) {
        g_ui.surface().draw_rounded_rect({gx - 3.0f, gy - 3.0f, 6.0f, 6.0f}, 1.0f, 0xFFFFFFFFU);
        g_ui.surface().outline_rect({gx - 3.0f, gy - 3.0f, 6.0f, 6.0f}, 0xFF7A7A7AU, 1.0f);
    };
    draw_gripper(cr.x + cr.w, cr.y + cr.h * 0.5f);
    draw_gripper(cr.x + cr.w * 0.5f, cr.y + cr.h);
    draw_gripper(cr.x + cr.w, cr.y + cr.h);

    // 4. Selection Marquee
    if (g_has_selection && g_sel_rect.w > 0 && g_sel_rect.h > 0) {
        float scale = g_zoom_pct / 100.0f;
        glm::Rect sel_scr{cr.x + g_sel_rect.x * scale, cr.y + g_sel_rect.y * scale,
                          g_sel_rect.w * scale, g_sel_rect.h * scale};
        g_ui.surface().draw_selection_marquee(sel_scr, 0xFF000000U, 0xFFFFFFFFU, 4);
    }

    // 5. Left Vertical Slider Rail
    render_left_rail(g_ui);

    // 6. Top Ribbon & Title Bar
    render_fluent_ribbon(g_ui);

    // 7. Right Layers Flyout Panel (if open)
    if (g_layers_open) {
        float lay_w = 180.0f;
        glm::Rect lay_rect{static_cast<float>(g_win_w) - lay_w, kTitleBarH + kRibbonH, lay_w, static_cast<float>(g_win_h) - kTitleBarH - kRibbonH - kStatusBarH};
        g_ui.draw_panel(lay_rect, 4.0f);
        g_ui.surface().draw_text(static_cast<int>(lay_rect.x + 12), static_cast<int>(lay_rect.y + 12), "Layers", 0xFF1A1A1AU, 1);
        bool v1 = true, v2 = true;
        g_ui.layer_row("##lay_1", "Layer 1 (Canvas)", v1, true);
        g_ui.layer_row("##lay_2", "Background", v2, false);
    }

    // 8. Bottom Status Bar
    render_status_bar(g_ui);

    g_ui.end();

    // Copy to DIB
    const std::uint32_t* src = g_ui.pixels();
    const std::size_t total_px = static_cast<std::size_t>(g_win_w) * static_cast<std::size_t>(g_win_h);
    if (g_dib.size() != total_px) g_dib.resize(total_px);
    for (std::size_t i = 0; i < total_px; ++i) {
        std::uint32_t p = src[i];
        g_dib[i] = ((p & 0xFFU) << 16U) | (p & 0xFF00U) | ((p >> 16U) & 0xFFU);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Win32 Presentation & Window Procedure
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

void on_resize(int nw, int nh) {
    if (nw <= 0 || nh <= 0) return;
    g_win_w = nw; g_win_h = nh;
    g_ui.resize(g_win_w, g_win_h);
    g_dib.resize(static_cast<std::size_t>(g_win_w * g_win_h));
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
            on_resize(LOWORD(lp), HIWORD(lp));
            InvalidateRect(hwnd, nullptr, FALSE);
            return 0;
        }
        case WM_ERASEBKGND: return 1;
        case WM_TIMER:
            if (wp == kTimer) InvalidateRect(hwnd, nullptr, FALSE);
            return 0;

        case WM_MOUSEMOVE: {
            g_mx = static_cast<int>(static_cast<short>(LOWORD(lp)));
            g_my = static_cast<int>(static_cast<short>(HIWORD(lp)));
            int cx, cy;
            if (g_ldown && screen_to_canvas(g_mx, g_my, cx, cy)) {
                on_canvas_move(cx, cy);
            }
            InvalidateRect(hwnd, nullptr, FALSE);
            return 0;
        }
        case WM_LBUTTONDOWN: {
            g_mx = static_cast<int>(static_cast<short>(LOWORD(lp)));
            g_my = static_cast<int>(static_cast<short>(HIWORD(lp)));
            g_ldown = true;
            SetCapture(hwnd);
            int cx, cy;
            if (screen_to_canvas(g_mx, g_my, cx, cy)) {
                on_canvas_down(cx, cy);
            }
            InvalidateRect(hwnd, nullptr, FALSE);
            return 0;
        }
        case WM_LBUTTONUP: {
            g_mx = static_cast<int>(static_cast<short>(LOWORD(lp)));
            g_my = static_cast<int>(static_cast<short>(HIWORD(lp)));
            int cx, cy;
            if (screen_to_canvas(g_mx, g_my, cx, cy)) {
                on_canvas_up(cx, cy);
            }
            g_ldown = false;
            ReleaseCapture();
            InvalidateRect(hwnd, nullptr, FALSE);
            return 0;
        }
        case WM_MOUSEWHEEL: {
            int delta = GET_WHEEL_DELTA_WPARAM(wp);
            if (GetKeyState(VK_CONTROL) & 0x8000) {
                g_zoom_pct = std::clamp(g_zoom_pct + (delta > 0 ? 10.0f : -10.0f), 25.0f, 400.0f);
            } else {
                g_scroll_y += (delta > 0 ? 20.0f : -20.0f);
            }
            InvalidateRect(hwnd, nullptr, FALSE);
            return 0;
        }
        case WM_KEYDOWN:
            switch (wp) {
                case 'Z': if (GetKeyState(VK_CONTROL) & 0x8000) do_undo(); break;
                case 'Y': if (GetKeyState(VK_CONTROL) & 0x8000) do_redo(); break;
                case VK_ESCAPE: g_has_selection = false; break;
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

} // namespace

// ─────────────────────────────────────────────────────────────────────────────
// WinMain Entry Point
// ─────────────────────────────────────────────────────────────────────────────
int WINAPI WinMain(HINSTANCE inst, HINSTANCE, LPSTR, int nShow) {
    clear_canvas_white();
    copy_surface_full(g_preview, g_canvas);

    const wchar_t* kClass = L"GlimmerWin11Paint";
    WNDCLASSW wc{};
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = inst;
    wc.hCursor       = LoadCursorW(nullptr, MAKEINTRESOURCEW(32512));
    wc.hbrBackground = nullptr;
    wc.lpszClassName = kClass;
    if (!RegisterClassW(&wc)) return 1;

    RECT bounds{0, 0, g_win_w, g_win_h};
    AdjustWindowRect(&bounds, WS_OVERLAPPEDWINDOW, FALSE);
    HWND hwnd = CreateWindowExW(0U, kClass, L"Untitled - Paint",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        bounds.right - bounds.left, bounds.bottom - bounds.top,
        nullptr, nullptr, inst, nullptr);
    if (!hwnd) return 1;

    ShowWindow(hwnd, nShow);
    UpdateWindow(hwnd);
    SetTimer(hwnd, kTimer, 16U, nullptr); // 60 fps

    MSG m{};
    int r = 0;
    while ((r = static_cast<int>(GetMessageW(&m, nullptr, 0U, 0U))) > 0) {
        TranslateMessage(&m);
        DispatchMessageW(&m);
    }
    return r < 0 ? 1 : static_cast<int>(m.wParam);
}
