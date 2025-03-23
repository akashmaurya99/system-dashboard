#include <windows.h>
#include <comdef.h>
#include <wbemidl.h>
#include <cstring>
#include <vector>
#include <string>
#include <iomanip>
#include <sstream>
#include <cstdint>
#include <iostream>

#pragma comment(lib, "wbemuuid.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "oleaut32.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "user32.lib")

using namespace std;

struct SystemInfo {
    string cpuName;
    int cpuCores;
    string osName;
    string osVersion;
    uint64_t totalRAM;
    uint64_t freeRAM;
    uint64_t diskTotal;
    uint64_t diskFree;
};

// Helper function to convert wide strings to UTF-8
string WideToUTF8(const wchar_t* wideStr) {
    if (!wideStr) return "";
    int size = WideCharToMultiByte(CP_UTF8, 0, wideStr, -1, nullptr, 0, nullptr, nullptr);
    vector<char> buffer(size);
    WideCharToMultiByte(CP_UTF8, 0, wideStr, -1, buffer.data(), size, nullptr, nullptr);
    return string(buffer.data());
}

// WMI Query for string values
string WMIQueryString(const wstring& query, const wstring& property) {
    HRESULT hres;
    IWbemLocator* pLoc = nullptr;
    IWbemServices* pSvc = nullptr;
    IEnumWbemClassObject* pEnumerator = nullptr;
    string result;

    hres = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hres)) return "Unknown";

    hres = CoCreateInstance(
        CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER,
        IID_IWbemLocator, (LPVOID*)&pLoc
    );

    if (SUCCEEDED(hres)) {
        hres = pLoc->ConnectServer(
            _bstr_t(L"ROOT\\CIMV2"), nullptr, nullptr,
            0, 0, 0, 0, &pSvc
        );
    }

    if (SUCCEEDED(hres)) {
        hres = CoSetProxyBlanket(
            pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE,
            nullptr, RPC_C_AUTHN_LEVEL_CALL,
            RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, EOAC_NONE
        );
    }

    if (SUCCEEDED(hres)) {
        hres = pSvc->ExecQuery(
            bstr_t(L"WQL"), bstr_t(query.c_str()),
            WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
            nullptr, &pEnumerator
        );
    }

    if (SUCCEEDED(hres)) {
        IWbemClassObject* pclsObj = nullptr;
        ULONG uReturn = 0;

        if (pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn) == S_OK) {
            VARIANT vtProp;
            VariantInit(&vtProp);

            if (SUCCEEDED(pclsObj->Get(property.c_str(), 0, &vtProp, 0, 0))) {
                if (vtProp.vt == VT_BSTR) {
                    result = WideToUTF8(vtProp.bstrVal);
                }
                VariantClear(&vtProp);
            }
            pclsObj->Release();
        }
    }

    if (pEnumerator) pEnumerator->Release();
    if (pSvc) pSvc->Release();
    if (pLoc) pLoc->Release();
    CoUninitialize();

    return result.empty() ? "Unknown" : result;
}

// WMI Query for numeric values
template<typename T>
T WMIQueryNumeric(const wstring& query, const wstring& property) {
    HRESULT hres;
    IWbemLocator* pLoc = nullptr;
    IWbemServices* pSvc = nullptr;
    IEnumWbemClassObject* pEnumerator = nullptr;
    T result = 0;

    hres = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hres)) return result;

    hres = CoCreateInstance(
        CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER,
        IID_IWbemLocator, (LPVOID*)&pLoc
    );

    if (SUCCEEDED(hres)) {
        hres = pLoc->ConnectServer(
            _bstr_t(L"ROOT\\CIMV2"), nullptr, nullptr,
            0, 0, 0, 0, &pSvc
        );
    }

    if (SUCCEEDED(hres)) {
        hres = CoSetProxyBlanket(
            pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE,
            nullptr, RPC_C_AUTHN_LEVEL_CALL,
            RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, EOAC_NONE
        );
    }

    if (SUCCEEDED(hres)) {
        hres = pSvc->ExecQuery(
            bstr_t(L"WQL"), bstr_t(query.c_str()),
            WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
            nullptr, &pEnumerator
        );
    }

    if (SUCCEEDED(hres)) {
        IWbemClassObject* pclsObj = nullptr;
        ULONG uReturn = 0;

        if (pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn) == S_OK) {
            VARIANT vtProp;
            VariantInit(&vtProp);

            if (SUCCEEDED(pclsObj->Get(property.c_str(), 0, &vtProp, 0, 0))) {
                if (vtProp.vt == VT_I4) {
                    result = static_cast<T>(vtProp.intVal);
                }
                VariantClear(&vtProp);
            }
            pclsObj->Release();
        }
    }

    if (pEnumerator) pEnumerator->Release();
    if (pSvc) pSvc->Release();
    if (pLoc) pLoc->Release();
    CoUninitialize();

    return result;
}

void GetCPUInfo(SystemInfo& info) {
    info.cpuName = WMIQueryString(L"SELECT Name FROM Win32_Processor", L"Name");
    info.cpuCores = WMIQueryNumeric<int>(L"SELECT NumberOfCores FROM Win32_Processor", L"NumberOfCores");
    
    // Fallback to GetSystemInfo if WMI fails
    if (info.cpuCores <= 0) {
        SYSTEM_INFO sysInfo;
        GetSystemInfo(&sysInfo);
        info.cpuCores = sysInfo.dwNumberOfProcessors;
    }
}

void GetOSInfo(SystemInfo& info) {
    info.osName = WMIQueryString(L"SELECT Caption FROM Win32_OperatingSystem", L"Caption");
    info.osVersion = WMIQueryString(L"SELECT Version FROM Win32_OperatingSystem", L"Version");
}

void GetMemoryInfo(SystemInfo& info) {
    MEMORYSTATUSEX memInfo = { sizeof(MEMORYSTATUSEX) };
    if (GlobalMemoryStatusEx(&memInfo)) {
        info.totalRAM = memInfo.ullTotalPhys / (1024 * 1024);
        info.freeRAM = memInfo.ullAvailPhys / (1024 * 1024);
    }
}

void GetDiskInfo(SystemInfo& info) {
    ULARGE_INTEGER freeBytes, totalBytes;
    if (GetDiskFreeSpaceExW(L"C:\\", nullptr, &totalBytes, &freeBytes)) {
        info.diskTotal = totalBytes.QuadPart / (1024 * 1024 * 1024);
        info.diskFree = freeBytes.QuadPart / (1024 * 1024 * 1024);
    }
}

string SystemInfoToJSON(const SystemInfo& info) {
    stringstream json;
    json << "{\n"
         << "  \"cpu\": {\n"
         << "    \"name\": \"" << info.cpuName << "\",\n"
         << "    \"cores\": " << info.cpuCores << "\n"
         << "  },\n"
         << "  \"os\": {\n"
         << "    \"name\": \"" << info.osName << "\",\n"
         << "    \"version\": \"" << info.osVersion << "\"\n"
         << "  },\n"
         << "  \"memory\": {\n"
         << "    \"total_mb\": " << info.totalRAM << ",\n"
         << "    \"free_mb\": " << info.freeRAM << "\n"
         << "  },\n"
         << "  \"disk\": {\n"
         << "    \"total_gb\": " << info.diskTotal << ",\n"
         << "    \"free_gb\": " << info.diskFree << "\n"
         << "  }\n"
         << "}";
    return json.str();
}

int main() {
    SystemInfo info;
    
    GetCPUInfo(info);
    GetOSInfo(info);
    GetMemoryInfo(info);
    GetDiskInfo(info);
    
    cout << SystemInfoToJSON(info) << endl;
    return 0;
}
