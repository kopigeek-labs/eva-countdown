// SPDX-License-Identifier: Apache-2.0
#include "timer.h"
#include "render.h"
#include <cassert>
#include <cstdio>
int main() {
    Countdown t;
    assert(t.remaining(0) == 300000000 && !t.running(0));
    t.toggle(1000000);
    assert(t.remaining(2000000) == 299000000);
    t.toggle(2100000); // Pause freezes elapsed time, not frame count.
    assert(t.remaining(1000000000) == 298900000 && !t.running(1000000000));
    t.toggle(1000000000);
    assert(t.remaining(1298899999) == 1);
    assert(t.remaining(1298900000) == 0 && !t.running(1298900000));
    assert(t.remaining(2000000000) == 0);
    t.toggle(2000000000);
    assert(t.remaining(2000000000) == Countdown::duration);
    t.reset();
    assert(!t.running(2000000001));
    Button b;
    assert(b.update(true, 0) == Button::None);
    assert(b.update(false, 10000) == Button::None); // Bounce isn't a click.
    assert(b.update(true, 20000) == Button::None);
    assert(b.update(true, 45000) == Button::None);
    assert(b.update(true, 845000) == Button::Hold);
    assert(b.update(true, 900000) == Button::None);
    assert(b.update(false, 950000) == Button::None);
    assert(b.update(false, 975000) == Button::None); // No click after hold.
    assert(b.update(true, 1000000) == Button::None);
    assert(b.update(true, 1025000) == Button::None);
    assert(b.update(false, 1100000) == Button::None);
    assert(b.update(false, 1125000) == Button::Click);
    eva::Frame frame;
    uint8_t packed[eva::bytes];
    // Asymmetric pattern catches transpose, mirroring and lost pixels.
    for (int y = 0; y < 300; ++y)
        for (int x = 0; x < 400; ++x)
            frame.pixel(x, y, (x * 17 + y * 23) % 31 < 13);
    eva::pack_panel(frame, packed);
    int visits[eva::bytes * 8]{};
    for (int y = 0; y < 300; ++y)
        for (int x = 0; x < 400; ++x) {
            int iy = 299 - y, index = (x / 2) * 75 + iy / 4, bit = 7 - ((iy % 4) * 2 + x % 2);
            assert(++visits[index * 8 + bit] == 1);
            assert(bool(packed[index] & (1 << bit)) == frame.pixel(x, y));
        }
    for (int v : visits)
        assert(v == 1);
    eva::render(frame, 0, false, 5000000000LL);
    eva::render(frame, Countdown::duration, false, 0);
    std::puts(
        "PASS: countdown, pause/resume, expiry/restart, debounce, hold suppression, pixel packing");
}
