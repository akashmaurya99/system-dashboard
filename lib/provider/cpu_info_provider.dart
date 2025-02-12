import 'dart:convert';
import 'package:flutter/material.dart';
import '../models/cpu_info_model.dart';
import '../services/macos_system_info.dart';

class CPUInfoProvider extends ChangeNotifier {
  // Initialize with default values.
  CpuInfo _cpuInfo = CpuInfo(
    processorName: "Unknown",
    coreCount: 0,
    threadCount: 0,
    baseClockSpeed: 0.0,
    currentClockSpeed: 0.0,
    temperature: 0.0,
    usagePercentage: 0.0,
    architecture: "Unknown",
    l1CacheSize: 0,
    l2CacheSize: 0,
    l3CacheSize: 0,
    vendor: "Unknown",
    instructionSet: "Unknown",
  );

  CpuInfo get cpuInfo => _cpuInfo;

  /// Call this method whenever you have new CPU data (e.g., from FFI).
  void updateCpuInfo(CpuInfo newCpuInfo) {
    _cpuInfo = newCpuInfo;
    notifyListeners();
  }

  /// Loads CPU info from the native FFI call, decodes the JSON,
  /// and updates the provider.
  Future<void> loadCpuInfo() async {
    // Get the JSON string from the native library.
    // (Assuming MacSystemInfo().getCpuInfo() returns the JSON string.)
    final String cpuJson = MacSystemInfo().getCpuInfo();

    // Decode the JSON string into a map.
    final Map<String, dynamic> data = jsonDecode(cpuJson);

    // Create a new CpuInfo instance using the decoded data.
    final CpuInfo newCpuInfo = CpuInfo(
      processorName: data['processorName'] as String? ?? "Unknown",
      coreCount:
          data['coreCount'] is num ? (data['coreCount'] as num).toInt() : 0,
      threadCount:
          data['threadCount'] is num ? (data['threadCount'] as num).toInt() : 0,
      baseClockSpeed: data['baseClockSpeed'] is num
          ? (data['baseClockSpeed'] as num).toDouble()
          : 0.0,
      currentClockSpeed: data['currentClockSpeed'] is num
          ? (data['currentClockSpeed'] as num).toDouble()
          : 0.0,
      temperature: data['temperature'] is num
          ? (data['temperature'] as num).toDouble()
          : 0.0,
      usagePercentage: data['usagePercentage'] is num
          ? (data['usagePercentage'] as num).toDouble()
          : 0.0,
      architecture: data['architecture'] as String? ?? "Unknown",
      l1CacheSize:
          data['l1CacheSize'] is num ? (data['l1CacheSize'] as num).toInt() : 0,
      l2CacheSize:
          data['l2CacheSize'] is num ? (data['l2CacheSize'] as num).toInt() : 0,
      l3CacheSize:
          data['l3CacheSize'] is num ? (data['l3CacheSize'] as num).toInt() : 0,
      vendor: data['vendor'] as String? ?? "Unknown",
      instructionSet: data['instructionSet'] as String? ?? "Unknown",
    );

    // Update the provider with the new data.
    updateCpuInfo(newCpuInfo);
  }
}
