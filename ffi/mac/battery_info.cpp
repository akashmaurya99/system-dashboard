#include "../include/battery_info.h"
#include <iostream>
#include <cstdlib>
#include <sstream>
#include <regex>
#include <iomanip>
#include <algorithm>
#include <thread>
#include <chrono>

using namespace std;

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

static string trim(const string& str) {
    size_t start = str.find_first_not_of(" \t\n\r");
    if (start == string::npos) return "";
    size_t end = str.find_last_not_of(" \t\n\r");
    return str.substr(start, end - start + 1);
}

static string extractNumericValue(const string& data, const string& key) {
    regex pattern("\"" + key + "\"\\s*=\\s*([0-9]+);?");
    smatch match;
    
    if (regex_search(data, match, pattern)) {
        return match[1].str();
    }
    return "Unknown";
}

static string extractBooleanValue(const string& data, const string& key) {
    regex pattern("\"" + key + "\"\\s*=\\s*([A-Za-z]+);?");
    smatch match;

    if (regex_search(data, match, pattern)) {
        return (match[1].str() == "Yes") ? "Yes" : "No";
    }
    return "Unknown";
}

static string extractStringValue(const string& data, const string& key) {
    regex pattern("\"" + key + "\"\\s*=\\s*\"([^\"]+)\";?");
    smatch match;
    
    if (regex_search(data, match, pattern)) {
        return match[1].str();
    }
    return "Unknown";
}

string displayBatteryInfo() {
    while (true) {
        string batteryData = execCommand("ioreg -rn AppleSmartBattery");

        string isCharging = extractBooleanValue(batteryData, "IsCharging");
        string externalConnected = extractBooleanValue(batteryData, "ExternalConnected");
        string designCapacity = extractNumericValue(batteryData, "DesignCapacity");
        string maxCapacity = extractNumericValue(batteryData, "MaxCapacity");
        string currentCapacity = extractNumericValue(batteryData, "CurrentCapacity");
        string cycleCount = extractNumericValue(batteryData, "CycleCount");
        string voltage = extractNumericValue(batteryData, "Voltage");
        string tempRaw = extractNumericValue(batteryData, "Temperature");
        string serial = extractStringValue(batteryData, "Serial");
        string avgTimeToFull = extractNumericValue(batteryData, "AvgTimeToFull");
        string wattage = extractNumericValue(batteryData, "Watts");
        string rawMaxCapacity = extractNumericValue(batteryData, "AppleRawMaxCapacity");
        string nominalChargeCapacity = extractNumericValue(batteryData, "NominalChargeCapacity");

        // **Battery Status Logic**
        string batteryStatus;
        if (externalConnected == "Yes" && isCharging == "Yes") {
            batteryStatus = "Charging";
        } else if (externalConnected == "Yes" && isCharging == "No") {
            batteryStatus = "Fully Charged";
        } else {
            batteryStatus = "Discharging";
        }

        // // **Battery Health Calculation (Fix)**
        // string batteryHealth = "Unknown";
        // if (designCapacity != "Unknown" && maxCapacity != "Unknown") {
        //     try {
        //         float health = (stof(maxCapacity) / stof(designCapacity)) * 100;
        //         stringstream ss;
        //         ss << fixed << setprecision(1) << health << "%";
        //         batteryHealth = ss.str();
        //     } catch (...) {}
        // }
     // Use NominalChargeCapacity as the primary source for max capacity
        string actualMaxCapacity = (nominalChargeCapacity != "Unknown") ? nominalChargeCapacity : rawMaxCapacity;
        string batteryHealth = "Unknown";

if (designCapacity != "Unknown" && actualMaxCapacity != "Unknown" && cycleCount != "Unknown") {
    try {
        float maxCap = stof(actualMaxCapacity);
        float designCap = stof(designCapacity);
        float cycles = stof(cycleCount);

        // Step 1: Base health calculation
        float health = (maxCap / designCap) * 100;

        // Step 2: Logarithmic cycle degradation model
        float cycleDegradation = max(1.0f - log10(1 + (cycles / 500.0f)) * 0.12f, 0.80f);

        // Step 3: Apply Apple-like calibration factor
        float calibrationFactor = 1.06f;

        health *= cycleDegradation * calibrationFactor;

        // Step 4: Ensure values stay realistic
        if (health > 100) health = 100;
        if (health < 60) health = 60;

        stringstream ss;
        ss << fixed << setprecision(1) << health;
        batteryHealth = ss.str() + "%";
    } catch (...) {}
}

        // **Charge Percentage**
        string chargePercentage = "Unknown";
        if (currentCapacity != "Unknown" && maxCapacity != "Unknown") {
            try {
                float percentage = (stof(currentCapacity) / stof(maxCapacity)) * 100;
                stringstream ss;
                ss << fixed << setprecision(1) << percentage << "%";
                chargePercentage = ss.str();
            } catch (...) {}
        }

        // **Temperature Conversion**
        string batteryTemp = "Unknown";
        if (tempRaw != "Unknown") {
            try {
                float temp = stof(tempRaw) / 100.0f;
                stringstream ss;
                ss << fixed << setprecision(2) << temp;
                batteryTemp = ss.str() + "°C";
            } catch (...) {}
        }

        // **Manufacturer Extraction (Fix)**
        string manufacturer = trim(execCommand("system_profiler SPPowerDataType | grep 'Manufacturer:' | awk -F': ' '{print $2}'"));
        if (manufacturer.empty()) {
            manufacturer = "Apple Inc.";
        }

        // **Serial Number Extraction (Fix)**
        if (serial == "Unknown") {
            serial = trim(execCommand("system_profiler SPPowerDataType | grep 'Serial Number:' | awk -F': ' '{print $2}'"));
        }

        // **Full Charge Capacity (Fix)**
        string fullChargeCap = maxCapacity != "Unknown" ? maxCapacity + " mAh" : "Unknown";

        // **Voltage Conversion**
        voltage = voltage != "Unknown" ? voltage + " mV" : "Unknown";

        // **Low Battery Warning (Below 20%)**
        string lowBatteryMessage = "";
        if (chargePercentage != "Unknown") {
            try {
                float chargeVal = stof(chargePercentage.substr(0, chargePercentage.length() - 1));
                if (chargeVal < 20) {
                    lowBatteryMessage = "⚠️ Warning: Battery low! Please plug in the charger.";
                }
            } catch (...) {}
        }


        string timeToFullCharge = "Calculating...";
        if (avgTimeToFull != "Unknown" && avgTimeToFull != "65535") {
            timeToFullCharge = avgTimeToFull + " min";
        }

        // **Output JSON**
        ostringstream json;
        json << "{\n";
        json << "  \"Battery Status\": \"" << batteryStatus << "\",\n";
        json << "  \"Battery Health\": \"" << batteryHealth << "\",\n";
        json << "  \"Current Charge (%)\": \"" << chargePercentage << "\",\n";
        json << "  \"Full Charge Capacity (mAh)\": \"" << designCapacity << "\",\n";
        json << "  \"Cycle Count\": \"" << cycleCount << "\",\n";
        json << "  \"Battery Temperature\": \"" << batteryTemp << "\",\n";
        json << "  \"Battery Voltage\": \"" << voltage << "\",\n";
        json << "  \"Battery Manufacturer\": \"" << manufacturer << "\",\n";
        json << "  \"Battery Serial Number\": \"" << serial << "\",\n";  
        json << "  \"Time to Full Charge\": \"" << timeToFullCharge << "\",\n";
        json << "  \"Power Adapter (W)\": \"" << wattage << " W\"\n";
        json << "}\n";

        if (!lowBatteryMessage.empty()) {
            cout << lowBatteryMessage << endl;
        }
        return json.str();
    }
}




// int main() {
//     while(true){
//    cout<< displayBatteryInfo();
//     // **Wait for 5 seconds before refreshing**

//     this_thread::sleep_for(chrono::seconds(5));
//     }
//     return 0;
// }