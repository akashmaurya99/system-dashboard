import 'dart:convert';
import 'package:flutter/material.dart';
import 'package:fl_chart/fl_chart.dart';
import 'package:system_info/theme/app_color.dart';
import 'package:system_info/utils/dynamic_size.dart';
import '../services/macos_system_info.dart';

class StorageProvider extends ChangeNotifier {
  int touchedIndex = -1;
  double usedSpace = 0;
  double freeSpace = 0;
  double totalSpace = 1; // Prevent division by zero

  StorageProvider() {
    _fetchStorageData();
  }

  void handleTouch(FlTouchEvent event, PieTouchResponse? pieTouchResponse) {
    if (!event.isInterestedForInteractions ||
        pieTouchResponse == null ||
        pieTouchResponse.touchedSection == null) {
      touchedIndex = -1;
    } else {
      touchedIndex = pieTouchResponse.touchedSection!.touchedSectionIndex;
    }
    notifyListeners();
  }

  Future<void> _fetchStorageData() async {
    try {
      final String diskUsageStr = MacSystemInfo().getDiskUsage("/");
      final Map<String, dynamic> diskUsage = jsonDecode(diskUsageStr);

      totalSpace = _parseGB(diskUsage["Total Disk Size"]);
      usedSpace = _parseGB(diskUsage["Used Disk Space"]);
      freeSpace = _parseGB(diskUsage["Free Disk Space"]);

      notifyListeners();
    } catch (e) {
      print("Error fetching storage data: $e");
    }
  }

  /// Converts "245.107 GB" -> 245.107 (double)
  double _parseGB(String? sizeStr) {
    if (sizeStr == null) return 0;
    return double.tryParse(sizeStr.replaceAll(" GB", "")) ?? 0;
  }

  List<PieChartSectionData> showingSections(BuildContext context) {
    final colors = [
      AppColors.contentColorBlue, // Used
      AppColors.contentColorYellow, // Free
    ];

    final data = [usedSpace, freeSpace];

    return List.generate(data.length, (i) {
      final isTouched = i == touchedIndex;
      final fontSize = isTouched ? 14.0 : 10.0;
      final radius = isTouched
          ? dynamicWidth(context) * 0.035
          : dynamicWidth(context) * 0.03;
      const shadows = [Shadow(color: Colors.black, blurRadius: 2)];

      return PieChartSectionData(
        color: colors[i],
        value: data[i],
        title: '${data[i].toStringAsFixed(1)} GB',
        radius: radius,
        titleStyle: TextStyle(
          fontSize: fontSize,
          fontWeight: FontWeight.bold,
          color: AppColors.mainTextColor1,
          shadows: shadows,
        ),
      );
    });
  }
}

// // Provider for real-time storage data
// import 'package:fl_chart/fl_chart.dart';
// import 'package:flutter/material.dart';
// import 'package:system_info/theme/app_color.dart';
// import 'package:system_info/utils/dynamic_size.dart';

// class StorageProvider extends ChangeNotifier {
//   int touchedIndex = -1;

//   void handleTouch(FlTouchEvent event, PieTouchResponse? pieTouchResponse) {
//     if (!event.isInterestedForInteractions ||
//         pieTouchResponse == null ||
//         pieTouchResponse.touchedSection == null) {
//       touchedIndex = -1;
//     } else {
//       touchedIndex = pieTouchResponse.touchedSection!.touchedSectionIndex;
//     }
//     notifyListeners();
//   }

//   List<PieChartSectionData> showingSections(context) {
//     final data = [40, 30, 15, 15];
//     final colors = [
//       AppColors.contentColorBlue,
//       AppColors.contentColorYellow,
//       AppColors.contentColorPurple,
//       AppColors.contentColorGreen
//     ];

//     return List.generate(4, (i) {
//       final isTouched = i == touchedIndex;
//       final fontSize = isTouched ? 14.0 : 10.0;
//       final radius = isTouched
//           ? dynamicWidth(context) * 0.035
//           : dynamicWidth(context) * 0.03;
//       const shadows = [Shadow(color: Colors.black, blurRadius: 2)];
//       return PieChartSectionData(
//         color: colors[i],
//         value: data[i].toDouble(),
//         title: '${data[i]}%',
//         radius: radius,
//         titleStyle: TextStyle(
//           fontSize: fontSize,
//           fontWeight: FontWeight.bold,
//           color: AppColors.mainTextColor1,
//           shadows: shadows,
//         ),
//       );
//     });
//   }
// }
