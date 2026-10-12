// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "driver/gpio.h"
namespace board {
// Waveshare ESP32-S3-RLCD-4.2 only. See THIRD_PARTY.md for the pinned vendor source.
constexpr gpio_num_t key = GPIO_NUM_18;
constexpr gpio_num_t boot = GPIO_NUM_0;
constexpr gpio_num_t mosi = GPIO_NUM_12;
constexpr gpio_num_t clock = GPIO_NUM_11;
constexpr gpio_num_t dc = GPIO_NUM_5;
constexpr gpio_num_t cs = GPIO_NUM_40;
constexpr gpio_num_t reset = GPIO_NUM_41;
constexpr int spi_hz = 10'000'000;
constexpr int64_t frame_interval_us = 40'000;
} // namespace board
