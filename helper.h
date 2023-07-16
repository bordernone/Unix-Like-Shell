#ifndef NYUSH_HELPER_H
#define NYUSH_HELPER_H
#include <stdbool.h>

#define MAX_STRING_SPLIT 1024

struct Terminate  {
    bool isEmpty;
    bool isAppend;
    char *filename;
};

struct StringSplit {
    int size;
    char *str[MAX_STRING_SPLIT];
};

char *removeWhitespaces(const char *str, bool freeOld);
char *getPathBase(const char *path);
char *currentDirHead();
char *getStringBefore(const char *str, const char delim);
char *getStringAfter(const char *str, const char delim);
char *getStringBeforeSubstring(const char *str, const char *subStr);
char *getStringAfterSubstring(const char *str, const char *subStr);
bool stringHasChar(const char *str, const char c);
struct StringSplit splitString(const char *str, const char delim);
bool stringHasSubstring(const char *str, const char *subStr);
int terminateFd(struct Terminate terminate);

#endif