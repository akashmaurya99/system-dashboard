import 'dart:ffi';
import 'package:ffi/ffi.dart';

// Define Structs

base class Temperature extends Struct {
  @Array(256)
  external Array<Int8> name;

  @Double()
  external double current;

  @Double()
  external double max;

  String get nameStr => name.toString();
}

base class Fan extends Struct {
  @Array(256)
  external Array<Int8> name;

  @Int64()
  external int current;

  @Int64()
  external int max;

  String get nameStr => name.toString();
}

base class CpuInfo extends Struct {
  @Array(256)
  external Array<Int8> model;

  @Int32()
  external int cores;

  @Double()
  external double frequency;

  @Double()
  external double usage;

  external Temperature temp;

  String get modelStr => model.toString();
}

base class MemoryInfo extends Struct {
  @Uint64()
  external int total;

  @Uint64()
  external int used;

  @Uint64()
  external int free;

  @Double()
  external double usage;
}

base class GpuInfo extends Struct {
  @Array(256)
  external Array<Int8> model;

  @Uint64()
  external int vram_total;

  @Uint64()
  external int vram_used;

  external Temperature temp;

  String get modelStr => model.toString();
}

base class BatteryInfo extends Struct {
  @Array(256)
  external Array<Int8> name;

  @Double()
  external double current;

  @Double()
  external double capacity;

  @Bool()
  external bool is_charging;

  String get nameStr => name[0].toString();
}

// Bind Native Functions

typedef GetCpuInfoNative = CpuInfo Function();
typedef GetMemoryInfoNative = MemoryInfo Function();
typedef GetGpuInfoNative = GpuInfo Function();
typedef GetBatteryInfoNative = BatteryInfo Function();

class SystemInfoBindings {
  late DynamicLibrary _lib;

  late CpuInfo Function() getCpuInfo;
  late MemoryInfo Function() getMemoryInfo;
  late GpuInfo Function() getGpuInfo;
  late BatteryInfo Function() getBatteryInfo;

  SystemInfoBindings() {
    _lib = DynamicLibrary.open("build/libsystem_info.dylib");

    getCpuInfo =
        _lib.lookupFunction<GetCpuInfoNative, GetCpuInfoNative>('get_cpu_info');
    getMemoryInfo =
        _lib.lookupFunction<GetMemoryInfoNative, GetMemoryInfoNative>(
            'get_memory_info');
    getGpuInfo =
        _lib.lookupFunction<GetGpuInfoNative, GetGpuInfoNative>('get_gpu_info');
    getBatteryInfo =
        _lib.lookupFunction<GetBatteryInfoNative, GetBatteryInfoNative>(
            'get_battery_info');
  }
}

// Main Function to Test FFI

void main() {
  final sysInfo = SystemInfoBindings();

  // CPU Info
  final cpu = sysInfo.getCpuInfo();
  print("CPU Model: ${cpu.modelStr}");
  print("Cores: ${cpu.cores}");
  print("Frequency: ${cpu.frequency} GHz");
  print("Usage: ${cpu.usage}%");
  print("Temperature: ${cpu.temp.current}°C (Max: ${cpu.temp.max}°C)");

  // Memory Info
  final memory = sysInfo.getMemoryInfo();
  print("\nMemory:");
  print("Total: ${memory.total ~/ (1024 * 1024 * 1024)} GB");
  print("Used: ${memory.used ~/ (1024 * 1024 * 1024)} GB");
  print("Free: ${memory.free ~/ (1024 * 1024 * 1024)} GB");
  print("Usage: ${memory.usage}%");

  // GPU Info
  final gpu = sysInfo.getGpuInfo();
  print("\nGPU Model: ${gpu.modelStr}");
  print("VRAM Total: ${gpu.vram_total ~/ (1024 * 1024)} MB");
  print("VRAM Used: ${gpu.vram_used ~/ (1024 * 1024)} MB");
  print("Temperature: ${gpu.temp.current}°C (Max: ${gpu.temp.max}°C)");

  // Battery Info
  final battery = sysInfo.getBatteryInfo();
  print("\nBattery:");
  print("Name: ${battery.nameStr}");
  print("Current Charge: ${battery.current} mAh");
  print("Capacity: ${battery.capacity} mAh");
  print("Charging: ${battery.is_charging ? 'Yes' : 'No'}");
}

// Helper Extension to Convert C Strings to Dart Strings
extension ArrayUtf8ToString on Pointer<Int8> {
  String toDartString() {
    return cast<Utf8>().toDartString();
  }
}

// import 'dart:ffi';
// import 'package:ffi/ffi.dart';

// // Load the compiled C++ dynamic library
// final DynamicLibrary _lib = DynamicLibrary.open("build/libsystem_info.dylib");

// // =============================
// // ✅ CPU Information
// // =============================

// // Get CPU Model
// final Pointer<Utf8> Function() _getCpuModel =
//     _lib.lookupFunction<Pointer<Utf8> Function(), Pointer<Utf8> Function()>(
//         "get_cpu_model");

// String getCpuModel() {
//   try {
//     return _getCpuModel().toDartString();
//   } catch (e) {
//     return "Unknown CPU Model";
//   }
// }

// // Get CPU Core Count
// final int Function() _getCpuCores =
//     _lib.lookupFunction<Int32 Function(), int Function()>("get_cpu_cores");

// int getCpuCores() {
//   try {
//     return _getCpuCores();
//   } catch (e) {
//     return -1; // Return -1 to indicate an error
//   }
// }

// // Get CPU Usage (Real-time)
// final double Function() _getCpuUsage =
//     _lib.lookupFunction<Float Function(), double Function()>("get_cpu_usage");

// double getCpuUsage() {
//   try {
//     return _getCpuUsage();
//   } catch (e) {
//     return 0.0; // Return 0% if an error occurs
//   }
// }

// // =============================
// // ✅ RAM Information
// // =============================

// // Get Total RAM (in MB)
// final int Function() _getTotalRam =
//     _lib.lookupFunction<Int64 Function(), int Function()>("get_total_ram");

// int getTotalRam() {
//   try {
//     return _getTotalRam();
//   } catch (e) {
//     return -1;
//   }
// }

// // Get RAM Usage (Used & Free in MB)
// void getRamUsage(void Function(int used, int free) callback) {
//   final Pointer<Int64> used = calloc<Int64>();
//   final Pointer<Int64> free = calloc<Int64>();

//   final void Function(Pointer<Int64>, Pointer<Int64>) _getRamUsage =
//       _lib.lookupFunction<Void Function(Pointer<Int64>, Pointer<Int64>),
//           void Function(Pointer<Int64>, Pointer<Int64>)>("get_ram_usage");

//   try {
//     _getRamUsage(used, free);
//     callback(used.value, free.value);
//   } catch (e) {
//     callback(-1, -1); // Return -1 if there's an error
//   } finally {
//     calloc.free(used);
//     calloc.free(free);
//   }
// }

// // =============================
// // ✅ GPU Information
// // =============================

// // Get GPU Model
// final Pointer<Utf8> Function() _getGpuModel =
//     _lib.lookupFunction<Pointer<Utf8> Function(), Pointer<Utf8> Function()>(
//         "get_gpu_model");

// String getGpuModel() {
//   try {
//     return _getGpuModel().toDartString();
//   } catch (e) {
//     return "Unknown GPU Model";
//   }
// }

// // Get GPU VRAM Size (in MB)
// final int Function() _getGpuVram =
//     _lib.lookupFunction<Int64 Function(), int Function()>("get_gpu_vram");

// int getGpuVram() {
//   try {
//     return _getGpuVram();
//   } catch (e) {
//     return -1;
//   }
// }

// // =============================
// // ✅ Storage Information
// // =============================

// // Get Disk Size (Total in GB)
// final int Function() _getDiskSize =
//     _lib.lookupFunction<Int64 Function(), int Function()>("get_disk_size");

// int getDiskSize() {
//   try {
//     return _getDiskSize();
//   } catch (e) {
//     return -1;
//   }
// }

// // Get Available Disk Space (Free in GB)
// final int Function() _getDiskFreeSpace = _lib
//     .lookupFunction<Int64 Function(), int Function()>("get_disk_free_space");

// int getDiskFreeSpace() {
//   try {
//     return _getDiskFreeSpace();
//   } catch (e) {
//     return -1;
//   }
// }

// // =============================
// // ✅ MAIN FUNCTION FOR TESTING
// // =============================
// void main() {
//   print("🔹 CPU Model: ${getCpuModel()}");
//   print("🔹 CPU Cores: ${getCpuCores()}");
//   print("🔹 CPU Usage: ${getCpuUsage().toStringAsFixed(2)} %");

//   print("🔹 Total RAM: ${getTotalRam()} MB");
//   getRamUsage((used, free) {
//     print("🔹 Used RAM: $used MB");
//     print("🔹 Free RAM: $free MB");
//   });

//   print("🔹 GPU Model: ${getGpuModel()}");
//   print("🔹 GPU VRAM: ${getGpuVram()} MB");

//   print("🔹 Disk Size: ${getDiskSize()} GB");
//   print("🔹 Free Disk Space: ${getDiskFreeSpace()} GB");
// }

// import 'dart:ffi';
// import 'dart:io';
// import 'package:ffi/ffi.dart';

// /// Load the shared library (adjust the path if needed)
// final DynamicLibrary systemInfoLib =
//     DynamicLibrary.open("build/libsysteminfo.dylib");

// /// Struct representing CPU usage
// base class CpuUsage extends Struct {
//   @Double()
//   external double user;

//   @Double()
//   external double system;

//   @Double()
//   external double idle;
// }

// /// Struct representing Memory usage
// base class MemoryUsage extends Struct {
//   @Uint64()
//   external int total;

//   @Uint64()
//   external int used;

//   @Uint64()
//   external int free;
// }

// /// Struct representing Battery Info
// base class BatteryInfo extends Struct {
//   @Double()
//   external double capacity;

//   @Double()
//   external double voltage;

//   @Bool()
//   external bool isCharging;
// }

// /// Struct representing Network Info
// base class NetworkInfo extends Struct {
//   @Array<Int8>(32)
//   external Array<Int8> interface;

//   @Uint64()
//   external int bytesIn;

//   @Uint64()
//   external int bytesOut;
// }

// /// Struct representing Process Info
// base class ProcessInfo extends Struct {
//   @Int32()
//   external int pid;

//   @Array<Uint8>(256)
//   external Array<Uint8> name;

//   @Double()
//   external double cpuUsage;

//   @Double()
//   external double memUsage;
// }

// /// Function bindings for system info retrieval
// final getCpuUsage = systemInfoLib
//     .lookupFunction<CpuUsage Function(), CpuUsage Function()>("get_cpu_usage");

// final getMemoryUsage = systemInfoLib.lookupFunction<MemoryUsage Function(),
//     MemoryUsage Function()>("get_memory_usage");

// final getGpuModel = systemInfoLib.lookupFunction<Pointer<Utf8> Function(),
//     Pointer<Utf8> Function()>("get_gpu_model");

// final getBatteryInfo = systemInfoLib.lookupFunction<BatteryInfo Function(),
//     BatteryInfo Function()>("get_battery_info");

// final getProcessList = systemInfoLib.lookupFunction<
//     Pointer<ProcessInfo> Function(),
//     Pointer<ProcessInfo> Function()>("get_process_list");

// final getNetworkUsage = systemInfoLib.lookupFunction<
//     Pointer<NetworkInfo> Function(),
//     Pointer<NetworkInfo> Function()>("get_network_usage");

// /// Helper function to convert Array<Int8> to Dart String
// String arrayToString(Array<Int8> array, {int maxLength = 32}) {
//   final List<int> charCodes = [];
//   for (int i = 0; i < maxLength; i++) {
//     final int char = array[i];
//     if (char == 0) break; // Stop at null terminator
//     charCodes.add(char);
//   }
//   return String.fromCharCodes(charCodes);
// }

// /// Main function to test the FFI integration
// void main() {
//   final cpu = getCpuUsage();
//   print(
//       "CPU Usage: User ${cpu.user}%, System ${cpu.system}%, Idle ${cpu.idle}%");

//   final memory = getMemoryUsage();
//   print(
//       "Memory: Total ${memory.total}, Used ${memory.used}, Free ${memory.free}");

//   final gpuPointer = getGpuModel();
//   final gpuModel = gpuPointer.cast<Utf8>().toDartString();
//   print("GPU Model: $gpuModel");
//   malloc.free(gpuPointer); // Free allocated memory

//   final battery = getBatteryInfo();
//   print(
//       "Battery: ${battery.capacity}% (${battery.voltage}V), Charging: ${battery.isCharging}");

//   final networkPointer = getNetworkUsage();
//   final network = networkPointer.ref;
//   print(
//       "Network: ${arrayToString(network.interface)} - In: ${network.bytesIn}, Out: ${network.bytesOut}");
//   malloc.free(networkPointer); // Free allocated memory
// }

// import 'dart:ffi';
// import 'package:ffi/ffi.dart';

// // Load the compiled C++ dynamic library
// final DynamicLibrary systemInfoLib =
//     DynamicLibrary.open("build/libsysteminfo.dylib");

// // Function type definitions for FFI
// typedef GetCpuModelFuncNative = Pointer<Utf8> Function();
// typedef GetCpuModelFuncDart = Pointer<Utf8> Function();

// typedef GetCpuCoresFuncNative = Int32 Function();
// typedef GetCpuCoresFuncDart = int Function();

// typedef GetRamSizeFuncNative = Uint64 Function();
// typedef GetRamSizeFuncDart = int Function();

// typedef GetOsNameFuncNative = Pointer<Utf8> Function();
// typedef GetOsNameFuncDart = Pointer<Utf8> Function();

// typedef GetOsVersionFuncNative = Pointer<Utf8> Function();
// typedef GetOsVersionFuncDart = Pointer<Utf8> Function();

// @Packed(1)
// final class DiskSpace extends Struct {
//   @Uint64()
//   external int total;

//   @Uint64()
//   external int free;
// }

// typedef GetDiskUsageFuncNative = DiskSpace Function(Pointer<Utf8>);
// typedef GetDiskUsageFuncDart = DiskSpace Function(Pointer<Utf8>);

// // Dart function bindings
// final GetCpuModelFuncDart getCpuModelNative =
//     systemInfoLib.lookupFunction<GetCpuModelFuncNative, GetCpuModelFuncDart>(
//         "get_cpu_model");

// final GetCpuCoresFuncDart getCpuCoresNative =
//     systemInfoLib.lookupFunction<GetCpuCoresFuncNative, GetCpuCoresFuncDart>(
//         "get_cpu_cores");

// final GetRamSizeFuncDart getRamSizeNative = systemInfoLib
//     .lookupFunction<GetRamSizeFuncNative, GetRamSizeFuncDart>("get_ram_size");

// final GetOsNameFuncDart getOsNameNative = systemInfoLib
//     .lookupFunction<GetOsNameFuncNative, GetOsNameFuncDart>("get_os_name");

// final GetOsVersionFuncDart getOsVersionNative =
//     systemInfoLib.lookupFunction<GetOsVersionFuncNative, GetOsVersionFuncDart>(
//         "get_os_version");

// final GetDiskUsageFuncDart getDiskUsageNative =
//     systemInfoLib.lookupFunction<GetDiskUsageFuncNative, GetDiskUsageFuncDart>(
//         "get_disk_usage");

// void main() {
//   // Get CPU Model
//   final Pointer<Utf8> cpuModelPtr = getCpuModelNative();
//   final String cpuModel = cpuModelPtr.toDartString();
//   malloc.free(cpuModelPtr);
//   print("CPU Model: $cpuModel");

//   // Get CPU Cores
//   final int cpuCores = getCpuCoresNative();
//   print("CPU Cores: $cpuCores");

//   // Get RAM Size
//   final int ramSize = getRamSizeNative();
//   print("Total RAM: ${ramSize ~/ (1024 * 1024)} MB");

//   // Get OS Name
//   final Pointer<Utf8> osNamePtr = getOsNameNative();
//   final String osName = osNamePtr.toDartString();
//   malloc.free(osNamePtr);
//   print("OS Name: $osName");

//   // Get OS Version
//   final Pointer<Utf8> osVersionPtr = getOsVersionNative();
//   final String osVersion = osVersionPtr.toDartString();
//   malloc.free(osVersionPtr);
//   print("OS Version: $osVersion");

//   // Get Disk Usage (Example: "/")
//   final Pointer<Utf8> pathPtr = "/".toNativeUtf8();
//   final DiskSpace diskSpace = getDiskUsageNative(pathPtr);
//   malloc.free(pathPtr);
//   print("Total Disk Space: ${diskSpace.total ~/ (1024 * 1024 * 1024)} GB");
//   print("Free Disk Space: ${diskSpace.free ~/ (1024 * 1024 * 1024)} GB");
// }
