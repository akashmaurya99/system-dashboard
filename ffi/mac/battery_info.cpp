#include "include/battery_info.h"
#include <iostream>
#include <cstdlib>
#include <sstream>
#include <regex>
#include <iomanip>
#include <algorithm>
#include <thread>
#include <chrono>
#include <ctime>
#include <cmath>
#include <cstring>

using namespace std;

// Execute a system command and return its output as a string.
static string execCommand(const char* cmd) {
    char buffer[256];
    string result;
    FILE* pipe = popen(cmd, "r");
    if (!pipe) return "Error";
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }
    pclose(pipe);
    return result;
}

// Remove leading and trailing whitespace.
static string trim(const string& str) {
    size_t start = str.find_first_not_of(" \t\n\r");
    if (start == string::npos) return "";
    size_t end = str.find_last_not_of(" \t\n\r");
    return str.substr(start, end - start + 1);
}

// Extract a numeric value (as a string) for a given key.
static string extractNumericValue(const string& data, const string& key) {
    regex pattern("\"" + key + "\"\\s*=\\s*([0-9]+)\\b;?");
    smatch match;
    if (regex_search(data, match, pattern)) {
        return match[1].str();
    }
    return "Unknown";
}

// Extract a boolean value (as a string “Yes” or “No”) for a given key.
static string extractBooleanValue(const string& data, const string& key) {
    regex pattern("\"" + key + "\"\\s*=\\s*([A-Za-z]+);?");
    smatch match;
    if (regex_search(data, match, pattern)) {
        return (match[1].str() == "Yes") ? "Yes" : "No";
    }
    return "Unknown";
}

// Extract a string value for a given key.
static string extractStringValue(const string& data, const string& key) {
    regex pattern("\"" + key + "\"\\s*=\\s*\"([^\"]+)\";?");
    smatch match;
    if (regex_search(data, match, pattern)) {
        return match[1].str();
    }
    return "Unknown";
}

// This function extracts system battery info, converts values to the proper types,
// and builds a JSON string whose keys match your Dart BatteryInfo model.
static string displayBatteryInfo() {
    // Get the battery data using ioreg.
    string batteryData = execCommand("ioreg -rn AppleSmartBattery");

    // Extract raw strings for various keys.
    string isChargingStr         = extractBooleanValue(batteryData, "IsCharging");
    string externalConnectedStr  = extractBooleanValue(batteryData, "ExternalConnected");
    string designCapacityStr     = extractNumericValue(batteryData, "DesignCapacity");
    string maxCapacityStr        = extractNumericValue(batteryData, "MaxCapacity");
    string currentCapacityStr    = extractNumericValue(batteryData, "CurrentCapacity");
    string cycleCountStr         = extractNumericValue(batteryData, "CycleCount");
    string voltageStr            = extractNumericValue(batteryData, "Voltage");
    string tempRawStr            = extractNumericValue(batteryData, "Temperature");
    string technologyStr         = extractStringValue(batteryData, "BatteryTechnology");
    if (technologyStr == "Unknown") {
        technologyStr = "Lithium-ion";
    }
    // Try to extract a key for last full charge time (if available).
    string lastFullChargeRaw     = extractStringValue(batteryData, "LastFullChargeDate");

    // --- Determine the actual max capacity in mAh ---
    // Some systems provide the true capacity using these keys.
    string rawMaxCapacity        = extractNumericValue(batteryData, "AppleRawMaxCapacity");
    string nominalChargeCapacity = extractNumericValue(batteryData, "NominalChargeCapacity");

    double actualMaxCapacityVal = 0.0;
    if (nominalChargeCapacity != "Unknown") {
        try {
            actualMaxCapacityVal = stod(nominalChargeCapacity);
        } catch (...) {
            actualMaxCapacityVal = 0.0;
        }
    } else if (rawMaxCapacity != "Unknown") {
        try {
            actualMaxCapacityVal = stod(rawMaxCapacity);
        } catch (...) {
            actualMaxCapacityVal = 0.0;
        }
    } else {
        try {
            actualMaxCapacityVal = stod(maxCapacityStr);
        } catch (...) {
            actualMaxCapacityVal = 0.0;
        }
    }
    // --- End of actual max capacity determination ---

    // --- Compute the instantaneous charge level ---
    // If the raw "CurrentCapacity" is less than 100 we assume it is given as a percentage.
    double chargeLevel = 0.0;  // This is the current battery charge (0–100%)
    if (currentCapacityStr != "Unknown" && actualMaxCapacityVal > 0) {
        try {
            double rawCurrentCapacity = stod(currentCapacityStr);
            if (rawCurrentCapacity < 100) {
                // Treat as percentage (i.e. battery is 93% charged)
                chargeLevel = rawCurrentCapacity;
            } else {
                // Otherwise compute percentage from mAh values.
                chargeLevel = (actualMaxCapacityVal > 0) ? (rawCurrentCapacity / actualMaxCapacityVal) * 100.0 : 0.0;
            }
        } catch (...) {
            chargeLevel = 0.0;
        }
    }
    // --- End of charge level computation ---

    // --- Compute Battery Health Percentage ---
    // Here we compute the battery's "health" as the ratio of its current maximum capacity to its design capacity,
    // then adjust using a degradation factor (which depends on cycle count) and a calibration factor.
    double batteryHealthPercent = 0.0;
    if (designCapacityStr != "Unknown" && actualMaxCapacityVal > 0 && cycleCountStr != "Unknown") {
        try {
            double designCap = stod(designCapacityStr);
            double cycles    = stod(cycleCountStr);
            double rawHealthPercent = (actualMaxCapacityVal / designCap) * 100.0;
            // Apply a logarithmic degradation model:
            double cycleDegradation = max(1.0 - log10(1 + (cycles / 500.0)) * 0.12, 0.80);
            double calibrationFactor = 1.06;
            batteryHealthPercent = rawHealthPercent * cycleDegradation * calibrationFactor;
            if (batteryHealthPercent > 100.0) batteryHealthPercent = 100.0;
        } catch (...) {
            batteryHealthPercent = 0.0;
        }
    }
    // --- End of battery health computation ---
    // In our revised approach, we will output this batteryHealthPercent as the "currentCapacity" value (in %).

    // --- Determine Qualitative Health (optional) ---
    // You may also wish to keep a qualitative indicator.
    string healthQual = "Unknown";
    if (batteryHealthPercent >= 90.0) {
        healthQual = "Good";
    } else if (batteryHealthPercent >= 80.0) {
        healthQual = "Normal";
    } else {
        healthQual = "Bad";
    }
    // --- End of qualitative health ---

    // Convert cycle count to an integer.
    int cycleCount = 0;
    try {
        if (cycleCountStr != "Unknown") {
            cycleCount = stoi(cycleCountStr);
        }
    } catch (...) {
        cycleCount = 0;
    }

    // Convert temperature (raw value is assumed to be in hundredths of a degree Celsius).
    double temperature = 0.0;
    try {
        if (tempRawStr != "Unknown") {
            temperature = stod(tempRawStr) / 100.0;
        }
    } catch (...) {
        temperature = 0.0;
    }

    // Convert voltage from mV to volts.
    double voltage = 0.0;
    try {
        if (voltageStr != "Unknown") {
            voltage = stod(voltageStr) / 1000.0;
        }
    } catch (...) {
        voltage = 0.0;
    }

    // Convert designed capacity.
    double designedCapacity = 0.0;
    try {
        if (designCapacityStr != "Unknown") {
            designedCapacity = stod(designCapacityStr);
        }
    } catch (...) {
        designedCapacity = 0.0;
    }

    // Set the power source: if connected to power then "AC" otherwise "Battery".
    string powerSource = (externalConnectedStr == "Yes") ? "AC" : "Battery";

    // Determine the last full charge time.
    string lastFullChargeTime;
    if (lastFullChargeRaw == "Unknown" || lastFullChargeRaw.empty()) {
        // If not provided, use the current system time in ISO8601 format.
        auto now = chrono::system_clock::now();
        time_t now_c = chrono::system_clock::to_time_t(now);
        char buf[100];
        if (strftime(buf, sizeof(buf), "%FT%TZ", gmtime(&now_c))) {
            lastFullChargeTime = buf;
        } else {
            lastFullChargeTime = "1970-01-01T00:00:00Z";
        }
    } else {
        lastFullChargeTime = lastFullChargeRaw;
    }

    // --- Build JSON Output ---
    // Here we output:
    // - "chargeLevel": the instantaneous battery charge (e.g. 93%)
    // - "currentCapacity": the computed battery health percentage (e.g. ~86%)
    ostringstream json;
    json << "{\n";
    json << "  \"chargeLevel\": " << fixed << setprecision(1) << chargeLevel << ",\n";
    json << "  \"isCharging\": " << ((isChargingStr == "Yes") ? "true" : "false") << ",\n";
    json << "  \"isConnectedToPower\": " << ((externalConnectedStr == "Yes") ? "true" : "false") << ",\n";
    json << "  \"health\": \"" << healthQual << "\",\n";
    json << "  \"technology\": \"" << technologyStr << "\",\n";
    json << "  \"cycleCount\": " << cycleCount << ",\n";
    json << "  \"temperature\": " << fixed << setprecision(2) << temperature << ",\n";
    json << "  \"voltage\": " << fixed << setprecision(2) << voltage << ",\n";
    json << "  \"currentCapacity\": " << fixed << setprecision(0) << batteryHealthPercent << ",\n";
    json << "  \"maxCapacity\": " << fixed << setprecision(0) << actualMaxCapacityVal << ",\n";
    json << "  \"designedCapacity\": " << fixed << setprecision(0) << designedCapacity << ",\n";
    json << "  \"powerSource\": \"" << powerSource << "\",\n";
    json << "  \"lastFullChargeTime\": \"" << lastFullChargeTime << "\"\n";
    json << "}\n";

    return json.str();
}

// Extern "C" function that returns a pointer to a C-style string (which should be freed by the caller).
extern "C" __attribute__((visibility("default"))) char* getBatteryInfo() {
    std::string data = displayBatteryInfo();
    char* cstr = (char*)malloc(data.size() + 1);
    if (cstr) {
        strcpy(cstr, data.c_str());
    }
    return cstr;
}








// #include "include/battery_info.h"
// #include <iostream>
// #include <cstdlib>
// #include <sstream>
// #include <regex>
// #include <iomanip>
// #include <algorithm>
// #include <thread>
// #include <chrono>

// using namespace std;

// static string execCommand(const char* cmd) {
//     char buffer[256];
//     string result;
//     FILE* pipe = popen(cmd, "r");
//     if (!pipe) return "Error";

//     while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
//         result += buffer;
//     }

//     pclose(pipe);
//     return result;
// }

// static string trim(const string& str) {
//     size_t start = str.find_first_not_of(" \t\n\r");
//     if (start == string::npos) return "";
//     size_t end = str.find_last_not_of(" \t\n\r");
//     return str.substr(start, end - start + 1);
// }

// static string extractNumericValue(const string& data, const string& key) {
//     regex pattern("\"" + key + "\"\\s*=\\s*([0-9]+);?");
//     smatch match;
    
//     if (regex_search(data, match, pattern)) {
//         return match[1].str();
//     }
//     return "Unknown";
// }

// static string extractBooleanValue(const string& data, const string& key) {
//     regex pattern("\"" + key + "\"\\s*=\\s*([A-Za-z]+);?");
//     smatch match;

//     if (regex_search(data, match, pattern)) {
//         return (match[1].str() == "Yes") ? "Yes" : "No";
//     }
//     return "Unknown";
// }

// static string extractStringValue(const string& data, const string& key) {
//     regex pattern("\"" + key + "\"\\s*=\\s*\"([^\"]+)\";?");
//     smatch match;
    
//     if (regex_search(data, match, pattern)) {
//         return match[1].str();
//     }
//     return "Unknown";
// }

// static string displayBatteryInfo() {
//     while (true) {
//         string batteryData = execCommand("ioreg -rn AppleSmartBattery");

//         string isCharging = extractBooleanValue(batteryData, "IsCharging");
//         string externalConnected = extractBooleanValue(batteryData, "ExternalConnected");
//         string designCapacity = extractNumericValue(batteryData, "DesignCapacity");
//         string maxCapacity = extractNumericValue(batteryData, "MaxCapacity");
//         string currentCapacity = extractNumericValue(batteryData, "CurrentCapacity");
//         string cycleCount = extractNumericValue(batteryData, "CycleCount");
//         string voltage = extractNumericValue(batteryData, "Voltage");
//         string tempRaw = extractNumericValue(batteryData, "Temperature");
//         string serial = extractStringValue(batteryData, "Serial");
//         string avgTimeToFull = extractNumericValue(batteryData, "AvgTimeToFull");
//         string wattage = extractNumericValue(batteryData, "Watts");
//         string rawMaxCapacity = extractNumericValue(batteryData, "AppleRawMaxCapacity");
//         string nominalChargeCapacity = extractNumericValue(batteryData, "NominalChargeCapacity");

//         // **Battery Status Logic**
//         string batteryStatus;
//         if (externalConnected == "Yes" && isCharging == "Yes") {
//             batteryStatus = "Charging";
//         } else if (externalConnected == "Yes" && isCharging == "No") {
//             batteryStatus = "Fully Charged";
//         } else {
//             batteryStatus = "Discharging";
//         }

//         // // **Battery Health Calculation (Fix)**
//         // string batteryHealth = "Unknown";
//         // if (designCapacity != "Unknown" && maxCapacity != "Unknown") {
//         //     try {
//         //         float health = (stof(maxCapacity) / stof(designCapacity)) * 100;
//         //         stringstream ss;
//         //         ss << fixed << setprecision(1) << health << "%";
//         //         batteryHealth = ss.str();
//         //     } catch (...) {}
//         // }
//      // Use NominalChargeCapacity as the primary source for max capacity
//         string actualMaxCapacity = (nominalChargeCapacity != "Unknown") ? nominalChargeCapacity : rawMaxCapacity;
//         string batteryHealth = "Unknown";

// if (designCapacity != "Unknown" && actualMaxCapacity != "Unknown" && cycleCount != "Unknown") {
//     try {
//         float maxCap = stof(actualMaxCapacity);
//         float designCap = stof(designCapacity);
//         float cycles = stof(cycleCount);

//         // Step 1: Base health calculation
//         float health = (maxCap / designCap) * 100;

//         // Step 2: Logarithmic cycle degradation model
//         float cycleDegradation = max(1.0f - log10(1 + (cycles / 500.0f)) * 0.12f, 0.80f);

//         // Step 3: Apply Apple-like calibration factor
//         float calibrationFactor = 1.06f;

//         health *= cycleDegradation * calibrationFactor;

//         // Step 4: Ensure values stay realistic
//         if (health > 100) health = 100;
//         if (health < 60) health = 60;

//         stringstream ss;
//         ss << fixed << setprecision(1) << health;
//         batteryHealth = ss.str() + "%";
//     } catch (...) {}
// }

//         // **Charge Percentage**
//         string chargePercentage = "Unknown";
//         if (currentCapacity != "Unknown" && maxCapacity != "Unknown") {
//             try {
//                 float percentage = (stof(currentCapacity) / stof(maxCapacity)) * 100;
//                 stringstream ss;
//                 ss << fixed << setprecision(1) << percentage << "%";
//                 chargePercentage = ss.str();
//             } catch (...) {}
//         }

//         // **Temperature Conversion**
//         string batteryTemp = "Unknown";
//         if (tempRaw != "Unknown") {
//             try {
//                 float temp = stof(tempRaw) / 100.0f;
//                 stringstream ss;
//                 ss << fixed << setprecision(2) << temp;
//                 batteryTemp = ss.str() + "°C";
//             } catch (...) {}
//         }

//         // **Manufacturer Extraction (Fix)**
//         string manufacturer = trim(execCommand("system_profiler SPPowerDataType | grep 'Manufacturer:' | awk -F': ' '{print $2}'"));
//         if (manufacturer.empty()) {
//             manufacturer = "Apple Inc.";
//         }

//         // **Serial Number Extraction (Fix)**
//         if (serial == "Unknown") {
//             serial = trim(execCommand("system_profiler SPPowerDataType | grep 'Serial Number:' | awk -F': ' '{print $2}'"));
//         }

//         // **Full Charge Capacity (Fix)**
//         string fullChargeCap = maxCapacity != "Unknown" ? maxCapacity + " mAh" : "Unknown";

//         // **Voltage Conversion**
//         voltage = voltage != "Unknown" ? voltage + " mV" : "Unknown";

//         // **Low Battery Warning (Below 20%)**
//         string lowBatteryMessage = "";
//         if (chargePercentage != "Unknown") {
//             try {
//                 float chargeVal = stof(chargePercentage.substr(0, chargePercentage.length() - 1));
//                 if (chargeVal < 20) {
//                     lowBatteryMessage = "⚠️ Warning: Battery low! Please plug in the charger.";
//                 }
//             } catch (...) {}
//         }


//         string timeToFullCharge = "Calculating...";
//         if (avgTimeToFull != "Unknown" && avgTimeToFull != "65535") {
//             timeToFullCharge = avgTimeToFull + " min";
//         }

//         // **Output JSON**
//         ostringstream json;
//         json << "{\n";
//         json << "  \"Battery Status\": \"" << batteryStatus << "\",\n";
//         json << "  \"Battery Health\": \"" << batteryHealth << "\",\n";
//         json << "  \"Current Charge (%)\": \"" << chargePercentage << "\",\n";
//         json << "  \"Full Charge Capacity (mAh)\": \"" << designCapacity << "\",\n";
//         json << "  \"Cycle Count\": \"" << cycleCount << "\",\n";
//         json << "  \"Battery Temperature\": \"" << batteryTemp << "\",\n";
//         json << "  \"Battery Voltage\": \"" << voltage << "\",\n";
//         json << "  \"Battery Manufacturer\": \"" << manufacturer << "\",\n";
//         json << "  \"Battery Serial Number\": \"" << serial << "\",\n";  
//         json << "  \"Time to Full Charge\": \"" << timeToFullCharge << "\",\n";
//         json << "  \"Power Adapter (W)\": \"" << wattage << " W\"\n";
//         json << "}\n";

//         if (!lowBatteryMessage.empty()) {
//             cout << lowBatteryMessage << endl;
//         }
//         return json.str();
//     }
// }


// extern "C" __attribute__((visibility("default"))) char* getBatteryInfo() {
//     std::string data = displayBatteryInfo();
//     char* cstr = (char*)malloc(data.size() + 1);
//     if (cstr) {
//         strcpy(cstr, data.c_str());
//     }
//     return cstr;
// }



// // Free allocated memory
// extern "C" __attribute__((visibility("default"))) void free_cstr(char* ptr) {
//     if (ptr) {
//         free(ptr);
//     }
// }