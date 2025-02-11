import 'package:flutter/material.dart';
import 'package:provider/provider.dart';
import '../../provider/disk_info_provider.dart';
import '../../theme/container_color.dart';
import '../../theme/container_radius.dart';
import '../../utils/dynamic_size.dart';

class DiskInfoContainer extends StatelessWidget {
  const DiskInfoContainer({super.key});

  @override
  Widget build(BuildContext context) {
    return LayoutBuilder(
      builder: (context, constraints) {
        return Consumer<DiskInfoProvider>(
          builder: (context, diskProvider, child) {
            final diskInfo = diskProvider.diskInfo;
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
                      Icon(Icons.storage, color: Colors.blue.shade400),
                      const SizedBox(width: 8),
                      Text(
                        "Disk Info",
                        style:
                            Theme.of(context).textTheme.titleMedium?.copyWith(
                                  fontWeight: FontWeight.w600,
                                ),
                      ),
                    ],
                  ),
                  const SizedBox(height: 10),
                  // Disk Details
                  _infoRow("Disk Name", diskInfo.diskName),
                  _infoRow("File System", diskInfo.fileSystemType),
                  _infoRow("Total Space",
                      "${diskInfo.totalSpace.toStringAsFixed(1)} GB"),
                  _infoRow("Used Space",
                      "${diskInfo.usedSpace.toStringAsFixed(1)} GB"),
                  _infoRow("Free Space",
                      "${diskInfo.freeSpace.toStringAsFixed(1)} GB"),
                  _infoRow("Read Speed",
                      "${diskInfo.readSpeed.toStringAsFixed(1)} MB/s"),
                  _infoRow("Write Speed",
                      "${diskInfo.writeSpeed.toStringAsFixed(1)} MB/s"),
                  _infoRow("SSD", diskInfo.isSSD ? "Yes" : "No"),
                  _infoRow(
                      "Partitions",
                      diskInfo.partitionCount == 0
                          ? "Not Available"
                          : "${diskInfo.partitionCount}"),
                  _infoRow(
                      "Temperature",
                      diskInfo.diskTemperature == 0
                          ? "Not Available"
                          : "${diskInfo.diskTemperature} °C"),
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
