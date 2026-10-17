// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <cstdio>
#include "assets.h"
#include "graphics.h"
#include "app.h"

namespace eva {
inline void power_panels(Frame &f, const App &a, int64_t now) {
    bool internal = a.power == Power::Internal, external = a.power == Power::External;
    if (a.mode == Mode::Clock)
        internal = external = true;
    else if (a.handover(now))
        internal = ((now - a.handover_at) / 200000) % 2 == 0;
    if (internal)
        f.invert(265, 17, 125, 96);
    if (external)
        f.invert(265, 185, 125, 96);
}
inline void header(Frame &f, const uint8_t *jp, const char *en) {
    f.rect(10, 15, 238, 47, false);
    f.bitmap(jp, 12, 18, 84, 20);
    f.text(en, 106, 19);
    f.rect(12, 40, 233, 1);
}
inline void four_digits(Frame &f, int left, int right, bool colon, int y = 65) {
    f.bitmap(kDigits[left / 10], 22, y, 40, 174);
    f.bitmap(kDigits[left % 10], 66, y, 40, 174);
    f.bitmap(kDigits[right / 10], 143, y, 40, 174);
    f.bitmap(kDigits[right % 10], 187, y, 40, 174);
    if (colon) {
        f.rect(119, y + 47, 7, 10);
        f.rect(119, y + 118, 7, 10);
    }
}
inline void render(Frame &f, const App &a, int64_t now) {
    std::memcpy(f.data, kBackground, bytes);
    power_panels(f, a, now);
    if (a.mode == Mode::Countdown) {
        const auto remaining = a.countdown.remaining(now);
        const bool running = a.countdown.running(now), ended = remaining == 0;
        const int cs = int((remaining + 9999) / 10000);
        f.bitmap(kDigits[cs / 6000], 12, 65, 40, 174);
        f.bitmap(kDigits[(cs / 100 % 60) / 10], 66, 65, 40, 174);
        f.bitmap(kDigits[cs / 100 % 10], 108, 65, 40, 174);
        f.bitmap(kSmallDigits[cs % 100 / 10], 166, 107, 32, 132);
        f.bitmap(kSmallDigits[cs % 10], 202, 107, 32, 132);
        f.rect(55, 111, 5, 9);
        f.rect(55, 190, 5, 9);
        f.rect(153, 136, 5, 9);
        f.rect(153, 204, 5, 9);
        // Selected speed stays visible while paused. STOP is a separate state.
        f.invert(13 + (a.racing ? 3 : 2) * 60, 253, 53, 30);
        if (!running)
            f.rect(17, 273, 44, 6);
        if ((remaining <= 10000000 && running && (now / 500000) % 2) || ended)
            f.invert(10, 62, 236, 181);
        f.rect(34, 42, 211, 18, false);
        const char *state = ended                              ? "ACTIVITY TERMINATED"
                            : a.handover(now)                  ? "INTERNAL POWER: ENGAGED"
                            : running                          ? "COUNTDOWN ACTIVE"
                            : remaining < a.countdown.length() ? "PAUSED"
                            : a.power == Power::External       ? "EXTERNAL POWER: READY"
                                                               : "STANDBY";
        f.text(state, 37, 42);
    } else if (a.mode == Mode::Clock) {
        header(f, kTitleClock, "LOCAL TIME");
        if (a.rtc_valid) {
            four_digits(f, a.clock.hour, a.clock.minute, true, 53);
            constexpr const char *days[]{"SUNDAY",   "MONDAY", "TUESDAY", "WEDNESDAY",
                                         "THURSDAY", "FRIDAY", "SATURDAY"};
            char date[40];
            std::snprintf(date, sizeof(date), "%04d.%02d.%02d   %s", a.clock.year, a.clock.month,
                          a.clock.day, days[weekday(a.clock)]);
            f.centered(date, 12, 231, 234);
        } else {
            for (int x : {22, 66, 143, 187})
                f.rect(x, 132, 40, 5);
            f.centered("SET CLOCK VIA USB", 12, 181, 234);
        }
    } else if (a.mode == Mode::Pomodoro) {
        header(f, kTitlePomodoro, "POMODORO TIMER");
        const auto remaining = a.pomodoro.remaining(now);
        const bool running = a.pomodoro.running(now);
        const int seconds = int((remaining + 999999) / 1000000);
        if (remaining == 0)
            f.centered("SESSION COMPLETE", 12, 44, 234);
        four_digits(f, seconds / 60, seconds % 60, true);
        // Keep the original status-strip artwork in every mode.
        if (!running)
            f.rect(17, 273, 44, 6);
        if (remaining == 0)
            f.invert(10, 62, 236, 181);
    }
    // Small navigation markers; a finished focus session stays discoverable.
    for (int i = 0; i < 3; ++i) {
        f.outline(115 + i * 12, 288, 7, 4);
        if (i == int(a.mode))
            f.rect(116 + i * 12, 289, 5, 2);
    }
    if (a.pomo_finished && a.mode != Mode::Pomodoro) {
        f.rect(267, 128, 120, 42, false);
        f.centered("POMODORO", 267, 130, 120);
        f.centered("COMPLETE", 267, 149, 120);
    }
}
} // namespace eva
