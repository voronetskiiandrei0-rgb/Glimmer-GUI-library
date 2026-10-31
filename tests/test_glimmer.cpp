#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "glimmer/glimmer.hpp"

#include <array>
#include <cstdint>
#include <limits>
#include <string>
#include <utility>
#include <vector>

TEST_CASE("surface pixels follow the packed RGBA golden image") {
    glm::Surface surface(3, 2);
    surface.fill_rect({0.0f, 0.0f, 3.0f, 2.0f}, 0xFF010203U);
    surface.fill_rect({1.0f, 0.0f, 1.0f, 1.0f}, 0xFF0A0B0CU);
    const std::array<std::uint32_t, 6> expected{{
        0xFF010203U, 0xFF0A0B0CU, 0xFF010203U,
        0xFF010203U, 0xFF010203U, 0xFF010203U,
    }};
    for (std::size_t index = 0; index < expected.size(); ++index) {
        CHECK(surface.pixels()[index] == expected[index]);
    }
}

TEST_CASE("surface clips drawing and source-over blends alpha") {
    glm::Surface surface(4, 4);
    surface.push_clip({1.0f, 1.0f, 2.0f, 2.0f});
    surface.fill_rect({0.0f, 0.0f, 4.0f, 4.0f}, 0xFFFF0000U);
    surface.pop_clip();
    CHECK(surface.pixels()[0] == 0U);
    CHECK(surface.pixels()[5] == 0xFFFF0000U);
    surface.fill_rect({1.0f, 1.0f, 1.0f, 1.0f}, 0x800000FFU);
    CHECK(surface.pixels()[5] == 0xFF7F0080U);
}

TEST_CASE("horizontal gradients interpolate packed colors deterministically") {
    glm::Surface surface(5, 2);
    surface.draw_horizontal_gradient({0.0f, 0.0f, 5.0f, 1.0f},
                                     0xFF0000FFU, 0xFFFF0000U);
    const std::array<std::uint32_t, 5> expected{{
        0xFF0000FFU, 0xFF4000BFU, 0xFF800080U, 0xFFBF0040U, 0xFFFF0000U,
    }};
    for (std::size_t x = 0; x < expected.size(); ++x) {
        CHECK(surface.pixels()[x] == expected[x]);
    }

    glm::Surface clipped(5, 1);
    clipped.push_clip({1.0f, 0.0f, 3.0f, 1.0f});
    clipped.draw_horizontal_gradient({0.0f, 0.0f, 5.0f, 1.0f},
                                     0xFF0000FFU, 0xFFFF0000U);
    clipped.pop_clip();
    CHECK(clipped.pixels()[0] == 0U);
    CHECK(clipped.pixels()[1] == expected[1]);
    CHECK(clipped.pixels()[2] == expected[2]);
    CHECK(clipped.pixels()[3] == expected[3]);
    CHECK(clipped.pixels()[4] == 0U);
}

TEST_CASE("horizontal gradient blends alpha and handles one-pixel widths") {
    glm::Surface surface(3, 1);
    surface.fill_rect({0.0f, 0.0f, 3.0f, 1.0f}, 0xFF202020U);
    surface.draw_horizontal_gradient({0.0f, 0.0f, 3.0f, 1.0f},
                                     0x000000FFU, 0xFFFF0000U);
    CHECK(surface.pixels()[0] == 0xFF202020U);
    CHECK(surface.pixels()[1] == 0xFF501050U);
    CHECK(surface.pixels()[2] == 0xFFFF0000U);

    glm::Surface single_pixel(1, 1);
    single_pixel.draw_horizontal_gradient({0.0f, 0.0f, 1.0f, 1.0f},
                                          0xFF123456U, 0xFFABCDEFU);
    CHECK(single_pixel.pixels()[0] == 0xFF123456U);
}

TEST_CASE("vertical gradients interpolate packed colors and respect clipping") {
    glm::Surface surface(2, 5);
    surface.draw_vertical_gradient({0.0f, 0.0f, 1.0f, 5.0f},
                                   0xFF0000FFU, 0xFFFF0000U);
    const std::array<std::uint32_t, 5> expected{{
        0xFF0000FFU, 0xFF4000BFU, 0xFF800080U, 0xFFBF0040U, 0xFFFF0000U,
    }};
    for (std::size_t y = 0; y < expected.size(); ++y) {
        CHECK(surface.pixels()[y * 2U] == expected[y]);
        CHECK(surface.pixels()[y * 2U + 1U] == 0U);
    }

    glm::Surface clipped(1, 5);
    clipped.push_clip({0.0f, 1.0f, 1.0f, 3.0f});
    clipped.draw_vertical_gradient({0.0f, 0.0f, 1.0f, 5.0f},
                                   0xFF0000FFU, 0xFFFF0000U);
    clipped.pop_clip();
    CHECK(clipped.pixels()[0] == 0U);
    CHECK(clipped.pixels()[1] == expected[1]);
    CHECK(clipped.pixels()[2] == expected[2]);
    CHECK(clipped.pixels()[3] == expected[3]);
    CHECK(clipped.pixels()[4] == 0U);
}

TEST_CASE("vertical gradient blends alpha and handles one-pixel heights") {
    glm::Surface surface(1, 3);
    surface.fill_rect({0.0f, 0.0f, 1.0f, 3.0f}, 0xFF202020U);
    surface.draw_vertical_gradient({0.0f, 0.0f, 1.0f, 3.0f},
                                   0x000000FFU, 0xFFFF0000U);
    CHECK(surface.pixels()[0] == 0xFF202020U);
    CHECK(surface.pixels()[1] == 0xFF501050U);
    CHECK(surface.pixels()[2] == 0xFFFF0000U);

    glm::Surface single_pixel(1, 1);
    single_pixel.draw_vertical_gradient({0.0f, 0.0f, 1.0f, 1.0f},
                                        0xFF123456U, 0xFFABCDEFU);
    CHECK(single_pixel.pixels()[0] == 0xFF123456U);
}

TEST_CASE("context gradient fills a layout row") {
    glm::Theme theme;
    theme.padding = 0;
    glm::Context ui(5, 1, theme);
    ui.begin(0, 0, false);
    ui.gradient(0xFF0000FFU, 0xFFFF0000U, 1.0f);
    ui.end();
    CHECK(ui.pixels()[0] == 0xFF0000FFU);
    CHECK(ui.pixels()[4] == 0xFFFF0000U);
}

TEST_CASE("context vertical gradient fills a layout column") {
    glm::Theme theme;
    theme.padding = 0;
    glm::Context ui(1, 5, theme);
    ui.begin(0, 0, false);
    ui.vertical_gradient(0xFF0000FFU, 0xFFFF0000U, 5.0f);
    ui.end();
    CHECK(ui.pixels()[0] == 0xFF0000FFU);
    CHECK(ui.pixels()[4] == 0xFFFF0000U);
}

TEST_CASE("bilinear gradients interpolate four corners deterministically") {
    glm::Surface surface(3, 3);
    surface.draw_bilinear_gradient(
        {0.0f, 0.0f, 3.0f, 3.0f}, 0xFF0000FFU, 0xFF00FF00U,
        0xFFFF0000U, 0xFFFFFFFFU);
    CHECK(surface.pixels()[0] == 0xFF0000FFU);
    CHECK(surface.pixels()[2] == 0xFF00FF00U);
    CHECK(surface.pixels()[6] == 0xFFFF0000U);
    CHECK(surface.pixels()[8] == 0xFFFFFFFFU);
    CHECK(surface.pixels()[4] == 0xFF808080U);
    CHECK(surface.pixels()[1] == 0xFF008080U);
    CHECK(surface.pixels()[7] == 0xFFFF8080U);

    glm::Surface clipped(3, 3);
    clipped.push_clip({1.0f, 1.0f, 1.0f, 1.0f});
    clipped.draw_bilinear_gradient(
        {0.0f, 0.0f, 3.0f, 3.0f}, 0xFF0000FFU, 0xFF00FF00U,
        0xFFFF0000U, 0xFFFFFFFFU);
    clipped.pop_clip();
    CHECK(clipped.pixels()[0] == 0U);
    CHECK(clipped.pixels()[4] == 0xFF808080U);
    CHECK(clipped.pixels()[8] == 0U);
}

TEST_CASE("bilinear gradients blend alpha and support single-pixel extents") {
    glm::Surface surface(1, 1);
    surface.fill_rect({0.0f, 0.0f, 1.0f, 1.0f}, 0xFF202020U);
    surface.draw_bilinear_gradient(
        {0.0f, 0.0f, 1.0f, 1.0f}, 0x000000FFU, 0xFFFF0000U,
        0xFF00FF00U, 0xFFFFFFFFU);
    CHECK(surface.pixels()[0] == 0xFF202020U);

    glm::Surface row(3, 1);
    row.draw_bilinear_gradient(
        {0.0f, 0.0f, 3.0f, 1.0f}, 0xFF0000FFU, 0xFFFF0000U,
        0xFF0000FFU, 0xFFFF0000U);
    CHECK(row.pixels()[0] == 0xFF0000FFU);
    CHECK(row.pixels()[1] == 0xFF800080U);
    CHECK(row.pixels()[2] == 0xFFFF0000U);
}

TEST_CASE("context bilinear gradient fills a layout row") {
    glm::Theme theme;
    theme.padding = 0;
    glm::Context ui(3, 3, theme);
    ui.begin(0, 0, false);
    ui.bilinear_gradient(0xFF0000FFU, 0xFF00FF00U, 0xFFFF0000U,
                         0xFFFFFFFFU, 3.0f);
    ui.end();
    CHECK(ui.pixels()[0] == 0xFF0000FFU);
    CHECK(ui.pixels()[2] == 0xFF00FF00U);
    CHECK(ui.pixels()[6] == 0xFFFF0000U);
    CHECK(ui.pixels()[8] == 0xFFFFFFFFU);
    CHECK(ui.pixels()[4] == 0xFF808080U);
}

TEST_CASE("checkerboard alternates square cells and preserves phase under clipping") {
    glm::Surface surface(6, 4);
    surface.draw_checkerboard({0.0f, 0.0f, 6.0f, 4.0f},
                              0xFF101010U, 0xFFF0F0F0U, 2);
    for (int y = 0; y < 4; ++y) {
        for (int x = 0; x < 6; ++x) {
            const bool light = ((x / 2) + (y / 2)) % 2 != 0;
            const std::uint32_t expected =
                light ? 0xFFF0F0F0U : 0xFF101010U;
            CHECK(surface.pixels()[static_cast<std::size_t>(y * 6 + x)] ==
                  expected);
        }
    }

    glm::Surface clipped(6, 4);
    clipped.push_clip({1.0f, 1.0f, 4.0f, 2.0f});
    clipped.draw_checkerboard({0.0f, 0.0f, 6.0f, 4.0f},
                              0xFF101010U, 0xFFF0F0F0U, 2);
    clipped.pop_clip();
    CHECK(clipped.pixels()[0] == 0U);
    CHECK(clipped.pixels()[1U * 6U + 1U] == 0xFF101010U);
    CHECK(clipped.pixels()[1U * 6U + 2U] == 0xFFF0F0F0U);
    CHECK(clipped.pixels()[2U * 6U + 1U] == 0xFFF0F0F0U);
    CHECK(clipped.pixels()[3U * 6U + 1U] == 0U);
}

TEST_CASE("checkerboard clamps cell size and source-over blends each cell") {
    glm::Surface surface(3, 1);
    surface.fill_rect({0.0f, 0.0f, 3.0f, 1.0f}, 0xFF202020U);
    surface.draw_checkerboard({0.0f, 0.0f, 3.0f, 1.0f},
                              0x800000FFU, 0x8000FF00U, 0);
    CHECK(surface.pixels()[0] == 0xFF101090U);
    CHECK(surface.pixels()[1] == 0xFF109010U);
    CHECK(surface.pixels()[2] == 0xFF101090U);
}

TEST_CASE("context checkerboard fills a layout row") {
    glm::Theme theme;
    theme.padding = 0;
    glm::Context ui(4, 2, theme);
    ui.begin(0, 0, false);
    ui.checkerboard(0xFF000000U, 0xFFFFFFFFU, 2.0f, 1);
    ui.end();
    CHECK(ui.pixels()[0] == 0xFF000000U);
    CHECK(ui.pixels()[1] == 0xFFFFFFFFU);
    CHECK(ui.pixels()[4] == 0xFFFFFFFFU);
    CHECK(ui.pixels()[5] == 0xFF000000U);
}

TEST_CASE("lines and the built-in font respect clipping and measurement") {
    glm::Surface surface(32, 24);
    surface.draw_line(0, 0, 4, 4, 0xFFFFFFFFU);
    CHECK(surface.pixels()[0] == 0xFFFFFFFFU);
    CHECK(surface.pixels()[4U * 32U + 4U] == 0xFFFFFFFFU);
    CHECK(surface.measure_text("Hi").w == 16.0f);
    CHECK(surface.measure_text("A\nB", 2).h == 64.0f);
    surface.push_clip({0.0f, 0.0f, 8.0f, 16.0f});
    surface.draw_text(0, 0, "A", 0xFFFFFFFFU);
    surface.pop_clip();
    CHECK(surface.pixels()[0] == 0xFFFFFFFFU);
    glm::Surface clipped_text(8, 16);
    clipped_text.push_clip({0.0f, 0.0f, 4.0f, 16.0f});
    clipped_text.draw_text(0, 0, "A", 0xFFFFFFFFU);
    clipped_text.pop_clip();
    bool has_ink = false;
    for (int y = 0; y < clipped_text.height(); ++y) {
        for (int x = 0; x < clipped_text.width(); ++x) {
            const bool filled = clipped_text.pixels()[y * clipped_text.width() + x] != 0U;
            const bool outside_clip = filled && x >= 4;
            CHECK_FALSE(outside_clip);
            has_ink = has_ink || filled;
        }
    }
    CHECK(has_ink);
}

TEST_CASE("text measurement and clipping handle empty and fallback text") {
    glm::Surface empty_text(8, 16);
    empty_text.fill_rect({0.0f, 0.0f, 8.0f, 16.0f}, 0xFF010203U);
    const glm::Rect empty_measurement = empty_text.measure_text("");
    CHECK(empty_measurement.w == 0.0f);
    CHECK(empty_measurement.h == 16.0f);
    CHECK(empty_text.measure_text("", 2).h == 32.0f);
    empty_text.draw_text(0, 0, "", 0xFFFFFFFFU);
    for (int index = 0; index < 8 * 16; ++index) {
        CHECK(empty_text.pixels()[index] == 0xFF010203U);
    }

    glm::Surface clipped_text(8, 16);
    clipped_text.push_clip({0.0f, 0.0f, 4.0f, 16.0f});
    clipped_text.draw_text(-4, 0, "A", 0xFFFFFFFFU);
    clipped_text.pop_clip();
    bool has_visible_ink = false;
    for (int y = 0; y < clipped_text.height(); ++y) {
        for (int x = 0; x < clipped_text.width(); ++x) {
            const bool filled = clipped_text.pixels()[y * clipped_text.width() + x] != 0U;
            if (x >= 4) {
                CHECK_FALSE(filled);
            }
            has_visible_ink = has_visible_ink || filled;
        }
    }
    CHECK(has_visible_ink);

    const std::string non_ascii("\xC3\xA9", 2U);
    glm::Surface fallback(16, 16);
    glm::Surface question_marks(16, 16);
    fallback.draw_text(0, 0, non_ascii, 0xFFFFFFFFU);
    question_marks.draw_text(0, 0, "??", 0xFFFFFFFFU);
    CHECK(fallback.measure_text(non_ascii).w == 16.0f);
    for (int index = 0; index < 16 * 16; ++index) {
        CHECK(fallback.pixels()[index] == question_marks.pixels()[index]);
    }
}

TEST_CASE("context detects button clicks and updates interactive widgets") {
    glm::Context ui(120, 80);
    ui.begin(10, 10, true);
    const glm::Interaction pressed = ui.button("Go");
    CHECK(pressed.active);
    ui.end();
    ui.begin(10, 10, false);
    const glm::Interaction clicked = ui.button("Go");
    CHECK(clicked.clicked);
    bool contextual_click = false;
    if (clicked) {
        contextual_click = true;
    }
    CHECK(contextual_click);
    ui.end();

    bool checked = false;
    ui.begin(10, 10, true);
    ui.checkbox("Enabled", checked);
    ui.end();
    ui.begin(10, 10, false);
    const glm::Interaction toggled = ui.checkbox("Enabled", checked);
    CHECK(toggled.changed);
    CHECK(checked);
    ui.end();
}

TEST_CASE("slider, text input, and list values are bounded and editable") {
    glm::Context ui(120, 100);
    float value = 0.5f;
    ui.begin(80, 16, true);
    const glm::Interaction moved = ui.slider("gain", value, 0.0f, 1.0f);
    CHECK(moved.active);
    CHECK(value > 0.5f);
    ui.end();

    std::string input = "a";
    ui.begin(80, 16, false);
    ui.slider("gain", value, 0.0f, 1.0f);
    ui.end();
    ui.begin(10, 10, true);
    ui.text_input("Name", input);
    ui.end();
    ui.begin(10, 10, false);
    ui.text_input("Name", input);
    ui.feed_char('b');
    const glm::Interaction edited = ui.text_input("Name", input);
    CHECK(input == "ab");
    CHECK(edited.changed);
    CHECK(ui.cursor() == 2U);
    ui.feed_key(glm::Key::Backspace);
    ui.text_input("Name", input);
    CHECK(input == "a");
    ui.end();

    const std::vector<std::string> items{"one", "two", "three"};
    std::size_t selected = 0U;
    std::size_t offset = 0U;
    ui.scroll(items.size(), 1U, 10, offset);
    CHECK(offset == 2U);
    ui.scroll(items.size(), 1U, -10, offset);
    CHECK(offset == 0U);
    selected = 2U;
    ui.begin(10, 10, true);
    ui.list("Items", items, selected, offset, 2U);
    ui.end();
    ui.begin(10, 10, false);
    const glm::Interaction list_result = ui.list("Items", items, selected, offset, 2U);
    CHECK(list_result.changed);
    CHECK(selected == 0U);
    ui.end();
}

TEST_CASE("nested windows scope widget IDs") {
    glm::Context ui(200, 120);
    std::uint64_t first = 0U;
    std::uint64_t second = 0U;
    ui.begin(10, 40, false);
    ui.window("First", {0.0f, 0.0f, 90.0f, 100.0f},
              [&] { ui.button("Same"); });
    first = ui.hot_id();
    ui.end();
    ui.begin(110, 40, false);
    ui.window("Second", {100.0f, 0.0f, 90.0f, 100.0f},
              [&] { ui.button("Same"); });
    second = ui.hot_id();
    ui.end();
    CHECK(first != second);
}

TEST_CASE("keyboard focus traverses and activates widgets") {
    glm::Context ui(140, 120);
    bool checked = false;
    float value = 0.5f;

    const auto draw_widgets = [&] {
        return std::array<glm::Interaction, 3>{
            ui.button("Run"), ui.checkbox("Enabled", checked),
            ui.slider("Gain", value, 0.0f, 1.0f)};
    };

    ui.begin(0, 0, false);
    draw_widgets();
    ui.end();

    ui.feed_key(glm::Key::Tab);
    ui.feed_key(glm::Key::Space);
    ui.begin(0, 0, false);
    const auto button_widgets = draw_widgets();
    CHECK(button_widgets[0].clicked);
    CHECK_FALSE(button_widgets[1].clicked);
    ui.end();

    ui.feed_key(glm::Key::Tab);
    ui.feed_key(glm::Key::Space);
    ui.begin(0, 0, false);
    const auto checkbox_widgets = draw_widgets();
    CHECK(checkbox_widgets[1].changed);
    CHECK(checked);
    ui.end();

    const float previous_value = value;
    ui.feed_key(glm::Key::Tab);
    ui.feed_key(glm::Key::Right);
    ui.begin(0, 0, false);
    const auto slider_widgets = draw_widgets();
    CHECK(slider_widgets[2].changed);
    CHECK(value > previous_value);
    ui.end();

    ui.feed_key(glm::Key::Tab);
    ui.begin(0, 0, false);
    const auto wrapped_widgets = draw_widgets();
    CHECK_FALSE(wrapped_widgets[0].clicked);
    ui.end();
    ui.feed_key(glm::Key::Space);
    ui.begin(0, 0, false);
    const auto repeated_widgets = draw_widgets();
    CHECK(repeated_widgets[0].clicked);
    ui.end();

    const float value_before_reverse = value;
    ui.feed_key(glm::Key::Tab, true);
    ui.feed_key(glm::Key::Left);
    ui.begin(0, 0, false);
    const auto reverse_widgets = draw_widgets();
    CHECK(reverse_widgets[2].changed);
    CHECK(value < value_before_reverse);
    ui.end();
}

TEST_CASE("keyboard list navigation keeps the selection visible") {
    glm::Context ui(140, 100);
    const std::vector<std::string> items{"One", "Two", "Three", "Four"};
    std::size_t selected = 0U;
    std::size_t offset = 0U;

    ui.begin(0, 0, false);
    ui.list("Presets", items, selected, offset, 2U);
    ui.end();

    ui.feed_key(glm::Key::Tab);
    ui.feed_key(glm::Key::Down);
    ui.feed_key(glm::Key::Down);
    ui.begin(0, 0, false);
    const glm::Interaction moved = ui.list("Presets", items, selected, offset, 2U);
    CHECK(moved.changed);
    CHECK(selected == 2U);
    CHECK(offset == 1U);
    ui.end();

    ui.feed_key(glm::Key::End);
    ui.begin(0, 0, false);
    const glm::Interaction end = ui.list("Presets", items, selected, offset, 2U);
    CHECK(end.changed);
    CHECK(selected == 3U);
    CHECK(offset == 2U);
    ui.end();
}

TEST_CASE("row and column cursors allocate exact padded rectangles and overflow") {
    glm::Theme theme;
    theme.padding = 6;
    glm::Context ui(100, 72, theme);

    ui.begin(0, 0, false);
    const glm::Rect first_row = ui.row();
    const glm::Rect second_row = ui.row(12.0f);
    ui.next_row(8.0f);
    const glm::Rect first_column = ui.column(30.0f);
    const glm::Rect second_column = ui.column(60.0f);
    const glm::Rect overflow_row = ui.row(10.0f);
    ui.end();

    CHECK(first_row.x == 6.0f);
    CHECK(first_row.y == 6.0f);
    CHECK(first_row.w == 88.0f);
    CHECK(first_row.h == 24.0f);
    CHECK(second_row.x == 6.0f);
    CHECK(second_row.y == 36.0f);
    CHECK(second_row.w == 88.0f);
    CHECK(second_row.h == 12.0f);
    CHECK(first_column.x == 6.0f);
    CHECK(first_column.y == 68.0f);
    CHECK(first_column.w == 30.0f);
    CHECK(first_column.h == 24.0f);
    CHECK(second_column.x == 42.0f);
    CHECK(second_column.y == 68.0f);
    CHECK(second_column.w == 60.0f);
    CHECK(second_column.h == 24.0f);
    CHECK(overflow_row.x == 108.0f);
    CHECK(overflow_row.y == 68.0f);
    CHECK(overflow_row.w == 88.0f);
    CHECK(overflow_row.h == 10.0f);
}

TEST_CASE("nested window rows and columns stay within parent content") {
    glm::Theme theme;
    theme.padding = 4;
    glm::Context ui(220, 180, theme);
    glm::Rect outer_first_row;
    glm::Rect inner_row;
    glm::Rect inner_first_column;
    glm::Rect inner_second_column;
    glm::Rect outer_second_row;

    ui.begin(0, 0, false);
    ui.window("Outer", {20.0f, 10.0f, 160.0f, 130.0f}, [&] {
        outer_first_row = ui.row(15.0f);
        ui.window("Inner rows", {36.0f, 58.0f, 100.0f, 80.0f}, [&] {
            inner_row = ui.row();
        });
        ui.window("Inner columns", {36.0f, 58.0f, 100.0f, 80.0f}, [&] {
            inner_first_column = ui.column(20.0f);
            inner_second_column = ui.column(30.0f);
        });
        outer_second_row = ui.row(10.0f);
    });
    ui.end();

    CHECK(outer_first_row.x == 24.0f);
    CHECK(outer_first_row.y == 42.0f);
    CHECK(outer_first_row.w == 152.0f);
    CHECK(outer_first_row.h == 15.0f);
    CHECK(inner_row.x == 40.0f);
    CHECK(inner_row.y == 90.0f);
    CHECK(inner_row.w == 92.0f);
    CHECK(inner_row.h == 24.0f);
    CHECK(inner_first_column.x == 40.0f);
    CHECK(inner_first_column.y == 90.0f);
    CHECK(inner_first_column.w == 20.0f);
    CHECK(inner_first_column.h == 24.0f);
    CHECK(inner_second_column.x == 64.0f);
    CHECK(inner_second_column.y == 90.0f);
    CHECK(inner_second_column.w == 30.0f);
    CHECK(inner_second_column.h == 24.0f);
    CHECK(outer_second_row.x == 24.0f);
    CHECK(outer_second_row.y == 61.0f);
    CHECK(outer_second_row.w == 152.0f);
    CHECK(outer_second_row.h == 10.0f);
}

TEST_CASE("wheel input scrolls hovered lists and clamps to their bounds") {
    const std::vector<std::string> items{"One", "Two", "Three", "Four"};
    glm::Context ui(140, 120);
    std::size_t selected = 0U;
    std::size_t offset = 1U;

    ui.begin(10, 10, false);
    ui.feed_wheel(60);
    ui.list("Items", items, selected, offset, 2U);
    CHECK(offset == 1U);
    ui.end();

    ui.begin(10, 10, false);
    ui.feed_wheel(60);
    ui.list("Items", items, selected, offset, 2U);
    CHECK(offset == 0U);
    ui.end();

    offset = 1U;
    ui.begin(10, 10, false);
    ui.feed_wheel(120);
    ui.list("Items", items, selected, offset, 2U);
    CHECK(offset == 0U);
    ui.end();

    ui.begin(10, 10, false);
    ui.feed_wheel(-120);
    ui.list("Items", items, selected, offset, 2U);
    CHECK(offset == 1U);
    ui.end();

    ui.begin(10, 10, false);
    ui.feed_wheel(std::numeric_limits<int>::min());
    ui.list("Items", items, selected, offset, 2U);
    CHECK(offset == 2U);
    ui.end();

    ui.begin(10, 10, false);
    ui.feed_wheel(std::numeric_limits<int>::max());
    ui.list("Items", items, selected, offset, 2U);
    CHECK(offset == 0U);
    ui.end();
}

TEST_CASE("wheel over non-list widgets is discarded") {
    const std::vector<std::string> items{"One", "Two", "Three"};
    glm::Context ui(140, 120);
    std::size_t selected = 0U;
    std::size_t offset = 1U;

    ui.begin(10, 10, false);
    ui.feed_wheel(-120);
    ui.button("Button");
    ui.list("Items", items, selected, offset, 2U);
    CHECK(offset == 1U);
    ui.end();

    ui.begin(10, 46, false);
    ui.button("Button");
    ui.list("Items", items, selected, offset, 2U);
    CHECK(offset == 1U);
    ui.end();
}

TEST_CASE("wheel scrolling works without taking keyboard focus from elsewhere") {
    const std::vector<std::string> items{"One", "Two", "Three", "Four"};
    glm::Context ui(140, 120);
    std::size_t selected = 0U;
    std::size_t offset = 0U;
    std::uint64_t button_id = 0U;

    ui.begin(10, 10, true);
    ui.button("Button");
    ui.list("Items", items, selected, offset, 2U);
    ui.end();
    ui.begin(10, 10, false);
    ui.button("Button");
    button_id = ui.focused_id();
    ui.list("Items", items, selected, offset, 2U);
    ui.end();

    ui.begin(10, 46, false);
    ui.feed_wheel(-120);
    ui.button("Button");
    ui.list("Items", items, selected, offset, 2U);
    CHECK(offset == 1U);
    CHECK(ui.focused_id() == button_id);
    ui.end();
}

TEST_CASE("text input clips long contents to its field bounds") {
    glm::Theme theme;
    theme.background = 0xFF010101U;
    theme.window = 0xFF000000U;
    theme.padding = 4;
    glm::Context ui(120, 100, theme);
    glm::Context baseline(120, 100, theme);
    std::string value(40U, 'W');

    ui.begin(0, 0, false);
    ui.window("Clip", {10.0f, 10.0f, 60.0f, 80.0f},
              [&] { ui.text_input("A long field label", value); });
    ui.end();
    baseline.begin(0, 0, false);
    baseline.window("Clip", {10.0f, 10.0f, 60.0f, 80.0f}, [] {});
    baseline.end();

    for (int y = 42; y < 70; ++y) {
        for (int x = 66; x < ui.width(); ++x) {
            CHECK(ui.pixels()[y * ui.width() + x] ==
                  baseline.pixels()[y * baseline.width() + x]);
        }
    }
}

TEST_CASE("text input keeps its caret visible after typing past the right edge") {
    glm::Theme theme;
    theme.background = 0xFF010101U;
    theme.window = 0xFF000000U;
    theme.padding = 4;
    glm::Context ui(120, 100, theme);
    glm::Context baseline(120, 100, theme);
    std::string value;

    const auto draw_field = [&] {
        ui.window("Caret", {10.0f, 10.0f, 60.0f, 80.0f},
                  [&] { ui.text_input("Name", value); });
    };

    ui.begin(15, 50, true);
    draw_field();
    ui.end();
    ui.begin(15, 50, false);
    draw_field();
    ui.end();
    for (int index = 0; index < 24; ++index) {
        ui.feed_char('W');
    }
    ui.begin(0, 0, false);
    draw_field();
    ui.end();
    baseline.begin(0, 0, false);
    baseline.window("Caret", {10.0f, 10.0f, 60.0f, 80.0f}, [] {});
    baseline.end();

    CHECK(ui.cursor() == value.size());
    bool caret_visible_inside_field = false;
    for (int y = 48; y < 64; ++y) {
        for (int x = 16; x < 65; ++x) {
            if (ui.pixels()[y * ui.width() + x] == theme.accent) {
                caret_visible_inside_field = true;
            }
        }
    }
    CHECK(caret_visible_inside_field);
    for (int y = 42; y < 70; ++y) {
        for (int x = 66; x < ui.width(); ++x) {
            CHECK(ui.pixels()[y * ui.width() + x] ==
                  baseline.pixels()[y * baseline.width() + x]);
        }
    }
}

TEST_CASE("window dragging persists caller-owned position and stops on release") {
    glm::Context ui(120, 100);
    glm::WindowState state{{20.0f, 20.0f, 80.0f, 70.0f}};
    const auto draw_window = [&] { ui.window("Movable", state, [] {}); };

    ui.begin(30, 25, false);
    draw_window();
    ui.end();
    ui.begin(30, 25, true);
    draw_window();
    CHECK(state.dragging);
    ui.end();
    ui.begin(45, 40, true);
    draw_window();
    CHECK(state.rect.x == 35.0f);
    CHECK(state.rect.y == 35.0f);
    ui.end();
    ui.begin(50, 45, false);
    draw_window();
    CHECK_FALSE(state.dragging);
    CHECK(state.rect.x == 40.0f);
    CHECK(state.rect.y == 40.0f);
    ui.end();
    ui.begin(90, 90, false);
    draw_window();
    CHECK(state.rect.x == 40.0f);
    CHECK(state.rect.y == 40.0f);
    ui.end();
}

TEST_CASE("window dragging clamps the title bar to the surface edges") {
    glm::Context ui(120, 100);
    glm::WindowState state{{20.0f, 20.0f, 80.0f, 70.0f}};
    const auto draw_window = [&] { ui.window("Movable", state, [] {}); };

    ui.begin(30, 25, false);
    draw_window();
    ui.end();
    ui.begin(30, 25, true);
    draw_window();
    ui.end();
    ui.begin(-20, -30, true);
    draw_window();
    CHECK(state.rect.x == 0.0f);
    CHECK(state.rect.y == 0.0f);
    ui.end();
    ui.begin(200, 200, true);
    draw_window();
    CHECK(state.rect.x == 40.0f);
    CHECK(state.rect.y == 71.0f);
    ui.end();
}

TEST_CASE("dockspace renders the selected panel in each dock group") {
    glm::Theme theme;
    theme.padding = 0;
    glm::Context ui(120, 80, theme);
    glm::DockspaceState dockspace;
    dockspace.panels = {
        {"scene", "Scene", glm::DockSide::Left},
        {"hierarchy", "Hierarchy", glm::DockSide::Left},
        {"profiler", "Profiler", glm::DockSide::Right},
    };

    ui.begin(0, 0, false);
    ui.begin_dockspace(dockspace, {10.0f, 10.0f, 100.0f, 60.0f});
    ui.dock_panel("scene", [&] { ui.gradient(0xFFFF0000U, 0xFFFF0000U, 5.0f); });
    ui.dock_panel("hierarchy", [&] {
        ui.gradient(0xFF00FF00U, 0xFF00FF00U, 5.0f);
    });
    ui.dock_panel("profiler", [&] {
        ui.gradient(0xFF0000FFU, 0xFF0000FFU, 5.0f);
    });
    ui.end_dockspace();
    ui.end();

    CHECK(ui.pixels()[37U * 120U + 13U] == 0xFFFF0000U);
    CHECK(ui.pixels()[37U * 120U + 71U] == 0xFF0000FFU);
}

TEST_CASE("dockspace tabs select and drag panels between groups") {
    glm::Theme theme;
    theme.padding = 0;
    glm::Context ui(120, 80, theme);
    glm::DockspaceState dockspace;
    dockspace.panels = {
        {"scene", "Scene", glm::DockSide::Left},
        {"hierarchy", "Hierarchy", glm::DockSide::Left},
        {"profiler", "Profiler", glm::DockSide::Right},
    };
    const auto draw = [&] {
        ui.begin_dockspace(dockspace, {10.0f, 10.0f, 100.0f, 60.0f});
        ui.dock_panel("scene", [&] {
            ui.gradient(0xFFFF0000U, 0xFFFF0000U, 5.0f);
        });
        ui.dock_panel("hierarchy", [&] {
            ui.gradient(0xFF00FF00U, 0xFF00FF00U, 5.0f);
        });
        ui.dock_panel("profiler", [&] {
            ui.gradient(0xFF0000FFU, 0xFF0000FFU, 5.0f);
        });
        ui.end_dockspace();
    };

    ui.begin(35, 20, true);
    draw();
    ui.end();
    ui.begin(80, 45, true);
    draw();
    ui.end();
    ui.begin(80, 45, false);
    draw();
    ui.end();

    CHECK(dockspace.panels[1].side == glm::DockSide::Right);
    CHECK(dockspace.active_right == "hierarchy");
    CHECK(ui.pixels()[37U * 120U + 13U] == 0xFFFF0000U);
    CHECK(ui.pixels()[37U * 120U + 71U] == 0xFF00FF00U);
}

TEST_CASE("dockspace splitter resizes its panes within safe limits") {
    glm::Context ui(120, 80);
    glm::DockspaceState dockspace;
    dockspace.panels = {
        {"scene", "Scene", glm::DockSide::Left},
        {"profiler", "Profiler", glm::DockSide::Right},
    };
    const auto draw = [&] {
        ui.begin_dockspace(dockspace, {10.0f, 10.0f, 100.0f, 60.0f});
        ui.end_dockspace();
    };
    const float original_ratio = dockspace.left_ratio;

    ui.begin(44, 45, true);
    draw();
    ui.end();
    ui.begin(110, 45, true);
    draw();
    ui.end();
    ui.begin(110, 45, false);
    draw();
    ui.end();

    CHECK(dockspace.left_ratio > original_ratio);
    CHECK(dockspace.left_ratio <= 0.85f);
}

TEST_CASE("profiler timeline maps event durations into a frame") {
    glm::Theme theme;
    theme.background = 0xFF010203U;
    theme.window = 0xFF111213U;
    theme.border = 0xFF212223U;
    theme.accent = 0xFFABCDEFU;
    theme.title_bar = 0xFFE06C75U;
    theme.padding = 0;
    glm::Context ui(120, 80, theme);
    const std::vector<glm::ProfileEvent> events{
        {"Update", 0.0f, 5.0f}, {"Render", 5.0f, 5.0f}};

    ui.begin(0, 0, false);
    ui.profiler_timeline(events, 10.0f, 16.67f, 64.0f);
    ui.end();

    const std::uint32_t* pixels = ui.pixels();
    CHECK(pixels[24U * 120U + 60U] == theme.accent);
    CHECK(pixels[45U * 120U + 100U] == theme.title_bar);
    CHECK(pixels[0] == theme.border);
}

TEST_CASE("profiler timeline clips events and ignores invalid durations") {
    glm::Theme theme;
    theme.window = 0xFF101010U;
    theme.border = 0xFF202020U;
    theme.accent = 0xFF303030U;
    theme.padding = 0;
    glm::Context ui(120, 96, theme);
    const std::vector<glm::ProfileEvent> events{
        {"Clipped", 8.0f, 8.0f},
        {"Negative", 2.0f, -1.0f},
        {"NaN", std::numeric_limits<float>::quiet_NaN(), 2.0f},
        {"Infinite", 1.0f, std::numeric_limits<float>::infinity()}};

    ui.begin(0, 0, false);
    ui.profiler_timeline(events, 10.0f, 8.0f, 80.0f);
    ui.profiler_timeline(events, 0.0f, 8.0f, 12.0f);
    ui.end();

    const std::uint32_t* pixels = ui.pixels();
    CHECK(pixels[24U * 120U + 108U] == theme.accent);
    CHECK(pixels[24U * 120U + 70U] == theme.window);
    CHECK(pixels[91U * 120U + 60U] == theme.window);
}

TEST_CASE("radio selects options by mouse and keyboard") {
    glm::Context ui(180, 120);
    const std::vector<std::string> options{"One", "Two", "Three"};
    int selected = 0;

    ui.begin(70, 10, true);
    const glm::Interaction pressed = ui.radio("Choice", options, selected);
    CHECK(pressed.active);
    ui.end();
    ui.begin(70, 10, false);
    const glm::Interaction clicked = ui.radio("Choice", options, selected);
    CHECK(clicked.clicked);
    CHECK(clicked.changed);
    CHECK(selected == 1);
    ui.end();

    ui.feed_key(glm::Key::Tab);
    ui.feed_key(glm::Key::Right);
    ui.begin(0, 0, false);
    const glm::Interaction moved = ui.radio("Choice", options, selected);
    CHECK(moved.changed);
    CHECK(selected == 2);
    ui.end();

    ui.feed_key(glm::Key::Space);
    ui.begin(0, 0, false);
    const glm::Interaction activated = ui.radio("Choice", options, selected);
    CHECK(activated.clicked);
    CHECK_FALSE(activated.changed);
    CHECK(selected == 2);
    ui.end();

    selected = 99;
    ui.begin(0, 0, false);
    ui.radio("Choice", options, selected);
    CHECK(selected == 2);
    ui.end();
}

TEST_CASE("progress bars clamp values and collapsing headers toggle") {
    glm::Context ui(160, 120);
    float below = -0.5f;
    float above = 1.5f;
    bool open = false;

    ui.begin(0, 0, false);
    const glm::Interaction low_result = ui.progress_bar("Low", below);
    const glm::Interaction high_result = ui.progress_bar("High", above);
    CHECK(below == -0.5f);
    CHECK(above == 1.5f);
    CHECK(ui.pixels()[8 * ui.width() + 8] == ui.theme().window);
    CHECK(ui.pixels()[36 * ui.width() + 151] == ui.theme().accent);
    CHECK_FALSE(low_result.active);
    CHECK_FALSE(high_result.active);
    ui.end();

    ui.begin(10, 10, true);
    const glm::Interaction progress_pressed = ui.progress_bar("Low", below);
    ui.progress_bar("High", above);
    CHECK(progress_pressed.active);
    ui.end();
    ui.begin(10, 10, false);
    const glm::Interaction progress_clicked = ui.progress_bar("Low", below);
    ui.progress_bar("High", above);
    CHECK(progress_clicked.clicked);
    CHECK_FALSE(progress_clicked.changed);
    ui.end();
    ui.feed_key(glm::Key::Tab);
    ui.feed_key(glm::Key::Tab);
    ui.feed_key(glm::Key::Space);
    ui.begin(0, 0, false);
    const glm::Interaction progress_activated = ui.progress_bar("Low", below);
    ui.progress_bar("High", above);
    CHECK(progress_activated.clicked);
    CHECK_FALSE(progress_activated.changed);
    CHECK(below == -0.5f);
    CHECK(above == 1.5f);
    ui.end();

    ui.begin(10, 10, true);
    const glm::Interaction pressed = ui.collapsing_header("Details", open);
    CHECK(pressed.active);
    CHECK_FALSE(open);
    ui.end();
    ui.begin(10, 10, false);
    const glm::Interaction clicked = ui.collapsing_header("Details", open);
    CHECK(clicked.clicked);
    CHECK(clicked.changed);
    CHECK(open);
    ui.end();

    ui.feed_key(glm::Key::Tab);
    ui.feed_key(glm::Key::Tab);
    ui.feed_key(glm::Key::Tab);
    ui.feed_key(glm::Key::Enter);
    ui.begin(0, 0, false);
    const glm::Interaction activated = ui.collapsing_header("Details", open);
    CHECK(activated.clicked);
    CHECK(activated.changed);
    CHECK_FALSE(open);
    ui.end();
}

TEST_CASE("wrapped text measures word breaks, hard breaks, and empty input") {
    glm::Surface surface(64, 80);

    const glm::Rect words = surface.measure_text_wrapped("ab cd", 24.0f);
    CHECK(words.w == 16.0f);
    CHECK(words.h == 32.0f);

    const glm::Rect long_word = surface.measure_text_wrapped("abcdef", 24.0f);
    CHECK(long_word.w == 24.0f);
    CHECK(long_word.h == 32.0f);

    const glm::Rect explicit_lines = surface.measure_text_wrapped("a\nb\nc", 64.0f);
    CHECK(explicit_lines.w == 8.0f);
    CHECK(explicit_lines.h == 48.0f);

    const glm::Rect empty = surface.measure_text_wrapped("", 24.0f);
    CHECK(empty.w == 0.0f);
    CHECK(empty.h == 16.0f);
}

TEST_CASE("wrapped labels use available width and return their layout height") {
    glm::Context ui(56, 100);
    ui.begin(0, 0, false);
    const float label_height = ui.label_wrapped("one two");
    const glm::Rect next_row = ui.row(4.0f);
    CHECK(label_height == 32.0f);
    CHECK(next_row.x == 8.0f);
    CHECK(next_row.y == 48.0f);
    CHECK(next_row.w == 40.0f);
    ui.end();
}

TEST_CASE("child regions clip drawing and nest their clip rectangles") {
    glm::Theme theme;
    theme.background = 0xFF010101U;
    theme.window = 0xFF000000U;
    glm::Context ui(100, 90, theme);
    glm::Context baseline(100, 90, theme);

    ui.begin(0, 0, false);
    ui.begin_child("Outer", {10.0f, 10.0f, 60.0f, 60.0f});
    ui.label("Outer contents");
    ui.begin_child("Inner", {40.0f, 40.0f, 60.0f, 60.0f});
    ui.label("A long inner label that crosses every edge");
    ui.end_child();
    ui.end_child();
    ui.end();

    baseline.begin(0, 0, false);
    baseline.begin_child("Outer", {10.0f, 10.0f, 60.0f, 60.0f});
    baseline.label("Outer contents");
    baseline.begin_child("Inner", {40.0f, 40.0f, 60.0f, 60.0f});
    baseline.end_child();
    baseline.end_child();
    baseline.end();

    for (int y = 0; y < ui.height(); ++y) {
        for (int x = 0; x < ui.width(); ++x) {
            const bool outside_outer = x < 10 || x >= 70 || y < 10 || y >= 70;
            if (outside_outer) {
                CHECK(ui.pixels()[y * ui.width() + x] ==
                      baseline.pixels()[y * baseline.width() + x]);
            }
        }
    }
}

TEST_CASE("child regions clamp wheel scrolling and scroll with keyboard") {
    glm::Context ui(100, 80);
    const glm::Rect child_rect{10.0f, 10.0f, 60.0f, 40.0f};
    const auto draw_child = [&] {
        ui.begin_child("Scroll", child_rect);
        ui.row(100.0f);
        ui.end_child();
    };

    ui.begin(0, 0, false);
    draw_child();
    ui.end();
    CHECK(ui.child_scroll_offset("Scroll") == 0.0f);

    ui.feed_wheel(-12000);
    ui.begin(20, 20, false);
    draw_child();
    ui.end();
    CHECK(ui.child_scroll_offset("Scroll") == 84.0f);

    ui.feed_wheel(-12000);
    ui.begin(20, 20, false);
    draw_child();
    ui.end();
    CHECK(ui.child_scroll_offset("Scroll") == 84.0f);

    ui.feed_wheel(12000);
    ui.begin(20, 20, false);
    draw_child();
    ui.end();
    CHECK(ui.child_scroll_offset("Scroll") == 0.0f);

    ui.begin(12, 12, true);
    draw_child();
    ui.end();
    ui.feed_key(glm::Key::Down);
    ui.begin(0, 0, false);
    draw_child();
    CHECK(ui.child_scroll_offset("Scroll") == 24.0f);
    CHECK(ui.active_id() == 0U);
    ui.end();
}

TEST_CASE("wheel input scrolls only the innermost hovered child") {
    glm::Context ui(120, 100);
    float inner_offset = 0.0f;
    const auto draw_children = [&] {
        ui.begin_child("Outer", {10.0f, 10.0f, 90.0f, 80.0f});
        ui.row(100.0f);
        ui.begin_child("Inner", {20.0f, 20.0f, 50.0f, 40.0f});
        ui.row(100.0f);
        ui.end_child();
        inner_offset = ui.child_scroll_offset("Inner");
        ui.end_child();
    };

    ui.begin(0, 0, false);
    draw_children();
    ui.end();
    ui.feed_wheel(-120);
    ui.begin(30, 30, false);
    draw_children();
    ui.end();

    CHECK(ui.child_scroll_offset("Outer") == 0.0f);
    CHECK(inner_offset == 24.0f);
}

TEST_CASE("combo boxes open, navigate, select, and close from the keyboard") {
    glm::Context ui(140, 120);
    const std::vector<std::string> options{"Alpha", "Beta", "Gamma"};
    std::size_t selected = 0U;

    ui.begin(0, 0, false);
    ui.combo("Mode", selected, options);
    ui.end();
    ui.feed_key(glm::Key::Tab);
    ui.feed_key(glm::Key::Enter);
    ui.begin(0, 0, false);
    const glm::Interaction opened = ui.combo("Mode", selected, options);
    CHECK(opened.clicked);
    CHECK_FALSE(opened.changed);
    ui.end();

    ui.feed_key(glm::Key::Down);
    ui.begin(0, 0, false);
    ui.combo("Mode", selected, options);
    ui.end();
    ui.feed_key(glm::Key::Enter);
    ui.begin(0, 0, false);
    const glm::Interaction selected_option = ui.combo("Mode", selected, options);
    CHECK(selected_option.clicked);
    CHECK(selected_option.changed);
    CHECK(selected == 1U);
    ui.end();

    ui.feed_key(glm::Key::Enter);
    ui.begin(0, 0, false);
    ui.combo("Mode", selected, options);
    ui.end();
    ui.feed_key(glm::Key::Escape);
    ui.begin(0, 0, false);
    const glm::Interaction escaped = ui.combo("Mode", selected, options);
    CHECK_FALSE(escaped.clicked);
    CHECK(selected == 1U);
    ui.end();
}

TEST_CASE("combo popups draw last and block clicks through to covered widgets") {
    glm::Context ui(140, 120);
    const std::vector<std::string> options{"First", "Second"};
    std::size_t selected = 0U;

    const auto draw_widgets = [&] {
        ui.combo("Mode", selected, options);
        return ui.button("Behind");
    };

    ui.begin(0, 0, false);
    draw_widgets();
    ui.end();
    ui.begin(20, 10, true);
    draw_widgets();
    ui.end();
    ui.begin(20, 10, false);
    draw_widgets();
    ui.end();
    CHECK(ui.pixels()[46 * ui.width() + 10] == ui.theme().accent);

    ui.begin(20, 60, true);
    const glm::Interaction pressed_behind = draw_widgets();
    CHECK_FALSE(pressed_behind.active);
    ui.end();
    ui.begin(20, 60, false);
    const glm::Interaction clicked_behind = draw_widgets();
    CHECK_FALSE(clicked_behind.clicked);
    CHECK(selected == 1U);
    ui.end();

    ui.begin(20, 10, true);
    draw_widgets();
    ui.end();
    ui.begin(20, 10, false);
    draw_widgets();
    ui.end();
    ui.begin(130, 110, true);
    draw_widgets();
    ui.end();
    CHECK(ui.pixels()[46 * ui.width() + 10] == ui.theme().button);
}

TEST_CASE("multiline text input inserts newlines and moves the caret by line") {
    glm::Context ui(160, 120);
    std::string value = "abc\nde";
    const auto draw_editor = [&] {
        return ui.multiline_text_input("Editor", value, 3U);
    };

    ui.begin(12, 12, true);
    draw_editor();
    ui.end();
    ui.begin(0, 0, false);
    draw_editor();
    ui.end();
    CHECK(ui.cursor() == 6U);

    ui.feed_key(glm::Key::Home);
    ui.begin(0, 0, false);
    draw_editor();
    ui.end();
    CHECK(ui.cursor() == 4U);
    ui.feed_key(glm::Key::End);
    ui.begin(0, 0, false);
    draw_editor();
    ui.end();
    CHECK(ui.cursor() == 6U);
    ui.feed_key(glm::Key::Enter);
    ui.begin(0, 0, false);
    const glm::Interaction inserted = draw_editor();
    CHECK(inserted.changed);
    CHECK(value == "abc\nde\n");
    CHECK(ui.cursor() == 7U);
    ui.end();
}

TEST_CASE("multiline vertical movement preserves its preferred column") {
    glm::Context ui(160, 140);
    std::string value = "abcdef\nxy\n12345";
    const auto draw_editor = [&] {
        ui.multiline_text_input("Editor", value, 4U);
    };

    ui.begin(12, 12, true);
    draw_editor();
    ui.end();
    ui.begin(0, 0, false);
    draw_editor();
    ui.end();

    ui.feed_key(glm::Key::Home);
    for (int step = 0; step < 4; ++step) {
        ui.feed_key(glm::Key::Right);
    }
    ui.begin(0, 0, false);
    draw_editor();
    ui.end();
    CHECK(ui.cursor() == 14U);

    ui.feed_key(glm::Key::Up);
    ui.begin(0, 0, false);
    draw_editor();
    ui.end();
    CHECK(ui.cursor() == 9U);

    ui.feed_key(glm::Key::Up);
    ui.begin(0, 0, false);
    draw_editor();
    ui.end();
    CHECK(ui.cursor() == 4U);

    ui.feed_key(glm::Key::Down);
    ui.begin(0, 0, false);
    draw_editor();
    ui.end();
    CHECK(ui.cursor() == 9U);

    ui.feed_key(glm::Key::Down);
    ui.begin(0, 0, false);
    draw_editor();
    ui.end();
    CHECK(ui.cursor() == 14U);
}

TEST_CASE("multiline editor scrolling keeps the caret in its viewport") {
    glm::Context ui(120, 100);
    std::string value = "first\nsecond\nthird\nfourth\nfifth";

    ui.begin(12, 12, true);
    ui.multiline_text_input("Editor", value, 2U);
    ui.end();
    ui.begin(0, 0, false);
    ui.multiline_text_input("Editor", value, 2U);
    ui.end();

    CHECK(ui.cursor() == value.size());
    CHECK(ui.child_scroll_offset("Editor") > 0.0f);

    for (int index = 0; index < 4; ++index) {
        ui.feed_key(glm::Key::Up);
        ui.begin(0, 0, false);
        ui.multiline_text_input("Editor", value, 2U);
        ui.end();
    }

    CHECK(ui.child_scroll_offset("Editor") == 0.0f);

    ui.begin(12, 66, true);
    ui.multiline_text_input("Editor", value, 2U);
    ui.button("Outside");
    ui.end();
    ui.begin(12, 66, false);
    ui.multiline_text_input("Editor", value, 2U);
    ui.button("Outside");
    ui.end();
    ui.feed_wheel(-12000);
    ui.begin(12, 12, false);
    ui.multiline_text_input("Editor", value, 2U);
    ui.button("Outside");
    CHECK(ui.child_scroll_offset("Editor") == 48.0f);
    ui.end();
}

TEST_CASE("filled circles and rounded rectangles have deterministic pixels") {
    glm::Surface circle(5, 5);
    circle.draw_filled_circle(2, 2, 1, 0xFFFFFFFFU);
    CHECK(circle.pixels()[2U * 5U + 2U] == 0xFFFFFFFFU);
    CHECK(circle.pixels()[1U * 5U + 2U] == 0xFFFFFFFFU);
    CHECK(circle.pixels()[3U * 5U + 2U] == 0xFFFFFFFFU);
    CHECK(circle.pixels()[2U * 5U + 1U] == 0xFFFFFFFFU);
    CHECK(circle.pixels()[2U * 5U + 3U] == 0xFFFFFFFFU);
    CHECK(circle.pixels()[1U * 5U + 1U] == 0U);
    CHECK(circle.pixels()[0] == 0U);

    glm::Surface rounded(8, 8);
    rounded.draw_rounded_rect({0.0f, 0.0f, 8.0f, 8.0f}, 3.0f, 0xFFFFFFFFU);
    CHECK(rounded.pixels()[0] == 0U);
    CHECK(rounded.pixels()[3U] == 0xFFFFFFFFU);
    CHECK(rounded.pixels()[3U * 8U + 3U] == 0xFFFFFFFFU);
    CHECK(rounded.pixels()[7U] == 0U);

    glm::Surface clamped(8, 8);
    clamped.draw_rounded_rect({0.0f, 0.0f, 8.0f, 8.0f}, 100.0f,
                              0xFFFFFFFFU);
    glm::Surface maximum_radius(8, 8);
    maximum_radius.draw_rounded_rect({0.0f, 0.0f, 8.0f, 8.0f}, 4.0f,
                                     0xFFFFFFFFU);
    for (std::size_t index = 0; index < 64U; ++index) {
        CHECK(clamped.pixels()[index] == maximum_radius.pixels()[index]);
    }
}

TEST_CASE("filled ellipses use pixel-center sampling and respect clipping") {
    glm::Surface surface(5, 3);
    surface.draw_filled_ellipse({0.0f, 0.0f, 5.0f, 3.0f}, 0xFFFFFFFFU);
    const std::array<std::array<bool, 5>, 3> expected{{
        {{false, true, true, true, false}},
        {{true, true, true, true, true}},
        {{false, true, true, true, false}},
    }};
    for (std::size_t y = 0; y < expected.size(); ++y) {
        for (std::size_t x = 0; x < expected[y].size(); ++x) {
            CHECK((surface.pixels()[y * 5U + x] != 0U) == expected[y][x]);
        }
    }

    glm::Surface clipped(5, 3);
    clipped.push_clip({2.0f, 0.0f, 2.0f, 3.0f});
    clipped.draw_filled_ellipse({0.0f, 0.0f, 5.0f, 3.0f}, 0xFFFFFFFFU);
    clipped.pop_clip();
    CHECK(clipped.pixels()[1U] == 0U);
    CHECK(clipped.pixels()[2U] == 0xFFFFFFFFU);
    CHECK(clipped.pixels()[5U + 1U] == 0U);
    CHECK(clipped.pixels()[5U + 2U] == 0xFFFFFFFFU);
}

TEST_CASE("filled ellipses reject non-positive extents and blend alpha") {
    glm::Surface surface(3, 3);
    surface.fill_rect({0.0f, 0.0f, 3.0f, 3.0f}, 0xFF202020U);
    surface.draw_filled_ellipse({0.0f, 0.0f, 3.0f, 3.0f}, 0x800000FFU);
    CHECK(surface.pixels()[4] == 0xFF101090U);

    glm::Surface invalid(3, 3);
    invalid.draw_filled_ellipse({1.0f, 0.0f, 0.0f, 3.0f}, 0xFFFFFFFFU);
    invalid.draw_filled_ellipse({0.0f, 1.0f, 3.0f, -1.0f}, 0xFFFFFFFFU);
    invalid.draw_filled_ellipse({1.0e30f, 0.0f, 1.0f, 1.0f},
                                0xFFFFFFFFU);
    CHECK(invalid.pixels()[4] == 0U);
}

TEST_CASE("filled triangles rasterize deterministically regardless of winding") {
    glm::Surface first(4, 4);
    glm::Surface reversed(4, 4);
    first.draw_filled_triangle(0.0f, 0.0f, 4.0f, 0.0f, 0.0f, 4.0f,
                               0xFFFFFFFFU);
    reversed.draw_filled_triangle(0.0f, 4.0f, 4.0f, 0.0f, 0.0f, 0.0f,
                                  0xFFFFFFFFU);
    const std::array<std::array<bool, 4>, 4> expected{{
        {{true, true, true, true}},
        {{true, true, true, false}},
        {{true, true, false, false}},
        {{true, false, false, false}},
    }};
    for (std::size_t y = 0; y < expected.size(); ++y) {
        for (std::size_t x = 0; x < expected[y].size(); ++x) {
            const std::size_t index = y * 4U + x;
            CHECK((first.pixels()[index] != 0U) == expected[y][x]);
            CHECK(reversed.pixels()[index] == first.pixels()[index]);
        }
    }
}

TEST_CASE("filled triangles clip, blend alpha, and reject degenerate input") {
    glm::Surface surface(4, 4);
    surface.fill_rect({0.0f, 0.0f, 4.0f, 4.0f}, 0xFF202020U);
    surface.push_clip({1.0f, 1.0f, 2.0f, 2.0f});
    surface.draw_filled_triangle(0.0f, 0.0f, 4.0f, 0.0f, 0.0f, 4.0f,
                                0x800000FFU);
    surface.pop_clip();
    CHECK(surface.pixels()[0] == 0xFF202020U);
    CHECK(surface.pixels()[1U * 4U + 1U] == 0xFF101090U);
    CHECK(surface.pixels()[3U * 4U + 3U] == 0xFF202020U);

    glm::Surface invalid(3, 3);
    invalid.draw_filled_triangle(0.0f, 0.0f, 1.0f, 1.0f, 2.0f, 2.0f,
                                 0xFFFFFFFFU);
    invalid.draw_filled_triangle(
        std::numeric_limits<float>::quiet_NaN(), 0.0f, 2.0f, 0.0f,
        0.0f, 2.0f, 0xFFFFFFFFU);
    CHECK(invalid.pixels()[4] == 0U);
}

TEST_CASE("filled polygons support concavity and either winding direction") {
    const std::array<float, 12> points{{
        0.0f, 0.0f, 4.0f, 0.0f, 4.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 4.0f, 0.0f, 4.0f,
    }};
    const std::array<float, 12> reversed{{
        0.0f, 4.0f, 1.0f, 4.0f, 1.0f, 1.0f,
        4.0f, 1.0f, 4.0f, 0.0f, 0.0f, 0.0f,
    }};
    glm::Surface first(4, 4);
    glm::Surface second(4, 4);
    first.draw_filled_polygon(points.data(), 6U, 0xFFFFFFFFU);
    second.draw_filled_polygon(reversed.data(), 6U, 0xFFFFFFFFU);
    const std::array<std::array<bool, 4>, 4> expected{{
        {{true, true, true, true}},
        {{true, false, false, false}},
        {{true, false, false, false}},
        {{true, false, false, false}},
    }};
    for (std::size_t y = 0; y < expected.size(); ++y) {
        for (std::size_t x = 0; x < expected[y].size(); ++x) {
            const std::size_t index = y * 4U + x;
            CHECK((first.pixels()[index] != 0U) == expected[y][x]);
            CHECK(first.pixels()[index] == second.pixels()[index]);
        }
    }
}

TEST_CASE("filled polygons use even-odd coverage for self-intersections") {
    const std::array<float, 8> bow_tie{{
        0.0f, 0.0f, 4.0f, 4.0f, 0.0f, 4.0f, 4.0f, 0.0f,
    }};
    glm::Surface surface(4, 4);
    surface.draw_filled_polygon(bow_tie.data(), 4U, 0xFFFFFFFFU);
    CHECK(surface.pixels()[1U] == 0xFFFFFFFFU);
    CHECK(surface.pixels()[4U] == 0U);
    CHECK(surface.pixels()[3U * 4U + 1U] == 0xFFFFFFFFU);
    CHECK(surface.pixels()[2U * 4U] == 0U);
}

TEST_CASE("polylines draw open paths without adding a closing edge") {
    const std::array<float, 6> points{{
        1.5f, 1.5f, 5.5f, 1.5f, 5.5f, 5.5f,
    }};
    glm::Surface open(7, 7);
    glm::Surface closed(7, 7);
    open.draw_polyline(points.data(), 3U, 0xFFFFFFFFU, 1.0f);
    closed.draw_polygon_outline(points.data(), 3U, 0xFFFFFFFFU, 1.0f);
    CHECK(open.pixels()[1U * 7U + 3U] == 0xFFFFFFFFU);
    CHECK(open.pixels()[3U * 7U + 5U] == 0xFFFFFFFFU);
    CHECK(open.pixels()[3U * 7U + 3U] == 0U);
    CHECK(closed.pixels()[3U * 7U + 3U] == 0xFFFFFFFFU);
}

TEST_CASE("polylines clip, blend joins once, and handle invalid input") {
    const std::array<float, 6> points{{
        1.5f, 1.5f, 5.5f, 1.5f, 5.5f, 5.5f,
    }};
    glm::Surface surface(7, 7);
    surface.fill_rect({0.0f, 0.0f, 7.0f, 7.0f}, 0xFF202020U);
    surface.push_clip({2.0f, 1.0f, 3.0f, 5.0f});
    surface.draw_polyline(points.data(), 3U, 0x800000FFU, 2.0f);
    surface.pop_clip();
    CHECK(surface.pixels()[1U * 7U + 1U] == 0xFF202020U);
    CHECK(surface.pixels()[1U * 7U + 4U] == 0xFF101090U);

    glm::Surface invalid(3, 3);
    invalid.draw_polyline(nullptr, 2U, 0xFFFFFFFFU);
    invalid.draw_polyline(points.data(), 1U, 0xFFFFFFFFU);
    invalid.draw_polyline(points.data(), 3U, 0xFFFFFFFFU, 0.0f);
    CHECK(invalid.pixels()[0] == 0U);
}

TEST_CASE("cubic Bezier curves include endpoints and honor subdivision count") {
    glm::Surface curve(9, 7);
    curve.draw_cubic_bezier(1.5f, 5.5f, 1.5f, 1.5f, 7.5f, 1.5f, 7.5f,
                            5.5f, 0xFFFFFFFFU, 1.0f, 16U);
    CHECK(curve.pixels()[5U * 9U + 1U] == 0xFFFFFFFFU);
    CHECK(curve.pixels()[5U * 9U + 7U] == 0xFFFFFFFFU);
    CHECK(curve.pixels()[2U * 9U + 4U] == 0xFFFFFFFFU);

    glm::Surface straight(5, 1);
    straight.draw_cubic_bezier(0.5f, 0.5f, 1.5f, 0.5f, 3.5f, 0.5f,
                               4.5f, 0.5f, 0xFFFFFFFFU, 1.0f, 1U);
    for (std::size_t x = 0; x < 5U; ++x) {
        CHECK(straight.pixels()[x] == 0xFFFFFFFFU);
    }
}

TEST_CASE("cubic Bezier curves clip, blend, and reject invalid parameters") {
    glm::Surface surface(8, 8);
    surface.fill_rect({0.0f, 0.0f, 8.0f, 8.0f}, 0xFF202020U);
    surface.push_clip({0.0f, 0.0f, 4.0f, 8.0f});
    surface.draw_cubic_bezier(1.5f, 5.5f, 2.0f, 1.5f, 6.0f, 1.5f,
                              6.5f, 5.5f, 0x800000FFU, 1.0f, 12U);
    surface.pop_clip();
    CHECK(surface.pixels()[5U * 8U + 1U] == 0xFF101090U);
    CHECK(surface.pixels()[5U * 8U + 6U] == 0xFF202020U);

    glm::Surface invalid(3, 3);
    invalid.draw_cubic_bezier(0.5f, 0.5f, 1.0f, 0.0f, 2.0f, 0.0f,
                              2.5f, 0.5f, 0xFFFFFFFFU, 1.0f, 0U);
    invalid.draw_cubic_bezier(
        0.5f, 0.5f, 1.0f, 0.0f,
        std::numeric_limits<float>::infinity(), 0.0f, 2.5f, 0.5f,
        0xFFFFFFFFU);
    CHECK(invalid.pixels()[0] == 0U);
}

TEST_CASE("filled polygons clip, blend, and reject invalid input") {
    const std::array<float, 8> points{{
        0.0f, 0.0f, 4.0f, 0.0f, 4.0f, 4.0f, 0.0f, 4.0f,
    }};
    glm::Surface surface(4, 4);
    surface.fill_rect({0.0f, 0.0f, 4.0f, 4.0f}, 0xFF202020U);
    surface.push_clip({1.0f, 1.0f, 2.0f, 2.0f});
    surface.draw_filled_polygon(points.data(), 4U, 0x800000FFU);
    surface.pop_clip();
    CHECK(surface.pixels()[0] == 0xFF202020U);
    CHECK(surface.pixels()[1U * 4U + 1U] == 0xFF101090U);
    CHECK(surface.pixels()[3U * 4U + 3U] == 0xFF202020U);

    glm::Surface invalid(3, 3);
    invalid.draw_filled_polygon(nullptr, 4U, 0xFFFFFFFFU);
    invalid.draw_filled_polygon(points.data(), 2U, 0xFFFFFFFFU);
    const std::array<float, 6> non_finite{{
        0.0f, 0.0f, std::numeric_limits<float>::quiet_NaN(), 1.0f,
        1.0f, 0.0f,
    }};
    invalid.draw_filled_polygon(non_finite.data(), 3U, 0xFFFFFFFFU);
    CHECK(invalid.pixels()[4] == 0U);
}

TEST_CASE("polygon outlines close the path and blend covered pixels once") {
    const std::array<float, 8> square{{
        1.5f, 1.5f, 5.5f, 1.5f, 5.5f, 5.5f, 1.5f, 5.5f,
    }};
    glm::Surface surface(7, 7);
    surface.fill_rect({0.0f, 0.0f, 7.0f, 7.0f}, 0xFF202020U);
    surface.draw_polygon_outline(square.data(), 4U, 0x800000FFU);
    CHECK(surface.pixels()[1U * 7U + 1U] == 0xFF101090U);
    CHECK(surface.pixels()[1U * 7U + 3U] == 0xFF101090U);
    CHECK(surface.pixels()[3U * 7U + 1U] == 0xFF101090U);
    CHECK(surface.pixels()[3U * 7U + 3U] == 0xFF202020U);
    CHECK(surface.pixels()[0] == 0xFF202020U);
}

TEST_CASE("polygon outlines clip, support thickness, and reject invalid input") {
    const std::array<float, 8> square{{
        0.5f, 0.5f, 4.5f, 0.5f, 4.5f, 4.5f, 0.5f, 4.5f,
    }};
    glm::Surface clipped(5, 5);
    clipped.push_clip({2.0f, 0.0f, 2.0f, 5.0f});
    clipped.draw_polygon_outline(square.data(), 4U, 0xFFFFFFFFU, 2.0f);
    clipped.pop_clip();
    CHECK(clipped.pixels()[1U * 5U + 1U] == 0U);
    CHECK(clipped.pixels()[1U * 5U + 2U] == 0xFFFFFFFFU);

    glm::Surface degenerate(5, 3);
    const std::array<float, 6> line{{
        0.5f, 1.5f, 4.5f, 1.5f, 2.5f, 1.5f,
    }};
    degenerate.draw_polygon_outline(line.data(), 3U, 0xFFFFFFFFU);
    CHECK(degenerate.pixels()[1U * 5U + 2U] == 0xFFFFFFFFU);

    glm::Surface invalid(3, 3);
    invalid.draw_polygon_outline(nullptr, 4U, 0xFFFFFFFFU);
    invalid.draw_polygon_outline(square.data(), 1U, 0xFFFFFFFFU);
    invalid.draw_polygon_outline(square.data(), 4U, 0xFFFFFFFFU, 0.0f);
    const std::array<float, 6> non_finite{{
        0.5f, 0.5f, 2.5f, 0.5f,
        std::numeric_limits<float>::infinity(), 2.5f,
    }};
    invalid.draw_polygon_outline(non_finite.data(), 3U, 0xFFFFFFFFU);
    CHECK(invalid.pixels()[0] == 0U);
}

TEST_CASE("triangle outlines draw each edge once with selectable thickness") {
    glm::Surface outline(7, 7);
    outline.draw_triangle_outline(1.5f, 1.5f, 5.5f, 1.5f, 1.5f, 5.5f,
                                  0xFFFFFFFFU);
    CHECK(outline.pixels()[1U * 7U + 3U] == 0xFFFFFFFFU);
    CHECK(outline.pixels()[3U * 7U + 1U] == 0xFFFFFFFFU);
    CHECK(outline.pixels()[3U * 7U + 4U] == 0U);
    CHECK(outline.pixels()[0] == 0U);

    glm::Surface thick(7, 7);
    thick.draw_triangle_outline(1.5f, 1.5f, 5.5f, 1.5f, 1.5f, 5.5f,
                                0xFFFFFFFFU, 2.0f);
    CHECK(thick.pixels()[3U * 7U + 3U] == 0xFFFFFFFFU);
}

TEST_CASE("triangle outlines clip, blend, and handle invalid and degenerate input") {
    glm::Surface surface(7, 7);
    surface.fill_rect({0.0f, 0.0f, 7.0f, 7.0f}, 0xFF202020U);
    surface.push_clip({2.0f, 1.0f, 3.0f, 1.0f});
    surface.draw_triangle_outline(1.5f, 1.5f, 5.5f, 1.5f, 1.5f, 5.5f,
                                  0x800000FFU);
    surface.pop_clip();
    CHECK(surface.pixels()[1U * 7U + 1U] == 0xFF202020U);
    CHECK(surface.pixels()[1U * 7U + 3U] == 0xFF101090U);

    glm::Surface degenerate(5, 3);
    degenerate.draw_triangle_outline(0.5f, 1.5f, 4.5f, 1.5f, 2.5f, 1.5f,
                                     0xFFFFFFFFU);
    CHECK(degenerate.pixels()[1U * 5U + 2U] == 0xFFFFFFFFU);

    glm::Surface invalid(3, 3);
    invalid.draw_triangle_outline(0.5f, 0.5f, 2.5f, 0.5f, 0.5f, 2.5f,
                                  0xFFFFFFFFU, 0.0f);
    invalid.draw_triangle_outline(
        std::numeric_limits<float>::quiet_NaN(), 0.5f, 2.5f, 0.5f,
        0.5f, 2.5f, 0xFFFFFFFFU);
    CHECK(invalid.pixels()[0] == 0U);
}

TEST_CASE("ellipse outlines draw a deterministic ring with requested thickness") {
    glm::Surface surface(9, 5);
    surface.draw_ellipse_outline({0.0f, 0.0f, 9.0f, 5.0f}, 0xFFFFFFFFU);
    CHECK(surface.pixels()[2U * 9U + 4U] == 0U);
    CHECK(surface.pixels()[0U * 9U + 4U] == 0xFFFFFFFFU);
    CHECK(surface.pixels()[2U * 9U] == 0xFFFFFFFFU);
    CHECK(surface.pixels()[1U * 9U + 2U] == 0U);

    glm::Surface thick(9, 5);
    thick.draw_ellipse_outline({0.0f, 0.0f, 9.0f, 5.0f}, 0xFFFFFFFFU, 3.0f);
    CHECK(thick.pixels()[2U * 9U + 4U] == 0xFFFFFFFFU);
    CHECK(thick.pixels()[0U * 9U + 4U] == 0xFFFFFFFFU);
}

TEST_CASE("ellipse outlines clip, blend alpha, and reject invalid thickness") {
    glm::Surface surface(7, 5);
    surface.fill_rect({0.0f, 0.0f, 7.0f, 5.0f}, 0xFF202020U);
    surface.push_clip({2.0f, 0.0f, 3.0f, 5.0f});
    surface.draw_ellipse_outline({0.0f, 0.0f, 7.0f, 5.0f},
                                 0x800000FFU, 1.0f);
    surface.pop_clip();
    CHECK(surface.pixels()[2U * 7U] == 0xFF202020U);
    CHECK(surface.pixels()[3U] == 0xFF101090U);

    glm::Surface invalid(3, 3);
    invalid.draw_ellipse_outline({0.0f, 0.0f, 3.0f, 3.0f},
                                 0xFFFFFFFFU, 0.0f);
    invalid.draw_ellipse_outline({0.0f, 0.0f, 3.0f, 3.0f},
                                 0xFFFFFFFFU,
                                 std::numeric_limits<float>::infinity());
    CHECK(invalid.pixels()[4] == 0U);
}

TEST_CASE("rounded rectangle outlines preserve rounded corners and thickness") {
    glm::Surface surface(9, 9);
    surface.draw_rounded_rect_outline({0.0f, 0.0f, 9.0f, 9.0f}, 3.0f,
                                     0xFFFFFFFFU);
    CHECK(surface.pixels()[0] == 0U);
    CHECK(surface.pixels()[4U] == 0xFFFFFFFFU);
    CHECK(surface.pixels()[3U * 9U] == 0xFFFFFFFFU);
    CHECK(surface.pixels()[4U * 9U + 4U] == 0U);

    glm::Surface thick(9, 9);
    thick.draw_rounded_rect_outline({0.0f, 0.0f, 9.0f, 9.0f}, 3.0f,
                                    0xFFFFFFFFU, 2.0f);
    CHECK(thick.pixels()[4U * 9U + 4U] == 0U);
    CHECK(thick.pixels()[1U * 9U + 4U] == 0xFFFFFFFFU);

    glm::Surface square_corner(7, 7);
    glm::Surface rectangle_outline(7, 7);
    square_corner.draw_rounded_rect_outline(
        {1.0f, 1.0f, 5.0f, 5.0f}, 0.0f, 0xFFFFFFFFU, 2.0f);
    rectangle_outline.outline_rect({1.0f, 1.0f, 5.0f, 5.0f},
                                   0xFFFFFFFFU, 2.0f);
    for (std::size_t index = 0; index < 49U; ++index) {
        CHECK(square_corner.pixels()[index] == rectangle_outline.pixels()[index]);
    }
}

TEST_CASE("rounded rectangle outlines clip, blend, and reject invalid inputs") {
    glm::Surface surface(9, 9);
    surface.fill_rect({0.0f, 0.0f, 9.0f, 9.0f}, 0xFF202020U);
    surface.push_clip({3.0f, 0.0f, 3.0f, 9.0f});
    surface.draw_rounded_rect_outline({0.0f, 0.0f, 9.0f, 9.0f}, 2.0f,
                                      0x800000FFU, 1.0f);
    surface.pop_clip();
    CHECK(surface.pixels()[0U] == 0xFF202020U);
    CHECK(surface.pixels()[3U] == 0xFF101090U);
    CHECK(surface.pixels()[4U * 9U + 4U] == 0xFF202020U);

    glm::Surface invalid(3, 3);
    invalid.draw_rounded_rect_outline({0.0f, 0.0f, 3.0f, 3.0f}, 1.0f,
                                     0xFFFFFFFFU, 0.0f);
    invalid.draw_rounded_rect_outline(
        {0.0f, 0.0f, 3.0f, 3.0f},
        std::numeric_limits<float>::quiet_NaN(), 0xFFFFFFFFU);
    invalid.draw_rounded_rect_outline(
        {0.0f, 0.0f, 3.0f, 3.0f}, 1.0f, 0xFFFFFFFFU,
        std::numeric_limits<float>::infinity());
    CHECK(invalid.pixels()[4] == 0U);
}

TEST_CASE("outlined rectangles draw a one-pixel border") {
    glm::Surface surface(6, 6);
    surface.outline_rect({1.0f, 1.0f, 4.0f, 4.0f}, 0xFFFFFFFFU);
    CHECK(surface.pixels()[1U * 6U + 1U] == 0xFFFFFFFFU);
    CHECK(surface.pixels()[1U * 6U + 2U] == 0xFFFFFFFFU);
    CHECK(surface.pixels()[2U * 6U + 1U] == 0xFFFFFFFFU);
    CHECK(surface.pixels()[2U * 6U + 2U] == 0U);
    CHECK(surface.pixels()[4U * 6U + 4U] == 0xFFFFFFFFU);
    CHECK(surface.pixels()[5U * 6U + 5U] == 0U);
}

TEST_CASE("dashed lines repeat their dash and gap pattern deterministically") {
    glm::Surface surface(9, 1);
    surface.draw_dashed_line(0, 0, 8, 0, 0xFFFFFFFFU, 2, 1);
    const std::array<bool, 9> expected{{
        true, true, false, true, true, false, true, true, false,
    }};
    for (std::size_t x = 0; x < expected.size(); ++x) {
        CHECK((surface.pixels()[x] != 0U) == expected[x]);
    }

    glm::Surface diagonal(5, 5);
    diagonal.draw_dashed_line(0, 0, 4, 4, 0xFFFFFFFFU, 1, 1);
    for (int coordinate = 0; coordinate < 5; ++coordinate) {
        const bool expected_pixel = coordinate % 2 == 0;
        CHECK((diagonal.pixels()[static_cast<std::size_t>(
                   coordinate * 5 + coordinate)] != 0U) == expected_pixel);
    }
}

TEST_CASE("dashed lines clip, blend, and handle dash parameters") {
    glm::Surface clipped(9, 1);
    clipped.push_clip({2.0f, 0.0f, 5.0f, 1.0f});
    clipped.draw_dashed_line(0, 0, 8, 0, 0xFFFFFFFFU, 2, 1);
    clipped.pop_clip();
    CHECK(clipped.pixels()[0] == 0U);
    CHECK(clipped.pixels()[2] == 0U);
    CHECK(clipped.pixels()[3] == 0xFFFFFFFFU);
    CHECK(clipped.pixels()[4] == 0xFFFFFFFFU);
    CHECK(clipped.pixels()[8] == 0U);

    glm::Surface invalid(3, 1);
    invalid.draw_dashed_line(0, 0, 2, 0, 0xFFFFFFFFU, 0, 1);
    CHECK(invalid.pixels()[0] == 0U);
    invalid.draw_dashed_line(0, 0, 2, 0, 0xFFFFFFFFU, 1, -3);
    CHECK(invalid.pixels()[0] == 0xFFFFFFFFU);
    CHECK(invalid.pixels()[1] == 0xFFFFFFFFU);
    CHECK(invalid.pixels()[2] == 0xFFFFFFFFU);
}

TEST_CASE("outlined circles draw a deterministic ring with requested thickness") {
    glm::Surface surface(5, 5);
    surface.draw_circle_outline(2, 2, 2, 0xFFFFFFFFU, 1);
    for (int y = 0; y < 5; ++y) {
        for (int x = 0; x < 5; ++x) {
            const int dx = x - 2;
            const int dy = y - 2;
            const int distance_squared = dx * dx + dy * dy;
            const bool expected = distance_squared <= 4 &&
                                  distance_squared > 1;
            CHECK((surface.pixels()[static_cast<std::size_t>(y * 5 + x)] != 0U) ==
                  expected);
        }
    }

    glm::Surface thick(5, 5);
    thick.draw_circle_outline(2, 2, 2, 0xFFFFFFFFU, 2);
    CHECK(thick.pixels()[2U * 5U + 2U] == 0xFFFFFFFFU);
    CHECK(thick.pixels()[0] == 0U);
}

TEST_CASE("outlined circles respect clipping and ignore invalid dimensions") {
    glm::Surface clipped(5, 5);
    clipped.push_clip({2.0f, 0.0f, 3.0f, 5.0f});
    clipped.draw_circle_outline(2, 2, 2, 0xFFFFFFFFU, 1);
    clipped.pop_clip();
    CHECK(clipped.pixels()[2U * 5U] == 0U);
    CHECK(clipped.pixels()[2U * 5U + 2U] == 0U);
    CHECK(clipped.pixels()[2U * 5U + 4U] == 0xFFFFFFFFU);

    glm::Surface empty(3, 3);
    empty.draw_circle_outline(1, 1, 1, 0xFFFFFFFFU, 0);
    empty.draw_circle_outline(1, 1, -1, 0xFFFFFFFFU, 1);
    empty.draw_circle_outline(1, 1, 0, 0xFFFFFFFFU, 1);
    CHECK(empty.pixels()[4] == 0xFFFFFFFFU);
    CHECK(empty.pixels()[0] == 0U);
}

TEST_CASE("image widget scales nearest-neighbor and respects parent clipping") {
    const std::array<std::uint32_t, 4> source{{
        0xFF0000FFU, 0xFF00FF00U, 0xFFFF0000U, 0xFFFFFFFFU,
    }};
    glm::Context scaled(4, 4);
    scaled.begin(0, 0, false);
    scaled.image(source.data(), 2, 2, {0.0f, 0.0f, 4.0f, 4.0f});
    scaled.end();
    const std::uint32_t* scaled_pixels = scaled.pixels();
    CHECK(scaled_pixels[0] == source[0]);
    CHECK(scaled_pixels[1] == source[0]);
    CHECK(scaled_pixels[2] == source[1]);
    CHECK(scaled_pixels[3] == source[1]);
    CHECK(scaled_pixels[2U * 4U] == source[2]);
    CHECK(scaled_pixels[2U * 4U + 2U] == source[3]);
    CHECK(scaled_pixels[3U * 4U + 3U] == source[3]);

    glm::Surface source_surface(2, 2);
    for (std::size_t index = 0; index < source.size(); ++index) {
        source_surface.pixels()[index] = source[index];
    }
    glm::Context surface_image(4, 4);
    surface_image.begin(0, 0, false);
    surface_image.image(source_surface, {0.0f, 0.0f, 4.0f, 4.0f});
    surface_image.end();
    CHECK(surface_image.pixels()[0] == source[0]);
    CHECK(surface_image.pixels()[3U] == source[1]);
    CHECK(surface_image.pixels()[3U * 4U] == source[2]);
    CHECK(surface_image.pixels()[3U * 4U + 3U] == source[3]);

    glm::Theme theme;
    theme.background = 0xFF123456U;
    glm::Context clipped(4, 4, theme);
    clipped.begin(0, 0, false);
    clipped.begin_child("ImageClip", {1.0f, 1.0f, 2.0f, 2.0f});
    clipped.image(source.data(), 2, 2, {0.0f, 0.0f, 4.0f, 4.0f});
    clipped.end_child();
    clipped.end();
    const std::uint32_t* clipped_pixels = clipped.pixels();
    CHECK(clipped_pixels[0] == theme.background);
    CHECK(clipped_pixels[1U] == theme.background);
    CHECK(clipped_pixels[1U * 4U] == theme.background);
    CHECK(clipped_pixels[1U * 4U + 1U] == source[0]);
    CHECK(clipped_pixels[1U * 4U + 2U] == source[1]);
    CHECK(clipped_pixels[2U * 4U + 1U] == source[2]);
    CHECK(clipped_pixels[2U * 4U + 2U] == source[3]);
    CHECK(clipped_pixels[3U * 4U + 3U] == theme.background);
}

TEST_CASE("tinted images modulate packed RGBA channels and preserve source pixels") {
    const std::uint32_t source = 0xFF336699U;
    glm::Surface surface(1, 1);
    surface.draw_tinted_image(&source, 1, 1, {0.0f, 0.0f, 1.0f, 1.0f},
                              0x80FF8040U);
    CHECK(surface.pixels()[0] == 0x80333326U);
    CHECK(source == 0xFF336699U);

    glm::Surface clipped(3, 1);
    clipped.push_clip({1.0f, 0.0f, 1.0f, 1.0f});
    clipped.draw_tinted_image(&source, 1, 1, {0.0f, 0.0f, 3.0f, 1.0f},
                              0xFFFFFFFFU);
    clipped.pop_clip();
    CHECK(clipped.pixels()[0] == 0U);
    CHECK(clipped.pixels()[1] == source);
    CHECK(clipped.pixels()[2] == 0U);
}

TEST_CASE("context tinted image applies its tint in surface coordinates") {
    const std::array<std::uint32_t, 2> source{{0xFFFFFFFFU, 0xFF000000U}};
    glm::Context ui(2, 1);
    ui.begin(0, 0, false);
    ui.tinted_image(source.data(), 2, 1, {0.0f, 0.0f, 2.0f, 1.0f},
                    0xFF0000FFU);
    ui.end();
    CHECK(ui.pixels()[0] == 0xFF0000FFU);
    CHECK(ui.pixels()[1] == 0xFF000000U);
}

TEST_CASE("tooltip appears after the configured hover frame delay and hides") {
    glm::Theme theme;
    theme.tooltip_delay_frames = 2U;
    theme.border = 0xFF123456U;
    glm::Context ui(120, 80, theme);

    ui.begin(10, 10, true);
    ui.button("Hover");
    ui.tooltip("Help");
    ui.end();
    CHECK(ui.pixels()[22U * 120U + 22U] != theme.border);

    ui.begin(10, 10, false);
    ui.button("Hover");
    ui.tooltip("Help");
    ui.end();
    CHECK(ui.pixels()[22U * 120U + 22U] == theme.border);

    ui.begin(110, 70, false);
    ui.button("Hover");
    ui.tooltip("Help");
    ui.end();
    CHECK(ui.pixels()[22U * 120U + 22U] != theme.border);
}

TEST_CASE("tooltip delay is configurable for immediate display") {
    glm::Theme theme;
    theme.tooltip_delay_frames = 1U;
    theme.border = 0xFF123456U;
    glm::Context ui(120, 80, theme);
    ui.begin(10, 10, false);
    ui.button("Hover");
    ui.tooltip("Help");
    ui.end();
    CHECK(ui.pixels()[22U * 120U + 22U] == theme.border);
}

TEST_CASE("toggle switch changes by mouse and keyboard activation") {
    glm::Context ui(120, 80);
    bool enabled = false;

    ui.begin(20, 20, true);
    const glm::Interaction pressed = ui.toggle("Enabled", enabled);
    CHECK(pressed.active);
    CHECK_FALSE(enabled);
    ui.end();

    ui.begin(20, 20, false);
    const glm::Interaction clicked = ui.toggle("Enabled", enabled);
    CHECK(clicked.clicked);
    CHECK(clicked.changed);
    CHECK(enabled);
    ui.end();

    ui.feed_key(glm::Key::Enter);
    ui.begin(0, 0, false);
    const glm::Interaction activated = ui.toggle("Enabled", enabled);
    CHECK(activated.clicked);
    CHECK(activated.changed);
    CHECK_FALSE(enabled);
    ui.end();
}

TEST_CASE("separator draws a themed line and does not take keyboard focus") {
    glm::Theme theme;
    theme.border = 0xFF123456U;
    glm::Context ui(120, 80, theme);
    ui.begin(0, 0, false);
    ui.button("Before");
    ui.separator();
    ui.button("After");
    ui.end();

    CHECK(ui.pixels()[44U * 120U + 8U] == theme.border);
    CHECK(ui.pixels()[46U * 120U + 8U] == theme.background);
    CHECK(ui.focused_id() == 0U);
}

TEST_CASE("radio-group tooltip follows hover across its full option row") {
    glm::Theme theme;
    theme.tooltip_delay_frames = 1U;
    theme.border = 0xFF123456U;
    glm::Context ui(120, 80, theme);
    int selected = 0;
    const std::vector<std::string> options{"One", "Two", "Three"};

    ui.begin(10, 12, false);
    ui.radio("Choice", options, selected);
    ui.tooltip("Select a choice");
    ui.end();

    CHECK(ui.pixels()[24U * 120U + 22U] == theme.border);
}

TEST_CASE("selectable items toggle caller state by mouse and keyboard") {
    glm::Context ui(160, 100);
    bool selected = false;

    ui.begin(20, 20, true);
    const glm::Interaction pressed = ui.selectable("Favorite", selected);
    CHECK(pressed.active);
    CHECK_FALSE(selected);
    ui.end();

    ui.begin(20, 20, false);
    const glm::Interaction clicked = ui.selectable("Favorite", selected);
    CHECK(clicked.clicked);
    CHECK(clicked.changed);
    CHECK(selected);
    ui.end();

    ui.feed_key(glm::Key::Space);
    ui.begin(0, 0, false);
    const glm::Interaction activated = ui.selectable("Favorite", selected);
    CHECK(activated.clicked);
    CHECK(activated.changed);
    CHECK_FALSE(selected);
    ui.end();
}

TEST_CASE("selectable selection is caller-owned across frames") {
    glm::Context ui(160, 100);
    bool selected = true;
    ui.begin(0, 0, false);
    const glm::Interaction rendered = ui.selectable("Pinned", selected);
    ui.end();

    CHECK_FALSE(rendered.changed);
    CHECK(selected);
}

TEST_CASE("tab bar selects tabs by mouse and keyboard navigation") {
    glm::Context ui(160, 100);
    const std::vector<std::string> tabs{"General", "Audio", "About"};
    std::size_t selected = 0U;

    ui.begin(60, 16, true);
    const glm::Interaction pressed = ui.tab_bar("Settings tabs", tabs, selected);
    CHECK(pressed.active);
    CHECK(selected == 0U);
    ui.end();

    ui.begin(60, 16, false);
    const glm::Interaction clicked = ui.tab_bar("Settings tabs", tabs, selected);
    CHECK(clicked.clicked);
    CHECK(clicked.changed);
    CHECK(selected == 1U);
    ui.end();

    ui.feed_key(glm::Key::Right);
    ui.begin(0, 0, false);
    const glm::Interaction moved = ui.tab_bar("Settings tabs", tabs, selected);
    CHECK(moved.changed);
    CHECK(selected == 2U);
    ui.end();

    ui.feed_key(glm::Key::Home);
    ui.begin(0, 0, false);
    const glm::Interaction first = ui.tab_bar("Settings tabs", tabs, selected);
    CHECK(first.changed);
    CHECK(selected == 0U);
    ui.end();
}

TEST_CASE("tab bar clamps selection and ignores empty options") {
    glm::Context ui(160, 100);
    const std::vector<std::string> tabs{"One", "Two"};
    std::size_t selected = 99U;
    ui.begin(0, 0, false);
    const glm::Interaction clamped = ui.tab_bar("Tabs", tabs, selected);
    CHECK_FALSE(clamped.clicked);
    CHECK(selected == 1U);
    const std::vector<std::string> empty_tabs;
    selected = 7U;
    const glm::Interaction empty = ui.tab_bar("Empty", empty_tabs, selected);
    CHECK_FALSE(empty.clicked);
    CHECK(selected == 7U);
    ui.end();
}

TEST_CASE("image button responds to mouse and keyboard activation") {
    const std::array<std::uint32_t, 4> image{{
        0xFFFF0000U, 0xFF00FF00U, 0xFF0000FFU, 0xFFFFFFFFU,
    }};
    glm::Context ui(96, 96);

    ui.begin(20, 20, true);
    const glm::Interaction pressed = ui.image_button("Preview", image.data(), 2, 2);
    CHECK(pressed.active);
    CHECK_FALSE(pressed.clicked);
    ui.end();

    ui.begin(20, 20, false);
    const glm::Interaction clicked = ui.image_button("Preview", image.data(), 2, 2);
    CHECK(clicked.clicked);
    ui.end();

    ui.feed_key(glm::Key::Enter);
    ui.begin(0, 0, false);
    const glm::Interaction activated =
        ui.image_button("Preview", image.data(), 2, 2);
    CHECK(activated.clicked);
    ui.end();
}

TEST_CASE("image button preserves aspect ratio and centers image in button") {
    glm::Theme theme;
    theme.background = 0xFF123456U;
    theme.button = 0xFF654321U;
    glm::Context ui(96, 96, theme);
    const std::array<std::uint32_t, 2> image{{0xFFFF0000U, 0xFF00FF00U}};

    ui.begin(0, 0, false);
    ui.image_button("Wide image", image.data(), 2, 1, 64.0f);
    ui.end();

    CHECK(ui.pixels()[10U * 96U + 9U] == theme.button);
    CHECK(ui.pixels()[19U * 96U + 9U] == theme.button);
    CHECK(ui.pixels()[20U * 96U + 9U] == image[0]);
    CHECK(ui.pixels()[20U * 96U + 47U] == image[0]);
    CHECK(ui.pixels()[20U * 96U + 48U] == image[1]);
    CHECK(ui.pixels()[20U * 96U + 86U] == image[1]);
    CHECK(ui.pixels()[61U * 96U + 9U] == theme.button);
}

TEST_CASE("integer stepper adjusts by mouse and keyboard within its range") {
    glm::Context ui(160, 100);
    int amount = 5;

    ui.begin(140, 20, true);
    const glm::Interaction increment_pressed =
        ui.stepper("Amount", amount, 0, 10, 2);
    CHECK(increment_pressed.active);
    CHECK(amount == 5);
    ui.end();
    ui.begin(140, 20, false);
    const glm::Interaction incremented =
        ui.stepper("Amount", amount, 0, 10, 2);
    CHECK(incremented.clicked);
    CHECK(incremented.changed);
    CHECK(amount == 7);
    ui.end();

    ui.begin(10, 20, true);
    ui.stepper("Amount", amount, 0, 10, 2);
    ui.end();
    ui.begin(10, 20, false);
    const glm::Interaction decremented =
        ui.stepper("Amount", amount, 0, 10, 2);
    CHECK(decremented.changed);
    CHECK(amount == 5);
    ui.end();

    ui.feed_key(glm::Key::Right);
    ui.begin(0, 0, false);
    const glm::Interaction keyboard_increment =
        ui.stepper("Amount", amount, 0, 10, 2);
    CHECK(keyboard_increment.changed);
    CHECK(amount == 7);
    ui.end();

    ui.feed_key(glm::Key::End);
    ui.begin(0, 0, false);
    CHECK(ui.stepper("Amount", amount, 0, 10, 2).changed);
    CHECK(amount == 10);
    ui.end();

    ui.feed_key(glm::Key::Right);
    ui.begin(0, 0, false);
    CHECK_FALSE(ui.stepper("Amount", amount, 0, 10, 2).changed);
    CHECK(amount == 10);
    ui.end();

    ui.feed_key(glm::Key::Home);
    ui.begin(0, 0, false);
    CHECK(ui.stepper("Amount", amount, 0, 10, 2).changed);
    CHECK(amount == 0);
    ui.end();
}

TEST_CASE("integer stepper clamps initial values and normalizes reversed bounds") {
    glm::Context ui(160, 100);
    int amount = 20;
    ui.begin(0, 0, false);
    const glm::Interaction clamped = ui.stepper("Amount", amount, 10, 0, 0);
    ui.end();

    CHECK_FALSE(clamped.clicked);
    CHECK(amount == 10);
}

TEST_CASE("disabled buttons cannot be clicked or receive keyboard focus") {
    glm::Theme theme;
    theme.window = 0xFF123456U;
    glm::Context ui(120, 80, theme);

    ui.begin(12, 12, true);
    const glm::Interaction pressed = ui.button("Unavailable", false);
    CHECK_FALSE(pressed.hovered);
    CHECK_FALSE(pressed.active);
    CHECK_FALSE(pressed.clicked);
    CHECK(ui.focused_id() == 0U);
    ui.end();

    ui.begin(12, 12, false);
    const glm::Interaction released = ui.button("Unavailable", false);
    CHECK_FALSE(released.clicked);
    CHECK(ui.focused_id() == 0U);
    ui.end();

    ui.feed_key(glm::Key::Enter);
    ui.begin(0, 0, false);
    const glm::Interaction keyboard = ui.button("Unavailable", false);
    CHECK_FALSE(keyboard.clicked);
    CHECK(ui.focused_id() == 0U);
    ui.end();

    CHECK(ui.pixels()[8U * 120U + 8U] == theme.window);
}

TEST_CASE("disabling a focused button clears its pending activation") {
    glm::Context ui(120, 80);

    ui.begin(12, 12, true);
    ui.button("Apply");
    ui.end();
    ui.begin(12, 12, false);
    ui.button("Apply");
    ui.end();
    CHECK(ui.focused_id() != 0U);

    ui.begin(0, 0, false);
    const glm::Interaction disabled = ui.button("Apply", false);
    CHECK_FALSE(disabled.clicked);
    CHECK(ui.focused_id() == 0U);
    ui.end();

    ui.feed_key(glm::Key::Enter);
    ui.begin(0, 0, false);
    CHECK_FALSE(ui.button("Apply", false).clicked);
    ui.end();
}

TEST_CASE("vertical slider maps pointer position and keyboard to its value") {
    glm::Context ui(160, 140);
    float value = 0.5f;

    ui.begin(80, 16, true);
    const glm::Interaction pressed = ui.vertical_slider("Level", value, 0.0f, 1.0f);
    CHECK(pressed.active);
    CHECK(value == 1.0f);
    ui.end();

    ui.begin(80, 16, false);
    const glm::Interaction released =
        ui.vertical_slider("Level", value, 0.0f, 1.0f);
    CHECK(released.clicked);
    CHECK_FALSE(released.changed);
    ui.end();

    ui.feed_key(glm::Key::Down);
    ui.begin(0, 0, false);
    const glm::Interaction lowered =
        ui.vertical_slider("Level", value, 0.0f, 1.0f);
    CHECK(lowered.changed);
    CHECK(value < 1.0f);
    ui.end();

    ui.feed_key(glm::Key::Home);
    ui.begin(0, 0, false);
    CHECK(ui.vertical_slider("Level", value, 0.0f, 1.0f).changed);
    CHECK(value == 0.0f);
    ui.end();

    ui.feed_key(glm::Key::End);
    ui.begin(0, 0, false);
    CHECK(ui.vertical_slider("Level", value, 0.0f, 1.0f).changed);
    CHECK(value == 1.0f);
    ui.end();

}

TEST_CASE("vertical slider handles reversed and fixed ranges") {
    glm::Context ui(160, 140);
    float value = 15.0f;
    ui.begin(0, 0, false);
    const glm::Interaction reversed =
        ui.vertical_slider("Range", value, 20.0f, 10.0f);
    CHECK_FALSE(reversed.changed);
    CHECK(value == 15.0f);
    ui.end();

    value = -4.0f;
    ui.begin(0, 0, false);
    const glm::Interaction fixed =
        ui.vertical_slider("Fixed", value, 3.0f, 3.0f);
    CHECK_FALSE(fixed.changed);
    CHECK(value == 3.0f);
    ui.end();
}

TEST_CASE("color palette selects swatches by mouse and keyboard") {
    glm::Context ui(160, 120);
    const std::vector<std::uint32_t> colors{
        0xFFFF0000U, 0xFF00FF00U, 0xFF0000FFU, 0xFFFFFF00U, 0xFFFF00FFU,
    };
    std::size_t selected = 0U;

    ui.begin(60, 12, true);
    const glm::Interaction pressed = ui.color_palette("Theme colors", colors, selected, 3U);
    CHECK(pressed.active);
    CHECK(selected == 0U);
    ui.end();

    ui.begin(60, 12, false);
    const glm::Interaction clicked = ui.color_palette("Theme colors", colors, selected, 3U);
    CHECK(clicked.clicked);
    CHECK(clicked.changed);
    CHECK(selected == 1U);
    ui.end();

    ui.feed_key(glm::Key::Down);
    ui.begin(0, 0, false);
    const glm::Interaction moved = ui.color_palette("Theme colors", colors, selected, 3U);
    CHECK(moved.changed);
    CHECK(selected == 4U);
    ui.end();

    ui.feed_key(glm::Key::Home);
    ui.begin(0, 0, false);
    CHECK(ui.color_palette("Theme colors", colors, selected, 3U).changed);
    CHECK(selected == 0U);
    ui.end();

    ui.feed_key(glm::Key::End);
    ui.begin(0, 0, false);
    CHECK(ui.color_palette("Theme colors", colors, selected, 3U).changed);
    CHECK(selected == 4U);
    ui.end();
}

TEST_CASE("color palette clamps indices and draws nothing for empty colors") {
    glm::Context ui(120, 100);
    const std::vector<std::uint32_t> colors{0xFF123456U, 0xFFABCDEFU};
    std::size_t selected = 9U;
    ui.begin(0, 0, false);
    const glm::Interaction clamped = ui.color_palette("Colors", colors, selected, 0U);
    CHECK_FALSE(clamped.clicked);
    CHECK(selected == 1U);

    const std::vector<std::uint32_t> empty_colors;
    selected = 9U;
    const glm::Interaction empty =
        ui.color_palette("Empty colors", empty_colors, selected, 4U);
    CHECK_FALSE(empty.clicked);
    CHECK(selected == 9U);
    ui.end();
}

TEST_CASE("range slider moves either handle without crossing the other") {
    glm::Context ui(160, 100);
    float lower = 0.25f;
    float upper = 0.75f;

    ui.begin(44, 22, true);
    const glm::Interaction pressed =
        ui.range_slider("Range", lower, upper, 0.0f, 1.0f);
    CHECK(pressed.active);
    ui.end();

    ui.begin(58, 22, true);
    const glm::Interaction dragged =
        ui.range_slider("Range", lower, upper, 0.0f, 1.0f);
    CHECK(dragged.changed);
    CHECK(lower > 0.25f);
    CHECK(lower < upper);
    ui.end();

    ui.begin(58, 22, false);
    const glm::Interaction released =
        ui.range_slider("Range", lower, upper, 0.0f, 1.0f);
    CHECK(released.clicked);
    ui.end();

    ui.begin(116, 22, true);
    ui.range_slider("Range", lower, upper, 0.0f, 1.0f);
    ui.end();
    ui.begin(30, 22, true);
    const glm::Interaction clamped =
        ui.range_slider("Range", lower, upper, 0.0f, 1.0f);
    CHECK(clamped.changed);
    CHECK(lower <= upper);
    CHECK(upper == lower);
    ui.end();
}

TEST_CASE("range slider handles keyboard focus and normalizes its values") {
    glm::Context ui(160, 100);
    float lower = -1.0f;
    float upper = 2.0f;

    ui.begin(0, 0, false);
    const glm::Interaction normalized =
        ui.range_slider("Range", lower, upper, 0.0f, 1.0f);
    CHECK_FALSE(normalized.changed);
    CHECK(lower == 0.0f);
    CHECK(upper == 1.0f);
    ui.end();

    lower = 0.8f;
    upper = 0.2f;
    ui.begin(0, 0, false);
    const glm::Interaction ordered =
        ui.range_slider("Range", lower, upper, 1.0f, 0.0f);
    CHECK_FALSE(ordered.changed);
    CHECK(lower == 0.8f);
    CHECK(upper == 0.8f);
    ui.end();

    lower = 0.0f;
    upper = 1.0f;
    ui.feed_key(glm::Key::Tab);
    ui.feed_key(glm::Key::Right);
    ui.begin(0, 0, false);
    const glm::Interaction lower_moved =
        ui.range_slider("Range", lower, upper, 0.0f, 1.0f);
    CHECK(lower_moved.changed);
    CHECK(lower > 0.0f);
    CHECK(lower < upper);
    ui.end();

    ui.feed_key(glm::Key::Tab);
    ui.feed_key(glm::Key::Home);
    ui.begin(0, 0, false);
    const glm::Interaction upper_clamped =
        ui.range_slider("Range", lower, upper, 0.0f, 1.0f);
    CHECK(upper_clamped.changed);
    CHECK(upper == lower);
    ui.end();
}

TEST_CASE("rating selects with the mouse and keyboard") {
    glm::Context ui(180, 100);
    int value = 0;

    ui.begin(93, 20, true);
    const glm::Interaction pressed = ui.rating("Clarity", value, 5);
    CHECK(pressed.active);
    CHECK(value == 0);
    ui.end();

    ui.begin(93, 20, false);
    const glm::Interaction selected = ui.rating("Clarity", value, 5);
    CHECK(selected.clicked);
    CHECK(selected.changed);
    CHECK(value == 4);
    ui.end();

    ui.feed_key(glm::Key::Left);
    ui.begin(0, 0, false);
    const glm::Interaction lowered = ui.rating("Clarity", value, 5);
    CHECK(lowered.changed);
    CHECK(value == 3);
    ui.end();

    ui.feed_key(glm::Key::Home);
    ui.begin(0, 0, false);
    CHECK(ui.rating("Clarity", value, 5).changed);
    CHECK(value == 0);
    ui.end();

    ui.feed_key(glm::Key::End);
    ui.begin(0, 0, false);
    CHECK(ui.rating("Clarity", value, 5).changed);
    CHECK(value == 5);
    ui.end();
}

TEST_CASE("rating clamps its value and supports a single-mark scale") {
    glm::Context ui(100, 80);
    int value = 9;
    ui.begin(0, 0, false);
    const glm::Interaction clamped = ui.rating("Score", value, 3);
    CHECK_FALSE(clamped.changed);
    CHECK(value == 3);
    ui.end();

    value = 0;
    ui.begin(12, 20, true);
    const glm::Interaction pressed = ui.rating("Single", value, 1);
    CHECK(pressed.active);
    ui.end();
    ui.begin(12, 20, false);
    const glm::Interaction selected = ui.rating("Single", value, 1);
    CHECK(selected.clicked);
    CHECK(selected.changed);
    CHECK(value == 1);
    ui.end();

    value = 0;
    ui.begin(0, 0, false);
    ui.rating("Minimum", value, 0);
    ui.end();
    ui.feed_key(glm::Key::Tab);
    ui.feed_key(glm::Key::End);
    ui.begin(0, 0, false);
    const glm::Interaction non_positive_maximum =
        ui.rating("Minimum", value, 0);
    CHECK(non_positive_maximum.changed);
    CHECK(value == 1);
    ui.end();
}

TEST_CASE("knob maps pointer angle and supports keyboard adjustment") {
    glm::Context ui(120, 100);
    float value = 0.0f;

    ui.begin(60, 18, true);
    const glm::Interaction pressed = ui.knob("Gain", value, 0.0f, 1.0f);
    CHECK(pressed.active);
    CHECK(value == doctest::Approx(0.5f));
    ui.end();

    ui.begin(60, 18, false);
    const glm::Interaction released = ui.knob("Gain", value, 0.0f, 1.0f);
    CHECK(released.clicked);
    ui.end();

    ui.feed_key(glm::Key::Right);
    ui.begin(0, 0, false);
    const glm::Interaction increased = ui.knob("Gain", value, 0.0f, 1.0f);
    CHECK(increased.changed);
    CHECK(value > 0.5f);
    ui.end();

    ui.feed_key(glm::Key::Home);
    ui.begin(0, 0, false);
    CHECK(ui.knob("Gain", value, 0.0f, 1.0f).changed);
    CHECK(value == 0.0f);
    ui.end();

    ui.feed_key(glm::Key::End);
    ui.begin(0, 0, false);
    CHECK(ui.knob("Gain", value, 0.0f, 1.0f).changed);
    CHECK(value == 1.0f);
    ui.end();

    ui.begin(60, 54, true);
    const glm::Interaction endpoint = ui.knob("Gain", value, 0.0f, 1.0f);
    CHECK(endpoint.active);
    CHECK(value == 1.0f);
    ui.end();
}

TEST_CASE("knob clamps values and handles reversed or fixed ranges") {
    glm::Context ui(120, 100);
    float value = 4.0f;
    ui.begin(0, 0, false);
    const glm::Interaction clamped = ui.knob("Bounded", value, 0.0f, 1.0f);
    CHECK_FALSE(clamped.changed);
    CHECK(value == 1.0f);
    ui.end();

    value = 0.5f;
    ui.begin(0, 0, false);
    const glm::Interaction reversed = ui.knob("Reversed", value, 1.0f, -1.0f);
    CHECK_FALSE(reversed.changed);
    CHECK(value == 0.5f);
    ui.end();

    value = -4.0f;
    ui.begin(0, 0, false);
    const glm::Interaction fixed = ui.knob("Fixed", value, 3.0f, 3.0f);
    CHECK_FALSE(fixed.changed);
    CHECK(value == 3.0f);
    ui.end();
}

TEST_CASE("drag value responds to pointer deltas and keyboard input") {
    glm::Context ui(160, 100);
    float value = 0.5f;

    ui.begin(40, 20, true);
    const glm::Interaction pressed =
        ui.drag_value("Fine tune", value, 0.0f, 1.0f, 0.01f);
    CHECK(pressed.active);
    CHECK(value == 0.5f);
    ui.end();

    ui.begin(50, 20, true);
    const glm::Interaction dragged =
        ui.drag_value("Fine tune", value, 0.0f, 1.0f, 0.01f);
    CHECK(dragged.changed);
    CHECK(value == doctest::Approx(0.6f));
    ui.end();

    ui.begin(50, 20, false);
    CHECK(ui.drag_value("Fine tune", value, 0.0f, 1.0f, 0.01f).clicked);
    ui.end();

    ui.feed_key(glm::Key::Right);
    ui.begin(0, 0, false);
    const glm::Interaction increased =
        ui.drag_value("Fine tune", value, 0.0f, 1.0f, 0.01f);
    CHECK(increased.changed);
    CHECK(value == doctest::Approx(0.61f));
    ui.end();

    ui.feed_key(glm::Key::End);
    ui.begin(0, 0, false);
    CHECK(ui.drag_value("Fine tune", value, 0.0f, 1.0f, 0.01f).changed);
    CHECK(value == 1.0f);
    ui.end();
}

TEST_CASE("drag value clamps ranges and ignores negative sensitivity") {
    glm::Context ui(160, 100);
    float value = 4.0f;
    ui.begin(0, 0, false);
    const glm::Interaction clamped =
        ui.drag_value("Clamp", value, 0.0f, 1.0f, 0.1f);
    CHECK_FALSE(clamped.changed);
    CHECK(value == 1.0f);
    ui.end();

    value = 0.5f;
    ui.begin(40, 20, true);
    ui.drag_value("Negative", value, 0.0f, 1.0f, -0.1f);
    ui.end();
    ui.begin(50, 20, true);
    const glm::Interaction ignored =
        ui.drag_value("Negative", value, 0.0f, 1.0f, -0.1f);
    CHECK_FALSE(ignored.changed);
    CHECK(value == 0.5f);
    ui.end();

    value = 0.5f;
    ui.begin(0, 0, false);
    const glm::Interaction reversed =
        ui.drag_value("Reversed", value, 1.0f, -1.0f, 0.01f);
    CHECK_FALSE(reversed.changed);
    CHECK(value == 0.5f);
    ui.end();

    value = -4.0f;
    ui.begin(0, 0, false);
    const glm::Interaction fixed =
        ui.drag_value("Fixed", value, 3.0f, 3.0f, 0.01f);
    CHECK_FALSE(fixed.changed);
    CHECK(value == 3.0f);
    ui.end();
}

TEST_CASE("xy pad maps pointer position and keyboard to two values") {
    glm::Context ui(180, 140);
    float x = 0.25f;
    float y = 0.75f;

    ui.begin(163, 17, true);
    const glm::Interaction pressed =
        ui.xy_pad("Position", x, y, 0.0f, 1.0f, 0.0f, 1.0f, 100.0f);
    CHECK(pressed.active);
    CHECK(x > 0.9f);
    CHECK(y > 0.9f);
    ui.end();

    ui.begin(16, 99, true);
    const glm::Interaction dragged =
        ui.xy_pad("Position", x, y, 0.0f, 1.0f, 0.0f, 1.0f, 100.0f);
    CHECK(dragged.changed);
    CHECK(x < 0.1f);
    CHECK(y < 0.1f);
    ui.end();

    ui.feed_key(glm::Key::Right);
    ui.feed_key(glm::Key::Up);
    ui.begin(0, 0, false);
    const glm::Interaction nudged =
        ui.xy_pad("Position", x, y, 0.0f, 1.0f, 0.0f, 1.0f, 100.0f);
    CHECK(nudged.changed);
    CHECK(x > 0.0f);
    CHECK(y > 0.0f);
    ui.end();
}

TEST_CASE("xy pad clamps values and normalizes reversed and fixed ranges") {
    glm::Context ui(180, 140);
    float x = 3.0f;
    float y = -2.0f;
    ui.begin(0, 0, false);
    const glm::Interaction clamped =
        ui.xy_pad("Clamp", x, y, 0.0f, 1.0f, 0.0f, 1.0f);
    CHECK_FALSE(clamped.changed);
    CHECK(x == 1.0f);
    CHECK(y == 0.0f);
    ui.end();

    x = 0.25f;
    y = 0.75f;
    ui.begin(0, 0, false);
    const glm::Interaction reversed =
        ui.xy_pad("Reversed", x, y, 1.0f, -1.0f, 2.0f, -2.0f);
    CHECK_FALSE(reversed.changed);
    CHECK(x == 0.25f);
    CHECK(y == 0.75f);
    ui.end();

    x = -4.0f;
    y = 9.0f;
    ui.begin(0, 0, false);
    const glm::Interaction fixed =
        ui.xy_pad("Fixed", x, y, 3.0f, 3.0f, 5.0f, 5.0f);
    CHECK_FALSE(fixed.changed);
    CHECK(x == 3.0f);
    CHECK(y == 5.0f);
    ui.end();
}

TEST_CASE("sparkline draws normalized sample paths in the theme accent") {
    glm::Theme theme;
    theme.background = 0xFF010203U;
    theme.window = 0xFF111213U;
    theme.border = 0xFF212223U;
    theme.accent = 0xFFABCDEFU;
    glm::Context ui(80, 60, theme);
    const std::vector<float> samples{0.0f, 1.0f, 0.0f};

    ui.begin(0, 0, false);
    ui.sparkline(samples, 0.0f, 1.0f, 32.0f);
    ui.end();

    const std::uint32_t* pixels = ui.pixels();
    CHECK(pixels[37U * 80U + 10U] == theme.accent);
    CHECK(pixels[10U * 80U + 39U] == theme.accent);
    CHECK(pixels[37U * 80U + 69U] == theme.accent);
    CHECK(pixels[11U * 80U + 11U] == theme.window);
    CHECK(pixels[0] == theme.background);
}

TEST_CASE("sparkline handles empty, constant, and non-finite samples") {
    glm::Theme theme;
    theme.window = 0xFF101010U;
    theme.border = 0xFF202020U;
    theme.accent = 0xFF303030U;
    glm::Context ui(80, 120, theme);
    const std::vector<float> empty;
    const std::vector<float> constant{0.5f, 0.5f, 0.5f};
    const std::vector<float> interrupted{
        0.0f, std::numeric_limits<float>::quiet_NaN(), 1.0f};
    const std::vector<float> single{0.75f};

    ui.begin(0, 0, false);
    ui.sparkline(empty, 0.0f, 1.0f, 20.0f);
    ui.sparkline(constant, 0.0f, 1.0f, 20.0f);
    ui.sparkline(interrupted, 0.0f, 1.0f, 20.0f);
    ui.sparkline(single, 0.0f, 1.0f, 20.0f);
    ui.end();

    const std::uint32_t* pixels = ui.pixels();
    CHECK(pixels[45U * 80U + 10U] == theme.accent);
    CHECK(pixels[45U * 80U + 39U] == theme.accent);
    CHECK(pixels[45U * 80U + 69U] == theme.accent);
    CHECK(pixels[81U * 80U + 10U] == theme.window);
    CHECK(pixels[66U * 80U + 69U] == theme.window);
    CHECK(pixels[74U * 80U + 39U] == theme.window);
    CHECK(pixels[97U * 80U + 40U] == theme.accent);
}

TEST_CASE("histogram draws proportional bars for sample buckets") {
    glm::Theme theme;
    theme.background = 0xFF010203U;
    theme.window = 0xFF111213U;
    theme.border = 0xFF212223U;
    theme.accent = 0xFFABCDEFU;
    glm::Context ui(40, 60, theme);
    const std::vector<float> samples{0.1f, 0.2f, 0.8f};

    ui.begin(0, 0, false);
    ui.histogram(samples, 0.0f, 1.0f, 4U, 24.0f);
    ui.end();

    const std::uint32_t* pixels = ui.pixels();
    CHECK(pixels[19U * 40U + 12U] == theme.accent);
    CHECK(pixels[19U * 40U + 14U] == theme.accent);
    CHECK(pixels[18U * 40U + 14U] == theme.accent);
    CHECK(pixels[21U * 40U + 27U] == theme.accent);
    CHECK(pixels[19U * 40U + 27U] == theme.window);
    CHECK(pixels[0] == theme.background);
}

TEST_CASE("histogram handles empty, non-finite, and degenerate ranges") {
    glm::Theme theme;
    theme.window = 0xFF101010U;
    theme.border = 0xFF202020U;
    theme.accent = 0xFF303030U;
    glm::Context ui(40, 80, theme);
    const std::vector<float> empty;
    const std::vector<float> invalid{
        std::numeric_limits<float>::quiet_NaN(),
        std::numeric_limits<float>::infinity()};
    const std::vector<float> constant{0.5f, 0.5f};

    ui.begin(0, 0, false);
    ui.histogram(empty, 0.0f, 1.0f, 4U, 20.0f);
    ui.histogram(invalid, 0.0f, 1.0f, 4U, 20.0f);
    ui.histogram(constant, 0.5f, 0.5f, 4U, 20.0f);
    ui.end();

    const std::uint32_t* pixels = ui.pixels();
    CHECK(pixels[12U * 40U + 12U] == theme.window);
    CHECK(pixels[40U * 40U + 12U] == theme.window);
    CHECK(pixels[70U * 40U + 22U] == theme.accent);
}

TEST_CASE("scatter plot maps bounded points into its chart area") {
    glm::Theme theme;
    theme.background = 0xFF010203U;
    theme.window = 0xFF111213U;
    theme.border = 0xFF212223U;
    theme.accent = 0xFFABCDEFU;
    theme.padding = 0;
    glm::Context ui(64, 32, theme);
    const std::vector<std::pair<float, float>> points{
        {0.0f, 10.0f}, {5.0f, 5.0f}, {10.0f, 0.0f}};

    ui.begin(0, 0, false);
    ui.scatter_plot(points, 0.0f, 10.0f, 0.0f, 10.0f, 24.0f);
    ui.end();

    const std::uint32_t* pixels = ui.pixels();
    CHECK(pixels[2U * 64U + 2U] == theme.accent);
    CHECK(pixels[11U * 64U + 31U] == theme.accent);
    CHECK(pixels[21U * 64U + 61U] == theme.accent);
    CHECK(pixels[2U * 64U + 32U] == theme.window);
    CHECK(pixels[0] == theme.border);
}

TEST_CASE("scatter plot clips outside and ignores invalid points and bounds") {
    glm::Theme theme;
    theme.window = 0xFF101010U;
    theme.border = 0xFF202020U;
    theme.accent = 0xFF303030U;
    theme.padding = 0;
    glm::Context ui(48, 72, theme);
    const std::vector<std::pair<float, float>> points{
        {-1.0f, 0.0f}, {11.0f, 10.0f},
        {std::numeric_limits<float>::quiet_NaN(), 5.0f},
        {5.0f, std::numeric_limits<float>::infinity()}};

    ui.begin(0, 0, false);
    ui.scatter_plot(points, 0.0f, 10.0f, 0.0f, 10.0f, 24.0f);
    ui.scatter_plot(points, 10.0f, 0.0f, 0.0f, 10.0f, 24.0f);
    ui.scatter_plot(points, 0.0f, std::numeric_limits<float>::quiet_NaN(),
                    0.0f, 10.0f, 12.0f);
    ui.end();

    const std::uint32_t* pixels = ui.pixels();
    CHECK(pixels[10U * 48U + 24U] == theme.window);
    CHECK(pixels[35U * 48U + 24U] == theme.window);
    CHECK(pixels[55U * 48U + 24U] == theme.window);
}

TEST_CASE("scatter plot centers points on degenerate coordinate ranges") {
    glm::Theme theme;
    theme.window = 0xFF101010U;
    theme.border = 0xFF202020U;
    theme.accent = 0xFF303030U;
    theme.padding = 0;
    glm::Context ui(32, 24, theme);
    const std::vector<std::pair<float, float>> point{{2.0f, 4.0f}};

    ui.begin(0, 0, false);
    ui.scatter_plot(point, 2.0f, 2.0f, 4.0f, 4.0f, 20.0f);
    ui.end();

    CHECK(ui.pixels()[9U * 32U + 15U] == theme.accent);
}

TEST_CASE("pie chart draws clockwise slices using its color palette") {
    glm::Theme theme;
    theme.background = 0xFF010203U;
    theme.window = 0xFF111213U;
    theme.border = 0xFF212223U;
    theme.padding = 0;
    glm::Context ui(48, 40, theme);
    const std::vector<float> values{1.0f, 1.0f};
    const std::vector<std::uint32_t> colors{0xFFFF0000U, 0xFF00FF00U};

    ui.begin(0, 0, false);
    ui.pie_chart(values, colors, 36.0f);
    ui.end();

    const std::uint32_t* pixels = ui.pixels();
    CHECK(pixels[4U * 48U + 24U] == colors[0]);
    CHECK(pixels[31U * 48U + 23U] == colors[1]);
    CHECK(pixels[39U * 48U] == theme.background);

    glm::Context repeated_ui(48, 40, theme);
    const std::vector<std::uint32_t> one_color{0xFFABCDEFU};
    repeated_ui.begin(0, 0, false);
    repeated_ui.pie_chart(values, one_color, 36.0f);
    repeated_ui.end();
    CHECK(repeated_ui.pixels()[4U * 48U + 24U] == one_color[0]);
    CHECK(repeated_ui.pixels()[31U * 48U + 23U] == one_color[0]);
}

TEST_CASE("pie chart ignores invalid slices and leaves empty charts blank") {
    glm::Theme theme;
    theme.window = 0xFF101010U;
    theme.border = 0xFF202020U;
    theme.padding = 0;
    glm::Context ui(40, 96, theme);
    const std::vector<float> mixed{
        std::numeric_limits<float>::quiet_NaN(), -2.0f, 3.0f,
        std::numeric_limits<float>::infinity()};
    const std::vector<std::uint32_t> one_color{0xFFABCDEFU};
    const std::vector<float> empty_values;
    const std::vector<std::uint32_t> colors{0xFFE06C75U, 0xFF61AFEFU};
    const std::vector<std::uint32_t> empty_colors;
    const std::vector<float> zero_values{0.0f, -1.0f};

    ui.begin(0, 0, false);
    ui.pie_chart(mixed, one_color, 24.0f);
    ui.pie_chart(empty_values, colors, 24.0f);
    ui.pie_chart(zero_values, colors, 24.0f);
    ui.pie_chart(mixed, empty_colors, 24.0f);
    ui.end();

    const std::uint32_t* pixels = ui.pixels();
    CHECK(pixels[10U * 40U + 20U] == one_color[0]);
    CHECK(pixels[35U * 40U + 20U] == theme.window);
    CHECK(pixels[60U * 40U + 20U] == theme.window);
    CHECK(pixels[83U * 40U + 20U] == theme.window);
}

TEST_CASE("donut chart draws palette slices around a themed center hole") {
    glm::Theme theme;
    theme.background = 0xFF010203U;
    theme.window = 0xFF111213U;
    theme.border = 0xFF212223U;
    theme.padding = 0;
    glm::Context ui(48, 40, theme);
    const std::vector<float> values{1.0f, 1.0f};
    const std::vector<std::uint32_t> colors{0xFFFF0000U, 0xFF00FF00U};

    ui.begin(0, 0, false);
    ui.donut_chart(values, colors, 36.0f);
    ui.end();

    const std::uint32_t* pixels = ui.pixels();
    CHECK(pixels[4U * 48U + 24U] == colors[0]);
    CHECK(pixels[31U * 48U + 23U] == colors[1]);
    CHECK(pixels[18U * 48U + 24U] == theme.window);
    CHECK(pixels[39U * 48U] == theme.background);
}

TEST_CASE("donut chart leaves the chart background for invalid data") {
    glm::Theme theme;
    theme.window = 0xFF101010U;
    theme.border = 0xFF202020U;
    theme.padding = 0;
    glm::Context ui(40, 48, theme);
    const std::vector<float> values{
        std::numeric_limits<float>::quiet_NaN(), 0.0f, -1.0f};
    const std::vector<std::uint32_t> colors{0xFFABCDEFU};

    ui.begin(0, 0, false);
    ui.donut_chart(values, colors, 24.0f);
    ui.end();

    CHECK(ui.pixels()[10U * 40U + 20U] == theme.window);
}

TEST_CASE("tree nodes toggle open with mouse and keyboard") {
    glm::Context ui(160, 120);
    bool open = false;

    ui.begin(12, 16, true);
    CHECK_FALSE(ui.begin_tree_node("Audio", open));
    ui.end();

    ui.begin(12, 16, false);
    const bool opened = ui.begin_tree_node("Audio", open);
    CHECK(opened);
    CHECK(open);
    if (opened) {
        ui.label("Nested setting");
        ui.end_tree_node();
    }
    ui.end();

    bool keyboard_open = false;
    ui.begin(0, 0, false);
    ui.begin_tree_node("Video", keyboard_open);
    ui.end();
    ui.feed_key(glm::Key::Tab);
    ui.feed_key(glm::Key::Enter);
    ui.begin(0, 0, false);
    const bool keyboard_result =
        ui.begin_tree_node("Video", keyboard_open);
    CHECK(keyboard_result);
    CHECK(keyboard_open);
    if (keyboard_result) {
        ui.end_tree_node();
    }
    ui.end();
}

TEST_CASE("tree nodes scope child IDs and restore parent layout") {
    glm::Context ui(160, 180);
    bool first_open = true;
    bool second_open = true;

    const auto draw_tree = [&] {
        CHECK(ui.begin_tree_node("First branch", first_open));
        ui.button("Repeated child");
        ui.end_tree_node();
        CHECK(ui.begin_tree_node("Second branch", second_open));
        ui.button("Repeated child");
        ui.end_tree_node();
    };

    ui.begin(22, 48, true);
    draw_tree();
    ui.end();
    ui.begin(22, 48, false);
    draw_tree();
    const std::uint64_t first_child_focus = ui.focused_id();
    ui.end();
    CHECK(first_child_focus != 0U);

    ui.begin(22, 116, true);
    draw_tree();
    ui.end();
    ui.begin(22, 116, false);
    draw_tree();
    const std::uint64_t second_child_focus = ui.focused_id();
    ui.end();
    CHECK(second_child_focus != 0U);
    CHECK(first_child_focus != second_child_focus);
}

TEST_CASE("color picker edits individual packed RGBA channels") {
    glm::Context ui(220, 180);
    std::uint32_t color = 0x7F204060U;

    ui.begin(0, 0, false);
    const glm::Interaction unchanged = ui.color_picker("Accent", color);
    CHECK_FALSE(unchanged.changed);
    CHECK(color == 0x7F204060U);
    ui.end();

    ui.feed_key(glm::Key::Tab);
    ui.feed_key(glm::Key::Right);
    ui.begin(0, 0, false);
    const glm::Interaction red_changed = ui.color_picker("Accent", color);
    CHECK(red_changed.changed);
    CHECK((color & 0xFFU) == 0x61U);
    CHECK(((color >> 8U) & 0xFFU) == 0x40U);
    CHECK(((color >> 16U) & 0xFFU) == 0x20U);
    CHECK(((color >> 24U) & 0xFFU) == 0x7FU);
    ui.end();

    ui.feed_key(glm::Key::Tab);
    ui.feed_key(glm::Key::End);
    ui.begin(0, 0, false);
    const glm::Interaction green_changed = ui.color_picker("Accent", color);
    CHECK(green_changed.changed);
    CHECK(((color >> 8U) & 0xFFU) == 0xFFU);
    CHECK((color & 0xFFU) == 0x61U);
    ui.end();

    ui.feed_key(glm::Key::Tab);
    ui.feed_key(glm::Key::Tab);
    ui.feed_key(glm::Key::End);
    ui.begin(0, 0, false);
    const glm::Interaction alpha_changed = ui.color_picker("Accent", color);
    CHECK(alpha_changed.changed);
    CHECK((color >> 24U) == 0xFFU);
    CHECK((color & 0xFFU) == 0x61U);
    ui.end();
}

TEST_CASE("color picker scopes channel focus and draws a color swatch") {
    glm::Theme theme;
    theme.window = 0xFF111111U;
    glm::Context ui(220, 180, theme);
    std::uint32_t first = 0xFF123456U;
    std::uint32_t second = 0xFF123456U;

    ui.begin(0, 0, false);
    ui.color_picker("First picker", first);
    ui.color_picker("Second picker", second);
    ui.end();
    CHECK(ui.pixels()[10U * ui.width() + 10U] == first);

    ui.feed_key(glm::Key::Tab);
    ui.feed_key(glm::Key::Right);
    ui.begin(0, 0, false);
    ui.color_picker("First picker", first);
    ui.color_picker("Second picker", second);
    ui.end();
    CHECK((first & 0xFFU) == 0x57U);
    CHECK((second & 0xFFU) == 0x56U);
}

TEST_CASE("tables allocate columns and advance rows independently") {
    glm::Theme theme;
    theme.padding = 8;
    theme.button = 0xFF101010U;
    theme.button_hover = 0xFF202020U;
    theme.button_active = 0xFF303030U;
    glm::Context ui(120, 140, theme);

    ui.begin(0, 0, false);
    ui.begin_table("Grid", 2U);
    ui.button("First cell");
    ui.next_table_column();
    ui.button("Second cell");
    ui.next_table_row();
    ui.button("Third cell");
    ui.next_table_column();
    ui.button("Fourth cell");
    ui.end_table();
    ui.button("After table");
    ui.end();

    const std::uint32_t* pixels = ui.pixels();
    CHECK(pixels[10U * 120U + 10U] == theme.button);
    CHECK(pixels[10U * 120U + 65U] == theme.button);
    CHECK(pixels[46U * 120U + 10U] == theme.button);
    CHECK(pixels[46U * 120U + 65U] == theme.button);
    CHECK(pixels[82U * 120U + 10U] == theme.button);
}

TEST_CASE("tables treat a zero column count as a single column") {
    glm::Theme theme;
    theme.button = 0xFF101010U;
    glm::Context ui(100, 80, theme);
    ui.begin(0, 0, false);
    ui.begin_table("Single column", 0U);
    ui.button("Only cell");
    ui.next_table_column();
    ui.end_table();
    ui.end();
    CHECK(ui.pixels()[10U * 100U + 10U] == theme.button);
}

TEST_CASE("table cell interactions work and nested tables restore their parent") {
    glm::Context ui(160, 180);
    bool clicked_first = false;
    bool clicked_second = false;
    bool clicked_parent = false;
    const auto draw_table = [&] {
        ui.begin_table("Outer", 2U);
        const glm::Interaction left = ui.button("Cell action");
        clicked_first = clicked_first || left.clicked;
        ui.next_table_column();
        const glm::Interaction right = ui.button("Cell action");
        clicked_second = clicked_second || right.clicked;
        ui.next_table_row();
        ui.button("Nested start");
        ui.begin_table("Inner", 2U);
        ui.button("Inner one");
        ui.next_table_column();
        ui.button("Inner two");
        ui.end_table();
        ui.end_table();
        const glm::Interaction parent = ui.button("Parent after");
        clicked_parent = clicked_parent || parent.clicked;
    };

    ui.begin(90, 10, true);
    draw_table();
    ui.end();
    ui.begin(90, 10, false);
    draw_table();
    ui.end();
    CHECK_FALSE(clicked_first);
    CHECK(clicked_second);

    ui.begin(10, 118, true);
    draw_table();
    ui.end();
    ui.begin(10, 118, false);
    draw_table();
    CHECK(clicked_parent);
    ui.end();
}
