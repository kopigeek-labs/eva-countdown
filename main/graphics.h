// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <cstdint>
#include <cstring>
#include <initializer_list>
#include "ui_assets.h"

namespace eva {
constexpr int width = 400, height = 300, stride = width / 8, bytes = stride * height;
struct Frame {
    uint8_t data[bytes]{}; // 1 = reflective white.
    void pixel(int x, int y, bool white) {
        if (x < 0 || x >= width || y < 0 || y >= height)
            return;
        const auto mask = uint8_t(0x80 >> (x & 7));
        auto &b = data[y * stride + x / 8];
        b = white ? uint8_t(b | mask) : uint8_t(b & ~mask);
    }
    bool pixel(int x, int y) const {
        return x >= 0 && x < width && y >= 0 && y < height &&
               (data[y * stride + x / 8] & (0x80 >> (x & 7)));
    }
    void rect(int x, int y, int w, int h, bool white = true) {
        for (int yy = y; yy < y + h; ++yy)
            for (int xx = x; xx < x + w; ++xx)
                pixel(xx, yy, white);
    }
    void outline(int x, int y, int w, int h) {
        rect(x, y, w, 1);
        rect(x, y + h - 1, w, 1);
        rect(x, y, 1, h);
        rect(x + w - 1, y, 1, h);
    }
    void invert(int x, int y, int w, int h) {
        for (int yy = y; yy < y + h; ++yy)
            for (int xx = x; xx < x + w; ++xx)
                pixel(xx, yy, !pixel(xx, yy));
    }
    void bitmap(const uint8_t *src, int x, int y, int w, int h, bool transparent = false) {
        for (int yy = 0; yy < h; ++yy)
            for (int xx = 0; xx < w; ++xx) {
                const bool on = src[yy * ((w + 7) / 8) + xx / 8] & (0x80 >> (xx & 7));
                if (on || !transparent)
                    pixel(x + xx, y + yy, on);
            }
    }
    void ellipse(int x, int y, int rx, int ry, bool white = true) {
        for (int yy = -ry; yy <= ry; ++yy)
            for (int xx = -rx; xx <= rx; ++xx)
                if (xx * xx * ry * ry + yy * yy * rx * rx <= rx * rx * ry * ry)
                    pixel(x + xx, y + yy, white);
    }
    void line(int x0, int y0, int x1, int y1, bool white = true) {
        const int dx = x1 > x0 ? x1 - x0 : x0 - x1, sx = x0 < x1 ? 1 : -1;
        const int dy = -(y1 > y0 ? y1 - y0 : y0 - y1), sy = y0 < y1 ? 1 : -1;
        int error = dx + dy;
        for (;;) {
            pixel(x0, y0, white);
            if (x0 == x1 && y0 == y1)
                break;
            const int e2 = 2 * error;
            if (e2 >= dy) {
                error += dy;
                x0 += sx;
            }
            if (e2 <= dx) {
                error += dx;
                y0 += sy;
            }
        }
    }
    static int text_width(const char *s) {
        int w = 0;
        for (; *s; ++s)
            if (*s >= 32 && *s <= 126)
                w += kAdvances[*s - 32] + 1;
        return w;
    }
    void text(const char *s, int x, int y) {
        for (; *s; ++s)
            if (*s >= 32 && *s <= 126) {
                bitmap(kLetters[*s - 32], x, y, 12, 20, true);
                x += kAdvances[*s - 32] + 1;
            }
    }
    void centered(const char *s, int x, int y, int w) { text(s, x + (w - text_width(s)) / 2, y); }
};
// Preserve the v0.1 panel mapping: 2 columns x 4 rows per byte.
inline void pack_panel(const Frame &f, uint8_t *out) {
    std::memset(out, 0, bytes);
    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x) {
            const int iy = height - 1 - y, index = (x / 2) * (height / 4) + iy / 4,
                      bit = 7 - ((iy % 4) * 2 + x % 2);
            if (f.pixel(x, y))
                out[index] |= uint8_t(1u << bit);
        }
}
} // namespace eva
