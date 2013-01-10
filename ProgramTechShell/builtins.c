/*******
 * Christian Duncan
 *
 * Builtins
 *    A set of functions to process various built-in
 *    commands.
 *    See builtins.h for any details.
 *******/

#include "builtins.h"
#include "global.h"
#include "varSet.h"
#include "command.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

void processSet(Command* cmd);
void processList(Command* cmd);
void processExit(Command* cmd);
void processCd(Command* cmd);
void processPrintStatus(Command* cmd);

char *builtinNames[] = { "CD", "SET", "LIST", "EXIT", "STATUS", NULL };
void (*builtinFn[])(Command*) = { processCd, processSet, processList,
                                  processExit, processPrintStatus,
                                  NULL
                                };

int exitStatusFlag = 0;   // Global variable: whether to print out exit status upon statement completion.

/***
 * processBuiltin:
 *    Determines if the given command is a builtin and executes
 *    it if so.
 *
 *    cmd: A BORROWED reference to the command to process
 *    Returns
 ***/
int processBuiltin(Command* cmd)
{
    assert(cmd->command != NULL);

    int i;
    for (i = 0; builtinNames[i] != NULL; i++)
    {
        // Does the given command match the builtin string name
        if (strcasecmp(cmd->command, builtinNames[i]) == 0)
        {
            // If so, execute the processing function for that command
            (builtinFn[i])(cmd);
            return 1;    // And return  1 (found builtin)
        }
    }

    return 0; // Did not find any builtin... execute normally
}

/***
 * processSet:
 *   Assign a variable a given value.
 *   Arg1: is the variable name
 *   Arg2: is the value.
 *   If Arg1 is empty - the command does nothing
 *   If Arg2 is empty - the command sets the variable to an empty string ""
 ***/
void processSet(Command* cmd)
{
    assert(cmd != NULL);
    if (cmd->head == NULL)
    {
        // No argument... do nothing
        return;
    }

    addToSet(varList, cmd->head->arg, cmd->head->next == NULL ? "" : cmd->head->next->arg);
}

/***
 * processList:
 *    List the variables and their values in the current shell
 ***/
void processList(Command* cmd)
{
#ifdef __DEBUG__
    fprintf(stderr, "DEBUG: Opening output file descriptor: %d\n", cmd->outFd);
#endif

    FILE* stream = fdopen(cmd->outFd, "a");  // Write to this stream (no need to close - fd not dup'ed)
    if (stream == NULL)
    {
        fprintf(stderr, "Error: %s\n", strerror(errno));
        return;
    }
    printSet(varList, stream);
    fflush(stream);
}

/***
 * processExit:
 *    Exit with exit status given by first argument (if a number)
 *    If no argument - returns 0.
 *    If first argument is not a valid number - behaviour is undefined
 *      (Could return any number)
 *      In our case, it uses atoi so gets the first part...
 ***/
void processExit(Command* cmd)
{
    if (cmd->head == NULL)
    {
        // No argument, just exit with value 0
        exit(0);
    }
    else
    {
        int status = atoi(cmd->head->arg);  // Get value of first argument
        exit(status);
    }
}

/***
 * processCd:
 *    Call chdir to change the current working directory
 ***/
void processCd(Command* cmd)
{
    const char *dirName;
    if (cmd->head == NULL)
    {
        // No argument, go to the home environment variable
        dirName = getenv("HOME");
        if (dirName == NULL)
            // No HOME environment variable set!  Use root /
            dirName = "/";
    }
    else
    {
        // Use first argument as location
        dirName = cmd->head->arg;
    }

    if (chdir(dirName) == -1)
    {
        // Error
        int localErr = errno;
        FILE* err = fdopen(cmd->errFd, "w");
        fprintf(err, ">> Error: %s\n", strerror(localErr));
        fflush(err);
    }
}

/***
 * processPrintStatus:
 *   Change the wait status notification.
 *   With no argument in the command list - toggles status.
 *   Otherwise, "ON" sets it on.
 *   "OFF" sets it off.
 *   If neither, then does nothing.
 *   Status: determines whether or not to print a statement's exit status.
 *       Default for status is OFF.
 ***/
void processPrintStatus(Command* cmd)
{
    if (cmd->head == NULL)
    {
        exitStatusFlag = !exitStatusFlag;
        return;
    }

    if (strcasecmp(cmd->head->arg, "OFF") == 0)
    {
        // Set it off
        exitStatusFlag = 0;
        return;
    }
    else if (strcasecmp(cmd->head->arg, "ON") == 0)
    {
        // Set it on
        exitStatusFlag = 1;
    }
}
