// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <cstdint>
#include <cstring>

namespace eva {
struct DateTime {
    int year = 0, month = 0, day = 0, hour = 0, minute = 0, second = 0;
};
inline bool leap(int year) { return year % 4 == 0 && (year % 100 != 0 || year % 400 == 0); }
inline int month_days(int year, int month) {
    constexpr int days[]{31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    return month >= 1 && month <= 12 ? days[month - 1] + (month == 2 && leap(year)) : 0;
}
inline bool valid(const DateTime &t) {
    return t.year >= 2000 && t.year <= 2099 && t.month >= 1 && t.month <= 12 && t.day >= 1 &&
           t.day <= month_days(t.year, t.month) && t.hour >= 0 && t.hour < 24 && t.minute >= 0 &&
           t.minute < 60 && t.second >= 0 && t.second < 60;
}
inline int weekday(const DateTime &t) { // Sunday = 0, PCF85063A convention.
    int days = 0;
    for (int y = 2000; y < t.year; ++y)
        days += leap(y) ? 366 : 365;
    for (int m = 1; m < t.month; ++m)
        days += month_days(t.year, m);
    return (6 + days + t.day - 1) % 7;
}
inline uint8_t bcd(int n) { return uint8_t((n / 10) * 16 + n % 10); }
inline int unbcd(uint8_t n) {
    return (n & 15) <= 9 && (n >> 4) <= 9 ? (n >> 4) * 10 + (n & 15) : -1;
}
inline void encode_rtc(const DateTime &t, uint8_t (&r)[7]) {
    r[0] = bcd(t.second);
    r[1] = bcd(t.minute);
    r[2] = bcd(t.hour);
    r[3] = bcd(t.day);
    r[4] = uint8_t(weekday(t));
    r[5] = bcd(t.month);
    r[6] = bcd(t.year - 2000);
}
inline bool decode_rtc(uint8_t control, const uint8_t *r, DateTime &out) {
    // Reject stopped/test/12h operation and oscillator-stop flag; never clear OS on read.
    if ((control & 0xa2) || (r[0] & 0x80) || (r[1] & 0x80) || (r[2] & 0xc0) || (r[3] & 0xc0) ||
        r[4] > 6 || (r[5] & 0xe0))
        return false;
    const int year = unbcd(r[6]);
    DateTime t{2000 + year, unbcd(r[5]), unbcd(r[3]), unbcd(r[2]), unbcd(r[1]), unbcd(r[0])};
    if (year < 0 || !valid(t))
        return false;
    out = t;
    return true;
}
// Exact, bounded protocol: TIME YYYY-MM-DDTHH:MM:SS (local wall time).
inline bool parse_time(const char *line, DateTime &out) {
    if (std::strlen(line) != 24 || std::strncmp(line, "TIME ", 5) != 0)
        return false;
    const char *s = line + 5;
    for (int i = 0; i < 19; ++i) {
        if (i == 4 || i == 7) {
            if (s[i] != '-')
                return false;
        } else if (i == 10) {
            if (s[i] != 'T')
                return false;
        } else if (i == 13 || i == 16) {
            if (s[i] != ':')
                return false;
        } else if (s[i] < '0' || s[i] > '9')
            return false;
    }
    auto two = [s](int i) { return (s[i] - '0') * 10 + s[i + 1] - '0'; };
    DateTime t{two(0) * 100 + two(2), two(5), two(8), two(11), two(14), two(17)};
    if (!valid(t))
        return false;
    out = t;
    return true;
}
// Discard an entire oversized line, rather than interpreting its tail as a command.
class LineReader {
  public:
    bool push(char c) {
        if (c == '\r')
            return false;
        if (c == '\n') {
            data_[size_] = 0;
            const bool ready = !overflow_ && size_ > 0;
            size_ = 0;
            overflow_ = false;
            return ready;
        }
        if (size_ + 1 < sizeof(data_) && !overflow_)
            data_[size_++] = c;
        else
            overflow_ = true;
        return false;
    }
    const char *line() const { return data_; }

  private:
    char data_[80]{};
    unsigned size_ = 0;
    bool overflow_ = false;
};
} // namespace eva
