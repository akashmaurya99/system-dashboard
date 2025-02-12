#include "include/running_app_info.h"
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <sstream>
#include <vector>
#include <string>
#include <ctime>
#include <sys/sysctl.h>
#include <pwd.h>
#include <libproc.h>
#include <mach/mach.h>
#include <unistd.h>
#include <ApplicationServices/ApplicationServices.h> // For window title lookup

using namespace std;

// Structure to hold detailed process information.
struct ProgramInfo {
    int pid;
    int parentPid;
    std::string name;
    double cpuUsage;       // Average CPU usage (percentage)
    int memoryUsage;       // Resident memory (in kilobytes)
    std::string executablePath;
    std::string startTime; // ISO8601 formatted string, e.g. "2022-03-15T14:30:00"
    int threadCount;
    std::string user;
    std::string state;     // Process state as a single character (e.g. "R", "S", "T", "Z")
    std::string windowTitle; // Window title (if available; otherwise "0")
};

//
// Helper: Convert a timeval (from kinfo_proc.p_starttime) to an ISO8601 string.
//
static std::string convertTimevalToISO(const timeval &tv) {
    time_t t = tv.tv_sec;
    struct tm tm;
    localtime_r(&t, &tm);
    char buf[64];
    strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S", &tm);
    return std::string(buf);
}

//
// Executes a shell command and returns its output as a string.
//
static string execCommand(const char* cmd) {
    string result;
    FILE* pipe = popen(cmd, "r");
    if (!pipe) {
        return "ERROR";
    }
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result.append(buffer);
    }
    pclose(pipe);
    return result;
}

//
// Helper: Retrieve the window title for a given process ID (pid) using the Core Graphics API.
// If the required permissions are not granted (or no title is available), returns "0".
//
static std::string getWindowTitleForPID(int pid) {
    std::string windowTitle = "";
    CFArrayRef windowList = CGWindowListCopyWindowInfo(kCGWindowListOptionOnScreenOnly, kCGNullWindowID);
    // If unable to get the window list (likely due to permission issues), return "0"
    if (!windowList) {
        return "0";
    }
    CFIndex count = CFArrayGetCount(windowList);
    for (CFIndex i = 0; i < count; i++) {
        CFDictionaryRef dict = (CFDictionaryRef)CFArrayGetValueAtIndex(windowList, i);
        if (dict) {
            // Check the owner PID.
            CFNumberRef ownerPIDRef = (CFNumberRef)CFDictionaryGetValue(dict, kCGWindowOwnerPID);
            int ownerPID = 0;
            if (ownerPIDRef)
                CFNumberGetValue(ownerPIDRef, kCFNumberIntType, &ownerPID);
            if (ownerPID == pid) {
                // Try to get the window title.
                CFStringRef titleRef = (CFStringRef)CFDictionaryGetValue(dict, kCGWindowName);
                if (titleRef) {
                    char title[256];
                    if (CFStringGetCString(titleRef, title, sizeof(title), kCFStringEncodingUTF8)) {
                        windowTitle = title;
                        break; // use the first matching nonempty title
                    }
                }
            }
        }
    }
    CFRelease(windowList);
    if (windowTitle.empty()) {
        // Return "0" if no title was found (which might be due to a permission issue)
        windowTitle = "0";
    }
    return windowTitle;
}

//
// Returns a JSON string listing the installed applications (by name).
// If the required permissions are not granted (detected via error output), returns 0.
//
static string getInstalledApplicationsJSON_Internal() {
    vector<string> apps;
    string output = execCommand("mdfind 'kMDItemContentType == \"com.apple.application-bundle\"'");
    // If mdfind reports a permission error or returns no data, return 0.
    if (output.find("Operation not permitted") != string::npos || output.empty()) {
        return "{ \"installed_applications\": 0 }";
    }
    stringstream ss(output);
    string line;
    while (getline(ss, line)) {
        size_t lastSlash = line.find_last_of("/");
        size_t dotApp = line.rfind(".app");
        if (lastSlash != string::npos && dotApp != string::npos) {
            string appName = line.substr(lastSlash + 1, dotApp - lastSlash - 1);
            apps.push_back(appName);
        }
    }
    ostringstream json;
    json << "{ \"installed_applications\": [";
    for (size_t i = 0; i < apps.size(); i++) {
        json << "\"" << apps[i] << "\"";
        if (i < apps.size() - 1)
            json << ", ";
    }
    json << "] }";
    return json.str();
}

//
// Retrieves detailed information about running processes.
// For any field that requires extra permission, if access is denied the code assigns 0 (or "0").
//
static string getRunningProcessesJSON_Internal() {
    vector<ProgramInfo> programs;
    
    int mib[4] = {CTL_KERN, KERN_PROC, KERN_PROC_ALL, 0};
    size_t size = 0;
    if (sysctl(mib, 4, NULL, &size, NULL, 0) < 0) {
        return "{ \"running_programs\": 0 }";
    }
    struct kinfo_proc* procList = (struct kinfo_proc*)malloc(size);
    if (!procList) {
        return "{ \"running_programs\": 0 }";
    }
    if (sysctl(mib, 4, procList, &size, NULL, 0) < 0) {
        free(procList);
        return "{ \"running_programs\": 0 }";
    }
    int count = size / sizeof(struct kinfo_proc);
    
    for (int i = 0; i < count; i++) {
        struct kinfo_proc proc = procList[i];
        ProgramInfo info;
        info.pid = proc.kp_proc.p_pid;
        info.parentPid = proc.kp_eproc.e_ppid;
        info.name = proc.kp_proc.p_comm; // initial command name
        info.startTime = convertTimevalToISO(proc.kp_proc.p_starttime);
        
        // Get the username from the UID.
        uid_t uid = proc.kp_eproc.e_ucred.cr_uid;
        struct passwd* pw = getpwuid(uid);
        if (pw) {
            info.user = pw->pw_name;
        } else {
            info.user = std::to_string(uid);
        }
        
        // Default (permission-guarded) values are set to 0.
        info.cpuUsage = 0.0;
        info.memoryUsage = 0;
        info.threadCount = 0;
        info.state = "0";
        info.windowTitle = "0";
        
        // Try to get task info.
        struct proc_taskallinfo taskallinfo;
        int ret = proc_pidinfo(info.pid, PROC_PIDTASKALLINFO, 0, &taskallinfo, sizeof(taskallinfo));
        if (ret == (int)sizeof(taskallinfo)) {
            info.memoryUsage = (int)(taskallinfo.ptinfo.pti_resident_size / 1024);
            info.threadCount = taskallinfo.ptinfo.pti_threadnum;
            double total_cpu_sec = (double)(taskallinfo.ptinfo.pti_total_user + taskallinfo.ptinfo.pti_total_system) / 1e6;
            time_t now = time(NULL);
            time_t start_time = proc.kp_proc.p_starttime.tv_sec;
            double lifetime = difftime(now, start_time);
            info.cpuUsage = (lifetime > 0) ? (total_cpu_sec / lifetime) * 100.0 : 0.0;
            
            // Map the BSD status to a state letter.
            int status = taskallinfo.pbsd.pbi_status;
            char stateChar = '?';
            switch(status) {
                case 1: stateChar = 'R'; break; // running
                case 2: stateChar = 'S'; break; // sleeping
                case 3: stateChar = 'S'; break; // idle/sleeping
                case 4: stateChar = 'T'; break; // stopped
                case 5: stateChar = 'Z'; break; // zombie
                default: stateChar = '?'; break;
            }
            info.state = std::string(1, stateChar);
        }
        
        // Retrieve the full executable path. If permission is lacking, set to "0".
        char pathBuffer[PROC_PIDPATHINFO_MAXSIZE];
        int retPath = proc_pidpath(info.pid, pathBuffer, sizeof(pathBuffer));
        if (retPath > 0) {
            info.executablePath = std::string(pathBuffer);
            size_t pos = info.executablePath.find_last_of("/");
            if (pos != std::string::npos && pos + 1 < info.executablePath.size()) {
                info.name = info.executablePath.substr(pos + 1);
            }
        } else {
            info.executablePath = "0";
            info.name = "0";
        }
        
        // Lookup the window title. (If missing permission, getWindowTitleForPID returns "0".)
        info.windowTitle = getWindowTitleForPID(info.pid);
        
        programs.push_back(info);
    }
    
    free(procList);
    
    // Build JSON output.
    ostringstream json;
    json << "{ \"running_programs\": [";
    for (size_t i = 0; i < programs.size(); i++) {
        const ProgramInfo &p = programs[i];
        json << "{";
        json << "\"pid\": " << p.pid << ", ";
        json << "\"parentPid\": " << p.parentPid << ", ";
        json << "\"name\": \"" << p.name << "\", ";
        json << "\"cpuUsage\": " << p.cpuUsage << ", ";
        json << "\"memoryUsage\": " << p.memoryUsage << ", ";
        json << "\"executablePath\": \"" << p.executablePath << "\", ";
        json << "\"startTime\": \"" << p.startTime << "\", ";
        json << "\"threadCount\": " << p.threadCount << ", ";
        json << "\"user\": \"" << p.user << "\", ";
        json << "\"state\": \"" << p.state << "\", ";
        json << "\"gpuUsage\": " << 0 << ", ";
        json << "\"windowTitle\": \"" << p.windowTitle << "\"";
        json << "}";
        if (i < programs.size() - 1)
            json << ", ";
    }
    json << "] }";
    return json.str();
}

//
// Exposed functions (using C linkage) for FFI.
//
// The caller on the Flutter side is responsible for freeing the returned memory.
//
extern "C" __attribute__((visibility("default")))
char* getRunningProcessesJSON() {
    string result = getRunningProcessesJSON_Internal();
    char* cstr = (char*)malloc(result.size() + 1);
    if (cstr) {
        strcpy(cstr, result.c_str());
    }
    return cstr;
}

extern "C" __attribute__((visibility("default")))
char* getInstalledApplicationsJSON() {
    string result = getInstalledApplicationsJSON_Internal();
    char* cstr = (char*)malloc(result.size() + 1);
    if (cstr) {
        strcpy(cstr, result.c_str());
    }
    return cstr;
}
