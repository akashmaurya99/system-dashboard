#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <winsock2.h>
#include <iphlpapi.h>
#include <windows.h>
#include <Wlanapi.h>

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "Wlanapi.lib")

using namespace std;

// Structure to hold network information
struct NetworkInfo {
    string hostname;
    string localIP;
    string publicIP;
    vector<string> macAddresses;
    vector<string> interfaces;
    string wifiSSID;
    int wifiSignal;
};

// Helper: Execute command and return output
static string execCommand(const char* cmd) {
    char buffer[256];
    string result;
    FILE* pipe = _popen(cmd, "r");
    if (!pipe) return "ERROR";
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }
    _pclose(pipe);
    return result;
}

// Helper: Get formatted MAC address
static string formatMAC(BYTE* addr, DWORD length) {
    stringstream ss;
    for (DWORD i = 0; i < length; i++) {
        ss << hex << setw(2) << setfill('0') << (int)addr[i];
        if (i != length - 1) ss << ":";
    }
    return ss.str();
}

static void getHostname(NetworkInfo& info) {
    char buffer[256];
    if (gethostname(buffer, sizeof(buffer)) == 0) {
        info.hostname = buffer;
    }
}

static void getLocalIP(NetworkInfo& info) {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    
    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) {
        WSACleanup();
        return;
    }

    hostent* host = gethostbyname(hostname);
    if (host && host->h_addr_list[0]) {
        info.localIP = inet_ntoa(*(in_addr*)host->h_addr_list[0]);
    }
    WSACleanup();
}

static void getPublicIP(NetworkInfo& info) {
    string ip = execCommand("curl -s ifconfig.me");
    if (!ip.empty() && ip.find("ERROR") == string::npos) {
        info.publicIP = ip;
    }
}

static void getMACAddresses(NetworkInfo& info) {
    IP_ADAPTER_INFO adapters[16];
    DWORD size = sizeof(adapters);
    if (GetAdaptersInfo(adapters, &size) == ERROR_SUCCESS) {
        PIP_ADAPTER_INFO adapter = adapters;
        while (adapter) {
            info.macAddresses.push_back(formatMAC(adapter->Address, adapter->AddressLength));
            adapter = adapter->Next;
        }
    }
}

static void getNetworkInterfaces(NetworkInfo& info) {
    IP_ADAPTER_INFO adapters[16];
    DWORD size = sizeof(adapters);
    if (GetAdaptersInfo(adapters, &size) == ERROR_SUCCESS) {
        PIP_ADAPTER_INFO adapter = adapters;
        while (adapter) {
            string desc = adapter->Description;
            string ip = adapter->IpAddressList.IpAddress.String;
            info.interfaces.push_back(desc + " (" + ip + ")");
            adapter = adapter->Next;
        }
    }
}

static void getWifiInfo(NetworkInfo& info) {
    HANDLE hClient = NULL;
    DWORD version = 0;
    if (WlanOpenHandle(2, NULL, &version, &hClient) != ERROR_SUCCESS) return;

    PWLAN_INTERFACE_INFO_LIST ifList = NULL;
    if (WlanEnumInterfaces(hClient, NULL, &ifList) == ERROR_SUCCESS && ifList->dwNumberOfItems > 0) {
        PWLAN_AVAILABLE_NETWORK_LIST netList = NULL;
        GUID guid = ifList->InterfaceInfo[0].InterfaceGuid;
        if (WlanGetAvailableNetworkList(hClient, &guid, 0, NULL, &netList) == ERROR_SUCCESS) {
            if (netList->dwNumberOfItems > 0) {
                info.wifiSSID = (char*)netList->Network[0].dot11Ssid.ucSSID;
                info.wifiSignal = netList->Network[0].wlanSignalQuality;
            }
            WlanFreeMemory(netList);
        }
        WlanFreeMemory(ifList);
    }
    WlanCloseHandle(hClient, NULL);
}

// Convert NetworkInfo to JSON format
static string networkInfoToJSON(const NetworkInfo& info) {
    stringstream json;
    json << "{\n";
    json << "  \"hostname\": \"" << info.hostname << "\",\n";
    json << "  \"local_ip\": \"" << info.localIP << "\",\n";
    json << "  \"public_ip\": \"" << info.publicIP << "\",\n";
    
    json << "  \"mac_addresses\": [";
    for (size_t i = 0; i < info.macAddresses.size(); i++) {
        json << "\"" << info.macAddresses[i] << "\"";
        if (i < info.macAddresses.size() - 1) json << ", ";
    }
    json << "],\n";
    
    json << "  \"interfaces\": [";
    for (size_t i = 0; i < info.interfaces.size(); i++) {
        json << "\"" << info.interfaces[i] << "\"";
        if (i < info.interfaces.size() - 1) json << ", ";
    }
    json << "],\n";
    
    json << "  \"wifi\": {\n";
    json << "    \"ssid\": \"" << info.wifiSSID << "\",\n";
    json << "    \"signal_strength\": " << info.wifiSignal << "\n";
    json << "  }\n";
    json << "}";
    
    return json.str();
}

int main() {
    NetworkInfo info;
    
    getHostname(info);
    getLocalIP(info);
    getPublicIP(info);
    getMACAddresses(info);
    getNetworkInterfaces(info);
    getWifiInfo(info);
    
    cout << networkInfoToJSON(info) << endl;
    return 0;
}
