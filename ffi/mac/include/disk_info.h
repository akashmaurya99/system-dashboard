#ifndef DISK_INFO_H
#define DISK_INFO_H

#include <string>

#ifdef __cplusplus
extern "C" {
#endif

// Get disk usage info in JSON format
__attribute__((visibility("default"))) char* getDiskUsageInternal(const char* diskPath);

// Get disk speed in JSON format
__attribute__((visibility("default"))) char* getDiskSpeedInternal();

// Get detailed disk info in JSON format
__attribute__((visibility("default"))) char* getDiskDetailsInternal();

// Free dynamically allocated memory
__attribute__((visibility("default"))) void free_cstr(char* ptr);

#ifdef __cplusplus
}
#endif

#endif // DISK_INFO_H




// #ifndef DISK_INFO_H
// #define DISK_INFO_H

// #include <string>

// std::string getDiskUsage(const std::string &diskPath);  // Add the parameter
// std::string getDiskSpeed();
// std::string getDiskDetails();

// #endif // DISK_INFO_H
