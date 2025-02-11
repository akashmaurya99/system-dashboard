import 'package:flutter/material.dart';
import 'package:provider/provider.dart';
import '../../provider/os_info_provider.dart';
import '../../theme/container_color.dart';
import '../../theme/container_radius.dart';
import '../../utils/dynamic_size.dart';

class OsInfoContainer extends StatelessWidget {
  const OsInfoContainer({super.key});

  @override
  Widget build(BuildContext context) {
    return LayoutBuilder(builder: (context, constraints) {
      return Consumer<OSInfoProvider>(
        builder: (context, osProvider, child) {
          final osInfo = osProvider.osInfo;
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
                      "OS Info",
                      style: Theme.of(context).textTheme.titleMedium?.copyWith(
                            fontWeight: FontWeight.w600,
                          ),
                    ),
                  ],
                ),
                const SizedBox(height: 10),
                // OS Details
                _infoRow("OS Name", osInfo.osName),
                _infoRow("OS Version", osInfo.osVersion),
                _infoRow("Build Number", osInfo.buildNumber),
                _infoRow("Kernel Version", osInfo.kernelVersion),
                _infoRow("Architecture", osInfo.is64Bit ? "64-bit" : "32-bit"),
                _infoRow("Uptime", osInfo.systemUptime),
                _infoRow("Device Name", osInfo.deviceName),
                _infoRow("Host Name", osInfo.hostName),
                _infoRow("User Name", osInfo.userName),
                _infoRow("Locale", osInfo.locale),
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
