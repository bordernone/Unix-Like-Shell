#include <stddef.h>
#include <malloc.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <stdbool.h>
#include <fcntl.h>


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

char *removeWhitespaces(const char *str, bool freeOld) {
    char *newStr = malloc(strlen(str) + 1);
    int i = 0;
    while (isspace(str[i])) {
        i++;
    }
    int j = strlen(str) - 1;
    if (j < 0) {
        newStr[0] = '\0';
        if (freeOld) {
            free((char *) str);
        }
        return newStr;
    }
    while (isspace(str[j])) {
        j--;
    }
    int k = 0;
    for (; i <= j; i++) {
        newStr[k++] = str[i];
    }
    newStr[k] = '\0';
    if (freeOld) {
        free((char *) str);
    }
    return newStr;
}

char *getPathBase(const char *path) {
    char *pathCopy = malloc(strlen(path) + 1);
    strcpy(pathCopy, path);
    char *token = strtok(pathCopy, "/");
    char *lastToken = token;
    while (token != NULL) {
        lastToken = token;
        token = strtok(NULL, "/");
    }
    char *ret;
    if (lastToken == NULL) {
        ret = malloc(1);
        ret[0] = '\0';
        free(pathCopy);
        return ret;
    } else {
        ret = malloc(strlen(lastToken) + 1);
    }
    strcpy(ret, lastToken);
    free(pathCopy);
    return ret;
}

char *currentDirHead() {
    char str[1024];
    getcwd(str, 1024);
    char *pathBase = getPathBase(str);
    char *ret = malloc(strlen(pathBase) + 2);
    strcpy(ret, pathBase);
    strcat(ret, "/");
    free(pathBase);
    return ret;
}

char *getStringBefore(const char *str, const char delim) {
    char *strCopy = malloc(strlen(str) + 1);
    strcpy(strCopy, str);
    char *del = malloc(2);
    del[0] = delim;
    del[1] = '\0';
    char *token = strtok(strCopy, del);
    if (token == NULL) {
        free(strCopy);
        free(del);
        // return empty string
        char *emptyString = malloc(1);
        emptyString[0] = '\0';
        return emptyString;
    }
    char *ret = malloc(strlen(token) + 1);
    strcpy(ret, token);
    ret[strlen(token)] = '\0';
    free(strCopy);
    free(del);
    return ret;
}

char *getStringAfter(const char *str, const char delim) {
    char *strCopy = malloc(strlen(str) + 1);
    strcpy(strCopy, str);
    char *strAfter = strchr(strCopy, delim);
    if (strAfter == NULL) {
        free(strCopy);
        // return empty string
        char *emptyString = malloc(1);
        emptyString[0] = '\0';
        return emptyString;
    }
    // remove delim
    strAfter++;
    char *ret = malloc(strlen(strAfter) + 1);
    strcpy(ret, strAfter);
    ret[strlen(strAfter)] = '\0';
    free(strCopy);
    return ret;
}

char *getStringBeforeSubstring(const char *str, const char *subStr) {
    char *strCopy = malloc(strlen(str) + 1);
    strcpy(strCopy, str);
    char *strBefore = strtok(strCopy, subStr);
    if (strBefore == NULL) {
        free(strCopy);
        // return empty string
        char *emptyString = malloc(1);
        emptyString[0] = '\0';
        return emptyString;
    }
    char *ret = malloc(strlen(strBefore) + 1);
    strcpy(ret, strBefore);
    ret[strlen(strBefore)] = '\0';
    free(strCopy);
    return ret;
}

char *getStringAfterSubstring(const char *str, const char *subStr) {
    char *strCopy = malloc(strlen(str) + 1);
    strcpy(strCopy, str);
    char *strAfter = strstr(strCopy, subStr);
    if (strAfter == NULL) {
        free(strCopy);
        // return empty string
        char *emptyString = malloc(1);
        emptyString[0] = '\0';
        return emptyString;
    }
    // remove subStr
    strAfter += strlen(subStr);
    char *ret = malloc(strlen(strAfter) + 1);
    strcpy(ret, strAfter);
    ret[strlen(strAfter)] = '\0';
    free(strCopy);
    return ret;
}

bool stringHasChar(const char *str, const char c) {
    for (long unsigned int i = 0; i < strlen(str); i++) {
        if (str[i] == c) {
            return true;
        }
    }
    return false;
}

struct StringSplit splitString(const char *str, const char delim){
    struct StringSplit ret;
    ret.size = 0;

    int subStringLength = 0;
    char *substr = NULL;
    for (long unsigned int i = 0; i < strlen(str); i++){
        if (str[i] != delim) {
            subStringLength++;
        } else {
            if (subStringLength > 0) {
                substr = malloc(subStringLength + 1);
                strncpy(substr, str + i - subStringLength, subStringLength);
                substr[subStringLength] = '\0';
                ret.str[ret.size++] = substr;
                subStringLength = 0;
            }
        }
    }

    if (subStringLength > 0) {
        substr = malloc(subStringLength + 1);
        strncpy(substr, str + strlen(str) - subStringLength, subStringLength);
        substr[subStringLength] = '\0';
        ret.str[ret.size++] = substr;
    }

    ret.str[ret.size] = NULL;

    return ret;
}

bool stringHasSubstring(const char *str, const char *subStr) {
    return strstr(str, subStr) != NULL;
}

int terminateFd(struct Terminate terminate) {
    if (terminate.isEmpty) {
        return -1;
    } else if (terminate.isAppend) {
        return open(terminate.filename, O_WRONLY | O_APPEND | O_CREAT, 0644);
    } else {
        return open(terminate.filename, O_WRONLY | O_TRUNC | O_CREAT, 0644);
    }
}