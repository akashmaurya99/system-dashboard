import 'package:flutter/material.dart';

import '../services/macos_system_info.dart';

void main() {
  runApp(const MyApp());
}

class MyApp extends StatelessWidget {
  const MyApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Mac System Info',
      theme: ThemeData.dark(),
      home: const SystemInfoScreen(),
    );
  }
}

class SystemInfoScreen extends StatefulWidget {
  const SystemInfoScreen({super.key});

  @override
  _SystemInfoScreenState createState() => _SystemInfoScreenState();
}

class _SystemInfoScreenState extends State<SystemInfoScreen> {
  final MacSystemInfo _systemInfo = MacSystemInfo();
  String _cpuInfo = 'Fetching...';
  double _cpuUsage = 0.0;
  String _ramInfo = 'Fetching...';

  @override
  void initState() {
    super.initState();
    _fetchSystemInfo();
  }

  void _fetchSystemInfo() async {
    setState(() {
      _cpuInfo = _systemInfo.getCpuInfo();
      _cpuUsage = _systemInfo.getCpuUsage();
      _ramInfo = _systemInfo.getRamInfo();
    });
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(title: const Text('Mac System Info')),
      body: Padding(
        padding: const EdgeInsets.all(16.0),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Text('CPU Info: $_cpuInfo', style: const TextStyle(fontSize: 16)),
            const SizedBox(height: 10),
            Text('CPU Usage: ${_cpuUsage.toStringAsFixed(2)}%',
                style: const TextStyle(fontSize: 16)),
            const SizedBox(height: 10),
            Text('RAM Info: $_ramInfo', style: const TextStyle(fontSize: 16)),
          ],
        ),
      ),
      floatingActionButton: FloatingActionButton(
        onPressed: _fetchSystemInfo,
        child: const Icon(Icons.refresh),
      ),
    );
  }
}
