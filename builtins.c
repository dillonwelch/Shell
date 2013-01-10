/*******
 * Dillon Welch
 * Original code from Dr. Duncan
 * Did CSC 222 Group Project with Nathan Lapp and James Poore
 *
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
void processStatus(Command* cmd);
void processCD(Command* cmd);
void processPWD(Command* cmd);

char *builtinNames[] = { "SET", "LIST", "EXIT", "STATUS", "CD", "PWD", NULL };
void (*builtinFn[])(Command*) = { processSet, processList, processExit, processStatus, processCD, processPWD, NULL };

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
        if (strcasecmp(cmd->command, builtinNames[i]) == 0) // Does the given command match the builtin string name
        {
            int savedIn;  // Saved input stream.
            int savedOut; // Saved output stream.
            if (cmd->input == PIPE_IN) // If input is from a pipe, dup the input stream.
            {
                savedIn = dup(0);
                dup2(comm[0], 0);
                close(comm[0]);
            }
            if (cmd->output == PIPE_OUT) // If output is from a pipe, dup the output stream.
            {
                savedOut = dup(1);
                if (pipe(comm) == -1)
                {
                    fprintf(stderr, "Error occurred opening pipe: %s\n", strerror(errno));
                    exit(1);
                }
                dup2(comm[1], 1);
                close(comm[1]);
            }

            (builtinFn[i])(cmd); // Execute the builtin.

            // Restore standard in and out if necessary
            if (cmd->input == PIPE_IN)
            {
                dup2(savedIn, 0);
                close(savedIn);
            }
            if (cmd->output == PIPE_OUT)
            {
                dup2(savedOut, 1);
                close(savedOut);
            }
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

    addToSet(varList, cmd->head->arg, cmd->head->next == NULL ? "" : cmd->head->next->arg, cmd->head->next == NULL ? -1 : cmd->head->next->tokenType);
}

/***
 * processList:
 *    List the variables and their values in the current shell
 ***/
void processList(Command* cmd)
{
    printSet(varList, stdout);
    fflush(stdout);
}

/***
 * processExit:
 *    Exits the shell.
 ***/
void processExit(Command* cmd)
{
    exit(EXIT_SUCCESS);
}

/***
 * processStatus:
 *     Toggles on/off reporting of the exit status (0 is off, 1 is on).
 ***/
void processStatus(Command* cmd)
{
    if(sFlag == 0)
    {
        sFlag = 1;
    }
    else
    {
        sFlag = 0;
    }
}

/***
 * processCD:
 *    Changes the directory. Changes to HOME (or root if there is no home) with no args, or to the arg if given.
 ***/
void processCD(Command* cmd)
{
    int error; // For storing the return value of chdir, will be -1 if there was an error.

    if(cmd->head == NULL) // If no arg given, chdir to HOME.
    {
        char *home = getenv("HOME");
        if(home == NULL) // If there is no home, chdir to root.
        {
            error = chdir("/");
        }
        else
        {
            error = chdir(home);
        }

        if(error == -1)
        {
            fprintf(stderr, "Unknown error with cd.\n");
        }
    }
    else // If an arg is given, chdir to arg.
    {
        error = chdir(cmd->head->arg);
        if(error == -1)
        {
            fprintf(stderr, "Directory %s not found.\n", cmd->head->arg);
        }
    }
    findDir();
}

/***
 * stringCopy:
 *    Customized copy function, pass it the length of HOME and it will copy src into dest from the correct point.
 ***/
char *stringCopy(char *dest, const char *src, size_t n, size_t homeLength)
{
    size_t i;
    // i = 2 because of ~/, i - 1 + homeLength starts at the first character after HOME/.
    for (i = 2 ; i < n && src[i - 1 + homeLength] != '\0' ; i++)
        dest[i] = src[i - 1 + homeLength];
    for ( ; i < n ; i++)
        dest[i] = '\0';

    return dest;

}

/***
 * findDir:
 *     Finds the current directory and sets dir to it (for printing the current directory in the prompt).
 ***/
void findDir()
{
    char tilde[2] = "~/"; // Replaces HOME part of current working directory.
    char *home = getenv("HOME");   // Gets the home directory (HOME) ands its length.
    int homeLength = strlen(home); // Length of HOME.
    char *holder = malloc(1000 * sizeof(char)); // Variable for the current working directory.
    getcwd(holder, 1000); 	// Gets the current working directory and stores it in holder.

    if(strncmp(holder, home, homeLength) == 0) // If the current working directory starts out with the home directory.
    {
        if(*(holder + homeLength) == '\0') // If the current working directory is just the home directory, print '~'.
        {
            dir = t;
        }
        else // Otherwise make a new string, copy ~/ into it, and then copy the part of the cwd that isn't the home directory.
        {
            char *pwd = malloc(1000 * sizeof(char));
            strncpy(pwd, tilde, 2);
            stringCopy(pwd, holder, 1000, homeLength);
            dir = pwd;
            free(pwd);
        }
    }
    else // Otherwise print out holder, as cwd does not have the home directory in it (for example "/").
    {
        dir = holder;
    }
    free(holder);
}

/***
 * processPWD:
 *    Prints the working directory, with the home directory truncated.
 ***/
void processPWD(Command *cmd)
{
    char tilde[2] = "~/"; // Replaces HOME part of current working directory.
    char *home = getenv("HOME");   // Gets the home directory ands its length.
    int homeLength = strlen(home); // Length of HOME.
    char *holder = malloc(1000 * sizeof(char)); // Variable for the current working directory.
    getcwd(holder, 1000); 	// Gets the current working directory and stores it in holder.

    if(strncmp(holder, home, homeLength) == 0) // If the current working directory starts out with the home directory..
    {
        if(*(holder + homeLength) == '\0') // If the current working directory is just the home directory, print '~'.
        {
            //printf("~\n");
            printf("%s\n", home);
        }
        else   // Otherwise make a new string, copy ~/ into it, and then copy the part of the cwd that isn't the home directory.
        {
            char *pwd = malloc(1000 * sizeof(char));
            strncpy(pwd, tilde, 2);
            stringCopy(pwd, holder, 1000, homeLength);
            printf("%s\n", pwd);
            dir = pwd;
            free(pwd);
        }
    }
    else // Otherwise print out holder, as cwd does not have the home directory in it.
    {
        printf("%s\n", holder);
        dir = holder;
    }
    free(holder);
}
