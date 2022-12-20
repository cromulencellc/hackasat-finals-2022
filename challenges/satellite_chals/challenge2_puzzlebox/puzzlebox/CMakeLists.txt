cmake_minimum_required(VERSION 2.6.4)
project(CFE_PUZZLEBOX CXX)


# Define external includes
include_directories(${osk_app_fw_MISSION_DIR}/fsw/platform_inc)
include_directories(${osk_app_fw_MISSION_DIR}/fsw/mission_inc)
include_directories(${osk_app_fw_MISSION_DIR}/fsw/app_inc)


# Define source files
set( sources    entrypoint.c
                fsw/src/initialization.c
                fsw/src/puzzlebox.c
)


# Create the app module
add_cfe_app(puzzlebox ${sources})
