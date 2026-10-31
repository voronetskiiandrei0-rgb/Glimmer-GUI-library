#include "glimmer/glimmer.hpp"

#include <cstdint>
#include <fstream>
#include <iostream>

namespace {

bool write_ppm(const char* path, const glm::Context& ui) {
    std::ofstream output(path, std::ios::binary);
    if (!output) {
        return false;
    }
    output << "P6\n" << ui.width() << ' ' << ui.height() << "\n255\n";
    const std::uint32_t* pixels = ui.pixels();
    for (int index = 0; index < ui.width() * ui.height(); ++index) {
        const std::uint32_t pixel = pixels[index];
        const char rgb[3] = {
            static_cast<char>(pixel & 0xFFU),
            static_cast<char>((pixel >> 8U) & 0xFFU),
            static_cast<char>((pixel >> 16U) & 0xFFU),
        };
        output.write(rgb, 3);
    }
    return output.good();
}

}  // namespace

int main() {
    glm::Context ui(480, 360);
    float volume = 0.65f;
    bool muted = false;
    ui.begin(0, 0, false);
    ui.window("Settings", {24.0f, 20.0f, 320.0f, 300.0f}, [&] {
        ui.label("Audio preferences");
        ui.label("Volume");
        ui.slider("vol", volume, 0.0f, 1.0f);
        ui.checkbox("Mute", muted);
        if (ui.button("Reset")) {
            volume = 0.5f;
        }
    });
    ui.end();
    if (!write_ppm("settings.ppm", ui)) {
        std::cerr << "Failed to write settings.ppm\n";
        return 1;
    }
    return 0;
}
