#pragma once

#include <core/ops.h>

#define _MESSAGE(s, type, ...) g_ops.log("%s %s:%d: " s "\n", type, __FILE__, __LINE__, ##__VA_ARGS__)

#define DEBUGMODE
#ifdef DEBUGMODE
#define DEBUG(s, ...) _MESSAGE(s, "DEBUG", ##__VA_ARGS__)
#else
#define DEBUG(S, ...)
#endif

#define ERROR(s, ...) _MESSAGE(s, "ERROR", ##__VA_ARGS__)

