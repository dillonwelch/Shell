/*******
 * Christian Duncan
 *
 * Command
 *    See command.h for details.
 *******/

// #define __DEBUG__

#include "command.h"
#include "global.h"
#include "builtins.h"
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_PROCS 100     // Maximum number of processes per statement

static int procCount = 0;        // Current number of active processes
static pid_t pidList[MAX_PROCS];    // List of active child processes (stored by pid)


/***
 * newCommand:
 *   Create a new command using given string
 *   Creates a null list of arguments
 *   REFERENCE returned is GIVEN
 ***/
Command* newCommand(const char* cmd)
{
    Command* ans = malloc(sizeof(Command));
    ans->command = strdup(cmd);
    ans->head = NULL;
    ans->tail = NULL;
    ans->input = STDIN;    // By default
    ans->output = STDOUT;  // By default
    ans->inFd = 0;
    ans->outFd = 1;
    ans->errFd = 2;
    return ans;
}

/***
 * freeCommand:
 *   Frees up the given command - and its argument list
 *   REFERENCE given is STOLEN (and freed)
 ***/
void freeCommand(Command* cmd)
{
    free(cmd->command);

    ArgList* head = cmd->head;
    while (head != NULL)
    {
        ArgList* next = head->next;  // Just in case ref. is lost
        free(head->arg);
        free(head);
        head = next;
    }

    free(cmd);
}

/***
 * executeCommand:
 *    Executes the given command...
 *    REFERENCEs are BORROWED
 ***/
void executeCommand(Command* cmd)
{
    // Get the input and output streams if needed

    // We need to fork a new process
    // Store the child process id
    if (procCount >= MAX_PROCS)
    {
        // No room left to store it!  Too many processes to handle on this line
        fprintf(stderr, ">> Error: Too many pipes.\n");
        return;
    }

    pidList[procCount++] = fork();
    if (pidList[procCount-1] == 0)
    {
        // Child process
        // Set up a null terminated array for the arguments
        char **argv = argListToArray(cmd->command, cmd->head);

        if (cmd->inFd != 0)
        {
#ifdef __DEBUG__
            fprintf(stderr, "DEBUG: Duplicating (input) file descriptor %d\n", cmd->inFd);
#endif
            dup2(cmd->inFd, 0);     // Make Instream of this process be from the given file descriptor
            close(cmd->inFd);       // This FD is no longer needed
            // WARNING: Might also need to close the OTHER END!!!!
        }

        if (cmd->outFd != 1)
        {
#ifdef __DEBUG__
            fprintf(stderr, "DEBUG: Duplicating (output) file descriptor %d\n", cmd->outFd);
#endif
            dup2(cmd->outFd, 1);    // Make Outstream of this process be from the given file descriptor
            close(cmd->outFd);      // This FD is no longer needed
            // WARNING: Might also need to close the OTHER END!!!!
        }

        if (cmd->errFd != 2)
        {
            dup2(cmd->errFd, 2);    // Make Errstream of this process be from given fd
            close(cmd->errFd);      // This FD is no longer needed
        }

        execvp(cmd->command, argv);

        // Should never be needed unless exec failed
        int localNo = errno;
        fprintf(stderr, ">> Error: %s\n", strerror(localNo));
        freeArgArray(argv);
        exit(localNo);
    }
}

/***
 * printCommand:
 *    Print out the details of the given command
 *    REFERENCEs are BORROWED
 ***/
void printCommand(Command* cmd, FILE* stream)
{
    if (cmd == NULL)
    {
        // Empty command, nothing to execute or print
        return;
    }

    fprintf(stream, "Command: %s\n", cmd->command);
    fprintf(stream, "...Input: %s\n", (cmd->input == STDIN ? "STDIN" : "PIPE"));
    fprintf(stream, "...Output: %s\n", (cmd->output == STDOUT ? "STDOUT" : "PIPE"));

    if (cmd->head != NULL)
    {
        // Print out the argument list
        int a;
        ArgList* curr = cmd->head;
        for (a = 1; curr != NULL; a++, curr=curr->next)
        {
            fprintf(stream, "...Arg %d: %s\n", a, curr->arg);
        }
    }
    else
    {
        fprintf(stream, "...No arguments\n");
    }
}

/***
 * processCommand:
 *    Process the command.
 *    Execute the commands
 *       Some are via exec
 *       Otherwise process certain builtin commands.
 *    REFERENCEs are BORROWED
 ***/
void processCommand(Command* cmd)
{
    assert(cmd != NULL);

    if (!processBuiltin(cmd))
    {
        // It was not a built-in so execute normally
        executeCommand(cmd);
    }
}

/***
 * addArg:
 *    Add a new argument to the command
 *    REFERENCEs are BORROWED
 ***/
void addArg(Command* cmd, const char* arg)
{
    // Allocate memory (be sure to check for Out-of-mem)
    ArgList* newArg = malloc(sizeof(ArgList));

    if (newArg == NULL)
    {
        fprintf(stderr, ">> Error: Out of memory.  Token not added.\n");
        return;
    }

    // Store the contents (the new argument)
    newArg->arg = strdup(arg);
    newArg->next = NULL;

    // Insert into the arglist - at the tail (if not empty)
    if (cmd->head == NULL)
    {
        // First argument
        cmd->head = cmd->tail = newArg;
    }
    else
    {
        cmd->tail = cmd->tail->next = newArg;
    }
}

/***
 * argListToArray:
 *    Takes the argument list given and returns a NULL terminated
 *    cmdName is placed as argument 0.
 *    array containing the arguments.
 *    Values returned are all GIVEN references.
 *    They need to be deallocated - which is best done via freeArgArray
 *    Purpose: Is in preparation for execvp (to pass arguments)
 ***/
char **argListToArray(const char *cmdName, const ArgList* list)
{
    // See how many elements are in the array
    const ArgList* curr;
    int cnt;
    for (cnt = 1, curr = list; curr != NULL; cnt++, curr = curr->next);
    cnt++;                           // Account for the final NULL

    char **argv =
        malloc(cnt * sizeof(char *));  // Allocate space for the array

    // Place copies of the arguments into the array
    argv[0] = strdup(cmdName);
    for (cnt = 1, curr = list; curr != NULL; cnt++, curr = curr->next)
    {
        argv[cnt] = strdup(curr->arg);
    }
    argv[cnt] = NULL;                // Don't forget final terminator
    return argv;
}

/***
 * freeArgArray:
 *   Free up the argument array created from prev. function.
 *   REFERENCEs passed are STOLEN (and freed)
 ***/
void freeArgArray(char **argArray)
{
    assert(argArray != NULL);

    char **curr;
    for (curr = argArray; *curr != NULL; curr++)
    {
        free(*curr);   // Free up the argument copy
    }
    free(argArray);      // Free up the entire array
}

/***
 * newStatement:
 *    Create a new statement.
 *    REFERENCE returned is GIVEN
 ***/
Statement* newStatement()
{
    Statement* ans = malloc(sizeof(Statement));
    ans->head = ans->tail = NULL;
    return ans;
}

/***
 * freeStatement:
 *   Frees up the given statement - (the command list really)
 *   REFERENCE is STOLEN (and freed)
 ***/
void freeStatement(Statement* stmt)
{
    CmdList* head = stmt->head;
    while (head != NULL)
    {
        CmdList* next = head->next;  // Just in case ref. is lost
        free(head->cmd);
        free(head);
        head = next;
    }

    free(stmt);
}

/***
 * addCommand:
 *    Add the given command to the statement list
 *    stmt: REFERNCE is BORROWED
 *    cmd: REFERENCE is STOLEN (added to end of stmt's list)
 ***/
void addCommand(Statement* stmt, Command* cmd)
{
    // Allocate memory (be sure to check for Out-of-mem)
    CmdList* newCmd = malloc(sizeof(CmdList));

    if (newCmd == NULL)
    {
        fprintf(stderr, ">> Error: Out of memory.  Command not added.\n");
        freeCommand(cmd);  // Free command - since we STOLE it
        return;
    }

    // Store the contents (the new command)
    newCmd->cmd = cmd;
    newCmd->next = NULL;

    // Insert into the cmdList - at the tail
    if (stmt->head == NULL)
    {
        // First command
        stmt->head = stmt->tail = newCmd;
    }
    else
    {
        stmt->tail = stmt->tail->next = newCmd;
    }
}

/***
 * processStatement:
 *    Process the statement (execute the list of commands)
 *    REFERENCEs are BORROWED
 ***/
void processStatement(Statement* stmt)
{
    procCount = 0;          // Number of children processes
    assert(stmt != NULL);

    CmdList* curr;
    int pipeFd[2];  // Pipe streams for command communication
    for (curr = stmt->head; curr != NULL; curr = curr->next)
    {
        // Set up the 3 key streams for the current statement
        if (curr->cmd->input == PIPE_IN)
        {
            // Input comes from previous pipe
            curr->cmd->inFd = pipeFd[0];
        }
        else
        {
            // Input comes from standard in
            curr->cmd->inFd = 0;
        }

        if (curr->cmd->output == PIPE_OUT)
        {
            // Need to pipe output to another command (next one)
            // Create a pipe
            if (pipe(pipeFd) == -1)
            {
                // Error: can't process any further
                fprintf(stderr, ">> Error: %s\n", strerror(errno));
                return;
            }
            curr->cmd->outFd = pipeFd[1];
        }
        else
        {
            // Output goes to standard out
            curr->cmd->outFd = 1;
        }

        curr->cmd->errFd = 2;
        processCommand(curr->cmd);

        // We are done with the pipes (ourselves)
        //    The child processes might not be (but that is their job)
        if (curr->cmd->inFd != 0)
        {
            close(curr->cmd->inFd);
        }

        if (curr->cmd->outFd != 1)
        {
            close(curr->cmd->outFd);
        }

        if (curr->cmd->errFd != 2)
        {
            close(curr->cmd->errFd);
        }
    }

    // Now wait for all of the children to finish
    int leftOver = procCount;
    int status;
    int exitStatus;

    while (leftOver > 0)
    {
        // Still some child processes left.
        pid_t pid = wait(&status);
        // Which one?
        int i;
        for (i = 0; i < procCount; i++)
        {
            if (pidList[i] == pid)
            {
                // Found the match
                pidList[i] = -1;  // Mark it off
                leftOver--;
                break;            // And leave loop
            }
        }
        assert(i < procCount);  // Huh?  Should never happen.  Unless prog. error.

#ifdef __DEBUG__
        fprintf(stderr, ">> DEBUG: Proc %d exited with status %d\n", pid,
                WEXITSTATUS(status));
#endif

        if (i == procCount-1)
        {
            // The pid was the LAST statement... this is our exit status for whole statement
            exitStatus = status;
        }
    }

    if (exitStatusFlag)
    {
        // Print out exit status flag to standard error
        fprintf(stderr, ">> Done: Exit %d\n", WEXITSTATUS(exitStatus));
    }
}

