import 'dart:convert';
import 'package:fl_chart/fl_chart.dart';
import 'package:flutter/material.dart';
import 'package:system_info/theme/app_color.dart';
import 'package:system_info/utils/dynamic_size.dart';
import '../services/macos_system_info.dart';

class RamProvider extends ChangeNotifier {
  int touchedIndex = -1;
  double usedRam = 0;
  double freeRam = 0;

  RamProvider() {
    fetchRamInfo();
  }

  void fetchRamInfo() {
    final ramInfo = MacSystemInfo().getRamInfo();

    // final parsedData = _parseRamInfo(ramInfo);
    final Map<String, dynamic> ramUsage = jsonDecode(ramInfo);

    usedRam = ramUsage["usedMemory"] ?? 0.0;
    freeRam = ramUsage["freeMemory"] ?? 0.0;
    notifyListeners();
  }

  // List<double> _parseRamInfo(String ramInfo) {
  //   try {
  //     final Map<String, dynamic> jsonData = json.decode(ramInfo);
  //     final usedMB = jsonData['Used_RAM_MB'] ?? 0;
  //     final freeMB = jsonData['Free_RAM_MB'] ?? 0;

  //     final usedGB = usedMB / 1024; // Convert MB to GB
  //     final freeGB = freeMB / 1024; // Convert MB to GB

  //     return [usedGB, freeGB];
  //   } catch (e) {
  //     return [0, 0];
  //   }
  // }

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

  List<PieChartSectionData> showingSections(BuildContext context) {
    final data = [usedRam, freeRam];
    final colors = [
      AppColors.contentColorBlue,
      AppColors.contentColorGreen,
    ];

    return List.generate(2, (i) {
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

// class RamProvider extends ChangeNotifier {
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
