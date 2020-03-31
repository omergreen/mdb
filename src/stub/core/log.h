#pragma once

#include <core/ops.h>

#define COLORS
#define DEBUGMODE

#ifdef COLORS
#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"
#else
#define KNRM  ""
#define KRED  ""
#define KGRN  ""
#define KYEL  ""
#define KBLU  ""
#define KMAG  ""
#define KCYN  ""
#define KWHT  ""
#endif


#define _MESSAGE(s, type, color, ...) g_ops.log("%s%s%s %s:%d: %s" s "%s\n", color, type, KNRM, __FILE__, __LINE__, color, ##__VA_ARGS__, KNRM)

#ifdef DEBUGMODE
#define DEBUG(s, ...) _MESSAGE(s, "DEBUG", KYEL, ##__VA_ARGS__)
#else
#define DEBUG(S, ...)
#endif

#define ERROR(s, ...) _MESSAGE(s, "ERROR", KRED, ##__VA_ARGS__)

