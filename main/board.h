// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <cstdint>
#include "driver/gpio.h"
namespace board {
// Waveshare ESP32-S3-RLCD-4.2 only. See THIRD_PARTY.md.
constexpr gpio_num_t key = GPIO_NUM_18, boot = GPIO_NUM_0;
constexpr gpio_num_t mosi = GPIO_NUM_12, clock = GPIO_NUM_11, dc = GPIO_NUM_5;
constexpr gpio_num_t cs = GPIO_NUM_40, reset = GPIO_NUM_41;
constexpr gpio_num_t sda = GPIO_NUM_13, scl = GPIO_NUM_14;
constexpr int spi_hz = 10000000;
constexpr int64_t frame_interval_us = 40000;
} // namespace board
