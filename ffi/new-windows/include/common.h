#pragma once

#include <string>
#include <vector>
#include <windows.h>
#include <comdef.h>
#include <Wbemidl.h>
#include <pdh.h>
#include <pdhmsg.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <nlohmann/json.hpp>

// Link with required Windows libraries
#pragma comment(lib, "wbemuuid.lib")
#pragma comment(lib, "pdh.lib")

using json = nlohmann::json;

// Initialize COM and WMI
inline bool initWMI(IWbemServices** pSvc) {
    // Initialize COM
    HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hr)) return false;

    // Initialize security
    hr = CoInitializeSecurity(
        NULL, -1, NULL, NULL, 
        RPC_C_AUTHN_LEVEL_DEFAULT, 
        RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL, EOAC_NONE, NULL);
    
    if (FAILED(hr)) {
        CoUninitialize();
        return false;
    }

    // Create WMI locator
    IWbemLocator* pLoc = NULL;
    hr = CoCreateInstance(
        CLSID_WbemLocator, 0, 
        CLSCTX_INPROC_SERVER, 
        IID_IWbemLocator, (LPVOID*)&pLoc);
    
    if (FAILED(hr)) {
        CoUninitialize();
        return false;
    }

    // Connect to WMI namespace
    hr = pLoc->ConnectServer(
        _bstr_t(L"ROOT\\CIMV2"), NULL, NULL, 0, 
        NULL, 0, 0, pSvc);
    
    pLoc->Release();
    
    if (FAILED(hr)) {
        CoUninitialize();
        return false;
    }

    // Set security levels
    hr = CoSetProxyBlanket(
        *pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL,
        RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL, EOAC_NONE);
    
    if (FAILED(hr)) {
        (*pSvc)->Release();
        CoUninitialize();
        return false;
    }

    return true;
}

// Clean up WMI resources
inline void cleanupWMI(IWbemServices* pSvc) {
    if (pSvc) pSvc->Release();
    CoUninitialize();
}

// Helper function to execute WMI queries
inline std::vector<IWbemClassObject*> executeWMIQuery(IWbemServices* pSvc, const std::wstring& query) {
    std::vector<IWbemClassObject*> results;
    
    IEnumWbemClassObject* pEnumerator = NULL;
    HRESULT hr = pSvc->ExecQuery(
        bstr_t("WQL"), 
        bstr_t(query.c_str()),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, 
        NULL,
        &pEnumerator);
    
    if (FAILED(hr)) return results;
    
    IWbemClassObject* pclsObj = NULL;
    ULONG uReturn = 0;
    
    while (pEnumerator) {
        hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
        
        if (0 == uReturn) break;
        
        results.push_back(pclsObj);
    }
    
    if (pEnumerator) pEnumerator->Release();
    
    return results;
}

// Helper to convert wide string to regular string
inline std::string wstrToStr(const std::wstring& wstr) {
    if (wstr.empty()) return std::string();
    
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &strTo[0], size_needed, NULL, NULL);
    
    // Remove terminating null character
    if (!strTo.empty() && strTo[strTo.size() - 1] == '\0') {
        strTo.resize(strTo.size() - 1);
    }
    
    return strTo;
}

// Initialize Performance Data Helper (PDH)
inline bool initPDH() {
    static bool initialized = false;
    
    if (!initialized) {
        HRESULT hr = PdhOpenQuery(NULL, 0, NULL);
        initialized = SUCCEEDED(hr);
    }
    
    return initialized;
}

// Helper for reading PDH counters
inline double readPDHCounter(const std::string& counterPath) {
    PDH_HQUERY query;
    PDH_HCOUNTER counter;
    PDH_FMT_COUNTERVALUE value;
    
    if (PdhOpenQuery(NULL, 0, &query) != ERROR_SUCCESS) {
        return 0.0;
    }
    
    if (PdhAddEnglishCounter(query, counterPath.c_str(), 0, &counter) != ERROR_SUCCESS) {
        PdhCloseQuery(query);
        return 0.0;
    }
    
    if (PdhCollectQueryData(query) != ERROR_SUCCESS) {
        PdhCloseQuery(query);
        return 0.0;
    }
    
    // Wait a moment for a second data point
    Sleep(100);
    
    if (PdhCollectQueryData(query) != ERROR_SUCCESS) {
        PdhCloseQuery(query);
        return 0.0;
    }
    
    if (PdhGetFormattedCounterValue(counter, PDH_FMT_DOUBLE, NULL, &value) != ERROR_SUCCESS) {
        PdhCloseQuery(query);
        return 0.0;
    }
    
    double result = value.doubleValue;
    PdhCloseQuery(query);
    
    return result;
}