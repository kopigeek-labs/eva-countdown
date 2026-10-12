// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <cstdint>

// All time is monotonic microseconds. Rendering cadence never changes duration.
class Countdown {
  public:
    static constexpr int64_t duration = 300LL * 1000000;
    int64_t remaining(int64_t now) const {
        const int64_t value = running_ ? saved_ - (now - started_) : saved_;
        return value > 0 ? value : 0;
    }
    bool running(int64_t now) const { return running_ && remaining(now) > 0; }
    void toggle(int64_t now) {
        if (running(now)) {
            saved_ = remaining(now);
            running_ = false;
        } else {
            if (remaining(now) == 0)
                saved_ = duration;
            started_ = now;
            running_ = true;
        }
    }
    void reset() {
        saved_ = duration;
        started_ = 0;
        running_ = false;
    }

  private:
    int64_t saved_ = duration;
    int64_t started_ = 0;
    bool running_ = false;
};

// Debounced events: a long press emits once, and suppresses its release click.
class Button {
  public:
    enum Event { None, Click, Hold };
    Event update(bool down, int64_t now) {
        if (down != raw_) {
            raw_ = down;
            changed_ = now;
        }
        if (raw_ != stable_ && now - changed_ >= 25000) {
            stable_ = raw_;
            if (stable_) {
                pressed_ = now;
                held_ = false;
            } else if (!held_)
                return Click;
        }
        if (stable_ && !held_ && now - pressed_ >= 800000) {
            held_ = true;
            return Hold;
        }
        return None;
    }

  private:
    bool raw_ = false, stable_ = false, held_ = false;
    int64_t changed_ = 0, pressed_ = 0;
};
