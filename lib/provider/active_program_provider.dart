import 'package:flutter/foundation.dart';
import '../models/active_program_model.dart';

class ProgramProvider extends ChangeNotifier {
  List<ProgramInfo> _programs = [
    ProgramInfo(
      pid: 303,
      name: "Terminal",
      cpuUsage: 3.1,
      memoryUsage: 200000,
      executablePath: "/System/Applications/Utilities/Terminal.app",
      startTime: DateTime.now().subtract(const Duration(minutes: 45)),
      threadCount: 10,
      parentPid: 1,
      user: "John Doe",
      diskRead: 300,
      diskWrite: 150,
      state: "Running",
      windowTitle: "Terminal",
    )
  ];

  List<ProgramInfo> get programs => _programs;

  /// Function to update the program list (will be called after fetching data from C++)
  void updatePrograms(List<ProgramInfo> newPrograms) {
    _programs = newPrograms;
    notifyListeners();
  }
}

// import 'dart:async';
// import 'dart:convert';
// import 'package:flutter/foundation.dart';
// import '../models/active_program_model.dart';
// import '../services/macos_system_info.dart';

// class ProgramProvider extends ChangeNotifier {
//   List<ProgramInfo> _programs = [];
//   Timer? _timer;

//   ProgramProvider() {
//     _startDataStream();
//   }

//   List<ProgramInfo> get programs => _programs;

//   void _startDataStream() {
//     _timer = Timer.periodic(const Duration(milliseconds: 200), (timer) {
//       _fetchRunningProcesses();
//     });
//   }

//   void _fetchRunningProcesses() {
//     try {
//       final rawData = MacSystemInfo().getRunningProcesses();
//       if (kDebugMode) {
//         print("Raw Data from MacSystemInfo: $rawData");
//       }

//       final Map<String, dynamic> jsonMap = jsonDecode(rawData);
//       final List<dynamic> jsonData = jsonMap["running_processes"] ?? [];

//       _programs = jsonData
//           .map((e) => ProgramInfo(
//                 pid: e['pid'] ?? 0,
//                 name: e['name'] ?? 'Unknown',
//                 cpuUsage: (e['cpuUsage'] as num?)?.toDouble() ?? 0.0,
//                 memoryUsage: e['memoryUsage'] ?? 0,
//                 executablePath: e['executablePath'] ?? '',
//                 startTime:
//                     DateTime.tryParse(e['startTime'] ?? '') ?? DateTime.now(),
//                 threadCount: e['threadCount'] ?? 0,
//                 parentPid: e['parentPid'] ?? 0,
//                 user: e['user'] ?? 'Unknown',
//                 diskRead: e['diskRead'] ?? 0,
//                 diskWrite: e['diskWrite'] ?? 0,
//                 state: e['state'] ?? 'Unknown',
//                 windowTitle: e['windowTitle'] ?? '',
//               ))
//           .toList();

//       notifyListeners();
//     } catch (e) {
//       if (kDebugMode) {
//         print("❌ Error fetching running processes: $e");
//       }
//     }
//   }

//   @override
//   void dispose() {
//     _timer?.cancel();
//     super.dispose();
//   }
// }
