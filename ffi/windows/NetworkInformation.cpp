#include <iostream>
#include <winsock2.h>
#include <iphlpapi.h>
#include <vector>
#include <windows.h>
#include <Wlanapi.h>
#include <fstream>

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "Wlanapi.lib")

using namespace std;

void GetHostName() {
    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) == 0) {
        cout << "Hostname: " << hostname << endl;
    } else {
        cerr << "Failed to get hostname" << endl;
    }
}

void GetLocalIPAddress() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    
    char hostname[256];
    gethostname(hostname, sizeof(hostname));
    
    struct hostent* host = gethostbyname(hostname);
    if (host) {
        cout << "Local IP Address: " << inet_ntoa(*((struct in_addr*)host->h_addr_list[0])) << endl;
    } else {
        cerr << "Failed to get local IP address" << endl;
    }

    WSACleanup();
}

void GetPublicIPAddress() {
    system("curl -s ifconfig.me > public_ip.txt");
    ifstream file("public_ip.txt");
    string public_ip;
    if (file >> public_ip) {
        cout << "Public IP Address: " << public_ip << endl;
    } else {
        cerr << "Failed to get public IP address" << endl;
    }
    file.close();
    system("del public_ip.txt");  // Delete the temp file
}

void GetMACAddress() {
    IP_ADAPTER_INFO adapterInfo[16];
    DWORD size = sizeof(adapterInfo);
    if (GetAdaptersInfo(adapterInfo, &size) == ERROR_SUCCESS) {
        PIP_ADAPTER_INFO adapter = adapterInfo;
        while (adapter) {
            cout << "MAC Address: ";
            for (UINT i = 0; i < adapter->AddressLength; i++) {
                printf("%02X", adapter->Address[i]);
                if (i < adapter->AddressLength - 1) cout << ":";
            }
            cout << endl;
            adapter = adapter->Next;
        }
    } else {
        cerr << "Failed to get MAC address" << endl;
    }
}

void GetActiveNetworkInterfaces() {
    IP_ADAPTER_INFO adapterInfo[16];
    DWORD size = sizeof(adapterInfo);
    if (GetAdaptersInfo(adapterInfo, &size) == ERROR_SUCCESS) {
        PIP_ADAPTER_INFO adapter = adapterInfo;
        cout << "Active Network Interfaces:" << endl;
        while (adapter) {
            cout << "  - " << adapter->Description << " (" << adapter->IpAddressList.IpAddress.String << ")" << endl;
            adapter = adapter->Next;
        }
    } else {
        cerr << "Failed to get active network interfaces" << endl;
    }
}

void GetWiFiSSID() {
    HANDLE hClient = NULL;
    DWORD dwMaxClient = 2, dwCurVersion = 0;
    DWORD dwResult = WlanOpenHandle(dwMaxClient, NULL, &dwCurVersion, &hClient);
    if (dwResult != ERROR_SUCCESS) {
        cerr << "Failed to open WLAN handle" << endl;
        return;
    }

    PWLAN_INTERFACE_INFO_LIST pIfList = NULL;
    if (WlanEnumInterfaces(hClient, NULL, &pIfList) != ERROR_SUCCESS) {
        cerr << "Failed to enumerate Wi-Fi interfaces" << endl;
        return;
    }

    if (pIfList->dwNumberOfItems == 0) {
        cout << "No Wi-Fi interfaces found" << endl;
        return;
    }

    PWLAN_AVAILABLE_NETWORK_LIST pNetworkList = NULL;
    if (WlanGetAvailableNetworkList(hClient, &pIfList->InterfaceInfo[0].InterfaceGuid, 0, NULL, &pNetworkList) != ERROR_SUCCESS) {
        cerr << "Failed to get Wi-Fi network list" << endl;
        return;
    }

    cout << "Wi-Fi SSID: " << pNetworkList->Network[0].dot11Ssid.ucSSID << endl;
    WlanFreeMemory(pNetworkList);
    WlanFreeMemory(pIfList);
    WlanCloseHandle(hClient, NULL);
}

void GetWiFiSignalStrength() {
    HANDLE hClient = NULL;
    DWORD dwMaxClient = 2, dwCurVersion = 0;
    DWORD dwResult = WlanOpenHandle(dwMaxClient, NULL, &dwCurVersion, &hClient);
    if (dwResult != ERROR_SUCCESS) {
        cerr << "Failed to open WLAN handle" << endl;
        return;
    }

    PWLAN_INTERFACE_INFO_LIST pIfList = NULL;
    if (WlanEnumInterfaces(hClient, NULL, &pIfList) != ERROR_SUCCESS) {
        cerr << "Failed to enumerate Wi-Fi interfaces" << endl;
        return;
    }

    if (pIfList->dwNumberOfItems == 0) {
        cout << "No Wi-Fi interfaces found" << endl;
        return;
    }

    PWLAN_AVAILABLE_NETWORK_LIST pNetworkList = NULL;
    if (WlanGetAvailableNetworkList(hClient, &pIfList->InterfaceInfo[0].InterfaceGuid, 0, NULL, &pNetworkList) != ERROR_SUCCESS) {
        cerr << "Failed to get Wi-Fi network list" << endl;
        return;
    }

    cout << "Wi-Fi Signal Strength: " << (int)pNetworkList->Network[0].wlanSignalQuality << "%" << endl;
    WlanFreeMemory(pNetworkList);
    WlanFreeMemory(pIfList);
    WlanCloseHandle(hClient, NULL);
}

int main() {
    cout << "==========================\n";
    cout << "   Network Information    \n";
    cout << "==========================\n";

    GetHostName();
    GetLocalIPAddress();
    GetPublicIPAddress();
    GetMACAddress();
    GetActiveNetworkInterfaces();
    GetWiFiSSID();
    GetWiFiSignalStrength();

    system("pause");
    return 0;
}
