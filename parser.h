#ifndef NYUSH_PARSER_H
#define NYUSH_PARSER_H

#include <stdbool.h>

#define MAX_ARGC 1024

struct Cmd {
    char *cmd;
    char *argv[MAX_ARGC];
    int argc;
};

struct CmdTerminate {
    struct Cmd cmd;
    struct Terminate terminate;
};

struct Cmd parseCmd(const char *cmdStr);
struct Terminate parseTerminate(const char *cmdStr);
struct CmdTerminate parseCmdTerminate(const char *cmd);

#endif