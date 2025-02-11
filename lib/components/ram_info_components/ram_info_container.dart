import 'package:flutter/material.dart';
import 'package:provider/provider.dart';
import '../../provider/ram_info_provider.dart';
import '../../theme/container_color.dart';
import '../../theme/container_radius.dart';
import '../../utils/dynamic_size.dart';

class RamInfoContainer extends StatelessWidget {
  const RamInfoContainer({super.key});

  @override
  Widget build(BuildContext context) {
    return LayoutBuilder(builder: (context, constraints) {
      return Consumer<RamInfoProvider>(
        builder: (context, provider, child) {
          final ramInfo = provider.ramInfo;
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
                Row(
                  children: [
                    Icon(Icons.memory, color: Colors.blue.shade400),
                    const SizedBox(width: 8),
                    Text(
                      "RAM Info",
                      style: Theme.of(context).textTheme.titleMedium?.copyWith(
                            fontWeight: FontWeight.w600,
                          ),
                    ),
                  ],
                ),
                const SizedBox(height: 10),
                _infoRow("Total Memory", "${ramInfo.totalMemory} GB"),
                _infoRow("Used Memory", "${ramInfo.usedMemory} GB"),
                _infoRow("Free Memory", "${ramInfo.freeMemory} GB"),
                _infoRow("Swap Total", "${ramInfo.swapTotal} GB"),
                _infoRow("Swap Used", "${ramInfo.swapUsed} GB"),
                _infoRow("Usage",
                    "${ramInfo.memoryUsagePercentage.toStringAsFixed(2)}%"),
                _infoRow("Memory Speed", "${ramInfo.memorySpeed} MHz"),
                _infoRow("Memory Type", ramInfo.memoryType),
                _infoRow("Modules", "${ramInfo.moduleCount} Sticks"),
                _infoRow("CAS Latency", "CL${ramInfo.casLatency}"),
              ],
            ),
          );
        },
      );
    });
  }

  Widget _infoRow(String label, String value) {
    return Padding(
      padding: const EdgeInsets.only(bottom: 6),
      child: Row(
        mainAxisAlignment: MainAxisAlignment.spaceBetween,
        children: [
          Text(label,
              style: TextStyle(fontSize: 14, fontWeight: FontWeight.w500)),
          Text(value,
              style: TextStyle(fontSize: 14, fontWeight: FontWeight.w400)),
        ],
      ),
    );
  }
}
