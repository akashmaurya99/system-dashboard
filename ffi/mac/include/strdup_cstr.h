#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <cstdlib>
#include <cstring>

#ifdef __cplusplus
extern "C" {
#endif

// Declare strdup_cstr function
char* strdup_cstr(const std::string& str);

#ifdef __cplusplus
}
#endif

#endif // UTILS_H
