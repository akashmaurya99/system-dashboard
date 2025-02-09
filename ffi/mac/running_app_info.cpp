#include "../include/running_app_info.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <unistd.h>
#include <sys/sysctl.h>

using namespace std;

struct ProcessInfo {
    int pid;
    string name;
    string user;
    double cpuUsage;
    double memUsage;
    int threads;
    string cpuTime;
    double gpuUsage;
    string kind;
};

// Function to execute shell command and return output
static string execCommand(const char* cmd) {
    char buffer[256];
    string result;
    FILE* pipe = popen(cmd, "r");
    if (!pipe) return "ERROR";
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }
    pclose(pipe);
    return result;
}

// Function to extract installed applications and return JSON
string getInstalledApplicationsJSON() {
    vector<string> apps;
    string output = execCommand("mdfind 'kMDItemContentType == \"com.apple.application-bundle\"'");
    stringstream ss(output);
    string line;

    while (getline(ss, line)) {
        size_t lastSlash = line.find_last_of("/");
        size_t dotApp = line.rfind(".app");

        if (lastSlash != string::npos && dotApp != string::npos) {
            string appName = line.substr(lastSlash + 1, dotApp - lastSlash - 1);
            apps.push_back(appName);
        }
    }

    // Convert vector to JSON
    ostringstream json;
    json << "{ \"installed_applications\": [";
    for (size_t i = 0; i < apps.size(); i++) {
        json << "\"" << apps[i] << "\"";
        if (i < apps.size() - 1) json << ", ";
    }
    json << "] }";

    return json.str();
}

// Function to get running processes and return JSON
string getRunningProcessesJSON() {
    vector<ProcessInfo> processes;
    FILE *fp = popen("ps -axo pid,comm,%cpu,%mem,user,etime,nthr | sort -k3 -nr", "r");
    if (!fp) return "{}";

    char buffer[512];
    fgets(buffer, sizeof(buffer), fp); // Skip header
    while (fgets(buffer, sizeof(buffer), fp)) {
        ProcessInfo process;
        stringstream ss(buffer);
        ss >> process.pid >> process.name >> process.cpuUsage >> process.memUsage;
        ss >> process.user >> process.cpuTime >> process.threads;
        process.kind = (process.user == "root") ? "System" : "User";
        process.gpuUsage = 0.0; // Placeholder for GPU stats
        processes.push_back(process);
    }
    pclose(fp);

    // Convert vector to JSON
    ostringstream json;
    json << "{ \"running_processes\": [";
    for (size_t i = 0; i < processes.size(); i++) {
        json << "{";
        json << "\"pid\": " << processes[i].pid << ", ";
        json << "\"name\": \"" << processes[i].name << "\", ";
        json << "\"user\": \"" << processes[i].user << "\", ";
        json << "\"cpu_usage\": " << processes[i].cpuUsage << ", ";
        json << "\"memory_usage\": " << processes[i].memUsage << ", ";
        json << "\"threads\": " << processes[i].threads << ", ";
        json << "\"cpu_time\": \"" << processes[i].cpuTime << "\", ";
        json << "\"gpu_usage\": " << processes[i].gpuUsage << ", ";
        json << "\"kind\": \"" << processes[i].kind << "\"";
        json << "}";
        if (i < processes.size() - 1) json << ", ";
    }
    json << "] }";

    return json.str();
}

// // Main function
// int main() {
//     cout << getInstalledApplicationsJSON() << endl;
//     while (true) {
//         system("clear"); // Clear screen before refreshing
//         cout << getRunningProcessesJSON() << endl;

//         sleep(3); // Refresh every 3 seconds
//     }
//     return 0;
// }