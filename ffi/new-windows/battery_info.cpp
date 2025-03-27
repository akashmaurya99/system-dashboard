#include "include/battery_info.h"
#include <windows.h>
#include <batclass.h>
#include <setupapi.h>
#include <devguid.h>

// Link with required libraries
#pragma comment(lib, "setupapi.lib")

// Get battery data from system
BatteryData getBatteryData() {
    BatteryData data = {};
    data.name = "Battery";
    data.status = "Unknown";
    data.health = "Unknown";
    data.timeRemaining = -1;
    
    // Check if the system has a battery
    SYSTEM_POWER_STATUS powerStatus;
    if (GetSystemPowerStatus(&powerStatus)) {
        data.isACConnected = (powerStatus.ACLineStatus == 1);
        
        // Get battery percentage
        if (powerStatus.BatteryLifePercent != 255) {
            data.percentage = powerStatus.BatteryLifePercent;
        } else {
            data.percentage = 0;
        }
        
        // Get battery status
        switch (powerStatus.BatteryFlag) {
            case 1:
                data.status = "High";
                break;
            case 2:
                data.status = "Low";
                break;
            case 4:
                data.status = "Critical";
                break;
            case 8:
                data.status = "Charging";
                break;
            case 128:
                data.status = "No Battery";
                break;
            default:
                data.status = "Unknown";
        }
        
        // Get time remaining
        if (powerStatus.BatteryLifeTime != 0xFFFFFFFF) {
            data.timeRemaining = powerStatus.BatteryLifeTime / 60; // Convert seconds to minutes
        }
    }
    
    // Get more detailed battery information using WMI
    IWbemServices* pSvc = NULL;
    if (initWMI(&pSvc)) {
        auto results = executeWMIQuery(pSvc, L"SELECT * FROM Win32_Battery");
        
        if (!results.empty()) {
            VARIANT vtProp;
            
            // Get battery name
            VariantInit(&vtProp);
            if (SUCCEEDED(results[0]->Get(L"Name", 0, &vtProp, 0, 0))) {
                data.name = _bstr_t(vtProp.bstrVal);
                VariantClear(&vtProp);
            }
            
            // Get manufacturer
            VariantInit(&vtProp);
            if (SUCCEEDED(results[0]->Get(L"Manufacturer", 0, &vtProp, 0, 0))) {
                data.manufacturer = _bstr_t(vtProp.bstrVal);
                VariantClear(&vtProp);
            }
            
            // Get design capacity
            VariantInit(&vtProp);
            if (SUCCEEDED(results[0]->Get(L"DesignCapacity", 0, &vtProp, 0, 0))) {
                data.designCapacity = vtProp.intVal;
                VariantClear(&vtProp);
            }
            
            // Get current capacity
            VariantInit(&vtProp);
            if (SUCCEEDED(results[0]->Get(L"FullChargeCapacity", 0, &vtProp, 0, 0))) {
                data.currentCapacity = vtProp.intVal;
                VariantClear(&vtProp);
            }
            
            // Get serial number
            VariantInit(&vtProp);
            if (SUCCEEDED(results[0]->Get(L"DeviceID", 0, &vtProp, 0, 0))) {
                data.serialNumber = _bstr_t(vtProp.bstrVal);
                VariantClear(&vtProp);
            }
            
            // Get chemistry
            VariantInit(&vtProp);
            if (SUCCEEDED(results[0]->Get(L"Chemistry", 0, &vtProp, 0, 0))) {
                int chemistry = vtProp.intVal;
                switch (chemistry) {
                    case 1: data.chemistry = "Other"; break;
                    case 2: data.chemistry = "Unknown"; break;
                    case 3: data.chemistry = "Lead Acid"; break;
                    case 4: data.chemistry = "Nickel Cadmium"; break;
                    case 5: data.chemistry = "Nickel Metal Hydride"; break;
                    case 6: data.chemistry = "Lithium Ion"; break;
                    case 7: data.chemistry = "Zinc Air"; break;
                    case 8: data.chemistry = "Lithium Polymer"; break;
                    default: data.chemistry = "Unknown";
                }
                VariantClear(&vtProp);
            }
            
            // Get battery health status
            if (data.currentCapacity > 0 && data.designCapacity > 0) {
                double healthPercent = (data.currentCapacity / data.designCapacity) * 100.0;
                
                if (healthPercent >= 80) {
                    data.health = "Good";
                } else if (healthPercent >= 50) {
                    data.health = "Fair";
                } else {
                    data.health = "Poor";
                }
            }
            
            // Get voltage
            VariantInit(&vtProp);
            if (SUCCEEDED(results[0]->Get(L"DesignVoltage", 0, &vtProp, 0, 0))) {
                data.voltage = vtProp.intVal;
                VariantClear(&vtProp);
            }
            
            results[0]->Release();
        }
        
        cleanupWMI(pSvc);
    }
    
    return data;
}

// Get battery information as JSON string
char* getBatteryInfo() {
    BatteryData data = getBatteryData();
    
    json batteryJson = {
        {"name", data.name},
        {"manufacturer", data.manufacturer},
        {"designCapacity", data.designCapacity},
        {"currentCapacity", data.currentCapacity},
        {"cycleCount", data.cycleCount},
        {"voltage", data.voltage},
        {"temperature", data.temperature},
        {"chargeRate", data.chargeRate},
        {"dischargeRate", data.dischargeRate},
        {"percentage", data.percentage},
        {"status", data.status},
        {"health", data.health},
        {"timeRemaining", data.timeRemaining},
        {"isACConnected", data.isACConnected},
        {"serialNumber", data.serialNumber},
        {"chemistry", data.chemistry}
    };
    
    return strdup_cstr(batteryJson.dump());
}