import 'package:flutter/material.dart';
import 'package:system_info/components/indicator.dart';
import 'package:system_info/theme/app_color.dart';

class RamLegend extends StatelessWidget {
  const RamLegend({super.key});

  @override
  Widget build(BuildContext context) {
    return Column(
      mainAxisAlignment: MainAxisAlignment.center,
      crossAxisAlignment: CrossAxisAlignment.start,
      children: const <Widget>[
        Text("RAM",
            style: TextStyle(
                fontSize: 16,
                fontWeight: FontWeight.w500,
                color: AppColors.mainTextColor2)),
        SizedBox(height: 10),
        Indicator(color: AppColors.contentColorBlue, text: 'Used RAM'),
        SizedBox(height: 4),
        // Indicator(color: AppColors.contentColorYellow, text: 'User Data'),
        // SizedBox(height: 4),
        // Indicator(color: AppColors.contentColorPurple, text: 'App Data'),
        // SizedBox(height: 4),
        Indicator(color: AppColors.contentColorGrey, text: 'Free RAM'),
      ],
    );
  }
}
