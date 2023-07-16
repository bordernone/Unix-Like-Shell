#include <string.h>
#include <malloc.h>
#include "helper.h"
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

#define MAX_ARGC 1024
#define PATH "/usr/bin"

struct Cmd {
    char *cmd;
    char *argv[MAX_ARGC];
    int argc;
};

struct CmdTerminate {
    struct Cmd cmd;
    struct Terminate terminate;
};

struct Cmd parseCmd(const char *cmdStr) {
    struct Cmd cmd = {NULL, {NULL}, 0};

    char *cmdStrCopy = removeWhitespaces(cmdStr, false); // remove whitespaces

    struct StringSplit split = splitString(cmdStrCopy, ' '); // split by space

    // Command
    cmd.cmd = split.str[0];
    split.str[0] = NULL; 
    split.str[0] = malloc(strlen(cmd.cmd) + 1); // allocate memory
    strcpy(split.str[0], cmd.cmd); // copy string
    // split.str[0] = getPathBase(cmd.cmd); // get base of path

    // Arguments
    for (int i = 0; i < split.size; i++) {
        cmd.argv[i] = split.str[i];
    }

    cmd.argv[split.size] = NULL; // NULL terminate

    // Argument count
    cmd.argc = split.size;

    // Check if cmd is an absolute path
    if (cmd.cmd[0] == '/') {
        // Absolute path
    } else if (stringHasChar(cmd.cmd, '/')) {
        // Relative path
        char currentDir[1024];
        getcwd(currentDir, 1024);
        char *path = malloc(strlen(currentDir) + strlen(cmd.cmd) + 2);
        strcpy(path, currentDir);
        strcat(path, "/");
        strcat(path, cmd.cmd);
        free(cmd.cmd);
        cmd.cmd = path;        
    } else {
        // find in PATH
        char *path = malloc(strlen(PATH) + strlen(cmd.cmd) + 2);
        strcpy(path, PATH);
        strcat(path, "/");
        strcat(path, cmd.cmd);
        free(cmd.cmd);
        cmd.cmd = path;
    }

    // Free memory
    free(cmdStrCopy);
    
    return cmd;
}

// Return -1 if there is no redirection. Returns 1 if there is overwriting redirection. Returns 2 if there is appending redirection. Sets the filename if last two cases.
struct Terminate parseTerminate(const char *cmdStr) {
    // if empty cmd
    if (strlen(cmdStr) == 0) {
       struct Terminate terminate = {true, false, NULL};
        return terminate;
    } else {
        // Two possibilities: '> filename' or '>> filename'
        char *cmdStrCopy = malloc(strlen(cmdStr) + 1);
        strcpy(cmdStrCopy, cmdStr);
        char *token = strtok(cmdStrCopy, " ");
        
        if (strcmp(token, ">") == 0) {
            // Overwrite
            token = strtok(NULL, " ");
            char *filename = malloc(strlen(token) + 1);
            strcpy(filename, token);
            struct Terminate terminate = {false, false, filename};
            free(cmdStrCopy);
            return terminate;
        } else if (strcmp(token, ">>") == 0) {
            // Append
            token = strtok(NULL, " ");
            char *filename = malloc(strlen(token) + 1);
            strcpy(filename, token);
            struct Terminate terminate = {false, true, filename};
            free(cmdStrCopy);
            return terminate;
        } else {
            // No redirection
            struct Terminate terminate = {false, false, NULL};
            free(cmdStrCopy);
            return terminate;
        }
    }
}

struct CmdTerminate parseCmdTerminate(const char *cmd) {
    char *cmdCopy = malloc(strlen(cmd) + 1);
    strcpy(cmdCopy, cmd);

    char *cmdStr = NULL;
    char *terminateStr = NULL;
    if (stringHasSubstring(cmdCopy, ">>")) {
        cmdStr = getStringBeforeSubstring(cmdCopy, ">>");
        terminateStr = getStringAfterSubstring(cmdCopy, ">>");
        cmdStr = removeWhitespaces(cmdStr, true);
        terminateStr = removeWhitespaces(terminateStr, true);
        // Prepend terminateStr with '>> ' to make it a valid terminate
        char *terminateStrWithDelimiter = malloc(strlen(terminateStr) + 4);
        terminateStrWithDelimiter[0] = '>';
        terminateStrWithDelimiter[1] = '>';
        terminateStrWithDelimiter[2] = ' ';
        strcpy(terminateStrWithDelimiter + 3, terminateStr);
        free(terminateStr);
        terminateStr = terminateStrWithDelimiter;
    } else if (stringHasChar(cmdCopy, '>')) {
        cmdStr = getStringBefore(cmdCopy, '>');
        terminateStr = getStringAfter(cmdCopy, '>');
        cmdStr = removeWhitespaces(cmdStr, true);
        terminateStr = removeWhitespaces(terminateStr, true);
        // Prepend terminateStr with '> ' to make it a valid terminate
        char *terminateStrWithDelimiter = malloc(strlen(terminateStr) + 3);
        terminateStrWithDelimiter[0] = '>';
        terminateStrWithDelimiter[1] = ' ';
        strcpy(terminateStrWithDelimiter + 2, terminateStr);
        free(terminateStr);
        terminateStr = terminateStrWithDelimiter;
    } else  {
        cmdStr = malloc(strlen(cmdCopy) + 1);
        strcpy(cmdStr, cmdCopy);
        cmdStr = removeWhitespaces(cmdStr, true);
        terminateStr = malloc(1);
        terminateStr[0] = '\0';
    }

    // Parse commands
    struct Cmd cmdParsed = parseCmd(cmdStr);
    struct Terminate terminate = parseTerminate(terminateStr);

    struct CmdTerminate cmdTerminate = {cmdParsed, terminate};

    // Free memory
    free(cmdCopy);
    free(cmdStr);
    free(terminateStr);

    return cmdTerminate;
}