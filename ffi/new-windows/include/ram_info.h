#pragma once

#include "common.h"
#include "strdup_cstr.h"
#include <string>
#include <vector>

// Get RAM information as JSON string
char* getRAMInfo();

// Structure to hold memory information
struct MemoryInfo {
    uint64_t totalPhysical;      // in bytes
    uint64_t availablePhysical;  // in bytes
    uint64_t usedPhysical;       // in bytes
    double usagePercentage;      // 0-100
    
    uint64_t totalVirtual;       // in bytes
    uint64_t availableVirtual;   // in bytes
    uint64_t usedVirtual;        // in bytes
    
    uint64_t totalPageFile;      // in bytes
    uint64_t availablePageFile;  // in bytes
    
    uint64_t totalCache;         // in bytes
    
    std::string memoryType;      // DDR4, DDR3, etc.
    int memorySpeed;             // in MHz
    int numberOfSlots;
    int usedSlots;
    
    struct MemorySlot {
        std::string bankLabel;
        std::string deviceLocator;
        std::string manufacturer;
        std::string partNumber;
        uint64_t capacity;       // in bytes
        int speed;               // in MHz
        std::string formFactor;  // DIMM, SODIMM, etc.
        std::string memoryType;  // DDR4, DDR3, etc.
        bool isPresent;
    };
    
    std::vector<MemorySlot> slots;
};

// Get detailed memory information
MemoryInfo getMemoryDetails();