import 'package:flutter/material.dart';
import 'package:provider/provider.dart';
import 'package:system_info/theme/app_color.dart';
import 'package:system_info/theme/container_color.dart';
import 'package:system_info/theme/container_radius.dart';
import '../provider/active_program_provider.dart';

class ActiveProgramsList extends StatefulWidget {
  const ActiveProgramsList({super.key});

  @override
  State<ActiveProgramsList> createState() => _ActiveProgramsListState();
}

class _ActiveProgramsListState extends State<ActiveProgramsList> {
  @override
  void initState() {
    super.initState();
  }

  @override
  Widget build(BuildContext context) {
    return Consumer<RunningProgramProvider>(
      builder: (context, provider, child) {
        return LayoutBuilder(
          builder: (context, constraints) {
            return Container(
              padding: const EdgeInsets.all(12),
              child: Column(
                crossAxisAlignment: CrossAxisAlignment.start,
                children: [
                  Row(
                    children: [
                      SizedBox(
                        width: 10,
                      ),
                      const Text(
                        "Active Programs",
                        style: TextStyle(
                          fontSize: 22,
                          fontWeight: FontWeight.bold,
                          color: AppColors.mainTextColor2,
                        ),
                      ),
                    ],
                  ),
                  const SizedBox(height: 12),
                  Expanded(
                    child: provider.programs.isEmpty
                        ? const Center(
                            child: Text(
                              "No active programs found",
                              style: TextStyle(color: AppColors.mainTextColor2),
                            ),
                          )
                        : ListView.builder(
                            itemCount: provider.programs.length,
                            itemBuilder: (context, index) {
                              final program = provider.programs[index];
                              return Card(
                                color: Colors.transparent,
                                shape: RoundedRectangleBorder(
                                    borderRadius: ContainerRadius
                                        .primary // Custom border radius
                                    ),
                                // elevation: 2,

                                // shadowColor: Colors.black.withOpacity(0.3),
                                child: Padding(
                                  padding: const EdgeInsets.all(1.0),
                                  child: Theme(
                                    data: Theme.of(context).copyWith(
                                      dividerColor: Colors
                                          .transparent, // Removes divider line
                                      splashColor: Colors
                                          .transparent, // Removes click effect
                                      highlightColor: Colors
                                          .transparent, // Removes highlight
                                    ),
                                    child: ExpansionTile(
                                      leading: SizedBox(
                                        width: 10,
                                      ),
                                      title: Text(
                                        program.name,
                                        style: TextStyle(
                                            fontSize: 14,
                                            color: AppColors.mainTextColor2),
                                      ),
                                      backgroundColor: ContainerColor.primary,
                                      collapsedBackgroundColor:
                                          ContainerColor.primary,
                                      shape: RoundedRectangleBorder(
                                          borderRadius: ContainerRadius
                                              .primary // Matching radius
                                          ),
                                      collapsedShape: RoundedRectangleBorder(
                                          borderRadius: ContainerRadius
                                              .primary // Matching collapsed radius
                                          ),
                                      subtitle: Text(
                                        "PID: ${program.pid} • CPU: ${program.cpuUsage.toStringAsFixed(2)}% • Memory: ${(program.memoryUsage / 1024).toStringAsFixed(1)} MB",
                                        style: const TextStyle(
                                            fontSize: 10,
                                            color: AppColors.mainTextColor2),
                                      ),
                                      children: [
                                        _infoTile("Executable Path",
                                            program.executablePath),
                                        _infoTile("Started", program.startTime),
                                        _infoTile("Thread Count",
                                            program.threadCount.toString()),
                                        _infoTile("User", program.user),
                                        _infoTile("State", program.state),
                                        // _infoTile("Window Title",
                                        //     program.windowTitle),
                                      ],
                                    ),
                                  ),
                                ),
                              );
                            },
                          ),
                  ),
                ],
              ),
            );
          },
        );
      },
    );
  }

  /// Helper function to create uniform list tiles with Material wrapper
  Widget _infoTile(String title, String value) {
    return Material(
      color: Colors.transparent,
      child: ListTile(
        contentPadding: const EdgeInsets.symmetric(horizontal: 16),
        title: Row(
          children: [
            Expanded(
              child: Text(
                "$title: ",
                style: TextStyle(fontSize: 13, color: AppColors.mainTextColor2),
                overflow: TextOverflow.clip,
              ),
            ),
            Expanded(
              child: Text(
                value,
                style: TextStyle(fontSize: 11, color: Colors.white70),
              ),
            ),
          ],
        ),
        dense: true, // Reduce tile height
        visualDensity: const VisualDensity(vertical: -3), // Compact layout
      ),
    );
  }
}
