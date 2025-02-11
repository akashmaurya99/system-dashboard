class GpuInfo {
  final String gpuName;
  final String vendor; // NVIDIA, AMD, Apple, Intel
  final double memorySize; // GB
  final double coreClockSpeed; // MHz
  final double memoryClockSpeed; // MHz
  final double temperature; // Celsius
  final double usagePercentage; // 0-100%
  final double vramUsage; // GB
  final String driverVersion;
  final bool isIntegrated; // true = integrated, false = dedicated

  GpuInfo({
    required this.gpuName,
    required this.vendor,
    required this.memorySize,
    required this.coreClockSpeed,
    required this.memoryClockSpeed,
    required this.temperature,
    required this.usagePercentage,
    required this.vramUsage,
    required this.driverVersion,
    required this.isIntegrated,
  });
}
