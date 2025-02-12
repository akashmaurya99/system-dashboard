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

  Future<void> fetchStorageData() async {
    try {
      final String diskUsageStr = MacSystemInfo().getDiskDetails();
      final Map<String, dynamic> diskUsage = jsonDecode(diskUsageStr);

      usedSpace = diskUsage["usedSpace"] ?? 0;
      freeSpace = diskUsage["freeSpace"] ?? 0;

      notifyListeners();
    } catch (e) {
      print("Error fetching storage data: $e");
    }
  }

  List<PieChartSectionData> showingSections(BuildContext context) {
    final colors = [
      AppColors.contentColorBlue, // Used
      AppColors.contentColorGrey, // Free
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
