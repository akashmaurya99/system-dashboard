cmake_minimum_required(VERSION 3.14)
project(mac_system_info)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)

# Set macOS deployment target (ensure compatibility with older versions if needed)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.14")

# Find macOS system libraries and frameworks
find_library(COREFOUNDATION_LIBRARY CoreFoundation)
find_library(IOKIT_LIBRARY IOKit)
find_library(APPKIT_LIBRARY AppKit)
find_library(SYSTEMCONFIG_LIBRARY SystemConfiguration)
find_library(APPLICATIONSERVICES_LIBRARY ApplicationServices)

# Define shared library
add_library(mac_system_info SHARED 
    ../ffi/mac/mac_system_info.cpp
    ../ffi/mac/battery_info.cpp
    ../ffi/mac/cpu_info.cpp
    ../ffi/mac/disk_info.cpp
    ../ffi/mac/gpu_info.cpp
    ../ffi/mac/os_info.cpp
    ../ffi/mac/ram_info.cpp
    ../ffi/mac/running_app_info.cpp
    ../ffi/mac/utils/strdup_cstr.cpp
)

# Set include directories
target_include_directories(mac_system_info PUBLIC ffi/mac/include)

# Link required macOS frameworks
target_link_libraries(mac_system_info 
    ${COREFOUNDATION_LIBRARY} 
    ${IOKIT_LIBRARY} 
    ${APPKIT_LIBRARY} 
    ${SYSTEMCONFIG_LIBRARY}
    ${APPLICATIONSERVICES_LIBRARY}
)

# Set output directory for shared library
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})



# add_custom_command(
#     TARGET mac_system_info POST_BUILD
#     # First, ensure the destination directory exists.
#     COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/Products/Release/system_info.app/Contents/MacOS
#     # Then, copy the dylib into that directory.
#     COMMAND ${CMAKE_COMMAND} -E copy_if_different
#         $<TARGET_FILE:mac_system_info>
#         ${CMAKE_BINARY_DIR}/Products/Release/system_info.app/Contents/MacOS/libmac_system_info.dylib
# )


# # Ensure the library is copied to the app bundle
# add_custom_command(
#     TARGET mac_system_info POST_BUILD
#     COMMAND ${CMAKE_COMMAND} -E copy
#     $<TARGET_FILE:mac_system_info> ${CMAKE_BINARY_DIR}/../system_info.app/Contents/MacOS/libmac_system_info.dylib
# )

