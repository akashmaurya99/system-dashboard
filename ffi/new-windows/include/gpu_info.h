#pragma once

#include "common.h"
#include "strdup_cstr.h"

// Get GPU information as JSON string
char* getGPUInfo();

// Get current GPU usage percentage
double getGPUUsage();

// Get detailed GPU information
struct GPUInfoData {
    std::string gpuName;
    std::string vendor;
    double memorySize;
    double coreClockSpeed;
    double memoryClockSpeed;
    double temperature;
    double usagePercentage;
    double vramUsage;
    std::string driverVersion;
    bool isIntegrated;
};

// Function to populate GPUInfoData struct
GPUInfoData getGPUDetails();