// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <cstdint>

// Monotonic microseconds: neither rendering nor RTC adjustments change duration.
class Countdown {
  public:
    static constexpr int64_t duration = 300LL * 1000000;
    explicit Countdown(int64_t length = duration) : length_(length), saved_(length) {}
    int64_t remaining(int64_t now) const {
        const int64_t value = running_ ? saved_ - (now - started_) : saved_;
        return value > 0 ? value : 0;
    }
    int64_t length() const { return length_; }
    bool running(int64_t now) const { return running_ && remaining(now) > 0; }
    void start(int64_t now) {
        if (running(now))
            return;
        if (remaining(now) == 0)
            saved_ = length_;
        started_ = now;
        running_ = true;
    }
    void toggle(int64_t now) {
        if (running(now)) {
            saved_ = remaining(now);
            running_ = false;
        } else
            start(now);
    }
    void reset() {
        saved_ = length_;
        started_ = 0;
        running_ = false;
    }
    void set_length(int64_t length) {
        length_ = length;
        reset();
    }

  private:
    int64_t length_, saved_, started_ = 0;
    bool running_ = false;
};

// One hold event; its release does not also generate a click.
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
