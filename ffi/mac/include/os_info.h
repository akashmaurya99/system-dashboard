#ifndef OS_INFO_H
#define OS_INFO_H

#ifdef __cplusplus
extern "C" {
#endif

// Get OS information in JSON format
__attribute__((visibility("default"))) char* getOsInfoJson();

// Free allocated memory for FFI
__attribute__((visibility("default"))) void free_cstr(char* ptr);

#ifdef __cplusplus
}
#endif

#endif // OS_INFO_H




// #ifndef OS_INFO_H
// #define OS_INFO_H

// #include <string>

// std::string getOsInfoJson();

// #endif // OS_INFO_H
