#include "include/common.h"
#include <windows.h>
#include <string>
#include <memory>
#include <vector>
#include <sstream>

// Utility function to duplicate a C string for FFI
char* strdup_cstr(const std::string& str) {
    char* cstr = (char*)malloc(str.length() + 1);
    if (cstr) {
        strcpy_s(cstr, str.length() + 1, str.c_str());
    }
    return cstr;
}

// Helper function to execute a command and get output
std::string execCommand(const char* cmd) {
    std::string result;
    HANDLE hPipeRead, hPipeWrite;
    
    SECURITY_ATTRIBUTES saAttr = {sizeof(SECURITY_ATTRIBUTES)};
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;
    
    // Create a pipe for the child process's STDOUT
    if (!CreatePipe(&hPipeRead, &hPipeWrite, &saAttr, 0)) {
        return "Error creating pipe";
    }
    
    // Ensure the read handle to the pipe is not inherited
    SetHandleInformation(hPipeRead, HANDLE_FLAG_INHERIT, 0);
    
    // Create the child process
    PROCESS_INFORMATION piProcInfo;
    STARTUPINFO siStartInfo = {sizeof(STARTUPINFO)};
    siStartInfo.hStdError = hPipeWrite;
    siStartInfo.hStdOutput = hPipeWrite;
    siStartInfo.dwFlags |= STARTF_USESTDHANDLES;
    
    // Create the command line
    std::string cmdLine = std::string("cmd.exe /c ") + cmd;
    std::vector<char> cmdLineChars(cmdLine.begin(), cmdLine.end());
    cmdLineChars.push_back('\0');
    
    // Start the child process
    BOOL bSuccess = CreateProcess(
        NULL,               // No module name (use command line)
        cmdLineChars.data(),// Command line
        NULL,               // Process handle not inheritable
        NULL,               // Thread handle not inheritable
        TRUE,               // Set handle inheritance
        CREATE_NO_WINDOW,   // No window
        NULL,               // Use parent's environment block
        NULL,               // Use parent's starting directory
        &siStartInfo,       // Pointer to STARTUPINFO structure
        &piProcInfo         // Pointer to PROCESS_INFORMATION structure
    );
    
    // Close handle to the pipe write end
    CloseHandle(hPipeWrite);
    
    // If the process was created successfully, read from pipe
    if (bSuccess) {
        // Read output from the child process
        char buffer[4096];
        DWORD dwRead;
        
        while (ReadFile(hPipeRead, buffer, sizeof(buffer) - 1, &dwRead, NULL) && dwRead != 0) {
            buffer[dwRead] = '\0';
            result += buffer;
        }
        
        // Close process and thread handles
        CloseHandle(piProcInfo.hProcess);
        CloseHandle(piProcInfo.hThread);
    }
    
    // Close pipe handle
    CloseHandle(hPipeRead);
    
    return result;
}

// Helper function to convert wide string to UTF-8
std::string wideToUtf8(const std::wstring& wstr) {
    if (wstr.empty()) return std::string();
    
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.length(), NULL, 0, NULL, NULL);
    std::string str(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.length(), &str[0], size_needed, NULL, NULL);
    
    return str;
}

// Helper function to convert UTF-8 to wide string
std::wstring utf8ToWide(const std::string& str) {
    if (str.empty()) return std::wstring();
    
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.length(), NULL, 0);
    std::wstring wstr(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.length(), &wstr[0], size_needed);
    
    return wstr;
}

// Format bytes to human-readable string (KB, MB, GB, etc.)
std::string formatBytes(ULONGLONG bytes) {
    std::stringstream ss;
    
    if (bytes < 1024) {
        ss << bytes << " B";
    } else if (bytes < 1024 * 1024) {
        ss << std::fixed << std::setprecision(2) << (bytes / 1024.0) << " KB";
    } else if (bytes < 1024 * 1024 * 1024) {
        ss << std::fixed << std::setprecision(2) << (bytes / (1024.0 * 1024.0)) << " MB";
    } else {
        ss << std::fixed << std::setprecision(2) << (bytes / (1024.0 * 1024.0 * 1024.0)) << " GB";
    }
    
    return ss.str();
}

// Format time in seconds to readable string
std::string formatTime(ULONGLONG seconds) {
    std::stringstream ss;
    
    ULONGLONG days = seconds / (60 * 60 * 24);
    seconds %= (60 * 60 * 24);
    
    ULONGLONG hours = seconds / (60 * 60);
    seconds %= (60 * 60);
    
    ULONGLONG minutes = seconds / 60;
    seconds %= 60;
    
    if (days > 0) {
        ss << days << " days, ";
    }
    
    if (hours > 0 || days > 0) {
        ss << hours << " hours, ";
    }
    
    ss << minutes << " minutes, " << seconds << " seconds";
    
    return ss.str();
}