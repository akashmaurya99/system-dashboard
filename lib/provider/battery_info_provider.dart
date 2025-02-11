import 'dart:convert';
import 'package:flutter/material.dart';
import 'package:intl/intl.dart';
import '../models/battery_Info_model.dart';
import '../services/macos_system_info.dart';

class BatteryInfoProvider extends ChangeNotifier {
  // Note: In this updated version, we assume that the model's
  // lastFullChargeTime field is of type String so that we can store a formatted date.
  BatteryInfo _batteryInfo = BatteryInfo(
    chargeLevel: 0.0,
    isCharging: false,
    isConnectedToPower: false,
    health: "Unknown",
    technology: "Unknown",
    cycleCount: 0,
    temperature: 0.0,
    voltage: 0.0,
    currentCapacity: 0.0,
    maxCapacity: 0.0,
    designedCapacity: 0.0,
    powerSource: "Unknown",
    lastFullChargeTime: "Unknown", // Now a readable string.
  );

  BatteryInfo get batteryInfo => _batteryInfo;

  /// Updates the battery info and notifies listeners.
  void updateBatteryInfo(BatteryInfo newBatteryInfo) {
    _batteryInfo = newBatteryInfo;
    notifyListeners();
  }

  /// Loads battery info by calling the native FFI function,
  /// decodes the JSON, converts the lastFullChargeTime to a readable format,
  /// and updates the provider.
  void loadBatteryInfo() {
    // Retrieve the JSON string from the native library.
    final String batteryInfoJson = MacSystemInfo().getBatteryInfo();

    // Decode the JSON string.
    final Map<String, dynamic> jsonData = jsonDecode(batteryInfoJson);

    // Parse the lastFullChargeTime string (ISO8601) and convert it to a readable format.
    final String rawDate = jsonData['lastFullChargeTime'] as String? ?? '';
    final DateTime parsedDate = DateTime.tryParse(rawDate) ?? DateTime.now();
    final String formattedDate =
        DateFormat('MMM d, y h:mm a').format(parsedDate);

    // Create a new BatteryInfo instance with the formatted lastFullChargeTime.
    final BatteryInfo newBatteryInfo = BatteryInfo(
      chargeLevel: jsonData['chargeLevel'] is num
          ? (jsonData['chargeLevel'] as num).toDouble()
          : 0.0,
      isCharging: jsonData['isCharging'] as bool? ?? false,
      isConnectedToPower: jsonData['isConnectedToPower'] as bool? ?? false,
      health: jsonData['health'] as String? ?? "Unknown",
      technology: jsonData['technology'] as String? ?? "Unknown",
      cycleCount: jsonData['cycleCount'] is num
          ? (jsonData['cycleCount'] as num).toInt()
          : 0,
      temperature: jsonData['temperature'] is num
          ? (jsonData['temperature'] as num).toDouble()
          : 0.0,
      voltage: jsonData['voltage'] is num
          ? (jsonData['voltage'] as num).toDouble()
          : 0.0,
      currentCapacity: jsonData['currentCapacity'] is num
          ? (jsonData['currentCapacity'] as num).toDouble()
          : 0.0,
      maxCapacity: jsonData['maxCapacity'] is num
          ? (jsonData['maxCapacity'] as num).toDouble()
          : 0.0,
      designedCapacity: jsonData['designedCapacity'] is num
          ? (jsonData['designedCapacity'] as num).toDouble()
          : 0.0,
      powerSource: jsonData['powerSource'] as String? ?? "Unknown",
      lastFullChargeTime: formattedDate, // Store the formatted date string.
    );

    // Update the provider.
    updateBatteryInfo(newBatteryInfo);
  }
}
