#include "include/ram_info.h"
#include <iostream>
#include <sys/sysctl.h>
#include <mach/mach.h>
#include <cstdlib>
#include <sstream>
#include <cstdio>
#include <string>
#include <regex>
#include <cstring>
#include <cstdint>
#include <cmath>

using namespace std;

// Executes a shell command and returns its trimmed output.
static string execCommand(const char* cmd) {
    char buffer[256];
    string result;
    FILE* pipe = popen(cmd, "r");
    if (!pipe) return "Error";
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }
    pclose(pipe);
    if (!result.empty()) {
        result.erase(result.find_last_not_of(" \n\r\t") + 1);
    }
    return result;
}

// Get total RAM size (in GB) using sysctl.
static double getTotalRAMSize() {
    int64_t ramSize;
    size_t size = sizeof(ramSize);
    if (sysctlbyname("hw.memsize", &ramSize, &size, NULL, 0) == 0) {
        return static_cast<double>(ramSize) / (1024 * 1024 * 1024); // bytes to GB
    }
    return -1; // Error case
}

// Get used RAM (in MB) using Mach VM statistics.
static double getUsedRAM_MB() {
    mach_msg_type_number_t count = HOST_VM_INFO64_COUNT;
    vm_statistics64_data_t vmStats;
    mach_port_t hostPort = mach_host_self();

    if (host_statistics64(hostPort, HOST_VM_INFO64, (host_info_t)&vmStats, &count) == KERN_SUCCESS) {
        double totalMB = getTotalRAMSize() * 1024; // GB to MB
        double freeMB = (vmStats.free_count + vmStats.inactive_count) * vm_page_size / (1024.0 * 1024.0);
        return totalMB - freeMB;
    }
    return -1; // Error case
}

// Get free RAM (in MB) using Mach VM statistics.
static double getFreeRAM_MB() {
    mach_msg_type_number_t count = HOST_VM_INFO64_COUNT;
    vm_statistics64_data_t vmStats;
    mach_port_t hostPort = mach_host_self();

    if (host_statistics64(hostPort, HOST_VM_INFO64, (host_info_t)&vmStats, &count) == KERN_SUCCESS) {
        return (vmStats.free_count + vmStats.inactive_count) * vm_page_size / (1024.0 * 1024.0);
    }
    return -1; // Error case
}

// Get Swap Total (in GB) by parsing the output of "sysctl vm.swapusage".
static double getSwapTotal() {
    string swapInfo = execCommand("sysctl vm.swapusage");
    // Expected format: "vm.swapusage: total = 1024.00M  used = 256.00M  free = 768.00M (encrypted)"
    regex swapRegex("total = ([0-9.]+)M");
    smatch match;
    if (regex_search(swapInfo, match, swapRegex)) {
        try {
            double totalMB = stod(match[1].str());
            return totalMB / 1024.0; // MB to GB
        } catch (...) { }
    }
    return 0;
}

// Get Swap Used (in GB) by parsing the output of "sysctl vm.swapusage".
static double getSwapUsed() {
    string swapInfo = execCommand("sysctl vm.swapusage");
    regex swapUsedRegex("used = ([0-9.]+)M");
    smatch match;
    if (regex_search(swapInfo, match, swapUsedRegex)) {
        try {
            double usedMB = stod(match[1].str());
            return usedMB / 1024.0; // MB to GB
        } catch (...) { }
    }
    return 0;
}

// Retrieve complete memory details by calling system_profiler.
static string getMemoryDetails() {
    return execCommand("system_profiler SPMemoryDataType");
}

// Get Memory Speed (in MHz).
static int getMemorySpeed() {
    // Try sysctl "hw.memfrequency" first.
    uint64_t memFreq = 0;
    size_t size = sizeof(memFreq);
    if (sysctlbyname("hw.memfrequency", &memFreq, &size, NULL, 0) == 0 && memFreq > 0) {
        int speed = static_cast<int>(memFreq / 1000000); // Hz to MHz
        if(speed > 0) return speed;
    }
    // Fallback: parse system_profiler output with case-insensitive regex.
    string memDetails = getMemoryDetails();
    regex speedRegex("Speed:\\s*([0-9]+)\\s*MHz", regex_constants::icase);
    smatch match;
    if (regex_search(memDetails, match, speedRegex)) {
        try {
            return stoi(match[1].str());
        } catch (...) { }
    }
    // Additional fallback: "Configured Memory Speed:" if available.
    regex confSpeedRegex("Configured Memory Speed:\\s*([0-9]+)\\s*MHz", regex_constants::icase);
    if (regex_search(memDetails, match, confSpeedRegex)) {
        try {
            return stoi(match[1].str());
        } catch (...) { }
    }
    return 0;
}

// Get Memory Type (e.g., DDR4, LPDDR4, LPDDR4X) from system_profiler output.
static string getMemoryType(const string &memDetails) {
    // Allow variations such as LPDDR4, LPDDR4X, DDR4, etc.
    regex typeRegex("Type:\\s*([A-Z]+DDR[0-9A-Z]*)", regex_constants::icase);
    smatch match;
    if (regex_search(memDetails, match, typeRegex)) {
        return match[1].str();
    }
    return "Unknown";
}

// Get Module Count by counting "BANK" lines; if none found, assume 1 (soldered memory).
static int getModuleCount(const string &memDetails) {
    regex bankRegex("BANK\\s+\\d+/DIMM\\d+:");
    auto words_begin = sregex_iterator(memDetails.begin(), memDetails.end(), bankRegex);
    auto words_end = sregex_iterator();
    int count = distance(words_begin, words_end);
    if (count == 0 && getTotalRAMSize() > 0) {
        return 1;
    }
    return count;
}

// Get CAS Latency (CL value) from system_profiler output.
static int getCasLatency(const string &memDetails) {
    regex casRegex("CAS Latency:\\s*(\\d+)", regex_constants::icase);
    smatch match;
    if (regex_search(memDetails, match, casRegex)) {
        try {
            return stoi(match[1].str());
        } catch (...) { }
    }
    // Fallback: try to find "CL" pattern.
    regex clRegex("CL\\s*(\\d+)", regex_constants::icase);
    if (regex_search(memDetails, match, clRegex)) {
        try {
            return stoi(match[1].str());
        } catch (...) { }
    }
    return 0;
}

// Compute memory usage percentage (used / total * 100).
static double getMemoryUsagePercentage(double totalGB, double usedGB) {
    if (totalGB > 0) {
        return (usedGB / totalGB) * 100.0;
    }
    return 0;
}

// Format all the RAM information as a JSON string.
static string generateRAMInfoJSON() {
    double totalMemory = getTotalRAMSize();                   // in GB
    double usedMemory = getUsedRAM_MB() / 1024.0;               // MB -> GB
    double freeMemory = getFreeRAM_MB() / 1024.0;               // MB -> GB
    double swapTotal = getSwapTotal();                          // in GB
    double swapUsed = getSwapUsed();                            // in GB
    double memoryUsagePercentage = getMemoryUsagePercentage(totalMemory, usedMemory);
    
    // Retrieve memory details once.
    string memDetails = getMemoryDetails();
    string memoryType = getMemoryType(memDetails);
    
    // Get memory speed from available sources.
    int memorySpeed = getMemorySpeed();
    // Fallback: if memorySpeed is still 0, use a default based on memory type.
    if (memorySpeed == 0) {
        if (memoryType.find("LPDDR4") != string::npos || memoryType.find("LPDDR4X") != string::npos) {
            memorySpeed = 4266;  // Default typical value for LPDDR4/LPDDR4X on Apple silicon
        } else if (memoryType.find("DDR4") != string::npos) {
            memorySpeed = 2400;  // Example default for DDR4
        }
    }
    
    int moduleCount = getModuleCount(memDetails);
    
    int casLatency = getCasLatency(memDetails);
    // Fallback: if CAS latency is still 0, use a default based on memory type.
    if (casLatency == 0) {
        if (memoryType.find("LPDDR4") != string::npos || memoryType.find("LPDDR4X") != string::npos) {
            casLatency = 19;  // Default typical value for LPDDR4/LPDDR4X
        } else if (memoryType.find("DDR4") != string::npos) {
            casLatency = 16;  // Example default for DDR4
        }
    }
    
    ostringstream json;
    json << "{\n";
    json << "  \"totalMemory\": " << totalMemory << ",\n";
    json << "  \"usedMemory\": " << usedMemory << ",\n";
    json << "  \"freeMemory\": " << freeMemory << ",\n";
    json << "  \"swapTotal\": " << swapTotal << ",\n";
    json << "  \"swapUsed\": " << swapUsed << ",\n";
    json << "  \"memoryUsagePercentage\": " << memoryUsagePercentage << ",\n";
    json << "  \"memorySpeed\": " << memorySpeed << ",\n";
    json << "  \"memoryType\": \"" << memoryType << "\",\n";
    json << "  \"moduleCount\": " << moduleCount << ",\n";
    json << "  \"casLatency\": " << casLatency << "\n";
    json << "}";
    return json.str();
}

// FFI-Compatible Wrapper: Returns a malloc'ed C-string which must be freed by the caller.
extern "C" __attribute__((visibility("default"))) char* getRAMInfoJSON() {
    string result = generateRAMInfoJSON();
    char* cstr = (char*)malloc(result.size() + 1);
    if (cstr) {
        strcpy(cstr, result.c_str());
    }
    return cstr;
}

// // Free allocated memory
// extern "C" __attribute__((visibility("default"))) void free_cstr(char* ptr) {
//     if (ptr) {
//         free(ptr);
//     }
// }