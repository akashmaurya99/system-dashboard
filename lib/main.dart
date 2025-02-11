import 'package:macos_ui/macos_ui.dart';
import 'package:fluent_ui/fluent_ui.dart';
import 'package:provider/provider.dart';
import 'package:screen_retriever/screen_retriever.dart';
import 'package:system_info/provider/cpu_provider.dart';
import 'package:system_info/ui/mac/mac_os_entry.dart';
import 'package:system_info/ui/window/windows_home_page.dart';
import 'dart:io';
import 'package:window_manager/window_manager.dart';
import 'provider/battery_info_provider.dart';
import 'provider/cpu_info_provider.dart';
import 'provider/disk_info_provider.dart';
import 'provider/gpu_info_provider.dart';
import 'provider/installed_apps_provider.dart';
import 'provider/os_info_provider.dart';
import 'provider/ram_info_provider.dart';
import 'provider/ram_provider.dart';
import 'provider/storage_provider.dart';
import 'provider/active_program_provider.dart';
import 'provider/gpu_provider.dart';
import 'provider/system_info_provider.dart';
import 'theme/theme_provider.dart';

void main() async {
  WidgetsFlutterBinding.ensureInitialized();
  // Must add this line for Windows and Linux
  await windowManager.ensureInitialized();

  // Fetch the primary display's size
  Display primaryDisplay = await screenRetriever.getPrimaryDisplay();
  final screenSize = primaryDisplay.size;

  // Calculate a size slightly smaller than full screen with a good aspect ratio
  // Here, we're reducing the dimensions by 5% on each side

  // Calculate a size slightly smaller than full screen with a good aspect ratio
  // Here, we're reducing the dimensions by 5% on each side
  final reducedWidth = screenSize.width * 0.9;
  final reducedHeight = screenSize.height * 0.9;

  final Size initialSize = Size(
    reducedWidth,
    reducedHeight,
  );

  WindowOptions windowOptions = WindowOptions(
    size: initialSize,
    // Minimum size can be slightly smaller than the initial size
    minimumSize: Size(initialSize.width * 0.8, initialSize.height * 0.8),
    titleBarStyle: TitleBarStyle.hidden,
    backgroundColor:
        Colors.transparent, // Make the window background transparent
  );

  windowManager.waitUntilReadyToShow(windowOptions, () async {
    await windowManager.show();
    await windowManager.focus();
  });
  runApp(
    AppEnteryPoint(),
  );
}

class AppEnteryPoint extends StatelessWidget {
  const AppEnteryPoint({super.key});

  @override
  Widget build(BuildContext context) => ChangeNotifierProvider(
      create: (context) => ThemeProvider(),
      builder: (context, _) {
        return MultiProvider(
            providers: [
              ChangeNotifierProvider(create: (_) => CPUUsageProvider()),
              ChangeNotifierProvider(create: (_) => GPUUsageProvider()),
              ChangeNotifierProvider(create: (_) => StorageProvider()),
              ChangeNotifierProvider(create: (_) => RamProvider()),
              ChangeNotifierProvider(create: (_) => ProgramProvider()),
              ChangeNotifierProvider(create: (_) => SystemInfoProvider()),
              ChangeNotifierProvider(create: (_) => InstalledAppsProvider()),
              ChangeNotifierProvider(create: (_) => RamInfoProvider()),
              ChangeNotifierProvider(create: (_) => BatteryInfoProvider()),
              ChangeNotifierProvider(create: (_) => CPUInfoProvider()),
              ChangeNotifierProvider(create: (_) => DiskInfoProvider()),
              ChangeNotifierProvider(create: (_) => OSInfoProvider()),
              ChangeNotifierProvider(create: (_) => GpuInfoProvider()),
            ],
            child: Platform.isMacOS
                ? MacosApp(
                    // title: 'My macOS App',
                    debugShowCheckedModeBanner: false,
                    theme: MacosThemeData.light(),
                    home: MacOsEntry(),
                  )
                : FluentApp(
                    debugShowCheckedModeBanner: false,
                    // title: 'My Windows App',
                    theme: FluentThemeData(brightness: Brightness.light),

                    home: WindowsHomePage(),
                  ));
      });
}
