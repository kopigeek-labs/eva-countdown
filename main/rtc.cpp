// SPDX-License-Identifier: Apache-2.0
#include "rtc.h"
#include "board.h"
#include "driver/i2c_master.h"

namespace {
i2c_master_bus_handle_t bus = nullptr;
i2c_master_dev_handle_t device = nullptr;
bool read(uint8_t reg, uint8_t *data, size_t count) {
    return device && i2c_master_transmit_receive(device, &reg, 1, data, count, 30) == ESP_OK;
}
bool write(uint8_t reg, uint8_t value) {
    const uint8_t data[]{reg, value};
    return device && i2c_master_transmit(device, data, sizeof(data), 30) == ESP_OK;
}
} // namespace
bool rtc_init() {
    i2c_master_bus_config_t config{};
    config.i2c_port = I2C_NUM_0;
    config.sda_io_num = board::sda;
    config.scl_io_num = board::scl;
    config.clk_source = I2C_CLK_SRC_DEFAULT;
    config.glitch_ignore_cnt = 7;
    // Board has 10k external pull-ups. Keep the bus at 100 kHz.
    config.flags.enable_internal_pullup = true;
    if (i2c_new_master_bus(&config, &bus) != ESP_OK)
        return false;
    i2c_device_config_t dev{};
    dev.dev_addr_length = I2C_ADDR_BIT_LEN_7;
    dev.device_address = 0x51;
    dev.scl_speed_hz = 100000;
    if (i2c_master_bus_add_device(bus, &dev, &device) != ESP_OK)
        return false;
    // Do not reset the RTC or clear oscillator-stop on boot.
    return true;
}
bool rtc_read(eva::DateTime &time) {
    uint8_t registers[11]{};
    return read(0, registers, sizeof(registers)) &&
           eva::decode_rtc(registers[0], registers + 4, time);
}
bool rtc_set(const eva::DateTime &time) {
    if (!eva::valid(time))
        return false;
    uint8_t control = 0;
    if (!read(0, &control, 1))
        return false;
    // Preserve load-capacitance selection, stop the divider, select 24-hour time.
    const uint8_t normal = control & 1;
    if (!write(0, normal | 0x20))
        return false;
    uint8_t clock[7];
    eva::encode_rtc(time, clock);
    uint8_t data[8]{0x04};
    for (int i = 0; i < 7; ++i)
        data[i + 1] = clock[i];
    // On a failed/partial write, leave STOP set: the UI must report invalid time.
    if (i2c_master_transmit(device, data, sizeof(data), 30) != ESP_OK)
        return false;
    if (!write(0, normal))
        return false;
    eva::DateTime verified;
    if (!rtc_read(verified))
        return false;
    return verified.year == time.year && verified.month == time.month && verified.day == time.day &&
           verified.hour == time.hour && verified.minute == time.minute &&
           verified.second == time.second;
}
