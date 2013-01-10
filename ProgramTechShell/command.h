/*******
 * Christian Duncan
 *
 * Command
 *    A specific command with a list of arguments
 *******/

#ifndef __COMMAND_H
#define __COMMAND_H

#include <stdio.h>

typedef struct argList
{
    char* arg;             // The argument string (REFERENCE is OWNED)
    struct argList* next;  // The next in the list (REFERENCE is OWNED)
} ArgList;

typedef struct
{
    char* command;  // The command name itself (REFERENCE is OWNED)
    ArgList* head;  // The head of the argument list (REFERENCE is OWNED)
    ArgList* tail;  // The tail of the argument list (REFERENCE is BORROWED - part of head's list)
    enum { STDIN, PIPE_IN } input;  // Identifies whether command gets input from stdin or a pipe
    enum { STDOUT, PIPE_OUT } output;  // Identifies whether command sends output to stdout or a pipe
    int inFd;
    int outFd;
    int errFd;
} Command;

/***
 * A list of commands
 ***/
typedef struct cmdList
{
    Command* cmd;         // The command (REFERENCE is OWNED)
    struct cmdList* next; // The next in the list (REFERENCE is OWNED)
} CmdList;

/***
 * A statement is a list of commands
 ***/
typedef struct statement
{
    CmdList* head;        // The head of the list of commands (REFERENCE is OWNED)
    CmdList* tail;        // The tail (for insertion) - REFERENCE is BORROWED - part of head's list
} Statement;

Command* newCommand(const char* cmd);
void freeCommand(Command* cmd);
void printCommand(Command* cmd, FILE* stream);
void processCommand(Command* cmd);
void executeCommand(Command* cmd);
void addArg(Command* cmd, const char* arg);
char **argListToArray(const char *cmdName, const ArgList* list);  // References returned are GIVEN
void freeArgArray(char **argArray);    // Free the argument array given

Statement* newStatement();
void freeStatement(Statement* stmt);
void addCommand(Statement* stmt, Command* cmd);
void processStatement(Statement* stmt);
#endif
