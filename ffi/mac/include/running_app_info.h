#ifndef RUNNING_APP_INFO_H
#define RUNNING_APP_INFO_H

#ifdef __cplusplus
extern "C" {
#endif

// Get installed applications in JSON format
__attribute__((visibility("default"))) char* getInstalledApplicationsJSON();

// Get running processes in JSON format
__attribute__((visibility("default"))) char* getRunningProcessesJSON();

// Free allocated memory for FFI
__attribute__((visibility("default"))) void free_cstr(char* ptr);

#ifdef __cplusplus
}
#endif

#endif // RUNNING_APP_INFO_H





// #ifndef RUNNING_APP_INFO_H
// #define RUNNING_APP_INFO_H

// #include <string>

// std::string getInstalledApplicationsJSON();
// std::string getRunningProcessesJSON();

// #endif // RUNNING_APP_INFO_H
