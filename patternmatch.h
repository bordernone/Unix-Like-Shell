
#ifndef NYUSH_PATTERNMATCH_H
#define NYUSH_PATTERNMATCH_H
#include <stdbool.h>

bool isMatch(const char *pattern, const char *str);
bool isBlankCommand(const char *str);
bool isCdCommand(const char *str);
bool isExitCommand(const char *str);
bool isFgCommand(const char *str);
bool isJobsCommand(const char *str);
bool isCmdWithRedirectAndTerminate(const char *str);
bool isCmdWithTerminateAndRedirect(const char *str);
bool isCmdWithTerminate(const char *str);
bool isCmdWithRecursive(const char *str);
bool isCmdWithRedirectRecursive(const char *str);

#endif
