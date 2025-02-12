import 'package:flutter/material.dart';
import 'package:macos_ui/macos_ui.dart';
import 'package:provider/provider.dart';
import 'package:system_info/components/battery_info_components/battery_info_container.dart';
import 'package:system_info/components/disk_info_components/disk_info_container.dart';
import 'package:system_info/components/gpu_info_compoents/gpu_info_container.dart';
import 'package:system_info/components/ram_info_components/ram_info_container.dart';
import '../../components/cpu_info_components/cpu_info_container.dart';
import '../../components/os_info_componets/os_info_container.dart';
import '../../provider/battery_info_provider.dart';
import '../../provider/cpu_info_provider.dart';
import '../../provider/gpu_info_provider.dart';
import '../../provider/installed_apps_provider.dart';
import '../../provider/os_info_provider.dart';
import '../../provider/ram_info_provider.dart';
import '../../widgets/installed_applications.dart';

class SpecificationPage extends StatefulWidget {
  const SpecificationPage({super.key});

  @override
  State<SpecificationPage> createState() => _SpecificationPageState();
}

class _SpecificationPageState extends State<SpecificationPage> {
  late Future<void> _loadAllDataFuture;

  @override
  void initState() {
    super.initState();
    // Delay the data loading until after the build is complete.
    _loadAllDataFuture = Future.delayed(Duration.zero, _loadAllData);
  }

  Future<void> _loadAllData() async {
    await Future.wait<bool>([
      Future<bool>.sync(() {
        Provider.of<BatteryInfoProvider>(context, listen: false)
            .loadBatteryInfo();
        return true;
      }),
      Future<bool>.sync(() {
        Provider.of<OSInfoProvider>(context, listen: false).loadOsInfo();
        return true;
      }),
      Future<bool>.sync(() {
        Provider.of<RamInfoProvider>(context, listen: false).loadRamInfo();
        return true;
      }),
      Future<bool>.sync(() {
        Provider.of<CPUInfoProvider>(context, listen: false).loadCpuInfo();
        return true;
      }),
      Future<bool>.sync(() {
        Provider.of<GpuInfoProvider>(context, listen: false).loadGpuInfo();
        return true;
      }),
      Future<bool>.sync(() {
        Provider.of<InstalledAppsProvider>(context, listen: false)
            .fetchInstalledApps();
        return true;
      }),
    ]);
  }

  @override
  Widget build(BuildContext context) {
    return MacosScaffold(
      backgroundColor: const Color.fromARGB(255, 241, 241, 244),
      toolBar: ToolBar(
        dividerColor: Colors.transparent,
      ),
      children: [
        ContentArea(builder: (context, scrollController) {
          return FutureBuilder(
            future: _loadAllDataFuture,
            builder: (context, snapshot) {
              if (snapshot.connectionState == ConnectionState.waiting) {
                // Display a loading indicator while data is being fetched.
                return const Center(child: CircularProgressIndicator());
              } else if (snapshot.hasError) {
                // In case of error, display an error message.
                return Center(child: Text("Error: ${snapshot.error}"));
              } else {
                // Data is loaded; build the main content.
                return Padding(
                  padding: const EdgeInsets.only(left: 20, top: 10, right: 20),
                  child: LayoutBuilder(
                    builder: (context, constraints) {
                      return Row(
                        crossAxisAlignment: CrossAxisAlignment.start,
                        children: [
                          Expanded(
                            flex: 4,
                            child: SingleChildScrollView(
                              child: Column(
                                crossAxisAlignment: CrossAxisAlignment.start,
                                children: [
                                  Row(
                                    children: [
                                      OsInfoContainer(),
                                      const SizedBox(width: 20),
                                      DiskInfoContainer(),
                                    ],
                                  ),
                                  const SizedBox(height: 20),
                                  Row(
                                    children: [
                                      RamInfoContainer(),
                                      const SizedBox(width: 20),
                                      GpuInfoContainer(),
                                    ],
                                  ),
                                  const SizedBox(height: 20),
                                  Row(
                                    children: [
                                      CpuInfoContainer(),
                                      const SizedBox(width: 20),
                                      BatteryInfoContainer(),
                                    ],
                                  ),
                                ],
                              ),
                            ),
                          ),
                          Expanded(flex: 2, child: InstalledAppsPage()),
                        ],
                      );
                    },
                  ),
                );
              }
            },
          );
        }),
      ],
    );
  }
}
