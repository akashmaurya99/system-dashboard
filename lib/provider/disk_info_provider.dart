import 'dart:convert';
import 'package:flutter/material.dart';
import '../models/disk_info_model.dart';
import '../services/macos_system_info.dart';

class DiskInfoProvider extends ChangeNotifier {
  DiskInfo _diskInfo = DiskInfo(
    diskName: "Unknown",
    fileSystemType: "Unknown",
    totalSpace: 0.0,
    usedSpace: 0.0,
    freeSpace: 0.0,
    readSpeed: 0.0,
    writeSpeed: 0.0,
    isSSD: false,
    partitionCount: 0,
    diskTemperature: 0,
  );

  DiskInfo get diskInfo => _diskInfo;

  DiskInfoProvider() {
    fetchDiskInfo();
  }

  /// Fetch disk info from the FFI call
  Future<void> fetchDiskInfo() async {
    try {
      final String diskInfoStr = MacSystemInfo().getDiskDetails();
      final Map<String, dynamic> diskData = jsonDecode(diskInfoStr);

      _diskInfo = DiskInfo(
        diskName: diskData["diskName"] ?? "Unknown",
        fileSystemType: diskData["fileSystemType"] ?? "Unknown",
        totalSpace: (diskData["totalSpace"] as num?)?.toDouble() ?? 0.0,
        usedSpace: (diskData["usedSpace"] as num?)?.toDouble() ?? 0.0,
        freeSpace: (diskData["freeSpace"] as num?)?.toDouble() ?? 0.0,
        readSpeed: (diskData["readSpeed"] as num?)?.toDouble() ?? 0.0,
        writeSpeed: (diskData["writeSpeed"] as num?)?.toDouble() ?? 0.0,
        isSSD: diskData["isSSD"] ?? false,
        partitionCount: diskData["partitionCount"] ?? 0,
        diskTemperature: diskData["diskTemperature"] ?? 0,
      );

      notifyListeners();
    } catch (e) {
      print("Error fetching disk info: $e");
    }
  }
}

// import 'package:flutter/material.dart';
// import '../models/disk_info_model.dart';

// class DiskInfoProvider extends ChangeNotifier {
//   DiskInfo _diskInfo = DiskInfo(
//     diskName: "Unknown",
//     fileSystemType: "Unknown",
//     totalSpace: 0.0,
//     usedSpace: 0.0,
//     freeSpace: 0.0,
//     readSpeed: 0.0,
//     writeSpeed: 0.0,
//     isSSD: false,
//     partitionCount: 0,
//     diskTemperature: 0,
//   );

//   DiskInfo get diskInfo => _diskInfo;

//   /// Call this method to update the disk information (e.g., from FFI).
//   void updateDiskInfo(DiskInfo newDiskInfo) {
//     _diskInfo = newDiskInfo;
//     notifyListeners();
//   }
// }
