cmake_minimum_required(VERSION 3.20.0)

# Custom board definitions live in boards/
set(BOARD_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(stem-player-bsp)

# ── BSP sources (board support — generally don't modify) ─────────
set(BSP_SOURCES
    subsys/system/ResetBreadcrumbs.cpp
    subsys/system/Watchdog.cpp
    subsys/power/PowerManager.cpp
    drivers/charger/charger_bq24232.c
    drivers/audio/cs42l42_codec.c
    drivers/audio/tas2505_codec.c
)

# ── App sources (your code here) ─────────────────────────────────
set(APP_SOURCES
    app/main.cpp
    app/AppStateMachine.cpp
    app/MidiController.cpp
    app/Leds.cpp
)

target_sources(app PRIVATE ${BSP_SOURCES} ${APP_SOURCES})
target_include_directories(app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/app
    ${CMAKE_CURRENT_SOURCE_DIR}/subsys
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${ZEPHYR_BASE}/lib/midi2
)

# C++17, no exceptions/RTTI for embedded.
target_compile_features(app PRIVATE cxx_std_17)
target_compile_options(app PRIVATE
    $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions -fno-rtti>
)
