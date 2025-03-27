#pragma once

#include "common.h"
#include "strdup_cstr.h"

// Get CPU information as JSON string
char* getJsonCpuData();

// Get current CPU usage percentage (0-100)
double getCPUUsage();

// Get CPU temperature (if available)
double getCPUTemperature();

// Get detailed CPU information
struct CPUInfo {
    std::string processorName;
    int coreCount;
    int threadCount;
    double baseClockSpeed;
    double currentClockSpeed;
    double temperature;
    double usagePercentage;
    std::string architecture;
    int l1CacheSize;
    int l2CacheSize;
    int l3CacheSize;
    std::string vendor;
    std::string instructionSet;
};

// Function to populate CPUInfo struct
CPUInfo getCPUDetails();