import 'dart:convert';
import 'package:flutter/material.dart';
import '../models/gpu_info_model.dart';
import '../services/macos_system_info.dart';

class GpuInfoProvider extends ChangeNotifier {
  GpuInfo _gpuInfo = GpuInfo(
    gpuName: "Unknown",
    vendor: "Unknown",
    memorySize: 0.0,
    coreClockSpeed: 0.0,
    memoryClockSpeed: 0.0,
    temperature: 0.0,
    usagePercentage: 0.0,
    vramUsage: 0.0,
    driverVersion: "",
    isIntegrated: false,
  );

  GpuInfo get gpuInfo => _gpuInfo;

  /// Update the GPU info with new data (e.g., from an FFI call)
  void updateGpuInfo(GpuInfo newGpuInfo) {
    _gpuInfo = newGpuInfo;
    notifyListeners();
  }

  /// Loads GPU info from the native FFI call, decodes the JSON,
  /// and updates the provider.
  Future<void> loadGpuInfo() async {
    // Retrieve the JSON string from your native library.
    // (Assuming MacSystemInfo().getGpuInfo() returns the JSON string.)
    final String gpuJson = MacSystemInfo().getGpuInfo();

    // Decode the JSON string into a Dart Map.
    final Map<String, dynamic> data = jsonDecode(gpuJson);

    // Create a new GpuInfo instance using the decoded data.
    final GpuInfo newGpuInfo = GpuInfo(
      gpuName: data['gpuName'] as String? ?? "Unknown",
      vendor: data['vendor'] as String? ?? "Unknown",
      memorySize: data['memorySize'] is num
          ? (data['memorySize'] as num).toDouble()
          : 0.0,
      coreClockSpeed: data['coreClockSpeed'] is num
          ? (data['coreClockSpeed'] as num).toDouble()
          : 0.0,
      memoryClockSpeed: data['memoryClockSpeed'] is num
          ? (data['memoryClockSpeed'] as num).toDouble()
          : 0.0,
      temperature: data['temperature'] is num
          ? (data['temperature'] as num).toDouble()
          : 0.0,
      usagePercentage: data['usagePercentage'] is num
          ? (data['usagePercentage'] as num).toDouble()
          : 0.0,
      vramUsage: data['vramUsage'] is num
          ? (data['vramUsage'] as num).toDouble()
          : 0.0,
      driverVersion: data['driverVersion'] as String? ?? "",
      isIntegrated: data['isIntegrated'] as bool? ?? false,
    );

    // Update the provider.
    updateGpuInfo(newGpuInfo);
  }
}
