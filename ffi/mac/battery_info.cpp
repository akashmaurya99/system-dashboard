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

