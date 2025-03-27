#include "include/battery_info.h"
#include "include/cpu_info.h"
#include "include/disk_info.h"
#include "include/gpu_info.h"
#include "include/os_info.h"
#include "include/ram_info.h"
#include "include/running_app_info.h"
#include "include/free_cstr.h"

#include <string>
#include <cstdlib>
#include <cstring>
#include "include/strdup_cstr.h"

extern "C" {


// Get Battery Info
__attribute__((visibility("default"))) char* batteryInfo() {
    return getBatteryInfo(); // Calls implementation from battery_info.cpp
}

// Get CPU Info
__attribute__((visibility("default"))) char* cpuData() {
    return getJsonCpuData(); // Calls correct implementation
}

// Get real-time CPU Usage
__attribute__((visibility("default"))) double cpuUsages() {
    return getCPUUsage(); // Calls correct implementation
}

// // Get Disk Usage
// __attribute__((visibility("default"))) char* diskUsages(const char* diskPath) {
//     return strdup_cstr(getDiskUsageInternal(diskPath)); // Calls correct implementation
// }

// // Get Disk Speed
// __attribute__((visibility("default"))) char* diskSpeed() {
//     return strdup_cstr(getDiskSpeedInternal()); // Calls correct implementation
// }

// Get Disk Details
__attribute__((visibility("default"))) char* diskDetails() {
    return getDiskInfo(); // Calls correct implementation
}

// Get GPU Info
__attribute__((visibility("default"))) char* gpuInfo() {
    return getGPUInfo(); // Calls correct implementation
}

// Get GPU Usage
__attribute__((visibility("default"))) double gpuUsages() {
    return getGPUUsage(); // Calls correct implementation
}

// Get OS Info
__attribute__((visibility("default"))) char* osInfo() {
    return getOsInfoJson(); // Calls correct implementation
}

// Get RAM Info
__attribute__((visibility("default"))) char* ramInfo() {
    return getRAMInfoJSON(); // Calls correct implementation
}

// Get Installed Applications
__attribute__((visibility("default"))) char* installedApplications() {
    return getInstalledApplicationsJSON(); // Calls correct implementation
}

// Get Running Processes
__attribute__((visibility("default"))) char* runningProcesses() {
    return getRunningProcessesJSON(); // Calls correct implementation
}

// Free allocated memory for FFI
__attribute__((visibility("default"))) void free_cstr(char* ptr) {
    if (ptr) {
        free(ptr);
    }
}

}