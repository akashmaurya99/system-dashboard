#include "include/ram_info.h"
#include <windows.h>
#include <sysinfoapi.h>
#include <psapi.h>
#include <sstream>
#include <iomanip>

// Get memory type as string
std::string getMemoryTypeString(int type) {
    switch (type) {
        case 0: return "Unknown";
        case 1: return "Other";
        case 2: return "DRAM";
        case 3: return "Synchronous DRAM";
        case 4: return "Cache DRAM";
        case 5: return "EDO";
        case 6: return "EDRAM";
        case 7: return "VRAM";
        case 8: return "SRAM";
        case 9: return "RAM";
        case 10: return "ROM";
        case 11: return "Flash";
        case 12: return "EEPROM";
        case 13: return "FEPROM";
        case 14: return "EPROM";
        case 15: return "CDRAM";
        case 16: return "3DRAM";
        case 17: return "SDRAM";
        case 18: return "SGRAM";
        case 19: return "RDRAM";
        case 20: return "DDR";
        case 21: return "DDR2";
        case 22: return "DDR2 FB-DIMM";
        case 24: return "DDR3";
        case 26: return "DDR4";
        case 27: return "DDR5";
        case 28: return "LPDDR";
        case 29: return "LPDDR2";
        case 30: return "LPDDR3";
        case 31: return "LPDDR4";
        case 32: return "LPDDR5";
        default: return "Unknown";
    }
}

// Get form factor as string
std::string getFormFactorString(int formFactor) {
    switch (formFactor) {
        case 0: return "Unknown";
        case 1: return "Other";
        case 2: return "SIP";
        case 3: return "DIP";
        case 4: return "ZIP";
        case 5: return "SOJ";
        case 6: return "Proprietary";
        case 7: return "SIMM";
        case 8: return "DIMM";
        case 9: return "TSOP";
        case 10: return "PGA";
        case 11: return "RIMM";
        case 12: return "SODIMM";
        case 13: return "SRIMM";
        case 14: return "SMD";
        case 15: return "SSMP";
        case 16: return "QFP";
        case 17: return "TQFP";
        case 18: return "SOIC";
        case 19: return "LCC";
        case 20: return "PLCC";
        case 21: return "BGA";
        case 22: return "FPBGA";
        case 23: return "LGA";
        default: return "Unknown";
    }
}

// Get formatted size string
std::string formatBytes(uint64_t bytes) {
    const char* units[] = {"B", "KB", "MB", "GB", "TB", "PB"};
    int unitIndex = 0;
    double size = static_cast<double>(bytes);
    
    while (size >= 1024.0 && unitIndex < 5) {
        size /= 1024.0;
        unitIndex++;
    }
    
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << size << " " << units[unitIndex];
    return oss.str();
}

// Get detailed memory information
MemoryInfo getMemoryDetails() {
    MemoryInfo info = {};
    
    // Get basic memory information
    MEMORYSTATUSEX memStatus = {0};
    memStatus.dwLength = sizeof(memStatus);
    
    if (GlobalMemoryStatusEx(&memStatus)) {
        info.totalPhysical = memStatus.ullTotalPhys;
        info.availablePhysical = memStatus.ullAvailPhys;
        info.usedPhysical = info.totalPhysical - info.availablePhysical;
        info.usagePercentage = memStatus.dwMemoryLoad;
        
        info.totalVirtual = memStatus.ullTotalVirtual;
        info.availableVirtual = memStatus.ullAvailVirtual;
        info.usedVirtual = info.totalVirtual - info.availableVirtual;
        
        info.totalPageFile = memStatus.ullTotalPageFile;
        info.availablePageFile = memStatus.ullAvailPageFile;
    }
    
    // Get cache information
    PERFORMANCE_INFORMATION perfInfo = {0};
    perfInfo.cb = sizeof(perfInfo);
    
    if (GetPerformanceInfo(&perfInfo, sizeof(perfInfo))) {
        info.totalCache = perfInfo.SystemCache * perfInfo.PageSize;
    }
    
    // Get detailed memory module information from WMI
    IWbemServices* pSvc = NULL;
    if (initWMI(&pSvc)) {
        // Get physical memory array information
        auto arrayResults = executeWMIQuery(pSvc, L"SELECT * FROM Win32_PhysicalMemoryArray");
        
        if (!arrayResults.empty()) {
            VARIANT vtProp;
            VariantInit(&vtProp);
            
            // Get number of slots
            if (SUCCEEDED(arrayResults[0]->Get(L"MemoryDevices", 0, &vtProp, 0, 0))) {
                info.numberOfSlots = vtProp.intVal;
                VariantClear(&vtProp);
            } else {
                info.numberOfSlots = 0;
            }
            
            arrayResults[0]->Release();
        }
        
        // Get memory module information
        auto moduleResults = executeWMIQuery(pSvc, L"SELECT * FROM Win32_PhysicalMemory");
        
        for (auto& obj : moduleResults) {
            VARIANT vtProp;
            VariantInit(&vtProp);
            
            MemoryInfo::MemorySlot slot;
            slot.isPresent = true;
            
            // Get bank label
            if (SUCCEEDED(obj->Get(L"BankLabel", 0, &vtProp, 0, 0))) {
                slot.bankLabel = _bstr_t(vtProp.bstrVal);
                VariantClear(&vtProp);
            } else {
                slot.bankLabel = "Unknown";
            }
            
            // Get device locator
            if (SUCCEEDED(obj->Get(L"DeviceLocator", 0, &vtProp, 0, 0))) {
                slot.deviceLocator = _bstr_t(vtProp.bstrVal);
                VariantClear(&vtProp);
            } else {
                slot.deviceLocator = "Unknown";
            }
            
            // Get manufacturer
            if (SUCCEEDED(obj->Get(L"Manufacturer", 0, &vtProp, 0, 0))) {
                slot.manufacturer = _bstr_t(vtProp.bstrVal);
                VariantClear(&vtProp);
            } else {
                slot.manufacturer = "Unknown";
            }
            
            // Get part number
            if (SUCCEEDED(obj->Get(L"PartNumber", 0, &vtProp, 0, 0))) {
                slot.partNumber = _bstr_t(vtProp.bstrVal);
                VariantClear(&vtProp);
            } else {
                slot.partNumber = "Unknown";
            }
            
            // Get capacity
            if (SUCCEEDED(obj->Get(L"Capacity", 0, &vtProp, 0, 0))) {
                slot.capacity = _wtoi64(vtProp.bstrVal);
                VariantClear(&vtProp);
            } else {
                slot.capacity = 0;
            }
            
            // Get speed
            if (SUCCEEDED(obj->Get(L"Speed", 0, &vtProp, 0, 0))) {
                slot.speed = vtProp.intVal;
                if (info.memorySpeed < slot.speed) {
                    info.memorySpeed = slot.speed;
                }
                VariantClear(&vtProp);
            } else {
                slot.speed = 0;
            }
            
            // Get form factor
            if (SUCCEEDED(obj->Get(L"FormFactor", 0, &vtProp, 0, 0))) {
                slot.formFactor = getFormFactorString(vtProp.intVal);
                VariantClear(&vtProp);
            } else {
                slot.formFactor = "Unknown";
            }
            
            // Get memory type
            if (SUCCEEDED(obj->Get(L"MemoryType", 0, &vtProp, 0, 0))) {
                slot.memoryType = getMemoryTypeString(vtProp.intVal);
                if (info.memoryType.empty()) {
                    info.memoryType = slot.memoryType;
                }
                VariantClear(&vtProp);
            } else {
                slot.memoryType = "Unknown";
            }
            
            info.slots.push_back(slot);
            info.usedSlots++;
            
            obj->Release();
        }
        
        cleanupWMI(pSvc);
    }
    
    return info;
}

// Get RAM information as JSON string
char* getRAMInfo() {
    MemoryInfo info = getMemoryDetails();
    
    json memoryJson = {
        {"totalPhysical", info.totalPhysical},
        {"totalPhysicalFormatted", formatBytes(info.totalPhysical)},
        {"availablePhysical", info.availablePhysical},
        {"availablePhysicalFormatted", formatBytes(info.availablePhysical)},
        {"usedPhysical", info.usedPhysical},
        {"usedPhysicalFormatted", formatBytes(info.usedPhysical)},
        {"usagePercentage", info.usagePercentage},
        
        {"totalVirtual", info.totalVirtual},
        {"totalVirtualFormatted", formatBytes(info.totalVirtual)},
        {"availableVirtual", info.availableVirtual},
        {"availableVirtualFormatted", formatBytes(info.availableVirtual)},
        {"usedVirtual", info.usedVirtual},
        {"usedVirtualFormatted", formatBytes(info.usedVirtual)},
        
        {"totalPageFile", info.totalPageFile},
        {"totalPageFileFormatted", formatBytes(info.totalPageFile)},
        {"availablePageFile", info.availablePageFile},
        {"availablePageFileFormatted", formatBytes(info.availablePageFile)},
        
        {"totalCache", info.totalCache},
        {"totalCacheFormatted", formatBytes(info.totalCache)},
        
        {"memoryType", info.memoryType},
        {"memorySpeed", info.memorySpeed},
        {"numberOfSlots", info.numberOfSlots},
        {"usedSlots", info.usedSlots}
    };
    
    json slotsArray = json::array();
    
    for (const auto& slot : info.slots) {
        json slotJson = {
            {"bankLabel", slot.bankLabel},
            {"deviceLocator", slot.deviceLocator},
            {"manufacturer", slot.manufacturer},
            {"partNumber", slot.partNumber},
            {"capacity", slot.capacity},
            {"capacityFormatted", formatBytes(slot.capacity)},
            {"speed", slot.speed},
            {"formFactor", slot.formFactor},
            {"memoryType", slot.memoryType},
            {"isPresent", slot.isPresent}
        };
        
        slotsArray.push_back(slotJson);
    }
    
    memoryJson["slots"] = slotsArray;
    
    return strdup_cstr(memoryJson.dump());
}