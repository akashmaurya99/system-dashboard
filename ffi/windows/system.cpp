#include <iostream>
#include <windows.h>
#include <tchar.h>
#include <string>
#include <vector>
#include <locale>
#include <codecvt>
#include <wbemidl.h>  // For WMI (CPU Temp, GPU, Fan Speed)
#pragma comment(lib, "wbemuuid.lib")  // Link WMI library

using namespace std;

bool running = true;

// Function to get CPU Model
string GetCPUModel() {
    HKEY hKey;
    WCHAR cpuModel[256] = {0};
    DWORD bufSize = sizeof(cpuModel);
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        RegQueryValueExW(hKey, L"ProcessorNameString", NULL, NULL, (LPBYTE)cpuModel, &bufSize);
        RegCloseKey(hKey);
    }
    wstring_convert<codecvt_utf8<wchar_t>> converter;
    return converter.to_bytes(cpuModel);
}

// Function to get CPU Core Count
int GetCPUCores() {
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    return sysInfo.dwNumberOfProcessors;
}

// Function to get CPU Usage (%)
double GetCPUUsage() {
    FILETIME idleTime, kernelTime, userTime;
    if (GetSystemTimes(&idleTime, &kernelTime, &userTime)) {
        static ULARGE_INTEGER lastIdle, lastKernel, lastUser;
        ULARGE_INTEGER currIdle, currKernel, currUser;
        currIdle.LowPart = idleTime.dwLowDateTime;
        currIdle.HighPart = idleTime.dwHighDateTime;
        currKernel.LowPart = kernelTime.dwLowDateTime;
        currKernel.HighPart = kernelTime.dwHighDateTime;
        currUser.LowPart = userTime.dwLowDateTime;
        currUser.HighPart = userTime.dwHighDateTime;

        ULONGLONG idleDiff = currIdle.QuadPart - lastIdle.QuadPart;
        ULONGLONG kernelDiff = currKernel.QuadPart - lastKernel.QuadPart;
        ULONGLONG userDiff = currUser.QuadPart - lastUser.QuadPart;
        ULONGLONG total = kernelDiff + userDiff;

        lastIdle = currIdle;
        lastKernel = currKernel;
        lastUser = currUser;

        return total ? (100.0 - (idleDiff * 100.0 / total)) : 0.0;
    }
    return 0.0;
}

// Function to get RAM Usage
void GetRAMUsage(MEMORYSTATUSEX &statex) {
    statex.dwLength = sizeof(statex);
    GlobalMemoryStatusEx(&statex);
}

// Function to get Disk Space
void GetDiskUsage(const wchar_t* drive, ULARGE_INTEGER &freeBytes, ULARGE_INTEGER &totalBytes) {
    GetDiskFreeSpaceExW(drive, &freeBytes, &totalBytes, NULL);
}

// Function to get Battery Status
void GetBatteryStatus(SYSTEM_POWER_STATUS &batteryStatus) {
    GetSystemPowerStatus(&batteryStatus);
}

// Function to get CPU Temperature, GPU Model, VRAM, and Fan Speed using WMI
string GetWMIData(const wstring &query, const wstring &property) {
    CoInitializeEx(0, COINIT_MULTITHREADED);
    IWbemLocator *locator = NULL;
    IWbemServices *services = NULL;
    CoCreateInstance(CLSID_WbemLocator, NULL, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID *)&locator);
    locator->ConnectServer(BSTR(L"ROOT\\CIMV2"), NULL, NULL, NULL, 0, NULL, NULL, &services);
    CoSetProxyBlanket(services, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);
    IEnumWbemClassObject *enumerator = NULL;
    services->ExecQuery(BSTR(L"WQL"), BSTR(query.c_str()), WBEM_FLAG_FORWARD_ONLY, NULL, &enumerator);
    IWbemClassObject *classObject = NULL;
    ULONG ret = 0;
    string result = "Unknown";
    while (enumerator && enumerator->Next(WBEM_INFINITE, 1, &classObject, &ret) == S_OK) {
        VARIANT value;
        classObject->Get(property.c_str(), 0, &value, 0, 0);
        if (value.vt == VT_BSTR) {
            wstring_convert<codecvt_utf8<wchar_t>> converter;
            result = converter.to_bytes(value.bstrVal);
        } else if (value.vt == VT_I4) {
            result = to_string(value.intVal);
        }
        VariantClear(&value);
        classObject->Release();
    }
    if (enumerator) enumerator->Release();
    if (services) services->Release();
    if (locator) locator->Release();
    CoUninitialize();
    return result;
}

int main() {
    while (running) {
        // RAM Usage
        MEMORYSTATUSEX memInfo;
        GetRAMUsage(memInfo);
        
        // CPU Info
        string cpuModel = GetCPUModel();
        int cpuCores = GetCPUCores();
        double cpuUsage = GetCPUUsage();

        // Disk Usage
        ULARGE_INTEGER freeBytesC, totalBytesC;
        ULARGE_INTEGER freeBytesD, totalBytesD;
        GetDiskUsage(L"C:\\", freeBytesC, totalBytesC);
        GetDiskUsage(L"D:\\", freeBytesD, totalBytesD);

        // Battery Status
        SYSTEM_POWER_STATUS batteryStatus;
        GetBatteryStatus(batteryStatus);

        // GPU & Fan Info using WMI
        string cpuTemperature = GetWMIData(L"SELECT Temperature FROM Win32_Processor", L"Temperature");
        string gpuModel = GetWMIData(L"SELECT Name FROM Win32_VideoController", L"Name");
        string gpuVRAM = GetWMIData(L"SELECT AdapterRAM FROM Win32_VideoController", L"AdapterRAM");
        string gpuTemperature = GetWMIData(L"SELECT CurrentTemperature FROM Win32_TemperatureProbe", L"CurrentTemperature");
        string fanSpeed = GetWMIData(L"SELECT DesiredSpeed FROM Win32_Fan", L"DesiredSpeed");

        // Display System Info
        system("cls");
        cout << "=== System Monitor ===" << endl;
        cout << "CPU Model: " << cpuModel << endl;
        cout << "CPU Cores: " << cpuCores << endl;
        cout << "CPU Usage: " << cpuUsage << " %" << endl;
        cout << "CPU Temperature: " << cpuTemperature << " °C" << endl;
        cout << "RAM Size: " << memInfo.ullTotalPhys / (1024 * 1024) << " MB" << endl;
        cout << "RAM Usage: " << (memInfo.ullTotalPhys - memInfo.ullAvailPhys) / (1024 * 1024) << " MB / "
             << memInfo.ullTotalPhys / (1024 * 1024) << " MB" << endl;
        cout << "Disk Usage (C:): " << (totalBytesC.QuadPart - freeBytesC.QuadPart) / (1024 * 1024 * 1024)
             << " GB free / " << totalBytesC.QuadPart / (1024 * 1024 * 1024) << " GB total" << endl;
        cout << "GPU Model: " << gpuModel << endl;
        cout << "GPU VRAM: " << gpuVRAM << " bytes" << endl;
        cout << "GPU Temperature: " << gpuTemperature << " °C" << endl;
        cout << "Fan Speed: " << fanSpeed << " RPM" << endl;
        cout << "Battery Status: " << (batteryStatus.BatteryLifePercent == 255 ? "Unknown" : to_string(batteryStatus.BatteryLifePercent) + "%") << endl;
        cout << "Charging: " << (batteryStatus.ACLineStatus == 1 ? "Yes" : "No") << endl;
        cout << "Press 1 to exit..." << endl;

        if (GetAsyncKeyState('1') & 0x8000) {
            running = false;
        }
        Sleep(5000);
    }
    return 0;
}
