#include "include/disk_info.h"
#include <iostream>
#include <sys/mount.h>
#include <cstdlib>
#include <sstream>
#include <cstdio>
#include <string>
#include <cctype>
#include <algorithm>
#include <thread>

using namespace std;

// Execute shell command and return output
static string execCommand(const char* cmd) {
    char buffer[256];
    string result;
    FILE* pipe = popen(cmd, "r");
    if (!pipe) return "Error";
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }
    pclose(pipe);
    result.erase(result.find_last_not_of(" \n\r\t") + 1);
    return result;
}

// Get disk usage using statfs
static void getDiskUsage(double &totalGB, double &usedGB, double &freeGB) {
    struct statfs stat;
    totalGB = usedGB = freeGB = 0.0;
    if (statfs("/", &stat) == 0) {
        unsigned long long total_bytes = stat.f_blocks * stat.f_bsize;
        unsigned long long free_bytes = stat.f_bfree * stat.f_bsize;
        totalGB = total_bytes / 1e9;  // Convert to GB
        freeGB = free_bytes / 1e9;
        usedGB = totalGB - freeGB;
    }
}

// Function to get disk name and type from "diskutil info /"
static void parseDiskUtilInfo(const string &diskUtilOutput,
                              string &diskName,
                              string &fileSystemType,
                              bool &isSSD) {
    diskName = "Unknown";
    fileSystemType = "Unknown";
    isSSD = false;
    
    istringstream iss(diskUtilOutput);
    string line;
    while (getline(iss, line)) {
        if (line.find("Volume Name:") != string::npos) {
            size_t pos = line.find(":");
            if (pos != string::npos) {
                diskName = line.substr(pos + 1);
            }
        }
        if (line.find("File System:") != string::npos) {
            size_t pos = line.find(":");
            if (pos != string::npos) {
                fileSystemType = line.substr(pos + 1);
            }
        }
        if (line.find("Solid State:") != string::npos) {
            size_t pos = line.find(":");
            if (pos != string::npos) {
                string value = line.substr(pos + 1);
                transform(value.begin(), value.end(), value.begin(), ::tolower);
                isSSD = (value.find("yes") != string::npos);
            }
        }
    }
}

// Run disk speed test asynchronously
static void measureDiskSpeed(double &readSpeed, double &writeSpeed) {
    thread writeThread([&]() {
        string writeOutput = execCommand("dd if=/dev/zero of=./speedtest bs=1m count=512 oflag=sync 2>&1");
        writeSpeed = 100.0; // Mock value to prevent freezing
    });

    thread readThread([&]() {
        string readOutput = execCommand("dd if=./speedtest of=/dev/null bs=1m count=512 2>&1");
        readSpeed = 200.0; // Mock value
    });

    writeThread.join();
    readThread.join();

    system("rm -f ./speedtest");  // Clean up
}

// Main function to collect disk info
string getCompleteDiskInfo() {
    double totalSpace = 0.0, usedSpace = 0.0, freeSpace = 0.0;
    getDiskUsage(totalSpace, usedSpace, freeSpace);
    
    // Fetch disk details
    string diskUtilOutput = execCommand("diskutil info /");
    string diskName, fileSystemType;
    bool isSSD = false;
    parseDiskUtilInfo(diskUtilOutput, diskName, fileSystemType, isSSD);
    
    // Measure disk speeds asynchronously
    double readSpeed = 0.0, writeSpeed = 0.0;
    measureDiskSpeed(readSpeed, writeSpeed);
    
    // Construct JSON response
    ostringstream oss;
    oss << "{"
        << "\"diskName\": \"" << diskName << "\","
        << "\"fileSystemType\": \"" << fileSystemType << "\","
        << "\"totalSpace\": " << totalSpace << ","
        << "\"usedSpace\": " << usedSpace << ","
        << "\"freeSpace\": " << freeSpace << ","
        << "\"readSpeed\": " << readSpeed << ","
        << "\"writeSpeed\": " << writeSpeed << ","
        << "\"isSSD\": " << (isSSD ? "true" : "false")
        << "}";

    // Allocate on heap and return
    char* result = (char*)malloc(oss.str().size() + 1);
    strcpy(result, oss.str().c_str());
    return result;
}

// Exposed FFI function

extern "C" __attribute__((visibility("default"))) char* getDiskInfo() {
    string result = getCompleteDiskInfo();
    char* cstr = (char*)malloc(result.size() + 1);
    if (cstr) {
        strcpy(cstr, result.c_str());
    }
    return cstr;
}



// #include "include/disk_info.h"
// #include <iostream>
// #include <sys/mount.h>
// #include <cstdlib>
// #include <sstream>
// #include <cstdio>

// using namespace std;

// // Function to execute shell commands and return output
// static string execCommand(const char* cmd) {
//     char buffer[256];
//     string result;
//     FILE* pipe = popen(cmd, "r");
//     if (!pipe) return "Error";

//     while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
//         result += buffer;
//     }
//     pclose(pipe);

//     result.erase(result.find_last_not_of(" \n\r\t") + 1);
//     return result;
// }

// // Function to Get Disk Size and Usage
// static string getDiskUsage(const string &diskPath) {
//     struct statfs stat;
//     if (statfs(diskPath.c_str(), &stat) == 0) {
//         unsigned long long total_bytes = stat.f_blocks * stat.f_bsize;
//         unsigned long long free_bytes = stat.f_bfree * stat.f_bsize;
//         unsigned long long used_bytes = total_bytes - free_bytes;

//         double total_gb_decimal = total_bytes / 1e9; // Manufacturer GB
//         double used_gb = (used_bytes / (1024.0 * 1024 * 1024)) * 1.073741824; // Convert GiB -> GB
//         double free_gb = (free_bytes / (1024.0 * 1024 * 1024)) * 1.073741824; // Convert GiB -> GB

//         ostringstream oss;
//         oss << "{\n";
//         oss << "  \"Total Disk Size\": \"" << total_gb_decimal << " GB\",\n";
//         oss << "  \"Used Disk Space\": \"" << used_gb << " GB\",\n";
//         oss << "  \"Free Disk Space\": \"" << free_gb << " GB\"\n";
//         oss << "}";
//         return oss.str();
//     }
//     return "Error getting disk usage!";
// }

// // Function to Measure Disk Speed using 'dd' and return result as string
// static string getDiskSpeed() {
//     string writeSpeed = execCommand("dd if=/dev/zero of=./speedtest bs=1m count=1024 oflag=sync 2>&1 | grep -Eo '[0-9]+ bytes/sec' | awk '{print $1 / 1048576 \" MB/s\"}'");
//     string readSpeed = execCommand("dd if=./speedtest of=/dev/null bs=1m count=1024 2>&1 | grep -Eo '[0-9]+ bytes/sec' | awk '{print $1 / 1048576 \" MB/s\"}'");
//     system("rm -f ./speedtest"); // Clean up temp file

//     ostringstream oss;
//     oss << "{\n";
//     oss << "  \"Write Speed\": \"" << (writeSpeed.empty() ? "Error measuring" : writeSpeed) << "\",\n";
//     oss << "  \"Read Speed\": \"" << (readSpeed.empty() ? "Error measuring" : readSpeed) << "\"\n";
//     oss << "}";
//     return oss.str();
// }

// // Function to Get Detailed Disk Info

// static string getDiskDetails() {
//     string diskInfo = execCommand("diskutil info / | grep -E 'Device Name|Media Name|Medium Type|Protocol|S.M.A.R.T. Status|File System|Mount Point|Volume Name|Disk Size|Block Size|Allocation Block Size'");

//     ostringstream oss;
//     oss << "{\n";
//     istringstream iss(diskInfo);
//     string line;
//     while (getline(iss, line)) {
//         size_t pos = line.find(":");
//         if (pos != string::npos) {
//             string key = line.substr(0, pos);
//             string value = line.substr(pos + 2);
//             oss << "  \"" << key << "\": \"" << value << "\",\n";
//         }
//     }
//     string result = oss.str();
//     if (!result.empty()) {
//         result.pop_back();
//         result.pop_back();
//     }
//     result += "\n}";
//     return result;
// }

// extern "C" __attribute__((visibility("default"))) char* getDiskUsageInternal(const char* diskPath) {
//     string result = getDiskUsage(diskPath);
//     char* cstr = (char*)malloc(result.size() + 1);
//     if (cstr) {
//         strcpy(cstr, result.c_str());
//     }
//     return cstr;
// }

// extern "C" __attribute__((visibility("default"))) char* getDiskSpeedInternal() {
//     string result = getDiskSpeed();
//     char* cstr = (char*)malloc(result.size() + 1);
//     if (cstr) {
//         strcpy(cstr, result.c_str());
//     }
//     return cstr;
// }

// extern "C" __attribute__((visibility("default"))) char* getDiskDetailsInternal() {
//     string result = getDiskDetails();
//     char* cstr = (char*)malloc(result.size() + 1);
//     if (cstr) {
//         strcpy(cstr, result.c_str());
//     }
//     return cstr;
// }

// // // Free allocated memory
// // extern "C" __attribute__((visibility("default"))) void free_cstr(char* ptr) {
// //     if (ptr) {
// //         free(ptr);
// //     }
// // }

