import 'dart:convert';
import 'package:flutter/foundation.dart';
import '../services/macos_system_info.dart';

class InstalledAppsProvider extends ChangeNotifier {
  final MacSystemInfo _systemInfo = MacSystemInfo();

  List<String> _apps = [];
  bool _isLoading = false;
  String _searchQuery = '';

  /// Returns the list of installed apps filtered by the search query.
  List<String> get apps {
    if (_searchQuery.isEmpty) {
      return _apps;
    } else {
      return _apps
          .where(
            (app) => app.toLowerCase().contains(_searchQuery.toLowerCase()),
          )
          .toList();
    }
  }

  bool get isLoading => _isLoading;

  /// Fetches the installed applications via FFI, sorts them alphabetically,
  /// and notifies listeners.
  Future<void> fetchInstalledApps() async {
    _isLoading = true;
    notifyListeners();

    try {
      final String jsonString = _systemInfo.getInstalledApplications();
      if (jsonString.isEmpty) {
        debugPrint("Received empty JSON string for installed applications.");
        _isLoading = false;
        notifyListeners();
        return;
      }
      // Parse the JSON. We expect a JSON object with an "installed_applications" list.
      final data = jsonDecode(jsonString);
      if (data is Map &&
          data.containsKey("installed_applications") &&
          data["installed_applications"] is List) {
        _apps = List<String>.from(data["installed_applications"]);
        // Sort the apps alphabetically.
        _apps.sort((a, b) => a.compareTo(b));
      } else {
        debugPrint("Invalid JSON format for installed applications.");
      }
    } catch (e) {
      debugPrint("Error fetching installed applications: $e");
    }

    _isLoading = false;
    notifyListeners();
  }

  /// Updates the search query and notifies listeners to update the UI.
  void setSearchQuery(String query) {
    _searchQuery = query;
    notifyListeners();
  }
}
