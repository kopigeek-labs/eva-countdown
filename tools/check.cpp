// SPDX-License-Identifier: Apache-2.0
#include "app.h"
#include "render.h"
#include <cassert>
#include <cstdio>
#include <cstring>

using namespace eva;
constexpr int64_t second = 1000000;
void timer_checks() {
    for (const int64_t duration : {60 * second, 300 * second, 1500 * second}) {
        Countdown t(duration);
        assert(t.remaining(0) == duration && !t.running(0));
        t.start(second);
        t.start(2 * second); // Starting twice must not rewind.
        assert(t.remaining(2 * second) == duration - second);
        t.toggle(2100000);
        assert(t.remaining(1000 * second) == duration - 1100000);
        t.start(1000 * second);
        assert(t.remaining(1000 * second + duration - 1100001) == 1);
        assert(t.remaining(1000 * second + duration - 1100000) == 0);
        assert(!t.running(20000 * second));
        t.toggle(20000 * second);
        assert(t.remaining(20000 * second) == duration);
        t.reset();
        assert(!t.running(20001 * second) && t.remaining(20001 * second) == duration);
    }
    Button b;
    assert(b.update(true, 0) == Button::None);
    assert(b.update(false, 10000) == Button::None);
    assert(b.update(true, 20000) == Button::None);
    assert(b.update(true, 45000) == Button::None);
    assert(b.update(true, 845000) == Button::Hold);
    assert(b.update(true, 900000) == Button::None);
    assert(b.update(false, 950000) == Button::None);
    assert(b.update(false, 975000) == Button::None);
    assert(b.update(true, 1000000) == Button::None);
    assert(b.update(true, 1025000) == Button::None);
    assert(b.update(false, 1100000) == Button::None);
    assert(b.update(false, 1125000) == Button::Click);
}
void app_checks() {
    App a;
    a.sample_power(Power::External, 0);
    assert(a.power == Power::Unknown);
    a.sample_power(Power::External, 150000);
    assert(a.power == Power::External);
    a.sample_power(Power::Internal, 200000);
    a.sample_power(Power::External, 250000);
    assert(a.power == Power::External && !a.countdown.running(250000));
    a.input(Button::None, Button::Hold, 300000);
    assert(a.racing && a.countdown.length() == 60 * second);
    a.sample_power(Power::Internal, second);
    a.sample_power(Power::Internal, 1150000);
    assert(a.power == Power::Internal && a.countdown.running(1150000) && a.handover(1150000));
    a.sample_power(Power::Internal, 2150000);
    assert(a.countdown.remaining(2150000) == 59 * second);
    assert(!a.handover(2350000));
    a.sample_power(Power::External, 3 * second);
    a.sample_power(Power::External, 3150000);
    assert(!a.countdown.running(3150000) && a.countdown.remaining(3150000) == 60 * second);
    a.input(Button::None, Button::Hold, 4 * second);
    assert(!a.racing && a.countdown.length() == 300 * second);
    a.input(Button::None, Button::Click, 5 * second);
    assert(a.mode == Mode::Clock);
    a.sample_power(Power::Internal, 6 * second);
    a.sample_power(Power::Internal, 6150000);
    assert(a.mode == Mode::Clock && !a.countdown.running(7 * second));
    a.input(Button::None, Button::Click, 8 * second);
    assert(a.mode == Mode::Pomodoro);
    a.input(Button::Click, Button::None, 10 * second);
    a.input(Button::None, Button::Click, 11 * second);
    assert(a.mode == Mode::Countdown);
    a.tick(20 * second);
    assert(a.pomodoro.remaining(20 * second) == 1490 * second);
    a.input(Button::None, Button::Click, 21 * second);
    assert(a.mode == Mode::Clock);
    a.input(Button::None, Button::Click, 22 * second);
    assert(a.mode == Mode::Pomodoro);
    a.input(Button::None, Button::Click, 23 * second);
    assert(a.mode == Mode::Countdown);
    a.tick(1510 * second);
    assert(a.pomo_finished && a.pomodoro.remaining(1510 * second) == 0);
    a.mode = Mode::Pomodoro;
    a.input(Button::Hold, Button::None, 1511 * second);
    assert(!a.pomo_finished && a.pomodoro.remaining(1511 * second) == 1500 * second);
    App boot_on_battery;
    boot_on_battery.sample_power(Power::Internal, 0);
    boot_on_battery.sample_power(Power::Internal, 150000);
    assert(!boot_on_battery.countdown.running(second));
    // Reset, mode changes and RTC do not change the independent active timer.
    a.mode = Mode::Countdown;
    a.input(Button::Click, Button::None, 2000 * second);
    a.input(Button::None, Button::Click, 2001 * second);
    a.clock = {2099, 12, 31, 23, 59, 59};
    assert(a.countdown.remaining(2002 * second) == 298 * second);
}
void clock_checks() {
    DateTime t{};
    assert(parse_time("TIME 2026-10-16T23:59:59", t));
    assert(weekday(t) == 5);
    assert(parse_time("TIME 2028-02-29T00:00:00", t));
    for (const char *bad :
         {"TIME 2026-02-29T00:00:00", "TIME 2100-01-01T00:00:00", "TIME 2026-13-01T00:00:00",
          "TIME 2026-10-16T24:00:00", "TIME 2026-10-16T23:60:00", "TIME 2026-10-16T23:59:60",
          "TIME 2026-10-16T23:59:59x", "TIME 2026-10-16 23:59:59", "TIME 2026-10-16Txx:59:59"})
        assert(!parse_time(bad, t));
    for (int year = 2000; year <= 2099; ++year)
        for (int month = 1; month <= 12; ++month) {
            DateTime input{year, month, month_days(year, month), 23, 59, 59}, output;
            uint8_t bytes[7];
            encode_rtc(input, bytes);
            assert(decode_rtc(0, bytes, output));
            assert(output.year == year && output.month == month && output.day == input.day &&
                   output.second == 59);
            assert(!decode_rtc(0x20, bytes, output));
            assert(!decode_rtc(2, bytes, output));
            bytes[0] |= 0x80;
            assert(!decode_rtc(0, bytes, output));
            bytes[0] = 0x6a;
            assert(!decode_rtc(0, bytes, output));
        }
    LineReader reader;
    for (int i = 0; i < 100; ++i)
        assert(!reader.push('x'));
    assert(!reader.push('\n'));
    for (char c : std::initializer_list<char>{'I', 'N', 'F', 'O', '\r'})
        assert(!reader.push(c));
    assert(reader.push('\n') && std::strcmp(reader.line(), "INFO") == 0);
}
void graphics_checks() {
    Frame frame;
    uint8_t packed[bytes];
    for (int y = 0; y < 300; ++y)
        for (int x = 0; x < 400; ++x)
            frame.pixel(x, y, (x * 17 + y * 23) % 31 < 13);
    pack_panel(frame, packed);
    int visits[bytes * 8]{};
    for (int y = 0; y < 300; ++y)
        for (int x = 0; x < 400; ++x) {
            const int iy = 299 - y, index = (x / 2) * 75 + iy / 4, bit = 7 - ((iy % 4) * 2 + x % 2);
            assert(++visits[index * 8 + bit] == 1);
            assert(bool(packed[index] & (1 << bit)) == frame.pixel(x, y));
        }
    for (int v : visits)
        assert(v == 1);
    App a;
    a.power = Power::External;
    render(frame, a, 0);
    Frame before = frame;
    a.power = Power::Internal;
    render(frame, a, 0);
    // A power-source change only changes the two power panels and status line.
    assert(before.pixel(266, 18) != frame.pixel(266, 18));
    assert(before.pixel(266, 186) != frame.pixel(266, 186));
    a.mode = Mode::Clock;
    a.rtc_valid = true;
    a.clock = {2026, 10, 16, 23, 59, 59};
    render(frame, a, 0);
    assert(frame.pixel(266, 18) && frame.pixel(266, 186));
    // Many frames under sanitizers, including maximum clock/pomodoro values.
    for (int mode = 0; mode < 3; ++mode) {
        a.mode = Mode(mode);
        for (int i = 0; i < 100; ++i) {
            render(frame, a, int64_t(i) * 130000);
        }
    }
    a.mode = Mode::Pomodoro;
    render(frame, a, 0);
    // The 25-minute tens digit must actually be a 2, not a wrapped one-digit minute.
    for (int y = 0; y < 174; ++y)
        for (int x = 0; x < 40; ++x)
            assert(frame.pixel(22 + x, 65 + y) == bool(kDigit2[y * 5 + x / 8] & (0x80 >> (x % 8))));
}
int main() {
    timer_checks();
    app_checks();
    clock_checks();
    graphics_checks();
    std::puts(
        "PASS: 1/5/25-minute timers, power debounce/handover, three-mode controls, background "
        "focus, RTC validation, bounded serial parser, all render modes and "
        "panel mapping");
}
