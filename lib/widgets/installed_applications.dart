import 'package:flutter/material.dart';
import 'package:macos_ui/macos_ui.dart';
import 'package:provider/provider.dart';
import 'package:system_info/theme/container_color.dart';
import 'package:system_info/theme/container_radius.dart';
import '../provider/installed_apps_provider.dart';
import '../theme/app_color.dart';

class InstalledAppsPage extends StatefulWidget {
  const InstalledAppsPage({super.key});

  @override
  State<InstalledAppsPage> createState() => _InstalledAppsPageState();
}

class _InstalledAppsPageState extends State<InstalledAppsPage> {
  @override
  void initState() {
    super.initState();
    // Automatically fetch installed apps after the first frame.
    WidgetsBinding.instance.addPostFrameCallback((_) {
      Provider.of<InstalledAppsProvider>(context, listen: false)
          .fetchInstalledApps();
    });
  }

  @override
  Widget build(BuildContext context) {
    return Consumer<InstalledAppsProvider>(
      builder: (context, provider, child) {
        return Padding(
          padding: const EdgeInsets.all(10.0),
          child: Column(
            children: [
              Row(
                children: [
                  SizedBox(
                    width: 5,
                  ),
                  const Text(
                    "Installed Applications",
                    style: TextStyle(
                      fontSize: 22,
                      fontWeight: FontWeight.bold,
                      color: AppColors.mainTextColor2,
                    ),
                  ),
                ],
              ),
              SizedBox(
                height: 10,
              ),
              // Always show the search field at the top.
              MacosTextField(
                placeholder: 'Search Apps',
                placeholderStyle:
                    const TextStyle(color: AppColors.mainTextColor2),
                onChanged: (query) {
                  provider.setSearchQuery(query);
                },
                style: const TextStyle(
                  fontSize: 12,
                ),
                maxLines: 1,
                maxLength: 20,
                padding:
                    const EdgeInsets.symmetric(vertical: 8.0, horizontal: 10),
              ),

              const SizedBox(height: 16),
              // The area below the search field will show the loading indicator,
              // no results message, or the list of apps.
              Expanded(
                child: provider.isLoading
                    ? const Center(child: ProgressCircle())
                    : provider.apps.isEmpty
                        ? const Center(child: Text("No apps found."))
                        : ListView.builder(
                            itemCount: provider.apps.length,
                            itemBuilder: (context, index) {
                              final appName = provider.apps[index];
                              return Container(
                                margin:
                                    const EdgeInsets.symmetric(vertical: 4.0),
                                padding: const EdgeInsets.all(12.0),
                                decoration: BoxDecoration(
                                    borderRadius: ContainerRadius.primary,
                                    color: ContainerColor.primary),
                                child: Text(
                                  appName,
                                  style: const TextStyle(fontSize: 12.0),
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
  }
}
