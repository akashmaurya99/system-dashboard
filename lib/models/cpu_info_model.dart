class CpuInfo {
  final String processorName;
  final int coreCount;
  final int threadCount;
  final double baseClockSpeed; // GHz
  final double currentClockSpeed; // GHz
  final double temperature; // Celsius
  final double usagePercentage; // 0-100%
  final String architecture; // "x86_64", "ARM"
  final int l1CacheSize; // KB
  final int l2CacheSize; // KB
  final int l3CacheSize; // KB
  final String vendor; // Intel, AMD, Apple Silicon, etc.
  final String instructionSet; // "x86_64", "ARM64"

  CpuInfo({
    required this.processorName,
    required this.coreCount,
    required this.threadCount,
    required this.baseClockSpeed,
    required this.currentClockSpeed,
    required this.temperature,
    required this.usagePercentage,
    required this.architecture,
    required this.l1CacheSize,
    required this.l2CacheSize,
    required this.l3CacheSize,
    required this.vendor,
    required this.instructionSet,
  });
}
