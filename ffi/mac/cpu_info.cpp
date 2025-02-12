#include "include/cpu_info.h"
#include <iostream>
#include <sys/sysctl.h>
#include <mach/mach.h>
#include <unistd.h>
#include <vector>
#include <iomanip>
#include <sstream>
#include <ctime>
#include <cstdlib>
#include <cstring>
#include "include/strdup_cstr.h"

using namespace std;



// Helper: Get a sysctl value (unsigned 64-bit)
static uint64_t getSysctlValue(const char* name) {
    uint64_t value = 0;
    size_t size = sizeof(value);
    if (sysctlbyname(name, &value, &size, NULL, 0) == 0) {
        return value;
    }
    return 0; // Return 0 if unavailable
}

// Get CPU Model (processorName)
static string getCPUModel() {
    char model[256];
    size_t size = sizeof(model);
    if (sysctlbyname("machdep.cpu.brand_string", model, &size, NULL, 0) == 0) {
        return string(model);
    }
    return "Unknown";
}

// Get CPU Architecture (returns a descriptive string)
static string getCPUArchitecture() {
    int isARM64 = 0;
    size_t size = sizeof(isARM64);
    sysctlbyname("hw.optional.arm64", &isARM64, &size, NULL, 0);
    return isARM64 ? "ARM64 (Apple Silicon)" : "x86_64 (Intel)";
}

// Get simplified architecture (for JSON output)
static string getSimpleArchitecture() {
    string archFull = getCPUArchitecture();
    if (archFull.find("ARM") != string::npos) {
        return "ARM64";
    }
    return "x86_64";
}

// Get logical core count (threadCount) using "hw.logicalcpu"
static int getCPUCoreCount() {
    uint64_t cores = getSysctlValue("hw.logicalcpu");
    return static_cast<int>(cores);
}

// Get physical core count using "hw.physicalcpu"
// If unavailable, fall back to logical core count.
static int getPhysicalCoreCount() {
    uint64_t cores = getSysctlValue("hw.physicalcpu");
    return (cores > 0) ? static_cast<int>(cores) : getCPUCoreCount();
}

// Get Base Clock Speed in GHz.
// On Apple Silicon these keys return 0 so we now return 0.0 to indicate data is not available.
static double getBaseClockSpeed() {
    uint64_t freq = 0;
    size_t size = sizeof(freq);
    if (sysctlbyname("hw.cpufrequency_max", &freq, &size, NULL, 0) == 0 && freq > 0) {
        return static_cast<double>(freq) / 1e9; // Hz to GHz
    }
    if (sysctlbyname("hw.cpufrequency", &freq, &size, NULL, 0) == 0 && freq > 0) {
        return static_cast<double>(freq) / 1e9; // Hz to GHz
    }
    // On Apple M1, these values are not exposed—return 0.0.
    return 0.0;
}

// For current clock speed we use the base value as a fallback.
static double getCurrentClockSpeed() {
    return getBaseClockSpeed();
}

// Temperature: Not available via sysctl – return 0.0
static double getCPUTemperature() {
    return 0.0;
}

// getCPUUsage() is provided (do not change this function)
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

// Get Cache Sizes (in bytes) using sysctl keys
static void getCPUCacheSizes(uint64_t& L1, uint64_t& L2, uint64_t& L3) {
    L1 = getSysctlValue("hw.l1dcachesize");
    L2 = getSysctlValue("hw.l2cachesize");
    L3 = getSysctlValue("hw.l3cachesize"); // Often 0 on Apple M1
}

// Get CPU Vendor from "machdep.cpu.vendor"
static string getCPUVendor() {
    char vendor[256] = {0};
    size_t size = sizeof(vendor);
    if (sysctlbyname("machdep.cpu.vendor", vendor, &size, NULL, 0) == 0) {
        return string(vendor);
    }
    if (getSimpleArchitecture() == "ARM64") {
        return "Apple";
    }
    return "Unknown";
}

// For instruction set, we simply use the simplified architecture.
static string getInstructionSet() {
    return getSimpleArchitecture();
}



// Function to return CPU information in JSON format (matching your Dart model)
extern "C" __attribute__((visibility("default"))) char* getJsonCpuData() {
    stringstream jsonStream;

    // Gather information
    string cpuModel = getCPUModel();
    int logicalCores = getCPUCoreCount();
    int physicalCores = getPhysicalCoreCount();
    double baseClock = getBaseClockSpeed();
    double currentClock = getCurrentClockSpeed();
    double temperature = getCPUTemperature();
    double usage = getCPUUsage();
    string architecture = getSimpleArchitecture();

    uint64_t L1, L2, L3;
    getCPUCacheSizes(L1, L2, L3);
    int l1CacheKB = static_cast<int>(L1 / 1024);
    int l2CacheKB = static_cast<int>(L2 / 1024);
    int l3CacheKB = static_cast<int>(L3 / 1024);

    string vendor = getCPUVendor();
    string instructionSet = getInstructionSet();

    // Build JSON object with keys matching your CpuInfo model
    jsonStream << "{\n";
    jsonStream << "  \"processorName\": \"" << cpuModel << "\",\n";
    jsonStream << "  \"coreCount\": " << physicalCores << ",\n";
    jsonStream << "  \"threadCount\": " << logicalCores << ",\n";
    jsonStream << "  \"baseClockSpeed\": " << fixed << setprecision(2) << baseClock << ",\n";
    jsonStream << "  \"currentClockSpeed\": " << fixed << setprecision(2) << currentClock << ",\n";
    jsonStream << "  \"temperature\": " << fixed << setprecision(2) << temperature << ",\n";
    jsonStream << "  \"usagePercentage\": " << fixed << setprecision(2) << usage << ",\n";
    jsonStream << "  \"architecture\": \"" << architecture << "\",\n";
    jsonStream << "  \"l1CacheSize\": " << l1CacheKB << ",\n";
    jsonStream << "  \"l2CacheSize\": " << l2CacheKB << ",\n";
    jsonStream << "  \"l3CacheSize\": " << l3CacheKB << ",\n";
    jsonStream << "  \"vendor\": \"" << vendor << "\",\n";
    jsonStream << "  \"instructionSet\": \"" << instructionSet << "\"\n";
    jsonStream << "}";
    
    return strdup_cstr(jsonStream.str());
}

// Expose real-time CPU usage function
extern "C" __attribute__((visibility("default"))) double getCPUUsageExtern() {
    return getCPUUsage();
}
