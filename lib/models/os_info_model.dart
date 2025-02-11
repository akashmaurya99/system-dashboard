class OsInfo {
  final String osName; // macOS, Windows, Linux
  final String osVersion;
  final String buildNumber;
  final String kernelVersion;
  final bool is64Bit;
  final String systemUptime; // e.g., "5 Days 12 Hours"
  final String deviceName;
  final String hostName;
  final String userName;
  final String locale;

  OsInfo({
    required this.osName,
    required this.osVersion,
    required this.buildNumber,
    required this.kernelVersion,
    required this.is64Bit,
    required this.systemUptime,
    required this.deviceName,
    required this.hostName,
    required this.userName,
    required this.locale,
  });
}
