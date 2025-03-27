#include "include/gpu_info.h"
#include "include/common.h"
#include <windows.h>
#include <setupapi.h>
#include <devguid.h>
#include <regstr.h>
#include <cfgmgr32.h>
#include <dxgi.h>
#include <d3d11.h>
#include <pdh.h>
#include <vector>
#include <string>
#include <sstream>
#include <mutex>

#pragma comment(lib, "setupapi.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")

// For tracking GPU usage
static PDH_HQUERY gpuQuery;
static std::vector<PDH_HCOUNTER> gpuCounters;
static bool gpuQueryInitialized = false;
static std::mutex gpuMutex;

// Initialize GPU performance counters
bool initGpuQuery() {
    std::lock_guard<std::mutex> lock(gpuMutex);
    
    if (gpuQueryInitialized) {
        return true;
    }
    
    // Create query
    if (PdhOpenQuery(NULL, 0, &gpuQuery) != ERROR_SUCCESS) {
        return false;
    }
    
    // Try to add counter for GPU usage
    PDH_HCOUNTER counter;
    if (PdhAddCounter(gpuQuery, L"\\GPU Engine(*)\\Utilization Percentage", 0, &counter) == ERROR_SUCCESS) {
        gpuCounters.push_back(counter);
    } else {
        // Try alternative counter
        if (PdhAddCounter(gpuQuery, L"\\GPU(*)\\% GPU Time", 0, &counter) == ERROR_SUCCESS) {
            gpuCounters.push_back(counter);
        }
    }
    
    if (gpuCounters.empty()) {
        PdhCloseQuery(gpuQuery);
        return false;
    }
    
    // Initialize query
    PdhCollectQueryData(gpuQuery);
    
    gpuQueryInitialized = true;
    return true;
}

// Clean up GPU performance counters
void cleanupGpuQuery() {
    std::lock_guard<std::mutex> lock(gpuMutex);
    
    if (gpuQueryInitialized) {
        PdhCloseQuery(gpuQuery);
        gpuCounters.clear();
        gpuQueryInitialized = false;
    }
}

// Get GPU usage as a percentage
double getGPUUsage() {
    std::lock_guard<std::mutex> lock(gpuMutex);
    
    if (!gpuQueryInitialized && !initGpuQuery()) {
        return 0.0;
    }
    
    if (gpuCounters.empty()) {
        return 0.0;
    }
    
    // Collect data
    PdhCollectQueryData(gpuQuery);
    
    // Sleep to get a difference in the counter
    Sleep(100);
    
    // Collect data again
    PdhCollectQueryData(gpuQuery);
    
    // Get the formatted values
    PDH_FMT_COUNTERVALUE counterVal;
    double totalUsage = 0.0;
    int validCounters = 0;
    
    for (const auto& counter : gpuCounters) {
        if (PdhGetFormattedCounterValue(counter, PDH_FMT_DOUBLE, NULL, &counterVal) == ERROR_SUCCESS) {
            totalUsage += counterVal.doubleValue;
            validCounters++;
        }
    }
    
    return validCounters > 0 ? (totalUsage / validCounters) : 0.0;
}

// Get detailed GPU information
std::vector<GPUInfo> getGPUDetails() {
    std::vector<GPUInfo> gpuList;
    
    // Try to get information using DXGI
    IDXGIFactory* factory = nullptr;
    if (SUCCEEDED(CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory))) {
        for (UINT i = 0; ; i++) {
            IDXGIAdapter* adapter = nullptr;
            HRESULT hr = factory->EnumAdapters(i, &adapter);
            
            if (hr == DXGI_ERROR_NOT_FOUND) {
                break;
            }
            
            if (SUCCEEDED(hr)) {
                DXGI_ADAPTER_DESC desc;
                if (SUCCEEDED(adapter->GetDesc(&desc))) {
                    GPUInfo gpu;
                    gpu.name = wideToUtf8(desc.Description);
                    gpu.vendorId = desc.VendorId;
                    gpu.deviceId = desc.DeviceId;
                    gpu.subsysId = desc.SubSysId;
                    gpu.revision = desc.Revision;
                    gpu.dedicatedVideoMemory = desc.DedicatedVideoMemory;
                    gpu.dedicatedSystemMemory = desc.DedicatedSystemMemory;
                    gpu.sharedSystemMemory = desc.SharedSystemMemory;
                    
                    // Determine vendor name from ID
                    switch (desc.VendorId) {
                        case 0x1002: gpu.vendor = "AMD"; break;
                        case 0x10DE: gpu.vendor = "NVIDIA"; break;
                        case 0x8086: gpu.vendor = "Intel"; break;
                        case 0x1414: gpu.vendor = "Microsoft"; break; // WARP
                        default: gpu.vendor = "Unknown"; break;
                    }
                    
                    gpuList.push_back(gpu);
                }
                adapter->Release();
            }
        }
        factory->Release();
    }
    
    // If DXGI failed, try to use SetupAPI
    if (gpuList.empty()) {
        HDEVINFO deviceInfoSet = SetupDiGetClassDevs(&GUID_DEVCLASS_DISPLAY, NULL, NULL, DIGCF_PRESENT);
        
        if (deviceInfoSet != INVALID_HANDLE_VALUE) {
            SP_DEVINFO_DATA deviceInfoData;
            deviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
            
            for (DWORD i = 0; SetupDiEnumDeviceInfo(deviceInfoSet, i, &deviceInfoData); i++) {
                WCHAR deviceName[256] = {0};
                WCHAR driverName[256] = {0};
                
                if (SetupDiGetDeviceRegistryPropertyW(deviceInfoSet, &deviceInfoData, SPDRP_DEVICEDESC, NULL,
                                                    (PBYTE)deviceName, sizeof(deviceName), NULL) &&
                    SetupDiGetDeviceRegistryPropertyW(deviceInfoSet, &deviceInfoData, SPDRP_DRIVER, NULL,
                                                    (PBYTE)driverName, sizeof(driverName), NULL)) {
                    
                    GPUInfo gpu;
                    gpu.name = wideToUtf8(deviceName);
                    gpu.driver = wideToUtf8(driverName);
                    
                    // Get hardware IDs
                    WCHAR hwIds[512] = {0};
                    if (SetupDiGetDeviceRegistryPropertyW(deviceInfoSet, &deviceInfoData, SPDRP_HARDWAREID, NULL,
                                                        (PBYTE)hwIds, sizeof(hwIds), NULL)) {
                        std::wstring hwIdStr(hwIds);
                        
                        // Extract vendor ID
                        size_t vendorPos = hwIdStr.find(L"VEN_");
                        if (vendorPos != std::wstring::npos) {
                            std::wstring vendorIdStr = hwIdStr.substr(vendorPos + 4, 4);
                            try {
                                gpu.vendorId = std::stoi(wideToUtf8(vendorIdStr), nullptr, 16);
                                
                                // Determine vendor name from ID
                                switch (gpu.vendorId) {
                                    case 0x1002: gpu.vendor = "AMD"; break;
                                    case 0x10DE: gpu.vendor = "NVIDIA"; break;
                                    case 0x8086: gpu.vendor = "Intel"; break;
                                    default: gpu.vendor = "Unknown"; break;
                                }
                            } catch (...) {
                                gpu.vendor = "Unknown";
                            }
                        }
                        
                        // Extract device ID
                        size_t devicePos = hwIdStr.find(L"DEV_");
                        if (devicePos != std::wstring::npos) {
                            std::wstring deviceIdStr = hwIdStr.substr(devicePos + 4, 4);
                            try {
                                gpu.deviceId = std::stoi(wideToUtf8(deviceIdStr), nullptr, 16);
                            } catch (...) {
                                gpu.deviceId = 0;
                            }
                        }
                    }
                    
                    // Get driver date and version
                    HKEY deviceKey = SetupDiOpenDevRegKey(deviceInfoSet, &deviceInfoData, DICS_FLAG_GLOBAL, 0, DIREG_DRV, KEY_QUERY_VALUE);
                    if (deviceKey != INVALID_HANDLE_VALUE) {
                        WCHAR driverVersion[50] = {0};
                        DWORD dataSize = sizeof(driverVersion);
                        
                        if (RegQueryValueExW(deviceKey, L"DriverVersion", nullptr, nullptr, (LPBYTE)driverVersion, &dataSize) == ERROR_SUCCESS) {
                            gpu.driverVersion = wideToUtf8(driverVersion);
                        }
                        
                        RegCloseKey(deviceKey);
                    }
                    
                    gpuList.push_back(gpu);
                }
            }
            
            SetupDiDestroyDeviceInfoList(deviceInfoSet);
        }
    }
    
    // Try to get additional information using WMI
    std::string wmiData = execCommand("wmic path win32_VideoController get AdapterCompatibility,AdapterRAM,CurrentRefreshRate,DriverDate,DriverVersion,VideoModeDescription,VideoProcessor /format:list");
    
    if (!wmiData.empty() && !gpuList.empty()) {
        // Parse WMI output
        std::stringstream ss(wmiData);
        std::string line;
        std::map<std::string, std::string> wmiInfo;
        size_t gpuIndex = 0;
        
        while (std::getline(ss, line)) {
            // Check for empty line - marks boundary between different GPUs
            if (line.find_first_not_of(" \t\r\n") == std::string::npos) {
                if (!wmiInfo.empty() && gpuIndex < gpuList.size()) {
                    // Update GPU info
                    if (wmiInfo.count("AdapterRAM") && gpuList[gpuIndex].dedicatedVideoMemory == 0) {
                        try {
                            gpuList[gpuIndex].dedicatedVideoMemory = std::stoull(wmiInfo["AdapterRAM"]);
                        } catch (...) {}
                    }
                    
                    if (wmiInfo.count("AdapterCompatibility") && gpuList[gpuIndex].vendor == "Unknown") {
                        gpuList[gpuIndex].vendor = wmiInfo["AdapterCompatibility"];
                    }
                    
                    if (wmiInfo.count("CurrentRefreshRate")) {
                        try {
                            gpuList[gpuIndex].refreshRate = std::stoi(wmiInfo["CurrentRefreshRate"]);
                        } catch (...) {}
                    }
                    
                    if (wmiInfo.count("DriverDate")) {
                        gpuList[gpuIndex].driverDate = wmiInfo["DriverDate"];
                    }
                    
                    if (wmiInfo.count("DriverVersion") && gpuList[gpuIndex].driverVersion.empty()) {
                        gpuList[gpuIndex].driverVersion = wmiInfo["DriverVersion"];
                    }
                    
                    if (wmiInfo.count("VideoModeDescription")) {
                        gpuList[gpuIndex].currentResolution = wmiInfo["VideoModeDescription"];
                    }
                    
                    if (wmiInfo.count("VideoProcessor")) {
                        gpuList[gpuIndex].processor = wmiInfo["VideoProcessor"];
                    }
                    
                    // Increment GPU index for next block
                    gpuIndex++;
                }
                
                // Reset for next GPU
                wmiInfo.clear();
                continue;
            }
            
            // Parse key-value pairs
            size_t delimiter = line.find('=');
            if (delimiter != std::string::npos) {
                std::string key = line.substr(0, delimiter);
                std::string value = line.substr(delimiter + 1);
                
                // Trim whitespace
                size_t keyStart = key.find_first_not_of(" \t\r\n");
                size_t keyEnd = key.find_last_not_of(" \t\r\n");
                if (keyStart != std::string::npos && keyEnd != std::string::npos) {
                    key = key.substr(keyStart, keyEnd - keyStart + 1);
                }
                
                size_t valueStart = value.find_first_not_of(" \t\r\n");
                size_t valueEnd = value.find_last_not_of(" \t\r\n");
                if (valueStart != std::string::npos && valueEnd != std::string::npos) {
                    value = value.substr(valueStart, valueEnd - valueStart + 1);
                }
                
                if (!key.empty() && !value.empty()) {
                    wmiInfo[key] = value;
                }
            }
        }
        
        // Process the last GPU block
        if (!wmiInfo.empty() && gpuIndex < gpuList.size()) {
            // Update GPU info (same as in the loop)
            if (wmiInfo.count("AdapterRAM") && gpuList[gpuIndex].dedicatedVideoMemory == 0) {
                try {
                    gpuList[gpuIndex].dedicatedVideoMemory = std::stoull(wmiInfo["AdapterRAM"]);
                } catch (...) {}
            }
            
            if (wmiInfo.count("AdapterCompatibility") && gpuList[gpuIndex].vendor == "Unknown") {
                gpuList[gpuIndex].vendor = wmiInfo["AdapterCompatibility"];
            }
            
            if (wmiInfo.count("CurrentRefreshRate")) {
                try {
                    gpuList[gpuIndex].refreshRate = std::stoi(wmiInfo["CurrentRefreshRate"]);
                } catch (...) {}
            }
            
            if (wmiInfo.count("DriverDate")) {
                gpuList[gpuIndex].driverDate = wmiInfo["DriverDate"];
            }
            
            if (wmiInfo.count("DriverVersion") && gpuList[gpuIndex].driverVersion.empty()) {
                gpuList[gpuIndex].driverVersion = wmiInfo["DriverVersion"];
            }
            
            if (wmiInfo.count("VideoModeDescription")) {
                gpuList[gpuIndex].currentResolution = wmiInfo["VideoModeDescription"];
            }
            
            if (wmiInfo.count("VideoProcessor")) {
                gpuList[gpuIndex].processor = wmiInfo["VideoProcessor"];
            }
        }
    }
    
    return gpuList;
}

// Get GPU information as JSON string
char* getGPUInfoJson() {
    std::vector<GPUInfo> gpuList = getGPUDetails();
    
    // Create JSON array of GPUs
    std::stringstream ss;
    ss << "[";
    
    for (size_t i = 0; i < gpuList.size(); ++i) {
        const auto& gpu = gpuList[i];
        
        if (i > 0) {
            ss << ",";
        }
        
        ss << "{";
        ss << "\"name\":\"" << gpu.name << "\",";
        ss << "\"vendor\":\"" << gpu.vendor << "\",";
        ss << "\"vendorId\":" << gpu.vendorId << ",";
        ss << "\"deviceId\":" << gpu.deviceId << ",";
        ss << "\"subsysId\":" << gpu.subsysId << ",";
        ss << "\"revision\":" << gpu.revision << ",";
        ss << "\"dedicatedVideoMemory\":\"" << formatBytes(gpu.dedicatedVideoMemory) << "\",";
        ss << "\"dedicatedSystemMemory\":\"" << formatBytes(gpu.dedicatedSystemMemory) << "\",";
        ss << "\"sharedSystemMemory\":\"" << formatBytes(gpu.sharedSystemMemory) << "\",";
        ss << "\"driver\":\"" << gpu.driver << "\",";
        ss << "\"driverVersion\":\"" << gpu.driverVersion << "\",";
        ss << "\"driverDate\":\"" << gpu.driverDate << "\",";
        ss << "\"processor\":\"" << gpu.processor << "\",";
        ss << "\"refreshRate\":" << gpu.refreshRate << ",";
        ss << "\"currentResolution\":\"" << gpu.currentResolution << "\"";
        ss << "}";
    }
    
    ss << "]";
    
    return strdup_cstr(ss.str());
}