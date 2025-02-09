#include "../include/battery_info.h"
#include "../include/cpu_info.h"
#include "../include/disk_info.h"
#include "../include/gpu_info.h"
#include "../include/os_info.h"
#include "../include/ram_info.h"
#include "../include/running_app_info.h"

#include <string>
#include <cstring> // For strdup

extern "C" {

// Helper function to return a duplicated C-string
char* strdup_cstr(const std::string& str) {
    char* cstr = (char*)malloc(str.size() + 1);
    if (cstr) {
        strcpy(cstr, str.c_str());
    }
    return cstr;
}

// Return battery info
char* getBatteryInfo() {
    return strdup_cstr(displayBatteryInfo());
}

// Return CPU data
char* getJsonCpuData() {
    return strdup_cstr(jsonCpuData());
}

// Return Disk usage
char* getJsonDiskUsage() {
    return strdup_cstr(getDiskUsage("/"));
}

// Return GPU info
char* getGpuInfo() {
    return strdup_cstr(extract_gpu_info());
}

// Return OS info
char* getJsonOsInfo() {
    return strdup_cstr(getOsInfoJson());
}

// Return RAM info
char* getJsonRamInfo() {
    return strdup_cstr(getRAMInfoJSON());
}

// Return Installed Applications
char* getJsonInstalledApp() {
    return strdup_cstr(getInstalledApplicationsJSON());
}

// Return Running Processes
char* getJsonRunningProcesses() {
    return strdup_cstr(getRunningProcessesJSON());
}

}








// #include "../include/battery_info.h"
// #include "../include/cpu_info.h"
// #include "../include/disk_info.h"
// #include "../include/gpu_info.h"
// #include "../include/os_info.h"
// #include "../include/ram_info.h"
// #include "../include/running_app_info.h"

// #include <string>

// static std::string batteryInfo;
// static std::string cpuData;
// static std::string diskUsage;
// static std::string gpuInfo;
// static std::string osInfo;
// static std::string ramInfo;
// static std::string installedApps;
// static std::string runningProcesses;

// extern "C" {

// // Return battery info
// const char* getBatteryInfo() {
//     batteryInfo = displayBatteryInfo();  
//     return batteryInfo.c_str();
// }

// // Return CPU data
// const char* getJsonCpuData() {
//     cpuData = jsonCpuData();
//     return cpuData.c_str();
// }

// // Return Disk usage
// const char* getJsonDiskUsage() {
//     diskUsage = getDiskUsage("/");  // Call with a valid disk path
//     return diskUsage.c_str();
// }

// // Return GPU info
// const char* getGpuInfo() {
//     gpuInfo = extract_gpu_info();
//     return gpuInfo.c_str();
// }

// // Return OS info
// const char* getJsonOsInfo() {
//     osInfo = getOsInfoJson();
//     return osInfo.c_str();
// }

// // Return RAM info
// const char* getJsonRamInfo() {
//     ramInfo = getRAMInfoJSON();
//     return ramInfo.c_str();
// }

// // Return Installed Applications
// const char* getJsonInstalledApp() {
//     installedApps = getInstalledApplicationsJSON();
//     return installedApps.c_str();
// }

// // Return Running Processes
// const char* getJsonRunningProcesses() {
//     runningProcesses = getRunningProcessesJSON();
//     return runningProcesses.c_str();
// }

// }



// #include "../include/battery_info.h"
// #include "../include/cpu_info.h"
// #include "../include/disk_info.h"
// #include "../include/gpu_info.h"
// #include "../include/os_info.h"
// #include "../include/ram_info.h"
// #include "../include/running_app_info.h"

// #include <string>

// using namespace std;

// // Static variables to hold string data for FFI compatibility
// static string batteryInfo;
// static string cpuData;
// static string diskUsage;
// static string gpuInfo;
// static string osInfo;
// static string ramInfo;
// static string installedApps;
// static string runningProcesses;

// extern "C" {

// // Return battery info
// const char* displayBatteryInfo() {
//     batteryInfo = getBatteryData();  
//     return batteryInfo.c_str();
// }

// // Return CPU data
// const char* jsonCpuData() {
//     cpuData = getCpuDataJson();
//     return cpuData.c_str();
// }

// // Return Disk usage
// const char* getDiskUsage() {
//     diskUsage = getDiskUsageJson();
//     return diskUsage.c_str();
// }

// // Return GPU info
// const char* extract_gpu_info() {
//     gpuInfo = getGPUDetailsJson();
//     return gpuInfo.c_str();
// }

// // Return OS info
// const char* getOsInfoJson() {
//     osInfo = getOsInfoJson();  // This function must return `std::string`
//     return osInfo.c_str();
// }

// // Return RAM info
// const char* getRAMInfoJSON() {
//     ramInfo = getRAMJson();
//     return ramInfo.c_str();
// }

// // Return Installed Applications
// const char* getInstalledApplicationsJSON() {
//     installedApps = getInstalledAppsJson();
//     return installedApps.c_str();
// }

// // Return Running Processes
// const char* getRunningProcessesJSON() {
//     runningProcesses = getRunningProcessesJson();
//     return runningProcesses.c_str();
// }
// }










// #include "../include/battery_info.h"
// #include "../include/cpu_info.h"
// #include "../include/disk_info.h"
// #include "../include/gpu_info.h"
// #include "../include/os_info.h"
// #include "../include/ram_info.h"
// #include "../include/running_app_info.h"


// extern "C" {
//     string displayBatteryInfo() {
//         return displayBatteryInfo();
//     }

//     string jsonCpuData() {
//         return jsonCpuData();
//     }

//     double getCPUUsage() {
//         return getCPUUsage();
//     }

//     string getDiskUsage() {
//         return getDiskUsage();
//     }

//     string getDiskSpeed() {
//         return getDiskSpeed();
//     }

//     string getDiskDetails() {
//         return getDiskDetails();
//     }

//     string extract_gpu_info() {
//         return extract_gpu_info();
//     }

//     double getGPUUsage() {
//         return getGPUUsage();
//     }

//     string getOsInfoJson() {
//         return getOsInfoJson();
//     }

//     string getRAMInfoJSON() {
//         return getRAMInfoJSON();
//     }

//     string getInstalledApplicationsJSON() {
//         return getInstalledApplicationsJSON();
//     }

//     string getRunningProcessesJSON() {
//         return getRunningProcessesJSON();
//     }
// }
