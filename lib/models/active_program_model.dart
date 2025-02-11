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
    required this.state,
    required this.windowTitle,
  });

  factory ProgramInfo.fromJson(Map<String, dynamic> json) {
    return ProgramInfo(
      pid: json['pid'],
      name: json['name'],
      cpuUsage: json['cpuUsage'].toDouble(),
      memoryUsage: json['memoryUsage'],
      executablePath: json['executablePath'],
      startTime: DateTime.parse(json['startTime']),
      threadCount: json['threadCount'],
      parentPid: json['parentPid'],
      user: json['user'],
      state: json['state'],
      windowTitle: json['windowTitle'],
    );
  }
}
