import 'dart:convert';
import 'package:flutter/material.dart';
import '../models/ram_info_model.dart';
import '../services/macos_system_info.dart';

class RamInfoProvider extends ChangeNotifier {
  RamInfo _ramInfo = RamInfo(
    totalMemory: 0.0,
    usedMemory: 0.0,
    freeMemory: 0.0,
    swapTotal: 0.0,
    swapUsed: 0.0,
    memoryUsagePercentage: 0.0,
    memorySpeed: 0,
    memoryType: "Unknown",
    moduleCount: 0,
    casLatency: 0,
  );

  RamInfo get ramInfo => _ramInfo;

  /// Update the RAM info and notify listeners.
  void updateRamInfo(RamInfo newRamInfo) {
    _ramInfo = newRamInfo;
    notifyListeners();
  }

  /// Loads RAM info from the native FFI call, parses the JSON, and updates the provider.
  void loadRamInfo() {
    // Call the native FFI function to get a JSON string.
    // (Make sure your FFI binding function name matches—here we assume it's `getRamInfo()`.)
    final String ramInfoJson = MacSystemInfo().getRamInfo();

    // Parse the JSON string.
    final Map<String, dynamic> jsonData = jsonDecode(ramInfoJson);

    // Create a new RamInfo instance from the JSON values.
    final RamInfo newRamInfo = RamInfo(
      totalMemory: (jsonData['totalMemory'] as num).toDouble(),
      usedMemory: (jsonData['usedMemory'] as num).toDouble(),
      freeMemory: (jsonData['freeMemory'] as num).toDouble(),
      swapTotal: (jsonData['swapTotal'] as num).toDouble(),
      swapUsed: (jsonData['swapUsed'] as num).toDouble(),
      memoryUsagePercentage:
          (jsonData['memoryUsagePercentage'] as num).toDouble(),
      memorySpeed: jsonData['memorySpeed'] as int,
      memoryType: jsonData['memoryType'] as String,
      moduleCount: jsonData['moduleCount'] as int,
      casLatency: jsonData['casLatency'] as int,
    );

    // Update the provider state.
    updateRamInfo(newRamInfo);
  }
}

// import 'package:flutter/material.dart';
// import '../models/ram_info_model.dart';

// class RamInfoProvider extends ChangeNotifier {
//   RamInfo _ramInfo = RamInfo(
//     totalMemory: 0.0,
//     usedMemory: 0.0,
//     freeMemory: 0.0,
//     swapTotal: 0.0,
//     swapUsed: 0.0,
//     memoryUsagePercentage: 0.0,
//     memorySpeed: 0,
//     memoryType: "Unknown",
//     moduleCount: 0,
//     casLatency: 0,
//   );

//   RamInfo get ramInfo => _ramInfo;

//   void updateRamInfo(RamInfo newRamInfo) {
//     _ramInfo = newRamInfo;
//     notifyListeners();
//   }
// }
