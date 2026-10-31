#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include "glimmer/glimmer.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace {
constexpr int kCanvasWidth = 980;
constexpr int kCanvasHeight = 720;
constexpr UINT_PTR kFrameTimer = 1U;

glm::Theme make_demo_theme() {
    glm::Theme theme;
    theme.tooltip_delay_frames = 18U;
    return theme;
}

glm::Context context(kCanvasWidth, kCanvasHeight, make_demo_theme());
std::vector<std::uint32_t> dib_pixels(
    static_cast<std::size_t>(kCanvasWidth) * static_cast<std::size_t>(kCanvasHeight));
float volume = 0.65f;
float output_range_minimum = 0.2f;
float output_range_maximum = 0.8f;
float balance = 0.5f;
float fine_tuning = 0.5f;
float pan_x = 0.5f;
float pan_y = 0.5f;
std::uint32_t custom_color = 0xFF56B6C2U;
int output_copies = 2;
int profile_rating = 4;
bool muted = false;
bool notifications = true;
bool spatial_audio = true;
bool advanced_open = true;
bool metadata_open = false;
bool favorite_cinema = true;
bool favorite_studio = false;
bool favorite_night = false;
std::string profile = "Default profile";
std::string applied_profile = "Default profile";
std::string notes = "Use Tab to navigate controls.\n"
                    "Try the combo popup and scrolling areas.";
std::vector<std::string> presets{
    "Balanced", "Bright", "Cinema", "Classic", "Custom", "Night", "Studio", "Warm",
};
std::size_t selected_preset = 1U;
std::size_t preset_offset = 0U;
std::size_t editing_tab = 0U;
const std::vector<std::string> editing_tabs{"Profile", "Notes", "Presets"};
std::vector<std::string> output_modes{"Speakers", "Headphones", "Virtual"};
std::size_t selected_output = 1U;
const std::vector<std::uint32_t> palette_colors{
    0xFF56B6C2U, 0xFFE06C75U, 0xFFE5C07BU, 0xFF98C379U,
    0xFF61AFEFU, 0xFFC678DDU,
};
const std::vector<float> activity_samples{
    0.18f, 0.32f, 0.24f, 0.58f, 0.46f, 0.72f, 0.55f, 0.86f,
    0.63f, 0.78f, 0.42f, 0.68f, 0.91f, 0.74f, 0.96f, 0.82f,
};
const std::vector<float> category_weights{45.0f, 30.0f, 15.0f, 10.0f};
const std::vector<std::pair<float, float>> response_points{
    {0.08f, 0.18f}, {0.16f, 0.28f}, {0.24f, 0.22f}, {0.35f, 0.52f},
    {0.44f, 0.43f}, {0.57f, 0.68f}, {0.69f, 0.58f}, {0.82f, 0.84f},
    {0.93f, 0.76f},
};
const std::vector<glm::ProfileEvent> profiler_events{
    {"Input", 0.0f, 0.42f},
    {"Layout", 0.42f, 1.18f},
    {"Widgets", 1.60f, 2.74f},
    {"Raster", 4.34f, 6.05f},
    {"Blit", 10.39f, 1.82f},
};
std::size_t selected_color = 0U;
glm::DockspaceState workspace{{
    {"controls", "Controls", glm::DockSide::Left},
    {"editor", "Editor", glm::DockSide::Right},
    {"profiler", "Profiler", glm::DockSide::Right},
    {"visuals", "Visuals", glm::DockSide::Right},
}};
constexpr std::array<std::uint32_t, 16> kPreviewImage{{
    0xFF3B82F6U, 0xFF38BDF8U, 0xFF34D399U, 0xFFA3E635U,
    0xFF2563EBU, 0xFF0EA5E9U, 0xFF10B981U, 0xFF84CC16U,
    0xFF1D4ED8U, 0xFF0284C7U, 0xFF059669U, 0xFF65A30DU,
    0xFF1E40AFU, 0xFF0369A1U, 0xFF047857U, 0xFF4D7C0FU,
}};
glm::Surface circle_outline_preview = [] {
    glm::Surface surface(48, 48);
    surface.draw_circle_outline(24, 24, 20, 0xFF56B6C2U, 3);
    return surface;
}();
glm::Surface filled_ellipse_preview = [] {
    glm::Surface surface(48, 32);
    surface.draw_filled_ellipse({2.0f, 2.0f, 44.0f, 28.0f}, 0xFF61AFEFU);
    return surface;
}();
glm::Surface ellipse_outline_preview = [] {
    glm::Surface surface(48, 32);
    surface.draw_ellipse_outline({2.0f, 2.0f, 44.0f, 28.0f},
                                0xFFE5C07BU, 2.0f);
    return surface;
}();
glm::Surface rounded_rect_outline_preview = [] {
    glm::Surface surface(48, 32);
    surface.draw_rounded_rect_outline({2.0f, 2.0f, 44.0f, 28.0f}, 7.0f,
                                      0xFF56B6C2U, 2.0f);
    return surface;
}();
glm::Surface filled_triangle_preview = [] {
    glm::Surface surface(48, 40);
    surface.draw_filled_triangle(4.0f, 36.0f, 24.0f, 4.0f, 44.0f, 36.0f,
                                 0xFF98C379U);
    return surface;
}();
glm::Surface triangle_outline_preview = [] {
    glm::Surface surface(48, 40);
    surface.draw_triangle_outline(4.5f, 35.5f, 24.0f, 4.5f, 43.5f, 35.5f,
                                  0xFFE5C07BU, 2.0f);
    return surface;
}();
glm::Surface polygon_preview = [] {
    glm::Surface surface(48, 40);
    const std::array<float, 12> vertices{{
        3.0f, 3.0f, 44.0f, 3.0f, 44.0f, 13.0f,
        21.0f, 13.0f, 21.0f, 37.0f, 3.0f, 37.0f,
    }};
    surface.draw_filled_polygon(vertices.data(), 6U, 0xFF61AFEFU);
    return surface;
}();
glm::Surface polygon_outline_preview = [] {
    glm::Surface surface(48, 40);
    const std::array<float, 12> vertices{{
        3.0f, 3.0f, 44.0f, 3.0f, 44.0f, 13.0f,
        21.0f, 13.0f, 21.0f, 37.0f, 3.0f, 37.0f,
    }};
    surface.draw_polygon_outline(vertices.data(), 6U, 0xFFE5C07BU, 2.0f);
    return surface;
}();
glm::Surface polyline_preview = [] {
    glm::Surface surface(48, 32);
    const std::array<float, 10> vertices{{
        3.0f, 27.0f, 13.0f, 5.0f, 24.0f, 23.0f,
        35.0f, 6.0f, 45.0f, 27.0f,
    }};
    surface.draw_polyline(vertices.data(), 5U, 0xFF98C379U, 2.0f);
    return surface;
}();
glm::Surface bezier_preview = [] {
    glm::Surface surface(48, 32);
    surface.draw_cubic_bezier(3.0f, 27.0f, 3.0f, 1.0f, 45.0f, 1.0f,
                              45.0f, 27.0f, 0xFF61AFEFU, 2.0f, 24U);
    return surface;
}();
glm::Surface dashed_line_preview = [] {
    glm::Surface surface(48, 12);
    surface.draw_dashed_line(1, 6, 46, 6, 0xFFE5C07BU, 5, 3);
    return surface;
}();
bool mouse_down = false;
int mouse_x = 0;
int mouse_y = 0;

// Renders the complete demo into Glimmer's CPU pixel buffer.
void render_frame() {
    context.begin(mouse_x, mouse_y, mouse_down);
    context.begin_dockspace(workspace, {8.0f, 8.0f, 964.0f, 704.0f});
    context.dock_panel("controls", [] {
        context.label("Immediate-mode controls");
        context.sparkline(activity_samples, 0.0f, 1.0f, 26.0f);
        context.histogram(activity_samples, 0.0f, 1.0f, 8U, 30.0f);
        context.scatter_plot(response_points, 0.0f, 1.0f, 0.0f, 1.0f, 36.0f);
        context.label("Category distribution");
        context.pie_chart(category_weights, palette_colors, 64.0f);
        context.donut_chart(category_weights, palette_colors, 64.0f);
        context.slider("Volume", volume, 0.0f, 1.0f);
        context.tooltip("Drag the slider or use arrow keys.");
        context.stepper("Output copies", output_copies, 1, 16, 1);
        context.tooltip("Use the buttons or arrow keys; Home/End jump to limits.");
        context.toggle("Spatial audio", spatial_audio);
        context.tooltip("Toggle responds to mouse, Enter, and Space.");
        context.checkbox("Mute audio", muted);
        context.tooltip("Checkboxes also participate in Tab focus.");
        context.checkbox("Show notifications", notifications);
        context.separator();
        context.label("Output mode");
        const std::vector<std::string> modes{"Stereo", "Surround", "Studio"};
        static int selected_mode = 1;
        context.radio("Output mode", modes, selected_mode);
        context.tooltip("Use Left/Right or Home/End to choose.");
        context.rating("Profile rating", profile_rating);
        context.tooltip("Use Left/Right to adjust the rating.");
        context.progress_bar("Volume level", volume);
        context.tooltip("Read-only progress display, clamped to 0..1.");
        context.combo("Output", selected_output, output_modes);
        context.tooltip("Open with Enter; Escape closes the popup.");
        context.separator();
        context.collapsing_header("Advanced options", advanced_open);
        context.tooltip("Click or press Enter to expand/collapse.");
        if (advanced_open) {
            context.label(muted ? "Audio is muted" : "Audio is enabled");
            context.label(notifications ? "Notifications on" : "Notifications off");
        }
        if (context.button("Reset volume").clicked) {
            volume = 0.5f;
        }
        context.label("Pinned sound profiles");
        context.selectable("Cinema profile", favorite_cinema);
        context.tooltip("Selectable rows retain their state in caller variables.");
        context.selectable("Studio profile", favorite_studio);
        context.selectable("Night profile", favorite_night);
        context.label("Accent palette");
        context.color_palette("Accent colors", palette_colors, selected_color, 3U);
        context.tooltip("Choose a color with mouse, arrows, or Home/End.");
        const glm::Rect color_preview = context.row(24.0f);
        context.image(&palette_colors[selected_color], 1, 1, color_preview);
    });
    context.dock_panel("editor", [] {
        context.tab_bar("Editing tabs", editing_tabs, editing_tab);
        context.tooltip("Switch pages with the mouse or arrow keys.");
        if (editing_tab == 0U) {
            context.label_wrapped(
                "Text widgets clip to their bounds and support keyboard editing.");
            context.text_input("Profile", profile, 32U);
            context.tooltip("Click here, then type a short profile name.");
            if (context.button("Apply profile", !profile.empty()).clicked) {
                applied_profile = profile;
            }
            context.label("Applied: " + applied_profile);
            context.label("Device options");
            context.begin_table("Device options", 2U);
            context.label("Output");
            context.label("Stereo");
            context.next_table_column();
            context.label("Sample rate");
            context.label("48 kHz");
            context.next_table_row();
            context.label("Status");
            context.label("Ready");
            context.end_table();
        } else if (editing_tab == 1U) {
            context.label("Multi-line notes");
            context.multiline_text_input("Notes", notes, 3U, 256U);
            context.tooltip("Enter inserts a newline; Up/Down preserve the column.");
            context.label("Two-dimensional pan");
            context.xy_pad("Pan", pan_x, pan_y, 0.0f, 1.0f, 0.0f, 1.0f,
                           112.0f);
            context.tooltip("Drag to set X/Y; positive Y points upward.");
            if (context.begin_tree_node("Note metadata", metadata_open)) {
                context.label("Caller-owned open state");
                context.label("Nested controls get scoped widget IDs");
                context.end_tree_node();
            }
            context.label("RGBA color editor");
            context.color_picker("Custom accent", custom_color);
            context.tooltip("Edit each packed color channel with drag or keys.");
        } else {
            context.label("Scrollable preset list");
            const glm::Interaction selection = context.list(
                "Presets", presets, selected_preset, preset_offset, 5U);
            if (selection.changed && selected_preset < presets.size()) {
                profile = presets[selected_preset];
            }
            context.tooltip("Hover and use the mouse wheel; arrows also navigate.");
        }
    });
    context.dock_panel("visuals", [] {
        context.label("Knob, range and vertical sliders");
        context.knob("Balance", balance, 0.0f, 1.0f);
        context.tooltip("Turn with the mouse; arrows adjust by one percent.");
        context.drag_value("Fine tuning", fine_tuning, 0.0f, 1.0f, 0.002f);
        context.tooltip("Drag horizontally for precise adjustments.");
        context.range_slider("Output range", output_range_minimum,
                             output_range_maximum, 0.0f, 1.0f);
        context.tooltip("Drag either handle or use Tab and arrow keys.");
        context.label("Image button");
        context.vertical_slider("Preview level", volume, 0.0f, 1.0f, 100.0f);
        context.tooltip("Drag vertically or use arrow keys.");
        if (context.image_button("Image preview", kPreviewImage.data(), 4, 4,
                                 96.0f).clicked) {
            selected_output = (selected_output + 1U) % output_modes.size();
        }
        context.tooltip("Click or press Enter to cycle the output mode.");
        context.label("Tinted image");
        context.tinted_image(kPreviewImage.data(), 4, 4, context.row(40.0f),
                             0xFF80C0FFU);
        context.separator();
        context.label("Accent gradient");
        context.gradient(0xFF56B6C2U, 0xFFE06C75U, 24.0f);
        context.vertical_gradient(0xFF56B6C2U, 0xFFE06C75U, 24.0f);
        context.bilinear_gradient(0xFF56B6C2U, 0xFFE06C75U,
                                  0xFF61AFEFU, 0xFFE5C07BU, 24.0f);
        context.label("Transparency preview");
        context.checkerboard(0xFF30343BU, 0xFF596170U, 32.0f, 8);
        context.label("Outlined circle");
        context.image(circle_outline_preview, context.row(48.0f));
        context.label("Filled ellipse");
        context.image(filled_ellipse_preview, context.row(32.0f));
        context.label("Ellipse outline");
        context.image(ellipse_outline_preview, context.row(32.0f));
        context.label("Rounded outline");
        context.image(rounded_rect_outline_preview, context.row(32.0f));
        context.label("Filled triangle");
        context.image(filled_triangle_preview, context.row(40.0f));
        context.label("Triangle outline");
        context.image(triangle_outline_preview, context.row(40.0f));
        context.label("Concave polygon");
        context.image(polygon_preview, context.row(40.0f));
        context.label("Polygon outline");
        context.image(polygon_outline_preview, context.row(40.0f));
        context.label("Open polyline");
        context.image(polyline_preview, context.row(32.0f));
        context.label("Cubic Bezier");
        context.image(bezier_preview, context.row(32.0f));
        context.label("Dashed line");
        context.image(dashed_line_preview, context.row(12.0f));
        context.label("Scrollable child");
        const auto visual_panel = std::find_if(
            workspace.panels.begin(), workspace.panels.end(),
            [](const glm::DockPanel& panel) { return panel.id == "visuals"; });
        const float dock_inner_x = workspace.rect.x + 1.0f;
        const float dock_inner_width = std::max(0.0f, workspace.rect.w - 7.0f);
        const float dock_left_width = dock_inner_width * workspace.left_ratio;
        const bool visual_is_left =
            visual_panel != workspace.panels.end() &&
            visual_panel->side == glm::DockSide::Left;
        const float visual_x =
            dock_inner_x + (visual_is_left ? 0.0f : dock_left_width + 5.0f);
        const float visual_width =
            visual_is_left ? dock_left_width : dock_inner_width - dock_left_width;
        context.begin_child(
            "Demo child",
            {visual_x + 8.0f, workspace.rect.y + 400.0f,
             std::max(0.0f, visual_width - 16.0f), 260.0f});
        for (int line = 1; line <= 12; ++line) {
            context.label("Child content line " + std::to_string(line));
        }
        context.end_child();
    });
    context.dock_panel("profiler", [] {
        context.label("CPU frame profile");
        context.label("Example CPU trace - 10.39 ms of 16.67 ms budget");
        context.profiler_timeline(profiler_events, 16.67f, 16.67f, 150.0f);
        context.label("CPU frame history");
        context.sparkline(activity_samples, 0.0f, 1.0f, 52.0f);
        context.label("Subsystem timings (sample)");
        context.begin_table("Profiler timings", 2U);
        context.label("Input");
        context.label("0.42 ms");
        context.next_table_column();
        context.label("Layout");
        context.label("1.18 ms");
        context.next_table_row();
        context.label("Widget build");
        context.label("2.74 ms");
        context.next_table_column();
        context.label("Raster");
        context.label("6.05 ms");
        context.next_table_row();
        context.label("Image blit");
        context.label("1.82 ms");
        context.next_table_column();
        context.label("Budget");
        context.label("16.67 ms");
        context.end_table();
        context.progress_bar("Frame budget", 10.39f / 16.67f);
    });
    context.end_dockspace();
    context.end();

    const std::uint32_t* source = context.pixels();
    for (std::size_t index = 0; index < dib_pixels.size(); ++index) {
        const std::uint32_t pixel = source[index];
        const std::uint32_t red = pixel & 0xFFU;
        const std::uint32_t green = (pixel >> 8U) & 0xFFU;
        const std::uint32_t blue = (pixel >> 16U) & 0xFFU;
        dib_pixels[index] = (red << 16U) | (green << 8U) | blue;
    }
}

// Presents Glimmer's packed pixels as a top-down Windows device-independent bitmap.
void present_frame(HDC device_context) {
    render_frame();

    BITMAPINFO bitmap_info{};
    bitmap_info.bmiHeader.biSize = sizeof(bitmap_info.bmiHeader);
    bitmap_info.bmiHeader.biWidth = kCanvasWidth;
    bitmap_info.bmiHeader.biHeight = -kCanvasHeight;
    bitmap_info.bmiHeader.biPlanes = 1;
    bitmap_info.bmiHeader.biBitCount = 32;
    bitmap_info.bmiHeader.biCompression = BI_RGB;
    StretchDIBits(device_context, 0, 0, kCanvasWidth, kCanvasHeight,
                  0, 0, kCanvasWidth, kCanvasHeight, dib_pixels.data(),
                  &bitmap_info, DIB_RGB_COLORS, SRCCOPY);
}

// Updates mouse coordinates from a client-area mouse message.
void update_mouse_position(LPARAM position) {
    mouse_x = static_cast<int>(static_cast<short>(LOWORD(position)));
    mouse_y = static_cast<int>(static_cast<short>(HIWORD(position)));
}

// Translates Windows input and paint messages into Glimmer frames.
LRESULT CALLBACK window_proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
    switch (message) {
        case WM_PAINT: {
            PAINTSTRUCT paint{};
            HDC device_context = BeginPaint(window, &paint);
            present_frame(device_context);
            EndPaint(window, &paint);
            return 0;
        }
        case WM_ERASEBKGND:
            return 1;
        case WM_TIMER:
            if (wparam == kFrameTimer) {
                InvalidateRect(window, nullptr, FALSE);
            }
            return 0;
        case WM_MOUSEMOVE:
            update_mouse_position(lparam);
            return 0;
        case WM_LBUTTONDOWN:
            update_mouse_position(lparam);
            mouse_down = true;
            SetCapture(window);
            InvalidateRect(window, nullptr, FALSE);
            return 0;
        case WM_LBUTTONUP:
            update_mouse_position(lparam);
            mouse_down = false;
            ReleaseCapture();
            InvalidateRect(window, nullptr, FALSE);
            return 0;
        case WM_CAPTURECHANGED:
            mouse_down = false;
            return 0;
        case WM_MOUSEWHEEL: {
            const int wheel_delta = GET_WHEEL_DELTA_WPARAM(wparam);
            POINT wheel_position{
                static_cast<LONG>(static_cast<short>(LOWORD(lparam))),
                static_cast<LONG>(static_cast<short>(HIWORD(lparam))),
            };
            ScreenToClient(window, &wheel_position);
            mouse_x = static_cast<int>(wheel_position.x);
            mouse_y = static_cast<int>(wheel_position.y);
            context.feed_wheel(wheel_delta);
            InvalidateRect(window, nullptr, FALSE);
            return 0;
        }
        case WM_CHAR:
            if (wparam >= 32U && wparam <= 126U) {
                context.feed_char(static_cast<char>(wparam));
                InvalidateRect(window, nullptr, FALSE);
            }
            return 0;
        case WM_KEYDOWN:
            switch (wparam) {
                case VK_TAB:
                    context.feed_key(glm::Key::Tab, (GetKeyState(VK_SHIFT) & 0x8000) != 0);
                    break;
                case VK_BACK:
                    context.feed_key(glm::Key::Backspace);
                    break;
                case VK_DELETE:
                    context.feed_key(glm::Key::Delete);
                    break;
                case VK_LEFT:
                    context.feed_key(glm::Key::Left);
                    break;
                case VK_RIGHT:
                    context.feed_key(glm::Key::Right);
                    break;
                case VK_HOME:
                    context.feed_key(glm::Key::Home);
                    break;
                case VK_END:
                    context.feed_key(glm::Key::End);
                    break;
                case VK_RETURN:
                    context.feed_key(glm::Key::Enter);
                    break;
                case VK_SPACE:
                    context.feed_key(glm::Key::Space);
                    break;
                case VK_UP:
                    context.feed_key(glm::Key::Up);
                    break;
                case VK_DOWN:
                    context.feed_key(glm::Key::Down);
                    break;
                case VK_ESCAPE:
                    context.feed_key(glm::Key::Escape);
                    break;
                default:
                    return DefWindowProcW(window, message, wparam, lparam);
            }
            InvalidateRect(window, nullptr, FALSE);
            return 0;
        case WM_DESTROY:
            KillTimer(window, kFrameTimer);
            PostQuitMessage(0);
            return 0;
        default:
            return DefWindowProcW(window, message, wparam, lparam);
    }
}

}  // namespace

// Opens the native interactive demo window and runs its input/render loop.
int WINAPI WinMain(HINSTANCE instance, HINSTANCE, LPSTR, int show_command) {
    const wchar_t* class_name = L"GlimmerLiveDemoWindow";
    WNDCLASSW window_class{};
    window_class.lpfnWndProc = window_proc;
    window_class.hInstance = instance;
    window_class.hCursor = LoadCursorW(nullptr, MAKEINTRESOURCEW(32512));
    window_class.hbrBackground = nullptr;
    window_class.lpszClassName = class_name;

    if (RegisterClassW(&window_class) == 0U) {
        MessageBoxW(nullptr, L"Could not register the Glimmer demo window.",
                    L"Glimmer", MB_OK | MB_ICONERROR);
        return 1;
    }

    RECT bounds{0, 0, kCanvasWidth, kCanvasHeight};
    AdjustWindowRect(&bounds, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU |
                                  WS_MINIMIZEBOX, FALSE);
    HWND window = CreateWindowExW(
        0U, class_name, L"Glimmer - CPU-rendered GUI demo",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, bounds.right - bounds.left,
        bounds.bottom - bounds.top, nullptr, nullptr, instance, nullptr);
    if (window == nullptr) {
        MessageBoxW(nullptr, L"Could not create the Glimmer demo window.",
                    L"Glimmer", MB_OK | MB_ICONERROR);
        return 1;
    }

    ShowWindow(window, show_command);
    UpdateWindow(window);
    if (SetTimer(window, kFrameTimer, 33U, nullptr) == 0U) {
        MessageBoxW(window, L"Could not start the demo frame timer.",
                    L"Glimmer", MB_OK | MB_ICONERROR);
        DestroyWindow(window);
        return 1;
    }

    MSG message{};
    int result = 0;
    while ((result = static_cast<int>(GetMessageW(&message, nullptr, 0U, 0U))) > 0) {
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }
    return result < 0 ? 1 : static_cast<int>(message.wParam);
}
