#include "../include/cpu_info.h"
#include <iostream>
#include <sys/sysctl.h>
#include <mach/mach.h>
#include <unistd.h>
#include <vector>
#include <iomanip>
#include <sstream>
#include <ctime>


using namespace std;

// Function to get sysctl integer values
uint64_t getSysctlValue(const char* name) {
    uint64_t value = 0;
    size_t size = sizeof(value);
    if (sysctlbyname(name, &value, &size, NULL, 0) == 0) {
        return value;
    }
    return 0; // Return 0 if unavailable
}

// Function to get CPU Model
string getCPUModel() {
    char model[256];
    size_t size = sizeof(model);
    if (sysctlbyname("machdep.cpu.brand_string", model, &size, NULL, 0) == 0) {
        return string(model);
    }
    return "Unknown";
}

// Function to detect Apple Silicon
string getCPUArchitecture() {
    int isARM64 = 0;
    size_t size = sizeof(isARM64);
    sysctlbyname("hw.optional.arm64", &isARM64, &size, NULL, 0);
    return isARM64 ? "ARM64 (Apple Silicon)" : "x86_64 (Intel)";
}

// Function to get CPU core count
int getCPUCoreCount() {
    return static_cast<int>(getSysctlValue("hw.logicalcpu"));
}

// Function to get L1, L2, and L3 cache sizes
void getCPUCacheSizes(uint64_t& L1, uint64_t& L2, uint64_t& L3) {
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

// cpu related data in json formate

string jsonCpuData() {
    stringstream json;
    json << "{\n";
    json << "  \"CPU Model\": \"" << getCPUModel() << "\",\n";
    json << "  \"CPU Architecture\": \"" << getCPUArchitecture() << "\",\n";
    json << "  \"CPU Cores\": " << getCPUCoreCount() << ",\n";
    json << "  \"Base Clock Speed\": \"N/A (Apple Silicon does not expose this)\",\n";
    
    uint64_t L1, L2, L3;
    getCPUCacheSizes(L1, L2, L3);
    json << "  \"L1 Cache\": " << L1 / 1024 << ",\n";
    json << "  \"L2 Cache\": " << L2 / (1024 * 1024) << ",\n";
    json << "  \"L3 Cache\": " << (L3 ? L3 / (1024 * 1024) : 0) << ",\n";
    json << "\n}\n";
    
    return json.str();
}


// void jsonCpuUtilizationData(){

//     cout << "{\n";
    
//     cout << "  \"Real-time CPU Usage\": [\n";
    
//     bool first_entry = true;
//     while (true) {
//         double usage = getCPUUsage();
//         if (usage >= 0) {
//             if (!first_entry) cout << ",\n";
//             first_entry = false;
//             cout << "    { \"Time\": " << time(nullptr) << ", \"CPU Usage\": " << fixed << setprecision(2) << usage << " }";
//         }
//         usleep(200000); // Sleep for 200ms
//     }
    
//     cout << "\n  ]\n}";

// }

// int main(){
//    cout<< jsonCpuData();
//     jsonCpuUtilizationData();
//     return 0;
// }