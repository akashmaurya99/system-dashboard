import 'package:flutter/material.dart';

class SystemInfoProvider with ChangeNotifier {
  final Map<String, List<String>> _systemInfo = {
    "System Info": ["Mac M1", "Mac OS 15"],
    "Storage": ["SSD", "512GB"],
    "RAM": ["DDR4", "16GB"],
    "CPU": ["AMD Ryzen 5 800G", "8 Cores"],
    "GPU": ['GeForce RTX 5090', 'GB202', '2017 MHz'],
  };

  Map<String, List<String>> get systemInfo => _systemInfo;

  void updateInfo(String key, List<String> details) {
    _systemInfo[key] = details;
    notifyListeners(); // Notify UI of changes
  }
}
