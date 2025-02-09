import 'package:flutter/material.dart';
import '../services/macos_system_info.dart';

void main() {
  runApp(MyApp());
}

class MyApp extends StatelessWidget {
  const MyApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Mac System Info',
      home: Scaffold(
        appBar: AppBar(title: Text('Mac System Info')),
        body: SystemInfoWidget(),
      ),
    );
  }
}

class SystemInfoWidget extends StatefulWidget {
  const SystemInfoWidget({super.key});

  @override
  _SystemInfoWidgetState createState() => _SystemInfoWidgetState();
}

class _SystemInfoWidgetState extends State<SystemInfoWidget> {
  String _diskUsage = '';
  String _batteryInfo = '';
  String _cpuInfo = '';
  String _gpuInfo = '';

  @override
  void initState() {
    super.initState();
    loadSystemInfo();
  }

  void loadSystemInfo() {
    setState(() {
      _diskUsage = MacSystemInfo.getDiskUsage();
      _batteryInfo = MacSystemInfo.getBatteryInfo();
      _cpuInfo = MacSystemInfo.getCPUInfo();
      _gpuInfo = MacSystemInfo.getGPUInfo();
    });
  }

  @override
  Widget build(BuildContext context) {
    return Padding(
      padding: const EdgeInsets.all(16.0),
      child: SingleChildScrollView(
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Text('🖥 CPU Info:', style: TextStyle(fontWeight: FontWeight.bold)),
            Text(_cpuInfo),
            SizedBox(height: 10),
            Text('🎮 GPU Info:', style: TextStyle(fontWeight: FontWeight.bold)),
            Text(_gpuInfo),
            SizedBox(height: 10),
            Text('🔋 Battery Info:',
                style: TextStyle(fontWeight: FontWeight.bold)),
            Text(_batteryInfo),
            SizedBox(height: 10),
            Text('💾 Disk Usage:',
                style: TextStyle(fontWeight: FontWeight.bold)),
            Text(_diskUsage),
            SizedBox(height: 20),
            ElevatedButton(
              onPressed: loadSystemInfo,
              child: Text('Refresh'),
            ),
          ],
        ),
      ),
    );
  }
}
