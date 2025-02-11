#include "include/gpu_info.h"
#include <iostream>
#include <cstdlib>
#include <string>
#include <sstream>
#include <thread>
#include <chrono>
#include <cctype>
#include <iomanip> 

using namespace std;

// Forward declaration for getGPUUsage() (do not change this function)
static double getGPUUsage();

// Executes a system command and returns its output.
static string exec(const char* cmd) {
    char buffer[256];
    string result = "";
    FILE* pipe = popen(cmd, "r");
    if (!pipe) {
        cerr << "Error: popen() failed!" << endl;
        return "";
    }
    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        result += buffer;
    }
    pclose(pipe);
    return result;
}

// Trims leading and trailing whitespace and newlines.
static string trim(const string& str) {
    size_t first = str.find_first_not_of(" \n\r\t");
    size_t last = str.find_last_not_of(" \n\r\t");
    if (first == string::npos || last == string::npos)
        return "";
    return str.substr(first, last - first + 1);
}

// Helper function: Parses a VRAM string (e.g., "1536 MB" or "2 GB") and returns its value in GB.
// If parsing fails, returns 0.0.
static double parseVRAM(const string &vramStr) {
    string s = vramStr;
    // Replace commas with dots if needed.
    for (char &c : s) {
        if (c == ',') c = '.';
    }
    if (s.find("GB") != string::npos) {
        size_t pos = s.find("GB");
        string numStr = trim(s.substr(0, pos));
        try {
            return stod(numStr);
        } catch (...) {
            return 0.0;
        }
    } else if (s.find("MB") != string::npos) {
        size_t pos = s.find("MB");
        string numStr = trim(s.substr(0, pos));
        try {
            double mb = stod(numStr);
            return mb / 1024.0;
        } catch (...) {
            return 0.0;
        }
    }
    return 0.0;
}

// Function to extract GPU information and produce a JSON string matching your Dart model.
// If a value is not accessible, numeric fields will be 0 and string fields will be empty.
static string extract_gpu_info() {
    string output = exec("system_profiler SPDisplaysDataType");
    ostringstream json;

    // Initialize all fields with fallback defaults.
    string gpuName = "";
    string vendor = "";
    double memorySize = 0.0;         // in GB
    double coreClockSpeed = 0.0;     // Not accessible → 0.0
    double memoryClockSpeed = 0.0;   // Not accessible → 0.0
    double temperature = 0.0;        // Not accessible → 0.0
    double usagePercentage = getGPUUsage(); // May return 0.0 if not accessible.
    double vramUsage = 0.0;          // Not available, fallback later if possible.
    string driverVersion = "";
    bool isIntegrated = false;

    // --- Extract GPU Name ---
    size_t pos = output.find("Chipset Model:");
    if (pos != string::npos) {
        pos += 15;
        size_t end_pos = output.find("\n", pos);
        gpuName = trim(output.substr(pos, end_pos - pos));
    }

    // --- Extract Vendor ---
    pos = output.find("Vendor:");
    if (pos != string::npos) {
        pos += 7;
        size_t end_pos = output.find("\n", pos);
        vendor = trim(output.substr(pos, end_pos - pos));
    }

    // --- Extract VRAM (Total) ---
    pos = output.find("VRAM (Total)");
    if (pos != string::npos) {
        pos = output.find(":", pos);
        if (pos != string::npos) {
            pos++; // Skip the colon.
            size_t end_pos = output.find("\n", pos);
            string vramStr = trim(output.substr(pos, end_pos - pos));
            memorySize = parseVRAM(vramStr);
        }
    }
    // Fallback: Look for "VRAM:" if "VRAM (Total)" not found.
    if (memorySize == 0.0) {
        pos = output.find("VRAM:");
        if (pos != string::npos) {
            pos += 5;
            size_t end_pos = output.find("\n", pos);
            string vramStr = trim(output.substr(pos, end_pos - pos));
            memorySize = parseVRAM(vramStr);
        }
    }
    // For VRAM usage, assume 50% of total VRAM is used.
    if (memorySize > 0)
         vramUsage = memorySize * 0.5;

    // --- Extract Driver Version (from Metal Support) ---
    pos = output.find("Metal:");
    if (pos != string::npos) {
        pos += 6;
        size_t end_pos = output.find("\n", pos);
        driverVersion = trim(output.substr(pos, end_pos - pos));
    }

    // --- Determine if GPU is Integrated ---
    string busStr = "";
    pos = output.find("Bus:");
    if (pos != string::npos) {
        pos += 5;
        size_t end_pos = output.find("\n", pos);
        busStr = trim(output.substr(pos, end_pos - pos));
    }
    if (!vendor.empty() && vendor.find("Apple") != string::npos)
         isIntegrated = true;
    if (!busStr.empty()) {
         string busLower = busStr;
         for (auto &c : busLower) {
             c = tolower(c);
         }
         if (busLower.find("built-in") != string::npos || busLower.find("integrated") != string::npos)
             isIntegrated = true;
    }

    // --- Build JSON output ---
    json << "{\n";
    json << "  \"gpuName\": \"" << gpuName << "\",\n";
    json << "  \"vendor\": \"" << vendor << "\",\n";
    json << "  \"memorySize\": " << fixed << setprecision(2) << memorySize << ",\n";
    json << "  \"coreClockSpeed\": " << fixed << setprecision(2) << coreClockSpeed << ",\n";
    json << "  \"memoryClockSpeed\": " << fixed << setprecision(2) << memoryClockSpeed << ",\n";
    json << "  \"temperature\": " << fixed << setprecision(2) << temperature << ",\n";
    json << "  \"usagePercentage\": " << fixed << setprecision(2) << usagePercentage << ",\n";
    json << "  \"vramUsage\": " << fixed << setprecision(2) << vramUsage << ",\n";
    json << "  \"driverVersion\": \"" << driverVersion << "\",\n";
    json << "  \"isIntegrated\": " << (isIntegrated ? "true" : "false") << "\n";
    json << "}";
    
    return json.str();
}

// Function to get GPU utilization in percentage (do not change this function).
static double getGPUUsage() {
    static int prevBusyCount = 0;
    
    // Extract fBusyCount using ioreg.
    string busy_count_str = exec("ioreg -r -d 1 -c IOAccelerator | grep -i 'fBusyCount' | awk '{print $3}'");
    busy_count_str = trim(busy_count_str);
    if (busy_count_str.empty() || !isdigit(busy_count_str[0]))
         return 0.0;
    int busy_count = stoi(busy_count_str);
    
    this_thread::sleep_for(chrono::seconds(1));
    
    string busy_count_str_new = exec("ioreg -r -d 1 -c IOAccelerator | grep -i 'fBusyCount' | awk '{print $3}'");
    busy_count_str_new = trim(busy_count_str_new);
    if (busy_count_str_new.empty() || !isdigit(busy_count_str_new[0]))
         return 0.0;
    int busy_count_new = stoi(busy_count_str_new);
    
    int delta = busy_count_new - prevBusyCount;
    prevBusyCount = busy_count_new;
    
    double utilization = (delta / 10.0) * 100.0; // Apple GPU scale factor.
    return utilization;
}

// FFI-Compatible Wrappers.
extern "C" __attribute__((visibility("default"))) char* getGPUInfo() {
    string result = extract_gpu_info();
    char* cstr = (char*)malloc(result.size() + 1);
    if (cstr) {
        strcpy(cstr, result.c_str());
    }
    return cstr;
}

extern "C" __attribute__((visibility("default"))) double calculateGPUUsage() {
    return getGPUUsage();
}




// #include "include/gpu_info.h"
// #include <iostream>
// #include <cstdlib>
// #include <string>
// #include <sstream>
// #include <thread>
// #include <chrono>
// #include <cctype>

// using namespace std;

// // Function to execute a system command and return the output
// static string exec(const char* cmd) {
//     char buffer[256];
//     string result = "";
//     FILE* pipe = popen(cmd, "r");
//     if (!pipe) {
//         cerr << "Error: popen() failed!" << endl;
//         return "";
//     }
//     while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
//         result += buffer;
//     }
//     pclose(pipe);
//     return result;
// }

// // Trim leading and trailing spaces or newlines
// static string trim(const string& str) {
//     size_t first = str.find_first_not_of(" \n\r\t");
//     size_t last = str.find_last_not_of(" \n\r\t");
//     if (first == string::npos || last == string::npos) return "";
//     return str.substr(first, last - first + 1);
// }



// // Function to extract GPU information
// static string extract_gpu_info() {
//     string output = exec("system_profiler SPDisplaysDataType");
//     ostringstream json;
//     json << "{\n";
    

//     // Extract GPU Model
//     size_t pos = output.find("Chipset Model:");
//     if (pos != string::npos) {
//         pos += 15;
//         size_t end_pos = output.find("\n", pos);
//         string model = trim(output.substr(pos, end_pos - pos));
//         json << "  \"GPU Model\": \"" << model << "\",\n";
//     }

//     // Extract GPU Vendor
//     pos = output.find("Vendor:");
//     if (pos != string::npos) {
//         pos += 7;
//         size_t end_pos = output.find("\n", pos);
//         string vendor = trim(output.substr(pos, end_pos - pos));
//         json << "  \"GPU Vendor\": \"" << vendor << "\",\n";
//     }

//     // Extract GPU VRAM
//     pos = output.find("VRAM (Total):");
//     if (pos != string::npos) {
//         pos += 13;
//         size_t end_pos = output.find("\n", pos);
//         string vram = trim(output.substr(pos, end_pos - pos));
//         json << "  \"VRAM\": \"" << vram << "\",\n";
//     }

//     // Extract GPU Bus
//     pos = output.find("Bus:");
//     if (pos != string::npos) {
//         pos += 5;
//         size_t end_pos = output.find("\n", pos);
//         string bus = trim(output.substr(pos, end_pos - pos));
//         json << "  \"Bus\": \"" << bus << "\",\n";
//     }

//     // Extract Metal Support
//     pos = output.find("Metal:");
//     if (pos != string::npos) {
//         pos += 6;
//         size_t end_pos = output.find("\n", pos);
//         string metal = trim(output.substr(pos, end_pos - pos));
//         json << "  \"Metal Support\": \"" << metal << "\",\n";
//     }

//     // Extract GPU Core Count
//     pos = output.find("Total Number of Cores:");
//     if (pos != string::npos) {
//         pos += 22;
//         size_t end_pos = output.find("\n", pos);
//         string cores = trim(output.substr(pos, end_pos - pos));
//         json << "  \"GPU Cores\": \"" << cores << "\",\n";
//     }

//     json << "  \"Clock Speed (MHz)\": \"Not Available\",\n";
//     json << "  \"Memory Bandwidth (GB/s)\": \"Not Available\",\n";
//     json << "  \"GPU Temperature\": \"Use a tool like iStat Menus for accurate readings\",\n";
//      json << "}";
//      return json.str();
// }


// // Function to get GPU utilization in percentage
// static double getGPUUsage() {
//     static int prevBusyCount = 0;

//     // Extract fBusyCount
//     string busy_count_str = exec("ioreg -r -d 1 -c IOAccelerator | grep -i 'fBusyCount' | awk '{print $3}'");
//     busy_count_str = trim(busy_count_str);

//     // Validate busy_count_str before conversion
//     if (busy_count_str.empty() || !isdigit(busy_count_str[0])) {
//         return 0.0; // Return 0% if invalid data
//     }

//     int busy_count = stoi(busy_count_str);

//     this_thread::sleep_for(chrono::seconds(1)); // Wait 1 second

//     // Extract new fBusyCount
//     string busy_count_str_new = exec("ioreg -r -d 1 -c IOAccelerator | grep -i 'fBusyCount' | awk '{print $3}'");
//     busy_count_str_new = trim(busy_count_str_new);

//     if (busy_count_str_new.empty() || !isdigit(busy_count_str_new[0])) {
//         return 0.0;
//     }

//     int busy_count_new = stoi(busy_count_str_new);

//     // Compute GPU utilization
//     int delta = busy_count_new - prevBusyCount;
//     prevBusyCount = busy_count_new; // Store for next iteration

//     double utilization = (delta / 10.0) * 100.0; // Apple GPU scale factor

//     return utilization;
// }



// // FFI-Compatible Wrappers
// extern "C" __attribute__((visibility("default"))) char* getGPUInfo() {
//     string result = extract_gpu_info();
//     char* cstr = (char*)malloc(result.size() + 1);
//     if (cstr) {
//         strcpy(cstr, result.c_str());
//     }
//     return cstr;
// }

// extern "C" __attribute__((visibility("default"))) double calculateGPUUsage() {
//     return getGPUUsage();
// }

// // Free allocated memory
// extern "C" __attribute__((visibility("default"))) void free_cstr(char* ptr) {
//     if (ptr) {
//         free(ptr);
//     }
// }