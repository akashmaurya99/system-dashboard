#include "include/cpu_info.h"
#include <iostream>
#include <sys/sysctl.h>
#include <mach/mach.h>
#include <unistd.h>
#include <vector>
#include <iomanip>
#include <sstream>
#include <ctime>
#include "include/strdup_cstr.h"



using namespace std;


// Function to get sysctl integer values
static uint64_t getSysctlValue(const char* name) {
    uint64_t value = 0;
    size_t size = sizeof(value);
    if (sysctlbyname(name, &value, &size, NULL, 0) == 0) {
        return value;
    }
    return 0; // Return 0 if unavailable
}

// Function to get CPU Model
static string getCPUModel() {
    char model[256];
    size_t size = sizeof(model);
    if (sysctlbyname("machdep.cpu.brand_string", model, &size, NULL, 0) == 0) {
        return string(model);
    }
    return "Unknown";
}

// Function to detect Apple Silicon
static string getCPUArchitecture() {
    int isARM64 = 0;
    size_t size = sizeof(isARM64);
    sysctlbyname("hw.optional.arm64", &isARM64, &size, NULL, 0);
    return isARM64 ? "ARM64 (Apple Silicon)" : "x86_64 (Intel)";
}

// Function to get CPU core count
static int getCPUCoreCount() {
    return static_cast<int>(getSysctlValue("hw.logicalcpu"));
}

// Function to get L1, L2, and L3 cache sizes
static void getCPUCacheSizes(uint64_t& L1, uint64_t& L2, uint64_t& L3) {
    L1 = getSysctlValue("hw.l1dcachesize");  // Data cache
    L2 = getSysctlValue("hw.l2cachesize");
    
    vector<uint64_t> cacheConfig(10, 0);
    size_t size = cacheConfig.size() * sizeof(uint64_t);
    if (sysctlbyname("hw.cacheconfig", cacheConfig.data(), &size, NULL, 0) == 0) {
        L3 = cacheConfig.size() > 2 ? cacheConfig[2] * 1024 : 0;
    } else {
        L3 = 0; // No L3 reported
    }
}

// Function to get real-time CPU usage
 double getCPUUsage() {
    static uint64_t prevUser = 0, prevSystem = 0, prevIdle = 0, prevNice = 0;

    mach_msg_type_number_t count = HOST_CPU_LOAD_INFO_COUNT;
    host_cpu_load_info_data_t cpuInfo;
    kern_return_t result = host_statistics(mach_host_self(), HOST_CPU_LOAD_INFO, (host_info_t)&cpuInfo, &count);

    if (result != KERN_SUCCESS) {
        return -1.0;
    }

    uint64_t user = cpuInfo.cpu_ticks[CPU_STATE_USER];
    uint64_t system = cpuInfo.cpu_ticks[CPU_STATE_SYSTEM];
    uint64_t idle = cpuInfo.cpu_ticks[CPU_STATE_IDLE];
    uint64_t nice = cpuInfo.cpu_ticks[CPU_STATE_NICE];

    uint64_t totalDiff = (user - prevUser) + (system - prevSystem) + (nice - prevNice);
    uint64_t total = totalDiff + (idle - prevIdle);

    prevUser = user;
    prevSystem = system;
    prevIdle = idle;
    prevNice = nice;

    return (total > 0) ? (100.0 * static_cast<double>(totalDiff) / static_cast<double>(total)) : 0.0;
}

// Function to return CPU information in JSON format
extern "C" __attribute__((visibility("default"))) char* getJsonCpuData() {
    stringstream jsonStream;
    
    jsonStream << "{\n";
    jsonStream << "  \"CPU Model\": \"" << getCPUModel() << "\",\n";
    jsonStream << "  \"CPU Architecture\": \"" << getCPUArchitecture() << "\",\n";
    jsonStream << "  \"CPU Cores\": " << getCPUCoreCount() << ",\n";
    jsonStream << "  \"Base Clock Speed\": \"N/A (Apple Silicon does not expose this)\",\n";

    uint64_t L1, L2, L3;
    getCPUCacheSizes(L1, L2, L3);

    jsonStream << "  \"L1 Cache\": " << L1 / 1024 << " KiB,\n";
    jsonStream << "  \"L2 Cache\": " << L2 / (1024 * 1024) << " MiB,\n";
    jsonStream << "  \"L3 Cache\": " << (L3 ? L3 / (1024 * 1024) : 0) << " MiB\n";
    jsonStream << "}";

    return strdup_cstr(jsonStream.str());
}


// Expose real-time CPU usage function
extern "C" __attribute__((visibility("default"))) double getCPUUsageExtern() {
    return getCPUUsage();
}

// // Free allocated memory
// extern "C" __attribute__((visibility("default"))) void free_cstr(char* ptr) {
//     if (ptr) {
//         free(ptr);
//     }
// }