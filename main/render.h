// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <cstdint>
#include <cstring>
#include "assets.h"

namespace eva {
constexpr int width = 400, height = 300, stride = width / 8, bytes = stride * height;
struct Frame {
    uint8_t data[bytes]{}; // Row-major, MSB first. 1 = reflective white.
    void pixel(int x, int y, bool white) {
        if (x < 0 || x >= width || y < 0 || y >= height)
            return;
        const auto mask = uint8_t(0x80 >> (x & 7));
        auto &b = data[y * stride + x / 8];
        b = white ? uint8_t(b | mask) : uint8_t(b & ~mask);
    }
    bool pixel(int x, int y) const { return data[y * stride + x / 8] & (0x80 >> (x & 7)); }
    void rect(int x, int y, int w, int h, bool white) {
        for (int yy = y; yy < y + h; ++yy)
            for (int xx = x; xx < x + w; ++xx)
                pixel(xx, yy, white);
    }
    void bitmap(const uint8_t *source, int x, int y, int w, int h) {
        for (int yy = 0; yy < h; ++yy)
            for (int xx = 0; xx < w; ++xx)
                pixel(x + xx, y + yy, source[yy * ((w + 7) / 8) + xx / 8] & (0x80 >> (xx & 7)));
    }
};
inline void render(Frame &f, int64_t remaining_us, bool running, int64_t now_us) {
    std::memcpy(f.data, kBackground, bytes);
    // Ceiling keeps 5:00:00 visible at start and never displays zero early.
    const int cs = int((remaining_us + 9999) / 10000);
    const int minutes = cs / 6000, seconds = (cs / 100) % 60, hundredths = cs % 100;
    f.bitmap(kDigits[minutes % 10], 12, 65, 40, 174);
    f.bitmap(kDigits[seconds / 10], 66, 65, 40, 174);
    f.bitmap(kDigits[seconds % 10], 108, 65, 40, 174);
    f.bitmap(kSmallDigits[hundredths / 10], 166, 107, 32, 132);
    f.bitmap(kSmallDigits[hundredths % 10], 202, 107, 32, 132);
    f.rect(55, 111, 5, 9, true);
    f.rect(55, 190, 5, 9, true);
    f.rect(153, 136, 5, 9, true);
    f.rect(153, 204, 5, 9, true);
    const bool ended = remaining_us == 0;
    const bool emergency = remaining_us <= 60000000 && !ended;
    const int mode = running ? 3 : 0;
    if (!emergency || !running || (now_us / 250000) % 2 == 0)
        f.rect(17 + 60 * mode, 273, 44, 6, true);
    // Monochrome emergency cue is local to the timer; static labels stay readable.
    if ((emergency && running && (now_us / 500000) % 2) || ended) {
        for (int y = 62; y < 243; ++y)
            for (int x = 10; x < 246; ++x)
                f.pixel(x, y, !f.pixel(x, y));
    }
}
// Waveshare landscape packing: each byte holds 2 columns by 4 rows.
inline void pack_panel(const Frame &f, uint8_t *out) {
    std::memset(out, 0, bytes);
    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x) {
            const int inverse_y = height - 1 - y;
            const int index = (x / 2) * (height / 4) + inverse_y / 4;
            const int bit = 7 - ((inverse_y % 4) * 2 + x % 2);
            if (f.pixel(x, y))
                out[index] |= uint8_t(1u << bit);
        }
}
} // namespace eva
