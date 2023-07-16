#ifndef NYUSH_COMMANDS_H
#define NYUSH_COMMANDS_H

#define MAX_ARGC 1024

struct Cmd {
    char *cmd;
    char *argv[MAX_ARGC];
    int argc;
};

int blankCommand();
int cdCommand(char *path);
int exitCommand(char *cmd);
int fgCommand(int jobIndex);
int jobsCommand();
void runCmdRecursive(struct Cmd *cmds, int currentIndex, int size, int myStdIn, int lastStdOut);
int runCmd(char *programPath, char *args[], int newStdin, int newStdout, const char *fullCmd);
int cmdWithRedirectAndTerminate(const char *cmd);
int cmdWithTerminateAndRedirect(const char *cmd);
int cmdWithTerminate(const char *cmd);
int cmdWithRecursive(const char *cmd);
int cmdWithRedirectAndRecursive(const char *cmd);

#endif
