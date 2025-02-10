#ifndef RAM_INFO_H
#define RAM_INFO_H

#ifdef __cplusplus
extern "C" {
#endif

// Get RAM information in JSON format
__attribute__((visibility("default"))) char* getRAMInfoJSON();

// Free allocated memory for FFI
__attribute__((visibility("default"))) void free_cstr(char* ptr);

#ifdef __cplusplus
}
#endif

#endif // RAM_INFO_H






// #ifndef RAM_INFO_H
// #define RAM_INFO_H

// #include <string>

// std::string getRAMInfoJSON();

// #endif // RAM_INFO_H
