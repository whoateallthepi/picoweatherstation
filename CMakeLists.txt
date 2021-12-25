cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(openaws_project)

pico_sdk_init()

add_executable(openaws openaws.c openaws.h constants.h types.h uart_lora.c uart_lora.h rak811.c rak811.h bme280.h bme280.c formatCSV.h formatCSV.c core1_processing.c core1_processing.h utilities.c utilities.h formatmessages.h formatmessages.c)

pico_enable_stdio_usb(openaws 1)
pico_enable_stdio_uart(openaws 0)

pico_add_extra_outputs(openaws)

target_link_libraries(openaws pico_stdlib hardware_uart hardware_rtc
   hardware_adc hardware_spi pico_unique_id pico_multicore )

