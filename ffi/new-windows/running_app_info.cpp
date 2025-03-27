#include "include/running_app_info.h"
#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <winternl.h>
#include <shlwapi.h>
#include <shlobj.h>
#include <propvarutil.h>
#include <wil/resource.h>
#include <chrono>

#pragma comment(lib, "psapi.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "version.lib")

// Helper to format memory sizes
std::string formatMemorySize(uint64_t bytes) {
    const char* suffixes[] = {"B", "KB", "MB", "GB", "TB"};
    int suffixIndex = 0;
    double size = static_cast<double>(bytes);
    
    while (size >= 1024 && suffixIndex < 4) {
        size /= 1024;
        suffixIndex++;
    }
    
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "%.2f %s", size, suffixes[suffixIndex]);
    return std::string(buffer);
}

// Get version info from file
bool getFileVersionInfo(const std::string& filePath, std::string& description, std::string& companyName) {
    DWORD verHandle = 0;
    DWORD verSize = GetFileVersionInfoSizeA(filePath.c_str(), &verHandle);
    
    if (verSize == 0) {
        return false;
    }
    
    std::vector<BYTE> verData(verSize);
    if (!GetFileVersionInfoA(filePath.c_str(), verHandle, verSize, verData.data())) {
        return false;
    }
    
    LPVOID lpBuffer = nullptr;
    UINT size = 0;
    
    // Get file description
    if (VerQueryValueA(verData.data(), "\\StringFileInfo\\040904B0\\FileDescription", &lpBuffer, &size) && size > 0) {
        description = std::string(static_cast<const char*>(lpBuffer), size - 1);
    }
    
    // Get company name
    if (VerQueryValueA(verData.data(), "\\StringFileInfo\\040904B0\\CompanyName", &lpBuffer, &size) && size > 0) {
        companyName = std::string(static_cast<const char*>(lpBuffer), size - 1);
    }
    
    return true;
}

// Get window title for a process
std::string getWindowTitle(DWORD pid) {
    struct EnumData {
        DWORD pid;
        HWND hwnd;
    };
    
    EnumData data = {pid, NULL};
    
    BOOL bResult = EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL {
        EnumData* pData = reinterpret_cast<EnumData*>(lParam);
        DWORD windowPid = 0;
        
        GetWindowThreadProcessId(hwnd, &windowPid);
        
        if (windowPid == pData->pid && IsWindowVisible(hwnd)) {
            pData->hwnd = hwnd;
            return FALSE; // Stop enumeration
        }
        
        return TRUE; // Continue enumeration
    }, reinterpret_cast<LPARAM>(&data));
    
    if (data.hwnd) {
        char title[256] = {0};
        GetWindowTextA(data.hwnd, title, sizeof(title));
        return std::string(title);
    }
    
    return "";
}

// Check if a process is responding
bool isProcessResponding(DWORD pid) {
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
    
    if (hProcess == NULL) {
        return false;
    }
    
    DWORD exitCode = 0;
    BOOL result = GetExitCodeProcess(hProcess, &exitCode);
    CloseHandle(hProcess);
    
    return (result && exitCode == STILL_ACTIVE);
}

// Get process start time as a string
std::string getProcessStartTime(DWORD pid) {
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
    
    if (hProcess == NULL) {
        return "";
    }
    
    FILETIME createTime, exitTime, kernelTime, userTime;
    if (!GetProcessTimes(hProcess, &createTime, &exitTime, &kernelTime, &userTime)) {
        CloseHandle(hProcess);
        return "";
    }
    
    CloseHandle(hProcess);
    
    // Convert FILETIME to system time
    SYSTEMTIME systemTime;
    FileTimeToSystemTime(&createTime, &systemTime);
    
    // Format time string
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "%02d/%02d/%04d %02d:%02d:%02d",
             systemTime.wMonth, systemTime.wDay, systemTime.wYear,
             systemTime.wHour, systemTime.wMinute, systemTime.wSecond);
    
    return std::string(buffer);
}

// Get username of the process owner
std::string getProcessUsername(DWORD pid) {
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
    
    if (hProcess == NULL) {
        return "Unknown";
    }
    
    HANDLE hToken = NULL;
    if (!OpenProcessToken(hProcess, TOKEN_QUERY, &hToken)) {
        CloseHandle(hProcess);
        return "Unknown";
    }
    
    CloseHandle(hProcess);
    
    DWORD dwSize = 0;
    GetTokenInformation(hToken, TokenUser, NULL, 0, &dwSize);
    
    if (dwSize == 0) {
        CloseHandle(hToken);
        return "Unknown";
    }
    
    std::vector<BYTE> userInfo(dwSize);
    if (!GetTokenInformation(hToken, TokenUser, userInfo.data(), dwSize, &dwSize)) {
        CloseHandle(hToken);
        return "Unknown";
    }
    
    CloseHandle(hToken);
    
    SID_NAME_USE sidNameUse;
    char userName[256] = {0};
    char domainName[256] = {0};
    DWORD userNameSize = sizeof(userName);
    DWORD domainNameSize = sizeof(domainName);
    
    TOKEN_USER* pTokenUser = reinterpret_cast<TOKEN_USER*>(userInfo.data());
    
    if (!LookupAccountSidA(
        NULL,
        pTokenUser->User.Sid,
        userName,
        &userNameSize,
        domainName,
        &domainNameSize,
        &sidNameUse
    )) {
        return "Unknown";
    }
    
    return std::string(domainName) + "\\" + std::string(userName);
}

// Get CPU usage for a process
double getProcessCpuUsage(DWORD pid) {
    static std::map<DWORD, std::pair<FILETIME, FILETIME>> lastCpuTimes;
    static FILETIME lastSystemTime = {0};
    
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
    
    if (hProcess == NULL) {
        return 0.0;
    }
    
    FILETIME createTime, exitTime, kernelTime, userTime;
    if (!GetProcessTimes(hProcess, &createTime, &exitTime, &kernelTime, &userTime)) {
        CloseHandle(hProcess);
        return 0.0;
    }
    
    FILETIME systemTime;
    GetSystemTimeAsFileTime(&systemTime);
    
    // Check if we have previous measurements for this process
    if (lastCpuTimes.find(pid) != lastCpuTimes.end()) {
        FILETIME prevKernelTime = lastCpuTimes[pid].first;
        FILETIME prevUserTime = lastCpuTimes[pid].second;
        
        // Calculate time differences
        ULARGE_INTEGER kernelDiff, userDiff, systemDiff;
        
        kernelDiff.LowPart = kernelTime.dwLowDateTime;
        kernelDiff.HighPart = kernelTime.dwHighDateTime;
        
        userDiff.LowPart = userTime.dwLowDateTime;
        userDiff.HighPart = userTime.dwHighDateTime;
        
        systemDiff.LowPart = systemTime.dwLowDateTime;
        systemDiff.HighPart = systemTime.dwHighDateTime;
        
        ULARGE_INTEGER prevKernel, prevUser, prevSystem;
        
        prevKernel.LowPart = prevKernelTime.dwLowDateTime;
        prevKernel.HighPart = prevKernelTime.dwHighDateTime;
        
        prevUser.LowPart = prevUserTime.dwLowDateTime;
        prevUser.HighPart = prevUserTime.dwHighDateTime;
        
        prevSystem.LowPart = lastSystemTime.dwLowDateTime;
        prevSystem.HighPart = lastSystemTime.dwHighDateTime;
        
        // Calculate CPU usage
        ULONGLONG kernelTime = kernelDiff.QuadPart - prevKernel.QuadPart;
        ULONGLONG userTime = userDiff.QuadPart - prevUser.QuadPart;
        ULONGLONG systemTime = systemDiff.QuadPart - prevSystem.QuadPart;
        
        if (systemTime > 0) {
            double cpuUsage = ((kernelTime + userTime) * 100.0) / systemTime;
            
            // Update last times
            lastCpuTimes[pid] = std::make_pair(kernelTime, userTime);
            lastSystemTime = systemTime;
            
            CloseHandle(hProcess);
            return cpuUsage;
        }
    }
    
    // Store current measurements for next time
    lastCpuTimes[pid] = std::make_pair(kernelTime, userTime);
    lastSystemTime = systemTime;
    
    CloseHandle(hProcess);
    return 0.0;
}

// Get detailed process information
std::vector<ProcessInfo> getRunningProcesses() {
    std::vector<ProcessInfo> processes;
    
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return processes;
    }
    
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    
    if (!Process32First(hSnapshot, &pe32)) {
        CloseHandle(hSnapshot);
        return processes;
    }
    
    do {
        ProcessInfo process;
        process.name = pe32.szExeFile;
        process.pid = pe32.th32ProcessID;
        process.threadCount = pe32.cntThreads;
        
        // Skip system processes
        if (process.pid == 0 || process.pid == 4) {
            continue;
        }
        
        // Get process path and additional info
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, process.pid);
        
        if (hProcess != NULL) {
            char filePath[MAX_PATH] = {0};
            if (GetModuleFileNameExA(hProcess, NULL, filePath, MAX_PATH)) {
                process.path = filePath;
                
                // Get file description and company name
                getFileVersionInfo(process.path, process.description, process.companyName);
            }
            
            // Get memory usage
            PROCESS_MEMORY_COUNTERS_EX pmc;
            if (GetProcessMemoryInfo(hProcess, (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc))) {
                process.memoryUsage = pmc.PrivateUsage;
            }
            
            CloseHandle(hProcess);
        }
        
        // Get CPU usage
        process.cpuUsage = getProcessCpuUsage(process.pid);
        
        // Get process status
        process.isResponding = isProcessResponding(process.pid);
        
        // Get process start time
        process.startTime = getProcessStartTime(process.pid);
        
        // Get process owner
        process.username = getProcessUsername(process.pid);
        
        // Get window title
        process.windowTitle = getWindowTitle(process.pid);
        
        processes.push_back(process);
        
    } while (Process32Next(hSnapshot, &pe32));
    
    CloseHandle(hSnapshot);
    
    return processes;
}

// Get running applications information as JSON string
char* getRunningAppsInfo() {
    auto processes = getRunningProcesses();
    json processArray = json::array();
    
    for (const auto& process : processes) {
        json processJson = {
            {"name", process.name},
            {"pid", process.pid},
            {"cpuUsage", process.cpuUsage},
            {"memoryUsage", process.memoryUsage},
            {"memoryUsageFormatted", formatMemorySize(process.memoryUsage)},
            {"path", process.path},
            {"username", process.username},
            {"startTime", process.startTime},
            {"threadCount", process.threadCount},
            {"isResponding", process.isResponding},
            {"windowTitle", process.windowTitle},
            {"description", process.description},
            {"companyName", process.companyName}
        };
        processArray.push_back(processJson);
    }
    
    return strdup_cstr(processArray.dump());
}