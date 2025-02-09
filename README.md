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
ffi/
│── mac/
│   ├── battery_info.cpp
│   ├── cpu_info.cpp
│   ├── disk_info.cpp
│   ├── gpu_info.cpp
│   ├── os_info.cpp
│   ├── ram_info.cpp
│   ├── running_app_info.cpp
│   ├── mac_system_info.cpp
│   ├── include/
│   │   ├── battery_info.h
│   │   ├── cpu_info.h
│   │   ├── disk_info.h
│   │   ├── gpu_info.h
│   │   ├── os_info.h
│   │   ├── ram_info.h
│   │   ├── running_app_info.h
│
│── windows/
│   ├── (Will contain C++ source files for Windows implementation)
│
frontend/
    ├── (Flutter UI components)
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

