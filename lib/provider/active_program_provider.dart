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

// import 'dart:convert';
// import 'package:flutter/foundation.dart';
// import '../models/active_program_model.dart';
// import '../services/macos_system_info.dart';

// class ProgramProvider extends ChangeNotifier {
//   final MacSystemInfo _systemInfo = MacSystemInfo();

//   List<ProgramInfo> _programs = [];
//   List<ProgramInfo> get programs => _programs;

//   ProgramProvider() {
//     fetchRunningProcesses();
//   }

//   /// Fetches running processes using FFI and updates the list
//   Future<void> fetchRunningProcesses() async {
//     try {
//       String jsonString = _systemInfo.getRunningProcesses();
//       debugPrint("Raw JSON from FFI: $jsonString");

//       // Validate response
//       if (jsonString.isEmpty) {
//         debugPrint("Error: Received empty JSON string from FFI.");
//         return;
//       }

//       // Decode JSON as a Map since it's wrapped in an object
//       final Map<String, dynamic> jsonData = jsonDecode(jsonString);

//       // Extract the list from 'runningProcesses'
//       if (!jsonData.containsKey("runningProcesses") ||
//           jsonData["runningProcesses"] is! List) {
//         debugPrint(
//             "Error: JSON format is incorrect. Expected 'runningProcesses' list.");
//         return;
//       }

//       final List<dynamic> processList = jsonData["runningProcesses"];

//       // Convert to ProgramInfo list
//       List<ProgramInfo> newPrograms =
//           processList.map((item) => ProgramInfo.fromJson(item)).toList();

//       _programs = newPrograms;
//       notifyListeners();
//     } catch (e) {
//       debugPrint("Error fetching running processes: $e");
//     }
//   }
// }

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
