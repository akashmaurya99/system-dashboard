import 'package:fl_chart/fl_chart.dart';
import 'package:flutter/material.dart';
import 'package:provider/provider.dart';
import 'package:system_info/theme/container_color.dart';
import 'package:system_info/theme/container_radius.dart';
import 'package:system_info/utils/dynamic_size.dart';
import '../components/ram_chart_components/ram_legend.dart';
import '../provider/ram_provider.dart';

class RamPieChart extends StatelessWidget {
  const RamPieChart({super.key});

  @override
  Widget build(BuildContext context) {
    return Consumer<RamProvider>(
      builder: (context, provider, child) {
        return LayoutBuilder(
          builder: (context, constraints) {
            double widthFactor = 0.24;
            return Container(
              padding:
                  EdgeInsets.symmetric(vertical: dynamicWidth(context) * 0.01),
              height: dynamicHeight(context) * 0.22,
              width: dynamicWidth(context) * widthFactor,
              decoration: BoxDecoration(
                color: ContainerColor.primary,
                borderRadius: ContainerRadius.primary,
              ),
              child: Row(
                mainAxisAlignment: MainAxisAlignment.spaceEvenly,
                children: [
                  AspectRatio(
                    aspectRatio: 1,
                    child: PieChart(
                      PieChartData(
                        pieTouchData: PieTouchData(
                          touchCallback: provider.handleTouch,
                        ),
                        borderData: FlBorderData(show: false),
                        sectionsSpace: 1,
                        centerSpaceRadius: 25,
                        sections: provider.showingSections(context),
                      ),
                    ),
                  ),
                  RamLegend(),
                ],
              ),
            );
          },
        );
      },
    );
  }
}
