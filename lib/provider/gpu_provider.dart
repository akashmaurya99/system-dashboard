import 'dart:async';
import 'package:fl_chart/fl_chart.dart';
import 'package:flutter/foundation.dart';
import '../services/macos_system_info.dart';

class GPUUsageProvider extends ChangeNotifier {
  final List<FlSpot> _gpuData = [];
  double _currentTime = 0;
  final double _maxWindow = 30; // Keep last 30 seconds of data
  Timer? _timer;

  GPUUsageProvider() {
    _startDataStream();
  }

  List<FlSpot> get gpuData => _gpuData;
  double get currentTime => _currentTime;
  double get maxWindow => _maxWindow;

  void _startDataStream() {
    _timer = Timer.periodic(const Duration(seconds: 1), (timer) async {
      final usage = await _fetchGpuUsage();
      _currentTime += 1;
      _gpuData.add(FlSpot(_currentTime, usage));

      // Remove old data points outside the time window
      while (
          _gpuData.isNotEmpty && _gpuData.first.x < _currentTime - _maxWindow) {
        _gpuData.removeAt(0);
      }

      notifyListeners();
    });
  }

  Future<double> _fetchGpuUsage() async {
    try {
      return MacSystemInfo().getGpuUsage(); // Fetch real GPU usage from FFI
    } catch (e) {
      if (kDebugMode) {
        print("Error fetching GPU usage: $e");
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

// import 'dart:async';
// import 'dart:math';
// import 'package:fl_chart/fl_chart.dart';
// import 'package:flutter/material.dart';

// class GPUUsageProvider extends ChangeNotifier {
//   final List<FlSpot> _cpuData = [];
//   double _currentTime = 0;
//   final double _maxWindow = 30; // Keep last 30 seconds of data
//   Timer? _timer;

//   GPUUsageProvider() {
//     _startDataStream();
//   }

//   List<FlSpot> get cpuData => _cpuData;
//   double get currentTime => _currentTime;
//   double get maxWindow => _maxWindow;

//   void _startDataStream() {
//     _timer = Timer.periodic(const Duration(milliseconds: 500), (timer) {
//       final usage = _getCpuUsage();
//       _currentTime += 0.5;
//       _cpuData.add(FlSpot(_currentTime, usage));

//       // Remove old data points outside the time window
//       while (
//           _cpuData.isNotEmpty && _cpuData.first.x < _currentTime - _maxWindow) {
//         _cpuData.removeAt(0);
//       }

//       notifyListeners();
//     });
//   }

//   double _getCpuUsage() {
//     final random = Random();
//     return (random.nextDouble() * 100).clamp(0, 100);
//   }

//   @override
//   void dispose() {
//     _timer?.cancel();
//     super.dispose();
//   }
// }
