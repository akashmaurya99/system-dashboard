// Provider for real-time storage data
import 'package:fl_chart/fl_chart.dart';
import 'package:flutter/material.dart';
import 'package:system_info/theme/app_color.dart';
import 'package:system_info/utils/dynamic_size.dart';

class RamProvider extends ChangeNotifier {
  int touchedIndex = -1;

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

  List<PieChartSectionData> showingSections(context) {
    final data = [40, 30, 15, 15];
    final colors = [
      AppColors.contentColorBlue,
      AppColors.contentColorYellow,
      AppColors.contentColorPurple,
      AppColors.contentColorGreen
    ];

    return List.generate(4, (i) {
      final isTouched = i == touchedIndex;
      final fontSize = isTouched ? 14.0 : 10.0;
      final radius = isTouched
          ? dynamicWidth(context) * 0.035
          : dynamicWidth(context) * 0.03;
      const shadows = [Shadow(color: Colors.black, blurRadius: 2)];
      return PieChartSectionData(
        color: colors[i],
        value: data[i].toDouble(),
        title: '${data[i]}%',
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
