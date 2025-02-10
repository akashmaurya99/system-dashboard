#include <iostream>
#include <windows.h>
#include <fileapi.h>
#include <vector>

using namespace std;

// Get File System Type
void getFileSystemType(const string &drive) {
    char fileSystemName[MAX_PATH] = {0};
    if (GetVolumeInformationA(drive.c_str(), NULL, 0, NULL, NULL, NULL, fileSystemName, MAX_PATH)) {
        cout << "File System Type for " << drive << ": " << fileSystemName << endl;
    } else {
        cout << "Failed to get file system type for " << drive << endl;
    }
}

// Get List of Mounted Drives
void getMountedDrives() {
    cout << "\nMounted Drives:\n";
    DWORD drives = GetLogicalDrives();
    for (char letter = 'A'; letter <= 'Z'; ++letter) {
        if (drives & (1 << (letter - 'A'))) {
            string drive = string(1, letter) + ":\\";
            cout << " - " << drive << endl;
            getFileSystemType(drive);
        }
    }
}

// Get External USB Devices
void getUSBDevices() {
    cout << "\nExternal USB Devices:\n";
    DWORD drives = GetLogicalDrives();
    for (char letter = 'A'; letter <= 'Z'; ++letter) {
        if (drives & (1 << (letter - 'A'))) {
            string drive = string(1, letter) + ":\\";
            UINT driveType = GetDriveTypeA(drive.c_str());
            if (driveType == DRIVE_REMOVABLE) {
                cout << " - " << drive << " (USB Drive)" << endl;
            }
        }
    }
}

// Get Total & Free Space Per Drive
void getDriveSpace() {
    cout << "\nDrive Space Information:\n";
    DWORD drives = GetLogicalDrives();
    for (char letter = 'A'; letter <= 'Z'; ++letter) {
        if (drives & (1 << (letter - 'A'))) {
            string drive = string(1, letter) + ":\\";
            ULARGE_INTEGER freeBytes, totalBytes, totalFreeBytes;
            if (GetDiskFreeSpaceExA(drive.c_str(), &freeBytes, &totalBytes, &totalFreeBytes)) {
                cout << "Drive: " << drive << " | Total: " << (totalBytes.QuadPart / (1024 * 1024 * 1024)) << " GB"
                     << " | Free: " << (freeBytes.QuadPart / (1024 * 1024 * 1024)) << " GB" << endl;
            }
        }
    }
}

// Get File Permissions & Hidden Files
void checkFilePermissions(const string &filePath) {
    DWORD attributes = GetFileAttributesA(filePath.c_str());
    if (attributes == INVALID_FILE_ATTRIBUTES) {
        cout << "Error accessing file: " << filePath << endl;
        return;
    }

    cout << "\nFile: " << filePath << endl;
    cout << "Permissions: ";
    if (attributes & FILE_ATTRIBUTE_READONLY) cout << "Read-Only ";
    if (attributes & FILE_ATTRIBUTE_HIDDEN) cout << "Hidden ";
    if (attributes & FILE_ATTRIBUTE_SYSTEM) cout << "System ";
    if (!(attributes & FILE_ATTRIBUTE_READONLY)) cout << "Writable ";
    cout << endl;
}

// Get Hidden Files & Directories in C:\Users
void listHiddenFiles(const string &directory) {
    cout << "\nHidden Files & Directories in " << directory << ":\n";
    WIN32_FIND_DATAA findFileData;
    HANDLE hFind = FindFirstFileA((directory + "\\*").c_str(), &findFileData);
    if (hFind == INVALID_HANDLE_VALUE) {
        cout << "No files found." << endl;
        return;
    }
    
    do {
        if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) {
            cout << " - " << findFileData.cFileName << endl;
        }
    } while (FindNextFileA(hFind, &findFileData));

    FindClose(hFind);
}

int main() {
    getMountedDrives();
    getUSBDevices();
    getDriveSpace();
    
    // Check permissions of a sample file (Change path accordingly)
    checkFilePermissions("C:\\Windows\\System32\\notepad.exe");

    // List hidden files in Users directory
    listHiddenFiles("C:\\Users");

    return 0;
}
