#include "include/cpu_info.h"
#include "include/common.h"
#include <windows.h>
#include <intrin.h>
#include <pdh.h>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <map>
#include <mutex>

// For tracking CPU usage
static PDH_HQUERY cpuQuery;
static PDH_HCOUNTER cpuTotal;
static bool cpuQueryInitialized = false;
static std::mutex cpuMutex;

// Initialize CPU performance counters
bool initCpuQuery() {
    std::lock_guard<std::mutex> lock(cpuMutex);
    
    if (cpuQueryInitialized) {
        return true;
    }
    
    // Create query
    if (PdhOpenQuery(NULL, 0, &cpuQuery) != ERROR_SUCCESS) {
        return false;
    }
    
    // Add counter
    if (PdhAddCounter(cpuQuery, L"\\Processor(_Total)\\% Processor Time", 0, &cpuTotal) != ERROR_SUCCESS) {
        PdhCloseQuery(cpuQuery);
        return false;
    }
    
    // Initialize query
    PdhCollectQueryData(cpuQuery);
    
    cpuQueryInitialized = true;
    return true;
}

// Clean up CPU performance counters
void cleanupCpuQuery() {
    std::lock_guard<std::mutex> lock(cpuMutex);
    
    if (cpuQueryInitialized) {
        PdhCloseQuery(cpuQuery);
        cpuQueryInitialized = false;
    }
}

// Get current CPU usage as a percentage
double getCPUUsage() {
    std::lock_guard<std::mutex> lock(cpuMutex);
    
    if (!cpuQueryInitialized && !initCpuQuery()) {
        return 0.0;
    }
    
    PDH_FMT_COUNTERVALUE counterVal;
    
    // Collect data
    PdhCollectQueryData(cpuQuery);
    
    // Sleep to get a difference in the counter
    Sleep(100);
    
    // Collect data again
    PdhCollectQueryData(cpuQuery);
    
    // Format the result
    if (PdhGetFormattedCounterValue(cpuTotal, PDH_FMT_DOUBLE, NULL, &counterVal) != ERROR_SUCCESS) {
        return 0.0;
    }
    
    return counterVal.doubleValue;
}

// Get detailed CPU information
CPUInfo getCPUDetails() {
    CPUInfo info;
    
    // Get processor info
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    
    // Get number of cores
    info.physicalCores = sysInfo.dwNumberOfProcessors;
    info.logicalCores = 0;  // Will be populated later
    
    // Get processor name from registry
    HKEY hKey;
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        WCHAR processorName[256];
        DWORD bufSize = sizeof(processorName);
        
        if (RegQueryValueExW(hKey, L"ProcessorNameString", nullptr, nullptr, (LPBYTE)processorName, &bufSize) == ERROR_SUCCESS) {
            info.name = wideToUtf8(processorName);
        }
        
        RegCloseKey(hKey);
    }
    
    // Get CPU vendor and features using CPUID
    int cpuInfo[4] = {0};
    
    // Get vendor
    __cpuid(cpuInfo, 0);
    char vendor[13];
    *((int*)vendor) = cpuInfo[1];     // EBX
    *((int*)(vendor + 4)) = cpuInfo[3]; // EDX
    *((int*)(vendor + 8)) = cpuInfo[2]; // ECX
    vendor[12] = '\0';
    info.vendor = vendor;
    
    // Get logical core count and features
    if (cpuInfo[0] >= 1) {
        __cpuid(cpuInfo, 1);
        
        // EBX bits 23-16 contain logical processors per physical core
        info.logicalCores = ((cpuInfo[1] >> 16) & 0xFF);
        
        // Check if HyperThreading is supported (EDX bit 28)
        info.hyperThreading = (cpuInfo[3] & (1 << 28)) != 0;
        
        // Check for SSE, SSE2, SSE3, SSSE3, SSE4.1, SSE4.2, AVX features
        info.features.clear();
        
        if (cpuInfo[3] & (1 << 25)) info.features.push_back("SSE");
        if (cpuInfo[3] & (1 << 26)) info.features.push_back("SSE2");
        if (cpuInfo[2] & (1 << 0))  info.features.push_back("SSE3");
        if (cpuInfo[2] & (1 << 9))  info.features.push_back("SSSE3");
        if (cpuInfo[2] & (1 << 19)) info.features.push_back("SSE4.1");
        if (cpuInfo[2] & (1 << 20)) info.features.push_back("SSE4.2");
        if (cpuInfo[2] & (1 << 28)) info.features.push_back("AVX");
        
        // Check for virtualization
        if (cpuInfo[2] & (1 << 5)) info.features.push_back("Virtualization");
    }
    
    // Check for AVX2 and AVX-512
    if (cpuInfo[0] >= 7) {
        __cpuid(cpuInfo, 7);
        
        if (cpuInfo[1] & (1 << 5))  info.features.push_back("AVX2");
        if (cpuInfo[1] & (1 << 16)) info.features.push_back("AVX-512");
    }
    
    // Get CPU clock speed
    DWORD dataSize = sizeof(DWORD);
    DWORD mhz = 0;
    
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        if (RegQueryValueExW(hKey, L"~MHz", nullptr, nullptr, (LPBYTE)&mhz, &dataSize) == ERROR_SUCCESS) {
            info.clockSpeed = mhz;
        }
        
        RegCloseKey(hKey);
    }
    
    // Get cache sizes using CPUID
    // L1 Data cache
    __cpuid(cpuInfo, 0x80000005);
    info.l1CacheSize = ((cpuInfo[2] >> 24) & 0xFF) * 1024;  // L1 data cache size in KB
    
    // L2 and L3 caches
    __cpuid(cpuInfo, 0x80000006);
    info.l2CacheSize = ((cpuInfo[2] >> 16) & 0xFFFF) * 1024;  // L2 cache size in KB
    info.l3CacheSize = ((cpuInfo[3] >> 18) & 0x3FFF) * 512 * 1024;  // L3 cache size in KB
    
    // Get additional CPU details using WMI
    std::string wmiData = execCommand("wmic cpu get MaxClockSpeed,L2CacheSize,L3CacheSize,Architecture,DataWidth,SocketDesignation /format:list");
    
    // Parse WMI output
    std::stringstream ss(wmiData);
    std::string line;
    
    while (std::getline(ss, line)) {
        // Trim whitespace
        size_t start = line.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) continue;
        
        size_t end = line.find_last_not_of(" \t\r\n");
        if (end == std::string::npos) continue;
        
        line = line.substr(start, end - start + 1);
        
        // Parse key-value pairs
        size_t delimiter = line.find('=');
        if (delimiter == std::string::npos) continue;
        
        std::string key = line.substr(0, delimiter);
        std::string value = line.substr(delimiter + 1);
        
        if (key == "MaxClockSpeed" && !value.empty()) {
            try {
                info.clockSpeed = std::stoi(value);
            } catch (...) {}
        } else if (key == "SocketDesignation" && !value.empty()) {
            info.socket = value;
        } else if (key == "DataWidth" && !value.empty()) {
            try {
                info.architecture = std::stoi(value);
            } catch (...) {}
        } else if (key == "L2CacheSize" && !value.empty() && info.l2CacheSize == 0) {
            try {
                info.l2CacheSize = std::stoi(value) * 1024;  // Convert KB to bytes
            } catch (...) {}
        } else if (key == "L3CacheSize" && !value.empty() && info.l3CacheSize == 0) {
            try {
                info.l3CacheSize = std::stoi(value) * 1024;  // Convert KB to bytes
            } catch (...) {}
        }
    }
    
    return info;
}

// Get CPU information as JSON string
char* getCpuInfoJson() {
    CPUInfo info = getCPUDetails();
    
    // Convert features vector to JSON array string
    std::string featuresStr = "[";
    for (size_t i = 0; i < info.features.size(); ++i) {
        if (i > 0) featuresStr += ",";
        featuresStr += "\"" + info.features[i] + "\"";
    }
    featuresStr += "]";
    
    // Format the data for display
    std::stringstream ss;
    ss << "{";
    ss << "\"name\":\"" << info.name << "\",";
    ss << "\"vendor\":\"" << info.vendor << "\",";
    ss << "\"socket\":\"" << info.socket << "\",";
    ss << "\"clockSpeed\":" << info.clockSpeed << ",";
    ss << "\"physicalCores\":" << info.physicalCores << ",";
    ss << "\"logicalCores\":" << info.logicalCores << ",";
    ss << "\"hyperThreading\":" << (info.hyperThreading ? "true" : "false") << ",";
    ss << "\"architecture\":" << info.architecture << ",";
    ss << "\"l1CacheSize\":" << info.l1CacheSize << ",";
    ss << "\"l2CacheSize\":" << info.l2CacheSize << ",";
    ss << "\"l3CacheSize\":" << info.l3CacheSize << ",";
    ss << "\"features\":" << featuresStr;
    ss << "}";
    
    return strdup_cstr(ss.str());
}