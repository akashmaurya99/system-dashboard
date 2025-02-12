import 'dart:async';
import 'dart:convert';
import 'package:flutter/foundation.dart';
import 'package:intl/intl.dart';
import '../models/battery_Info_model.dart';
import '../services/macos_system_info.dart';

class BatteryInfoProvider extends ChangeNotifier {
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
    lastFullChargeTime: "Unknown", // A readable string.
  );

  BatteryInfo get batteryInfo => _batteryInfo;

  Timer? _timer;

  BatteryInfoProvider() {
    // Immediately load battery info and then start auto-refresh.
    loadBatteryInfo();
    _startAutoRefresh();
  }

  /// Updates the battery info and notifies listeners.
  void updateBatteryInfo(BatteryInfo newBatteryInfo) {
    _batteryInfo = newBatteryInfo;
    notifyListeners();
  }

  /// Converts an ISO8601 string to a more human-readable format.
  String _formatLastFullChargeTime(String rawDate) {
    try {
      final DateTime parsedDate = DateTime.tryParse(rawDate) ?? DateTime.now();
      // Example format: "Feb 12, 2025 10:17 PM"
      return DateFormat('MMM d, y h:mm a').format(parsedDate);
    } catch (e) {
      if (kDebugMode) {
        print("Error formatting date: $e");
      }
      return rawDate;
    }
  }

  /// Loads battery info by calling the native FFI function, decoding the JSON,
  /// converting the lastFullChargeTime to a readable format, and updating the provider.
  Future<void> loadBatteryInfo() async {
    try {
      // Retrieve the JSON string from the native library.
      final String batteryInfoJson = MacSystemInfo().getBatteryInfo();

      // Decode the JSON string.
      final Map<String, dynamic> jsonData = jsonDecode(batteryInfoJson);

      // Format the lastFullChargeTime.
      final String rawDate = jsonData['lastFullChargeTime'] as String? ?? '';
      final String formattedDate = _formatLastFullChargeTime(rawDate);

      // Create a new BatteryInfo instance with the formatted date.
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
        lastFullChargeTime: formattedDate, // Store the formatted date.
      );

      // Update the provider with new battery info.
      updateBatteryInfo(newBatteryInfo);
    } catch (e) {
      if (kDebugMode) {
        debugPrint("Error fetching battery info: $e");
      }
    }
  }

  /// Starts a timer to fetch battery info every 1 minute.
  void _startAutoRefresh() {
    _timer = Timer.periodic(const Duration(minutes: 1), (timer) {
      loadBatteryInfo();
    });
  }

  @override
  void dispose() {
    _timer?.cancel();
    super.dispose();
  }
}
