// SPDX-License-Identifier: Apache-2.0
// Adapted from Waveshare display_bsp.cpp, Apache-2.0, copyright 2026 Waveshare.
// Modified: explicit synchronous SPI, internal DMA buffer, no PSRAM/LUT allocation.
#include "display.h"
#include "board.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_check.h"
#include "esp_heap_caps.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <cassert>

namespace {
constexpr gpio_num_t dc = board::dc, rst = board::reset;
spi_device_handle_t spi;
uint8_t *wire_buffer;
void send(bool is_data, const void *buffer, size_t bytes) {
    ESP_ERROR_CHECK(gpio_set_level(dc, is_data));
    spi_transaction_t tx{};
    tx.length = bytes * 8;
    tx.tx_buffer = buffer;
    ESP_ERROR_CHECK(spi_device_polling_transmit(spi, &tx));
}
void command(uint8_t value) { send(false, &value, 1); }
void data(uint8_t value) { send(true, &value, 1); }
void reset() {
    ESP_ERROR_CHECK(gpio_set_level(rst, 1));
    vTaskDelay(pdMS_TO_TICKS(50));
    ESP_ERROR_CHECK(gpio_set_level(rst, 0));
    vTaskDelay(pdMS_TO_TICKS(20));
    ESP_ERROR_CHECK(gpio_set_level(rst, 1));
    vTaskDelay(pdMS_TO_TICKS(50));
}
} // namespace
void display_init() {
    gpio_config_t gpio{};
    gpio.pin_bit_mask = (1ULL << dc) | (1ULL << rst);
    gpio.mode = GPIO_MODE_OUTPUT;
    ESP_ERROR_CHECK(gpio_config(&gpio));
    spi_bus_config_t bus{};
    bus.mosi_io_num = board::mosi;
    bus.miso_io_num = -1;
    bus.sclk_io_num = board::clock;
    bus.quadwp_io_num = -1;
    bus.quadhd_io_num = -1;
    bus.max_transfer_sz = eva::bytes;
    ESP_ERROR_CHECK(spi_bus_initialize(SPI3_HOST, &bus, SPI_DMA_CH_AUTO));
    spi_device_interface_config_t device{};
    device.clock_speed_hz = board::spi_hz;
    device.mode = 0;
    device.spics_io_num = board::cs;
    device.queue_size = 1;
    ESP_ERROR_CHECK(spi_bus_add_device(SPI3_HOST, &device, &spi));
    wire_buffer =
        static_cast<uint8_t *>(heap_caps_malloc(eva::bytes, MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL));
    assert(wire_buffer);

    reset();

    command(0xD6); // NVM Load Control
    data(0x17);
    data(0x02);

    command(0xD1); // Booster Enable
    data(0x01);

    command(0xC0); // Gate Voltage Control
    data(0x11);
    data(0x04);

    command(0xC1); // VSHP Setting
    data(0x69);
    data(0x69);
    data(0x69);
    data(0x69);

    command(0xC2);
    data(0x19);
    data(0x19);
    data(0x19);
    data(0x19);

    command(0xC4);
    data(0x4B);
    data(0x4B);
    data(0x4B);
    data(0x4B);

    command(0xC5);
    data(0x19);
    data(0x19);
    data(0x19);
    data(0x19);

    command(0xD8);
    data(0x80);
    data(0xE9);

    command(0xB2);
    data(0x02);

    command(0xB3);
    data(0xE5);
    data(0xF6);
    data(0x05);
    data(0x46);
    data(0x77);
    data(0x77);
    data(0x77);
    data(0x77);
    data(0x76);
    data(0x45);

    command(0xB4);
    data(0x05);
    data(0x46);
    data(0x77);
    data(0x77);
    data(0x77);
    data(0x77);
    data(0x76);
    data(0x45);

    command(0x62);
    data(0x32);
    data(0x03);
    data(0x1F);

    command(0xB7);
    data(0x13);

    command(0xB0);
    data(0x64);

    command(0x11);
    vTaskDelay(pdMS_TO_TICKS(200));
    command(0xC9);
    data(0x00);

    command(0x36);
    data(0x48);

    command(0x3A);
    data(0x11);

    command(0xB9);
    data(0x20);

    command(0xB8);
    data(0x29);

    command(0x21);

    command(0x2A);
    data(0x12);
    data(0x2A);

    command(0x2B);
    data(0x00);
    data(0xC7);

    command(0x35);
    data(0x00);

    command(0xD0);
    data(0xFF);

    command(0x38);
    command(0x29);
}
void display_frame(const eva::Frame &frame) {
    eva::pack_panel(frame, wire_buffer);
    command(0x2A);
    data(0x12);
    data(0x2A);
    command(0x2B);
    data(0x00);
    data(0xC7);
    command(0x2C);
    send(true, wire_buffer, eva::bytes); // Completes before the buffer can be reused.
}
