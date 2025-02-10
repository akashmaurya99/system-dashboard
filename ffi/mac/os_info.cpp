#include "include/os_info.h"
#include <iostream>
#include <cstdlib>
#include <sstream>
#include <iomanip>

using namespace std;

// Function to execute shell commands and capture output
static string execCommand(const char* cmd) {
    char buffer[256];
    string result;
    FILE* pipe = popen(cmd, "r");
    if (!pipe) return "Error";

    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }
    pclose(pipe);

    result.erase(result.find_last_not_of(" \n\r\t") + 1);
    return result;
}

// Get macOS system details
static string getOSName() { return execCommand("sw_vers -productName"); }
static string getOSVersion() { return execCommand("sw_vers -productVersion"); }
static string getKernelVersion() { return execCommand("uname -r"); }
static string getSystemUptime() { return execCommand("uptime | awk -F', ' '{print $1}'"); }

// Get Timezone
static string getTimezone() {
    string timezone = execCommand("systemsetup -gettimezone 2>/dev/null | awk '{print $NF}'");
    if (timezone.empty() || timezone == "exiting!") {
        timezone = execCommand("readlink /etc/localtime | awk -F'/' '{print $NF}'");
    }
    return timezone.empty() ? "Unknown" : timezone;
}

// Get Boot Time
static string getBootTime() {
    string timestamp = execCommand("sysctl -n kern.boottime | awk '{print $4}' | tr -d ,");
    if (timestamp.empty()) return "Unknown";
    return execCommand(("date -r " + timestamp + " '+%Y-%m-%d %H:%M:%S'").c_str());
}

// Get System Language
static string getSystemLanguage() {
    string lang = execCommand("defaults read -g AppleLanguages | awk 'NR==2' | tr -d '(),\"'");
    return lang.empty() ? "Unknown" : lang;
}

// Get Friendly Mac Model Name (e.g., "MacBook Air (M1, 2020)")
static string getMacModel() {
    string model = execCommand("system_profiler SPHardwareDataType | awk -F\": \" '/Model Name/ {print $2}'");
    string chip = execCommand("system_profiler SPHardwareDataType | awk -F\": \" '/Chip/ {print $2}'");

    if (!chip.empty()) {
        model += " (" + chip + ")";
    }

    return model.empty() ? "Unknown" : model;
}



// Function to return system information as a JSON string
static string generateOsInfoJson() {
    ostringstream json;
    json << "{\n";
    json << "  \"PC Model\": \"" << getMacModel() << "\",\n";
    json << "  \"OS Name\": \"" << getOSName() << "\",\n";
    json << "  \"OS Version\": \"" << getOSVersion() << "\",\n";
    json << "  \"OS Kernel Version\": \"" << getKernelVersion() << "\",\n";
    json << "  \"System Uptime\": \"" << getSystemUptime() << "\",\n";
    json << "  \"Timezone\": \"" << getTimezone() << "\",\n";
    json << "  \"Boot Time\": \"" << getBootTime() << "\",\n";
    json << "  \"Default System Language\": \"" << getSystemLanguage() << "\"\n";
    json << "}";
    return json.str();
}



// FFI-Compatible Wrapper
extern "C" __attribute__((visibility("default"))) char* getOsInfoJson() {
    string result = generateOsInfoJson();
    char* cstr = (char*)malloc(result.size() + 1);
    if (cstr) {
        strcpy(cstr, result.c_str());
    }
    return cstr;
}

// // Free allocated memory
// extern "C" __attribute__((visibility("default"))) void free_cstr(char* ptr) {
//     if (ptr) {
//         free(ptr);
//     }
// }