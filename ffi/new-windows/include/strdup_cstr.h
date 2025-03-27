#pragma once

#include <string>
#include <cstring>
#include <cstdlib>

// Helper function to create a duplicate of a string for C interop
inline char* strdup_cstr(const std::string& str) {
    char* cstr = static_cast<char*>(malloc(str.length() + 1));
    if (cstr) {
        strcpy(cstr, str.c_str());
    }
    return cstr;
}