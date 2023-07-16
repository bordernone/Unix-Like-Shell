#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include "commands.h"
#include "helper.h"
#include "patternmatch.h"
#include "backgroundprocesses.h"


#define MAX_BACKGROUND_PROCESSES 150

extern int debug;
int currentRunningChildPid = -1;
struct BackgroundProcess backgroundProcesses[MAX_BACKGROUND_PROCESSES];

void handleStopSignal(int signal) {
    // Forward the signal to the child process
    if (currentRunningChildPid != -1) {
        if (debug) printf("Child is running, forwarding signal to child %d \n", currentRunningChildPid);
        kill(currentRunningChildPid, signal);
    } else {
        if (debug) printf("Child is not running, ignoring signal \n");
    }
}

void printPrompt() {
    char *currentDir = currentDirHead();
    if (strcmp(currentDir, "/") == 0) {
        printf("[nyush /]$ ");
    } else {
        // Remove last slash
        currentDir[strlen(currentDir) - 1] = '\0';
        printf("[nyush %s]$ ", currentDir);
        currentDir[strlen(currentDir)] = '/'; // Restore last slash
    }
    fflush(stdout);
    free(currentDir);
}

char *getCommand() {
    fflush(stdout);
    fflush(stdin);

    size_t size = 1024;
    char *command = malloc(size * sizeof(char));
    // detect EOF
    if (fgets(command, size, stdin) == NULL) {
        command[0] = '\0';
        // Clear stdin using fseek
        if (feof(stdin)) {
            exit(0);
        }
    } else {
        command[strlen(command) - 1] = '\0';
    }
    // ssize_t characters;
    // characters = getline(&command, &size, stdin);
    // if (characters == -1) {
    //     command[0] = '\0';
    // } else {
    //     command[strlen(command) - 1] = '\0';
    // }

    return command;
}

void InitInterface() {
    signal(SIGTSTP, handleStopSignal);
    signal(SIGINT, handleStopSignal);
    signal(SIGSTOP, handleStopSignal);
    signal(SIGQUIT, handleStopSignal);

    // Initialize background processes
    for (int i = 0; i < MAX_BACKGROUND_PROCESSES; i++) {
        backgroundProcesses[i].cmd = NULL;
        backgroundProcesses[i].name = NULL;
        backgroundProcesses[i].pid = -1;
    }

    while (1) {
        printPrompt();
        char *command = getCommand();
        if (isBlankCommand(command)) {
            blankCommand();
        } else if (isCdCommand(command)) {
            char *dir = strtok(command, " ");
            dir = strtok(NULL, " ");
            if (cdCommand(dir) == -1) {
                fprintf(stderr, "Error: invalid directory\n");
            }
        } else if (isExitCommand(command)) { // TODO: KILL all children processes
            exitCommand(command);
        } else if (isFgCommand(command)) {
            char *arg = strtok(command, " ");
            arg = strtok(NULL, " ");
            fgCommand(atoi(arg));
        } else if (isJobsCommand(command)) {
            jobsCommand();
        } else if (isCmdWithRedirectAndTerminate(command)) {
            cmdWithRedirectAndTerminate(command);
        } else if (isCmdWithTerminateAndRedirect(command)) {
            cmdWithTerminateAndRedirect(command);
        } else if (isCmdWithTerminate(command)) {
            cmdWithTerminate(command);
        } else if (isCmdWithRecursive(command)) {
            cmdWithRecursive(command);
        } else if (isCmdWithRedirectRecursive(command)) {
            cmdWithRedirectAndRecursive(command);
        } else {
            fprintf(stderr, "Error: invalid command\n");
        }
        free(command);
        fflush(stdout);
    }
}
