import 'package:flutter/material.dart';
import 'package:macos_ui/macos_ui.dart';
import 'package:provider/provider.dart';
import '../../provider/active_program_provider.dart';
import '../../provider/ram_provider.dart';
import '../../provider/storage_provider.dart';
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
        Provider.of<RunningProgramProvider>(context, listen: false)
            .fetchRunningProcesses();
        return true;
      }),
      Future<bool>.sync(() {
        Provider.of<StorageProvider>(context, listen: false).fetchStorageData();
        return true;
      }),
      Future<bool>.sync(() {
        Provider.of<RamProvider>(context, listen: false).fetchRamInfo();
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
              // Show a loading indicator until the future completes.
              if (snapshot.connectionState == ConnectionState.waiting) {
                return const Center(child: CircularProgressIndicator());
              } else if (snapshot.hasError) {
                // In case of error, display an error message.
                return Center(child: Text("Error: ${snapshot.error}"));
              } else {
                // Data has loaded; build the main content.
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
                                const SizedBox(height: 20),
                                Row(
                                  children: const [
                                    StoragePieChart(),
                                    SizedBox(width: 20),
                                    RamPieChart(),
                                  ],
                                ),
                                const SizedBox(height: 20),
                                Row(
                                  children: const [
                                    CPUUsageChart(),
                                    SizedBox(width: 20),
                                    GPUUsageChart(),
                                  ],
                                ),
                              ],
                            ),
                            const SizedBox(width: 25),
                            Flexible(
                              child: SizedBox(
                                height: constraints.maxHeight,
                                child: const ActiveProgramsList(),
                              ),
                            ),
                          ],
                        ),
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
