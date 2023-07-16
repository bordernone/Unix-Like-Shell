#include <bits/types/time_t.h>
#include <malloc.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>

#define MAX_BACKGROUND_PROCESSES 150

struct BackgroundProcess {
    int pid;
    char *name;
    char *cmd;
};

extern struct BackgroundProcess backgroundProcesses[];

double currentTimeInMilliseconds() {
    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);
    return spec.tv_sec * 1000 + spec.tv_nsec / 1.0e6;
}

void addBackgroundProcess(int pid, const char *name, const char *cmd) {
    int indexToInsert = 0;
    for (; indexToInsert< 150; indexToInsert++) {
        if (backgroundProcesses[indexToInsert].name == NULL) {
            break;
        }
    }

    backgroundProcesses[indexToInsert].pid = pid;
    backgroundProcesses[indexToInsert].name = malloc(sizeof(char) * strlen(name));
    strcpy(backgroundProcesses[indexToInsert].name, name);
    backgroundProcesses[indexToInsert].cmd = malloc(sizeof(char) * strlen(cmd));
    strcpy(backgroundProcesses[indexToInsert].cmd, cmd);
}

void removeBackgroundProcess(int index) {
    free(backgroundProcesses[index].name);
    free(backgroundProcesses[index].cmd);
    backgroundProcesses[index].name = NULL;
    backgroundProcesses[index].cmd = NULL;
    backgroundProcesses[index].pid = -1;

    // Shift all the processes after this one
    for (int i = index; i < MAX_BACKGROUND_PROCESSES - 1; i++) {
        backgroundProcesses[i].cmd = backgroundProcesses[i + 1].cmd;
        backgroundProcesses[i].name = backgroundProcesses[i + 1].name;
        backgroundProcesses[i].pid = backgroundProcesses[i + 1].pid;
        if (backgroundProcesses[i].name == NULL) {
            break;
        }
    }
}