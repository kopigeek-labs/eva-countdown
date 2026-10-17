// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "timer.h"
#include "clock_time.h"

namespace eva {
enum class Mode { Countdown, Clock, Pomodoro };
enum class Power { Unknown, External, Internal };

// Requires a stable sample for 150 ms. Initial observation is not an unplug event.
class PowerFilter {
  public:
    Power update(Power sample, int64_t now) {
        if (sample != candidate_) {
            candidate_ = sample;
            since_ = now;
        }
        if (now - since_ >= 150000)
            stable_ = candidate_;
        return stable_;
    }

  private:
    Power candidate_ = Power::Unknown, stable_ = Power::Unknown;
    int64_t since_ = 0;
};

class App {
  public:
    Mode mode = Mode::Countdown;
    Power power = Power::Unknown;
    bool racing = false, rtc_valid = false, pomo_finished = false;
    DateTime clock{};
    Countdown countdown;
    Countdown pomodoro{25LL * 60 * 1000000};
    int64_t handover_at = -2000000;

    void sample_power(Power raw, int64_t now) {
        const Power next = filter_.update(raw, now);
        if (next == power)
            return;
        const Power previous = power;
        power = next;
        if (power == Power::External)
            countdown.reset();
        if (previous == Power::External && power == Power::Internal) {
            handover_at = now;
            // Never switch away from a clock/focus session on a cable event.
            if (mode == Mode::Countdown) {
                countdown.reset();
                countdown.start(now);
            }
        }
    }
    bool handover(int64_t now) const { return now >= handover_at && now - handover_at < 1200000; }
    void input(Button::Event key, Button::Event boot, int64_t now) {
        if (boot == Button::Click) {
            mode = Mode((int(mode) + 1) % 3);
            return;
        }
        if (boot == Button::Hold && mode == Mode::Countdown) {
            racing = !racing;
            countdown.set_length((racing ? 60LL : 300LL) * 1000000);
            return;
        }
        if (mode == Mode::Countdown) {
            if (key == Button::Click)
                countdown.toggle(now);
            if (key == Button::Hold)
                countdown.reset();
        } else if (mode == Mode::Pomodoro) {
            if (key == Button::Click) {
                pomodoro.toggle(now);
                pomo_finished = false;
            }
            if (key == Button::Hold) {
                pomodoro.reset();
                pomo_finished = false;
            }
        }
    }
    void tick(int64_t now) {
        if (pomodoro.remaining(now) == 0)
            pomo_finished = true;
    }

  private:
    PowerFilter filter_;
};
} // namespace eva
