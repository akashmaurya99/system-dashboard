import 'package:flutter/material.dart';
import 'package:provider/provider.dart';
import '../../provider/gpu_info_provider.dart';
import '../../theme/app_color.dart';
import '../../theme/container_color.dart';
import '../../theme/container_radius.dart';
import '../../utils/dynamic_size.dart';

class GpuInfoContainer extends StatelessWidget {
  const GpuInfoContainer({super.key});

  @override
  Widget build(BuildContext context) {
    return LayoutBuilder(
      builder: (context, constraints) {
        return Consumer<GpuInfoProvider>(
          builder: (context, gpuProvider, child) {
            final gpuInfo = gpuProvider.gpuInfo;
            double widthFactor = 0.26;
            return Container(
              padding: const EdgeInsets.only(
                  top: 15, left: 15, right: 15, bottom: 40),
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
                      Icon(Icons.developer_board, color: Colors.blue.shade400),
                      const SizedBox(width: 8),
                      Text(
                        "GPU Info",
                        style: Theme.of(context)
                            .textTheme
                            .titleMedium
                            ?.copyWith(
                                fontWeight: FontWeight.w600,
                                color: AppColors.mainTextColor2),
                      ),
                    ],
                  ),
                  const SizedBox(height: 10),
                  // GPU Details
                  _infoRow("GPU Name", gpuInfo.gpuName),
                  _infoRow("Vendor", gpuInfo.vendor),
                  _infoRow(
                      "Memory Size",
                      gpuInfo.memorySize == 0.0
                          ? "Not Available"
                          : "${gpuInfo.memorySize.toStringAsFixed(1)} GB"),
                  _infoRow(
                      "Core Clock",
                      gpuInfo.coreClockSpeed == 0.0
                          ? "Not Available"
                          : "${gpuInfo.coreClockSpeed.toStringAsFixed(1)} MHz"),
                  _infoRow(
                      "Memory Clock",
                      gpuInfo.memoryClockSpeed == 0.0
                          ? "Not Available"
                          : "${gpuInfo.memoryClockSpeed.toStringAsFixed(1)} MHz"),
                  _infoRow(
                      "Temperature",
                      gpuInfo.temperature == 0.0
                          ? "Not Available"
                          : "${gpuInfo.temperature.toStringAsFixed(1)} °C"),
                  _infoRow(
                      "Usage",
                      gpuInfo.usagePercentage == 0.0
                          ? "Not Available"
                          : "${gpuInfo.usagePercentage.toStringAsFixed(1)}%"),
                  _infoRow(
                      "VRAM Usage",
                      gpuInfo.vramUsage == 0.0
                          ? "Not Available"
                          : "${gpuInfo.vramUsage.toStringAsFixed(1)} GB"),
                  _infoRow(
                      "Driver",
                      gpuInfo.vramUsage == 0.0
                          ? "Not Available"
                          : gpuInfo.driverVersion),
                  _infoRow("Type",
                      gpuInfo.isIntegrated ? "Integrated" : "Dedicated"),
                ],
              ),
            );
          },
        );
      },
    );
  }

  Widget _infoRow(String label, String value) {
    return Padding(
      padding: const EdgeInsets.only(bottom: 6),
      child: Row(
        mainAxisAlignment: MainAxisAlignment.spaceBetween,
        children: [
          Text(
            label,
            style: const TextStyle(
                fontSize: 14,
                fontWeight: FontWeight.w300,
                color: AppColors.mainTextColor2),
          ),
          Flexible(
            child: Text(
              value,
              style: const TextStyle(
                  fontSize: 14,
                  fontWeight: FontWeight.w300,
                  color: AppColors.mainTextColor2),
              textAlign: TextAlign.right,
            ),
          ),
        ],
      ),
    );
  }
}
