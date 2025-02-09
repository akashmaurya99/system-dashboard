#include <windows.h>
#include <comdef.h>
#include <Wbemidl.h>
#include <cstring>
#include <memory>


#pragma comment(lib, "wbemuuid.lib")

extern "C" const char* wmi_query(const wchar_t* query, const wchar_t* property) {
    HRESULT hres;
    IWbemLocator* pLoc = nullptr;
    IWbemServices* pSvc = nullptr;
    IEnumWbemClassObject* pEnumerator = nullptr;
    char* result = nullptr;

    hres = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hres)) return nullptr;

    hres = CoInitializeSecurity(
        NULL, -1, NULL, NULL,
        RPC_C_AUTHN_LEVEL_DEFAULT,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL, EOAC_NONE, NULL
    );

    if (SUCCEEDED(hres)) {
        hres = CoCreateInstance(
            CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER,
            IID_IWbemLocator, (LPVOID*)&pLoc
        );
    }

    if (SUCCEEDED(hres)) {
        hres = pLoc->ConnectServer(
            _bstr_t(L"ROOT\\CIMV2"), NULL, NULL,
            0, NULL, 0, 0, &pSvc
        );
    }

    if (SUCCEEDED(hres)) {
        hres = CoSetProxyBlanket(
            pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE,
            NULL, RPC_C_AUTHN_LEVEL_CALL,
            RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE
        );
    }

    if (SUCCEEDED(hres)) {
        hres = pSvc->ExecQuery(
            bstr_t(L"WQL"), bstr_t(query),
            WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
            NULL, &pEnumerator
        );
    }

    if (SUCCEEDED(hres)) {
        IWbemClassObject* pclsObj = nullptr;
        ULONG uReturn = 0;

        while (pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn) == S_OK) {
            VARIANT vtProp;
            if (SUCCEEDED(pclsObj->Get(property, 0, &vtProp, 0, 0))) {
                _bstr_t bstrProp(vtProp.bstrVal);
                result = _strdup(static_cast<const char*>(bstrProp));
                VariantClear(&vtProp);
            }
            pclsObj->Release();
            break;
        }
    }

    if (pEnumerator) pEnumerator->Release();
    if (pSvc) pSvc->Release();
    if (pLoc) pLoc->Release();
    CoUninitialize();

    return result;
}

extern "C" {

const char* get_cpu_model() {
    return wmi_query(L"SELECT Name FROM Win32_Processor", L"Name");
}

int get_cpu_cores() {
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    return sysInfo.dwNumberOfProcessors;
}

unsigned long long get_ram_size() {
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    return GlobalMemoryStatusEx(&memInfo) ? memInfo.ullTotalPhys : 0;
}

struct DiskSpace {
    unsigned long long total;
    unsigned long long free;
};

DiskSpace get_disk_usage(const char* path) {
    DiskSpace space = {0, 0};
    ULARGE_INTEGER freeBytes, totalBytes;
    if (GetDiskFreeSpaceExA(path, nullptr, &totalBytes, &freeBytes)) {
        space.total = totalBytes.QuadPart;
        space.free = freeBytes.QuadPart;
    }
    return space;
}

const char* get_os_name() {
    return wmi_query(L"SELECT Caption FROM Win32_OperatingSystem", L"Caption");
}

const char* get_os_version() {
    return wmi_query(L"SELECT Version FROM Win32_OperatingSystem", L"Version");
}

} // extern "C"