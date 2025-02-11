class RamInfo {
  final double totalMemory; // GB
  final double usedMemory; // GB
  final double freeMemory; // GB
  final double swapTotal; // GB
  final double swapUsed; // GB
  final double memoryUsagePercentage; // 0-100%
  final int memorySpeed; // MHz
  final String memoryType; // DDR4, DDR5, LPDDR5
  final int moduleCount; // Number of RAM sticks
  final int casLatency; // CL value

  RamInfo({
    required this.totalMemory,
    required this.usedMemory,
    required this.freeMemory,
    required this.swapTotal,
    required this.swapUsed,
    required this.memoryUsagePercentage,
    required this.memorySpeed,
    required this.memoryType,
    required this.moduleCount,
    required this.casLatency,
  });
}
