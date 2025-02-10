#ifndef CPU_INFO_H
#define CPU_INFO_H

#include <string>

#ifdef __cplusplus
extern "C" {
#endif

// Get CPU information in JSON format
__attribute__((visibility("default"))) char* getJsonCpuData();

// Get real-time CPU usage
__attribute__((visibility("default"))) double getCPUUsageExtern();

// Free dynamically allocated memory
__attribute__((visibility("default"))) void free_cstr(char* ptr);

#ifdef __cplusplus
}
#endif

#endif // CPU_INFO_H





// #ifndef CPU_INFO_H
// #define CPU_INFO_H

// #include <string>

// std::string jsonCpuData();
// double getCPUUsage();

// #endif // CPU_INFO_H





// #ifndef CPU_INFO_H
// #define CPU_INFO_H

// using namespace std;

// #include <string>

// string jsonCpuData();
// double getCPUUsage();

// #endif
