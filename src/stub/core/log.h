/*
 * Logging functions & colors
 */

#pragma once

#include <target/interface.h>

#define COLORIZE_FORMAT(format) "%s" format "%s"
#define COLORIZE_ARGS(color, ...) color, ##__VA_ARGS__, KNRM

#ifdef COLORS
#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"
#define KBOLD "\x1B[1m"
#define KDIM "\x1B[2m"
#define KUNDERLINE "\x1B[4m"
#else
#define KNRM  ""
#define KRED  ""
#define KGRN  ""
#define KYEL  ""
#define KBLU  ""
#define KMAG  ""
#define KCYN  ""
#define KWHT  ""
#define KBOLD ""
#define KDIM ""
#define KUNDERLINE ""
#endif


#define _MESSAGE(s, type, color, ...) target_log(COLORIZE_FORMAT("%s") " %s:%d: %s" s "%s\n", COLORIZE_ARGS(color, type), __FILE__, __LINE__, COLORIZE_ARGS(color, ##__VA_ARGS__))

#ifdef DEBUGMODE
#define DEBUG(s, ...) _MESSAGE(s, "DEBUG", KYEL, ##__VA_ARGS__)
#else
#define DEBUG(S, ...)
#endif

#define ERROR(s, ...) _MESSAGE(s, "ERROR", KRED, ##__VA_ARGS__)
 
#define _STRINGIFY(a) #a
#define STRINGIFY(a) _STRINGIFY(a)
// TODO: maybe make the debugger quit on assert?
#define assert(condition) do { \
                              if (condition) { \
                                  ERROR("assert(" STRINGIFY(condition) ") failed"); \
                              } \
                          } while(0);

