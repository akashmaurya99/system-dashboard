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


// ✅ Get Battery Info
__attribute__((visibility("default"))) char* batteryInfo() {
    return strdup_cstr(getBatteryInfo()); // Calls correct implementation
}

// ✅ Get CPU Info
__attribute__((visibility("default"))) char* cpuData() {
    return strdup_cstr(getJsonCpuData()); // Calls correct implementation
}

// ✅ Get real-time CPU Usage
__attribute__((visibility("default"))) double cpuUsages() {
    return getCPUUsageExtern(); // Calls correct implementation
}

// ✅ Get Disk Usage
__attribute__((visibility("default"))) char* diskUsages(const char* diskPath) {
    return strdup_cstr(getDiskUsageInternal(diskPath)); // Calls correct implementation
}

// ✅ Get Disk Speed
__attribute__((visibility("default"))) char* diskSpeed() {
    return strdup_cstr(getDiskSpeedInternal()); // Calls correct implementation
}

// ✅ Get Disk Details
__attribute__((visibility("default"))) char* diskDetails() {
    return strdup_cstr(getDiskDetailsInternal()); // Calls correct implementation
}

// ✅ Get GPU Info
__attribute__((visibility("default"))) char* gpuInfo() {
    return strdup_cstr(getGPUInfo()); // Calls correct implementation
}

// ✅ Get GPU Usage
__attribute__((visibility("default"))) double gpuUsages() {
    return calculateGPUUsage(); // Calls correct implementation
}

// ✅ Get OS Info
__attribute__((visibility("default"))) char* osInfo() {
    return strdup_cstr(getOsInfoJson()); // Calls correct implementation
}

// ✅ Get RAM Info
__attribute__((visibility("default"))) char* ramInfo() {
    return strdup_cstr(getRAMInfoJSON()); // Calls correct implementation
}

// ✅ Get Installed Applications
__attribute__((visibility("default"))) char* installedApplications() {
    return strdup_cstr(getInstalledApplicationsJSON()); // Calls correct implementation
}

// ✅ Get Running Processes
__attribute__((visibility("default"))) char* runningProcesses() {
    return strdup_cstr(getRunningProcessesJSON()); // Calls correct implementation
}

// ✅ Free allocated memory for FFI
__attribute__((visibility("default"))) void free_cstr(char* ptr) {
    if (ptr) {
        free(ptr);
    }
}

}