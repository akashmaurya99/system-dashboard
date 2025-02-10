#ifndef BATTERY_INFO_H
#define BATTERY_INFO_H

#include <string>

#ifdef __cplusplus
extern "C" {
#endif

__attribute__((visibility("default"))) char* getBatteryInfo();
// Free dynamically allocated memory
__attribute__((visibility("default"))) void free_cstr(char* ptr);

#ifdef __cplusplus
}
#endif

#endif // BATTERY_INFO_H






// #ifndef BATTERY_INFO_H
// #define BATTERY_INFO_H

// #include <string>

// #ifdef __cplusplus
// extern "C" {
// #endif

// char* getBatteryInfo();  // Expose the function as C

// #ifdef __cplusplus
// }
// #endif

// #endif // BATTERY_INFO_H
