import 'package:flutter/material.dart';
import 'package:provider/provider.dart';
import '../../provider/cpu_info_provider.dart';
import '../../theme/container_color.dart';
import '../../theme/container_radius.dart';
import '../../utils/dynamic_size.dart';

class CpuInfoContainer extends StatelessWidget {
  const CpuInfoContainer({super.key});

  @override
  Widget build(BuildContext context) {
    return LayoutBuilder(builder: (context, constraints) {
      return Consumer<CPUInfoProvider>(
        builder: (context, provider, child) {
          final cpuInfo = provider.cpuInfo;
          double widthFactor = 0.26;
          return Container(
            padding:
                const EdgeInsets.only(top: 15, left: 15, right: 15, bottom: 40),
            width: dynamicWidth(context) * widthFactor,
            decoration: BoxDecoration(
              color: ContainerColor.primary,
              borderRadius: ContainerRadius.primary,
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
                // Header Row
                Row(
                  children: [
                    Icon(Icons.computer, color: Colors.blue.shade400),
                    const SizedBox(width: 8),
                    Text(
                      "CPU Info",
                      style: Theme.of(context).textTheme.titleMedium?.copyWith(
                            fontWeight: FontWeight.w600,
                          ),
                    ),
                  ],
                ),
                const SizedBox(height: 10),
                // CPU Details
                _infoRow("Processor", cpuInfo.processorName),
                _infoRow("Cores", "${cpuInfo.coreCount}"),
                _infoRow("Threads", "${cpuInfo.threadCount}"),
                _infoRow(
                    "Base Clock",
                    cpuInfo.baseClockSpeed == 0.0
                        ? "Not Available"
                        : "${cpuInfo.baseClockSpeed.toStringAsFixed(2)} GHz"),
                _infoRow(
                    "Current Clock",
                    cpuInfo.currentClockSpeed == 0.0
                        ? "Not Available"
                        : "${cpuInfo.currentClockSpeed.toStringAsFixed(2)} GHz"),
                _infoRow(
                    "Temperature",
                    cpuInfo.temperature == 0.0
                        ? "Not Available"
                        : "${cpuInfo.temperature.toStringAsFixed(1)} °C"),
                _infoRow(
                    "Usage", "${cpuInfo.usagePercentage.toStringAsFixed(1)}%"),
                _infoRow("Architecture", cpuInfo.architecture),
                _infoRow("L1 Cache", "${cpuInfo.l1CacheSize} KB"),
                _infoRow("L2 Cache", "${cpuInfo.l2CacheSize} KB"),
                _infoRow(
                    "L3 Cache",
                    cpuInfo.l3CacheSize == 0.0
                        ? "Not Available"
                        : "${cpuInfo.l3CacheSize} KB"),
                _infoRow("Vendor", cpuInfo.vendor),
                _infoRow("Instruction Set", cpuInfo.instructionSet),
              ],
            ),
          );
        },
      );
    });
  }

  // Helper method to create a row for each piece of info.
  Widget _infoRow(String label, String value) {
    return Padding(
      padding: const EdgeInsets.only(bottom: 6),
      child: Row(
        mainAxisAlignment: MainAxisAlignment.spaceBetween,
        children: [
          Text(
            label,
            style: const TextStyle(fontSize: 14, fontWeight: FontWeight.w500),
          ),
          Flexible(
            child: Text(
              value,
              style: const TextStyle(fontSize: 14, fontWeight: FontWeight.w400),
              textAlign: TextAlign.right,
            ),
          ),
        ],
      ),
    );
  }
}
