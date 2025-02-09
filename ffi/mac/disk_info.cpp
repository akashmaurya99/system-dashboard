#include "../include/disk_info.h"
#include <iostream>
#include <sys/mount.h>
#include <cstdlib>
#include <sstream>
#include <cstdio>

using namespace std;

// Function to execute shell commands and return output
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

// Function to Get Disk Size and Usage
string getDiskUsage(const string &diskPath) {
    struct statfs stat;
    if (statfs(diskPath.c_str(), &stat) == 0) {
        unsigned long long total_bytes = stat.f_blocks * stat.f_bsize;
        unsigned long long free_bytes = stat.f_bfree * stat.f_bsize;
        unsigned long long used_bytes = total_bytes - free_bytes;

        double total_gb_decimal = total_bytes / 1e9; // Manufacturer GB
        double used_gb = (used_bytes / (1024.0 * 1024 * 1024)) * 1.073741824; // Convert GiB -> GB
        double free_gb = (free_bytes / (1024.0 * 1024 * 1024)) * 1.073741824; // Convert GiB -> GB

        ostringstream oss;
        oss << "{\n";
        oss << "  \"Total Disk Size\": \"" << total_gb_decimal << " GB\",\n";
        oss << "  \"Used Disk Space\": \"" << used_gb << " GB\",\n";
        oss << "  \"Free Disk Space\": \"" << free_gb << " GB\"\n";
        oss << "}";
        return oss.str();
    }
    return "Error getting disk usage!";
}

// Function to Measure Disk Speed using 'dd' and return result as string
string getDiskSpeed() {
    string writeSpeed = execCommand("dd if=/dev/zero of=./speedtest bs=1m count=1024 oflag=sync 2>&1 | grep -Eo '[0-9]+ bytes/sec' | awk '{print $1 / 1048576 \" MB/s\"}'");
    string readSpeed = execCommand("dd if=./speedtest of=/dev/null bs=1m count=1024 2>&1 | grep -Eo '[0-9]+ bytes/sec' | awk '{print $1 / 1048576 \" MB/s\"}'");
    system("rm -f ./speedtest"); // Clean up temp file

    ostringstream oss;
    oss << "{\n";
    oss << "  \"Write Speed\": \"" << (writeSpeed.empty() ? "Error measuring" : writeSpeed) << "\",\n";
    oss << "  \"Read Speed\": \"" << (readSpeed.empty() ? "Error measuring" : readSpeed) << "\"\n";
    oss << "}";
    return oss.str();
}

// Function to Get Detailed Disk Info
string getDiskDetails() {
    string diskInfo = execCommand("diskutil info / | grep -E 'Device Name|Media Name|Medium Type|Protocol|S.M.A.R.T. Status|File System|Mount Point|Volume Name|Disk Size|Block Size|Allocation Block Size'");

    ostringstream oss;
    oss << "{\n";
    istringstream iss(diskInfo);
    string line;
    while (getline(iss, line)) {
        size_t pos = line.find(":");
        if (pos != string::npos) {
            string key = line.substr(0, pos);
            string value = line.substr(pos + 2);
            oss << "  \"" << key << "\": \"" << value << "\",\n";
        }
    }
    string result = oss.str();
    if (!result.empty()) {
        result.pop_back(); // Remove last comma
        result.pop_back();
    }
    result += "\n}";
    return result;
}

// int main() {
//     cout << "Disk Usage:\n" << getDiskUsage("/") << "\n\n";
//     cout << "Disk Speed:\n" << getDiskSpeed() << "\n\n";
//     cout << "Disk Details:\n" << getDiskDetails() << "\n";

//     return 0;
// }
