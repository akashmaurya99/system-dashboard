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
import '../../provider/os_info_provider.dart';
import '../../provider/ram_info_provider.dart';
import '../../widgets/installed_applications.dart';

class SpecificationPage extends StatefulWidget {
  const SpecificationPage({super.key});

  @override
  State<SpecificationPage> createState() => _SpecificationPageState();
}

class _SpecificationPageState extends State<SpecificationPage> {
  @override
  void initState() {
    super.initState();
    // Automatically fetch installed apps after the first frame.
    WidgetsBinding.instance.addPostFrameCallback((_) {
      Provider.of<BatteryInfoProvider>(context, listen: false)
          .loadBatteryInfo();
    });

    // Automatically fetch os info after the first frame.
    WidgetsBinding.instance.addPostFrameCallback((_) {
      Provider.of<OSInfoProvider>(context, listen: false).loadOsInfo();
    });

    // Automatically fetch ram info after the first frame.
    WidgetsBinding.instance.addPostFrameCallback((_) {
      Provider.of<RamInfoProvider>(context, listen: false).loadRamInfo();
    });

    // Automatically fetch cpu info after the first frame.

    WidgetsBinding.instance.addPostFrameCallback((_) {
      Provider.of<CPUInfoProvider>(context, listen: false).loadCpuInfo();
    });

    // Automatically fetch gpu info after the first frame.

    WidgetsBinding.instance.addPostFrameCallback((_) {
      Provider.of<GpuInfoProvider>(context, listen: false).loadGpuInfo();
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
                return Row(
                  crossAxisAlignment: CrossAxisAlignment.start,
                  children: [
                    Expanded(
                      flex: 4,
                      child: SingleChildScrollView(
                        child: Column(
                          crossAxisAlignment: CrossAxisAlignment.start,
                          children: [
                            // The StoragePiChart's height is set relative to the available space.
                            Row(
                              children: [
                                OsInfoContainer(),
                                const SizedBox(width: 20),
                                DiskInfoContainer(),
                              ],
                            ),
                            SizedBox(
                              height: 20,
                            ),
                            Row(
                              children: [
                                RamInfoContainer(),
                                const SizedBox(width: 20),
                                GpuInfoContainer(),
                              ],
                            ),
                            SizedBox(
                              height: 20,
                            ),
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
        }),
      ],
    );
  }
}
