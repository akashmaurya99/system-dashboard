import 'dart:convert';
import 'package:flutter/foundation.dart';
import '../models/active_program_model.dart';
import '../services/macos_system_info.dart';

class RunningProgramProvider extends ChangeNotifier {
  List<ProgramInfo> _programs = [];

  List<ProgramInfo> get programs => _programs;

  Future<void> fetchRunningProcesses() async {
    try {
      // Get the JSON string from your native code.
      final jsonString = MacSystemInfo().getRunningProcesses();

      // Decode the JSON string into a map.
      final Map<String, dynamic> decoded = json.decode(jsonString);

      // Extract the list from the map using the "running_programs" key.
      final List<dynamic> jsonList = decoded['running_programs'];

      // Map each JSON object to a ProgramInfo model.
      _programs = jsonList
          .map((program) => ProgramInfo(
                pid: program['pid'] as int,
                name: program['name'] as String,
                cpuUsage: (program['cpuUsage'] as num).toDouble(),
                memoryUsage: program['memoryUsage'] as int,
                executablePath: program['executablePath'] as String,
                // Assuming your "startTime" field is a Unix timestamp in seconds.
                startTime: program['startTime'],
                threadCount: program['threadCount'] as int,
                parentPid: program['parentPid'] as int,
                user: program['user'] as String,
                state: program['state'] as String,
                windowTitle: program['windowTitle'] as String,
              ))
          .toList();

      notifyListeners();
    } catch (e) {
      debugPrint("Error fetching running processes: $e");
    }
  }
}
