cmake_minimum_required(VERSION 3.0)

project(usbdatasender LANGUAGES C)

add_executable(usbdatasender main.c)

target_link_libraries(usbdatasender -lusb-1.0)

install(TARGETS usbdatasender RUNTIME DESTINATION bin)
