#include "../include/ram_info.h"
#include <iostream>
#include <sys/sysctl.h>
#include <mach/mach.h>
#include <cstdlib>
#include <sstream>
#include <cstdio>
#include <string>
#include <regex>

using namespace std;

// Function to get Total RAM Size (in GB)
double getTotalRAMSize() {
    int64_t ramSize;
    size_t size = sizeof(ramSize);
    if (sysctlbyname("hw.memsize", &ramSize, &size, NULL, 0) == 0) {
        return static_cast<double>(ramSize) / (1024 * 1024 * 1024); // Convert to GB
    }
    return -1; // Error case
}

// Function to get Used RAM (in MB)
double getUsedRAM() {
    mach_msg_type_number_t count = HOST_VM_INFO64_COUNT;
    vm_statistics64_data_t vmStats;
    mach_port_t hostPort = mach_host_self();

    if (host_statistics64(hostPort, HOST_VM_INFO64, (host_info_t)&vmStats, &count) == KERN_SUCCESS) {
        double totalMB = getTotalRAMSize() * 1024; // Convert GB to MB
        double freeMB = (vmStats.free_count + vmStats.inactive_count) * vm_page_size / (1024 * 1024);
        return totalMB - freeMB;
    }
    return -1; // Error case
}

// Function to get Free RAM (in MB)
double getFreeRAM() {
    mach_msg_type_number_t count = HOST_VM_INFO64_COUNT;
    vm_statistics64_data_t vmStats;
    mach_port_t hostPort = mach_host_self();

    if (host_statistics64(hostPort, HOST_VM_INFO64, (host_info_t)&vmStats, &count) == KERN_SUCCESS) {
        return (vmStats.free_count + vmStats.inactive_count) * vm_page_size / (1024 * 1024);
    }
    return -1; // Error case
}

// Function to extract key-value pairs from system_profiler output
string getRAMDetailsFormatted() {
    FILE *pipe = popen("system_profiler SPMemoryDataType", "r");
    if (!pipe) return "Error fetching RAM details";

    string details, line;
    char buffer[256];
    ostringstream jsonDetails;
    
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        line = buffer;
        size_t colonPos = line.find(":");
        if (colonPos != string::npos) {
            string key = line.substr(0, colonPos);
            string value = line.substr(colonPos + 1);

            // Trim whitespace
            key = regex_replace(key, regex("^\\s+|\\s+$"), "");
            value = regex_replace(value, regex("^\\s+|\\s+$"), "");

            if (!key.empty() && !value.empty()) {
                jsonDetails << "\"" << key << "\": \"" << value << "\", ";
            }
        }
    }
    pclose(pipe);

    string formattedDetails = jsonDetails.str();
    if (!formattedDetails.empty()) {
        formattedDetails.pop_back(); // Remove last comma
        formattedDetails.pop_back();
    }
    
    return formattedDetails;
}

// Function to format RAM info in JSON format
string getRAMInfoJSON() {
    ostringstream json;
    json << "{";
    json << "\"Total_RAM_GB\": " << getTotalRAMSize() << ", ";
    json << "\"Used_RAM_MB\": " << getUsedRAM() << ", ";
    json << "\"Free_RAM_MB\": " << getFreeRAM() << ", ";
    json << getRAMDetailsFormatted();
    json << "}";

    return json.str();
}

// // Main function for testing
// int main() {
//     cout << getRAMInfoJSON() << endl;
//     return 0;
// }