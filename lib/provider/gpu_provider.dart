import 'dart:async';
import 'dart:math';
import 'package:fl_chart/fl_chart.dart';
import 'package:flutter/material.dart';

class GPUUsageProvider extends ChangeNotifier {
  final List<FlSpot> _cpuData = [];
  double _currentTime = 0;
  final double _maxWindow = 30; // Keep last 30 seconds of data
  Timer? _timer;

  GPUUsageProvider() {
    _startDataStream();
  }

  List<FlSpot> get cpuData => _cpuData;
  double get currentTime => _currentTime;
  double get maxWindow => _maxWindow;

  void _startDataStream() {
    _timer = Timer.periodic(const Duration(milliseconds: 500), (timer) {
      final usage = _getCpuUsage();
      _currentTime += 0.5;
      _cpuData.add(FlSpot(_currentTime, usage));

      // Remove old data points outside the time window
      while (
          _cpuData.isNotEmpty && _cpuData.first.x < _currentTime - _maxWindow) {
        _cpuData.removeAt(0);
      }

      notifyListeners();
    });
  }

  double _getCpuUsage() {
    final random = Random();
    return (random.nextDouble() * 100).clamp(0, 100);
  }

  @override
  void dispose() {
    _timer?.cancel();
    super.dispose();
  }
}
