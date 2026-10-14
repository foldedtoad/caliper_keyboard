# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

set_property(GLOBAL APPEND PROPERTY HEX_FILES_TO_MERGE
      ${app_provision_hex})

set(BOARD_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(DTS_ROOT_BINDINGS "${CMAKE_CURRENT_SOURCE_DIR}")

# Select the board on the command line, e.g.:
#   west build -b xiao_caliper/nrf52840
# (a BOARD set here would override -b and defeat board qualifiers)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(caliper)

include_directories(app PRIVATE inc)

FILE(GLOB app_sources src/*.c)
target_sources(app PRIVATE
  ${app_sources}
  )

# zephyr_compile_options(-save-temps)
