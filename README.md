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
#### Prerequisites
- Install **Xcode** (for C++ compilation)
- Install **CMake** (for building shared libraries)
- Install **Flutter** (for UI development)

#### Steps
1. **Clone the Repository**:
   ```sh
   git clone https://github.com/akashmaurya99/system-dashboard.git
   cd system-dashboard
   ```

2. **Navigate to the macOS FFI Directory & Compile C++ Code**:
   ```sh
   cd ffi/mac/
   ```
   This folder contains multiple C++ source files responsible for gathering system data.

3. **Navigate to `macos/` and Build the Shared Library**:
   ```sh
   mkdir -p build && cd build
   cmake ..
   make -j$(sysctl -n hw.ncpu)
   ```
   This will generate `libmac_system_info.dylib`.

4. **Verify the Exposed C++ Functions**:
   ```sh
   nm -gU libmac_system_info.dylib
   ```
   This command lists all functions accessible via Flutter FFI.

5. **Ensure `libmac_system_info.dylib` is Copied to `build/macos/Build/Products/Debug/system_info.app/Contents/MacOS` to run app in Debug Mode**
   If the `.dylib` file is not found in the expected directory, manually move it:
   ```sh
   build/macos/Build/Products/Debug/system_info.app/Contents/MacOS
   ```
6. **Ensure `libmac_system_info.dylib` is Copied to `build/macos/Build/Products/Release/system_info.app/Contents/MacOS` to run app in Release Mode**
   If the `.dylib` file is not found in the expected directory, manually move it:
   ```sh
   build/macos/Build/Products/Release/system_info.app/Contents/MacOS
   ```
7. **Run the Flutter App**:
   ```sh
   cd ../../
   flutter run -d macos
   ```

---

### Windows

#### Steps
1. **Clone the Repository**:
   ```sh
   git clone https://github.com/akashmaurya99/system-dashboard.git
   cd system-dashboard
   ```

2. **Navigate to the Windows FFI Directory & Compile C++ Code**:
   ```sh
   cd ffi/windows/
   ```
   This folder contains C++ files for gathering system data.

3. **Navigate to `windows/` and Build the Shared Library**:**:
   ```sh
   mkdir -p build && cd build
   cmake ..
   cmake --build . --config Release
   ```
   This generates the necessary DLL file.

4. **Run the Flutter App**:
   ```sh
   flutter run -d windows
   ```

---

## Contribution Guidelines
We welcome contributions! If you'd like to contribute:
- Report bugs or request features via [GitHub Issues](https://github.com/akashmaurya99/system-dashboard/issues)
- Submit a Pull Request with well-documented changes
- Ensure your code follows best practices and is tested before submission

---

## License
This project is licensed under the [MIT License](LICENSE).

---

## Future Enhancements
- Add more detailed process monitoring
- Implement a notification system for system health alerts
- Introduce plugin support for additional system insights

Stay tuned for updates and feel free to contribute!

## Download
🚀 **Get the Latest Version:**  
[Download for macOS](https://github.com/akashmaurya99/system-dashboard/releases/latest)