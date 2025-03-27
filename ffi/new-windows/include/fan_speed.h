#pragma once

#include "common.h"
#include "strdup_cstr.h"
#include <vector>

// Get fan information as JSON string
char* getFanInfo();

// Get the primary fan speed as a percentage
double getFanSpeedValue();

// Get detailed fan information
struct FanData {
    std::string name;
    double currentSpeed;   // RPM
    double maxSpeed;       // RPM
    double percentage;     // 0-100
    std::string location;  // CPU, GPU, System, etc.
    bool isControlAvailable;
};

// Get fan data from the system
std::vector<FanData> getFanData();