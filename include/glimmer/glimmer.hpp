#ifndef GLIMMER_GLIMMER_HPP
#define GLIMMER_GLIMMER_HPP

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <string>
#include <utility>
#include <unordered_map>
#include <vector>

namespace glm {

// An integer-coordinate rectangle used by drawing and layout APIs.
struct Rect {
    float x = 0.0f;
    float y = 0.0f;
    float w = 0.0f;
    float h = 0.0f;
};

namespace detail {

constexpr std::size_t kGlyphCount = 95;

constexpr std::array<std::array<std::uint8_t, 7>, 36> kAlphaNumeric{{
    {{14, 17, 17, 31, 17, 17, 17}}, {{30, 17, 17, 30, 17, 17, 30}},
    {{14, 17, 16, 16, 16, 17, 14}}, {{30, 17, 17, 17, 17, 17, 30}},
    {{31, 16, 16, 30, 16, 16, 31}}, {{31, 16, 16, 30, 16, 16, 16}},
    {{14, 17, 16, 23, 17, 17, 15}}, {{17, 17, 17, 31, 17, 17, 17}},
    {{14, 4, 4, 4, 4, 4, 14}}, {{7, 2, 2, 2, 18, 18, 12}},
    {{17, 18, 20, 24, 20, 18, 17}}, {{16, 16, 16, 16, 16, 16, 31}},
    {{17, 27, 21, 21, 17, 17, 17}}, {{17, 25, 21, 19, 17, 17, 17}},
    {{14, 17, 17, 17, 17, 17, 14}}, {{30, 17, 17, 30, 16, 16, 16}},
    {{14, 17, 17, 17, 21, 18, 13}}, {{30, 17, 17, 30, 20, 18, 17}},
    {{15, 16, 16, 14, 1, 1, 30}}, {{31, 4, 4, 4, 4, 4, 4}},
    {{17, 17, 17, 17, 17, 17, 14}}, {{17, 17, 17, 17, 17, 10, 4}},
    {{17, 17, 17, 21, 21, 21, 10}}, {{17, 17, 10, 4, 10, 17, 17}},
    {{17, 17, 10, 4, 4, 4, 4}}, {{31, 1, 2, 4, 8, 16, 31}},
    {{14, 17, 19, 21, 25, 17, 14}}, {{4, 12, 4, 4, 4, 4, 14}},
    {{14, 17, 1, 2, 4, 8, 31}}, {{30, 1, 1, 14, 1, 1, 30}},
    {{2, 6, 10, 18, 31, 2, 2}}, {{31, 16, 16, 30, 1, 1, 30}},
    {{14, 16, 16, 30, 17, 17, 14}}, {{31, 1, 2, 4, 8, 8, 8}},
    {{14, 17, 17, 14, 17, 17, 14}}, {{14, 17, 17, 15, 1, 1, 14}},
}};

constexpr std::uint8_t glyph_row(char character, std::size_t row) {
    const unsigned char code = static_cast<unsigned char>(character);
    if (code >= static_cast<unsigned char>('a') &&
        code <= static_cast<unsigned char>('z')) {
        return kAlphaNumeric[static_cast<std::size_t>(code - 'a')][row];
    }
    if (code >= static_cast<unsigned char>('A') &&
        code <= static_cast<unsigned char>('Z')) {
        return kAlphaNumeric[static_cast<std::size_t>(code - 'A')][row];
    }
    if (code >= static_cast<unsigned char>('0') &&
        code <= static_cast<unsigned char>('9')) {
        return kAlphaNumeric[26U + static_cast<std::size_t>(code - '0')][row];
    }
    constexpr std::array<std::array<std::uint8_t, 7>, 16> punctuation{{
        {{0, 0, 0, 0, 0, 0, 0}}, {{4, 4, 4, 4, 4, 0, 4}},
        {{10, 10, 10, 0, 0, 0, 0}}, {{10, 31, 10, 10, 31, 10, 0}},
        {{4, 15, 20, 14, 5, 30, 4}}, {{24, 25, 2, 4, 8, 19, 3}},
        {{12, 18, 20, 8, 21, 18, 13}}, {{4, 4, 8, 0, 0, 0, 0}},
        {{2, 4, 8, 8, 8, 4, 2}}, {{8, 4, 2, 2, 2, 4, 8}},
        {{0, 4, 21, 14, 21, 4, 0}}, {{0, 4, 4, 31, 4, 4, 0}},
        {{0, 0, 0, 0, 4, 4, 8}}, {{0, 0, 0, 31, 0, 0, 0}},
        {{0, 0, 0, 0, 0, 12, 12}}, {{1, 2, 2, 4, 8, 8, 16}},
    }};
    if (character >= ' ' && character <= '/') {
        return punctuation[static_cast<std::size_t>(character - ' ')][row];
    }
    if (character >= ':' && character <= '@') {
        constexpr std::array<std::array<std::uint8_t, 7>, 7> extra{{
            {{0, 12, 12, 0, 12, 12, 0}}, {{0, 12, 12, 0, 4, 4, 8}},
            {{2, 4, 8, 16, 8, 4, 2}}, {{0, 0, 31, 0, 31, 0, 0}},
            {{8, 4, 2, 1, 2, 4, 8}}, {{14, 17, 1, 2, 4, 0, 4}},
            {{14, 17, 23, 21, 23, 16, 14}},
        }};
        return extra[static_cast<std::size_t>(character - ':')][row];
    }
    if (character == '[' || character == '\\' || character == ']' ||
        character == '^' || character == '_' || character == '`') {
        constexpr std::array<std::array<std::uint8_t, 7>, 6> extra{{
            {{14, 8, 8, 8, 8, 8, 14}}, {{16, 8, 8, 4, 2, 2, 1}},
            {{14, 2, 2, 2, 2, 2, 14}}, {{4, 10, 17, 0, 0, 0, 0}},
            {{0, 0, 0, 0, 0, 0, 31}}, {{8, 4, 2, 0, 0, 0, 0}},
        }};
        return extra[static_cast<std::size_t>(character - '[')][row];
    }
    if (character >= '{' && character <= '~') {
        constexpr std::array<std::array<std::uint8_t, 7>, 6> extra{{
            {{6, 8, 8, 24, 8, 8, 6}}, {{4, 4, 4, 0, 4, 4, 4}},
            {{24, 4, 4, 3, 4, 4, 24}}, {{0, 0, 9, 22, 16, 0, 0}},
            {{0, 0, 0, 31, 0, 0, 0}}, {{0, 0, 0, 0, 0, 0, 0}},
        }};
        return extra[static_cast<std::size_t>(character - '{')][row];
    }
    return 14U;
}

constexpr std::array<std::array<std::uint8_t, 16>, kGlyphCount> make_font() {
    std::array<std::array<std::uint8_t, 16>, kGlyphCount> result{};
    for (std::size_t character = 0; character < kGlyphCount; ++character) {
        for (std::size_t row = 0; row < 7; ++row) {
            const auto bits = glyph_row(static_cast<char>(character + 32U), row);
            result[character][row * 2U + 1U] =
                static_cast<std::uint8_t>(bits << 2U);
            result[character][row * 2U + 2U] =
                static_cast<std::uint8_t>(bits << 2U);
        }
    }
    return result;
}

constexpr auto kFont = make_font();

inline Rect intersect(Rect first, Rect second) {
    const float left = std::max(first.x, second.x);
    const float top = std::max(first.y, second.y);
    const float right = std::min(first.x + first.w, second.x + second.w);
    const float bottom = std::min(first.y + first.h, second.y + second.h);
    return {left, top, std::max(0.0f, right - left), std::max(0.0f, bottom - top)};
}

inline std::vector<std::string> wrap_text(const std::string& text,
                                          std::size_t maximum_columns) {
    maximum_columns = std::max<std::size_t>(1U, maximum_columns);
    std::vector<std::string> lines(1U);
    std::size_t line_length = 0U;
    std::size_t position = 0U;
    while (position < text.size()) {
        if (text[position] == '\n') {
            lines.emplace_back();
            line_length = 0U;
            ++position;
            continue;
        }
        if (text[position] == ' ') {
            ++position;
            continue;
        }
        std::size_t word_end = position;
        while (word_end < text.size() && text[word_end] != ' ' &&
               text[word_end] != '\n') {
            ++word_end;
        }
        const std::string word = text.substr(position, word_end - position);
        if (line_length != 0U && line_length + 1U + word.size() <= maximum_columns) {
            lines.back().push_back(' ');
            lines.back() += word;
            line_length += 1U + word.size();
        } else {
            if (line_length != 0U) {
                lines.emplace_back();
                line_length = 0U;
            }
            std::size_t word_position = 0U;
            while (word_position < word.size()) {
                const std::size_t available = maximum_columns - line_length;
                const std::size_t count =
                    std::min(available, word.size() - word_position);
                lines.back().append(word, word_position, count);
                line_length += count;
                word_position += count;
                if (word_position < word.size()) {
                    lines.emplace_back();
                    line_length = 0U;
                }
            }
        }
        position = word_end;
    }
    return lines;
}

inline std::size_t wrap_capacity(float maximum_width, int scale) {
    const float glyph_width = static_cast<float>(8 * std::max(1, scale));
    if (maximum_width < glyph_width) {
        return 1U;
    }
    return static_cast<std::size_t>(maximum_width / glyph_width);
}

inline std::uint32_t blend(std::uint32_t destination, std::uint32_t source) {
    const std::uint32_t alpha = source >> 24U;
    const std::uint32_t inverse = 255U - alpha;
    const std::uint32_t dst_alpha = destination >> 24U;
    const std::uint32_t out_alpha = alpha + (dst_alpha * inverse + 127U) / 255U;
    if (out_alpha == 0U) {
        return 0U;
    }
    const auto channel = [alpha, inverse, dst_alpha, out_alpha, destination, source](
                             unsigned shift) {
        const std::uint32_t src = (source >> shift) & 0xFFU;
        const std::uint32_t dst = (destination >> shift) & 0xFFU;
        const std::uint32_t premultiplied =
            src * alpha + (dst * dst_alpha * inverse + 127U) / 255U;
        return ((premultiplied + out_alpha / 2U) / out_alpha) & 0xFFU;
    };
    return (out_alpha << 24U) | (channel(16U) << 16U) | (channel(8U) << 8U) |
           channel(0U);
}

}  // namespace detail

// A CPU RGBA surface; pixels are packed as 0xAABBGGRR.
class Surface {
public:
    // Creates a transparent surface with non-negative dimensions.
    Surface(int width, int height)
        : width_(std::max(0, width)), height_(std::max(0, height)),
          pixels_(static_cast<std::size_t>(width_) * static_cast<std::size_t>(height_), 0U) {
        clips_.push_back({0.0f, 0.0f, static_cast<float>(width_),
                          static_cast<float>(height_)});
    }

    // Returns the surface width in pixels.
    int width() const { return width_; }
    // Returns the surface height in pixels.
    int height() const { return height_; }
    // Returns a read-only pointer to width times height packed pixels.
    const std::uint32_t* pixels() const { return pixels_.data(); }
    // Returns a writable pointer to width times height packed pixels.
    std::uint32_t* pixels() { return pixels_.data(); }

    // Pushes a clip rectangle intersected with the current clip.
    void push_clip(Rect rect) {
        clips_.push_back(detail::intersect(clips_.back(), rect));
    }
    // Removes the most recently pushed clip, preserving the surface clip.
    void pop_clip() {
        if (clips_.size() > 1U) {
            clips_.pop_back();
        }
    }
    // Fills a rectangle using source-over alpha blending.
    void fill_rect(Rect rect, std::uint32_t color) {
        const Rect clipped = detail::intersect(clips_.back(), rect);
        const int left = std::max(0, static_cast<int>(std::floor(clipped.x)));
        const int top = std::max(0, static_cast<int>(std::floor(clipped.y)));
        const int right = std::min(width_, static_cast<int>(std::ceil(clipped.x + clipped.w)));
        const int bottom = std::min(height_, static_cast<int>(std::ceil(clipped.y + clipped.h)));
        for (int y = top; y < bottom; ++y) {
            for (int x = left; x < right; ++x) {
                blend_pixel(x, y, color);
            }
        }
    }
    // Draws a left-to-right packed-color gradient using source-over blending.
    void draw_horizontal_gradient(Rect rect, std::uint32_t left_color,
                                  std::uint32_t right_color) {
        draw_gradient(rect, left_color, right_color, false);
    }
    // Draws a top-to-bottom packed-color gradient using source-over blending.
    void draw_vertical_gradient(Rect rect, std::uint32_t top_color,
                                std::uint32_t bottom_color) {
        draw_gradient(rect, top_color, bottom_color, true);
    }
    // Draws a four-corner packed-color gradient using source-over blending.
    void draw_bilinear_gradient(Rect rect, std::uint32_t top_left,
                                std::uint32_t top_right,
                                std::uint32_t bottom_left,
                                std::uint32_t bottom_right) {
        if (rect.w <= 0.0f || rect.h <= 0.0f) {
            return;
        }
        const Rect clipped = detail::intersect(clips_.back(), rect);
        const int left = std::max(0, static_cast<int>(std::floor(clipped.x)));
        const int top = std::max(0, static_cast<int>(std::floor(clipped.y)));
        const int right = std::min(
            width_, static_cast<int>(std::ceil(clipped.x + clipped.w)));
        const int bottom = std::min(
            height_, static_cast<int>(std::ceil(clipped.y + clipped.h)));
        const double raster_left = std::floor(static_cast<double>(rect.x));
        const double raster_top = std::floor(static_cast<double>(rect.y));
        const double horizontal_steps = std::max(
            0.0, std::ceil(static_cast<double>(rect.x) + rect.w) -
                     raster_left - 1.0);
        const double vertical_steps = std::max(
            0.0, std::ceil(static_cast<double>(rect.y) + rect.h) -
                     raster_top - 1.0);
        for (int y = top; y < bottom; ++y) {
            const double vertical_position =
                vertical_steps == 0.0
                    ? 0.0
                    : std::clamp(static_cast<double>(y) - raster_top, 0.0,
                                 vertical_steps) /
                          vertical_steps;
            for (int x = left; x < right; ++x) {
                const double horizontal_position =
                    horizontal_steps == 0.0
                        ? 0.0
                        : std::clamp(static_cast<double>(x) - raster_left, 0.0,
                                     horizontal_steps) /
                              horizontal_steps;
                std::uint32_t color = 0U;
                for (unsigned int shift = 0U; shift < 32U; shift += 8U) {
                    const double first_row =
                        static_cast<double>((top_left >> shift) & 0xFFU) *
                            (1.0 - horizontal_position) +
                        static_cast<double>((top_right >> shift) & 0xFFU) *
                            horizontal_position;
                    const double second_row =
                        static_cast<double>((bottom_left >> shift) & 0xFFU) *
                            (1.0 - horizontal_position) +
                        static_cast<double>((bottom_right >> shift) & 0xFFU) *
                            horizontal_position;
                    const double channel =
                        first_row * (1.0 - vertical_position) +
                        second_row * vertical_position;
                    color |= static_cast<std::uint32_t>(std::lround(channel))
                             << shift;
                }
                blend_pixel(x, y, color);
            }
        }
    }
    // Draws a clipped alternating square pattern using source-over blending.
    void draw_checkerboard(Rect rect, std::uint32_t first_color,
                           std::uint32_t second_color, int cell_size = 8) {
        if (rect.w <= 0.0f || rect.h <= 0.0f) {
            return;
        }
        const Rect clipped = detail::intersect(clips_.back(), rect);
        const int left = std::max(0, static_cast<int>(std::floor(clipped.x)));
        const int top = std::max(0, static_cast<int>(std::floor(clipped.y)));
        const int right = std::min(
            width_, static_cast<int>(std::ceil(clipped.x + clipped.w)));
        const int bottom = std::min(
            height_, static_cast<int>(std::ceil(clipped.y + clipped.h)));
        const double cell = static_cast<double>(std::max(1, cell_size));
        for (int y = top; y < bottom; ++y) {
            const double cell_y = std::floor(
                (static_cast<double>(y) - rect.y) / cell);
            const bool alternate_y = std::fmod(cell_y, 2.0) != 0.0;
            for (int x = left; x < right; ++x) {
                const double cell_x = std::floor(
                    (static_cast<double>(x) - rect.x) / cell);
                const bool alternate_x = std::fmod(cell_x, 2.0) != 0.0;
                blend_pixel(x, y, alternate_x != alternate_y
                                      ? second_color
                                      : first_color);
            }
        }
    }
    // Draws a filled circle centered on an integer pixel coordinate.
    void draw_filled_circle(int center_x, int center_y, int radius,
                            std::uint32_t color) {
        if (radius < 0) {
            return;
        }
        const std::int64_t left_bound =
            static_cast<std::int64_t>(center_x) - radius;
        const std::int64_t right_bound =
            static_cast<std::int64_t>(center_x) + radius;
        const std::int64_t top_bound =
            static_cast<std::int64_t>(center_y) - radius;
        const std::int64_t bottom_bound =
            static_cast<std::int64_t>(center_y) + radius;
        const int left = static_cast<int>(std::max<std::int64_t>(0, left_bound));
        const int right = static_cast<int>(std::min<std::int64_t>(
            static_cast<std::int64_t>(width_) - 1, right_bound));
        const int top = static_cast<int>(std::max<std::int64_t>(0, top_bound));
        const int bottom = static_cast<int>(std::min<std::int64_t>(
            static_cast<std::int64_t>(height_) - 1, bottom_bound));
        const double radius_squared =
            static_cast<double>(radius) * static_cast<double>(radius);
        for (int y = top; y <= bottom; ++y) {
            for (int x = left; x <= right; ++x) {
                if (!inside_clip(x, y)) {
                    continue;
                }
                const double dx = static_cast<double>(x) - center_x;
                const double dy = static_cast<double>(y) - center_y;
                if (dx * dx + dy * dy <= radius_squared) {
                    blend_pixel(x, y, color);
                }
            }
        }
    }
    // Draws a filled ellipse inside a surface-coordinate rectangle.
    void draw_filled_ellipse(Rect rect, std::uint32_t color) {
        if (!std::isfinite(rect.x) || !std::isfinite(rect.y) ||
            !std::isfinite(rect.w) || !std::isfinite(rect.h) ||
            rect.w <= 0.0f || rect.h <= 0.0f) {
            return;
        }
        const Rect clipped = detail::intersect(clips_.back(), rect);
        if (clipped.w <= 0.0f || clipped.h <= 0.0f) {
            return;
        }
        const int left = std::max(0, static_cast<int>(std::floor(clipped.x)));
        const int top = std::max(0, static_cast<int>(std::floor(clipped.y)));
        const int right = std::min(
            width_, static_cast<int>(std::ceil(clipped.x + clipped.w)));
        const int bottom = std::min(
            height_, static_cast<int>(std::ceil(clipped.y + clipped.h)));
        const double center_x =
            static_cast<double>(rect.x) + static_cast<double>(rect.w) * 0.5;
        const double center_y =
            static_cast<double>(rect.y) + static_cast<double>(rect.h) * 0.5;
        const double radius_x = static_cast<double>(rect.w) * 0.5;
        const double radius_y = static_cast<double>(rect.h) * 0.5;
        for (int y = top; y < bottom; ++y) {
            for (int x = left; x < right; ++x) {
                const double normalized_x =
                    (static_cast<double>(x) + 0.5 - center_x) / radius_x;
                const double normalized_y =
                    (static_cast<double>(y) + 0.5 - center_y) / radius_y;
                if (normalized_x * normalized_x +
                        normalized_y * normalized_y <=
                    1.0) {
                    blend_pixel(x, y, color);
                }
            }
        }
    }
    // Draws a filled triangle using pixel-center edge tests.
    void draw_filled_triangle(float x0, float y0, float x1, float y1,
                              float x2, float y2, std::uint32_t color) {
        if (!std::isfinite(x0) || !std::isfinite(y0) ||
            !std::isfinite(x1) || !std::isfinite(y1) ||
            !std::isfinite(x2) || !std::isfinite(y2)) {
            return;
        }
        const double ax = x0;
        const double ay = y0;
        const double bx = x1;
        const double by = y1;
        const double cx = x2;
        const double cy = y2;
        const double area = (bx - ax) * (cy - ay) -
                            (by - ay) * (cx - ax);
        if (area == 0.0) {
            return;
        }
        const Rect& clip = clips_.back();
        const double left_bound = std::max(
            0.0, std::max(static_cast<double>(std::min({x0, x1, x2})),
                          static_cast<double>(clip.x)));
        const double top_bound = std::max(
            0.0, std::max(static_cast<double>(std::min({y0, y1, y2})),
                          static_cast<double>(clip.y)));
        const double right_bound = std::min(
            static_cast<double>(width_),
            std::min(static_cast<double>(std::max({x0, x1, x2})),
                     static_cast<double>(clip.x) + clip.w));
        const double bottom_bound = std::min(
            static_cast<double>(height_),
            std::min(static_cast<double>(std::max({y0, y1, y2})),
                     static_cast<double>(clip.y) + clip.h));
        if (right_bound <= left_bound || bottom_bound <= top_bound) {
            return;
        }
        const int left = static_cast<int>(std::floor(left_bound));
        const int top = static_cast<int>(std::floor(top_bound));
        const int right = static_cast<int>(std::ceil(right_bound));
        const int bottom = static_cast<int>(std::ceil(bottom_bound));
        const auto edge = [](double x_a, double y_a, double x_b, double y_b,
                             double x, double y) {
            return (x_b - x_a) * (y - y_a) - (y_b - y_a) * (x - x_a);
        };
        for (int y = top; y < bottom; ++y) {
            for (int x = left; x < right; ++x) {
                if (!inside_clip(x, y)) {
                    continue;
                }
                const double pixel_x = static_cast<double>(x) + 0.5;
                const double pixel_y = static_cast<double>(y) + 0.5;
                const double edge0 = edge(ax, ay, bx, by, pixel_x, pixel_y);
                const double edge1 = edge(bx, by, cx, cy, pixel_x, pixel_y);
                const double edge2 = edge(cx, cy, ax, ay, pixel_x, pixel_y);
                if ((area > 0.0 && edge0 >= 0.0 && edge1 >= 0.0 &&
                     edge2 >= 0.0) ||
                    (area < 0.0 && edge0 <= 0.0 && edge1 <= 0.0 &&
                     edge2 <= 0.0)) {
                    blend_pixel(x, y, color);
                }
            }
        }
    }
    // Fills a polygon from interleaved x/y coordinates using even-odd coverage.
    void draw_filled_polygon(const float* vertices, std::size_t vertex_count,
                             std::uint32_t color) {
        if (vertices == nullptr || vertex_count < 3U ||
            vertex_count > std::numeric_limits<std::size_t>::max() / 2U) {
            return;
        }
        double minimum_x = std::numeric_limits<double>::infinity();
        double minimum_y = std::numeric_limits<double>::infinity();
        double maximum_x = -std::numeric_limits<double>::infinity();
        double maximum_y = -std::numeric_limits<double>::infinity();
        for (std::size_t index = 0U; index < vertex_count; ++index) {
            const float vertex_x = vertices[index * 2U];
            const float vertex_y = vertices[index * 2U + 1U];
            if (!std::isfinite(vertex_x) || !std::isfinite(vertex_y)) {
                return;
            }
            minimum_x = std::min(minimum_x, static_cast<double>(vertex_x));
            minimum_y = std::min(minimum_y, static_cast<double>(vertex_y));
            maximum_x = std::max(maximum_x, static_cast<double>(vertex_x));
            maximum_y = std::max(maximum_y, static_cast<double>(vertex_y));
        }
        const Rect& clip = clips_.back();
        if (!std::isfinite(clip.x) || !std::isfinite(clip.y) ||
            !std::isfinite(clip.w) || !std::isfinite(clip.h) ||
            clip.w <= 0.0f || clip.h <= 0.0f) {
            return;
        }
        const double left_bound =
            std::max({0.0, minimum_x, static_cast<double>(clip.x)});
        const double top_bound =
            std::max({0.0, minimum_y, static_cast<double>(clip.y)});
        const double right_bound = std::min(
            {static_cast<double>(width_), maximum_x,
             static_cast<double>(clip.x) + clip.w});
        const double bottom_bound = std::min(
            {static_cast<double>(height_), maximum_y,
             static_cast<double>(clip.y) + clip.h});
        if (right_bound <= left_bound || bottom_bound <= top_bound) {
            return;
        }
        const int left = static_cast<int>(std::floor(left_bound));
        const int top = static_cast<int>(std::floor(top_bound));
        const int right = static_cast<int>(std::ceil(right_bound));
        const int bottom = static_cast<int>(std::ceil(bottom_bound));
        for (int y = top; y < bottom; ++y) {
            for (int x = left; x < right; ++x) {
                if (!inside_clip(x, y)) {
                    continue;
                }
                const double pixel_x = static_cast<double>(x) + 0.5;
                const double pixel_y = static_cast<double>(y) + 0.5;
                bool inside = false;
                bool on_boundary = false;
                for (std::size_t index = 0U; index < vertex_count; ++index) {
                    const std::size_t next =
                        index + 1U == vertex_count ? 0U : index + 1U;
                    const double start_x = vertices[index * 2U];
                    const double start_y = vertices[index * 2U + 1U];
                    const double end_x = vertices[next * 2U];
                    const double end_y = vertices[next * 2U + 1U];
                    const double cross =
                        (pixel_x - start_x) * (end_y - start_y) -
                        (pixel_y - start_y) * (end_x - start_x);
                    if (cross == 0.0 &&
                        pixel_x >= std::min(start_x, end_x) &&
                        pixel_x <= std::max(start_x, end_x) &&
                        pixel_y >= std::min(start_y, end_y) &&
                        pixel_y <= std::max(start_y, end_y)) {
                        on_boundary = true;
                        break;
                    }
                    if ((start_y > pixel_y) != (end_y > pixel_y)) {
                        const double intersection_x =
                            start_x + (pixel_y - start_y) *
                                          (end_x - start_x) /
                                          (end_y - start_y);
                        if (pixel_x < intersection_x) {
                            inside = !inside;
                        }
                    }
                }
                if (inside || on_boundary) {
                    blend_pixel(x, y, color);
                }
            }
        }
    }
    // Draws a closed polygon outline from interleaved x/y coordinates.
    void draw_polygon_outline(const float* vertices, std::size_t vertex_count,
                              std::uint32_t color, float thickness = 1.0f) {
        draw_path_outline(vertices, vertex_count, color, thickness, true);
    }
    // Draws an open polyline from interleaved x/y coordinates.
    void draw_polyline(const float* vertices, std::size_t vertex_count,
                       std::uint32_t color, float thickness = 1.0f) {
        draw_path_outline(vertices, vertex_count, color, thickness, false);
    }
    // Draws a cubic Bézier curve using a fixed number of polyline segments.
    void draw_cubic_bezier(float x0, float y0, float x1, float y1, float x2,
                           float y2, float x3, float y3,
                           std::uint32_t color, float thickness = 1.0f,
                           std::size_t segments = 32U) {
        if (!std::isfinite(x0) || !std::isfinite(y0) ||
            !std::isfinite(x1) || !std::isfinite(y1) ||
            !std::isfinite(x2) || !std::isfinite(y2) ||
            !std::isfinite(x3) || !std::isfinite(y3) ||
            !std::isfinite(thickness) || thickness <= 0.0f ||
            segments == 0U ||
            segments > (std::numeric_limits<std::size_t>::max() / 2U) - 1U) {
            return;
        }
        std::vector<float> vertices((segments + 1U) * 2U);
        const double start_x = x0;
        const double start_y = y0;
        const double control1_x = x1;
        const double control1_y = y1;
        const double control2_x = x2;
        const double control2_y = y2;
        const double end_x = x3;
        const double end_y = y3;
        for (std::size_t index = 0U; index <= segments; ++index) {
            const double t = static_cast<double>(index) /
                             static_cast<double>(segments);
            const double inverse_t = 1.0 - t;
            const double start_weight = inverse_t * inverse_t * inverse_t;
            const double control1_weight =
                3.0 * inverse_t * inverse_t * t;
            const double control2_weight = 3.0 * inverse_t * t * t;
            const double end_weight = t * t * t;
            vertices[index * 2U] = static_cast<float>(
                start_weight * start_x + control1_weight * control1_x +
                control2_weight * control2_x + end_weight * end_x);
            vertices[index * 2U + 1U] = static_cast<float>(
                start_weight * start_y + control1_weight * control1_y +
                control2_weight * control2_y + end_weight * end_y);
        }
        draw_polyline(vertices.data(), segments + 1U, color, thickness);
    }
    // Draws a triangle outline with pixel-center sampling and the given thickness.
    void draw_triangle_outline(float x0, float y0, float x1, float y1,
                               float x2, float y2, std::uint32_t color,
                               float thickness = 1.0f) {
        const std::array<float, 6> vertices{{x0, y0, x1, y1, x2, y2}};
        draw_polygon_outline(vertices.data(), 3U, color, thickness);
    }
    // Draws an ellipse border by insetting its bounding rectangle by thickness.
    void draw_ellipse_outline(Rect rect, std::uint32_t color,
                              float thickness = 1.0f) {
        if (!std::isfinite(rect.x) || !std::isfinite(rect.y) ||
            !std::isfinite(rect.w) || !std::isfinite(rect.h) ||
            !std::isfinite(thickness) || rect.w <= 0.0f || rect.h <= 0.0f ||
            thickness <= 0.0f) {
            return;
        }
        const Rect clipped = detail::intersect(clips_.back(), rect);
        if (clipped.w <= 0.0f || clipped.h <= 0.0f) {
            return;
        }
        const int left = std::max(0, static_cast<int>(std::floor(clipped.x)));
        const int top = std::max(0, static_cast<int>(std::floor(clipped.y)));
        const int right = std::min(
            width_, static_cast<int>(std::ceil(clipped.x + clipped.w)));
        const int bottom = std::min(
            height_, static_cast<int>(std::ceil(clipped.y + clipped.h)));
        const double center_x =
            static_cast<double>(rect.x) + static_cast<double>(rect.w) * 0.5;
        const double center_y =
            static_cast<double>(rect.y) + static_cast<double>(rect.h) * 0.5;
        const double outer_radius_x = static_cast<double>(rect.w) * 0.5;
        const double outer_radius_y = static_cast<double>(rect.h) * 0.5;
        const Rect inner{
            rect.x + thickness, rect.y + thickness,
            rect.w - thickness * 2.0f, rect.h - thickness * 2.0f};
        const bool has_inner = inner.w > 0.0f && inner.h > 0.0f;
        const double inner_radius_x = static_cast<double>(inner.w) * 0.5;
        const double inner_radius_y = static_cast<double>(inner.h) * 0.5;
        for (int y = top; y < bottom; ++y) {
            for (int x = left; x < right; ++x) {
                const double pixel_x = static_cast<double>(x) + 0.5;
                const double pixel_y = static_cast<double>(y) + 0.5;
                const double outer_x = (pixel_x - center_x) / outer_radius_x;
                const double outer_y = (pixel_y - center_y) / outer_radius_y;
                const bool inside_outer =
                    outer_x * outer_x + outer_y * outer_y <= 1.0;
                bool inside_inner = false;
                if (has_inner) {
                    const double inner_x =
                        (pixel_x - center_x) / inner_radius_x;
                    const double inner_y =
                        (pixel_y - center_y) / inner_radius_y;
                    inside_inner =
                        inner_x * inner_x + inner_y * inner_y <= 1.0;
                }
                if (inside_outer && !inside_inner) {
                    blend_pixel(x, y, color);
                }
            }
        }
    }
    // Draws an integer-coordinate circle outline with the requested pixel thickness.
    void draw_circle_outline(int center_x, int center_y, int radius,
                             std::uint32_t color, int thickness = 1) {
        if (radius < 0 || thickness <= 0) {
            return;
        }
        if (radius == 0) {
            draw_filled_circle(center_x, center_y, 0, color);
            return;
        }
        if (thickness >= radius) {
            draw_filled_circle(center_x, center_y, radius, color);
            return;
        }
        const std::int64_t left_bound =
            static_cast<std::int64_t>(center_x) - radius;
        const std::int64_t right_bound =
            static_cast<std::int64_t>(center_x) + radius;
        const std::int64_t top_bound =
            static_cast<std::int64_t>(center_y) - radius;
        const std::int64_t bottom_bound =
            static_cast<std::int64_t>(center_y) + radius;
        const int left = static_cast<int>(std::max<std::int64_t>(0, left_bound));
        const int right = static_cast<int>(std::min<std::int64_t>(
            static_cast<std::int64_t>(width_) - 1, right_bound));
        const int top = static_cast<int>(std::max<std::int64_t>(0, top_bound));
        const int bottom = static_cast<int>(std::min<std::int64_t>(
            static_cast<std::int64_t>(height_) - 1, bottom_bound));
        const std::int64_t inner_radius =
            std::max(0, radius - thickness);
        const std::int64_t outer_squared =
            static_cast<std::int64_t>(radius) * radius;
        const std::int64_t inner_squared = inner_radius * inner_radius;
        for (int y = top; y <= bottom; ++y) {
            for (int x = left; x <= right; ++x) {
                if (!inside_clip(x, y)) {
                    continue;
                }
                const std::int64_t dx =
                    static_cast<std::int64_t>(x) - center_x;
                const std::int64_t dy =
                    static_cast<std::int64_t>(y) - center_y;
                const std::int64_t distance_squared = dx * dx + dy * dy;
                if (distance_squared <= outer_squared &&
                    distance_squared > inner_squared) {
                    blend_pixel(x, y, color);
                }
            }
        }
    }
    // Draws a filled rounded rectangle with its radius clamped to half its shorter side.
    void draw_rounded_rect(Rect rect, float radius, std::uint32_t color) {
        if (rect.w <= 0.0f || rect.h <= 0.0f) {
            return;
        }
        const float maximum_radius = std::min(rect.w, rect.h) * 0.5f;
        const float corner_radius = std::min(std::max(0.0f, radius), maximum_radius);
        if (corner_radius == 0.0f) {
            fill_rect(rect, color);
            return;
        }
        const Rect clipped = detail::intersect(clips_.back(), rect);
        const int left = std::max(0, static_cast<int>(std::floor(clipped.x)));
        const int top = std::max(0, static_cast<int>(std::floor(clipped.y)));
        const int right = std::min(width_, static_cast<int>(std::ceil(clipped.x + clipped.w)));
        const int bottom = std::min(height_, static_cast<int>(std::ceil(clipped.y + clipped.h)));
        const double radius_squared =
            static_cast<double>(corner_radius) * static_cast<double>(corner_radius);
        const double left_center = static_cast<double>(rect.x + corner_radius);
        const double right_center =
            static_cast<double>(rect.x + rect.w - corner_radius);
        const double top_center = static_cast<double>(rect.y + corner_radius);
        const double bottom_center =
            static_cast<double>(rect.y + rect.h - corner_radius);
        for (int y = top; y < bottom; ++y) {
            for (int x = left; x < right; ++x) {
                const double pixel_x = static_cast<double>(x) + 0.5;
                const double pixel_y = static_cast<double>(y) + 0.5;
                const double nearest_x = std::max(left_center,
                                                  std::min(pixel_x, right_center));
                const double nearest_y = std::max(top_center,
                                                  std::min(pixel_y, bottom_center));
                const double dx = pixel_x - nearest_x;
                const double dy = pixel_y - nearest_y;
                if (dx * dx + dy * dy <= radius_squared) {
                    blend_pixel(x, y, color);
                }
            }
        }
    }
    // Draws a rounded rectangle outline with an inward border thickness.
    void draw_rounded_rect_outline(Rect rect, float radius,
                                   std::uint32_t color,
                                   float thickness = 1.0f) {
        if (!std::isfinite(rect.x) || !std::isfinite(rect.y) ||
            !std::isfinite(rect.w) || !std::isfinite(rect.h) ||
            !std::isfinite(radius) || !std::isfinite(thickness) ||
            rect.w <= 0.0f || rect.h <= 0.0f || thickness <= 0.0f) {
            return;
        }
        if (radius <= 0.0f) {
            outline_rect(rect, color, thickness);
            return;
        }
        const Rect clipped = detail::intersect(clips_.back(), rect);
        if (clipped.w <= 0.0f || clipped.h <= 0.0f) {
            return;
        }
        const int left = std::max(0, static_cast<int>(std::floor(clipped.x)));
        const int top = std::max(0, static_cast<int>(std::floor(clipped.y)));
        const int right = std::min(
            width_, static_cast<int>(std::ceil(clipped.x + clipped.w)));
        const int bottom = std::min(
            height_, static_cast<int>(std::ceil(clipped.y + clipped.h)));
        const double outer_x = static_cast<double>(rect.x);
        const double outer_y = static_cast<double>(rect.y);
        const double outer_right = outer_x + static_cast<double>(rect.w);
        const double outer_bottom = outer_y + static_cast<double>(rect.h);
        const double outer_radius = std::min(
            static_cast<double>(radius),
            std::min(static_cast<double>(rect.w),
                     static_cast<double>(rect.h)) * 0.5);
        const double border = static_cast<double>(thickness);
        const double inner_x = outer_x + border;
        const double inner_y = outer_y + border;
        const double inner_right = outer_right - border;
        const double inner_bottom = outer_bottom - border;
        const bool has_inner = inner_right > inner_x && inner_bottom > inner_y;
        const double inner_radius = has_inner
            ? std::min(std::max(0.0, outer_radius - border),
                       std::min(inner_right - inner_x,
                                inner_bottom - inner_y) * 0.5)
            : 0.0;
        for (int y = top; y < bottom; ++y) {
            for (int x = left; x < right; ++x) {
                const double pixel_x = static_cast<double>(x) + 0.5;
                const double pixel_y = static_cast<double>(y) + 0.5;
                const double outer_nearest_x = std::max(
                    outer_x + outer_radius,
                    std::min(pixel_x, outer_right - outer_radius));
                const double outer_nearest_y = std::max(
                    outer_y + outer_radius,
                    std::min(pixel_y, outer_bottom - outer_radius));
                const double outer_dx = pixel_x - outer_nearest_x;
                const double outer_dy = pixel_y - outer_nearest_y;
                const bool inside_outer =
                    outer_dx * outer_dx + outer_dy * outer_dy <=
                    outer_radius * outer_radius;
                bool inside_inner = false;
                if (has_inner) {
                    const double inner_nearest_x = std::max(
                        inner_x + inner_radius,
                        std::min(pixel_x, inner_right - inner_radius));
                    const double inner_nearest_y = std::max(
                        inner_y + inner_radius,
                        std::min(pixel_y, inner_bottom - inner_radius));
                    const double inner_dx = pixel_x - inner_nearest_x;
                    const double inner_dy = pixel_y - inner_nearest_y;
                    inside_inner =
                        inner_dx * inner_dx + inner_dy * inner_dy <=
                        inner_radius * inner_radius;
                }
                if (inside_outer && !inside_inner) {
                    blend_pixel(x, y, color);
                }
            }
        }
    }
    // Draws a rectangle outline using the requested non-overlapping border thickness.
    void outline_rect(Rect rect, std::uint32_t color, float thickness = 1.0f) {
        if (rect.w <= 0.0f || rect.h <= 0.0f || thickness <= 0.0f) {
            return;
        }
        const float border = std::min(thickness, std::min(rect.w, rect.h) * 0.5f);
        fill_rect({rect.x, rect.y, rect.w, border}, color);
        fill_rect({rect.x, rect.y + rect.h - border, rect.w, border}, color);
        const float inner_height = rect.h - 2.0f * border;
        if (inner_height > 0.0f) {
            fill_rect({rect.x, rect.y + border, border, inner_height}, color);
            fill_rect({rect.x + rect.w - border, rect.y + border, border,
                       inner_height}, color);
        }
    }
    // Blits a pixel buffer with nearest-neighbor scaling and source-over blending.
    void draw_image(const std::uint32_t* source, int source_width,
                    int source_height, Rect rect) {
        draw_tinted_image(source, source_width, source_height, rect,
                          0xFFFFFFFFU);
    }
    // Blits a nearest-neighbor image after multiplying its packed RGBA channels by a tint.
    void draw_tinted_image(const std::uint32_t* source, int source_width,
                           int source_height, Rect rect,
                           std::uint32_t tint) {
        if (source == nullptr || source_width <= 0 || source_height <= 0 ||
            rect.w <= 0.0f || rect.h <= 0.0f) {
            return;
        }
        const Rect clipped = detail::intersect(clips_.back(), rect);
        const int left = std::max(0, static_cast<int>(std::floor(clipped.x)));
        const int top = std::max(0, static_cast<int>(std::floor(clipped.y)));
        const int right = std::min(width_, static_cast<int>(std::ceil(clipped.x + clipped.w)));
        const int bottom = std::min(height_, static_cast<int>(std::ceil(clipped.y + clipped.h)));
        for (int y = top; y < bottom; ++y) {
            const double source_y =
                (static_cast<double>(y) + 0.5 - rect.y) * source_height / rect.h;
            const int sample_y = std::max(
                0, std::min(source_height - 1, static_cast<int>(std::floor(source_y))));
            for (int x = left; x < right; ++x) {
                const double source_x =
                    (static_cast<double>(x) + 0.5 - rect.x) * source_width / rect.w;
                const int sample_x = std::max(
                    0, std::min(source_width - 1,
                                static_cast<int>(std::floor(source_x))));
                const std::size_t source_index =
                    static_cast<std::size_t>(sample_y) *
                        static_cast<std::size_t>(source_width) +
                    static_cast<std::size_t>(sample_x);
                const std::uint32_t source_pixel = source[source_index];
                std::uint32_t tinted_pixel = 0U;
                for (unsigned int shift = 0U; shift < 32U; shift += 8U) {
                    const std::uint32_t source_channel =
                        (source_pixel >> shift) & 0xFFU;
                    const std::uint32_t tint_channel =
                        (tint >> shift) & 0xFFU;
                    const std::uint32_t tinted_channel =
                        (source_channel * tint_channel + 127U) / 255U;
                    tinted_pixel |= tinted_channel << shift;
                }
                blend_pixel(x, y, tinted_pixel);
            }
        }
    }
    // Draws a one-pixel Bresenham line using source-over blending.
    void draw_line(int x0, int y0, int x1, int y1, std::uint32_t color) {
        const int dx = std::abs(x1 - x0);
        const int sx = x0 < x1 ? 1 : -1;
        const int dy = -std::abs(y1 - y0);
        const int sy = y0 < y1 ? 1 : -1;
        int error = dx + dy;
        for (;;) {
            if (inside_clip(x0, y0)) {
                blend_pixel(x0, y0, color);
            }
            if (x0 == x1 && y0 == y1) {
                break;
            }
            const int twice_error = 2 * error;
            if (twice_error >= dy) {
                error += dy;
                x0 += sx;
            }
            if (twice_error <= dx) {
                error += dx;
                y0 += sy;
            }
        }
    }
    // Draws a Bresenham line in repeating dash and gap lengths.
    void draw_dashed_line(int x0, int y0, int x1, int y1,
                          std::uint32_t color, int dash_length = 4,
                          int gap_length = 4) {
        if (dash_length <= 0) {
            return;
        }
        const std::int64_t gap =
            static_cast<std::int64_t>(std::max(0, gap_length));
        const std::int64_t period =
            static_cast<std::int64_t>(dash_length) + gap;
        const int dx = std::abs(x1 - x0);
        const int sx = x0 < x1 ? 1 : -1;
        const int dy = -std::abs(y1 - y0);
        const int sy = y0 < y1 ? 1 : -1;
        int error = dx + dy;
        std::int64_t step = 0;
        for (;;) {
            if (step % period < dash_length && inside_clip(x0, y0)) {
                blend_pixel(x0, y0, color);
            }
            if (x0 == x1 && y0 == y1) {
                break;
            }
            const int twice_error = 2 * error;
            if (twice_error >= dy) {
                error += dy;
                x0 += sx;
            }
            if (twice_error <= dx) {
                error += dx;
                y0 += sy;
            }
            ++step;
        }
    }
    // Draws ASCII text in the built-in 8-by-16 bitmap font.
    void draw_text(int x, int y, const std::string& text, std::uint32_t color,
                   int scale = 1) {
        const int glyph_scale = std::max(1, scale);
        int pen_x = x;
        for (const unsigned char character : text) {
            if (character == static_cast<unsigned char>('\n')) {
                pen_x = x;
                y += 16 * glyph_scale;
                continue;
            }
            const std::size_t index = character >= 32U && character <= 126U
                                          ? static_cast<std::size_t>(character - 32U)
                                          : static_cast<std::size_t>('?' - ' ');
            for (int row = 0; row < 16; ++row) {
                const std::uint8_t bits = detail::kFont[index][static_cast<std::size_t>(row)];
                for (int column = 0; column < 8; ++column) {
                    if ((bits & static_cast<std::uint8_t>(0x80U >> column)) != 0U) {
                        fill_rect({static_cast<float>(pen_x + column * glyph_scale),
                                   static_cast<float>(y + row * glyph_scale),
                                   static_cast<float>(glyph_scale),
                                   static_cast<float>(glyph_scale)}, color);
                    }
                }
            }
            pen_x += 8 * glyph_scale;
        }
    }
    // Returns text width and height in pixels for the selected font scale.
    Rect measure_text(const std::string& text, int scale = 1) const {
        const int glyph_scale = std::max(1, scale);
        int line_width = 0;
        int widest = 0;
        int lines = 1;
        for (const char character : text) {
            if (character == '\n') {
                widest = std::max(widest, line_width);
                line_width = 0;
                ++lines;
            } else {
                line_width += 8 * glyph_scale;
            }
        }
        widest = std::max(widest, line_width);
        return {0.0f, 0.0f, static_cast<float>(widest),
                static_cast<float>(lines * 16 * glyph_scale)};
    }
    // Returns the dimensions of text wrapped to the supplied maximum width.
    Rect measure_text_wrapped(const std::string& text, float maximum_width,
                              int scale = 1) const {
        const int glyph_scale = std::max(1, scale);
        const std::vector<std::string> lines =
            detail::wrap_text(text, detail::wrap_capacity(maximum_width, glyph_scale));
        std::size_t widest_line = 0U;
        for (const std::string& line : lines) {
            widest_line = std::max(widest_line, line.size());
        }
        return {0.0f, 0.0f,
                static_cast<float>(widest_line * static_cast<std::size_t>(8 * glyph_scale)),
                static_cast<float>(lines.size()) * 16.0f *
                    static_cast<float>(glyph_scale)};
    }

private:
    void draw_path_outline(const float* vertices, std::size_t vertex_count,
                           std::uint32_t color, float thickness, bool closed) {
        if (vertices == nullptr || vertex_count < 2U ||
            vertex_count > std::numeric_limits<std::size_t>::max() / 2U ||
            !std::isfinite(thickness) || thickness <= 0.0f) {
            return;
        }
        const double half_thickness = static_cast<double>(thickness) * 0.5;
        double minimum_x = std::numeric_limits<double>::infinity();
        double minimum_y = std::numeric_limits<double>::infinity();
        double maximum_x = -std::numeric_limits<double>::infinity();
        double maximum_y = -std::numeric_limits<double>::infinity();
        for (std::size_t index = 0U; index < vertex_count; ++index) {
            const float vertex_x = vertices[index * 2U];
            const float vertex_y = vertices[index * 2U + 1U];
            if (!std::isfinite(vertex_x) || !std::isfinite(vertex_y)) {
                return;
            }
            minimum_x = std::min(minimum_x, static_cast<double>(vertex_x));
            minimum_y = std::min(minimum_y, static_cast<double>(vertex_y));
            maximum_x = std::max(maximum_x, static_cast<double>(vertex_x));
            maximum_y = std::max(maximum_y, static_cast<double>(vertex_y));
        }
        const Rect& clip = clips_.back();
        if (!std::isfinite(clip.x) || !std::isfinite(clip.y) ||
            !std::isfinite(clip.w) || !std::isfinite(clip.h) ||
            clip.w <= 0.0f || clip.h <= 0.0f) {
            return;
        }
        const double left_bound = std::max(
            {0.0, minimum_x - half_thickness, static_cast<double>(clip.x)});
        const double top_bound = std::max(
            {0.0, minimum_y - half_thickness, static_cast<double>(clip.y)});
        const double right_bound = std::min(
            {static_cast<double>(width_), maximum_x + half_thickness,
             static_cast<double>(clip.x) + clip.w});
        const double bottom_bound = std::min(
            {static_cast<double>(height_), maximum_y + half_thickness,
             static_cast<double>(clip.y) + clip.h});
        if (right_bound <= left_bound || bottom_bound <= top_bound) {
            return;
        }
        const int left = static_cast<int>(std::floor(left_bound));
        const int top = static_cast<int>(std::floor(top_bound));
        const int right = static_cast<int>(std::ceil(right_bound));
        const int bottom = static_cast<int>(std::ceil(bottom_bound));
        const auto distance_squared_to_segment = [](
            double point_x, double point_y, double start_x, double start_y,
            double end_x, double end_y) {
            const double segment_x = end_x - start_x;
            const double segment_y = end_y - start_y;
            const double segment_length_squared =
                segment_x * segment_x + segment_y * segment_y;
            double projection = 0.0;
            if (segment_length_squared > 0.0) {
                projection = ((point_x - start_x) * segment_x +
                              (point_y - start_y) * segment_y) /
                             segment_length_squared;
                projection = std::max(0.0, std::min(1.0, projection));
            }
            const double delta_x =
                point_x - (start_x + projection * segment_x);
            const double delta_y =
                point_y - (start_y + projection * segment_y);
            return delta_x * delta_x + delta_y * delta_y;
        };
        const std::size_t segment_count =
            closed ? vertex_count : vertex_count - 1U;
        const double radius_squared = half_thickness * half_thickness;
        for (int y = top; y < bottom; ++y) {
            for (int x = left; x < right; ++x) {
                if (!inside_clip(x, y)) {
                    continue;
                }
                const double pixel_x = static_cast<double>(x) + 0.5;
                const double pixel_y = static_cast<double>(y) + 0.5;
                bool covered = false;
                for (std::size_t index = 0U; index < segment_count; ++index) {
                    const std::size_t next =
                        index + 1U == vertex_count ? 0U : index + 1U;
                    if (distance_squared_to_segment(
                            pixel_x, pixel_y, vertices[index * 2U],
                            vertices[index * 2U + 1U], vertices[next * 2U],
                            vertices[next * 2U + 1U]) <= radius_squared) {
                        covered = true;
                        break;
                    }
                }
                if (covered) {
                    blend_pixel(x, y, color);
                }
            }
        }
    }

    void draw_gradient(Rect rect, std::uint32_t first_color,
                       std::uint32_t last_color, bool vertical) {
        const float extent = vertical ? rect.h : rect.w;
        if (extent <= 0.0f || rect.w <= 0.0f || rect.h <= 0.0f) {
            return;
        }
        const Rect clipped = detail::intersect(clips_.back(), rect);
        const int left = std::max(0, static_cast<int>(std::floor(clipped.x)));
        const int top = std::max(0, static_cast<int>(std::floor(clipped.y)));
        const int right = std::min(
            width_, static_cast<int>(std::ceil(clipped.x + clipped.w)));
        const int bottom = std::min(
            height_, static_cast<int>(std::ceil(clipped.y + clipped.h)));
        const double raster_start =
            std::floor(static_cast<double>(vertical ? rect.y : rect.x));
        const double raster_end = std::ceil(
            static_cast<double>(vertical ? rect.y + rect.h : rect.x + rect.w));
        const double steps = std::max(0.0, raster_end - raster_start - 1.0);
        for (int y = top; y < bottom; ++y) {
            for (int x = left; x < right; ++x) {
                const double coordinate =
                    static_cast<double>(vertical ? y : x) - raster_start;
                const double position =
                    steps == 0.0 ? 0.0 : std::clamp(coordinate, 0.0, steps);
                std::uint32_t color = 0U;
                for (unsigned int shift = 0U; shift < 32U; shift += 8U) {
                    const int first = static_cast<int>(
                        (first_color >> shift) & 0xFFU);
                    const int last = static_cast<int>(
                        (last_color >> shift) & 0xFFU);
                    const double channel =
                        steps == 0.0
                            ? static_cast<double>(first)
                            : (static_cast<double>(first) * (steps - position) +
                               static_cast<double>(last) * position) /
                                  steps;
                    color |= static_cast<std::uint32_t>(std::lround(channel))
                             << shift;
                }
                blend_pixel(x, y, color);
            }
        }
    }
    bool inside_clip(int x, int y) const {
        const Rect& clip = clips_.back();
        return x >= 0 && y >= 0 && x < width_ && y < height_ &&
               static_cast<float>(x) >= clip.x && static_cast<float>(y) >= clip.y &&
               static_cast<float>(x) < clip.x + clip.w &&
               static_cast<float>(y) < clip.y + clip.h;
    }
    void blend_pixel(int x, int y, std::uint32_t color) {
        const std::size_t index = static_cast<std::size_t>(y) *
                                      static_cast<std::size_t>(width_) +
                                  static_cast<std::size_t>(x);
        pixels_[index] = detail::blend(pixels_[index], color);
    }

    int width_;
    int height_;
    std::vector<std::uint32_t> pixels_;
    std::vector<Rect> clips_;
};

// The customizable colors and spacing used by Context.
struct Theme {
    std::uint32_t background = 0xFF282C34U;
    std::uint32_t window = 0xFF3A3F4BU;
    std::uint32_t title_bar = 0xFF2D6A9FU;
    std::uint32_t text = 0xFFFFFFFFU;
    std::uint32_t accent = 0xFF56B6C2U;
    std::uint32_t button = 0xFF4B5263U;
    std::uint32_t button_hover = 0xFF5C6370U;
    std::uint32_t button_active = 0xFF2D6A9FU;
    std::uint32_t border = 0xFF181A1FU;
    int padding = 8;
    int font_scale = 1;
    std::size_t tooltip_delay_frames = 30U;
};

// Caller-owned position and pointer state for a draggable window.
struct WindowState {
    Rect rect{};
    bool dragging = false;
    float drag_offset_x = 0.0f;
    float drag_offset_y = 0.0f;
};

// Dock group in which a panel initially appears.
enum class DockSide {
    Left,
    Right
};

// Stable identifier, visible title, and initial group for one dock panel.
struct DockPanel {
    std::string id;
    std::string title;
    DockSide side = DockSide::Left;
};

// Named CPU timing interval measured from the beginning of a frame.
struct ProfileEvent {
    std::string name;
    float start_ms = 0.0f;
    float duration_ms = 0.0f;
};

// Caller-owned layout and tab state for a resizable two-group dockspace.
struct DockspaceState {
    std::vector<DockPanel> panels;
    float left_ratio = 0.35f;
    std::string active_left;
    std::string active_right;
    std::string dragging_panel;
    bool dragging_splitter = false;
    Rect rect{};
};

namespace detail {
struct DockspaceGeometry {
    Rect inner;
    Rect left;
    Rect right;
    Rect splitter;
    Rect left_content;
    Rect right_content;
};
}  // namespace detail

// Keyboard actions accepted by feed_key.
enum class Key {
    Backspace,
    Delete,
    Left,
    Right,
    Home,
    End,
    Enter,
    Tab,
    Space,
    Up,
    Down,
    Escape
};

// Result of an immediate-mode widget interaction.
struct Interaction {
    bool hovered = false;
    bool active = false;
    bool clicked = false;
    bool changed = false;

    // Converts to true when this widget was clicked during the current frame.
    explicit operator bool() const { return clicked; }
};

// Immediate-mode GUI context that renders into a CPU pixel buffer.
class Context {
public:
    // Creates a transparent render target and applies the supplied theme.
    Context(int width, int height, Theme theme = {})
        : surface_(width, height), theme_(theme), root_{0.0f, 0.0f,
          static_cast<float>(std::max(0, width)), static_cast<float>(std::max(0, height))},
          cursor_x_(static_cast<float>(theme.padding)),
          cursor_y_(static_cast<float>(theme.padding)),
          content_width_(root_.w - 2.0f * static_cast<float>(theme.padding)) {
        input_clips_.push_back(root_);
    }

    // Begins a frame, clears the surface, and records the current pointer state.
    void begin(int mouse_x, int mouse_y, bool mouse_down) {
        ++frame_index_;
        std::fill(surface_.pixels(), surface_.pixels() +
                  static_cast<std::size_t>(surface_.width()) *
                      static_cast<std::size_t>(surface_.height()), 0U);
        surface_.fill_rect(root_, theme_.background);
        mouse_x_ = mouse_x;
        mouse_y_ = mouse_y;
        mouse_down_ = mouse_down;
        mouse_pressed_ = mouse_down && !previous_mouse_down_;
        mouse_released_ = !mouse_down && previous_mouse_down_;
        hot_id_ = 0U;
        last_widget_id_ = 0U;
        last_widget_hovered_ = false;
        const bool pointer_over_previous_popup =
            contains_point(previous_popup_rect_, mouse_x_, mouse_y_);
        const bool pointer_over_previous_combo =
            contains_point(previous_combo_rect_, mouse_x_, mouse_y_);
        if (open_combo_id_ != 0U && mouse_pressed_ &&
            !pointer_over_previous_popup && !pointer_over_previous_combo) {
            open_combo_id_ = 0U;
        }
        popup_input_blocked_ =
            open_combo_id_ != 0U && pointer_over_previous_popup;
        pointer_claimed_id_ = 0U;
        frame_popup_draws_.clear();
        frame_tooltip_draws_.clear();
        active_popup_rect_ = {};
        active_combo_rect_ = {};
        frame_open_combo_seen_ = false;
        child_wheel_target_ = 0U;
        std::size_t deepest_child = 0U;
        for (const ChildHit& child : previous_child_hits_) {
            if (contains_point(child.rect, mouse_x_, mouse_y_) &&
                contains_point(child.clip, mouse_x_, mouse_y_) &&
                (child_wheel_target_ == 0U || child.depth >= deepest_child)) {
                child_wheel_target_ = child.id;
                deepest_child = child.depth;
            }
        }
        frame_child_hits_.clear();
        if (!mouse_down_ && !mouse_released_) {
            active_id_ = 0U;
        }
        frame_focus_order_.clear();
        cursor_x_ = static_cast<float>(theme_.padding);
        cursor_y_ = static_cast<float>(theme_.padding);
        content_width_ = root_.w - 2.0f * static_cast<float>(theme_.padding);
    }
    // Ends a frame and saves pointer state for edge detection next frame.
    void end() {
        end_dockspace();
        if (open_combo_id_ != 0U && !frame_open_combo_seen_) {
            open_combo_id_ = 0U;
        }
        for (const PopupDraw& popup : frame_popup_draws_) {
            draw_popup(popup);
        }
        for (const TooltipDraw& tooltip : frame_tooltip_draws_) {
            draw_tooltip(tooltip);
        }
        previous_popup_rect_ = active_popup_rect_;
        previous_combo_rect_ = active_combo_rect_;
        previous_mouse_down_ = mouse_down_;
        focus_order_.swap(frame_focus_order_);
        previous_child_hits_.swap(frame_child_hits_);
        if (std::find(focus_order_.begin(), focus_order_.end(), focused_id_) ==
            focus_order_.end()) {
            focused_id_ = 0U;
            focused_text_id_ = 0U;
        }
        pending_navigation_.erase(
            std::remove_if(pending_navigation_.begin(), pending_navigation_.end(),
                           [this](const std::pair<std::uint64_t, Key>& navigation) {
                               return std::find(focus_order_.begin(), focus_order_.end(),
                                                navigation.first) == focus_order_.end();
                           }),
            pending_navigation_.end());
        if (std::find(focus_order_.begin(), focus_order_.end(),
                      keyboard_activation_id_) == focus_order_.end()) {
            keyboard_activation_id_ = 0U;
        }
        if (pending_wheel_delta_ != 0) {
            pending_wheel_delta_ = 0;
            wheel_remainder_ = 0;
        }
    }
    // Returns a read-only pointer to width times height packed pixels.
    const std::uint32_t* pixels() const { return surface_.pixels(); }
    // Returns the render target width.
    int width() const { return surface_.width(); }
    // Returns the render target height.
    int height() const { return surface_.height(); }
    // Returns a const reference to the active theme.
    const Theme& theme() const { return theme_; }
    // Returns the ID currently under the pointer.
    std::uint64_t hot_id() const { return hot_id_; }
    // Returns the ID currently held by the pointer.
    std::uint64_t active_id() const { return active_id_; }
    // Returns the ID currently receiving keyboard focus.
    std::uint64_t focused_id() const { return focused_id_; }
    // Returns the mouse-down state supplied to begin.
    bool mouse_down() const { return mouse_down_; }
    // Returns a child region's vertical scroll offset in the current ID scope.
    float child_scroll_offset(const std::string& id) const {
        const auto state = child_states_.find(widget_id(id));
        return state == child_states_.end() ? 0.0f : state->second.scroll_y;
    }
    // Draws a caller-provided image buffer at the supplied surface-coordinate rectangle.
    void image(const std::uint32_t* pixels, int image_width, int image_height,
               Rect rect) {
        clear_last_widget();
        surface_.draw_image(pixels, image_width, image_height, rect);
    }
    // Draws a caller-owned packed image after multiplying each RGBA channel by a tint.
    void tinted_image(const std::uint32_t* pixels, int image_width,
                      int image_height, Rect rect, std::uint32_t tint) {
        clear_last_widget();
        surface_.draw_tinted_image(pixels, image_width, image_height, rect, tint);
    }
    // Draws another surface at the supplied surface-coordinate rectangle.
    void image(const Surface& source, Rect rect) {
        clear_last_widget();
        surface_.draw_image(source.pixels(), source.width(), source.height(), rect);
    }
    // Draws a left-to-right gradient in a full-width layout row.
    void gradient(std::uint32_t left_color, std::uint32_t right_color,
                  float height = 24.0f) {
        clear_last_widget();
        const Rect rect = allocate(std::max(0.0f, height));
        surface_.draw_horizontal_gradient(rect, left_color, right_color);
    }
    // Draws a top-to-bottom gradient in a full-width layout row.
    void vertical_gradient(std::uint32_t top_color, std::uint32_t bottom_color,
                           float height = 24.0f) {
        clear_last_widget();
        const Rect rect = allocate(std::max(0.0f, height));
        surface_.draw_vertical_gradient(rect, top_color, bottom_color);
    }
    // Draws a four-corner gradient in a full-width layout row.
    void bilinear_gradient(std::uint32_t top_left, std::uint32_t top_right,
                           std::uint32_t bottom_left,
                           std::uint32_t bottom_right,
                           float height = 24.0f) {
        clear_last_widget();
        const Rect rect = allocate(std::max(0.0f, height));
        surface_.draw_bilinear_gradient(rect, top_left, top_right,
                                        bottom_left, bottom_right);
    }
    // Draws a checkerboard pattern in a full-width layout row.
    void checkerboard(std::uint32_t first_color, std::uint32_t second_color,
                      float height = 32.0f, int cell_size = 8) {
        clear_last_widget();
        const Rect rect = allocate(std::max(0.0f, height));
        surface_.draw_checkerboard(rect, first_color, second_color, cell_size);
    }
    // Draws a focusable image button fitted within a full-width layout row.
    Interaction image_button(const std::string& id_text,
                             const std::uint32_t* pixels,
                             int source_width, int source_height,
                             float height = 64.0f) {
        const std::uint64_t id = widget_id(id_text);
        const Rect rect = allocate(std::max(0.0f, height));
        const Interaction result = interact(id, rect);
        surface_.fill_rect(rect, result.active ? theme_.button_active
                                               : result.hovered ? theme_.button_hover
                                                                : theme_.button);
        if (pixels != nullptr && source_width > 0 && source_height > 0 &&
            rect.w > 0.0f && rect.h > 0.0f) {
            const float scale = std::min(
                rect.w / static_cast<float>(source_width),
                rect.h / static_cast<float>(source_height));
            const float image_width = static_cast<float>(source_width) * scale;
            const float image_height = static_cast<float>(source_height) * scale;
            const Rect image_rect{
                rect.x + (rect.w - image_width) * 0.5f,
                rect.y + (rect.h - image_height) * 0.5f,
                image_width, image_height};
            surface_.draw_image(pixels, source_width, source_height, image_rect);
        }
        surface_.outline_rect(rect, result.active ? theme_.accent : theme_.border);
        draw_focus(id, rect);
        return result;
    }

    // Draws a label in the next layout row.
    Interaction label(const std::string& text) {
        clear_last_widget();
        const Rect rect = allocate(20.0f);
        surface_.draw_text(static_cast<int>(rect.x), static_cast<int>(rect.y + 2.0f),
                           text, theme_.text, theme_.font_scale);
        return {};
    }
    // Draws a word-wrapped label and returns the height allocated in pixels.
    float label_wrapped(const std::string& text) {
        clear_last_widget();
        const int font_scale = std::max(1, theme_.font_scale);
        const Rect measured =
            surface_.measure_text_wrapped(text, content_width_, font_scale);
        const Rect rect = allocate(measured.h);
        const std::vector<std::string> lines =
            detail::wrap_text(text, detail::wrap_capacity(content_width_, font_scale));
        surface_.push_clip(rect);
        for (std::size_t index = 0U; index < lines.size(); ++index) {
            const int line_y = static_cast<int>(
                rect.y + 2.0f + static_cast<float>(index) * 16.0f *
                                      static_cast<float>(font_scale));
            surface_.draw_text(static_cast<int>(rect.x), line_y, lines[index],
                               theme_.text, font_scale);
        }
        surface_.pop_clip();
        return measured.h;
    }
    // Adds a delayed tooltip for the previously declared hovered widget.
    void tooltip(const std::string& text) {
        if (last_widget_id_ == 0U || !last_widget_hovered_ || text.empty()) {
            if (!last_widget_hovered_) {
                tooltip_hover_id_ = 0U;
                tooltip_hover_frames_ = 0U;
            }
            return;
        }
        if (tooltip_hover_id_ == last_widget_id_ &&
            tooltip_last_frame_ + 1U == frame_index_) {
            if (tooltip_hover_frames_ < std::numeric_limits<std::size_t>::max()) {
                ++tooltip_hover_frames_;
            }
        } else if (tooltip_last_frame_ != frame_index_ ||
                   tooltip_hover_id_ != last_widget_id_) {
            tooltip_hover_id_ = last_widget_id_;
            tooltip_hover_frames_ = 1U;
        }
        tooltip_last_frame_ = frame_index_;
        if (tooltip_hover_frames_ < theme_.tooltip_delay_frames) {
            return;
        }
        const Rect text_size = surface_.measure_text(text, theme_.font_scale);
        const float padding = static_cast<float>(std::max(0, theme_.padding));
        const float width = std::min(root_.w, text_size.w + padding * 2.0f);
        const float height = std::min(root_.h, text_size.h + padding * 2.0f);
        const float x = std::max(root_.x, std::min(
            static_cast<float>(mouse_x_) + 12.0f, root_.x + root_.w - width));
        const float y = std::max(root_.y, std::min(
            static_cast<float>(mouse_y_) + 12.0f, root_.y + root_.h - height));
        frame_tooltip_draws_.push_back({{x, y, width, height}, text});
    }
    // Draws a button and returns its hover, active, and click state.
    Interaction button(const std::string& text) {
        return button(text, true);
    }
    // Draws a button that can be excluded from pointer and keyboard interaction.
    Interaction button(const std::string& text, bool enabled) {
        const std::uint64_t id = widget_id(text);
        const Rect rect = allocate(28.0f);
        const Interaction result = interact(id, rect, true, enabled);
        const std::uint32_t color = !enabled ? theme_.window
                                      : result.active ? theme_.button_active
                                      : result.hovered ? theme_.button_hover
                                                       : theme_.button;
        surface_.fill_rect(rect, color);
        surface_.draw_text(static_cast<int>(rect.x + theme_.padding),
                           static_cast<int>(rect.y + 6.0f), text,
                           enabled ? theme_.text : theme_.button_hover,
                           theme_.font_scale);
        if (enabled) {
            draw_focus(id, rect);
        }
        return result;
    }
    // Draws a checkbox and updates its value when clicked.
    Interaction checkbox(const std::string& text, bool& value) {
        const std::uint64_t id = widget_id(text);
        const Rect rect = allocate(24.0f);
        Interaction result = interact(id, rect);
        const float side = 16.0f;
        const Rect box{rect.x, rect.y + 4.0f, side, side};
        surface_.fill_rect(box, theme_.border);
        surface_.fill_rect({box.x + 2.0f, box.y + 2.0f, side - 4.0f, side - 4.0f},
                           value ? theme_.accent : theme_.window);
        if (value) {
            surface_.draw_line(static_cast<int>(box.x + 3.0f),
                               static_cast<int>(box.y + 8.0f),
                               static_cast<int>(box.x + 7.0f),
                               static_cast<int>(box.y + 12.0f), theme_.text);
            surface_.draw_line(static_cast<int>(box.x + 7.0f),
                               static_cast<int>(box.y + 12.0f),
                               static_cast<int>(box.x + 14.0f),
                               static_cast<int>(box.y + 4.0f), theme_.text);
        }
        surface_.draw_text(static_cast<int>(box.x + side + theme_.padding),
                           static_cast<int>(rect.y + 4.0f), text, theme_.text,
                           theme_.font_scale);
        if (result.clicked) {
            value = !value;
            result.changed = true;
        }
        draw_focus(id, rect);
        return result;
    }
    // Draws a focusable selection row and toggles its caller-owned state.
    Interaction selectable(const std::string& text, bool& selected) {
        const std::uint64_t id = widget_id(text);
        const Rect rect = allocate(26.0f);
        Interaction result = interact(id, rect);
        if (result.clicked) {
            selected = !selected;
            result.changed = true;
        }
        const std::uint32_t color =
            selected ? theme_.accent
                     : result.active ? theme_.button_active
                                     : result.hovered ? theme_.button_hover
                                                      : theme_.button;
        surface_.fill_rect(rect, color);
        surface_.draw_text(static_cast<int>(rect.x + theme_.padding),
                           static_cast<int>(rect.y + 5.0f), text, theme_.text,
                           theme_.font_scale);
        draw_focus(id, rect);
        return result;
    }
    // Draws a keyboard-navigable tab row and updates the caller-owned tab index.
    Interaction tab_bar(const std::string& group,
                        const std::vector<std::string>& tabs,
                        std::size_t& selected) {
        const std::uint64_t id = widget_id(group);
        const Rect rect = allocate(30.0f);
        Interaction result;
        if (tabs.empty()) {
            return result;
        }
        selected = std::min(selected, tabs.size() - 1U);
        const float tab_width = rect.w / static_cast<float>(tabs.size());
        for (std::size_t index = 0U; index < tabs.size(); ++index) {
            const std::uint64_t tab_id =
                id ^ ((static_cast<std::uint64_t>(index) + 1U) *
                      0xD6E8FEB86659FD93ULL);
            const Rect tab_rect{rect.x + tab_width * static_cast<float>(index),
                                rect.y, tab_width, rect.h};
            const Interaction tab_result = interact(tab_id, tab_rect);
            for (auto navigation = pending_navigation_.begin();
                 navigation != pending_navigation_.end();) {
                if (navigation->first != tab_id) {
                    ++navigation;
                    continue;
                }
                const std::size_t previous_selection = selected;
                switch (navigation->second) {
                    case Key::Left:
                    case Key::Up:
                        if (selected > 0U) {
                            --selected;
                        }
                        break;
                    case Key::Right:
                    case Key::Down:
                        if (selected + 1U < tabs.size()) {
                            ++selected;
                        }
                        break;
                    case Key::Home:
                        selected = 0U;
                        break;
                    case Key::End:
                        selected = tabs.size() - 1U;
                        break;
                    default:
                        break;
                }
                focused_id_ =
                    id ^ ((static_cast<std::uint64_t>(selected) + 1U) *
                          0xD6E8FEB86659FD93ULL);
                focused_text_id_ = 0U;
                result.changed =
                    result.changed || selected != previous_selection;
                navigation = pending_navigation_.erase(navigation);
            }
            if (tab_result.clicked) {
                result.changed = result.changed || selected != index;
                selected = index;
            }
            result.hovered = result.hovered || tab_result.hovered;
            result.active = result.active || tab_result.active;
            result.clicked = result.clicked || tab_result.clicked;
            const std::uint32_t color =
                selected == index ? theme_.accent
                                  : tab_result.active ? theme_.button_active
                                                      : tab_result.hovered
                                                            ? theme_.button_hover
                                                            : theme_.button;
            surface_.fill_rect(tab_rect, color);
            surface_.draw_text(static_cast<int>(tab_rect.x + theme_.padding),
                               static_cast<int>(tab_rect.y + 7.0f), tabs[index],
                               theme_.text, theme_.font_scale);
            draw_focus(tab_id, tab_rect);
        }
        last_widget_id_ = id;
        last_widget_hovered_ = result.hovered;
        return result;
    }
    // Draws a keyboard-navigable grid of caller-provided color swatches.
    Interaction color_palette(const std::string& id_text,
                              const std::vector<std::uint32_t>& colors,
                              std::size_t& selected,
                              std::size_t columns = 4U) {
        const std::uint64_t id = widget_id(id_text);
        Interaction result;
        if (colors.empty()) {
            return result;
        }
        selected = std::min(selected, colors.size() - 1U);
        const std::size_t column_count =
            std::max<std::size_t>(1U, std::min(columns, colors.size()));
        const std::size_t row_count =
            (colors.size() + column_count - 1U) / column_count;
        constexpr float cell_height = 24.0f;
        const float gap = static_cast<float>(std::max(0, theme_.padding / 2));
        const float allocated_height =
            static_cast<float>(row_count) * cell_height +
            static_cast<float>(row_count - 1U) * gap;
        const Rect rect = allocate(allocated_height);
        const float cell_width =
            std::max(0.0f, (rect.w - static_cast<float>(column_count - 1U) * gap) /
                               static_cast<float>(column_count));
        for (std::size_t index = 0U; index < colors.size(); ++index) {
            const std::size_t row_index = index / column_count;
            const std::size_t column_index = index % column_count;
            const std::uint64_t swatch_id =
                id ^ ((static_cast<std::uint64_t>(index) + 1U) *
                      0xA0761D6478BD642FULL);
            const Rect swatch{
                rect.x + static_cast<float>(column_index) * (cell_width + gap),
                rect.y + static_cast<float>(row_index) * (cell_height + gap),
                cell_width, cell_height};
            const Interaction swatch_result = interact(swatch_id, swatch);
            for (auto navigation = pending_navigation_.begin();
                 navigation != pending_navigation_.end();) {
                if (navigation->first != swatch_id) {
                    ++navigation;
                    continue;
                }
                const std::size_t previous_selection = selected;
                switch (navigation->second) {
                    case Key::Left:
                        if (selected > 0U) {
                            --selected;
                        }
                        break;
                    case Key::Right:
                        if (selected + 1U < colors.size()) {
                            ++selected;
                        }
                        break;
                    case Key::Up:
                        selected = selected >= column_count
                                       ? selected - column_count
                                       : selected % column_count;
                        break;
                    case Key::Down:
                        if (selected + column_count < colors.size()) {
                            selected += column_count;
                        } else if (selected + 1U < colors.size()) {
                            selected = colors.size() - 1U;
                        }
                        break;
                    case Key::Home:
                        selected = 0U;
                        break;
                    case Key::End:
                        selected = colors.size() - 1U;
                        break;
                    default:
                        break;
                }
                const std::size_t focused_index = selected;
                focused_id_ =
                    id ^ ((static_cast<std::uint64_t>(focused_index) + 1U) *
                          0xA0761D6478BD642FULL);
                focused_text_id_ = 0U;
                result.changed =
                    result.changed || selected != previous_selection;
                navigation = pending_navigation_.erase(navigation);
            }
            if (swatch_result.clicked) {
                result.changed = result.changed || selected != index;
                selected = index;
            }
            result.hovered = result.hovered || swatch_result.hovered;
            result.active = result.active || swatch_result.active;
            result.clicked = result.clicked || swatch_result.clicked;
            surface_.fill_rect(swatch, theme_.border);
            const float inset = selected == index ? 3.0f : 2.0f;
            surface_.fill_rect(
                {swatch.x + inset, swatch.y + inset,
                 std::max(0.0f, swatch.w - 2.0f * inset),
                 cell_height - 2.0f * inset},
                colors[index]);
            if (selected == index) {
                surface_.outline_rect(swatch, theme_.accent, 2.0f);
            }
            draw_focus(swatch_id, swatch);
        }
        last_widget_id_ = id;
        last_widget_hovered_ = result.hovered;
        return result;
    }
    // Draws four keyboard-focusable sliders for a packed RGBA color.
    Interaction color_picker(const std::string& id_text,
                             std::uint32_t& color) {
        const std::uint32_t original_color = color;
        const Rect preview = allocate(24.0f);
        surface_.fill_rect(preview, color);
        surface_.outline_rect(preview, theme_.border);
        id_stack_.push_back(hash(id_text, id_stack_.back()));
        Interaction result;
        const auto edit_channel = [this, &result, &color](
                                      const std::string& name,
                                      unsigned int shift) {
            float channel = static_cast<float>((color >> shift) & 0xFFU);
            const Interaction channel_result =
                drag_value(name, channel, 0.0f, 255.0f, 1.0f);
            const std::uint32_t channel_value =
                static_cast<std::uint32_t>(std::lround(channel));
            const std::uint32_t mask = 0xFFU << shift;
            color = (color & ~mask) | ((channel_value & 0xFFU) << shift);
            result.hovered = result.hovered || channel_result.hovered;
            result.active = result.active || channel_result.active;
            result.clicked = result.clicked || channel_result.clicked;
        };
        edit_channel("Red", 0U);
        edit_channel("Green", 8U);
        edit_channel("Blue", 16U);
        edit_channel("Alpha", 24U);
        id_stack_.pop_back();
        result.changed = color != original_color;
        last_widget_id_ = widget_id(id_text);
        last_widget_hovered_ = result.hovered;
        return result;
    }
    // Draws a themed discrete rating scale with mouse and keyboard selection.
    Interaction rating(const std::string& id_text, int& value,
                       int maximum = 5) {
        const std::uint64_t id = widget_id(id_text);
        const Rect allocated = allocate(28.0f);
        const int count = std::max(1, maximum);
        const int previous_value = std::clamp(value, 0, count);
        value = previous_value;
        const float available_width = std::max(0.0f, allocated.w);
        const float cell_width = std::max(
            1.0f, std::min(24.0f,
                           available_width / static_cast<float>(count)));
        const Rect rect{allocated.x, allocated.y,
                        std::min(available_width,
                                 cell_width * static_cast<float>(count)),
                        allocated.h};
        if (rect.w <= 0.0f) {
            return {};
        }
        Interaction result = interact(id, rect);
        if (result.clicked) {
            const float relative_x = static_cast<float>(mouse_x_) - rect.x;
            const int selected = static_cast<int>(relative_x / cell_width);
            value = std::clamp(selected + 1, 1, count);
        }
        for (auto navigation = pending_navigation_.begin();
             navigation != pending_navigation_.end();) {
            if (navigation->first != id) {
                ++navigation;
                continue;
            }
            switch (navigation->second) {
                case Key::Left:
                case Key::Down:
                    value = std::max(0, value - 1);
                    break;
                case Key::Right:
                case Key::Up:
                    value = value < count ? value + 1 : count;
                    break;
                case Key::Home:
                    value = 0;
                    break;
                case Key::End:
                    value = count;
                    break;
                default:
                    break;
            }
            navigation = pending_navigation_.erase(navigation);
        }
        result.changed = value != previous_value;
        const int hovered_mark =
            result.hovered
                ? std::clamp(static_cast<int>(
                                 (static_cast<float>(mouse_x_) - rect.x) /
                                 cell_width),
                             0, count - 1)
                : -1;
        const int radius =
            std::max(1, std::min(5, static_cast<int>(cell_width * 0.3f)));
        for (std::int64_t index = 0; index < count; ++index) {
            const int center_x = static_cast<int>(
                rect.x + (static_cast<float>(index) + 0.5f) * cell_width);
            const int center_y = static_cast<int>(rect.y + rect.h * 0.5f);
            const bool marked = index < value;
            const bool hovered = index == hovered_mark;
            surface_.draw_filled_circle(
                center_x, center_y, radius,
                marked ? theme_.accent
                       : hovered ? theme_.button_hover : theme_.border);
            if (!marked) {
                surface_.draw_filled_circle(
                    center_x, center_y, std::max(0, radius - 2),
                    hovered ? theme_.button_hover : theme_.button);
            }
        }
        draw_focus(id, rect);
        return result;
    }
    // Draws a themed toggle switch and updates its caller-owned boolean state.
    Interaction toggle(const std::string& text, bool& value) {
        const std::uint64_t id = widget_id(text);
        const Rect rect = allocate(28.0f);
        Interaction result = interact(id, rect);
        if (result.clicked) {
            value = !value;
            result.changed = true;
        }
        const float track_width = 40.0f;
        const float track_height = 20.0f;
        const float track_y = rect.y + 4.0f;
        const std::uint32_t track_color =
            value ? theme_.accent
                  : result.active ? theme_.button_active
                                  : result.hovered ? theme_.button_hover
                                                   : theme_.button;
        surface_.draw_rounded_rect(
            {rect.x, track_y, track_width, track_height}, track_height * 0.5f,
            track_color);
        const int knob_x = static_cast<int>(
            rect.x + (value ? track_width - track_height * 0.5f
                            : track_height * 0.5f));
        const int knob_y = static_cast<int>(track_y + track_height * 0.5f);
        surface_.draw_filled_circle(knob_x, knob_y, 7, theme_.text);
        surface_.draw_text(static_cast<int>(rect.x + track_width + theme_.padding),
                           static_cast<int>(rect.y + 6.0f), text, theme_.text,
                           theme_.font_scale);
        draw_focus(id, rect);
        return result;
    }
    // Draws a non-interactive horizontal divider using the theme border color.
    void separator() {
        const Rect rect = allocate(2.0f);
        surface_.fill_rect(rect, theme_.border);
    }
    // Draws a non-interactive line chart from caller-provided samples.
    void sparkline(const std::vector<float>& samples,
                   float minimum, float maximum, float height = 36.0f) {
        clear_last_widget();
        const Rect rect = allocate(std::max(12.0f, height));
        surface_.fill_rect(rect, theme_.border);
        const Rect area{rect.x + 2.0f, rect.y + 2.0f,
                        std::max(0.0f, rect.w - 4.0f),
                        std::max(0.0f, rect.h - 4.0f)};
        surface_.fill_rect(area, theme_.window);
        const float low = std::min(minimum, maximum);
        const float high = std::max(minimum, maximum);
        if (samples.empty() || !std::isfinite(low) || !std::isfinite(high) ||
            area.w < 1.0f || area.h < 1.0f) {
            return;
        }
        const auto sample_position = [&](std::size_t index) {
            const float x_ratio = samples.size() > 1U
                                      ? static_cast<float>(index) /
                                            static_cast<float>(samples.size() - 1U)
                                      : 0.5f;
            const float sample = samples[index];
            const float y_ratio =
                !std::isfinite(sample) || high == low
                    ? 0.5f
                    : std::clamp((sample - low) / (high - low), 0.0f, 1.0f);
            const float x = area.x + x_ratio * std::max(0.0f, area.w - 1.0f);
            const float y = area.y +
                            (1.0f - y_ratio) * std::max(0.0f, area.h - 1.0f);
            return std::pair<int, int>{static_cast<int>(x), static_cast<int>(y)};
        };
        if (samples.size() == 1U) {
            if (std::isfinite(samples.front())) {
                const auto point = sample_position(0U);
                surface_.draw_filled_circle(point.first, point.second, 1,
                                            theme_.accent);
            }
            return;
        }
        surface_.push_clip(area);
        for (std::size_t index = 1U; index < samples.size(); ++index) {
            if (!std::isfinite(samples[index - 1U]) ||
                !std::isfinite(samples[index])) {
                continue;
            }
            const auto previous = sample_position(index - 1U);
            const auto current = sample_position(index);
            surface_.draw_line(previous.first, previous.second,
                               current.first, current.second, theme_.accent);
        }
        surface_.pop_clip();
    }
    // Draws a non-interactive histogram of caller-provided samples.
    void histogram(const std::vector<float>& samples, float minimum,
                   float maximum, std::size_t bins, float height = 48.0f) {
        clear_last_widget();
        const Rect rect = allocate(std::max(12.0f, height));
        surface_.fill_rect(rect, theme_.border);
        const Rect area{rect.x + 2.0f, rect.y + 2.0f,
                        std::max(0.0f, rect.w - 4.0f),
                        std::max(0.0f, rect.h - 4.0f)};
        surface_.fill_rect(area, theme_.window);
        const float low = std::min(minimum, maximum);
        const float high = std::max(minimum, maximum);
        if (samples.empty() || !std::isfinite(low) || !std::isfinite(high) ||
            area.w < 1.0f || area.h < 1.0f) {
            return;
        }
        const std::size_t maximum_bins =
            static_cast<std::size_t>(std::max(1.0f, std::floor(area.w)));
        const std::size_t bin_count =
            std::max<std::size_t>(1U, std::min(bins, maximum_bins));
        std::vector<std::size_t> counts(bin_count, 0U);
        for (const float sample : samples) {
            if (!std::isfinite(sample)) {
                continue;
            }
            const float ratio = high == low
                                    ? 0.5f
                                    : std::clamp((sample - low) / (high - low),
                                                 0.0f, 1.0f);
            const std::size_t index = std::min(
                bin_count - 1U,
                static_cast<std::size_t>(ratio * static_cast<float>(bin_count)));
            ++counts[index];
        }
        const std::size_t largest_bin =
            *std::max_element(counts.begin(), counts.end());
        if (largest_bin == 0U) {
            return;
        }
        for (std::size_t index = 0U; index < bin_count; ++index) {
            if (counts[index] == 0U) {
                continue;
            }
            const float left_ratio =
                static_cast<float>(index) / static_cast<float>(bin_count);
            const float right_ratio =
                static_cast<float>(index + 1U) / static_cast<float>(bin_count);
            const float bar_x = area.x + area.w * left_ratio;
            const float bar_width = area.w * (right_ratio - left_ratio);
            const float bar_height =
                area.h * static_cast<float>(counts[index]) /
                static_cast<float>(largest_bin);
            surface_.fill_rect(
                {bar_x, area.y + area.h - bar_height, bar_width, bar_height},
                theme_.accent);
        }
    }
    // Draws caller-provided CPU timing intervals on a frame-relative timeline.
    void profiler_timeline(const std::vector<ProfileEvent>& events,
                           float frame_duration_ms,
                           float frame_budget_ms = 16.67f,
                           float height = 120.0f) {
        clear_last_widget();
        const Rect rect = allocate(std::max(48.0f, height));
        surface_.fill_rect(rect, theme_.border);
        const Rect area{rect.x + 2.0f, rect.y + 2.0f,
                        std::max(0.0f, rect.w - 4.0f),
                        std::max(0.0f, rect.h - 4.0f)};
        surface_.fill_rect(area, theme_.window);
        if (!std::isfinite(frame_duration_ms) || frame_duration_ms <= 0.0f ||
            area.w < 48.0f || area.h < 18.0f) {
            return;
        }

        constexpr float header_height = 18.0f;
        constexpr float row_height = 18.0f;
        const float label_width = std::min(64.0f, area.w * 0.4f);
        const Rect timeline{area.x + label_width, area.y + header_height,
                            std::max(0.0f, area.w - label_width - 2.0f),
                            std::max(0.0f, area.h - header_height)};
        if (timeline.w < 1.0f) {
            return;
        }

        surface_.push_clip(area);
        surface_.draw_text(static_cast<int>(area.x + 2.0f),
                           static_cast<int>(area.y + 1.0f),
                           "CPU frame timeline", theme_.text, theme_.font_scale);
        for (int tick = 0; tick <= 4; ++tick) {
            const float ratio = static_cast<float>(tick) * 0.25f;
            const int x = static_cast<int>(
                timeline.x + ratio * std::max(0.0f, timeline.w - 1.0f));
            surface_.draw_line(x, static_cast<int>(timeline.y), x,
                               static_cast<int>(area.y + area.h - 1.0f),
                               theme_.border);
        }
        const std::size_t visible_rows = static_cast<std::size_t>(
            std::max(0.0f, std::floor(timeline.h / row_height)));
        const std::size_t row_count = std::min(events.size(), visible_rows);
        const std::array<std::uint32_t, 3> colors{{
            theme_.accent, theme_.title_bar, theme_.button_hover,
        }};
        for (std::size_t index = 0U; index < row_count; ++index) {
            const ProfileEvent& event = events[index];
            const float row_y =
                timeline.y + row_height * static_cast<float>(index);
            surface_.draw_text(static_cast<int>(area.x + 2.0f),
                               static_cast<int>(row_y + 1.0f), event.name,
                               theme_.text, theme_.font_scale);
            if (!std::isfinite(event.start_ms) ||
                !std::isfinite(event.duration_ms) || event.duration_ms <= 0.0f) {
                continue;
            }
            const double start =
                std::max(0.0, static_cast<double>(event.start_ms));
            const double finish = std::min(
                static_cast<double>(frame_duration_ms),
                static_cast<double>(event.start_ms) +
                    static_cast<double>(event.duration_ms));
            if (finish <= start) {
                continue;
            }
            const double frame = static_cast<double>(frame_duration_ms);
            const float start_ratio = static_cast<float>(start / frame);
            const float finish_ratio = static_cast<float>(finish / frame);
            const float bar_x =
                timeline.x + start_ratio * timeline.w;
            const float bar_end =
                timeline.x + finish_ratio * timeline.w;
            surface_.draw_rounded_rect(
                {bar_x, row_y + 3.0f,
                 std::max(1.0f, bar_end - bar_x), row_height - 6.0f},
                2.0f, colors[index % colors.size()]);
        }
        if (std::isfinite(frame_budget_ms) && frame_budget_ms > 0.0f &&
            frame_budget_ms <= frame_duration_ms) {
            const float budget_x =
                timeline.x + (frame_budget_ms / frame_duration_ms) * timeline.w;
            surface_.draw_dashed_line(
                static_cast<int>(budget_x), static_cast<int>(timeline.y),
                static_cast<int>(budget_x),
                static_cast<int>(area.y + area.h - 1.0f), 0xFFE06C75U, 2, 2);
        }
        surface_.pop_clip();
    }
    // Draws caller-provided bounded points as a clipped scatter plot.
    void scatter_plot(const std::vector<std::pair<float, float>>& points,
                      float minimum_x, float maximum_x, float minimum_y,
                      float maximum_y, float height = 48.0f) {
        clear_last_widget();
        const Rect rect = allocate(std::max(12.0f, height));
        surface_.fill_rect(rect, theme_.border);
        const Rect area{rect.x + 2.0f, rect.y + 2.0f,
                        std::max(0.0f, rect.w - 4.0f),
                        std::max(0.0f, rect.h - 4.0f)};
        surface_.fill_rect(area, theme_.window);

        if (points.empty() || !std::isfinite(minimum_x) ||
            !std::isfinite(maximum_x) || !std::isfinite(minimum_y) ||
            !std::isfinite(maximum_y) ||
            area.w < 1.0f || area.h < 1.0f) {
            return;
        }
        const float low_x = std::min(minimum_x, maximum_x);
        const float high_x = std::max(minimum_x, maximum_x);
        const float low_y = std::min(minimum_y, maximum_y);
        const float high_y = std::max(minimum_y, maximum_y);

        surface_.push_clip(area);
        for (const auto& point : points) {
            if (!std::isfinite(point.first) || !std::isfinite(point.second) ||
                point.first < low_x || point.first > high_x ||
                point.second < low_y || point.second > high_y) {
                continue;
            }
            const double x_ratio =
                high_x == low_x
                    ? 0.5
                    : (static_cast<double>(point.first) -
                       static_cast<double>(low_x)) /
                          (static_cast<double>(high_x) -
                           static_cast<double>(low_x));
            const double y_ratio =
                high_y == low_y
                    ? 0.5
                    : (static_cast<double>(point.second) -
                       static_cast<double>(low_y)) /
                          (static_cast<double>(high_y) -
                           static_cast<double>(low_y));
            const int x = static_cast<int>(
                area.x + static_cast<float>(
                             x_ratio * static_cast<double>(
                                           std::max(0.0f, area.w - 1.0f))));
            const int y = static_cast<int>(
                area.y + static_cast<float>(
                             (1.0 - y_ratio) * static_cast<double>(
                                                   std::max(0.0f, area.h - 1.0f))));
            surface_.draw_filled_circle(x, y, 1, theme_.accent);
        }
        surface_.pop_clip();
    }
    // Draws a pie chart from positive finite values and a cycling color palette.
    void pie_chart(const std::vector<float>& values,
                   const std::vector<std::uint32_t>& colors,
                   float height = 72.0f) {
        clear_last_widget();
        draw_pie_chart(values, colors, allocate(std::max(12.0f, height)), 0.0f);
    }
    // Draws a pie chart with a centered hole and a cycling color palette.
    void donut_chart(const std::vector<float>& values,
                     const std::vector<std::uint32_t>& colors,
                     float height = 72.0f) {
        clear_last_widget();
        draw_pie_chart(values, colors, allocate(std::max(12.0f, height)), 0.5f);
    }
    // Draws radio options and updates the caller-owned selected option index.
    Interaction radio(const std::string& group,
                      const std::vector<std::string>& options, int& selected) {
        const std::uint64_t id = widget_id(group);
        const Rect rect = allocate(28.0f);
        Interaction result;
        if (options.empty()) {
            return result;
        }
        selected = std::clamp(selected, 0, static_cast<int>(options.size() - 1U));
        const float option_width = rect.w / static_cast<float>(options.size());
        for (std::size_t index = 0; index < options.size(); ++index) {
            const std::uint64_t option_id =
                id ^ (static_cast<std::uint64_t>(index + 1U) * 0x9E3779B97F4A7C15ULL);
            const Rect option_rect{rect.x + option_width * static_cast<float>(index),
                                   rect.y, option_width, rect.h};
            const Interaction option_result = interact(option_id, option_rect);
            for (auto navigation = pending_navigation_.begin();
                 navigation != pending_navigation_.end();) {
                if (navigation->first != option_id) {
                    ++navigation;
                    continue;
                }
                const int old_value = selected;
                switch (navigation->second) {
                    case Key::Left:
                    case Key::Up:
                        selected = std::max(0, selected - 1);
                        break;
                    case Key::Right:
                    case Key::Down:
                        selected = std::min(static_cast<int>(options.size() - 1U),
                                            selected + 1);
                        break;
                    case Key::Home:
                        selected = 0;
                        break;
                    case Key::End:
                        selected = static_cast<int>(options.size() - 1U);
                        break;
                    default:
                        break;
                }
                focused_id_ =
                    id ^ ((static_cast<std::uint64_t>(selected) + 1U) *
                          0x9E3779B97F4A7C15ULL);
                focused_text_id_ = 0U;
                result.changed = result.changed || selected != old_value;
                navigation = pending_navigation_.erase(navigation);
            }
            if (option_result.clicked) {
                const int option_index = static_cast<int>(index);
                result.changed = result.changed || selected != option_index;
                selected = option_index;
            }
            result.hovered = result.hovered || option_result.hovered;
            result.active = result.active || option_result.active;
            result.clicked = result.clicked || option_result.clicked;
            const bool is_selected = selected == static_cast<int>(index);
            surface_.fill_rect(option_rect, option_result.active
                                                ? theme_.button_active
                                                : option_result.hovered
                                                      ? theme_.button_hover
                                                      : theme_.button);
            const float marker_y = option_rect.y + 6.0f;
            surface_.fill_rect({option_rect.x + 4.0f, marker_y, 16.0f, 16.0f},
                               theme_.border);
            surface_.fill_rect({option_rect.x + 7.0f, marker_y + 3.0f, 10.0f, 10.0f},
                               is_selected ? theme_.accent : theme_.window);
            surface_.draw_text(static_cast<int>(option_rect.x + 24.0f),
                               static_cast<int>(option_rect.y + 6.0f), options[index],
                               theme_.text, theme_.font_scale);
            draw_focus(option_id, option_rect);
        }
        last_widget_id_ = id;
        last_widget_hovered_ = result.hovered;
        return result;
    }
    // Draws a read-only progress bar with its displayed value clamped to [0, 1].
    Interaction progress_bar(const std::string& id_text, float value) {
        const std::uint64_t id = widget_id(id_text);
        const Rect rect = allocate(20.0f);
        const Interaction result = interact(id, rect);
        const float progress = std::clamp(value, 0.0f, 1.0f);
        surface_.fill_rect(rect, theme_.window);
        surface_.fill_rect({rect.x, rect.y, rect.w * progress, rect.h}, theme_.accent);
        draw_focus(id, rect);
        return result;
    }
    // Draws a focusable header that toggles the caller-owned open state.
    Interaction collapsing_header(const std::string& text, bool& open) {
        const std::uint64_t id = widget_id(text);
        const Rect rect = allocate(28.0f);
        Interaction result = interact(id, rect);
        if (result.clicked) {
            open = !open;
            result.changed = true;
        }
        const std::uint32_t color = result.active ? theme_.button_active
                                      : result.hovered ? theme_.button_hover
                                                       : theme_.button;
        surface_.fill_rect(rect, color);
        surface_.draw_text(static_cast<int>(rect.x + theme_.padding),
                           static_cast<int>(rect.y + 6.0f), open ? "v " : "> ",
                           theme_.accent, theme_.font_scale);
        surface_.draw_text(static_cast<int>(rect.x + theme_.padding + 16),
                           static_cast<int>(rect.y + 6.0f), text, theme_.text,
                           theme_.font_scale);
        draw_focus(id, rect);
        return result;
    }
    // Begins a caller-owned hierarchical section and scopes child widget IDs.
    bool begin_tree_node(const std::string& text, bool& open) {
        const std::uint64_t id = widget_id(text);
        const Rect rect = allocate(24.0f);
        const Interaction result = interact(id, rect);
        if (result.clicked) {
            open = !open;
        }
        surface_.fill_rect(rect, result.active ? theme_.button_active
                                               : result.hovered ? theme_.button_hover
                                                                : theme_.button);
        const int center_x = static_cast<int>(rect.x + 10.0f);
        const int center_y = static_cast<int>(rect.y + 12.0f);
        if (open) {
            surface_.draw_line(center_x - 3, center_y - 2, center_x + 3,
                               center_y - 2, theme_.text);
            surface_.draw_line(center_x - 3, center_y - 2, center_x,
                               center_y + 3, theme_.text);
            surface_.draw_line(center_x + 3, center_y - 2, center_x,
                               center_y + 3, theme_.text);
        } else {
            surface_.draw_line(center_x - 2, center_y - 3, center_x - 2,
                               center_y + 3, theme_.text);
            surface_.draw_line(center_x - 2, center_y - 3, center_x + 3,
                               center_y, theme_.text);
            surface_.draw_line(center_x - 2, center_y + 3, center_x + 3,
                               center_y, theme_.text);
        }
        surface_.draw_text(static_cast<int>(rect.x + 20.0f),
                           static_cast<int>(rect.y + 4.0f), text, theme_.text,
                           theme_.font_scale);
        draw_focus(id, rect);
        if (!open) {
            return false;
        }
        tree_stack_.push_back({cursor_x_, content_width_});
        id_stack_.push_back(hash(text, id_stack_.back()));
        constexpr float indentation = 12.0f;
        cursor_x_ += indentation;
        content_width_ = std::max(0.0f, content_width_ - indentation);
        return true;
    }
    // Ends the currently open tree node and restores its parent ID and layout.
    void end_tree_node() {
        if (tree_stack_.empty()) {
            return;
        }
        const TreeFrame frame = tree_stack_.back();
        tree_stack_.pop_back();
        id_stack_.pop_back();
        cursor_x_ = frame.previous_cursor_x;
        content_width_ = frame.previous_content_width;
    }
    // Draws a combo box whose popup is rendered in the final overlay layer.
    Interaction combo(const std::string& id_text, std::size_t& selected,
                      const std::vector<std::string>& options) {
        const std::uint64_t id = widget_id(id_text);
        const Rect rect = allocate(28.0f);
        const bool keyboard_activation = keyboard_activation_id_ == id;
        Interaction result = interact(id, rect);
        if (result.active || result.clicked) {
            pointer_claimed_id_ = id;
        }
        ComboState& state = combo_states_[id];
        if (options.empty()) {
            selected = 0U;
            if (open_combo_id_ == id) {
                open_combo_id_ = 0U;
            }
        } else {
            selected = std::min(selected, options.size() - 1U);
        }
        if (open_combo_id_ == id && !options.empty()) {
            state.highlight = std::min(state.highlight, options.size() - 1U);
            for (auto navigation = pending_navigation_.begin();
                 navigation != pending_navigation_.end();) {
                if (navigation->first != id) {
                    ++navigation;
                    continue;
                }
                switch (navigation->second) {
                    case Key::Up:
                    case Key::Left:
                        if (state.highlight > 0U) {
                            --state.highlight;
                        }
                        break;
                    case Key::Down:
                    case Key::Right:
                        if (state.highlight + 1U < options.size()) {
                            ++state.highlight;
                        }
                        break;
                    case Key::Home:
                        state.highlight = 0U;
                        break;
                    case Key::End:
                        state.highlight = options.size() - 1U;
                        break;
                    default:
                        break;
                }
                navigation = pending_navigation_.erase(navigation);
            }
        }
        if (result.clicked && !options.empty()) {
            if (open_combo_id_ == id && keyboard_activation) {
                result.changed = selected != state.highlight;
                selected = state.highlight;
                open_combo_id_ = 0U;
            } else if (open_combo_id_ == id) {
                open_combo_id_ = 0U;
            } else {
                open_combo_id_ = id;
                state.highlight = selected;
                state.has_pressed_option = false;
                active_id_ = 0U;
            }
        }
        if (open_combo_id_ == id && !options.empty()) {
            const float row_height = 20.0f;
            const Rect popup_rect{rect.x, rect.y + rect.h, rect.w,
                                  row_height * static_cast<float>(options.size())};
            const int option = contains_point(popup_rect, mouse_x_, mouse_y_)
                                   ? static_cast<int>((static_cast<float>(mouse_y_) -
                                                       popup_rect.y) /
                                                      row_height)
                                   : -1;
            if (option >= 0 && static_cast<std::size_t>(option) < options.size()) {
                state.highlight = static_cast<std::size_t>(option);
                if (mouse_pressed_) {
                    state.pressed_option = static_cast<std::size_t>(option);
                    state.has_pressed_option = true;
                    focused_id_ = id;
                    focused_text_id_ = 0U;
                }
            }
            if (mouse_released_ && state.has_pressed_option) {
                if (option >= 0 &&
                    state.pressed_option == static_cast<std::size_t>(option)) {
                    const std::size_t previous_selection = selected;
                    selected = state.pressed_option;
                    state.highlight = selected;
                    result.changed = previous_selection != selected;
                    result.clicked = true;
                    open_combo_id_ = 0U;
                }
                state.has_pressed_option = false;
            }
        }
        const std::string caption =
            options.empty() ? id_text : options[selected];
        surface_.fill_rect(rect, result.active ? theme_.button_active
                                               : result.hovered ? theme_.button_hover
                                                                : theme_.button);
        surface_.draw_text(static_cast<int>(rect.x + theme_.padding),
                           static_cast<int>(rect.y + 6.0f), caption, theme_.text,
                           theme_.font_scale);
        draw_focus(id, rect);
        if (open_combo_id_ == id && !options.empty()) {
            const float row_height = 20.0f;
            active_popup_rect_ = {
                rect.x, rect.y + rect.h, rect.w,
                row_height * static_cast<float>(options.size())};
            active_combo_rect_ = rect;
            frame_open_combo_seen_ = true;
            frame_popup_draws_.push_back({active_popup_rect_, options, state.highlight});
        }
        return result;
    }
    // Draws a slider and clamps its value to the supplied range.
    Interaction slider(const std::string& id_text, float& value, float minimum,
                       float maximum) {
        const std::uint64_t id = widget_id(id_text);
        const Rect rect = allocate(28.0f);
        Interaction result = interact(id, rect);
        const float low = std::min(minimum, maximum);
        const float high = std::max(minimum, maximum);
        const float track_y = rect.y + 10.0f;
        const float track_width = std::max(1.0f, rect.w);
        const float old_value = std::clamp(value, low, high);
        value = old_value;
        if (result.active && mouse_down_) {
            const float ratio = std::clamp((static_cast<float>(mouse_x_) - rect.x) /
                                               track_width,
                                           0.0f, 1.0f);
            value = low + ratio * (high - low);
            result.changed = value != old_value;
        }
        for (auto navigation = pending_navigation_.begin();
             navigation != pending_navigation_.end();) {
            if (navigation->first != id) {
                ++navigation;
                continue;
            }
            const float step = (high - low) / 100.0f;
            switch (navigation->second) {
                case Key::Left:
                case Key::Down:
                    value = std::max(low, value - step);
                    break;
                case Key::Right:
                case Key::Up:
                    value = std::min(high, value + step);
                    break;
                case Key::Home:
                    value = low;
                    break;
                case Key::End:
                    value = high;
                    break;
                default:
                    break;
            }
            result.changed = result.changed || value != old_value;
            navigation = pending_navigation_.erase(navigation);
        }
        const float ratio = high == low ? 0.0f : (value - low) / (high - low);
        surface_.fill_rect({rect.x, track_y, track_width, 8.0f}, theme_.window);
        surface_.fill_rect({rect.x, track_y, track_width * ratio, 8.0f}, theme_.accent);
        surface_.fill_rect({rect.x + track_width * ratio - 4.0f, track_y - 4.0f, 8.0f, 16.0f},
                           theme_.text);
        draw_focus(id, rect);
        return result;
    }
    // Draws a two-handle slider for selecting a bounded numeric interval.
    Interaction range_slider(const std::string& id_text, float& lower_value,
                              float& upper_value, float minimum, float maximum) {
        const std::uint64_t id = widget_id(id_text);
        const std::uint64_t lower_id = id ^ 0x8CB92BA72F3D8DD7ULL;
        const std::uint64_t upper_id = id ^ 0xDB4F0B9175AE2165ULL;
        const Rect rect = allocate(28.0f);
        const float low = std::min(minimum, maximum);
        const float high = std::max(minimum, maximum);
        lower_value = std::clamp(lower_value, low, high);
        upper_value = std::clamp(upper_value, low, high);
        if (lower_value > upper_value) {
            upper_value = lower_value;
        }
        const float normalized_lower = lower_value;
        const float normalized_upper = upper_value;
        const float track_width = std::max(1.0f, rect.w);
        const float lower_ratio =
            high == low ? 0.0f : (lower_value - low) / (high - low);
        const float upper_ratio =
            high == low ? 0.0f : (upper_value - low) / (high - low);
        const float lower_x = rect.x + track_width * lower_ratio;
        const float upper_x = rect.x + track_width * upper_ratio;
        Rect lower_hit{lower_x - 7.0f, rect.y, 14.0f, rect.h};
        Rect upper_hit{upper_x - 7.0f, rect.y, 14.0f, rect.h};
        if (mouse_pressed_ &&
            static_cast<float>(mouse_x_) >= lower_hit.x &&
            static_cast<float>(mouse_x_) < lower_hit.x + lower_hit.w &&
            static_cast<float>(mouse_y_) >= lower_hit.y &&
            static_cast<float>(mouse_y_) < lower_hit.y + lower_hit.h &&
            static_cast<float>(mouse_x_) >= upper_hit.x &&
            static_cast<float>(mouse_x_) < upper_hit.x + upper_hit.w &&
            static_cast<float>(mouse_y_) >= upper_hit.y &&
            static_cast<float>(mouse_y_) < upper_hit.y + upper_hit.h) {
            if (std::abs(static_cast<float>(mouse_x_) - lower_x) <=
                std::abs(static_cast<float>(mouse_x_) - upper_x)) {
                upper_hit.w = 0.0f;
            } else {
                lower_hit.w = 0.0f;
            }
        }
        Interaction lower_result = interact(lower_id, lower_hit);
        Interaction upper_result = interact(upper_id, upper_hit);
        if (lower_result.active && mouse_down_) {
            const float ratio = std::clamp(
                (static_cast<float>(mouse_x_) - rect.x) / track_width, 0.0f, 1.0f);
            lower_value = std::min(low + ratio * (high - low), upper_value);
        }
        if (upper_result.active && mouse_down_) {
            const float ratio = std::clamp(
                (static_cast<float>(mouse_x_) - rect.x) / track_width, 0.0f, 1.0f);
            upper_value = std::max(low + ratio * (high - low), lower_value);
        }
        for (auto navigation = pending_navigation_.begin();
             navigation != pending_navigation_.end();) {
            if (navigation->first != lower_id &&
                navigation->first != upper_id) {
                ++navigation;
                continue;
            }
            const bool lower_focused = navigation->first == lower_id;
            float& value = lower_focused ? lower_value : upper_value;
            const float bound = lower_focused ? upper_value : lower_value;
            const float step = (high - low) / 100.0f;
            switch (navigation->second) {
                case Key::Left:
                case Key::Down:
                    value = std::max(lower_focused ? low : bound, value - step);
                    break;
                case Key::Right:
                case Key::Up:
                    value = std::min(lower_focused ? bound : high, value + step);
                    break;
                case Key::Home:
                    value = lower_focused ? low : bound;
                    break;
                case Key::End:
                    value = lower_focused ? bound : high;
                    break;
                default:
                    break;
            }
            navigation = pending_navigation_.erase(navigation);
        }
        Interaction result;
        result.hovered = lower_result.hovered || upper_result.hovered;
        result.active = lower_result.active || upper_result.active;
        result.clicked = lower_result.clicked || upper_result.clicked;
        result.changed = lower_value != normalized_lower ||
                         upper_value != normalized_upper;
        const float updated_lower_ratio =
            high == low ? 0.0f : (lower_value - low) / (high - low);
        const float updated_upper_ratio =
            high == low ? 0.0f : (upper_value - low) / (high - low);
        const float updated_lower_x = rect.x + track_width * updated_lower_ratio;
        const float updated_upper_x = rect.x + track_width * updated_upper_ratio;
        surface_.fill_rect({rect.x, rect.y + 10.0f, track_width, 8.0f},
                           theme_.window);
        surface_.fill_rect(
            {updated_lower_x, rect.y + 10.0f,
             std::max(0.0f, updated_upper_x - updated_lower_x), 8.0f},
            theme_.accent);
        const Rect updated_lower_hit{
            updated_lower_x - 7.0f, rect.y, 14.0f, rect.h};
        const Rect updated_upper_hit{
            updated_upper_x - 7.0f, rect.y, 14.0f, rect.h};
        const std::uint32_t lower_color =
            lower_result.active ? theme_.button_active
                                : lower_result.hovered ? theme_.button_hover
                                                       : theme_.text;
        const std::uint32_t upper_color =
            upper_result.active ? theme_.button_active
                                : upper_result.hovered ? theme_.button_hover
                                                       : theme_.text;
        surface_.fill_rect({updated_lower_x - 4.0f, rect.y + 6.0f, 8.0f, 16.0f},
                           lower_color);
        surface_.fill_rect({updated_upper_x - 4.0f, rect.y + 6.0f, 8.0f, 16.0f},
                           upper_color);
        draw_focus(lower_id, updated_lower_hit);
        draw_focus(upper_id, updated_upper_hit);
        last_widget_id_ = id;
        last_widget_hovered_ = result.hovered;
        return result;
    }
    // Draws a rotary value control adjusted by pointer angle or keyboard input.
    Interaction knob(const std::string& id_text, float& value, float minimum,
                     float maximum) {
        const std::uint64_t id = widget_id(id_text);
        const Rect rect = allocate(56.0f);
        const float low = std::min(minimum, maximum);
        const float high = std::max(minimum, maximum);
        const float old_value = std::clamp(value, low, high);
        value = old_value;
        const float center_x = rect.x + rect.w * 0.5f;
        const float center_y = rect.y + rect.h * 0.5f;
        const Rect hit_rect{center_x - 22.0f, center_y - 22.0f, 44.0f, 44.0f};
        Interaction result = interact(id, hit_rect);
        constexpr float pi = 3.14159265358979323846f;
        constexpr float start_angle = 3.0f * pi / 4.0f;
        constexpr float sweep_angle = 3.0f * pi / 2.0f;
        if (result.active && mouse_down_ && high > low) {
            const float dx = static_cast<float>(mouse_x_) - center_x;
            const float dy = static_cast<float>(mouse_y_) - center_y;
            if (dx != 0.0f || dy != 0.0f) {
                float angle = std::atan2(dy, dx);
                if (angle < start_angle) {
                    angle += 2.0f * pi;
                }
                const float ratio = std::clamp(
                    (angle - start_angle) / sweep_angle, 0.0f, 1.0f);
                value = low + ratio * (high - low);
            }
        }
        for (auto navigation = pending_navigation_.begin();
             navigation != pending_navigation_.end();) {
            if (navigation->first != id) {
                ++navigation;
                continue;
            }
            const float step = (high - low) / 100.0f;
            switch (navigation->second) {
                case Key::Left:
                case Key::Down:
                    value = std::max(low, value - step);
                    break;
                case Key::Right:
                case Key::Up:
                    value = std::min(high, value + step);
                    break;
                case Key::Home:
                    value = low;
                    break;
                case Key::End:
                    value = high;
                    break;
                default:
                    break;
            }
            navigation = pending_navigation_.erase(navigation);
        }
        result.changed = value != old_value;
        const float ratio = high == low ? 0.0f : (value - low) / (high - low);
        constexpr int arc_segments = 32;
        for (int segment = 0; segment < arc_segments; ++segment) {
            const float segment_start =
                static_cast<float>(segment) / static_cast<float>(arc_segments);
            const float segment_end =
                static_cast<float>(segment + 1) /
                static_cast<float>(arc_segments);
            const float start_x =
                center_x + std::cos(start_angle + sweep_angle * segment_start) * 16.0f;
            const float start_y =
                center_y + std::sin(start_angle + sweep_angle * segment_start) * 16.0f;
            const float end_x =
                center_x + std::cos(start_angle + sweep_angle * segment_end) * 16.0f;
            const float end_y =
                center_y + std::sin(start_angle + sweep_angle * segment_end) * 16.0f;
            const std::uint32_t color =
                segment_end <= ratio ? theme_.accent : theme_.border;
            surface_.draw_line(static_cast<int>(start_x), static_cast<int>(start_y),
                               static_cast<int>(end_x), static_cast<int>(end_y),
                               color);
        }
        surface_.draw_filled_circle(static_cast<int>(center_x),
                                    static_cast<int>(center_y), 10,
                                    theme_.button);
        const float indicator_angle = start_angle + sweep_angle * ratio;
        surface_.draw_line(
            static_cast<int>(center_x), static_cast<int>(center_y),
            static_cast<int>(center_x + std::cos(indicator_angle) * 9.0f),
            static_cast<int>(center_y + std::sin(indicator_angle) * 9.0f),
            result.active ? theme_.button_active
                          : result.hovered ? theme_.button_hover : theme_.text);
        draw_focus(id, hit_rect);
        return result;
    }
    // Draws a value row adjusted by horizontal pointer movement or keyboard input.
    Interaction drag_value(const std::string& id_text, float& value,
                           float minimum, float maximum,
                           float sensitivity = 0.01f) {
        const std::uint64_t id = widget_id(id_text);
        const Rect rect = allocate(28.0f);
        Interaction result = interact(id, rect);
        const float low = std::min(minimum, maximum);
        const float high = std::max(minimum, maximum);
        const float old_value = std::clamp(value, low, high);
        value = old_value;
        const float step = std::max(0.0f, sensitivity);
        if (result.active && mouse_down_) {
            if (mouse_pressed_) {
                drag_value_last_x_ = mouse_x_;
            } else {
                const std::int64_t delta = static_cast<std::int64_t>(mouse_x_) -
                                           static_cast<std::int64_t>(
                                               drag_value_last_x_);
                drag_value_last_x_ = mouse_x_;
                value = std::clamp(value + static_cast<float>(delta) * step,
                                   low, high);
            }
        }
        for (auto navigation = pending_navigation_.begin();
             navigation != pending_navigation_.end();) {
            if (navigation->first != id) {
                ++navigation;
                continue;
            }
            switch (navigation->second) {
                case Key::Left:
                case Key::Down:
                    value = std::max(low, value - step);
                    break;
                case Key::Right:
                case Key::Up:
                    value = std::min(high, value + step);
                    break;
                case Key::Home:
                    value = low;
                    break;
                case Key::End:
                    value = high;
                    break;
                default:
                    break;
            }
            navigation = pending_navigation_.erase(navigation);
        }
        result.changed = value != old_value;
        surface_.fill_rect(rect, result.active ? theme_.button_active
                                               : result.hovered ? theme_.button_hover
                                                                : theme_.button);
        surface_.draw_text(static_cast<int>(rect.x + theme_.padding),
                           static_cast<int>(rect.y + 6.0f), id_text,
                           theme_.text, theme_.font_scale);
        const float track_width = rect.w * 0.4f;
        const float track_x = rect.x + rect.w - track_width -
                              static_cast<float>(theme_.padding);
        const float ratio = high == low ? 0.0f : (value - low) / (high - low);
        surface_.fill_rect({track_x, rect.y + 10.0f, track_width, 8.0f},
                           theme_.window);
        surface_.fill_rect({track_x, rect.y + 10.0f, track_width * ratio, 8.0f},
                           theme_.accent);
        draw_focus(id, rect);
        return result;
    }
    // Draws a two-dimensional value pad with positive Y toward the top.
    Interaction xy_pad(const std::string& id_text, float& x_value,
                       float& y_value, float minimum_x, float maximum_x,
                       float minimum_y, float maximum_y,
                       float height = 120.0f) {
        const std::uint64_t id = widget_id(id_text);
        const Rect rect = allocate(std::max(40.0f, height));
        Interaction result = interact(id, rect);
        const float low_x = std::min(minimum_x, maximum_x);
        const float high_x = std::max(minimum_x, maximum_x);
        const float low_y = std::min(minimum_y, maximum_y);
        const float high_y = std::max(minimum_y, maximum_y);
        const float old_x = std::clamp(x_value, low_x, high_x);
        const float old_y = std::clamp(y_value, low_y, high_y);
        x_value = old_x;
        y_value = old_y;
        const float inset = std::min(
            8.0f, std::max(0.0f, std::min(rect.w, rect.h) * 0.25f));
        const Rect area{rect.x + inset, rect.y + inset,
                        std::max(0.0f, rect.w - 2.0f * inset),
                        std::max(0.0f, rect.h - 2.0f * inset)};
        if (result.active && mouse_down_ && area.w > 0.0f && area.h > 0.0f) {
            const float ratio_x = std::clamp(
                (static_cast<float>(mouse_x_) - area.x) / area.w, 0.0f, 1.0f);
            const float ratio_y = std::clamp(
                1.0f - (static_cast<float>(mouse_y_) - area.y) / area.h,
                0.0f, 1.0f);
            if (high_x > low_x) {
                x_value = low_x + ratio_x * (high_x - low_x);
            }
            if (high_y > low_y) {
                y_value = low_y + ratio_y * (high_y - low_y);
            }
        }
        for (auto navigation = pending_navigation_.begin();
             navigation != pending_navigation_.end();) {
            if (navigation->first != id) {
                ++navigation;
                continue;
            }
            const float step_x = (high_x - low_x) / 100.0f;
            const float step_y = (high_y - low_y) / 100.0f;
            switch (navigation->second) {
                case Key::Left:
                    x_value = std::max(low_x, x_value - step_x);
                    break;
                case Key::Right:
                    x_value = std::min(high_x, x_value + step_x);
                    break;
                case Key::Down:
                    y_value = std::max(low_y, y_value - step_y);
                    break;
                case Key::Up:
                    y_value = std::min(high_y, y_value + step_y);
                    break;
                case Key::Home:
                    x_value = low_x;
                    y_value = low_y;
                    break;
                case Key::End:
                    x_value = high_x;
                    y_value = high_y;
                    break;
                default:
                    break;
            }
            navigation = pending_navigation_.erase(navigation);
        }
        result.changed = x_value != old_x || y_value != old_y;
        surface_.fill_rect(rect, theme_.border);
        surface_.fill_rect(area, theme_.window);
        for (int division = 1; division < 3; ++division) {
            const float fraction = static_cast<float>(division) / 3.0f;
            const int grid_x =
                static_cast<int>(area.x + area.w * fraction);
            const int grid_y =
                static_cast<int>(area.y + area.h * fraction);
            surface_.draw_line(grid_x, static_cast<int>(area.y), grid_x,
                               static_cast<int>(area.y + area.h) - 1,
                               theme_.border);
            surface_.draw_line(static_cast<int>(area.x), grid_y,
                               static_cast<int>(area.x + area.w) - 1, grid_y,
                               theme_.border);
        }
        const float ratio_x =
            high_x == low_x ? 0.5f : (x_value - low_x) / (high_x - low_x);
        const float ratio_y =
            high_y == low_y ? 0.5f : (y_value - low_y) / (high_y - low_y);
        const int cursor_x =
            static_cast<int>(area.x + ratio_x * area.w);
        const int cursor_y =
            static_cast<int>(area.y + (1.0f - ratio_y) * area.h);
        surface_.draw_line(cursor_x, static_cast<int>(area.y), cursor_x,
                           static_cast<int>(area.y + area.h) - 1,
                           theme_.accent);
        surface_.draw_line(static_cast<int>(area.x), cursor_y,
                           static_cast<int>(area.x + area.w) - 1, cursor_y,
                           theme_.accent);
        surface_.draw_filled_circle(cursor_x, cursor_y, 4, theme_.text);
        draw_focus(id, rect);
        return result;
    }
    // Draws a vertical slider with its minimum at the bottom and maximum at the top.
    Interaction vertical_slider(const std::string& id_text, float& value,
                                float minimum, float maximum,
                                float height = 120.0f) {
        const std::uint64_t id = widget_id(id_text);
        const Rect rect = allocate(std::max(28.0f, height));
        Interaction result = interact(id, rect);
        const float low = std::min(minimum, maximum);
        const float high = std::max(minimum, maximum);
        const float old_value = std::clamp(value, low, high);
        value = old_value;
        const float track_width = 12.0f;
        const float track_height = std::max(1.0f, rect.h - 16.0f);
        const float track_x = rect.x + (rect.w - track_width) * 0.5f;
        const float track_y = rect.y + 8.0f;
        if (result.active && mouse_down_) {
            const float ratio = std::clamp(
                1.0f - (static_cast<float>(mouse_y_) - track_y) / track_height,
                0.0f, 1.0f);
            value = low + ratio * (high - low);
        }
        for (auto navigation = pending_navigation_.begin();
             navigation != pending_navigation_.end();) {
            if (navigation->first != id) {
                ++navigation;
                continue;
            }
            const float step = (high - low) / 100.0f;
            switch (navigation->second) {
                case Key::Left:
                case Key::Down:
                    value = std::max(low, value - step);
                    break;
                case Key::Right:
                case Key::Up:
                    value = std::min(high, value + step);
                    break;
                case Key::Home:
                    value = low;
                    break;
                case Key::End:
                    value = high;
                    break;
                default:
                    break;
            }
            navigation = pending_navigation_.erase(navigation);
        }
        result.changed = value != old_value;
        const float ratio = high == low ? 0.0f : (value - low) / (high - low);
        const float fill_height = track_height * ratio;
        surface_.fill_rect(rect, theme_.window);
        surface_.fill_rect({track_x, track_y, track_width, track_height},
                           theme_.border);
        surface_.fill_rect({track_x, track_y + track_height - fill_height,
                            track_width, fill_height}, theme_.accent);
        const float knob_y = track_y + track_height * (1.0f - ratio) - 4.0f;
        surface_.fill_rect({track_x - 6.0f, knob_y, track_width + 12.0f, 8.0f},
                           theme_.text);
        draw_focus(id, rect);
        return result;
    }
    // Draws a bounded integer stepper controlled by pointer buttons or navigation keys.
    Interaction stepper(const std::string& id_text, int& value, int minimum,
                        int maximum, int step = 1) {
        const std::uint64_t id = widget_id(id_text);
        const Rect rect = allocate(28.0f);
        Interaction result = interact(id, rect);
        const std::int64_t low = static_cast<std::int64_t>(
            std::min(minimum, maximum));
        const std::int64_t high = static_cast<std::int64_t>(
            std::max(minimum, maximum));
        const std::int64_t increment =
            static_cast<std::int64_t>(std::max(1, step));
        const int original_value = value;
        value = static_cast<int>(std::clamp(
            static_cast<std::int64_t>(value), low, high));
        const int bounded_value = value;
        const float button_width = std::min(32.0f, rect.w * 0.5f);
        if (result.clicked && button_width > 0.0f) {
            if (static_cast<float>(mouse_x_) < rect.x + button_width) {
                const std::int64_t candidate =
                    static_cast<std::int64_t>(value) - increment;
                value = static_cast<int>(std::max(low, candidate));
            } else if (static_cast<float>(mouse_x_) >=
                       rect.x + rect.w - button_width) {
                const std::int64_t candidate =
                    static_cast<std::int64_t>(value) + increment;
                value = static_cast<int>(std::min(high, candidate));
            }
        }
        for (auto navigation = pending_navigation_.begin();
             navigation != pending_navigation_.end();) {
            if (navigation->first != id) {
                ++navigation;
                continue;
            }
            switch (navigation->second) {
                case Key::Left:
                case Key::Down: {
                    const std::int64_t candidate =
                        static_cast<std::int64_t>(value) - increment;
                    value = static_cast<int>(std::max(low, candidate));
                    break;
                }
                case Key::Right:
                case Key::Up: {
                    const std::int64_t candidate =
                        static_cast<std::int64_t>(value) + increment;
                    value = static_cast<int>(std::min(high, candidate));
                    break;
                }
                case Key::Home:
                    value = static_cast<int>(low);
                    break;
                case Key::End:
                    value = static_cast<int>(high);
                    break;
                default:
                    break;
            }
            navigation = pending_navigation_.erase(navigation);
        }
        result.changed = value != original_value;
        const std::uint32_t button_color =
            result.active ? theme_.button_active
                          : result.hovered ? theme_.button_hover : theme_.button;
        surface_.fill_rect(rect, theme_.window);
        surface_.fill_rect({rect.x, rect.y, button_width, rect.h}, button_color);
        surface_.fill_rect({rect.x + rect.w - button_width, rect.y,
                            button_width, rect.h}, button_color);
        surface_.draw_text(static_cast<int>(rect.x + button_width * 0.5f - 4.0f),
                           static_cast<int>(rect.y + 6.0f), "-", theme_.text,
                           theme_.font_scale);
        surface_.draw_text(
            static_cast<int>(rect.x + rect.w - button_width * 0.5f - 4.0f),
            static_cast<int>(rect.y + 6.0f), "+", theme_.text,
            theme_.font_scale);
        const std::string value_text = std::to_string(value);
        const Rect value_size = surface_.measure_text(value_text, theme_.font_scale);
        const int text_x = static_cast<int>(
            rect.x + (rect.w - value_size.w) * 0.5f);
        surface_.draw_text(text_x, static_cast<int>(rect.y + 6.0f),
                           value_text, theme_.text, theme_.font_scale);
        draw_focus(id, rect);
        result.changed = result.changed || value != bounded_value;
        return result;
    }
    // Begins a dockspace, draws its tabs, and handles splitter and tab dragging.
    void begin_dockspace(DockspaceState& state, Rect rect) {
        clear_last_widget();
        if (active_dockspace_ != nullptr) {
            end_dockspace();
        }
        state.rect = rect;
        state.left_ratio = std::isfinite(state.left_ratio)
                               ? std::clamp(state.left_ratio, 0.15f, 0.85f)
                               : 0.35f;
        detail::DockspaceGeometry geometry = dockspace_geometry(state);

        if (state.dragging_splitter) {
            if (mouse_down_) {
                const float usable_width = geometry.left.w + geometry.right.w;
                if (usable_width > 0.0f) {
                    state.left_ratio = std::clamp(
                        (static_cast<float>(mouse_x_) - geometry.inner.x) /
                            usable_width,
                        0.15f, 0.85f);
                }
            } else {
                state.dragging_splitter = false;
            }
        } else if (mouse_pressed_ &&
                   contains_point(geometry.splitter, mouse_x_, mouse_y_) &&
                   contains_point(input_clips_.back(), mouse_x_, mouse_y_)) {
            state.dragging_splitter = true;
        }

        geometry = dockspace_geometry(state);
        if (!state.dragging_panel.empty()) {
            if (mouse_released_) {
                const auto panel = std::find_if(
                    state.panels.begin(), state.panels.end(),
                    [&state](const DockPanel& candidate) {
                        return candidate.id == state.dragging_panel;
                    });
                if (panel != state.panels.end() &&
                    contains_point(state.rect, mouse_x_, mouse_y_) &&
                    contains_point(input_clips_.back(), mouse_x_, mouse_y_)) {
                    const DockSide target =
                        static_cast<float>(mouse_x_) <
                                geometry.splitter.x + geometry.splitter.w * 0.5f
                            ? DockSide::Left
                            : DockSide::Right;
                    panel->side = target;
                    if (target == DockSide::Left) {
                        state.active_left = panel->id;
                    } else {
                        state.active_right = panel->id;
                    }
                }
                state.dragging_panel.clear();
            }
        } else if (mouse_pressed_ &&
                   contains_point(input_clips_.back(), mouse_x_, mouse_y_)) {
            const auto select_tab = [this, &state](DockSide side, Rect group) {
                if (static_cast<float>(mouse_y_) < group.y ||
                    static_cast<float>(mouse_y_) >= group.y + 24.0f ||
                    static_cast<float>(mouse_x_) < group.x ||
                    static_cast<float>(mouse_x_) >= group.x + group.w) {
                    return false;
                }
                std::vector<std::size_t> members;
                for (std::size_t index = 0; index < state.panels.size(); ++index) {
                    if (state.panels[index].side == side) {
                        members.push_back(index);
                    }
                }
                if (members.empty()) {
                    return false;
                }
                const float tab_width =
                    group.w / static_cast<float>(members.size());
                if (tab_width <= 0.0f) {
                    return false;
                }
                const std::size_t tab_index = std::min(
                    members.size() - 1U,
                    static_cast<std::size_t>(
                        (static_cast<float>(mouse_x_) - group.x) / tab_width));
                const DockPanel& panel = state.panels[members[tab_index]];
                if (side == DockSide::Left) {
                    state.active_left = panel.id;
                } else {
                    state.active_right = panel.id;
                }
                state.dragging_panel = panel.id;
                return true;
            };
            if (!select_tab(DockSide::Left, geometry.left)) {
                select_tab(DockSide::Right, geometry.right);
            }
        }

        geometry = dockspace_geometry(state);
        ensure_active_dock_panel(state, DockSide::Left);
        ensure_active_dock_panel(state, DockSide::Right);
        active_dockspace_ = &state;
        active_dockspace_geometry_ = geometry;
        surface_.push_clip(rect);
        surface_.fill_rect(rect, theme_.border);
        surface_.fill_rect(geometry.inner, theme_.window);
        surface_.fill_rect(geometry.splitter, theme_.border);
        draw_dock_tabs(state, DockSide::Left, geometry.left,
                       state.active_left);
        draw_dock_tabs(state, DockSide::Right, geometry.right,
                       state.active_right);
        if (group_panel_count(state, DockSide::Left) == 0U) {
            surface_.draw_text(static_cast<int>(geometry.left.x + theme_.padding),
                               static_cast<int>(geometry.left.y + 32.0f),
                               "Drop panel here", theme_.text, theme_.font_scale);
        }
        if (group_panel_count(state, DockSide::Right) == 0U) {
            surface_.draw_text(static_cast<int>(geometry.right.x + theme_.padding),
                               static_cast<int>(geometry.right.y + 32.0f),
                               "Drop panel here", theme_.text, theme_.font_scale);
        }
    }
    // Draws the active panel body through a callback and clips it to its dock group.
    template <typename Callback>
    bool dock_panel(const std::string& id, Callback&& callback) {
        if (active_dockspace_ == nullptr) {
            return false;
        }
        const auto panel = std::find_if(
            active_dockspace_->panels.begin(), active_dockspace_->panels.end(),
            [&id](const DockPanel& candidate) { return candidate.id == id; });
        if (panel == active_dockspace_->panels.end()) {
            return false;
        }
        const std::string& active =
            panel->side == DockSide::Left ? active_dockspace_->active_left
                                          : active_dockspace_->active_right;
        if (active != id) {
            return false;
        }

        const Rect content = panel->side == DockSide::Left
                                 ? active_dockspace_geometry_.left_content
                                 : active_dockspace_geometry_.right_content;
        const Rect previous_root = root_;
        const float previous_x = cursor_x_;
        const float previous_y = cursor_y_;
        const float previous_width = content_width_;
        surface_.push_clip(content);
        input_clips_.push_back(detail::intersect(input_clips_.back(), content));
        id_stack_.push_back(hash(id, id_stack_.back()));
        root_ = content;
        cursor_x_ = content.x + static_cast<float>(theme_.padding);
        cursor_y_ = content.y + static_cast<float>(theme_.padding);
        content_width_ =
            content.w - 2.0f * static_cast<float>(theme_.padding);
        std::forward<Callback>(callback)();
        id_stack_.pop_back();
        input_clips_.pop_back();
        surface_.pop_clip();
        root_ = previous_root;
        cursor_x_ = previous_x;
        cursor_y_ = previous_y;
        content_width_ = previous_width;
        return true;
    }
    // Ends the current dockspace and restores the surface clip stack.
    void end_dockspace() {
        if (active_dockspace_ == nullptr) {
            return;
        }
        surface_.pop_clip();
        active_dockspace_ = nullptr;
    }
    // Draws a titled panel using caller-owned position and drag state.
    template <typename Callback>
    void window(const std::string& title, WindowState& state, Callback&& callback) {
        constexpr float title_height = 28.0f;
        const Rect title_bar{state.rect.x, state.rect.y, state.rect.w, title_height};
        const bool pointer_in_parent =
            contains_point(input_clips_.back(), mouse_x_, mouse_y_);
        if (mouse_pressed_ && pointer_in_parent &&
            contains_point(title_bar, mouse_x_, mouse_y_)) {
            state.dragging = true;
            state.drag_offset_x = static_cast<float>(mouse_x_) - state.rect.x;
            state.drag_offset_y = static_cast<float>(mouse_y_) - state.rect.y;
        }
        if (state.dragging) {
            if (mouse_down_ || mouse_released_) {
                const float maximum_x =
                    std::max(0.0f, static_cast<float>(surface_.width()) - state.rect.w);
                const float maximum_y =
                    std::max(0.0f, static_cast<float>(surface_.height()) -
                                       title_height - 1.0f);
                state.rect.x = std::clamp(static_cast<float>(mouse_x_) -
                                              state.drag_offset_x,
                                          0.0f, maximum_x);
                state.rect.y = std::clamp(static_cast<float>(mouse_y_) -
                                              state.drag_offset_y,
                                          0.0f, maximum_y);
            }
            if (mouse_released_) {
                state.dragging = false;
            }
        }
        window(title, state.rect, std::forward<Callback>(callback));
    }

    // Draws a titled panel, scopes its ID, and lays out its contents vertically.
    template <typename Callback>
    void window(const std::string& title, Rect rect, Callback&& callback) {
        const Rect previous_root = root_;
        const float previous_x = cursor_x_;
        const float previous_y = cursor_y_;
        const float previous_width = content_width_;
        surface_.fill_rect(rect, theme_.border);
        surface_.fill_rect({rect.x + 1.0f, rect.y + 1.0f, rect.w - 2.0f, rect.h - 2.0f},
                           theme_.window);
        const float title_height = 28.0f;
        surface_.fill_rect({rect.x + 1.0f, rect.y + 1.0f, rect.w - 2.0f, title_height},
                           theme_.title_bar);
        surface_.draw_text(static_cast<int>(rect.x + theme_.padding),
                           static_cast<int>(rect.y + 7.0f), title, theme_.text,
                           theme_.font_scale);
        id_stack_.push_back(hash(title, id_stack_.back()));
        const Rect content_clip{rect.x + 1.0f, rect.y + title_height + 1.0f,
                                rect.w - 2.0f, rect.h - title_height - 2.0f};
        surface_.push_clip(content_clip);
        input_clips_.push_back(detail::intersect(input_clips_.back(), content_clip));
        root_ = rect;
        cursor_x_ = rect.x + static_cast<float>(theme_.padding);
        cursor_y_ = rect.y + title_height + static_cast<float>(theme_.padding);
        content_width_ = rect.w - 2.0f * static_cast<float>(theme_.padding);
        std::forward<Callback>(callback)();
        surface_.pop_clip();
        input_clips_.pop_back();
        id_stack_.pop_back();
        root_ = previous_root;
        cursor_x_ = previous_x;
        cursor_y_ = previous_y;
        content_width_ = previous_width;
    }
    // Begins a clipped, vertically scrollable child region at a surface rectangle.
    void begin_child(const std::string& child_name, Rect rect) {
        const std::uint64_t id = widget_id(child_name);
        ChildState& state = child_states_[id];
        const Rect parent_clip = input_clips_.back();
        const Rect visible_clip = detail::intersect(parent_clip, rect);
        frame_child_hits_.push_back(
            {id, rect, visible_clip, child_stack_.size()});
        const bool hovered = (pointer_claimed_id_ == 0U || pointer_claimed_id_ == id) &&
                             !popup_input_blocked_ &&
                             contains_point(rect, mouse_x_, mouse_y_) &&
                             contains_point(parent_clip, mouse_x_, mouse_y_);
        register_focusable(id);
        if (hovered) {
            hot_id_ = id;
            if (mouse_pressed_) {
                active_id_ = id;
                focused_id_ = id;
                focused_text_id_ = 0U;
            }
        }
        if (mouse_released_ && active_id_ == id) {
            active_id_ = 0U;
        }
        if (child_wheel_target_ == id && hovered && pending_wheel_delta_ != 0) {
            const std::int64_t total_delta =
                saturating_add(pending_wheel_delta_, wheel_remainder_);
            const std::int64_t row_delta = total_delta / 120;
            wheel_remainder_ = total_delta % 120;
            pending_wheel_delta_ = 0;
            state.scroll_y -= static_cast<float>(row_delta) * 24.0f;
        }
        const float visible_height =
            std::max(0.0f, rect.h - 2.0f * static_cast<float>(theme_.padding));
        float maximum_scroll =
            std::max(0.0f, state.content_height - visible_height);
        for (auto navigation = pending_navigation_.begin();
             navigation != pending_navigation_.end();) {
            if (navigation->first != id) {
                ++navigation;
                continue;
            }
            switch (navigation->second) {
                case Key::Up:
                    state.scroll_y -= 24.0f;
                    break;
                case Key::Down:
                    state.scroll_y += 24.0f;
                    break;
                case Key::Home:
                    state.scroll_y = 0.0f;
                    break;
                case Key::End:
                    state.scroll_y = maximum_scroll;
                    break;
                default:
                    break;
            }
            navigation = pending_navigation_.erase(navigation);
        }
        state.scroll_y = std::clamp(state.scroll_y, 0.0f, maximum_scroll);
        surface_.fill_rect(rect, theme_.border);
        surface_.fill_rect({rect.x + 1.0f, rect.y + 1.0f, rect.w - 2.0f,
                            rect.h - 2.0f},
                           theme_.window);
        child_stack_.push_back(
            {id, rect, root_, cursor_x_, cursor_y_, content_width_});
        id_stack_.push_back(hash(child_name, id_stack_.back()));
        surface_.push_clip(rect);
        input_clips_.push_back(visible_clip);
        root_ = rect;
        cursor_x_ = rect.x + static_cast<float>(theme_.padding);
        cursor_y_ = rect.y + static_cast<float>(theme_.padding) - state.scroll_y;
        content_width_ =
            rect.w - 2.0f * static_cast<float>(theme_.padding);
    }
    // Ends the current child region and restores its parent's layout and clip.
    void end_child() {
        if (child_stack_.empty()) {
            return;
        }
        const ChildFrame frame = child_stack_.back();
        ChildState& state = child_states_[frame.id];
        state.content_height = std::max(
            0.0f, cursor_y_ + state.scroll_y -
                      (frame.rect.y + static_cast<float>(theme_.padding)));
        const float visible_height =
            std::max(0.0f, frame.rect.h -
                               2.0f * static_cast<float>(theme_.padding));
        state.scroll_y = std::clamp(
            state.scroll_y, 0.0f,
            std::max(0.0f, state.content_height - visible_height));
        draw_focus(frame.id, frame.rect);
        surface_.pop_clip();
        input_clips_.pop_back();
        id_stack_.pop_back();
        child_stack_.pop_back();
        root_ = frame.previous_root;
        cursor_x_ = frame.previous_cursor_x;
        cursor_y_ = frame.previous_cursor_y;
        content_width_ = frame.previous_content_width;
    }
    // Begins an evenly divided table and scopes IDs to the table label.
    void begin_table(const std::string& table_id, std::size_t columns) {
        const std::size_t column_count = std::max<std::size_t>(1U, columns);
        const float gap = static_cast<float>(std::max(0, theme_.padding));
        const float available_width = std::max(0.0f, content_width_);
        const float cell_width = std::max(
            0.0f, (available_width -
                   gap * static_cast<float>(column_count - 1U)) /
                      static_cast<float>(column_count));
        table_stack_.push_back(
            {cursor_x_, content_width_, cell_width, gap, column_count, 0U, 0U,
             cursor_y_, cursor_y_, 0U});
        TableFrame& frame = table_stack_.back();
        frame.table_scope_id = hash(table_id, id_stack_.back());
        id_stack_.push_back(frame.table_scope_id);
        id_stack_.push_back(table_cell_scope(frame));
        content_width_ = cell_width;
    }
    // Advances to the next evenly sized cell in the current table row.
    void next_table_column() {
        if (table_stack_.empty()) {
            return;
        }
        TableFrame& frame = table_stack_.back();
        frame.row_bottom_y = std::max(frame.row_bottom_y, cursor_y_);
        if (frame.current_column + 1U >= frame.column_count) {
            return;
        }
        ++frame.current_column;
        cursor_x_ = frame.previous_cursor_x +
                    static_cast<float>(frame.current_column) *
                        (frame.cell_width + frame.column_gap);
        cursor_y_ = frame.row_start_y;
        content_width_ = frame.cell_width;
        id_stack_.back() = table_cell_scope(frame);
    }
    // Advances to a new table row after all used cells have been measured.
    void next_table_row() {
        if (table_stack_.empty()) {
            return;
        }
        TableFrame& frame = table_stack_.back();
        frame.row_bottom_y = std::max(frame.row_bottom_y, cursor_y_);
        frame.row_start_y = frame.row_bottom_y;
        frame.row_bottom_y = frame.row_start_y;
        frame.current_column = 0U;
        ++frame.row_index;
        cursor_x_ = frame.previous_cursor_x;
        cursor_y_ = frame.row_start_y;
        content_width_ = frame.cell_width;
        id_stack_.back() = table_cell_scope(frame);
    }
    // Ends a table and restores its enclosing layout and ID scope.
    void end_table() {
        if (table_stack_.empty()) {
            return;
        }
        const TableFrame frame = table_stack_.back();
        table_stack_.pop_back();
        id_stack_.pop_back();
        id_stack_.pop_back();
        const float row_bottom = std::max(frame.row_bottom_y, cursor_y_);
        cursor_x_ = frame.previous_cursor_x;
        cursor_y_ = row_bottom;
        content_width_ = frame.previous_content_width;
    }
    // Allocates a layout row with the requested height and advances the cursor.
    Rect row(float height = 24.0f) { return allocate(height); }
    // Allocates a column of the requested width and advances the horizontal cursor.
    Rect column(float width = 100.0f) {
        const Rect result{cursor_x_, cursor_y_, width, 24.0f};
        cursor_x_ += width + static_cast<float>(theme_.padding);
        return result;
    }
    // Moves the vertical layout cursor to the next row.
    void next_row(float height = 24.0f) {
        cursor_y_ += height + static_cast<float>(theme_.padding);
    }

    // Inserts a character at the focused text cursor.
    void feed_char(char character) {
        if (focused_text_id_ != 0U && focused_id_ == focused_text_id_ &&
            edit_cursor_ <= edit_buffer_.size()) {
            edit_buffer_.insert(edit_cursor_, 1U, character);
            ++edit_cursor_;
            edit_changed_ = true;
            vertical_goal_valid_ = false;
        }
    }
    // Queues a wheel delta in 120-unit notches; positive deltas scroll lists up.
    void feed_wheel(int wheel_delta) {
        pending_wheel_delta_ = saturating_add(
            pending_wheel_delta_, static_cast<std::int64_t>(wheel_delta));
    }
    // Applies a navigation or editing key to the focused text cursor.
    void feed_key(Key key, bool shift = false) {
        if (key == Key::Tab) {
            move_focus(shift);
            return;
        }
        if (key == Key::Escape && open_combo_id_ != 0U &&
            focused_id_ == open_combo_id_) {
            open_combo_id_ = 0U;
            keyboard_activation_id_ = 0U;
            pending_navigation_.clear();
            return;
        }
        if (focused_text_id_ != 0U && focused_id_ == focused_text_id_) {
            feed_text_key(key);
            return;
        }
        if (focused_id_ == 0U) {
            return;
        }
        if (key == Key::Enter || key == Key::Space) {
            keyboard_activation_id_ = focused_id_;
            return;
        }
        if (key == Key::Left || key == Key::Right || key == Key::Up ||
            key == Key::Down || key == Key::Home || key == Key::End) {
            pending_navigation_.emplace_back(focused_id_, key);
        }
    }

    // Returns the focused text cursor position.
    std::size_t cursor() const { return edit_cursor_; }
    // Draws a single-line text field and synchronizes its value with the edit buffer.
    Interaction text_input(const std::string& label_text, std::string& value,
                           std::size_t maximum_length = 64U) {
        const std::uint64_t id = widget_id(label_text);
        edit_maximum_length_ = maximum_length;
        if (focused_id_ == id) {
            multiline_text_id_ = 0U;
        }
        const Rect rect = allocate(28.0f);
        Interaction result = interact(id, rect);
        if (focused_id_ == id && focused_text_id_ != id) {
            focused_text_id_ = id;
            edit_buffer_ = value;
            edit_cursor_ = edit_buffer_.size();
            edit_changed_ = false;
            vertical_goal_valid_ = false;
        }
        if (focused_id_ == id) {
            if (edit_buffer_.size() > maximum_length) {
                edit_buffer_.resize(maximum_length);
                edit_cursor_ = std::min(edit_cursor_, edit_buffer_.size());
            }
            value = edit_buffer_;
            result.changed = edit_changed_;
            edit_changed_ = false;
        }
        surface_.fill_rect(rect, theme_.window);
        const std::string displayed_text = label_text + ": " + value;
        const int font_scale = std::max(1, theme_.font_scale);
        const int glyph_width = 8 * font_scale;
        const float content_left = rect.x + static_cast<float>(theme_.padding);
        const float content_width =
            std::max(0.0f, rect.w - 2.0f * static_cast<float>(theme_.padding));
        const float glyph_width_float = static_cast<float>(glyph_width);
        const std::size_t visible_characters =
            static_cast<std::size_t>(content_width / glyph_width_float);
        const std::size_t text_cursor =
            label_text.size() + 2U +
            ((focused_text_id_ == id && focused_id_ == id)
                 ? std::min(edit_cursor_, value.size())
                 : value.size());
        const float caret_room = std::max(0.0f, content_width - 1.0f);
        const std::size_t maximum_caret_column =
            static_cast<std::size_t>(caret_room / glyph_width_float);
        const std::size_t first_visible_character =
            text_cursor > maximum_caret_column
                ? text_cursor - maximum_caret_column
                : 0U;
        const std::size_t visible_end =
            std::min(displayed_text.size(),
                     first_visible_character + visible_characters);
        const std::string visible_text =
            displayed_text.substr(first_visible_character,
                                  visible_end - first_visible_character);
        surface_.push_clip(rect);
        surface_.draw_text(static_cast<int>(content_left),
                           static_cast<int>(rect.y + 6.0f), visible_text,
                           theme_.text, font_scale);
        if (focused_text_id_ == id && focused_id_ == id && rect.w > 0.0f) {
            const std::size_t caret_column = text_cursor - first_visible_character;
            const float maximum_caret_x = std::max(rect.x, rect.x + rect.w - 1.0f);
            const int caret_x = static_cast<int>(std::min(
                content_left + static_cast<float>(caret_column) * glyph_width_float,
                maximum_caret_x));
            const int caret_y = static_cast<int>(rect.y + 5.0f);
            surface_.draw_line(caret_x, caret_y, caret_x,
                               caret_y + 17 * font_scale,
                               theme_.accent);
        }
        surface_.pop_clip();
        draw_focus(id, rect);
        return result;
    }
    // Draws a vertically scrollable multi-line text editor with line-aware navigation.
    Interaction multiline_text_input(const std::string& label_text,
                                     std::string& value,
                                     std::size_t visible_lines = 4U,
                                     std::size_t maximum_length = 4096U) {
        const std::uint64_t id = widget_id(label_text);
        const int font_scale = std::max(1, theme_.font_scale);
        const float line_height = 16.0f * static_cast<float>(font_scale);
        const std::size_t displayed_lines = std::max<std::size_t>(1U, visible_lines);
        const float height =
            static_cast<float>(displayed_lines) * line_height +
            2.0f * static_cast<float>(theme_.padding);
        const Rect rect = allocate(height);
        Interaction result = interact(id, rect);
        edit_maximum_length_ = maximum_length;
        if (focused_id_ == id && focused_text_id_ != id) {
            focused_text_id_ = id;
            multiline_text_id_ = id;
            edit_buffer_ = value;
            edit_cursor_ = edit_buffer_.size();
            edit_changed_ = false;
            vertical_goal_valid_ = false;
        }
        if (focused_id_ == id) {
            multiline_text_id_ = id;
            if (edit_buffer_.size() > maximum_length) {
                edit_buffer_.resize(maximum_length);
                edit_cursor_ = std::min(edit_cursor_, edit_buffer_.size());
            }
            value = edit_buffer_;
            result.changed = edit_changed_;
            edit_changed_ = false;
        }
        ChildState& scroll_state = child_states_[id];
        std::size_t line_count = 1U;
        for (const char character : value) {
            if (character == '\n') {
                ++line_count;
            }
        }
        scroll_state.content_height =
            static_cast<float>(line_count) * line_height;
        const Rect visible_clip = detail::intersect(input_clips_.back(), rect);
        frame_child_hits_.push_back(
            {id, rect, visible_clip, child_stack_.size()});
        if (child_wheel_target_ == id && !popup_input_blocked_ &&
            contains_point(rect, mouse_x_, mouse_y_) && pending_wheel_delta_ != 0) {
            const std::int64_t total_delta =
                saturating_add(pending_wheel_delta_, wheel_remainder_);
            const std::int64_t row_delta = total_delta / 120;
            wheel_remainder_ = total_delta % 120;
            pending_wheel_delta_ = 0;
            scroll_state.scroll_y -= static_cast<float>(row_delta) * 24.0f;
        }
        const float content_left = rect.x + static_cast<float>(theme_.padding);
        const float content_top = rect.y + static_cast<float>(theme_.padding);
        const float viewport_height =
            std::max(0.0f, rect.h - 2.0f * static_cast<float>(theme_.padding));
        float maximum_scroll =
            std::max(0.0f, scroll_state.content_height - viewport_height);
        if (focused_id_ == id && focused_text_id_ == id) {
            const std::size_t caret_line =
                static_cast<std::size_t>(std::count(
                    edit_buffer_.begin(),
                    edit_buffer_.begin() +
                        static_cast<std::string::difference_type>(
                            std::min(edit_cursor_, edit_buffer_.size())),
                    '\n'));
            const float caret_top = static_cast<float>(caret_line) * line_height;
            if (caret_top < scroll_state.scroll_y) {
                scroll_state.scroll_y = caret_top;
            } else if (caret_top + line_height >
                       scroll_state.scroll_y + viewport_height) {
                scroll_state.scroll_y = caret_top + line_height - viewport_height;
            }
        }
        scroll_state.scroll_y =
            std::clamp(scroll_state.scroll_y, 0.0f, maximum_scroll);
        surface_.fill_rect(rect, theme_.window);
        surface_.push_clip(rect);
        std::size_t line_start = 0U;
        std::size_t line_index = 0U;
        while (line_start <= value.size()) {
            const std::size_t line_end = value.find('\n', line_start);
            const std::size_t length =
                (line_end == std::string::npos ? value.size() : line_end) - line_start;
            const std::string line = value.substr(line_start, length);
            const int line_y = static_cast<int>(
                content_top + static_cast<float>(line_index) * line_height -
                scroll_state.scroll_y);
            surface_.draw_text(static_cast<int>(content_left), line_y, line,
                               theme_.text, font_scale);
            if (line_end == std::string::npos) {
                break;
            }
            line_start = line_end + 1U;
            ++line_index;
        }
        if (focused_id_ == id && focused_text_id_ == id) {
            const std::size_t caret_line =
                static_cast<std::size_t>(std::count(
                    edit_buffer_.begin(),
                    edit_buffer_.begin() +
                        static_cast<std::string::difference_type>(
                            std::min(edit_cursor_, edit_buffer_.size())),
                    '\n'));
            const std::size_t caret_line_start =
                edit_cursor_ == 0U
                    ? 0U
                    : edit_buffer_.rfind('\n', edit_cursor_ - 1U) ==
                              std::string::npos
                          ? 0U
                          : edit_buffer_.rfind('\n', edit_cursor_ - 1U) + 1U;
            const std::size_t caret_column =
                std::min(edit_cursor_, edit_buffer_.size()) - caret_line_start;
            const int caret_x = static_cast<int>(
                content_left + static_cast<float>(caret_column) *
                                   8.0f * static_cast<float>(font_scale));
            const int caret_y = static_cast<int>(
                content_top + static_cast<float>(caret_line) * line_height -
                scroll_state.scroll_y);
            surface_.draw_line(caret_x, caret_y, caret_x,
                               caret_y + static_cast<int>(line_height) - 1,
                               theme_.accent);
        }
        surface_.pop_clip();
        draw_focus(id, rect);
        return result;
    }

    // Draws a clipped scrollable list and updates its selection on click or key input.
    Interaction list(const std::string& label_text,
                     const std::vector<std::string>& items, std::size_t& selected,
                     std::size_t& scroll_offset, std::size_t visible_rows = 5U) {
        const std::uint64_t id = widget_id(label_text);
        const float row_height = 20.0f;
        const Rect rect = allocate(row_height * static_cast<float>(visible_rows));
        register_focusable(id);
        if (items.empty()) {
            selected = 0U;
            scroll_offset = 0U;
            return {};
        }
        selected = std::min(selected, items.size() - 1U);
        const std::size_t max_offset = items.size() > visible_rows
                                           ? items.size() - visible_rows
                                           : 0U;
        scroll_offset = std::min(scroll_offset, max_offset);
        const bool pointer_over_list =
            contains_point(rect, mouse_x_, mouse_y_) &&
            contains_point(input_clips_.back(), mouse_x_, mouse_y_);
        if (pointer_over_list && visible_rows > 0U && pending_wheel_delta_ != 0) {
            const std::int64_t total_delta =
                saturating_add(pending_wheel_delta_, wheel_remainder_);
            const std::int64_t row_delta = total_delta / 120;
            wheel_remainder_ = total_delta % 120;
            pending_wheel_delta_ = 0;
            if (row_delta > 0) {
                const std::size_t amount = static_cast<std::size_t>(
                    static_cast<std::uint64_t>(row_delta));
                scroll_offset = amount >= scroll_offset ? 0U : scroll_offset - amount;
            } else if (row_delta < 0) {
                const std::uint64_t amount =
                    static_cast<std::uint64_t>(-(row_delta + 1)) + 1U;
                const std::size_t available = max_offset - scroll_offset;
                scroll_offset =
                    amount >= static_cast<std::uint64_t>(available)
                        ? max_offset
                        : scroll_offset + static_cast<std::size_t>(amount);
            }
        }
        Interaction result;
        for (auto navigation = pending_navigation_.begin();
             navigation != pending_navigation_.end();) {
            if (navigation->first != id) {
                ++navigation;
                continue;
            }
            const std::size_t previous_selection = selected;
            switch (navigation->second) {
                case Key::Up:
                case Key::Left:
                    if (selected > 0U) {
                        --selected;
                    }
                    break;
                case Key::Down:
                case Key::Right:
                    if (selected + 1U < items.size()) {
                        ++selected;
                    }
                    break;
                case Key::Home:
                    selected = 0U;
                    break;
                case Key::End:
                    selected = items.size() - 1U;
                    break;
                default:
                    break;
            }
            result.changed = selected != previous_selection;
            navigation = pending_navigation_.erase(navigation);
            if (selected < scroll_offset) {
                scroll_offset = selected;
            } else if (selected >= scroll_offset + visible_rows && visible_rows > 0U) {
                scroll_offset = selected - visible_rows + 1U;
            }
        }
        const bool keyboard_activated = keyboard_activation_id_ == id;
        if (keyboard_activated) {
            keyboard_activation_id_ = 0U;
            result.clicked = true;
        }
        surface_.push_clip(rect);
        for (std::size_t row_index = 0; row_index < visible_rows; ++row_index) {
            const std::size_t item_index = scroll_offset + row_index;
            if (item_index >= items.size()) {
                break;
            }
            const Rect item_rect{rect.x, rect.y + row_height * static_cast<float>(row_index),
                                 rect.w, row_height};
            const Interaction item_result = interact(id ^ (item_index + 1U), item_rect, false);
            if (item_index == selected) {
                surface_.fill_rect(item_rect, theme_.accent);
            } else if (item_result.hovered) {
                surface_.fill_rect(item_rect, theme_.button_hover);
            }
            surface_.draw_text(static_cast<int>(item_rect.x + theme_.padding),
                               static_cast<int>(item_rect.y + 2.0f), items[item_index],
                               theme_.text, theme_.font_scale);
            if (item_result.clicked) {
                selected = item_index;
                focused_id_ = id;
                focused_text_id_ = 0U;
                result = item_result;
                result.changed = true;
            } else if (item_result.hovered) {
                result.hovered = true;
                result.active = item_result.active;
            }
        }
        surface_.pop_clip();
        draw_focus(id, rect);
        return result;
    }

    // Scrolls a list offset by a signed number of rows and clamps it to the items.
    void scroll(std::size_t item_count, std::size_t visible_rows, int delta,
                std::size_t& scroll_offset) const {
        const std::size_t maximum = item_count > visible_rows
                                        ? item_count - visible_rows
                                        : 0U;
        scroll_offset = std::min(scroll_offset, maximum);
        if (delta < 0) {
            const std::size_t amount = static_cast<std::size_t>(-static_cast<long long>(delta));
            scroll_offset = amount > scroll_offset ? 0U : scroll_offset - amount;
        } else {
            const std::size_t amount = static_cast<std::size_t>(delta);
            scroll_offset = amount > maximum - scroll_offset
                                ? maximum
                                : scroll_offset + amount;
        }
    }

private:
    static detail::DockspaceGeometry dockspace_geometry(
        const DockspaceState& state) {
        detail::DockspaceGeometry geometry{};
        geometry.inner = {state.rect.x + 1.0f, state.rect.y + 1.0f,
                          std::max(0.0f, state.rect.w - 2.0f),
                          std::max(0.0f, state.rect.h - 2.0f)};
        const float splitter_width =
            std::min(5.0f, geometry.inner.w);
        const float pane_width =
            std::max(0.0f, geometry.inner.w - splitter_width);
        const float left_width =
            pane_width * std::clamp(state.left_ratio, 0.15f, 0.85f);
        const float tab_height = std::min(24.0f, geometry.inner.h);
        geometry.left = {geometry.inner.x, geometry.inner.y, left_width,
                          geometry.inner.h};
        geometry.splitter = {geometry.inner.x + left_width, geometry.inner.y,
                             splitter_width, geometry.inner.h};
        geometry.right = {geometry.splitter.x + splitter_width,
                          geometry.inner.y,
                          std::max(0.0f, pane_width - left_width),
                          geometry.inner.h};
        geometry.left_content = {
            geometry.left.x, geometry.left.y + tab_height, geometry.left.w,
            std::max(0.0f, geometry.left.h - tab_height)};
        geometry.right_content = {
            geometry.right.x, geometry.right.y + tab_height, geometry.right.w,
            std::max(0.0f, geometry.right.h - tab_height)};
        return geometry;
    }
    static std::size_t group_panel_count(const DockspaceState& state,
                                         DockSide side) {
        return static_cast<std::size_t>(std::count_if(
            state.panels.begin(), state.panels.end(),
            [side](const DockPanel& panel) { return panel.side == side; }));
    }
    static void ensure_active_dock_panel(DockspaceState& state, DockSide side) {
        std::string& active =
            side == DockSide::Left ? state.active_left : state.active_right;
        const auto panel = std::find_if(
            state.panels.begin(), state.panels.end(),
            [side, &active](const DockPanel& candidate) {
                return candidate.side == side && candidate.id == active;
            });
        if (panel != state.panels.end()) {
            return;
        }
        const auto first = std::find_if(
            state.panels.begin(), state.panels.end(),
            [side](const DockPanel& candidate) { return candidate.side == side; });
        active = first == state.panels.end() ? std::string{} : first->id;
    }
    void draw_dock_tabs(const DockspaceState& state, DockSide side, Rect group,
                        const std::string& active) {
        std::vector<const DockPanel*> members;
        for (const DockPanel& panel : state.panels) {
            if (panel.side == side) {
                members.push_back(&panel);
            }
        }
        const float tab_height = std::min(24.0f, group.h);
        const Rect strip{group.x, group.y, group.w, tab_height};
        surface_.fill_rect(strip, theme_.border);
        if (members.empty() || group.w <= 0.0f) {
            return;
        }
        surface_.push_clip(strip);
        const float tab_width = group.w / static_cast<float>(members.size());
        for (std::size_t index = 0; index < members.size(); ++index) {
            const Rect tab{group.x + tab_width * static_cast<float>(index),
                           group.y, tab_width, tab_height};
            const bool selected = members[index]->id == active;
            surface_.fill_rect(tab, selected ? theme_.title_bar : theme_.button);
            surface_.draw_text(static_cast<int>(tab.x + theme_.padding),
                               static_cast<int>(tab.y + 6.0f),
                               members[index]->title, theme_.text,
                               theme_.font_scale);
        }
        surface_.pop_clip();
    }
    void draw_pie_chart(const std::vector<float>& values,
                        const std::vector<std::uint32_t>& colors, Rect rect,
                        float hole_ratio) {
        surface_.fill_rect(rect, theme_.border);
        const Rect area{rect.x + 2.0f, rect.y + 2.0f,
                        std::max(0.0f, rect.w - 4.0f),
                        std::max(0.0f, rect.h - 4.0f)};
        surface_.fill_rect(area, theme_.window);
        if (values.empty() || colors.empty() || area.w < 1.0f ||
            area.h < 1.0f) {
            return;
        }

        float maximum = 0.0f;
        for (const float value : values) {
            if (std::isfinite(value) && value > maximum) {
                maximum = value;
            }
        }
        if (maximum <= 0.0f) {
            return;
        }

        std::vector<double> weights;
        weights.reserve(values.size());
        double total = 0.0;
        for (const float value : values) {
            const double weight = std::isfinite(value) && value > 0.0f
                                     ? static_cast<double>(value) /
                                           static_cast<double>(maximum)
                                     : 0.0;
            weights.push_back(weight);
            total += weight;
        }
        if (total <= 0.0) {
            return;
        }

        std::vector<double> slice_ends;
        std::vector<std::uint32_t> slice_colors;
        slice_ends.reserve(values.size());
        slice_colors.reserve(values.size());
        double accumulated = 0.0;
        for (std::size_t index = 0U; index < weights.size(); ++index) {
            if (weights[index] <= 0.0) {
                continue;
            }
            accumulated += weights[index] / total;
            slice_ends.push_back(accumulated);
            slice_colors.push_back(colors[index % colors.size()]);
        }
        if (slice_ends.empty()) {
            return;
        }
        slice_ends.back() = 1.0;

        const float radius = std::min(area.w, area.h) * 0.5f;
        const float inner_radius = radius * hole_ratio;
        const double radius_squared =
            static_cast<double>(radius) * static_cast<double>(radius);
        const double inner_radius_squared =
            static_cast<double>(inner_radius) * static_cast<double>(inner_radius);
        const float center_x = area.x + area.w * 0.5f;
        const float center_y = area.y + area.h * 0.5f;
        const double pi = 3.14159265358979323846;
        const double turn = 2.0 * pi;
        const int left = static_cast<int>(std::floor(center_x - radius));
        const int right = static_cast<int>(std::ceil(center_x + radius));
        const int top = static_cast<int>(std::floor(center_y - radius));
        const int bottom = static_cast<int>(std::ceil(center_y + radius));
        surface_.push_clip(area);
        for (int y = top; y < bottom; ++y) {
            for (int x = left; x < right; ++x) {
                const double dx = static_cast<double>(x) + 0.5 -
                                  static_cast<double>(center_x);
                const double dy = static_cast<double>(y) + 0.5 -
                                  static_cast<double>(center_y);
                const double distance_squared = dx * dx + dy * dy;
                if (distance_squared > radius_squared ||
                    distance_squared < inner_radius_squared) {
                    continue;
                }
                double angle = std::atan2(dy, dx) + pi * 0.5;
                if (angle < 0.0) {
                    angle += turn;
                }
                const double position = angle / turn;
                const auto slice = std::lower_bound(slice_ends.begin(),
                                                    slice_ends.end(), position);
                const std::size_t index =
                    slice == slice_ends.end()
                        ? slice_colors.size() - 1U
                        : static_cast<std::size_t>(slice - slice_ends.begin());
                surface_.fill_rect({static_cast<float>(x), static_cast<float>(y),
                                    1.0f, 1.0f}, slice_colors[index]);
            }
        }
        surface_.pop_clip();
    }

    struct ChildState {
        float scroll_y = 0.0f;
        float content_height = 0.0f;
    };
    struct ChildFrame {
        std::uint64_t id;
        Rect rect;
        Rect previous_root;
        float previous_cursor_x;
        float previous_cursor_y;
        float previous_content_width;
    };
    struct TreeFrame {
        float previous_cursor_x;
        float previous_content_width;
    };
    struct TableFrame {
        float previous_cursor_x;
        float previous_content_width;
        float cell_width;
        float column_gap;
        std::size_t column_count;
        std::size_t current_column;
        std::size_t row_index;
        float row_start_y;
        float row_bottom_y;
        std::uint64_t table_scope_id;
    };
    struct ChildHit {
        std::uint64_t id;
        Rect rect;
        Rect clip;
        std::size_t depth;
    };
    struct ComboState {
        std::size_t highlight = 0U;
        std::size_t pressed_option = 0U;
        bool has_pressed_option = false;
    };
    struct PopupDraw {
        Rect rect;
        std::vector<std::string> options;
        std::size_t highlight;
    };
    struct TooltipDraw {
        Rect rect;
        std::string text;
    };
    static bool contains_point(Rect rect, int x, int y) {
        return static_cast<float>(x) >= rect.x && static_cast<float>(y) >= rect.y &&
               static_cast<float>(x) < rect.x + rect.w &&
               static_cast<float>(y) < rect.y + rect.h;
    }
    static std::int64_t saturating_add(std::int64_t first, std::int64_t second) {
        const std::int64_t maximum = std::numeric_limits<std::int64_t>::max();
        const std::int64_t minimum = std::numeric_limits<std::int64_t>::min();
        if (second > 0 && first > maximum - second) {
            return maximum;
        }
        if (second < 0 && first < minimum - second) {
            return minimum;
        }
        return first + second;
    }
    void draw_popup(const PopupDraw& popup) {
        surface_.push_clip(root_);
        surface_.fill_rect(popup.rect, theme_.border);
        const Rect content{popup.rect.x + 1.0f, popup.rect.y + 1.0f,
                           popup.rect.w - 2.0f, popup.rect.h - 2.0f};
        surface_.fill_rect(content, theme_.window);
        surface_.push_clip(content);
        constexpr float row_height = 20.0f;
        for (std::size_t index = 0U; index < popup.options.size(); ++index) {
            const Rect option_rect{content.x, content.y + row_height *
                                    static_cast<float>(index),
                                   content.w, row_height};
            if (index == popup.highlight) {
                surface_.fill_rect(option_rect, theme_.accent);
            } else if (contains_point(popup.rect, mouse_x_, mouse_y_) &&
                       contains_point(option_rect, mouse_x_, mouse_y_)) {
                surface_.fill_rect(option_rect, theme_.button_hover);
            }
            surface_.draw_text(static_cast<int>(option_rect.x + theme_.padding),
                               static_cast<int>(option_rect.y + 2.0f),
                               popup.options[index], theme_.text, theme_.font_scale);
        }
        surface_.pop_clip();
        surface_.pop_clip();
    }
    void draw_tooltip(const TooltipDraw& tooltip_draw) {
        surface_.push_clip(root_);
        surface_.fill_rect(tooltip_draw.rect, theme_.border);
        const float padding = static_cast<float>(std::max(0, theme_.padding));
        const Rect content{tooltip_draw.rect.x + 1.0f,
                           tooltip_draw.rect.y + 1.0f,
                           tooltip_draw.rect.w - 2.0f,
                           tooltip_draw.rect.h - 2.0f};
        surface_.fill_rect(content, theme_.window);
        const Rect text_clip{content.x + padding, content.y + padding,
                             content.w - padding * 2.0f,
                             content.h - padding * 2.0f};
        surface_.push_clip(text_clip);
        surface_.draw_text(static_cast<int>(content.x + padding),
                           static_cast<int>(content.y + padding),
                           tooltip_draw.text, theme_.text, theme_.font_scale);
        surface_.pop_clip();
        surface_.pop_clip();
    }
    void clear_last_widget() {
        last_widget_id_ = 0U;
        last_widget_hovered_ = false;
    }
    void feed_text_key(Key key) {
        if (multiline_text_id_ != 0U &&
            multiline_text_id_ == focused_text_id_) {
            const std::size_t cursor =
                std::min(edit_cursor_, edit_buffer_.size());
            const std::size_t previous_newline =
                cursor == 0U ? std::string::npos
                             : edit_buffer_.rfind('\n', cursor - 1U);
            const std::size_t line_start =
                previous_newline == std::string::npos ? 0U
                                                       : previous_newline + 1U;
            const std::size_t next_newline = edit_buffer_.find('\n', cursor);
            const std::size_t line_end =
                next_newline == std::string::npos ? edit_buffer_.size()
                                                  : next_newline;
            switch (key) {
                case Key::Backspace:
                    if (cursor > 0U) {
                        edit_buffer_.erase(cursor - 1U, 1U);
                        edit_cursor_ = cursor - 1U;
                        edit_changed_ = true;
                    }
                    vertical_goal_valid_ = false;
                    break;
                case Key::Delete:
                    if (cursor < edit_buffer_.size()) {
                        edit_buffer_.erase(cursor, 1U);
                        edit_changed_ = true;
                    }
                    vertical_goal_valid_ = false;
                    break;
                case Key::Left:
                    if (cursor > 0U) {
                        edit_cursor_ = cursor - 1U;
                    }
                    vertical_goal_valid_ = false;
                    break;
                case Key::Right:
                    if (cursor < edit_buffer_.size()) {
                        edit_cursor_ = cursor + 1U;
                    }
                    vertical_goal_valid_ = false;
                    break;
                case Key::Home:
                    edit_cursor_ = line_start;
                    vertical_goal_valid_ = false;
                    break;
                case Key::End:
                    edit_cursor_ = line_end;
                    vertical_goal_valid_ = false;
                    break;
                case Key::Up:
                case Key::Down: {
                    if (!vertical_goal_valid_) {
                        vertical_goal_column_ = cursor - line_start;
                        vertical_goal_valid_ = true;
                    }
                    if (key == Key::Up && line_start > 0U) {
                        const std::size_t prior_end = line_start - 1U;
                        const std::size_t prior_newline =
                            prior_end == 0U
                                ? std::string::npos
                                : edit_buffer_.rfind('\n', prior_end - 1U);
                        const std::size_t prior_start =
                            prior_newline == std::string::npos
                                ? 0U
                                : prior_newline + 1U;
                        edit_cursor_ =
                            prior_start +
                            std::min(vertical_goal_column_, prior_end - prior_start);
                    } else if (key == Key::Down &&
                               next_newline != std::string::npos) {
                        const std::size_t next_start = next_newline + 1U;
                        const std::size_t following_newline =
                            edit_buffer_.find('\n', next_start);
                        const std::size_t following_end =
                            following_newline == std::string::npos
                                ? edit_buffer_.size()
                                : following_newline;
                        edit_cursor_ =
                            next_start +
                            std::min(vertical_goal_column_,
                                     following_end - next_start);
                    }
                    break;
                }
                case Key::Enter:
                    if (edit_buffer_.size() < edit_maximum_length_) {
                        edit_buffer_.insert(cursor, 1U, '\n');
                        edit_cursor_ = cursor + 1U;
                        edit_changed_ = true;
                    }
                    vertical_goal_valid_ = false;
                    break;
                case Key::Tab:
                case Key::Space:
                case Key::Escape:
                    break;
            }
            return;
        }
        switch (key) {
            case Key::Backspace:
                if (edit_cursor_ > 0U && edit_cursor_ <= edit_buffer_.size()) {
                    edit_buffer_.erase(edit_cursor_ - 1U, 1U);
                    --edit_cursor_;
                    edit_changed_ = true;
                }
                break;
            case Key::Delete:
                if (edit_cursor_ < edit_buffer_.size()) {
                    edit_buffer_.erase(edit_cursor_, 1U);
                    edit_changed_ = true;
                }
                break;
            case Key::Left:
                if (edit_cursor_ > 0U) {
                    --edit_cursor_;
                }
                break;
            case Key::Right:
                if (edit_cursor_ < edit_buffer_.size()) {
                    ++edit_cursor_;
                }
                break;
            case Key::Home:
                edit_cursor_ = 0U;
                break;
            case Key::End:
                edit_cursor_ = edit_buffer_.size();
                break;
            case Key::Enter:
                focused_id_ = 0U;
                focused_text_id_ = 0U;
                break;
            case Key::Tab:
            case Key::Space:
            case Key::Up:
            case Key::Down:
            case Key::Escape:
                break;
        }
    }
    void move_focus(bool reverse) {
        if (focus_order_.empty()) {
            focused_id_ = 0U;
            focused_text_id_ = 0U;
            return;
        }
        const auto current = std::find(focus_order_.begin(), focus_order_.end(), focused_id_);
        std::size_t index = 0U;
        if (current == focus_order_.end()) {
            index = reverse ? focus_order_.size() - 1U : 0U;
        } else {
            const std::size_t current_index =
                static_cast<std::size_t>(std::distance(focus_order_.begin(), current));
            index = reverse
                        ? (current_index == 0U ? focus_order_.size() - 1U
                                               : current_index - 1U)
                        : (current_index + 1U) % focus_order_.size();
        }
        focused_id_ = focus_order_[index];
        focused_text_id_ = 0U;
        keyboard_activation_id_ = 0U;
        pending_navigation_.clear();
    }
    void register_focusable(std::uint64_t id) {
        if (std::find(frame_focus_order_.begin(), frame_focus_order_.end(), id) ==
            frame_focus_order_.end()) {
            frame_focus_order_.push_back(id);
        }
    }
    void draw_focus(std::uint64_t id, Rect rect) {
        if (focused_id_ != id || rect.w <= 0.0f || rect.h <= 0.0f) {
            return;
        }
        const int left = static_cast<int>(rect.x);
        const int top = static_cast<int>(rect.y);
        const int right = static_cast<int>(rect.x + rect.w) - 1;
        const int bottom = static_cast<int>(rect.y + rect.h) - 1;
        surface_.draw_line(left, top, right, top, theme_.accent);
        surface_.draw_line(left, bottom, right, bottom, theme_.accent);
        surface_.draw_line(left, top, left, bottom, theme_.accent);
        surface_.draw_line(right, top, right, bottom, theme_.accent);
    }
    static std::uint64_t hash(const std::string& text, std::uint64_t seed) {
        std::uint64_t value = 1469598103934665603ULL ^ seed;
        for (const unsigned char character : text) {
            value ^= character;
            value *= 1099511628211ULL;
        }
        return value == 0U ? 1U : value;
    }
    static std::uint64_t table_cell_scope(const TableFrame& frame) {
        const std::string cell_id =
            std::to_string(frame.row_index) + ":" +
            std::to_string(frame.current_column);
        return hash(cell_id, frame.table_scope_id);
    }
    std::uint64_t widget_id(const std::string& text) const {
        return hash(text, id_stack_.back());
    }
    Rect allocate(float height) {
        const Rect rect{cursor_x_, cursor_y_, std::max(0.0f, content_width_), height};
        cursor_y_ += height + static_cast<float>(theme_.padding);
        return rect;
    }
    Interaction interact(std::uint64_t id, Rect rect, bool focusable = true,
                         bool enabled = true) {
        if (focusable && enabled) {
            register_focusable(id);
        }
        const bool hovered = enabled &&
                             (pointer_claimed_id_ == 0U || pointer_claimed_id_ == id) &&
                             !popup_input_blocked_ &&
                             static_cast<float>(mouse_x_) >= rect.x &&
                             static_cast<float>(mouse_y_) >= rect.y &&
                             static_cast<float>(mouse_x_) < rect.x + rect.w &&
                             static_cast<float>(mouse_y_) < rect.y + rect.h;
        last_widget_id_ = id;
        last_widget_hovered_ = hovered;
        if (hovered) {
            hot_id_ = id;
        }
        if (!enabled) {
            if (active_id_ == id) {
                active_id_ = 0U;
            }
            if (focused_id_ == id) {
                focused_id_ = 0U;
                focused_text_id_ = 0U;
            }
            if (keyboard_activation_id_ == id) {
                keyboard_activation_id_ = 0U;
            }
            pending_navigation_.erase(
                std::remove_if(
                    pending_navigation_.begin(), pending_navigation_.end(),
                    [id](const std::pair<std::uint64_t, Key>& navigation) {
                        return navigation.first == id;
                    }),
                pending_navigation_.end());
            last_widget_hovered_ = false;
            return {};
        }
        if (hovered && mouse_pressed_) {
            active_id_ = id;
            focused_id_ = id;
            if (focused_text_id_ != id) {
                focused_text_id_ = 0U;
            }
        }
        const bool keyboard_clicked = keyboard_activation_id_ == id;
        if (keyboard_clicked) {
            keyboard_activation_id_ = 0U;
        }
        const bool clicked = (hovered && mouse_released_ && active_id_ == id) ||
                             keyboard_clicked;
        const bool active = active_id_ == id && mouse_down_;
        if (mouse_released_ && active_id_ == id) {
            active_id_ = 0U;
        }
        return {hovered, active, clicked, false};
    }

    Surface surface_;
    Theme theme_;
    Rect root_;
    float cursor_x_;
    float cursor_y_;
    float content_width_;
    int mouse_x_ = 0;
    int mouse_y_ = 0;
    int drag_value_last_x_ = 0;
    bool mouse_down_ = false;
    bool previous_mouse_down_ = false;
    bool mouse_pressed_ = false;
    bool mouse_released_ = false;
    std::size_t frame_index_ = 0U;
    std::uint64_t last_widget_id_ = 0U;
    bool last_widget_hovered_ = false;
    std::uint64_t tooltip_hover_id_ = 0U;
    std::size_t tooltip_hover_frames_ = 0U;
    std::size_t tooltip_last_frame_ = 0U;
    std::uint64_t hot_id_ = 0U;
    std::uint64_t active_id_ = 0U;
    std::uint64_t focused_id_ = 0U;
    std::uint64_t focused_text_id_ = 0U;
    std::uint64_t keyboard_activation_id_ = 0U;
    std::vector<std::pair<std::uint64_t, Key>> pending_navigation_;
    std::vector<Rect> input_clips_;
    std::unordered_map<std::uint64_t, ChildState> child_states_;
    std::unordered_map<std::uint64_t, ComboState> combo_states_;
    std::vector<ChildFrame> child_stack_;
    std::vector<TreeFrame> tree_stack_;
    std::vector<TableFrame> table_stack_;
    std::vector<ChildHit> previous_child_hits_;
    std::vector<ChildHit> frame_child_hits_;
    std::uint64_t child_wheel_target_ = 0U;
    std::uint64_t open_combo_id_ = 0U;
    bool popup_input_blocked_ = false;
    std::uint64_t pointer_claimed_id_ = 0U;
    bool frame_open_combo_seen_ = false;
    Rect previous_popup_rect_{};
    Rect previous_combo_rect_{};
    Rect active_popup_rect_{};
    Rect active_combo_rect_{};
    std::vector<PopupDraw> frame_popup_draws_;
    std::vector<TooltipDraw> frame_tooltip_draws_;
    std::int64_t pending_wheel_delta_ = 0;
    std::int64_t wheel_remainder_ = 0;
    std::vector<std::uint64_t> focus_order_;
    std::vector<std::uint64_t> frame_focus_order_;
    DockspaceState* active_dockspace_ = nullptr;
    detail::DockspaceGeometry active_dockspace_geometry_{};
    std::string edit_buffer_;
    std::size_t edit_cursor_ = 0U;
    bool edit_changed_ = false;
    std::uint64_t multiline_text_id_ = 0U;
    std::size_t edit_maximum_length_ = 64U;
    std::size_t vertical_goal_column_ = 0U;
    bool vertical_goal_valid_ = false;
    std::vector<std::uint64_t> id_stack_{1469598103934665603ULL};
};

}  // namespace glm

#endif
