#include "include/battery_info.h"
#include "include/cpu_info.h"
#include "include/disk_info.h"
#include "include/gpu_info.h"
#include "include/os_info.h"
#include "include/ram_info.h"
#include "include/running_app_info.h"
#include "include/common.h"

#include <string>
#include <cstdlib>
#include <cstring>
#include <windows.h>

extern "C" {

// Get Battery Info
__declspec(dllexport) char* batteryInfo() {
    return getBatteryInfo();
}

// Get CPU Info
__declspec(dllexport) char* cpuData() {
    return getCpuInfoJson();
}

// Get real-time CPU Usage
__declspec(dllexport) double cpuUsages() {
    return getCPUUsage();
}

// Get Disk Details
__declspec(dllexport) char* diskDetails() {
    return getDiskInfoJson();
}

// Get GPU Info
__declspec(dllexport) char* gpuInfo() {
    return getGPUInfoJson();
}

// Get GPU Usage
__declspec(dllexport) double gpuUsages() {
    return getGPUUsage();
}

// Get OS Info
__declspec(dllexport) char* osInfo() {
    return getOsInfoJson();
}

// Get RAM Info
__declspec(dllexport) char* ramInfo() {
    return getRamInfoJson();
}

// Get Running Processes
__declspec(dllexport) char* runningProcesses() {
    return getRunningProcessesJson();
}

// Free allocated memory for FFI
__declspec(dllexport) void free_cstr(char* ptr) {
    if (ptr) {
        free(ptr);
    }
}

} // extern "C"