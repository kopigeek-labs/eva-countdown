// SPDX-License-Identifier: Apache-2.0
#include "render.h"
#include <cstdio>
#include <cstdlib>
#include <string>
int main(int argc, char **argv) {
    const std::string scene = argc > 1 ? argv[1] : "normal";
    const int64_t now = argc > 2 ? std::strtoll(argv[2], nullptr, 10) : 0;
    eva::App app;
    app.power = eva::Power::External;
    if (scene == "racing") {
        app.racing = true;
        app.countdown.set_length(60000000);
    } else if (scene == "handover" || scene == "running" || scene == "ended") {
        app.power = eva::Power::Internal;
        app.countdown.start(0);
        if (scene == "handover")
            app.handover_at = 0;
    } else if (scene == "clock" || scene == "unset") {
        app.mode = eva::Mode::Clock;
        app.rtc_valid = scene == "clock";
        app.clock = {2026, 10, 16, 21, 7, int(now / 1000000) % 60};
    } else if (scene == "pomodoro" || scene == "focus-done") {
        app.mode = eva::Mode::Pomodoro;
        if (scene == "focus-done")
            app.pomodoro.start(0);
    } else if (scene != "normal") {
        std::fprintf(stderr, "Unknown preview scene\n");
        return 1;
    }
    eva::Frame frame;
    eva::render(frame, app, now);
    std::printf("P4\n400 300\n");
    for (uint8_t b : frame.data)
        std::putchar(b ^ 255);
}
