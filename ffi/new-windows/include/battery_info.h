#pragma once

#include "common.h"
#include "strdup_cstr.h"

// Get battery information as JSON string
char* getBatteryInfo();

// Detailed battery information
struct BatteryData {
    std::string name;
    std::string manufacturer;
    double designCapacity;       // in mWh
    double currentCapacity;      // in mWh
    double cycleCount;
    double voltage;              // in mV
    double temperature;          // in Celsius
    double chargeRate;           // in mW, positive when charging
    double dischargeRate;        // in mW, positive when discharging
    double percentage;           // 0-100
    std::string status;          // "Charging", "Discharging", "Full", "Not present", etc.
    std::string health;          // "Good", "Fair", "Poor", etc.
    double timeRemaining;        // in minutes, -1 if unknown
    bool isACConnected;
    std::string serialNumber;
    std::string chemistry;       // "Li-ion", "NiMH", etc.
};