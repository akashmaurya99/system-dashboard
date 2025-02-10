#ifndef GPU_INFO_H
#define GPU_INFO_H

#include <string>

#ifdef __cplusplus
extern "C" {
#endif

// Get GPU information in JSON format
__attribute__((visibility("default"))) char* getGPUInfo();

// Get GPU utilization in percentage
__attribute__((visibility("default"))) double calculateGPUUsage();

// Free dynamically allocated memory
__attribute__((visibility("default"))) void free_cstr(char* ptr);

#ifdef __cplusplus
}
#endif

#endif // GPU_INFO_H





// #ifndef GPU_INFO_H
// #define GPU_INFO_H

// #include <string>

// std::string extract_gpu_info();
// double getGPUUsage();

// #endif // GPU_INFO_H