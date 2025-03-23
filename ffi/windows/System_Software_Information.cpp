#include <iostream>
#include <windows.h>
#include <tlhelp32.h>
#include <vector>
#include <ctime>
#include <locale>
#include <sstream>
#include <iomanip>
#include <codecvt>

using namespace std;

// Structure to hold system information
struct SystemInfo {
    struct OSInfo {
        string name;
        string version;
        int buildNumber;
    } os;

    struct KernelInfo {
        int major;
        int minor;
    } kernel;

    struct Uptime {
        int hours;
        int minutes;
    } uptime;

    string timezone;
    string bootTime;
    string language;
    vector<string> processes;
    vector<string> users;
};

// Helper: Convert wide string to UTF-8
static string wideToUTF8(const wchar_t* wstr) {
    if (!wstr) return "";
    wstring_convert<codecvt_utf8<wchar_t>> conv;
    return conv.to_bytes(wstr);
}

// Helper: Get OS information
static void getOSInfo(SystemInfo& info) {
    OSVERSIONINFOEX osvi = { sizeof(OSVERSIONINFOEX) };
    if (GetVersionEx((OSVERSIONINFO*)&osvi)) {
        info.os.name = "Windows";
        info.os.version = to_string(osvi.dwMajorVersion) + "." + to_string(osvi.dwMinorVersion);
        info.os.buildNumber = osvi.dwBuildNumber;
    }
}

// Helper: Get kernel version
static void getKernelVersion(SystemInfo& info) {
    DWORD version = GetVersion();
    info.kernel.major = LOBYTE(LOWORD(version));
    info.kernel.minor = HIBYTE(LOWORD(version));
}

// Helper: Calculate system uptime
static void getSystemUptime(SystemInfo& info) {
    DWORD seconds = GetTickCount64() / 1000;
    info.uptime.hours = seconds / 3600;
    info.uptime.minutes = (seconds % 3600) / 60;
}

// Helper: Get timezone
static void getTimezone(SystemInfo& info) {
    TIME_ZONE_INFORMATION tz;
    GetTimeZoneInformation(&tz);
    info.timezone = wideToUTF8(tz.StandardName);
}

// Helper: Get boot time
static void getBootTime(SystemInfo& info) {
    ULONGLONG uptime = GetTickCount64() / 1000;
    time_t bootTime = time(nullptr) - uptime;
    char buffer[26];
    ctime_s(buffer, sizeof(buffer), &bootTime);
    info.bootTime = buffer;
    info.bootTime.erase(info.bootTime.find_last_not_of("\n") + 1);
}

// Helper: Get system language
static void getSystemLanguage(SystemInfo& info) {
    wchar_t locale[LOCALE_NAME_MAX_LENGTH];
    if (GetUserDefaultLocaleName(locale, LOCALE_NAME_MAX_LENGTH)) {
        info.language = wideToUTF8(locale);
    }
}

// Helper: List running processes
static void getRunningProcesses(SystemInfo& info) {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32W pe = { sizeof(PROCESSENTRY32W) };
        if (Process32FirstW(hSnapshot, &pe)) {
            do {
                info.processes.push_back(
                    wideToUTF8(pe.szExeFile) + 
                    " (PID: " + 
                    to_string(pe.th32ProcessID) + 
                    ")"
                );
            } while (Process32NextW(hSnapshot, &pe));
        }
        CloseHandle(hSnapshot);
    }
}

// Helper: List user accounts
static void getUserAccounts(SystemInfo& info) {
    // Simplified implementation - replace with NetUserEnum for production
    vector<string> defaultUsers = {"Administrator", "Guest", "DefaultUser"};
    info.users = defaultUsers;
}

// Convert SystemInfo to JSON
static string systemInfoToJSON(const SystemInfo& info) {
    stringstream json;
    json << "{\n";
    
    // OS Info
    json << "  \"os\": {\n"
         << "    \"name\": \"" << info.os.name << "\",\n"
         << "    \"version\": \"" << info.os.version << "\",\n"
         << "    \"build\": " << info.os.buildNumber << "\n"
         << "  },\n";
    
    // Kernel Info
    json << "  \"kernel\": {\n"
         << "    \"version\": \"" << info.kernel.major << "." << info.kernel.minor << "\"\n"
         << "  },\n";
    
    // Uptime
    json << "  \"uptime\": {\n"
         << "    \"hours\": " << info.uptime.hours << ",\n"
         << "    \"minutes\": " << info.uptime.minutes << "\n"
         << "  },\n";
    
    // Other fields
    json << "  \"timezone\": \"" << info.timezone << "\",\n"
         << "  \"boot_time\": \"" << info.bootTime << "\",\n"
         << "  \"language\": \"" << info.language << "\",\n";
    
    // Processes
    json << "  \"processes\": [";
    for (size_t i = 0; i < info.processes.size(); i++) {
        json << "\"" << info.processes[i] << "\"";
        if (i < info.processes.size() - 1) json << ", ";
    }
    json << "],\n";
    
    // Users
    json << "  \"users\": [";
    for (size_t i = 0; i < info.users.size(); i++) {
        json << "\"" << info.users[i] << "\"";
        if (i < info.users.size() - 1) json << ", ";
    }
    json << "]\n";
    
    json << "}";
    return json.str();
}

int main() {
    SystemInfo info;
    
    getOSInfo(info);
    getKernelVersion(info);
    getSystemUptime(info);
    getTimezone(info);
    getBootTime(info);
    getSystemLanguage(info);
    getRunningProcesses(info);
    getUserAccounts(info);
    
    cout << systemInfoToJSON(info) << endl;
    return 0;
}
