import 'dart:ffi';
import 'dart:io';
import 'package:ffi/ffi.dart';

/// Load the shared library
final DynamicLibrary _lib = () {
  if (Platform.isMacOS) {
    // Get the absolute path of the library inside the Flutter macOS bundle
    // return DynamicLibrary.open("libmac_system_info.dylib");
    // Platform.resolvedExecutable gives the full path to the executable.
    // Its parent directory is the folder containing your executable (i.e. Contents/MacOS).

    final String exePath = Platform.resolvedExecutable;
    final String directory = File(exePath).parent.path;
    final String libPath = '$directory/libmac_system_info.dylib';

    // Optionally, print the path to debug:
    // print('Loading library from: $libPath');

    return DynamicLibrary.open(libPath);
  } else {
    throw UnsupportedError('This FFI module only supports macOS.');
  }
}();

/// Define C function signatures and Dart FFI bindings
class MacSystemInfo {
  /// Singleton instance
  static final MacSystemInfo _instance = MacSystemInfo._internal();
  factory MacSystemInfo() => _instance;
  MacSystemInfo._internal();

  /// Memory cleanup function
  final void Function(Pointer<Utf8>) _freeCStr = _lib.lookupFunction<
      Void Function(Pointer<Utf8>), void Function(Pointer<Utf8>)>('free_cstr');

  /// Helper function to convert a C-string pointer to a Dart string
  String _getString(Pointer<Utf8> ptr) {
    final result = ptr.toDartString();
    _freeCStr(ptr);
    return result;
  }

  /// Battery Info
  String getBatteryInfo() {
    final ptr =
        _lib.lookupFunction<Pointer<Utf8> Function(), Pointer<Utf8> Function()>(
            'batteryInfo')();
    return _getString(ptr);
  }

  /// CPU Info
  String getCpuInfo() {
    final ptr =
        _lib.lookupFunction<Pointer<Utf8> Function(), Pointer<Utf8> Function()>(
            'cpuData')();
    return _getString(ptr);
  }

  /// CPU Usage (Real-time)
  double getCpuUsage() {
    return _lib
        .lookupFunction<Double Function(), double Function()>('cpuUsages')();
  }

  /// Disk Details
  String getDiskDetails() {
    final ptr =
        _lib.lookupFunction<Pointer<Utf8> Function(), Pointer<Utf8> Function()>(
            'diskDetails')();
    return _getString(ptr);
  }

  /// GPU Info
  String getGpuInfo() {
    final ptr =
        _lib.lookupFunction<Pointer<Utf8> Function(), Pointer<Utf8> Function()>(
            'gpuInfo')();
    return _getString(ptr);
  }

  /// GPU Usage
  double getGpuUsage() {
    return _lib
        .lookupFunction<Double Function(), double Function()>('gpuUsages')();
  }

  /// OS Info
  String getOsInfo() {
    final ptr =
        _lib.lookupFunction<Pointer<Utf8> Function(), Pointer<Utf8> Function()>(
            'osInfo')();
    return _getString(ptr);
  }

  /// RAM Info
  String getRamInfo() {
    final ptr =
        _lib.lookupFunction<Pointer<Utf8> Function(), Pointer<Utf8> Function()>(
            'ramInfo')();
    return _getString(ptr);
  }

  /// Installed Applications
  String getInstalledApplications() {
    final ptr =
        _lib.lookupFunction<Pointer<Utf8> Function(), Pointer<Utf8> Function()>(
            'installedApplications')();
    return _getString(ptr);
  }

  /// Running Processes
  String getRunningProcesses() {
    final ptr =
        _lib.lookupFunction<Pointer<Utf8> Function(), Pointer<Utf8> Function()>(
            'runningProcesses')();

    return _getString(ptr);
  }
}
