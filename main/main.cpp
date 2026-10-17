// SPDX-License-Identifier: Apache-2.0
#include "board.h"
#include "display.h"
#include "app.h"
#include "render.h"
#include "rtc.h"
#include "driver/gpio.h"
#include "driver/usb_serial_jtag.h"
#include "driver/usb_serial_jtag_vfs.h"
#include "esp_app_desc.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <cstdio>
#include <cstring>

namespace {
constexpr const char *tag = "eva";
void reply(const char *line) {
    // Finite wait: a disconnected USB console must not stall animation/timers.
    usb_serial_jtag_write_bytes(line, std::strlen(line), pdMS_TO_TICKS(10));
}
void clock_reply(const char *prefix, const eva::DateTime &t) {
    char buffer[80];
    std::snprintf(buffer, sizeof(buffer), "%s%04d-%02d-%02dT%02d:%02d:%02d\n", prefix, t.year,
                  t.month, t.day, t.hour, t.minute, t.second);
    reply(buffer);
}
void command(const char *line, eva::App &app) {
    if (std::strcmp(line, "INFO") == 0) {
        char b[120];
        std::snprintf(b, sizeof(b), "EVA %s BOARD=WAVESHARE-ESP32-S3-RLCD-4.2 POWER=USB-HOST\n",
                      esp_app_get_description()->version);
        reply(b);
    } else if (std::strcmp(line, "TIME?") == 0) {
        app.rtc_valid = rtc_read(app.clock);
        if (app.rtc_valid)
            clock_reply("TIME ", app.clock);
        else
            reply("ERR RTC_UNSET_OR_UNAVAILABLE\n");
    } else {
        eva::DateTime time;
        if (!eva::parse_time(line, time)) {
            reply("ERR EXPECTED_TIME_YYYY-MM-DDTHH:MM:SS\n");
            return;
        }
        app.rtc_valid = false;
        if (!rtc_set(time)) {
            reply("ERR RTC_WRITE_OR_VERIFY_FAILED\n");
            return;
        }
        app.clock = time;
        app.rtc_valid = true;
        clock_reply("OK TIME ", time);
    }
}
} // namespace

extern "C" void app_main(void) {
    gpio_config_t buttons{};
    buttons.pin_bit_mask = (1ULL << board::key) | (1ULL << board::boot);
    buttons.mode = GPIO_MODE_INPUT;
    buttons.pull_up_en = GPIO_PULLUP_ENABLE;
    ESP_ERROR_CHECK(gpio_config(&buttons));
    usb_serial_jtag_driver_config_t usb{};
    usb.rx_buffer_size = 256;
    usb.tx_buffer_size = 512;
    ESP_ERROR_CHECK(usb_serial_jtag_driver_install(&usb));
    usb_serial_jtag_vfs_use_driver();
    static eva::App app;
    const bool rtc_ready = rtc_init();
    if (rtc_ready)
        app.rtc_valid = rtc_read(app.clock);
    ESP_LOGI(tag, "v%s: BOOT next screen; hold BOOT Normal/Racing; KEY action; hold KEY reset",
             esp_app_get_description()->version);
    ESP_LOGI(tag,
             "USB-host detection only; host sleep looks disconnected; wall chargers unsupported");
    display_init();
    static eva::Frame frame, previous;
    Button key, boot;
    eva::LineReader lines;
    bool first = true;
    int64_t last_frame = -board::frame_interval_us, last_rtc = 0, last_power = -50000;
    for (;;) {
        const auto now = esp_timer_get_time();
        if (now - last_power >= 50000) {
            // Explicit owner-approved stock-board compromise; never infer VBUS from BAT_ADC/STAT.
            app.sample_power(
                usb_serial_jtag_is_connected() ? eva::Power::External : eva::Power::Internal, now);
            last_power = now;
        }
        app.input(key.update(gpio_get_level(board::key) == 0, now),
                  boot.update(gpio_get_level(board::boot) == 0, now), now);
        app.tick(now);
        char input[64];
        const int count = usb_serial_jtag_read_bytes(input, sizeof(input), 0);
        for (int i = 0; i < count; ++i)
            if (lines.push(input[i]))
                command(lines.line(), app);
        if (rtc_ready && now - last_rtc >= 500000) {
            app.rtc_valid = rtc_read(app.clock);
            last_rtc = now;
        }
        if (now - last_frame >= board::frame_interval_us) {
            eva::render(frame, app, now);
            if (first || std::memcmp(frame.data, previous.data, eva::bytes) != 0) {
                display_frame(frame);
                std::memcpy(previous.data, frame.data, eva::bytes);
                first = false;
            }
            last_frame = now;
        }
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}
