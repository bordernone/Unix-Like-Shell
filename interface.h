#ifndef NYUSH_INTERFACE_H
#define NYUSH_INTERFACE_H

void handleStopSignal(int signal);
void printPrompt();
char* getCommand();
void saveLastCommand(char *command);
void InitInterface();

#endif