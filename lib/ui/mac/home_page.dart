import 'package:flutter/material.dart';
import 'package:macos_ui/macos_ui.dart';
import 'package:provider/provider.dart';
import '../../provider/battery_info_provider.dart';
import '../../provider/installed_apps_provider.dart';
import '../../widgets/active_programs_lists.dart';
import '../../widgets/cpu_uses_chart.dart';
import '../../widgets/gpu_uses_chart.dart';
import '../../widgets/ram_pie_chart.dart';
import '../../widgets/storage_pie_chart.dart';

class HomePage extends StatefulWidget {
  const HomePage({super.key});

  @override
  State<HomePage> createState() => _HomePageState();
}

class _HomePageState extends State<HomePage> {
  @override
  void initState() {
    super.initState();
    // Automatically fetch installed apps after the first frame.
    WidgetsBinding.instance.addPostFrameCallback((_) {
      Provider.of<InstalledAppsProvider>(context, listen: false)
          .fetchInstalledApps();
    });

    // Automatically fetch system info after the first frame.
    WidgetsBinding.instance.addPostFrameCallback((_) {
      Provider.of<BatteryInfoProvider>(context, listen: false)
          .loadBatteryInfo();
    });
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
          return Padding(
            padding: const EdgeInsets.only(left: 20, top: 10, right: 20),
            child: LayoutBuilder(
              builder: (context, constraints) {
                return SingleChildScrollView(
                  controller: scrollController,
                  child: Row(
                    crossAxisAlignment: CrossAxisAlignment.start,
                    children: [
                      Column(
                        crossAxisAlignment: CrossAxisAlignment.start,
                        children: [
                          // The StoragePiChart's height is set relative to the available space.
                          // SystemInfoContainer(),
                          SizedBox(
                            height: 20,
                          ),

                          Row(
                            children: [
                              StoragePieChart(),
                              SizedBox(
                                width: 20,
                              ),
                              RamPieChart(),
                            ],
                          ),
                          // Add more responsive widgets here if needed
                          SizedBox(
                            height: 20,
                          ),
                          Row(
                            children: [
                              CPUUsageChart(),
                              SizedBox(
                                width: 20,
                              ),
                              GPUUsageChart(),
                            ],
                          ),
                        ],
                      ),
                      SizedBox(
                        width: 25,
                      ),
                      Flexible(
                        child: SizedBox(
                          height: constraints.maxHeight, // Define height
                          child: ActiveProgramsList(),
                        ),
                      ),
                    ],
                  ),
                );
              },
            ),
          );
        }),
      ],
    );
  }
}
