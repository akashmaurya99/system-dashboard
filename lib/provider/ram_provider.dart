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

  Future<void> fetchRamInfo() async {
    final ramInfo = MacSystemInfo().getRamInfo();

    // final parsedData = _parseRamInfo(ramInfo);
    final Map<String, dynamic> ramUsage = jsonDecode(ramInfo);

    usedRam = ramUsage["usedMemory"] ?? 0.0;
    freeRam = ramUsage["freeMemory"] ?? 0.0;
    notifyListeners();
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

  List<PieChartSectionData> showingSections(BuildContext context) {
    final data = [usedRam, freeRam];
    final colors = [
      AppColors.contentColorBlue,
      AppColors.contentColorGrey,
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
