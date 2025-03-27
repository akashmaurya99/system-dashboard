#include "include/fan_speed.h"
#include <map>

// Try vendor-specific implementations for Dell systems
bool getDellFanInfo(std::vector<FanData>& fans) {
    IWbemServices* pSvc = NULL;
    bool found = false;
    
    if (initWMI(&pSvc)) {
        auto results = executeWMIQuery(pSvc, L"SELECT * FROM Win32_PerfRawData_Counters_ThermalZoneInformation");
        
        for (auto& obj : results) {
            FanData fan;
            fan.name = "Dell System Fan";
            fan.location = "System";
            
            VARIANT vtProp;
            VariantInit(&vtProp);
            
            if (SUCCEEDED(obj->Get(L"PercentPassiveCooling", 0, &vtProp, 0, 0))) {
                fan.percentage = vtProp.intVal;
                VariantClear(&vtProp);
            } else {
                fan.percentage = 0;
            }
            
            // Dell often reports percentage directly; convert to RPM
            fan.currentSpeed = fan.percentage * 50; // Rough estimate: 5000 RPM max
            fan.maxSpeed = 5000;
            fan.isControlAvailable = false;
            
            fans.push_back(fan);
            found = true;
            
            obj->Release();
        }
        
        cleanupWMI(pSvc);
    }
    
    return found;
}

// Helper function for HP systems
bool getHPFanInfo(std::vector<FanData>& fans) {
    IWbemServices* pSvc = NULL;
    bool found = false;
    
    if (initWMI(&pSvc)) {
        // Try HP namespace if available
        auto results = executeWMIQuery(pSvc, L"SELECT * FROM Win32_Fan");
        
        for (auto& obj : results) {
            FanData fan;
            VARIANT vtProp;
            VariantInit(&vtProp);
            
            // Get fan name
            if (SUCCEEDED(obj->Get(L"Name", 0, &vtProp, 0, 0))) {
                fan.name = _bstr_t(vtProp.bstrVal);
                VariantClear(&vtProp);
            } else {
                fan.name = "HP System Fan";
            }
            
            fan.location = "System";
            
            // Get active status (0 or 1)
            if (SUCCEEDED(obj->Get(L"ActiveCooling", 0, &vtProp, 0, 0))) {
                fan.percentage = vtProp.boolVal ? 100.0 : 0.0;
                VariantClear(&vtProp);
            } else {
                fan.percentage = 50.0; // Default if unknown
            }
            
            // Convert percentage to RPM for display
            fan.currentSpeed = fan.percentage * 50; // Rough estimate
            fan.maxSpeed = 5000;
            fan.isControlAvailable = false;
            
            fans.push_back(fan);
            found = true;
            
            obj->Release();
        }
        
        cleanupWMI(pSvc);
    }
    
    return found;
}

// Helper function for Lenovo systems
bool getLenovoFanInfo(std::vector<FanData>& fans) {
    IWbemServices* pSvc = NULL;
    bool found = false;
    
    if (initWMI(&pSvc)) {
        // Try WMI classes often found in Lenovo systems
        auto results = executeWMIQuery(pSvc, L"SELECT * FROM Win32_TemperatureProbe");
        
        for (auto& obj : results) {
            VARIANT vtProp;
            VariantInit(&vtProp);
            
            // Check if this is a fan-related probe
            std::string description;
            if (SUCCEEDED(obj->Get(L"Description", 0, &vtProp, 0, 0))) {
                description = _bstr_t(vtProp.bstrVal);
                VariantClear(&vtProp);
                
                // Skip if not fan-related
                if (description.find("Fan") == std::string::npos) {
                    obj->Release();
                    continue;
                }
            }
            
            FanData fan;
            fan.name = "Lenovo " + description;
            fan.location = "System";
            
            // Lenovo typically reports current status rather than speed
            if (SUCCEEDED(obj->Get(L"CurrentReading", 0, &vtProp, 0, 0))) {
                fan.percentage = static_cast<double>(vtProp.intVal);
                if (fan.percentage > 100) fan.percentage = 100;
                VariantClear(&vtProp);
            } else {
                fan.percentage = 50; // Default if unknown
            }
            
            // Convert percentage to RPM for display
            fan.currentSpeed = fan.percentage * 50; // Rough conversion
            fan.maxSpeed = 5000;
            fan.isControlAvailable = false;
            
            fans.push_back(fan);
            found = true;
            
            obj->Release();
        }
        
        cleanupWMI(pSvc);
    }
    
    return found;
}

// Helper to get generic fan information through WMI
bool getGenericFanInfo(std::vector<FanData>& fans) {
    IWbemServices* pSvc = NULL;
    bool found = false;
    
    if (initWMI(&pSvc)) {
        auto results = executeWMIQuery(pSvc, L"SELECT * FROM Win32_Fan");
        
        for (auto& obj : results) {
            FanData fan;
            VARIANT vtProp;
            VariantInit(&vtProp);
            
            // Get fan name
            if (SUCCEEDED(obj->Get(L"Name", 0, &vtProp, 0, 0))) {
                fan.name = _bstr_t(vtProp.bstrVal);
                VariantClear(&vtProp);
            } else {
                fan.name = "System Fan";
            }
            
            // Get fan description
            if (SUCCEEDED(obj->Get(L"Description", 0, &vtProp, 0, 0))) {
                fan.location = _bstr_t(vtProp.bstrVal);
                VariantClear(&vtProp);
            } else {
                fan.location = "System";
            }
            
            // Get current speed
            if (SUCCEEDED(obj->Get(L"DesiredSpeed", 0, &vtProp, 0, 0))) {
                fan.currentSpeed = vtProp.intVal;
                VariantClear(&vtProp);
            } else {
                fan.currentSpeed = 0;
            }
            
            // Calculate percentage if we have speeds
            if (fan.currentSpeed > 0 && fan.maxSpeed > 0) {
                fan.percentage = (fan.currentSpeed / fan.maxSpeed) * 100.0;
                if (fan.percentage > 100) fan.percentage = 100;
            } else {
                // If DesiredSpeed is reported as a percentage
                fan.percentage = fan.currentSpeed;
                fan.maxSpeed = 100;
            }
            
            fan.isControlAvailable = false;
            fans.push_back(fan);
            found = true;
            
            obj->Release();
        }
        
        cleanupWMI(pSvc);
    }
    
    return found;
}

// Get fan data from system
std::vector<FanData> getFanData() {
    std::vector<FanData> fans;
    
    // Try vendor-specific implementations first
    if (getDellFanInfo(fans)) {
        return fans;
    }
    
    if (getHPFanInfo(fans)) {
        return fans;
    }
    
    if (getLenovoFanInfo(fans)) {
        return fans;
    }
    
    // Try generic implementations
    bool foundGenericFans = getGenericFanInfo(fans);
    
    // If we still have no fans, add a placeholder
    if (fans.empty()) {
        FanData placeholderFan;
        placeholderFan.name = "System Fan";
        placeholderFan.location = "System";
        placeholderFan.currentSpeed = 0;
        placeholderFan.maxSpeed = 100;
        placeholderFan.percentage = 0;
        placeholderFan.isControlAvailable = false;
        fans.push_back(placeholderFan);
    }
    
    return fans;
}

// Get fan information as JSON string
char* getFanInfo() {
    auto fans = getFanData();
    json fanArray = json::array();
    
    for (const auto& fan : fans) {
        json fanJson = {
            {"name", fan.name},
            {"location", fan.location},
            {"currentSpeed", fan.currentSpeed},
            {"maxSpeed", fan.maxSpeed},
            {"percentage", fan.percentage},
            {"isControlAvailable", fan.isControlAvailable}
        };
        fanArray.push_back(fanJson);
    }
    
    return strdup_cstr(fanArray.dump());
}

// Get the primary fan speed as a percentage
double getFanSpeedValue() {
    auto fans = getFanData();
    
    if (!fans.empty()) {
        // Return the first fan's percentage, preferably a system fan
        for (const auto& fan : fans) {
            if (fan.location == "System") {
                return fan.percentage;
            }
        }
        return fans[0].percentage;
    }
    
    return 0.0;
}