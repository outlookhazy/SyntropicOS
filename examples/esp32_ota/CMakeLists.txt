cmake_minimum_required(VERSION 3.16)

# Point to SyntropicOS sources (two levels up from examples/esp32_ota/)
set(SYNTROPIC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../syntropic)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(syntropic_esp32_ota)
