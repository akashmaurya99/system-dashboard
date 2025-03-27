#pragma once

#include "common.h"
#include "strdup_cstr.h"
#include <vector>

// Get running applications information as JSON string
char* getRunningAppsInfo();

// Structure to hold process information
struct ProcessInfo {
    std::string name;
    DWORD pid;
    double cpuUsage;
    uint64_t memoryUsage;
    std::string path;
    std::string username;
    std::string startTime;
    int threadCount;
    bool isResponding;
    std::string windowTitle;
    std::string description;
    std::string companyName;
};

// Get detailed process information
std::vector<ProcessInfo> getRunningProcesses();