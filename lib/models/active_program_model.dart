class ProgramInfo {
  final int pid;
  final String name;
  final double cpuUsage;
  final int memoryUsage;
  final String executablePath;
  final DateTime startTime;
  final int threadCount;
  final int parentPid;
  final String user;
  final int diskRead;
  final int diskWrite;
  final String state;
  final String windowTitle;

  ProgramInfo({
    required this.pid,
    required this.name,
    required this.cpuUsage,
    required this.memoryUsage,
    required this.executablePath,
    required this.startTime,
    required this.threadCount,
    required this.parentPid,
    required this.user,
    required this.diskRead,
    required this.diskWrite,
    required this.state,
    required this.windowTitle,
  });
}
