#pragma once

#include "common.h"
#include "strdup_cstr.h"
#include <string>

// Get OS information as JSON string
char* getOsInfoJson();

// Structure to hold OS information
struct OSInfo {
    std::string name;
    std::string version;
    std::string buildNumber;
    std::string edition;
    std::string architecture;
    std::string hostname;
    std::string username;
    std::string domain;
    std::string installDate;
    std::string lastBootTime;
    uint64_t uptime;             // in seconds
    std::string uptimeFormatted;
    std::string windowsDirectory;
    std::string systemDirectory;
    std::string systemDrive;
    std::string countryCode;
    std::string language;
    std::string timeZone;
    bool isAdmin;
    bool isVirtualMachine;
    bool isTabletMode;
    bool isRemoteSession;
    std::string licenseStatus;
    std::string activationStatus;
};

// Get detailed OS information
OSInfo getOSDetails();