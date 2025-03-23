#include <iostream>
#include <windows.h>
#include <fileapi.h>
#include <vector>
#include <sstream>
#include <iomanip>
#include <cstdint>  // Required for uint64_t

using namespace std;

// Structure to hold storage information
struct StorageInfo {
    struct DriveInfo {
        string letter;
        string fsType;
        uint64_t totalSpaceGB;
        uint64_t freeSpaceGB;
    };

    struct USBDevice {
        string driveLetter;
    };

    struct FilePermissions {
        string path;
        bool readOnly;
        bool hidden;
        bool system;
    };

    vector<DriveInfo> drives;
    vector<USBDevice> usbDevices;
    vector<string> hiddenFiles;
    FilePermissions sampleFilePerms;
};

// Helper: Convert bytes to GB
static uint64_t bytesToGB(ULARGE_INTEGER bytes) {
    return bytes.QuadPart / (1024 * 1024 * 1024);
}

// Helper: Get file system information
static void getDriveInfo(StorageInfo& info) {
    DWORD drives = GetLogicalDrives();
    for (char letter = 'A'; letter <= 'Z'; ++letter) {
        if (drives & (1 << (letter - 'A'))) {
            StorageInfo::DriveInfo drive;
            drive.letter = string(1, letter) + ":\\";
            
            // Get file system type
            char fsName[MAX_PATH] = {0};
            if (GetVolumeInformationA(drive.letter.c_str(), NULL, 0, NULL, NULL, NULL, fsName, MAX_PATH)) {
                drive.fsType = fsName;
            }

            // Get space information
            ULARGE_INTEGER freeBytes, totalBytes;
            if (GetDiskFreeSpaceExA(drive.letter.c_str(), &freeBytes, &totalBytes, NULL)) {
                drive.totalSpaceGB = bytesToGB(totalBytes);
                drive.freeSpaceGB = bytesToGB(freeBytes);
            }

            info.drives.push_back(drive);
        }
    }
}

// Helper: Detect USB devices
static void getUSBDevices(StorageInfo& info) {
    DWORD drives = GetLogicalDrives();
    for (char letter = 'A'; letter <= 'Z'; ++letter) {
        if (drives & (1 << (letter - 'A'))) {
            string drive = string(1, letter) + ":\\";
            if (GetDriveTypeA(drive.c_str()) == DRIVE_REMOVABLE) {
                info.usbDevices.push_back({drive});
            }
        }
    }
}

// Helper: Check file permissions
static void checkFilePermissions(StorageInfo& info, const string& path) {
    DWORD attrs = GetFileAttributesA(path.c_str());
    if (attrs != INVALID_FILE_ATTRIBUTES) {
        info.sampleFilePerms = {
            path,
            (attrs & FILE_ATTRIBUTE_READONLY) != 0,
            (attrs & FILE_ATTRIBUTE_HIDDEN) != 0,
            (attrs & FILE_ATTRIBUTE_SYSTEM) != 0
        };
    }
}

// Helper: Find hidden files
static void findHiddenFiles(StorageInfo& info, const string& path) {
    WIN32_FIND_DATAA findData;
    HANDLE hFind = FindFirstFileA((path + "\\*").c_str(), &findData);
    
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (findData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) {
                info.hiddenFiles.push_back(findData.cFileName);
            }
        } while (FindNextFileA(hFind, &findData));
        FindClose(hFind);
    }
}

// Convert StorageInfo to JSON
static string storageInfoToJSON(const StorageInfo& info) {
    stringstream json;
    json << "{\n";
    
    // Drives
    json << "  \"drives\": [\n";
    for (size_t i = 0; i < info.drives.size(); i++) {
        const auto& d = info.drives[i];
        json << "    {\n"
             << "      \"letter\": \"" << d.letter << "\",\n"
             << "      \"fs_type\": \"" << d.fsType << "\",\n"
             << "      \"total_gb\": " << d.totalSpaceGB << ",\n"
             << "      \"free_gb\": " << d.freeSpaceGB << "\n"
             << "    }" << (i < info.drives.size()-1 ? "," : "") << "\n";
    }
    json << "  ],\n";
    
    // USB Devices
    json << "  \"usb_devices\": [";
    for (size_t i = 0; i < info.usbDevices.size(); i++) {
        json << "\"" << info.usbDevices[i].driveLetter << "\"";
        if (i < info.usbDevices.size()-1) json << ", ";
    }
    json << "],\n";
    
    // File Permissions
    json << "  \"sample_file_permissions\": {\n"
         << "    \"path\": \"" << info.sampleFilePerms.path << "\",\n"
         << "    \"read_only\": " << boolalpha << info.sampleFilePerms.readOnly << ",\n"
         << "    \"hidden\": " << info.sampleFilePerms.hidden << ",\n"
         << "    \"system\": " << info.sampleFilePerms.system << "\n"
         << "  },\n";
    
    // Hidden Files
    json << "  \"hidden_files\": [";
    for (size_t i = 0; i < info.hiddenFiles.size(); i++) {
        json << "\"" << info.hiddenFiles[i] << "\"";
        if (i < info.hiddenFiles.size()-1) json << ", ";
    }
    json << "]\n";
    
    json << "}";
    return json.str();
}

int main() {
    StorageInfo info;
    
    getDriveInfo(info);
    getUSBDevices(info);
    checkFilePermissions(info, "C:\\Windows\\System32\\notepad.exe");
    findHiddenFiles(info, "C:\\Users");
    
    cout << storageInfoToJSON(info) << endl;
    return 0;
}
