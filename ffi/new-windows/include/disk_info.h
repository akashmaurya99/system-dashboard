#pragma once

#include "common.h"
#include "strdup_cstr.h"
#include <string>
#include <vector>

// Get disk information as JSON string
char* getDiskInfo();

// Structure to hold partition information
struct PartitionInfo {
    std::string name;
    std::string mountPoint;
    std::string fileSystem;
    uint64_t totalSize;      // in bytes
    uint64_t freeSpace;      // in bytes
    double usedPercentage;   // 0-100
    bool isBootPartition;
    bool isSystemPartition;
};

// Structure to hold physical disk information
struct PhysicalDiskInfo {
    std::string model;
    std::string serialNumber;
    std::string interfaceType;  // SATA, NVMe, USB, etc.
    std::string mediaType;      // HDD, SSD, etc.
    uint64_t size;              // in bytes
    bool isRemovable;
    int diskNumber;
    std::string status;
    std::string health;
    double temperature;         // in Celsius
    uint64_t bytesRead;         // total bytes read
    uint64_t bytesWritten;      // total bytes written
    double readRate;            // in MB/s
    double writeRate;           // in MB/s
    std::vector<PartitionInfo> partitions;
};

// Get detailed disk information
std::vector<PhysicalDiskInfo> getPhysicalDisks();

// Get partition information
std::vector<PartitionInfo> getPartitions();