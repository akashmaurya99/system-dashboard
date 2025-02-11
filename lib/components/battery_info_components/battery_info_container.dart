import 'package:flutter/material.dart';
import 'package:provider/provider.dart';
import '../../provider/battery_info_provider.dart';
import '../../theme/container_color.dart';
import '../../theme/container_radius.dart';
import '../../utils/dynamic_size.dart';

class BatteryInfoContainer extends StatelessWidget {
  const BatteryInfoContainer({super.key});

  @override
  Widget build(BuildContext context) {
    return LayoutBuilder(builder: (context, constraints) {
      return Consumer<BatteryInfoProvider>(
        builder: (context, batteryProvider, child) {
          final batteryInfo = batteryProvider.batteryInfo;
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
                    Icon(Icons.battery_std, color: Colors.blue.shade400),
                    const SizedBox(width: 8),
                    Text(
                      "Battery Info",
                      style: Theme.of(context).textTheme.titleMedium?.copyWith(
                            fontWeight: FontWeight.w600,
                          ),
                    ),
                  ],
                ),
                const SizedBox(height: 10),
                _infoRow("Charge Level",
                    "${batteryInfo.chargeLevel.toStringAsFixed(1)}%"),
                _infoRow("Charging", batteryInfo.isCharging ? "Yes" : "No"),
                _infoRow("Connected to Power",
                    batteryInfo.isConnectedToPower ? "Yes" : "No"),
                _infoRow("Health", batteryInfo.health),
                _infoRow("Technology", batteryInfo.technology),
                _infoRow("Cycle Count", "${batteryInfo.cycleCount}"),
                _infoRow("Temperature",
                    "${batteryInfo.temperature.toStringAsFixed(1)} °C"),
                _infoRow(
                    "Voltage", "${batteryInfo.voltage.toStringAsFixed(1)} V"),
                _infoRow("Current Capacity",
                    "${batteryInfo.currentCapacity.toStringAsFixed(1)} %"),
                _infoRow("Max Capacity",
                    "${batteryInfo.maxCapacity.toStringAsFixed(1)} mAh"),
                _infoRow("Designed Capacity",
                    "${batteryInfo.designedCapacity.toStringAsFixed(1)} mAh"),
                _infoRow("Power Source", batteryInfo.powerSource),
                _infoRow("Last Full Charge", batteryInfo.lastFullChargeTime),
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
