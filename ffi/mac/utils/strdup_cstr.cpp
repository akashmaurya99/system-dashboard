#include "../include/strdup_cstr.h"
#include <cstdlib>
#include <cstring>

// Helper function to duplicate a string
char* strdup_cstr(const std::string& str) {
    char* cstr = (char*)malloc(str.size() + 1);
    if (cstr) {
        strcpy(cstr, str.c_str());
    }
    return cstr;
}
