import 'package:fl_chart/fl_chart.dart';
import 'package:flutter/material.dart';
import 'package:provider/provider.dart';
import 'package:system_info/utils/dynamic_size.dart';
import '../provider/cpu_provider.dart';
import '../theme/container_color.dart';

class CPUUsageChart extends StatelessWidget {
  const CPUUsageChart({super.key});

  LineChartData _buildChartData(BuildContext context, List<FlSpot> cpuData,
      double currentTime, double maxWindow) {
    final isDark = Theme.of(context).brightness == Brightness.dark;
    final textColor = isDark ? Colors.white : Colors.black;
    final gridColor = isDark ? Colors.white24 : Colors.black12;

    return LineChartData(
      minX: currentTime - maxWindow,
      maxX: currentTime,
      minY: 0,
      maxY: 100,
      lineTouchData: LineTouchData(
        enabled: true,
        touchTooltipData: LineTouchTooltipData(
          getTooltipColor: (_) => Colors.blueGrey.withOpacity(0.8),
          getTooltipItems: (spots) => spots.map((spot) {
            return LineTooltipItem(
              '${spot.y.toStringAsFixed(1)}%',
              TextStyle(
                color: textColor,
                fontWeight: FontWeight.bold,
              ),
            );
          }).toList(),
        ),
      ),
      gridData: FlGridData(
        show: true,
        drawVerticalLine: true,
        horizontalInterval: 20,
        getDrawingHorizontalLine: (value) => FlLine(
          color: gridColor,
          strokeWidth: 0.5,
        ),
      ),
      titlesData: FlTitlesData(
        leftTitles: AxisTitles(
          sideTitles: SideTitles(
            showTitles: false,
            interval: 20,
            reservedSize: 40,
            getTitlesWidget: (value, _) => Text(
              '${value.toInt()}%',
              style: TextStyle(color: textColor, fontSize: 9),
            ),
          ),
        ),
        rightTitles:
            const AxisTitles(sideTitles: SideTitles(showTitles: false)),
        topTitles: const AxisTitles(sideTitles: SideTitles(showTitles: false)),
        bottomTitles:
            const AxisTitles(sideTitles: SideTitles(showTitles: false)),
      ),
      borderData: FlBorderData(
        show: true,
        border: Border.all(color: gridColor, width: 0.8),
      ),
      lineBarsData: [
        LineChartBarData(
          spots: cpuData,
          isCurved: true,
          curveSmoothness: 0.2,
          color: const Color.fromARGB(255, 65, 130, 241),
          barWidth: 1.5,
          belowBarData: BarAreaData(
            show: true,
            gradient: LinearGradient(
              colors: [
                Colors.blueAccent.withOpacity(0.4),
                Colors.lightBlueAccent.withOpacity(0.1),
              ],
              begin: Alignment.topCenter,
              end: Alignment.bottomCenter,
            ),
          ),
          preventCurveOvershootingThreshold: 10,
          dotData: FlDotData(show: false),
        ),
      ],
    );
  }

  @override
  Widget build(BuildContext context) {
    return LayoutBuilder(builder: (context, constraints) {
      return Consumer<CPUUsageProvider>(
        builder: (context, provider, child) {
          double widthFactor = 0.24;
          return Container(
            padding: EdgeInsets.only(top: 15, left: 15, right: 15, bottom: 40),
            width: dynamicWidth(context) * widthFactor,
            // height: dynamicHeight(context) * 0.25,
            decoration: BoxDecoration(
              color: ContainerColor.primary,
              borderRadius: BorderRadius.circular(16),
              boxShadow: [
                BoxShadow(
                  color: Colors.black.withOpacity(0.1),
                  blurRadius: 8,
                  spreadRadius: 1,
                ),
              ],
            ),
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                Row(
                  children: [
                    Icon(Icons.monitor_heart, color: Colors.blue.shade400),
                    const SizedBox(width: 8),
                    Text(
                      "CPU Utilization",
                      style: Theme.of(context).textTheme.titleMedium?.copyWith(
                            fontWeight: FontWeight.w600,
                          ),
                    ),
                  ],
                ),
                SizedBox(
                  height: 10,
                ),
                SizedBox(
                  height: dynamicHeight(context) * 0.15,
                  child: ClipRRect(
                    borderRadius: BorderRadius.circular(12),
                    child: provider.cpuData.isNotEmpty
                        ? LineChart(
                            _buildChartData(context, provider.cpuData,
                                provider.currentTime, provider.maxWindow),
                            duration: const Duration(milliseconds: 300),
                          )
                        : const Center(child: CircularProgressIndicator()),
                  ),
                ),
              ],
            ),
          );
        },
      );
    });
  }
}
