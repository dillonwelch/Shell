/*******
 * Dillon Welch
 * Original code from Dr. Duncan
 * Did CSC 222 Group Project with Nathan Lapp and James Poore
 *
 * Command
 *    See command.h for details.
 *******/

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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

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
    ans->input = STDIN; // By default
    ans->output = STDOUT; // By default
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
        ArgList* next = head->next; // Just in case ref. is lost
        free(head->arg);
        free(head);
        head = next;
    }

    free(cmd);
}

/***
 * processCommand:
 *    Process the command.
 *    Execute the commands
 *       Some are via exec
 *       Otherwise process certain builtin commands.
 *    REFERENCEs are BORROWED
 *    Returns process id of child command (0 if builtin)
 ***/
int processCommand(Command* cmd)
{
    assert(cmd != NULL);

    if (!processBuiltin(cmd)) // processBuiltin will execute a builtin
    {
        int a = 0; // Index for args.
        char **args = (char**) calloc(1000, sizeof(char*)); // Argument array (for execvp).

        ArgList* curr = cmd->head;
        *(args + a) = cmd->command;
        for (a = 1; curr != NULL; a++, curr = curr->next) // Populates the array with cmd and its args.
        {
            *(args + a) = curr->arg;
        }

        *(args + a) = NULL; // Null terminated array necessary for execvp.

        int inputPipe = comm[0];
        if (cmd->output == PIPE_OUT || cmd->output == OFILE) // If output is to a pipe or a file
        {
            // Create a new pipe
            if (pipe(comm) == -1)
            {
                fprintf(stderr, "Error occurred opening pipe: %s\n", strerror(errno));
                exit(1);
            }
        }

        int child = fork();
        if (child == 0)
        {
            // Child process
            if (cmd->input == PIPE_IN) // If input is from a pipe, redirect input from previous pipe.
            {
                dup2(inputPipe, 0); // Make Inputstream (of this proc.) be pipe in.
                close(inputPipe); // inputPipe is now 0.
            }

            if (cmd->output == PIPE_OUT) // If output is to a pipe, redirect output to a new pipe.
            {
                dup2(comm[1], 1); // Make Outstream of this process be pipe out.
                close(comm[1]); // Comm[1] is now 1.
                close(comm[0]); // Important: close streams you don't need.
            }

            if (cmd->input == IFILE || inputFlag == 1) // If input is from a file, redirect input from the file.
            {
                int file = open(input, O_RDONLY); // If the file exists, open it.
                if(file == -1) // If the file doesn't exist, print an error and exit.
                {
                    fprintf(stderr, "%s: File does not exist\n", input);
                    exit(1);
                }
                dup2(file, 0); // Make the file stream be the input stream.
                close(file); // file is now 0.
                free(input);
            }

            if (cmd->output == OFILE || outputFlag == 1) // If output is to a file, redirect to the file.
            {
                int file = open(output, O_WRONLY); // If the file exists, open it.
                if(file == -1)
                {
                    file = creat(output, S_IRWXU); // If it doesn't exist, create it.
                }
                dup2(file, 1); // Make outstream of this process to be the file.
                close(file); // File is now 1.
                close(comm[0]); // Don't need this anymore.
                free(output);
            }

            if (cmd->output == ERRFILE || errorFlag == 1)
            {
                int file = open(error, O_WRONLY); // If the file exists, open it.
                if(file == -1)
                {
                    file = creat(error, S_IRWXU); // If it doesn't exist, create it.
                }
                dup2(file, 2);  // Make outstream to be error.
                close(file);    // file is now 2.
                close(comm[0]); // Don't need this anymore.
                free(error);
            }

            if (execvp(cmd->command, args) == -1) // Execute the command, if it fails then print an error and exit.
            {
                fprintf(stderr, "Error: Command not recognized\n");
            }
            exit(1);
        }
        else
        {
            // Close all unneeded pipes
            if (cmd->input == PIPE_IN) close(inputPipe);
            if (cmd->output == PIPE_OUT) close(comm[1]);
        }
        free(args);
        args = NULL;
        findDir();
        return child;
    }
    else
    {
        // Builtins return 0 for child id
        findDir();
        return 0;
    }
}

/***
 * addArg:
 *    Add a new argument to the command
 *    REFERENCEs are BORROWED
 ***/
void addArg(Command* cmd, const char* arg, int token)
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
    newArg->tokenType = token;

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
