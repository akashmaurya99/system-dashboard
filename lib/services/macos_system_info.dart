import 'dart:ffi';
import 'dart:io';
import 'package:ffi/ffi.dart';

// Function to get the correct library path dynamically
String getLibraryPath() {
  if (!Platform.isMacOS) {
    throw UnsupportedError('This library is only supported on macOS.');
  }

  // Resolving the correct path inside the macOS app bundle
  String executablePath = Platform.resolvedExecutable;
  String appDir = File(executablePath).parent.path;
  String dylibPath = '$appDir/libmac_system_info.dylib';

  if (!File(dylibPath).existsSync()) {
    throw Exception('Dynamic library not found at: $dylibPath');
  }

  return dylibPath;
}

// Load the shared library
final DynamicLibrary systemInfoLib = DynamicLibrary.open(getLibraryPath());

// Define FFI function signatures
typedef GetInfoNative = Pointer<Utf8> Function();
typedef GetInfoDart = Pointer<Utf8> Function();

// Bind the C++ functions
class MacSystemInfo {
  static final GetInfoDart _getDiskUsage =
      systemInfoLib.lookupFunction<GetInfoNative, GetInfoDart>('getDiskUsage');

  static final GetInfoDart _getBatteryInfo = systemInfoLib
      .lookupFunction<GetInfoNative, GetInfoDart>('displayBatteryInfo');

  static final GetInfoDart _getCPUInfo =
      systemInfoLib.lookupFunction<GetInfoNative, GetInfoDart>('getCPUInfo');

  static final GetInfoDart _getGPUInfo =
      systemInfoLib.lookupFunction<GetInfoNative, GetInfoDart>('getGPUInfo');

  // Utility method to safely convert FFI Pointer<Utf8> to Dart String
  static String _convertPointerToString(Pointer<Utf8> ptr) {
    try {
      return ptr.toDartString();
    } finally {
      malloc.free(ptr);
    }
  }

  static String getDiskUsage() {
    return _convertPointerToString(_getDiskUsage());
  }

  static String getBatteryInfo() {
    return _convertPointerToString(_getBatteryInfo());
  }

  static String getCPUInfo() {
    return _convertPointerToString(_getCPUInfo());
  }

  static String getGPUInfo() {
    return _convertPointerToString(_getGPUInfo());
  }
}
