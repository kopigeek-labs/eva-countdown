idf_component_register(SRCS "main.cpp" "display.cpp" "rtc.cpp"
    INCLUDE_DIRS "." REQUIRES driver esp_timer esp_app_format esp_driver_i2c esp_driver_usb_serial_jtag)
