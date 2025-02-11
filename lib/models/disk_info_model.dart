class DiskInfo {
  final String diskName;
  final String fileSystemType; // Example: "APFS", "NTFS", "ext4"
  final double totalSpace; // GB
  final double usedSpace; // GB
  final double freeSpace; // GB
  final double readSpeed; // MB/s
  final double writeSpeed; // MB/s
  final bool isSSD;
  final int partitionCount;
  final int diskTemperature; // Celsius

  DiskInfo({
    required this.diskName,
    required this.fileSystemType,
    required this.totalSpace,
    required this.usedSpace,
    required this.freeSpace,
    required this.readSpeed,
    required this.writeSpeed,
    required this.isSSD,
    required this.partitionCount,
    required this.diskTemperature,
  });
}
