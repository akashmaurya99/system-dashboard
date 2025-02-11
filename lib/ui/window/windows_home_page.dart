import 'package:fluent_ui/fluent_ui.dart';
import 'package:system_info/widgets/gpu_uses_chart.dart';
import '../../widgets/active_programs_lists.dart';
import '../../widgets/cpu_uses_chart.dart';
import '../../widgets/ram_pie_chart.dart';
import '../../widgets/system_info_container.dart';

class WindowsHomePage extends StatelessWidget {
  const WindowsHomePage({super.key});

  @override
  Widget build(BuildContext context) {
    return ScaffoldPage(
      resizeToAvoidBottomInset: false,
      content: Padding(
        padding: const EdgeInsets.only(left: 20, top: 20, right: 20),
        child: LayoutBuilder(
          builder: (context, constraints) {
            return SingleChildScrollView(
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
                          // StoragePieChart(),
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
      ),
    );
  }
}
