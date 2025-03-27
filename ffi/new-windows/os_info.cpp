#include "include/os_info.h"
#include <windows.h>
#include <sysinfoapi.h>
#include <lmcons.h>
#include <shlobj.h>
#include <wtsapi32.h>
#include <versionhelpers.h>
#include <ctime>
#include <sstream>
#include <iomanip>

#pragma comment(lib, "wtsapi32.lib")

// Format uptime as string (days, hours, minutes, seconds)
std::string formatUptime(uint64_t seconds) {
    uint64_t days = seconds / 86400;
    seconds %= 86400;
    uint64_t hours = seconds / 3600;
    seconds %= 3600;
    uint64_t minutes = seconds / 60;
    seconds %= 60;
    
    std::ostringstream oss;
    if (days > 0) {
        oss << days << " days, ";
    }
    oss << std::setfill('0') << std::setw(2) << hours << ":"
        << std::setfill('0') << std::setw(2) << minutes << ":"
        << std::setfill('0') << std::setw(2) << seconds;
    
    return oss.str();
}

// Format time as ISO8601 string
std::string formatTime(FILETIME ft) {
    SYSTEMTIME st;
    FileTimeToSystemTime(&ft, &st);
    
    std::ostringstream oss;
    oss << st.wYear << "-"
        << std::setfill('0') << std::setw(2) << st.wMonth << "-"
        << std::setfill('0') << std::setw(2) << st.wDay << "T"
        << std::setfill('0') << std::setw(2) << st.wHour << ":"
        << std::setfill('0') << std::setw(2) << st.wMinute << ":"
        << std::setfill('0') << std::setw(2) << st.wSecond << "Z";
    
    return oss.str();
}

// Format current local time as ISO8601 string
std::string formatCurrentTime() {
    time_t now = time(nullptr);
    struct tm tm_info;
    localtime_s(&tm_info, &now);
    
    std::ostringstream oss;
    oss << (tm_info.tm_year + 1900) << "-"
        << std::setfill('0') << std::setw(2) << (tm_info.tm_mon + 1) << "-"
        << std::setfill('0') << std::setw(2) << tm_info.tm_mday << "T"
        << std::setfill('0') << std::setw(2) << tm_info.tm_hour << ":"
        << std::setfill('0') << std::setw(2) << tm_info.tm_min << ":"
        << std::setfill('0') << std::setw(2) << tm_info.tm_sec << "Z";
    
    return oss.str();
}

// Check if running elevated (as administrator)
bool isRunningAsAdmin() {
    BOOL isAdmin = FALSE;
    PSID adminGroup = NULL;
    SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;
    
    if (AllocateAndInitializeSid(&ntAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
                                DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &adminGroup)) {
        CheckTokenMembership(NULL, adminGroup, &isAdmin);
        FreeSid(adminGroup);
    }
    
    return isAdmin != FALSE;
}

// Check if running in a virtual machine
bool isVirtualMachine() {
    bool result = false;
    IWbemServices* pSvc = NULL;
    
    if (initWMI(&pSvc)) {
        // Check for VM specific hardware or software
        auto results = executeWMIQuery(pSvc, L"SELECT * FROM Win32_ComputerSystem");
        
        if (!results.empty()) {
            VARIANT vtProp;
            VariantInit(&vtProp);
            
            if (SUCCEEDED(results[0]->Get(L"Manufacturer", 0, &vtProp, 0, 0))) {
                std::wstring manufacturer = vtProp.bstrVal;
                std::transform(manufacturer.begin(), manufacturer.end(), manufacturer.begin(), ::toupper);
                
                // Check for common VM vendors
                if (manufacturer.find(L"VMWARE") != std::wstring::npos ||
                    manufacturer.find(L"VIRTUALBOX") != std::wstring::npos ||
                    manufacturer.find(L"MICROSOFT CORPORATION") != std::wstring::npos ||
                    manufacturer.find(L"PARALLELS") != std::wstring::npos ||
                    manufacturer.find(L"QEMU") != std::wstring::npos ||
                    manufacturer.find(L"XEN") != std::wstring::npos) {
                    result = true;
                }
                
                VariantClear(&vtProp);
            }
            
            if (!result && SUCCEEDED(results[0]->Get(L"Model", 0, &vtProp, 0, 0))) {
                std::wstring model = vtProp.bstrVal;
                std::transform(model.begin(), model.end(), model.begin(), ::toupper);
                
                // Check for common VM model names
                if (model.find(L"VIRTUAL") != std::wstring::npos ||
                    model.find(L"VMWARE") != std::wstring::npos ||
                    model.find(L"VIRTUALBOX") != std::wstring::npos ||
                    model.find(L"KVM") != std::wstring::npos ||
                    model.find(L"HVM") != std::wstring::npos) {
                    result = true;
                }
                
                VariantClear(&vtProp);
            }
            
            results[0]->Release();
        }
        
        // Additional checks could look for VM-specific devices
        if (!result) {
            auto deviceResults = executeWMIQuery(pSvc, L"SELECT * FROM Win32_PnPEntity WHERE (Caption LIKE '%VMware%' OR Caption LIKE '%VBox%')");
            result = !deviceResults.empty();
            
            for (auto& obj : deviceResults) {
                obj->Release();
            }
        }
        
        cleanupWMI(pSvc);
    }
    
    return result;
}

// Check if device is in tablet mode
bool isTabletMode() {
    // This is a simplistic check - a more advanced implementation would
    // monitor for WM_SETTINGCHANGE messages with "ConvertibleSlateMode" wparam
    
    HKEY hKey;
    DWORD value = 0;
    DWORD dataSize = sizeof(DWORD);
    
    // Open registry key for tablet mode
    if (RegOpenKeyEx(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\ImmersiveShell", 
                     0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        // Read TabletMode value
        if (RegQueryValueEx(hKey, L"TabletMode", NULL, NULL, (LPBYTE)&value, &dataSize) == ERROR_SUCCESS) {
            RegCloseKey(hKey);
            return value != 0;
        }
        RegCloseKey(hKey);
    }
    
    // Fallback to checking if a touch digitizer is present
    int value_GetSystemMetrics = GetSystemMetrics(SM_DIGITIZER);
    return (value_GetSystemMetrics & NID_READY) && 
           ((value_GetSystemMetrics & NID_INTEGRATED_TOUCH) || 
            (value_GetSystemMetrics & NID_INTEGRATED_PEN));
}

// Check if current session is a remote session
bool isRemoteSession() {
    return GetSystemMetrics(SM_REMOTESESSION) != 0;
}

// Get Windows activation status
std::string getActivationStatus() {
    // Note: Requires admin rights for detailed info
    std::string status = "Unknown";
    
    HKEY hKey;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\SoftwareProtectionPlatform", 
                     0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        WCHAR value[255] = {0};
        DWORD dataSize = sizeof(value);
        
        if (RegQueryValueEx(hKey, L"LicenseStatus", NULL, NULL, (LPBYTE)value, &dataSize) == ERROR_SUCCESS) {
            DWORD licenseStatus = _wtoi(value);
            
            switch (licenseStatus) {
                case 0: status = "Unlicensed"; break;
                case 1: status = "Licensed"; break;
                case 2: status = "OOBGrace"; break;
                case 3: status = "OOTGrace"; break;
                case 4: status = "NonGenuineGrace"; break;
                case 5: status = "Notification"; break;
                case 6: status = "ExtendedGrace"; break;
                default: status = "Unknown";
            }
        }
        
        RegCloseKey(hKey);
    }
    
    return status;
}

// Get detailed OS information
OSInfo getOSDetails() {
    OSInfo info = {};
    
    // Get OS version info
    OSVERSIONINFOEX osInfo = {0};
    osInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    
    // Note: GetVersionEx is deprecated, but still works for basic info
    // For production, use VerifyVersionInfo or RtlGetVersion
    
    typedef NTSTATUS(WINAPI* RtlGetVersionFn)(PRTL_OSVERSIONINFOW);
    HMODULE hNtDll = GetModuleHandle(L"ntdll.dll");
    if (hNtDll) {
        RtlGetVersionFn RtlGetVersion = (RtlGetVersionFn)GetProcAddress(hNtDll, "RtlGetVersion");
        if (RtlGetVersion) {
            RtlGetVersion((PRTL_OSVERSIONINFOW)&osInfo);
        }
    }
    
    // Fallback to GetVersionEx if RtlGetVersion is not available
    if (osInfo.dwMajorVersion == 0) {
        // Suppress deprecation warning
        #pragma warning(disable:4996)
        GetVersionEx((LPOSVERSIONINFO)&osInfo);
        #pragma warning(default:4996)
    }
    
    // Get version string
    info.version = std::to_string(osInfo.dwMajorVersion) + "." + 
                   std::to_string(osInfo.dwMinorVersion) + "." + 
                   std::to_string(osInfo.dwBuildNumber);
    
    // Get build number
    info.buildNumber = std::to_string(osInfo.dwBuildNumber);
    
    // Get OS name based on version
    if (osInfo.dwMajorVersion == 10) {
        if (osInfo.dwBuildNumber >= 22000) {
            info.name = "Windows 11";
        } else {
            info.name = "Windows 10";
        }
    } else if (osInfo.dwMajorVersion == 6) {
        switch (osInfo.dwMinorVersion) {
            case 0: info.name = "Windows Vista"; break;
            case 1: info.name = "Windows 7"; break;
            case 2: info.name = "Windows 8"; break;
            case 3: info.name = "Windows 8.1"; break;
            default: info.name = "Windows NT"; break;
        }
    } else if (osInfo.dwMajorVersion == 5) {
        switch (osInfo.dwMinorVersion) {
            case 0: info.name = "Windows 2000"; break;
            case 1: info.name = "Windows XP"; break;
            case 2: info.name = "Windows Server 2003"; break;
            default: info.name = "Windows NT"; break;
        }
    } else {
        info.name = "Windows";
    }
    
    // Get edition
    if (osInfo.wProductType == VER_NT_WORKSTATION) {
        // Additional checks for specific editions
        DWORD dwType = 0;
        if (GetProductInfo(osInfo.dwMajorVersion, osInfo.dwMinorVersion, 
                          osInfo.wServicePackMajor, osInfo.wServicePackMinor, &dwType)) {
            switch (dwType) {
                case PRODUCT_ULTIMATE: info.edition = "Ultimate"; break;
                case PRODUCT_PROFESSIONAL: info.edition = "Professional"; break;
                case PRODUCT_HOME_PREMIUM: info.edition = "Home Premium"; break;
                case PRODUCT_HOME_BASIC: info.edition = "Home Basic"; break;
                case PRODUCT_ENTERPRISE: info.edition = "Enterprise"; break;
                case PRODUCT_STARTER: info.edition = "Starter"; break;
                case PRODUCT_EDUCATION: info.edition = "Education"; break;
                case PRODUCT_ENTERPRISE_S: info.edition = "Enterprise S"; break;
                case PRODUCT_PROFESSIONAL_N: info.edition = "Professional N"; break;
                case PRODUCT_PROFESSIONAL_WMC: info.edition = "Professional with Media Center"; break;
                default: info.edition = ""; break;
            }
        }
    } else if (osInfo.wProductType == VER_NT_SERVER) {
        info.edition = "Server";
    } else if (osInfo.wProductType == VER_NT_DOMAIN_CONTROLLER) {
        info.edition = "Domain Controller";
    }
    
    // Combine name and edition
    if (!info.edition.empty()) {
        info.name += " " + info.edition;
    }
    
    // Get architecture
    SYSTEM_INFO sysInfo = {0};
    GetNativeSystemInfo(&sysInfo);
    
    switch (sysInfo.wProcessorArchitecture) {
        case PROCESSOR_ARCHITECTURE_AMD64: info.architecture = "x64"; break;
        case PROCESSOR_ARCHITECTURE_ARM: info.architecture = "ARM"; break;
        case PROCESSOR_ARCHITECTURE_ARM64: info.architecture = "ARM64"; break;
        case PROCESSOR_ARCHITECTURE_INTEL: info.architecture = "x86"; break;
        default: info.architecture = "Unknown"; break;
    }
    
    // Get hostname
    WCHAR hostname[MAX_COMPUTERNAME_LENGTH + 1] = {0};
    DWORD hostnameSize = sizeof(hostname) / sizeof(hostname[0]);
    if (GetComputerNameW(hostname, &hostnameSize)) {
        info.hostname = wstrToStr(hostname);
    } else {
        info.hostname = "Unknown";
    }
    
    // Get username
    WCHAR username[UNLEN + 1] = {0};
    DWORD usernameSize = sizeof(username) / sizeof(username[0]);
    if (GetUserNameW(username, &usernameSize)) {
        info.username = wstrToStr(username);
    } else {
        info.username = "Unknown";
    }
    
    // Get domain
    WCHAR domain[MAX_COMPUTERNAME_LENGTH + 1] = {0};
    DWORD domainSize = sizeof(domain) / sizeof(domain[0]);
    if (GetComputerNameExW(ComputerNameDnsDomain, domain, &domainSize) && domain[0] != L'\0') {
        info.domain = wstrToStr(domain);
    } else {
        // Try to get workgroup
        if (GetComputerNameExW(ComputerNameWorkgroup, domain, &domainSize) && domain[0] != L'\0') {
            info.domain = wstrToStr(domain);
        } else {
            info.domain = "WORKGROUP";
        }
    }
    
    // Get install date and last boot time from WMI
    IWbemServices* pSvc = NULL;
    if (initWMI(&pSvc)) {
        auto osResults = executeWMIQuery(pSvc, L"SELECT * FROM Win32_OperatingSystem");
        
        if (!osResults.empty()) {
            VARIANT vtProp;
            VariantInit(&vtProp);
            
            // Get install date
            if (SUCCEEDED(osResults[0]->Get(L"InstallDate", 0, &vtProp, 0, 0))) {
                std::wstring installDate = vtProp.bstrVal;
                
                // Convert WMI datetime format (yyyymmddHHMMSS.mmmmmm+UUU) to ISO8601
                if (installDate.length() >= 14) {
                    std::string year = wstrToStr(installDate.substr(0, 4));
                    std::string month = wstrToStr(installDate.substr(4, 2));
                    std::string day = wstrToStr(installDate.substr(6, 2));
                    std::string hour = wstrToStr(installDate.substr(8, 2));
                    std::string minute = wstrToStr(installDate.substr(10, 2));
                    std::string second = wstrToStr(installDate.substr(12, 2));
                    
                    info.installDate = year + "-" + month + "-" + day + "T" + 
                                      hour + ":" + minute + ":" + second + "Z";
                } else {
                    info.installDate = wstrToStr(installDate);
                }
                
                VariantClear(&vtProp);
            } else {
                info.installDate = "Unknown";
            }
            
            // Get last boot time
            if (SUCCEEDED(osResults[0]->Get(L"LastBootUpTime", 0, &vtProp, 0, 0))) {
                std::wstring lastBootTime = vtProp.bstrVal;
                
                // Convert WMI datetime format (yyyymmddHHMMSS.mmmmmm+UUU) to ISO8601
                if (lastBootTime.length() >= 14) {
                    std::string year = wstrToStr(lastBootTime.substr(0, 4));
                    std::string month = wstrToStr(lastBootTime.substr(4, 2));
                    std::string day = wstrToStr(lastBootTime.substr(6, 2));
                    std::string hour = wstrToStr(lastBootTime.substr(8, 2));
                    std::string minute = wstrToStr(lastBootTime.substr(10, 2));
                    std::string second = wstrToStr(lastBootTime.substr(12, 2));
                    
                    info.lastBootTime = year + "-" + month + "-" + day + "T" + 
                                       hour + ":" + minute + ":" + second + "Z";
                } else {
                    info.lastBootTime = wstrToStr(lastBootTime);
                }
                
                VariantClear(&vtProp);
            } else {
                info.lastBootTime = "Unknown";
            }
            
            // Get Windows directory
            if (SUCCEEDED(osResults[0]->Get(L"WindowsDirectory", 0, &vtProp, 0, 0))) {
                info.windowsDirectory = wstrToStr(vtProp.bstrVal);
                VariantClear(&vtProp);
            } else {
                info.windowsDirectory = "Unknown";
            }
            
            // Get system directory
            if (SUCCEEDED(osResults[0]->Get(L"SystemDirectory", 0, &vtProp, 0, 0))) {
                info.systemDirectory = wstrToStr(vtProp.bstrVal);
                VariantClear(&vtProp);
            } else {
                info.systemDirectory = "Unknown";
            }
            
            // Get system drive
            if (SUCCEEDED(osResults[0]->Get(L"SystemDrive", 0, &vtProp, 0, 0))) {
                info.systemDrive = wstrToStr(vtProp.bstrVal);
                VariantClear(&vtProp);
            } else {
                info.systemDrive = "C:";
            }
            
            // Get country code
            if (SUCCEEDED(osResults[0]->Get(L"CountryCode", 0, &vtProp, 0, 0))) {
                info.countryCode = wstrToStr(vtProp.bstrVal);
                VariantClear(&vtProp);
            } else {
                info.countryCode = "Unknown";
            }
            
            // Get locale
            if (SUCCEEDED(osResults[0]->Get(L"Locale", 0, &vtProp, 0, 0))) {
                info.language = wstrToStr(vtProp.bstrVal);
                VariantClear(&vtProp);
            } else {
                info.language = "Unknown";
            }
            
            osResults[0]->Release();
        }
        
        // Get timezone
        auto timezoneResults = executeWMIQuery(pSvc, L"SELECT * FROM Win32_TimeZone");
        
        if (!timezoneResults.empty()) {
            VARIANT vtProp;
            VariantInit(&vtProp);
            
            if (SUCCEEDED(timezoneResults[0]->Get(L"Caption", 0, &vtProp, 0, 0))) {
                info.timeZone = wstrToStr(vtProp.bstrVal);
                VariantClear(&vtProp);
            } else {
                info.timeZone = "Unknown";
            }
            
            timezoneResults[0]->Release();
        }
        
        cleanupWMI(pSvc);
    }
    
    // Calculate uptime
    if (info.lastBootTime != "Unknown") {
        ULARGE_INTEGER now;
        FILETIME nowFt;
        GetSystemTimeAsFileTime(&nowFt);
        
        now.LowPart = nowFt.dwLowDateTime;
        now.HighPart = nowFt.dwHighDateTime;
        
        // Convert last boot time to file time
        SYSTEMTIME stBoot = {0};
        if (info.lastBootTime.length() >= 19) {
            stBoot.wYear = std::stoi(info.lastBootTime.substr(0, 4));
            stBoot.wMonth = std::stoi(info.lastBootTime.substr(5, 2));
            stBoot.wDay = std::stoi(info.lastBootTime.substr(8, 2));
            stBoot.wHour = std::stoi(info.lastBootTime.substr(11, 2));
            stBoot.wMinute = std::stoi(info.lastBootTime.substr(14, 2));
            stBoot.wSecond = std::stoi(info.lastBootTime.substr(17, 2));
        }
        
        FILETIME ftBoot;
        if (SystemTimeToFileTime(&stBoot, &ftBoot)) {
            ULARGE_INTEGER boot;
            boot.LowPart = ftBoot.dwLowDateTime;
            boot.HighPart = ftBoot.dwHighDateTime;
            
            // Calculate difference in 100-nanosecond intervals
            ULARGE_INTEGER diff;
            diff.QuadPart = now.QuadPart - boot.QuadPart;
            
            // Convert to seconds (1 second = 10,000,000 intervals)
            info.uptime = diff.QuadPart / 10000000ULL;
            info.uptimeFormatted = formatUptime(info.uptime);
        } else {
            info.uptime = 0;
            info.uptimeFormatted = "Unknown";
        }
    } else {
        info.uptime = 0;
        info.uptimeFormatted = "Unknown";
    }
    
    // Check if running as admin
    info.isAdmin = isRunningAsAdmin();
    
    // Check if running in a virtual machine
    info.isVirtualMachine = isVirtualMachine();
    
    // Check if device is in tablet mode
    info.isTabletMode = isTabletMode();
    
    // Check if session is a remote session
    info.isRemoteSession = isRemoteSession();
    
    // Get license status
    info.licenseStatus = "Unknown";
    
    // Get activation status
    info.activationStatus = getActivationStatus();
    
    return info;
}

// Get OS information as JSON string
char* getOsInfoJson() {
    OSInfo info = getOSDetails();
    
    json osJson = {
        {"name", info.name},
        {"version", info.version},
        {"buildNumber", info.buildNumber},
        {"edition", info.edition},
        {"architecture", info.architecture},
        {"hostname", info.hostname},
        {"username", info.username},
        {"domain", info.domain},
        {"installDate", info.installDate},
        {"lastBootTime", info.lastBootTime},
        {"uptime", info.uptime},
        {"uptimeFormatted", info.uptimeFormatted},
        {"windowsDirectory", info.windowsDirectory},
        {"systemDirectory", info.systemDirectory},
        {"systemDrive", info.systemDrive},
        {"countryCode", info.countryCode},
        {"language", info.language},
        {"timeZone", info.timeZone},
        {"isAdmin", info.isAdmin},
        {"isVirtualMachine", info.isVirtualMachine},
        {"isTabletMode", info.isTabletMode},
        {"isRemoteSession", info.isRemoteSession},
        {"licenseStatus", info.licenseStatus},
        {"activationStatus", info.activationStatus}
    };
    
    return strdup_cstr(osJson.dump());
}