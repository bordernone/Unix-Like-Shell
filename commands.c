#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "helper.h"
#include "parser.h"
#include "backgroundprocesses.h"

#define MAX_ARGC 1024

extern int debug;
extern int currentRunningChildPid;
extern struct BackgroundProcess backgroundProcesses[];

int blankCommand() { return 0; }

int cdCommand(char *path) {
    return chdir(path);
}

int exitCommand(char *cmd) {
    if (backgroundProcesses[0].cmd != NULL) {
        // Print to stderr
        fprintf(stderr, "Error: there are suspended jobs\n");
        return 0;
    }
    free(cmd);
    exit(0);
    // TODO: kill all child processes
    return 0;
}

int fgCommand(int jobIndex) {
    jobIndex--;
    if (jobIndex < 0 || jobIndex >= MAX_BACKGROUND_PROCESSES || backgroundProcesses[jobIndex].cmd == NULL) {
        fprintf(stderr, "Error: invalid job\n");
        return -1;
    }

    // Make a copy of the arguments
    char *fullCmd = malloc(sizeof(char) * strlen(backgroundProcesses[jobIndex].cmd));
    strcpy(fullCmd, backgroundProcesses[jobIndex].cmd);
    char *name = malloc(sizeof(char) * strlen(backgroundProcesses[jobIndex].name));
    strcpy(name, backgroundProcesses[jobIndex].name);
    int childPid = backgroundProcesses[jobIndex].pid;

    // Remove the background process
    removeBackgroundProcess(jobIndex);
    
    if (debug) {
        printf("Sent SIGCONT to child with pid %d \n", childPid);
    }

    // Send SIGCONT to the process
    kill(childPid, SIGCONT);


    // Wait for the process to finish
    currentRunningChildPid = childPid;
    int status;
    waitpid(childPid, &status, WUNTRACED);
    currentRunningChildPid = -1;

    // Check if the child was stopped
    if (WIFSTOPPED(status)) {
        if (debug) {
            printf("Child was stopped\n");
        }
        addBackgroundProcess(childPid, name, fullCmd);
    }

    // Check if child was terminated
    if (WIFSIGNALED(status)) {
        if (debug) {
            printf("Child was terminated\n");
        }
    }

    // Flush stdout, stdin
    fflush(stdout);
    fflush(stdin);

    return status;
}

int jobsCommand() {
    for (int i = 0; i < MAX_BACKGROUND_PROCESSES; i++) {
        if (backgroundProcesses[i].cmd != NULL) {
            printf("[%d] %s\n", i + 1, backgroundProcesses[i].cmd);
        } else {
            break;
        }
    }
    return 0;
}

void runCmdRecursive(struct Cmd *cmds, int currentIndex, int size, int myStdIn, int lastStdOut) {
    if (currentIndex < (size - 1)) {
        // create a pipe
        int pipefd[2];
        pipe(pipefd);

        // Old stdin
        int oldStdin = dup(STDIN_FILENO);
        if (myStdIn != -1) {
            dup2(myStdIn, STDIN_FILENO);
        }

        // Old stdout
        int oldStdout = dup(STDOUT_FILENO);
        dup2(pipefd[1], STDOUT_FILENO);
        
        // int myStdOut = pipefd[1];
        int futureStdIn = pipefd[0];
        int childPid = fork();
        if (childPid == 0) {
            // Child
            execv(cmds[currentIndex].cmd, cmds[currentIndex].argv);
            // If execv returns, it failed
            fprintf(stderr, "Error: invalid program\n");
            // Close pipes
            close(pipefd[0]);
            close(pipefd[1]);
            exit(0);
        } else {
            // Parent
            // Close Pipe
            close(pipefd[1]);

            // Restore
            dup2(oldStdout, STDOUT_FILENO);
            dup2(oldStdin, STDIN_FILENO);

            int helperChild = fork();
            if (helperChild == 0) {
                runCmdRecursive(cmds, currentIndex + 1, size, futureStdIn, lastStdOut);
                exit(0);
            } else {                
                close(pipefd[0]);
                // Wait for the child to finish
                int status;
                waitpid(childPid, &status, WUNTRACED);

                // Check if child was stopped
                if (WIFSTOPPED(status)) {
                    if (debug) {
                        printf("Child was stopped\n");
                    }
                    // addBackgroundProcess(childPid, cmds[currentIndex].name, cmds[currentIndex].cmd);
                }

                // Check if child was terminated
                if (WIFSIGNALED(status)) {
                    if (debug) {
                        printf("Child was terminated\n");
                    }
                }

                // Wait for the helper child to finish
                int helperStatus;
                waitpid(helperChild, &helperStatus, WUNTRACED);

                // Check if child was stopped
                if (WIFSTOPPED(helperStatus)) {
                    if (debug) {
                        printf("Child was stopped\n");
                    }
                    // addBackgroundProcess(childPid, cmds[currentIndex].name, cmds[currentIndex].cmd);
                }

                // Check if child was terminated
                if (WIFSIGNALED(helperStatus)) {
                    if (debug) {
                        printf("Child was terminated\n");
                    }
                }
            }
        }
    } else {
        // Last
        int oldStdin = dup(STDIN_FILENO);
        if (myStdIn != -1) {
            dup2(myStdIn, STDIN_FILENO);
        }
        
        int oldStdout = dup(STDOUT_FILENO);
        if (lastStdOut != -1) {
            dup2(lastStdOut, STDOUT_FILENO);
        }

        int childPid = fork();
        if (childPid == 0) {
            // Child
            execv(cmds[currentIndex].cmd, cmds[currentIndex].argv);
            // If execv returns, it failed
            fprintf(stderr, "Error: invalid program\n");
            
            close(myStdIn);
            exit(0);
        } else {
            // Parent
            close(myStdIn);

            // Restore
            dup2(oldStdin, STDIN_FILENO);
            dup2(oldStdout, STDOUT_FILENO);

            // Wait for the child to finish
            int status;
            waitpid(childPid, &status, WUNTRACED);

            // Check if child was stopped
            if (WIFSTOPPED(status)) {
                if (debug) {
                    printf("Child was stopped\n");
                }
                // addBackgroundProcess(childPid, cmds[currentIndex].name, cmds[currentIndex].cmd);
            }

            // Check if child was terminated
            if (WIFSIGNALED(status)) {
                if (debug) {
                    printf("Child was terminated\n");
                }
            }

            exit(0);
        }
    }
}

int runCmd(char *programPath, char *args[], int newStdin, int newStdout,
           const char *fullCmd) { // fullCmd is used for storing full command for jobs
    // Check if the program exists
    if (access(programPath, F_OK) == -1) {
        fprintf(stderr, "Error: invalid program\n");
        return -1;
    } else {
        int orgStdin = dup(STDIN_FILENO);
        int orgStdout = dup(STDOUT_FILENO);

        if (newStdin != -1) {
            fflush(stdout);
            dup2(newStdin, STDIN_FILENO);
        }

        if (newStdout != -1) {
            dup2(newStdout, STDOUT_FILENO);
        }

        int childPid = fork();

        if (childPid == -1) {
            fprintf(stderr, "Error: fork failed\n");

            // restore stdin and stdout
            if (newStdin != -1) {
                dup2(orgStdin, STDIN_FILENO);
            }

            if (newStdout != -1) {
                dup2(orgStdout, STDOUT_FILENO);
            }

            // Flush stdout, stdin
            fflush(stdout);
            fflush(stdin);

            return -1;
        } else if (childPid == 0) {
            // child process
            execv(programPath, args);
            // if execv returns, it means there was an error
            perror("execv");
            exit(1);
        } else {
            // parent process
            currentRunningChildPid = childPid;
            int status;
            waitpid(childPid, &status, WUNTRACED);
            currentRunningChildPid = -1;

            // Check if the child was stopped
            if (WIFSTOPPED(status)) {
                if (debug) {
                    printf("Child was stopped\n");
                }
                addBackgroundProcess(childPid, args[0], fullCmd);
            }

            // Check if child was terminated
            if (WIFSIGNALED(status)) {
                if (debug) {
                    printf("Child was terminated\n");
                }
            }

            // restore stdin and stdout
            if (newStdin != -1) {
                dup2(orgStdin, STDIN_FILENO);
            }

            if (newStdout != -1) {
                dup2(orgStdout, STDOUT_FILENO);
            }

            // Flush stdout, stdin
            fflush(stdout);
            fflush(stdin);

            return status;
        }
    }
}

// [cmd] '<' [filename] [terminate]
int cmdWithRedirectAndTerminate(const char *cmd) {
    char *cmdCopy = malloc(strlen(cmd) + 1);
    strcpy(cmdCopy, cmd);

    char *cmdWithArgs = getStringBefore(cmdCopy, '<');
    char *filenameAndTerminate = getStringAfter(cmdCopy, '<');
    cmdWithArgs = removeWhitespaces(cmdWithArgs, true);
    filenameAndTerminate = removeWhitespaces(filenameAndTerminate, true);

    char *filename = getStringBefore(filenameAndTerminate, ' ');
    char *terminateStr = getStringAfter(filenameAndTerminate, ' ');
    filename = removeWhitespaces(filename, true);
    terminateStr = removeWhitespaces(terminateStr, true);

    // Parse commands
    struct Cmd cmdParsed = parseCmd(cmdWithArgs);
    struct Terminate terminate = parseTerminate(terminateStr);

    // Open file
    int stdinFd = open(filename, O_RDONLY);
    if (stdinFd == -1) {
        fprintf(stderr, "Error: invalid file\n");
    } else {
        int stdoutFd = -1;
        if (!terminate.isEmpty) {
            if (terminate.isAppend) {
                stdoutFd = open(terminate.filename, O_WRONLY | O_APPEND | O_CREAT, 0644);
            } else {
                stdoutFd = open(terminate.filename, O_WRONLY | O_TRUNC | O_CREAT, 0644);
            }
        }
        // Run command
        runCmd(cmdParsed.cmd, cmdParsed.argv, stdinFd, stdoutFd, cmdCopy);
        // Close file
        close(stdinFd);
        if (stdoutFd != -1) {
            close(stdoutFd);
        }
    }

    // Free memory
    free(cmdCopy);
    free(filenameAndTerminate);
    free(cmdWithArgs);
    free(filename);
    free(terminateStr);
    free(terminate.filename);
    for (int i = 0; i < cmdParsed.argc; i++) {
        free(cmdParsed.argv[i]);
    }
    free(cmdParsed.cmd);
    return 0;
}

// [cmd] [terminate] '<' [filename]
int cmdWithTerminateAndRedirect(const char *cmd) {
    char *cmdCopy = malloc(strlen(cmd) + 1);
    strcpy(cmdCopy, cmd);

    struct StringSplit split = splitString(cmdCopy, '<');
    split.str[0] = removeWhitespaces(split.str[0], true);
    split.str[1] = removeWhitespaces(split.str[1], true);
    char *cmdWithArgsandTerminate = split.str[0];
    char *stdinFilename = split.str[1];

    struct CmdTerminate cmdTerminate = parseCmdTerminate(cmdWithArgsandTerminate);

    // Parse commands
    struct Cmd cmdParsed = cmdTerminate.cmd;
    struct Terminate terminate = cmdTerminate.terminate;

    // Open files
    int stdinFd = open(stdinFilename, O_RDONLY);
    if (stdinFd == -1) {
        fprintf(stderr, "Error: invalid file\n");
    } else {
        int stdoutFd = -1;
        if (!terminate.isEmpty) {
            if (terminate.isAppend) {
                stdoutFd = open(terminate.filename, O_WRONLY | O_APPEND | O_CREAT, 0644);
            } else {
                stdoutFd = open(terminate.filename, O_WRONLY | O_TRUNC | O_CREAT, 0644);
            }
        }
        // Run command
        runCmd(cmdParsed.cmd, cmdParsed.argv, stdinFd, stdoutFd, cmdCopy);
        // Close files
        close(stdinFd);
        if (stdoutFd != -1) {
            close(stdoutFd);
        }
    }

    // Free memory
    free(terminate.filename);
    for (int i = 0; i < cmdParsed.argc; i++) {
        free(cmdParsed.argv[i]);
    }
    free(cmdParsed.cmd);
    for (int i = 0; i < split.size; i++) {
        free(split.str[i]);
    }
    free(cmdCopy);

    return 0;
}

// [cmd] [terminate]
int cmdWithTerminate(const char *cmd) {
    struct CmdTerminate cmdTerminate = parseCmdTerminate(cmd);

    // Parse commands
    struct Cmd cmdParsed = cmdTerminate.cmd;
    struct Terminate terminate = cmdTerminate.terminate;

    // Open files
    int stdoutFd = -1;
    if (!terminate.isEmpty) {
        if (terminate.isAppend) {
            stdoutFd = open(terminate.filename, O_WRONLY | O_APPEND | O_CREAT, 0644);
        } else {
            stdoutFd = open(terminate.filename, O_WRONLY | O_TRUNC | O_CREAT, 0644);
        }
    }
    // Run command
    runCmd(cmdParsed.cmd, cmdParsed.argv, -1, stdoutFd, cmd);
    // Close files
    if (stdoutFd != -1) {
        close(stdoutFd);
    }

    // Free memory
    free(terminate.filename);
    for (int i = 0; i < cmdParsed.argc; i++) {
        free(cmdParsed.argv[i]);
    }
    free(cmdParsed.cmd);

    return 0;
}

// [cmd] | [cmd] | [cmd] | ... | [cmd] [terminate]
int cmdWithRecursive(const char *cmd) {
    char *cmdCopy = malloc(strlen(cmd) + 1);
    strcpy(cmdCopy, cmd);

    struct StringSplit blocks = splitString(cmdCopy, '|');
    for (int i = 0; i < blocks.size; i++) {
        blocks.str[i] = removeWhitespaces(blocks.str[i], true);
    }

    struct Cmd *cmds = malloc(sizeof(struct Cmd) * blocks.size);
    struct CmdTerminate cmdTerminate = parseCmdTerminate(blocks.str[blocks.size - 1]);
    for (int i = 0; i < blocks.size - 1; i++) {
        cmds[i] = parseCmd(blocks.str[i]);
    }
    cmds[blocks.size - 1] = cmdTerminate.cmd;
    struct Terminate terminate = cmdTerminate.terminate;

    // Run commands
    int lastStdoutFd = terminateFd(terminate);
    runCmdRecursive(cmds, 0, blocks.size, -1, lastStdoutFd);

    // Free memory
    for (int i = 0; i < blocks.size; i++) {
        free(blocks.str[i]);
    }
    for (int i = 0; i < blocks.size; i++) {
        for (int j = 0; j < cmds[i].argc; j++) {
            free(cmds[i].argv[j]);
        }
        free(cmds[i].cmd);
    }
    free(cmds);
    free(terminate.filename);
    free(cmdCopy);
    return 0;
}

// [cmd] '<' [filename] | [cmd] | [cmd] | ... | [cmd] [terminate]
int cmdWithRedirectAndRecursive(const char *cmd) {
    char *cmdCopy = malloc(strlen(cmd) + 1);
    strcpy(cmdCopy, cmd);

    struct StringSplit blocks = splitString(cmdCopy, '|');
    for (int i = 0; i < blocks.size; i++) {
        blocks.str[i] = removeWhitespaces(blocks.str[i], true);
    }

    struct Cmd *cmds = malloc(sizeof(struct Cmd) * blocks.size);
    struct StringSplit cmdWithFilename = splitString(blocks.str[0], '<');
    for (int i = 0; i < cmdWithFilename.size; i++) {
        cmdWithFilename.str[i] = removeWhitespaces(cmdWithFilename.str[i], true);
    }
    cmds[0] = parseCmd(cmdWithFilename.str[0]);

    struct CmdTerminate cmdTerminate = parseCmdTerminate(blocks.str[blocks.size - 1]);
    for (int i = 1; i < blocks.size; i++) {
        cmds[i] = i == (blocks.size - 1) ? cmdTerminate.cmd : parseCmd(blocks.str[i]);
    }

    struct Terminate terminate = cmdTerminate.terminate;
    int stdInFd = open(cmdWithFilename.str[1], O_RDONLY);
    if (stdInFd == -1) {
        fprintf(stderr, "Error: invalid file\n");
    } else {
        // Run commands
        int lastStdoutFd = terminateFd(terminate);
        runCmdRecursive(cmds, 0, blocks.size, stdInFd, lastStdoutFd);
    }

    // Free memory
    for (int i = 0; i < blocks.size; i++) {
        free(blocks.str[i]);
    }
    for (int i = 0; i < blocks.size; i++) {
        for (int j = 0; j < cmds[i].argc; j++) {
            free(cmds[i].argv[j]);
        }
        free(cmds[i].cmd);
    }
    free(cmds);
    free(terminate.filename);
    free(cmdWithFilename.str[0]);
    free(cmdWithFilename.str[1]);
    free(cmdCopy);
    return 0;
}