#include <iostream>
#include <windows.h>
#include <comdef.h>
#include <wbemidl.h>
#include <iomanip>
#include <sstream>
#include <mutex>
#include <vector>       // Added
#include <string>       
#pragma comment(lib, "wbemuuid.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "oleaut32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "advapi32.lib")

using namespace std;

// Structure to hold all system information
struct SystemInfo {
    // Processor
    string cpuName;
    int cpuCores;
    float cpuUsage;
    float cpuTemp;

    // Memory
    uint64_t totalRAM;
    uint64_t usedRAM;

    // Storage
    uint64_t diskTotal;
    uint64_t diskUsed;

    // GPU
    string gpuName;
    uint64_t gpuVRAM;
    float gpuTemp;

    // Power
    int batteryLevel;
    bool isCharging;
};

// Helper function for WMI initialization
static bool InitWMI(IWbemServices*& pSvc) {
    IWbemLocator* pLoc = nullptr;
    if (FAILED(CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, 
                              IID_IWbemLocator, (LPVOID*)&pLoc))) {
        return false;
    }

    if (FAILED(pLoc->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), NULL, NULL, 0, 0, NULL, 0, &pSvc))) {
        pLoc->Release();
        return false;
    }

    CoSetProxyBlanket(pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL,
                     RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);
    pLoc->Release();
    return true;
}

// Helper: Convert wide string to UTF-8
static string WideToUTF8(const wchar_t* wideStr) {
    if (!wideStr) return "";
    int length = WideCharToMultiByte(CP_UTF8, 0, wideStr, -1, NULL, 0, NULL, NULL);
    vector<char> buffer(length);
    WideCharToMultiByte(CP_UTF8, 0, wideStr, -1, buffer.data(), length, NULL, NULL);
    return string(buffer.data());
}

// Get WMI property value
template<typename T>
static T GetWMIValue(const wstring& query, const wstring& property) {
    T result{};
    IWbemServices* pSvc = nullptr;
    if (!InitWMI(pSvc)) return result;

    IEnumWbemClassObject* enumerator = nullptr;
    pSvc->ExecQuery(bstr_t(L"WQL"), bstr_t(query.c_str()), 
                   WBEM_FLAG_FORWARD_ONLY, nullptr, &enumerator);

    IWbemClassObject* clsObj = nullptr;
    ULONG ret = 0;
    
    if (enumerator && SUCCEEDED(enumerator->Next(WBEM_INFINITE, 1, &clsObj, &ret))) {
        VARIANT vtProp;
        VariantInit(&vtProp);
        
        if (SUCCEEDED(clsObj->Get(property.c_str(), 0, &vtProp, 0, 0))) {
            if constexpr (is_same_v<T, string>) {
                if (vtProp.vt == VT_BSTR) {
                    result = WideToUTF8(vtProp.bstrVal);
                }
            } else {
                result = static_cast<T>(vtProp.uintVal);
            }
            VariantClear(&vtProp);
        }
        clsObj->Release();
    }
    
    if (enumerator) enumerator->Release();
    pSvc->Release();
    return result;
}

// Get CPU metrics
static void GetCPUMetrics(SystemInfo& info) {
    static ULARGE_INTEGER lastIdle{}, lastKernel{}, lastUser{};
    FILETIME idle, kernel, user;
    
    if (GetSystemTimes(&idle, &kernel, &user)) {
        ULARGE_INTEGER currentIdle{idle.dwLowDateTime, idle.dwHighDateTime};
        ULARGE_INTEGER currentKernel{kernel.dwLowDateTime, kernel.dwHighDateTime};
        ULARGE_INTEGER currentUser{user.dwLowDateTime, user.dwHighDateTime};

        uint64_t idleDiff = currentIdle.QuadPart - lastIdle.QuadPart;
        uint64_t total = (currentKernel.QuadPart + currentUser.QuadPart) - 
                        (lastKernel.QuadPart + lastUser.QuadPart);

        info.cpuUsage = total > 0 ? (100.0f - (idleDiff * 100.0f / total)) : 0.0f;
        lastIdle = currentIdle;
        lastKernel = currentKernel;
        lastUser = currentUser;
    }
}

// Get memory metrics
static void GetMemoryMetrics(SystemInfo& info) {
    MEMORYSTATUSEX mem{sizeof(MEMORYSTATUSEX)};
    if (GlobalMemoryStatusEx(&mem)) {
        info.totalRAM = mem.ullTotalPhys >> 20; // MB
        info.usedRAM = (mem.ullTotalPhys - mem.ullAvailPhys) >> 20;
    }
}

// Get storage metrics
static void GetStorageMetrics(SystemInfo& info) {
    ULARGE_INTEGER free, total;
    if (GetDiskFreeSpaceExW(L"C:\\", nullptr, &total, &free)) {
        info.diskTotal = total.QuadPart >> 30; // GB
        info.diskUsed = (total.QuadPart - free.QuadPart) >> 30;
    }
}

// Get power metrics
static void GetPowerMetrics(SystemInfo& info) {
    SYSTEM_POWER_STATUS status;
    if (GetSystemPowerStatus(&status)) {
        info.batteryLevel = status.BatteryLifePercent == 255 ? -1 : status.BatteryLifePercent;
        info.isCharging = status.ACLineStatus == 1;
    }
}

// Convert SystemInfo to JSON
static string SystemInfoToJSON(const SystemInfo& info) {
    stringstream json;
    json << "{\n";
    
    // CPU Section
    json << "  \"cpu\": {\n"
         << "    \"name\": \"" << info.cpuName << "\",\n"
         << "    \"cores\": " << info.cpuCores << ",\n"
         << "    \"usage\": " << fixed << setprecision(1) << info.cpuUsage << ",\n"
         << "    \"temperature\": " << info.cpuTemp << "\n"
         << "  },\n";
    
    // Memory Section
    json << "  \"memory\": {\n"
         << "    \"total_mb\": " << info.totalRAM << ",\n"
         << "    \"used_mb\": " << info.usedRAM << "\n"
         << "  },\n";
    
    // Storage Section
    json << "  \"storage\": {\n"
         << "    \"total_gb\": " << info.diskTotal << ",\n"
         << "    \"used_gb\": " << info.diskUsed << "\n"
         << "  },\n";
    
    // GPU Section
    json << "  \"gpu\": {\n"
         << "    \"name\": \"" << info.gpuName << "\",\n"
         << "    \"vram_mb\": " << (info.gpuVRAM >> 20) << ",\n"
         << "    \"temperature\": " << info.gpuTemp << "\n"
         << "  },\n";
    
    // Power Section
    json << "  \"power\": {\n"
         << "    \"battery_level\": " << info.batteryLevel << ",\n"
         << "    \"is_charging\": " << (info.isCharging ? "true" : "false") << "\n"
         << "  }\n";
    
    json << "}";
    return json.str();
}

// Main entry point
extern "C" __attribute__((visibility("default")))
char* GetSystemMetricsJSON() {
    CoInitializeEx(0, COINIT_MULTITHREADED);
    
    SystemInfo info;
    
    // Collect data
    info.cpuName = GetWMIValue<string>(L"SELECT Name FROM Win32_Processor", L"Name");
    info.cpuCores = GetWMIValue<int>(L"SELECT NumberOfCores FROM Win32_Processor", L"NumberOfCores");
    info.cpuTemp = GetWMIValue<float>(L"SELECT Temperature FROM Win32_Processor", L"Temperature") / 10.0f;
    
    GetCPUMetrics(info);
    GetMemoryMetrics(info);
    GetStorageMetrics(info);
    GetPowerMetrics(info);
    
    info.gpuName = GetWMIValue<string>(L"SELECT Name FROM Win32_VideoController", L"Name");
    info.gpuVRAM = GetWMIValue<uint64_t>(L"SELECT AdapterRAM FROM Win32_VideoController", L"AdapterRAM");
    info.gpuTemp = GetWMIValue<float>(L"SELECT CurrentTemperature FROM Win32_TemperatureProbe", L"CurrentTemperature");
    
    CoUninitialize();

    string json = SystemInfoToJSON(info);
    char* result = new char[json.size() + 1];
    strcpy(result, json.c_str());
    return result;
}

// For testing
int main() {
    char* json = GetSystemMetricsJSON();
    cout << json << endl;
    delete[] json;
    return 0;
}
