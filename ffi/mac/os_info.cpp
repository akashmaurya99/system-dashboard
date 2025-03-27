#include "include/os_info.h"
#include <iostream>
#include <cstdlib>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <string>

using namespace std;

// Executes a shell command and returns the output as a trimmed string.
static string execCommand(const char* cmd) {
    char buffer[256];
    string result;
    FILE* pipe = popen(cmd, "r");
    if (!pipe) return "Error";
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }
    pclose(pipe);
    if (!result.empty()) {
        result.erase(result.find_last_not_of(" \n\r\t") + 1);
    }
    return result;
}

// OS Name from sw_vers
static string getOSName() {
    return execCommand("sw_vers -productName");
}

// OS Version from sw_vers
static string getOSVersion() {
    return execCommand("sw_vers -productVersion");
}

// Build Number from sw_vers
static string getBuildNumber() {
    return execCommand("sw_vers -buildVersion");
}

// Kernel Version via uname
static string getKernelVersion() {
    return execCommand("uname -r");
}

// Check if the system is 64-bit using the machine architecture.
static bool isSystem64Bit() {
    string arch = execCommand("uname -m");
    return (arch.find("64") != string::npos);
}

// Retrieves boot time (in seconds) from sysctl output.
// Expected format: "{ sec = 1675818611, usec = 0 }"
static time_t getBootTimeSeconds() {
    string bootInfo = execCommand("sysctl -n kern.boottime");
    size_t pos = bootInfo.find("sec =");
    if (pos != string::npos) {
        size_t start = bootInfo.find_first_of("0123456789", pos);
        size_t end = bootInfo.find_first_not_of("0123456789", start);
        string secStr = bootInfo.substr(start, end - start);
        try {
            return static_cast<time_t>(stoll(secStr));
        } catch (...) {
            return 0;
        }
    }
    return 0;
}

// Computes system uptime as a formatted string "X Days Y Hours"
static string getSystemUptime() {
    time_t bootTime = getBootTimeSeconds();
    if (bootTime == 0) return "Unknown";
    time_t now = time(nullptr);
    int diff = static_cast<int>(difftime(now, bootTime));
    int days = diff / (3600 * 24);
    diff %= (3600 * 24);
    int hours = diff / 3600;
    ostringstream oss;
    if (days > 0) {
        oss << days << " Days " << hours << " Hours";
    } else {
        oss << hours << " Hours";
    }
    return oss.str();
}

// Retrieves the device name using scutil.
static string getDeviceName() {
    string name = execCommand("scutil --get ComputerName");
    return name.empty() ? "Unknown" : name;
}

// Retrieves the host name.
static string getHostName() {
    string name = execCommand("hostname");
    return name.empty() ? "Unknown" : name;
}

// Retrieves the current user's name.
static string getUserName() {
    string name = execCommand("whoami");
    return name.empty() ? "Unknown" : name;
}

// Retrieves the system locale.
static string getLocale() {
    string locale = execCommand("defaults read -g AppleLocale");
    return locale.empty() ? "Unknown" : locale;
}

// Builds a JSON string containing all OS information fields.
static string generateOsInfoJson() {
    ostringstream json;
    json << "{\n";
    json << "  \"osName\": \"" << getOSName() << "\",\n";
    json << "  \"osVersion\": \"" << getOSVersion() << "\",\n";
    json << "  \"buildNumber\": \"" << getBuildNumber() << "\",\n";
    json << "  \"kernelVersion\": \"" << getKernelVersion() << "\",\n";
    json << "  \"is64Bit\": " << (isSystem64Bit() ? "true" : "false") << ",\n";
    json << "  \"systemUptime\": \"" << getSystemUptime() << "\",\n";
    json << "  \"deviceName\": \"" << getDeviceName() << "\",\n";
    json << "  \"hostName\": \"" << getHostName() << "\",\n";
    json << "  \"userName\": \"" << getUserName() << "\",\n";
    json << "  \"locale\": \"" << getLocale() << "\"\n";
    json << "}";
    return json.str();
}

// FFI-Compatible wrapper: returns a C-style string that must be freed by the caller.
char* getOsInfoJson() {
    string result = generateOsInfoJson();
    char* cstr = (char*)malloc(result.size() + 1);
    if (cstr) {
        strcpy(cstr, result.c_str());
    }
    return cstr;
}
