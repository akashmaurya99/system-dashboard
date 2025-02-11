class BatteryInfo {
  final double chargeLevel; // Percentage (0-100)
  final bool isCharging;
  final bool isConnectedToPower;
  final String health; // Example: "Good", "Bad", "Replace Soon"
  final String technology; // Example: "Lithium-ion"
  final int cycleCount; // Number of charge cycles
  final double temperature; // In Celsius
  final double voltage; // In volts
  final double currentCapacity; // In mAh
  final double maxCapacity; // In mAh
  final double designedCapacity; // In mAh
  final String powerSource; // "AC", "Battery"
  final String lastFullChargeTime;

  BatteryInfo({
    required this.chargeLevel,
    required this.isCharging,
    required this.isConnectedToPower,
    required this.health,
    required this.technology,
    required this.cycleCount,
    required this.temperature,
    required this.voltage,
    required this.currentCapacity,
    required this.maxCapacity,
    required this.designedCapacity,
    required this.powerSource,
    required this.lastFullChargeTime,
  });
}
