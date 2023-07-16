#ifndef NYUSH_BACKGROUNDPROCESSES_H
#define NYUSH_BACKGROUNDPROCESSES_H

#define MAX_BACKGROUND_PROCESSES 150

struct BackgroundProcess {
    int pid;
    char *name;
    char *cmd;
};

double currentTimeInMilliseconds();
void addBackgroundProcess(int pid, const char *name, const char *cmd);
void removeBackgroundProcess(int index);

#endif 
