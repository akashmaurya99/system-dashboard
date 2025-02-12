import 'dart:async';
import 'package:fl_chart/fl_chart.dart';
import 'package:flutter/foundation.dart';
import '../services/macos_system_info.dart';

class CPUUsageProvider extends ChangeNotifier {
  final List<FlSpot> _cpuData = [];
  double _currentTime = 0;
  final double _maxWindow = 30; // Keep last 30 seconds of data
  Timer? _timer;

  CPUUsageProvider() {
    _startDataStream();
  }

  List<FlSpot> get cpuData => _cpuData;
  double get currentTime => _currentTime;
  double get maxWindow => _maxWindow;

  void _startDataStream() {
    _timer = Timer.periodic(const Duration(seconds: 1), (timer) async {
      final usage = await _fetchCpuUsage();
      _currentTime += 1;
      _cpuData.add(FlSpot(_currentTime, usage));

      // Remove old data points outside the time window
      while (
          _cpuData.isNotEmpty && _cpuData.first.x < _currentTime - _maxWindow) {
        _cpuData.removeAt(0);
      }

      notifyListeners();
    });
  }

  Future<double> _fetchCpuUsage() async {
    try {
      return MacSystemInfo().getCpuUsage(); // Get actual CPU usage from FFI
    } catch (e) {
      if (kDebugMode) {
        print("Error fetching CPU usage: $e");
      }
      return 0.0; // Default to 0 if error occurs
    }
  }

  @override
  void dispose() {
    _timer?.cancel();
    super.dispose();
  }
}
