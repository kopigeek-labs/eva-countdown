// SPDX-License-Identifier: Apache-2.0
#include "display.h"
#include "board.h"
#include "timer.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

extern "C" void app_main() {
    gpio_config_t inputs{};
    inputs.pin_bit_mask = (1ULL << board::key) | (1ULL << board::boot);
    inputs.mode = GPIO_MODE_INPUT;
    inputs.pull_up_en = GPIO_PULLUP_ENABLE;
    ESP_ERROR_CHECK(gpio_config(&inputs));
    display_init();
    static eva::Frame frame, previous;
    Countdown timer;
    Button key, boot;
    bool first = true;
    int64_t next_frame = 0;
    ESP_LOGI("eva", "Ready: KEY start/pause; hold KEY or press BOOT reset. 5 minutes.");
    while (true) {
        const int64_t now = esp_timer_get_time();
        const auto key_event = key.update(gpio_get_level(board::key) == 0, now);
        const auto boot_event = boot.update(gpio_get_level(board::boot) == 0, now);
        if (key_event == Button::Hold || boot_event == Button::Click ||
            boot_event == Button::Hold) {
            timer.reset();
            next_frame = 0;
            ESP_LOGI("eva", "Reset");
        } else if (key_event == Button::Click) {
            timer.toggle(now);
            next_frame = 0;
            ESP_LOGI("eva", "%s", timer.running(now) ? "Running" : "Paused");
        }
        if (now >= next_frame) {
            eva::render(frame, timer.remaining(now), timer.running(now), now);
            if (first || std::memcmp(frame.data, previous.data, eva::bytes) != 0) {
                display_frame(frame);
                previous = frame;
                first = false;
            }
            next_frame =
                now + board::frame_interval_us; // 25 Hz target; elapsed time uses monotonic clock.
        }
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}
