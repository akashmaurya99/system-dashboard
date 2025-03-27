cmake_minimum_required(VERSION 3.10)
project(windows_system_info)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Find required packages
find_package(nlohmann_json CONFIG REQUIRED)

# Add source files
set(SOURCES
    windows_system_info.cpp
    cpu_info.cpp
    gpu_info.cpp
    fan_speed.cpp
    battery_info.cpp
    disk_info.cpp
    ram_info.cpp
    os_info.cpp
    running_app_info.cpp
    common.cpp
)

# Create shared library
add_library(windows_system_info SHARED ${SOURCES})

# Link against required libraries
target_link_libraries(windows_system_info PRIVATE
    nlohmann_json::nlohmann_json
    wbemuuid
    pdh
    setupapi
    cfgmgr32
    iphlpapi
    psapi
    wtsapi32
    version
)

# Set output directory
set_target_properties(windows_system_info PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

# Install target
install(TARGETS windows_system_info
    RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
    LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
)