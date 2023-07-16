

#include <stdbool.h>
#include <regex.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <errno.h>
#include "helper.h"


char *commandRegex[7] = {
        "^cd ([^ \t><|*!`'\"]+)$", // [cd] [dir]
		"^fg ([0-9]+)$", // [fg] [arg]
		"^([^ \t><|*!`'\"]+)( ([^ \t><|*!`'\"]+))* < ([^ \t><|*!`'\"]+)(|( > ([^ \t><|*!`'\"]+))|( >> ([^ \t><|*!`'\"]+)))$", // [cmd] '<' [filename] [terminate]
        "^([^ \t><|*!`'\"]+)( [^ \t><|*!`'\"]+)*(|( > [^ \t><|*!`'\"]+)|( >> [^ \t><|*!`'\"]+)) < ([^ \t><|*!`'\"]+)$", // [cmd] [terminate] '<' [filename]
        "^([^ \t><|*!`'\"]+)( ([^ \t><|*!`'\"]+))*(|( > ([^ \t><|*!`'\"]+))|( >> ([^ \t><|*!`'\"]+)))$", // [cmd] [terminate]
        "^(([^ \t><|*!`'\"]+)( ([^ \t><|*!`'\"]+))*)( \\| (([^ \t><|*!`'\"]+)( ([^ \t><|*!`'\"]+))*))*( \\| (([^ \t><|*!`'\"]+)( ([^ \t><|*!`'\"]+))*)(|( > ([^ \t><|*!`'\"]+))|( >> ([^ \t><|*!`'\"]+))))$", // [cmd] | [cmd] | ... | [cmd] [terminate]
        "^(([^ \t><|*!`'\"]+)( ([^ \t><|*!`'\"]+))*) < ([^ \t><|*!`'\"]+)( \\| (([^ \t><|*!`'\"]+)( ([^ \t><|*!`'\"]+))*))*( \\| (([^ \t><|*!`'\"]+)( ([^ \t><|*!`'\"]+))*)(|( > ([^ \t><|*!`'\"]+))|( >> ([^ \t><|*!`'\"]+))))$", // [cmd] < [filename] | [cmd] | ... | [cmd] [terminate]
};

bool isMatch(const char *pattern, const char *str) {
    regex_t regex;
    int reti = regcomp(&regex, pattern, REG_EXTENDED);
    if (reti) {
		printf("%d", reti);
        printf("Could not compile regex. Error no: %d\n", errno);
        return false;
    }
    reti = regexec(&regex, str, 0, NULL, 0);
    regfree(&regex);
    if (!reti) { // check if reti is 0
        return true;
    } else if (reti == REG_NOMATCH) {
        return false;
    } else {
        printf("Regex match failed\n");
        return false;
    }
}

bool isBlankCommand(const char *str) { return strcmp(str, "") == 0; }

bool isCdCommand(const char *str) { return isMatch(commandRegex[0], str); }

bool isExitCommand(const char *str) { 
	return strcmp(str, "exit") == 0;
}

bool isFgCommand(const char *str) { return isMatch(commandRegex[1], str); }

bool isJobsCommand(const char *str) { return strcmp(str, "jobs") == 0; }

// [cmd] '<' [filename] [terminate]
bool isCmdWithRedirectAndTerminate(const char *str) {
	char *strCopy = malloc(strlen(str) + 1);
	strcpy(strCopy, str);
	// first word before space mustn't be cd, fg, exit, jobs
	// POSIX regex doesn't support lookaround
	char *firstWord = strtok(strCopy, " ");
	if (strcmp(firstWord, "cd") == 0 || strcmp(firstWord, "fg") == 0 || strcmp(firstWord, "exit") == 0 || strcmp(firstWord, "jobs") == 0) {
		free(strCopy);
        return false;
	}
	free(strCopy);
	return isMatch(commandRegex[2], str); 
}

// [cmd] [terminate] '<' [filename]
bool isCmdWithTerminateAndRedirect(const char *str) {
    char *strCopy = malloc(strlen(str) + 1);
    strcpy(strCopy, str);
    // first word before space mustn't be cd, fg, exit, jobs
    // POSIX regex doesn't support lookaround
    char *firstWord = strtok(strCopy, " ");
    if (strcmp(firstWord, "cd") == 0 || strcmp(firstWord, "fg") == 0 || strcmp(firstWord, "exit") == 0 || strcmp(firstWord, "jobs") == 0) {
        free(strCopy);
        return false;
    }
    free(strCopy);
    return isMatch(commandRegex[3], str); 
}

// [cmd] [terminate]
bool isCmdWithTerminate(const char *str) {
    char *strCopy = malloc(strlen(str) + 1);
    strcpy(strCopy, str);
    // first word before space mustn't be cd, fg, exit, jobs
    // POSIX regex doesn't support lookaround
    char *firstWord = strtok(strCopy, " ");
    if (strcmp(firstWord, "cd") == 0 || strcmp(firstWord, "fg") == 0 || strcmp(firstWord, "exit") == 0 || strcmp(firstWord, "jobs") == 0) {
        free(strCopy);
        return false;
    }
    free(strCopy);
    return isMatch(commandRegex[4], str); 
}

// [cmd] | [cmd] | ... | [cmd] [terminate]
bool isCmdWithRecursive(const char *str) {
    char *strCopy = malloc(strlen(str) + 1);
    strcpy(strCopy, str);
    // first word before space mustn't be cd, fg, exit, jobs
    // POSIX regex doesn't support lookaround
    bool isValid = true;
    struct StringSplit split = splitString(strCopy, '|');
    for (int i = 0; i < split.size; i++){
        char *currentWord = split.str[i];
        char *firstWord = strtok(currentWord, " ");
        if (strcmp(firstWord, "cd") == 0 || strcmp(firstWord, "fg") == 0 || strcmp(firstWord, "exit") == 0 || strcmp(firstWord, "jobs") == 0) {
            isValid = false;
            break;
        }
    }

    free(strCopy);
    for (int i = 0; i < split.size; i++) {
        free(split.str[i]);
    }

    return isValid && isMatch(commandRegex[5], str);
}

// [cmd] < [filename] | [cmd] | ... | [cmd] [terminate]
bool isCmdWithRedirectRecursive(const char *str) {
    char *strCopy = malloc(strlen(str) + 1);
    strcpy(strCopy, str);
    // first word before space mustn't be cd, fg, exit, jobs
    bool isValid = true;
    struct StringSplit split = splitString(strCopy, '|');
    for (int i = 0; i < split.size; i++){
        char *currentWord = split.str[i];
        char *firstWord = strtok(currentWord, " ");
        if (strcmp(firstWord, "cd") == 0 || strcmp(firstWord, "fg") == 0 || strcmp(firstWord, "exit") == 0 || strcmp(firstWord, "jobs") == 0) {
            isValid = false;
            break;
        }
    }

    free(strCopy);
    for (int i = 0; i < split.size; i++) {
        free(split.str[i]);
    }

    return isValid && isMatch(commandRegex[6], str);
}