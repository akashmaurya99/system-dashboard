import 'dart:convert';
import 'package:flutter/material.dart';
import '../models/os_info_model.dart';
import '../services/macos_system_info.dart';

class OSInfoProvider extends ChangeNotifier {
  OsInfo _osInfo = OsInfo(
    osName: "Unknown",
    osVersion: "Unknown",
    buildNumber: "Unknown",
    kernelVersion: "Unknown",
    is64Bit: true,
    systemUptime: "Unknown",
    deviceName: "Unknown",
    hostName: "Unknown",
    userName: "Unknown",
    locale: "Unknown",
  );

  OsInfo get osInfo => _osInfo;

  /// Update the OS info with new data and notify listeners.
  void updateOsInfo(OsInfo newOsInfo) {
    _osInfo = newOsInfo;
    notifyListeners();
  }

  /// Loads OS info from the native FFI call.
  void loadOsInfo() {
    // Retrieve the JSON string from the native library.
    final String osInfoJson = MacSystemInfo().getOsInfo();

    // Decode the JSON string into a Map.
    final Map<String, dynamic> jsonData = jsonDecode(osInfoJson);

    // Create a new OsInfo instance from the parsed JSON.
    final OsInfo newOsInfo = OsInfo(
      osName: jsonData['osName'] as String? ?? "Unknown",
      osVersion: jsonData['osVersion'] as String? ?? "Unknown",
      buildNumber: jsonData['buildNumber'] as String? ?? "Unknown",
      kernelVersion: jsonData['kernelVersion'] as String? ?? "Unknown",
      is64Bit: jsonData['is64Bit'],
      systemUptime: jsonData['systemUptime'] as String? ?? "Unknown",
      deviceName: jsonData['deviceName'] as String? ?? "Unknown",
      hostName: jsonData['hostName'] as String? ?? "Unknown",
      userName: jsonData['userName'] as String? ?? "Unknown",
      locale: jsonData['locale'] as String? ?? "Unknown",
    );

    // Update provider state.
    updateOsInfo(newOsInfo);
  }
}
