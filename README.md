# System Dashboard

System Dashboard is an open-source application for macOS and Windows that provides real-time system information in a modern graphical interface.

## Features
- Displays CPU, RAM, Disk, GPU, Battery, and OS information
- Real-time CPU usage tracking
- Modern UI built with Flutter
- Cross-platform support (macOS & Windows)

## Tech Stack
- **Backend:** C++ (for low-level system data retrieval)
- **Frontend:** Flutter (for rendering the UI)

## Project Structure
```
system-dashboard/
|
├── assets/
|
|-- ffi/
|   │── mac/
|   │   ├── battery_info.cpp
|   │   ├── cpu_info.cpp
|   │   ├── disk_info.cpp
|   │   ├── gpu_info.cpp
|   │   ├── os_info.cpp
|   │   ├── ram_info.cpp
|   │   ├── running_app_info.cpp
|   │   ├── mac_system_info.cpp
|   │   ├── include/
|   │   │   ├── battery_info.h
|   │   │   ├── cpu_info.h
|   │   │   ├── disk_info.h
|   │   │   ├── gpu_info.h
|   │   │   ├── os_info.h
|   │   │   ├── ram_info.h
|   │   │   ├── running_app_info.h
|   │
|   │── windows/
|       ├── (Will contain C++ source files for Windows implementation)
|   
|-- lib/
|   |-- components/
|   |   |-- active_program_components/
|   |   |   |-- program_title.dart
|   |   |-- ram_chart_components/
|   |   |   |-- ram_legend.dart
|   |   |-- storage_chart_componrnts/
|   |   |   |-- storage_legend.dart
|   |   |-- system_info_components/
|   |   |   |-- hardware_info_section.dart
|   |   |   |-- info_row.dart
|   |   |   |-- system_info_section.dart
|   |   |-- indicator.dart
|   |   |-- system_info.dart
|   |
|   |-- models/
|   |   |-- active_program_model.dart
|   |-- provider/
|   |   |-- active_program_provider.dart
|   |   |-- cpu_provider.dart
|   |   |-- gpu_provider.dart
|   |   |-- system_info_provider.dart
|   |
|   |-- services/
|   |   |-- ffi_service.dart
|   |-- theme/
|   |   |-- app_color.dart
|   |   |-- container_color.dart
|   |   |-- container_radius.dart
|   |
|   |--ui/
|   |   |-- mac/
|   |   |   |-- mac_os_home_page.dart
|   |   |
|   |   |-- windows/
|   |       |-- windows_os_home_page.dart
|   |   
|   |-- utils/
|   |   |-- dynamic_size.dart
|   |
|   |-- widgets/
|   |   |-- active_programs_lists.dart
|   |   |-- cpu_uses_chart.dart
|   |   |-- gpu_uses_chart.dart
|   |   |-- ram_pie_chart.dart
|   |   |-- storage_pie_chart.dart
|   |   |-- system_info_container.dart
|   |
|   |-- main.dart
|
|-- pubspec.yaml
|
|-- LICENSE

```

## Setup & Build Instructions
### macOS
1. Install dependencies (Xcode, CMake, etc.)
2. Navigate to `ffi/mac/` and compile the C++ code
3. Run the Flutter app

### Windows
1. Install dependencies (Visual Studio, CMake, etc.)
2. Navigate to `ffi/windows/` and compile the C++ code
3. Run the Flutter app

## Contributions
Contributions are welcome! Feel free to open an issue or submit a pull request.

## License
[MIT License](LICENSE)

