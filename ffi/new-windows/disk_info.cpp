#include "include/disk_info.h"
#include <windows.h>
#include <fileapi.h>
#include <winioctl.h>
#include <setupapi.h>
#include <devguid.h>
#include <initguid.h>
#include <cfgmgr32.h>
#include <sstream>
#include <iomanip>

// Link required libraries
#pragma comment(lib, "setupapi.lib")
#pragma comment(lib, "cfgmgr32.lib")

// Get readable format for bytes
std::string formatBytes(uint64_t bytes) {
    const char* units[] = {"B", "KB", "MB", "GB", "TB", "PB"};
    int unitIndex = 0;
    double size = static_cast<double>(bytes);
    
    while (size >= 1024.0 && unitIndex < 5) {
        size /= 1024.0;
        unitIndex++;
    }
    
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << size << " " << units[unitIndex];
    return oss.str();
}

// Get partition information
std::vector<PartitionInfo> getPartitions() {
    std::vector<PartitionInfo> partitions;
    WCHAR drives[MAX_PATH] = {0};
    
    // Get logical drive strings
    if (GetLogicalDriveStringsW(MAX_PATH, drives) == 0) {
        return partitions;
    }
    
    // Iterate through each drive
    for (WCHAR* drive = drives; *drive != L'\0'; drive += wcslen(drive) + 1) {
        PartitionInfo partition;
        partition.mountPoint = wstrToStr(std::wstring(drive));
        
        // Get drive type
        UINT driveType = GetDriveTypeW(drive);
        if (driveType == DRIVE_UNKNOWN || driveType == DRIVE_NO_ROOT_DIR) {
            continue;
        }
        
        // Get volume name
        WCHAR volumeName[MAX_PATH] = {0};
        WCHAR fileSystemName[MAX_PATH] = {0};
        DWORD serialNumber = 0;
        DWORD maxComponentLength = 0;
        DWORD fileSystemFlags = 0;
        
        if (GetVolumeInformationW(drive, volumeName, MAX_PATH, &serialNumber, &maxComponentLength, 
                                &fileSystemFlags, fileSystemName, MAX_PATH)) {
            partition.name = wstrToStr(std::wstring(volumeName));
            partition.fileSystem = wstrToStr(std::wstring(fileSystemName));
        } else {
            partition.name = "Volume";
            partition.fileSystem = "Unknown";
        }
        
        // Get disk space information
        ULARGE_INTEGER freeBytesAvailable;
        ULARGE_INTEGER totalBytes;
        ULARGE_INTEGER totalFreeBytes;
        
        if (GetDiskFreeSpaceExW(drive, &freeBytesAvailable, &totalBytes, &totalFreeBytes)) {
            partition.totalSize = totalBytes.QuadPart;
            partition.freeSpace = totalFreeBytes.QuadPart;
            
            if (partition.totalSize > 0) {
                partition.usedPercentage = 100.0 * (1.0 - (static_cast<double>(partition.freeSpace) / 
                                                static_cast<double>(partition.totalSize)));
            } else {
                partition.usedPercentage = 0.0;
            }
        } else {
            partition.totalSize = 0;
            partition.freeSpace = 0;
            partition.usedPercentage = 0.0;
        }
        
        // Check if this is the system partition
        WCHAR systemDir[MAX_PATH] = {0};
        GetSystemDirectoryW(systemDir, MAX_PATH);
        systemDir[2] = L'\0'; // Truncate to drive letter + :\
        partition.isSystemPartition = (wcsicmp(systemDir, drive) == 0);
        
        // Check if this is the boot partition
        WCHAR bootDir[MAX_PATH] = {0};
        GetWindowsDirectoryW(bootDir, MAX_PATH);
        bootDir[2] = L'\0'; // Truncate to drive letter + :\
        partition.isBootPartition = (wcsicmp(bootDir, drive) == 0);
        
        partitions.push_back(partition);
    }
    
    return partitions;
}

// Get physical disk information
std::vector<PhysicalDiskInfo> getPhysicalDisks() {
    std::vector<PhysicalDiskInfo> disks;
    IWbemServices* pSvc = NULL;
    
    if (initWMI(&pSvc)) {
        // Query physical disks
        auto diskResults = executeWMIQuery(pSvc, L"SELECT * FROM Win32_DiskDrive");
        
        for (auto& obj : diskResults) {
            PhysicalDiskInfo disk;
            VARIANT vtProp;
            VariantInit(&vtProp);
            
            // Get disk model
            if (SUCCEEDED(obj->Get(L"Model", 0, &vtProp, 0, 0))) {
                disk.model = _bstr_t(vtProp.bstrVal);
                VariantClear(&vtProp);
            } else {
                disk.model = "Unknown Disk";
            }
            
            // Get serial number
            if (SUCCEEDED(obj->Get(L"SerialNumber", 0, &vtProp, 0, 0))) {
                disk.serialNumber = _bstr_t(vtProp.bstrVal);
                VariantClear(&vtProp);
            } else {
                disk.serialNumber = "Unknown";
            }
            
            // Get interface type
            if (SUCCEEDED(obj->Get(L"InterfaceType", 0, &vtProp, 0, 0))) {
                disk.interfaceType = _bstr_t(vtProp.bstrVal);
                VariantClear(&vtProp);
            } else {
                disk.interfaceType = "Unknown";
            }
            
            // Get media type (determine if SSD or HDD)
            disk.mediaType = "HDD"; // Default to HDD
            
            // Try to detect SSD using additional queries
            if (disk.model.find("SSD") != std::string::npos || 
                disk.model.find("Solid") != std::string::npos) {
                disk.mediaType = "SSD";
            }
            
            // Try to detect NVMe
            if (disk.interfaceType.find("NVMe") != std::string::npos || 
                disk.model.find("NVMe") != std::string::npos) {
                disk.mediaType = "NVMe SSD";
                disk.interfaceType = "NVMe";
            }
            
            // Get disk size
            if (SUCCEEDED(obj->Get(L"Size", 0, &vtProp, 0, 0))) {
                disk.size = _wtoi64(vtProp.bstrVal);
                VariantClear(&vtProp);
            } else {
                disk.size = 0;
            }
            
            // Get disk number
            if (SUCCEEDED(obj->Get(L"Index", 0, &vtProp, 0, 0))) {
                disk.diskNumber = vtProp.intVal;
                VariantClear(&vtProp);
            } else {
                disk.diskNumber = -1;
            }
            
            // Get removable status
            if (SUCCEEDED(obj->Get(L"MediaType", 0, &vtProp, 0, 0))) {
                std::string mediaType = _bstr_t(vtProp.bstrVal);
                disk.isRemovable = (mediaType.find("Removable") != std::string::npos);
                VariantClear(&vtProp);
            } else {
                disk.isRemovable = false;
            }
            
            // Get status
            if (SUCCEEDED(obj->Get(L"Status", 0, &vtProp, 0, 0))) {
                disk.status = _bstr_t(vtProp.bstrVal);
                VariantClear(&vtProp);
            } else {
                disk.status = "Unknown";
            }
            
            // Get SMART status (health) using Win32_DiskDrive class
            disk.health = "Unknown";
            
            // Get disk performance data
            disk.bytesRead = 0;
            disk.bytesWritten = 0;
            disk.readRate = 0.0;
            disk.writeRate = 0.0;
            disk.temperature = 0.0;
            
            // Get performance counters
            std::wstring diskInstance = L"PhysicalDisk(" + std::to_wstring(disk.diskNumber) + L")";
            std::wstring diskReadBytes = L"\\PhysicalDisk(" + std::to_wstring(disk.diskNumber) + L")\\Disk Read Bytes/sec";
            std::wstring diskWriteBytes = L"\\PhysicalDisk(" + std::to_wstring(disk.diskNumber) + L")\\Disk Write Bytes/sec";
            
            PDH_HQUERY query = NULL;
            PDH_HCOUNTER readCounter = NULL;
            PDH_HCOUNTER writeCounter = NULL;
            
            if (PdhOpenQuery(NULL, 0, &query) == ERROR_SUCCESS) {
                if (PdhAddEnglishCounter(query, diskReadBytes.c_str(), 0, &readCounter) == ERROR_SUCCESS &&
                    PdhAddEnglishCounter(query, diskWriteBytes.c_str(), 0, &writeCounter) == ERROR_SUCCESS) {
                    // Collect for 1 second to get rate
                    PdhCollectQueryData(query);
                    Sleep(1000);
                    PdhCollectQueryData(query);
                    
                    PDH_FMT_COUNTERVALUE readValue, writeValue;
                    if (PdhGetFormattedCounterValue(readCounter, PDH_FMT_DOUBLE, NULL, &readValue) == ERROR_SUCCESS) {
                        disk.readRate = readValue.doubleValue / (1024.0 * 1024.0); // Convert to MB/s
                    }
                    
                    if (PdhGetFormattedCounterValue(writeCounter, PDH_FMT_DOUBLE, NULL, &writeValue) == ERROR_SUCCESS) {
                        disk.writeRate = writeValue.doubleValue / (1024.0 * 1024.0); // Convert to MB/s
                    }
                }
                PdhCloseQuery(query);
            }
            
            // Get associated partitions
            std::wstring diskPath;
            if (SUCCEEDED(obj->Get(L"DeviceID", 0, &vtProp, 0, 0))) {
                diskPath = vtProp.bstrVal;
                VariantClear(&vtProp);
                
                // Query partition to disk relationship
                std::wstring partitionQuery = L"ASSOCIATORS OF {Win32_DiskDrive.DeviceID='" + 
                                             diskPath + L"'} WHERE AssocClass=Win32_DiskDriveToDiskPartition";
                
                auto partitionResults = executeWMIQuery(pSvc, partitionQuery);
                
                for (auto& partObj : partitionResults) {
                    // Get partition info
                    if (SUCCEEDED(partObj->Get(L"DeviceID", 0, &vtProp, 0, 0))) {
                        std::wstring partitionPath = vtProp.bstrVal;
                        VariantClear(&vtProp);
                        
                        // Query logical disks associated with this partition
                        std::wstring logicalDiskQuery = L"ASSOCIATORS OF {Win32_DiskPartition.DeviceID='" + 
                                                      partitionPath + L"'} WHERE AssocClass=Win32_LogicalDiskToPartition";
                        
                        auto logicalDiskResults = executeWMIQuery(pSvc, logicalDiskQuery);
                        
                        for (auto& logicalObj : logicalDiskResults) {
                            if (SUCCEEDED(logicalObj->Get(L"DeviceID", 0, &vtProp, 0, 0))) {
                                std::wstring driveLetter = vtProp.bstrVal;
                                VariantClear(&vtProp);
                                
                                // Find the matching partition in our partitions list
                                for (auto& partition : getPartitions()) {
                                    if (partition.mountPoint.find(wstrToStr(driveLetter)) != std::string::npos) {
                                        disk.partitions.push_back(partition);
                                        break;
                                    }
                                }
                            }
                            logicalObj->Release();
                        }
                    }
                    partObj->Release();
                }
            }
            
            obj->Release();
            disks.push_back(disk);
        }
        
        cleanupWMI(pSvc);
    }
    
    return disks;
}

// Get disk information as JSON string
char* getDiskInfo() {
    auto disks = getPhysicalDisks();
    json disksArray = json::array();
    
    for (const auto& disk : disks) {
        json partitionsArray = json::array();
        
        for (const auto& partition : disk.partitions) {
            json partitionJson = {
                {"name", partition.name},
                {"mountPoint", partition.mountPoint},
                {"fileSystem", partition.fileSystem},
                {"totalSize", partition.totalSize},
                {"totalSizeFormatted", formatBytes(partition.totalSize)},
                {"freeSpace", partition.freeSpace},
                {"freeSpaceFormatted", formatBytes(partition.freeSpace)},
                {"usedPercentage", partition.usedPercentage},
                {"isBootPartition", partition.isBootPartition},
                {"isSystemPartition", partition.isSystemPartition}
            };
            partitionsArray.push_back(partitionJson);
        }
        
        json diskJson = {
            {"model", disk.model},
            {"serialNumber", disk.serialNumber},
            {"interfaceType", disk.interfaceType},
            {"mediaType", disk.mediaType},
            {"size", disk.size},
            {"sizeFormatted", formatBytes(disk.size)},
            {"isRemovable", disk.isRemovable},
            {"diskNumber", disk.diskNumber},
            {"status", disk.status},
            {"health", disk.health},
            {"temperature", disk.temperature},
            {"bytesRead", disk.bytesRead},
            {"bytesWritten", disk.bytesWritten},
            {"readRate", disk.readRate},
            {"writeRate", disk.writeRate},
            {"partitions", partitionsArray}
        };
        
        disksArray.push_back(diskJson);
    }
    
    return strdup_cstr(disksArray.dump());
}