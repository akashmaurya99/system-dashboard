cmake_minimum_required(VERSION 3.14)
project(mac_system_info)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)

# Find macOS system libraries
find_library(COREFOUNDATION_LIBRARY CoreFoundation)
find_library(IOKIT_LIBRARY IOKit)
find_library(APPKIT_LIBRARY AppKit)
find_library(SYSTEMCONFIG_LIBRARY SystemConfiguration)

# Define shared library
add_library(mac_system_info SHARED 
    mac_system_info.cpp
    battery_info.cpp
    cpu_info.cpp
    disk_info.cpp
    gpu_info.cpp
    os_info.cpp
    ram_info.cpp
    running_app_info.cpp
)

# Set include directories (modern approach)
target_include_directories(mac_system_info PUBLIC include)

# Link required macOS frameworks
target_link_libraries(mac_system_info 
    ${COREFOUNDATION_LIBRARY} 
    ${IOKIT_LIBRARY} 
    ${APPKIT_LIBRARY} 
    ${SYSTEMCONFIG_LIBRARY}
)

# Optional: Set macOS deployment target (adjust as needed)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.14")

set(MAC_SYSTEM_INFO_LIBRARY ${CMAKE_CURRENT_SOURCE_DIR}/libmac_system_info.dylib)
target_link_libraries(Runner PRIVATE ${MAC_SYSTEM_INFO_LIBRARY})
