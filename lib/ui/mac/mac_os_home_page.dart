import 'package:flutter/material.dart';
import 'package:macos_ui/macos_ui.dart';
import '../../widgets/active_programs_lists.dart';
import '../../widgets/cpu_uses_chart.dart';
import '../../widgets/gpu_uses_chart.dart';
import '../../widgets/ram_pie_chart.dart';
import '../../widgets/storage_pie_chart.dart';
import '../../widgets/system_info_container.dart';

class MacOSHomePage extends StatelessWidget {
  const MacOSHomePage({super.key});

  @override
  Widget build(BuildContext context) {
    return MacosWindow(
      backgroundColor: const Color.fromARGB(255, 241, 241, 244),
      child: MacosScaffold(
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
                            SystemInfoContainer(),
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
      ),
    );
  }
}
