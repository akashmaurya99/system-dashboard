#include <iostream>
#include <windows.h>
#include <sysinfoapi.h>
#include <tlhelp32.h>
#include <vector>
#include <ctime>
#include <locale.h>

using namespace std;

void GetOSInfo() {
    OSVERSIONINFOEX osvi;
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    if (GetVersionEx((OSVERSIONINFO*)&osvi)) {
        cout << " Operating System Name & Version: Windows " 
             << osvi.dwMajorVersion << "." << osvi.dwMinorVersion 
             << " (Build " << osvi.dwBuildNumber << ")\n";
    }
}

void GetKernelVersion() {
    DWORD version = GetVersion();
    DWORD major = (DWORD)(LOBYTE(LOWORD(version)));
    DWORD minor = (DWORD)(HIBYTE(LOWORD(version)));
    cout << " OS Kernel Version: " << major << "." << minor << endl;
}

void GetSystemUptime() {
    DWORD uptime = GetTickCount64() / 1000;
    cout << " System Uptime: " << uptime / 3600 << " hours, " 
         << (uptime % 3600) / 60 << " minutes\n";
}

void GetTimeZone() {
    TIME_ZONE_INFORMATION tz;
    GetTimeZoneInformation(&tz);
    wcout << L" Timezone: " << tz.StandardName << endl;
}

void GetBootTime() {
    ULONGLONG uptime = GetTickCount64() / 1000;
    time_t currentTime;
    time(&currentTime);
    time_t bootTime = currentTime - uptime;
    cout << " System Boot Time: " << ctime(&bootTime);
}

void GetDefaultSystemLanguage() {
    WCHAR locale[LOCALE_NAME_MAX_LENGTH];
    if (GetUserDefaultLocaleName(locale, LOCALE_NAME_MAX_LENGTH)) {
        wcout << L" Default System Language: " << locale << endl;
    }
}

void ListRunningProcesses() {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        cerr << " Failed to retrieve process snapshot.\n";
        return;
    }

    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(PROCESSENTRY32);
    if (Process32First(hSnapshot, &pe)) {
        cout << " Running Processes:\n";
        do {
            wcout << L"   - " << pe.szExeFile << L" (PID: " << pe.th32ProcessID << L")\n";
        } while (Process32Next(hSnapshot, &pe));
    }
    CloseHandle(hSnapshot);
}

void ListUserAccounts() {
    cout << " List of User Accounts:\n";
    system("wmic useraccount get name");
}

int main() {
    cout << "==========================\n";
    cout << "    System Information  \n";
    cout << "==========================\n";

    GetOSInfo();
    GetKernelVersion();
    GetSystemUptime();
    GetTimeZone();
    GetBootTime();
    GetDefaultSystemLanguage();
    ListRunningProcesses();
    ListUserAccounts();

    return 0;
}
