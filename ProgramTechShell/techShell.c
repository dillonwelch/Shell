/*******
 * Christian Duncan
 *
 * TechShell
 *   This program is a simple shell script
 *   that is only partially functional.
 *
 *   It supports recognizing several built-in commands:
 *     SET [var] [value]: set the variable "var" to the given "value" argument.
 *               default value is ""
 *     LIST: prints a list of all current known variables and their values.
 *     ENV [var]: Makes the given variable from the list an environment
 *         variable (exported to subprocesses)
 *
 *   It ignores COMMENTS
 *     A COMMENT is started by the token # and continues to end of the line.
 *
 *   It also executes STATEMENTS
 *     A STATEMENT is a sequence of piped commands that ends with either
 *     a new line or a semicolon.
 *     The exit status of a statement is the exit status of the last
 *     command in the sequence.
 *
 *   Redirects:
 *      Commands can have the standard output and standard input and
 *      standard error redirected.
 *      < FILE: Redirects standard input from FILE.
 *      > FILE: Redirects standard output to FILE.
 *      >& FILE: Redirects standard error to FILE.
 *
 *   Variable substitution:
 *      Variables are substituted using the following sequence:
 *        $var$
 *      ...
 ********/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "global.h"
#include "tokenizer.h"
#include "varSet.h"
#include "command.h"
#include "builtins.h"

#define MAX_LINE_LENGTH 500
#define MAX_SUBSTITUTION_LEVEL 10

// The set of variables in this shell.
VarSet* varList = NULL;

int interactiveFlag;   // Defines if in interactive mode or not (for prompting)

/***
 * preprocess:
 *   Takes a given token and does variable replacement (if needed)
 *   Returns a string representing the expanded string.
 *      Sets changeFlgag to 1 if any substitution was made and 0 otherwise
 *   REFERENCE returned is GIVEN
 ***/
char* preprocess(char* token, int *changeFlag)
{
    *changeFlag = 0;
    // Find variable names that are separated by $...$
    char *response = malloc((MAX_LINE_LENGTH+1)*sizeof(char));
    char *responseEnd = response + MAX_LINE_LENGTH;
    char *currResponse, *curr, *start;

    start = NULL;

    // Go through each character in the token
    for (currResponse = response, curr = token; *curr != '\0'; curr++)
    {
        if (*curr == '$')         // Variable name delimiter
        {
            if (start == NULL)      //    Start of variable name
            {
                *changeFlag = 1;       // A substitution will have been made (or truncated!)
                start = curr;
            }
            else
            {
                char temp = *curr;    //    Mark the end with a 0
                *curr = '\0';
                // Lookup the variable name in the varSet
                VarSet* match = findInSet(varList, start+1);
                *curr = temp;         //    Replace previous character back (so transparent - safer)
                if (match != NULL)
                {
                    // No error if no match found - and copy value if there is.
                    char* copy;
                    for (copy = match->value; *copy != '\0' && currResponse < responseEnd;
                            copy++, currResponse++)
                    {
                        *currResponse = *copy;
                    }
                }
                start = NULL;         // Reset to next variable name
            }
        }
        else
        {
            if (start == NULL)
            {
                if (currResponse < responseEnd)
                {
                    // If too long - it is just ignored - otherwise character is copied in.
                    *currResponse = *curr;
                    currResponse++;
                }
            }
        }
    }
    *currResponse = '\0';   // Terminate our (expanded) string copy
    return response;
}

/***
 * processLine:
 *    line: string to process (REFERENCE is BORROWED)
 ***/
void processLine(char* line)
{
    enum { CMD, PIPED_CMD, ARGS } processMode ;
    processMode = CMD;
    Statement* stmt = newStatement();   // Store the current statement
    Command* cmd = NULL;
    int doneFlag = 0;
    char *expandedToken = NULL;

    startToken(line);
    aToken answer;

    answer = getNextToken();
    while (!doneFlag)
    {
        switch (answer.type)
        {
        case ERROR:
            // Error (for some reason)
            fprintf(stderr, "Error parsing line.\n");
            if (cmd != NULL)
            {
                freeCommand(cmd);
                cmd = NULL;
            }

            if (stmt != NULL)
            {
                freeStatement(stmt);
                stmt = NULL;
            }
            return;

        case BASIC:
        case DOUBLE_QUOTE:
        case SINGLE_QUOTE:
            if (answer.type != SINGLE_QUOTE)
            {
                int changeFlag = 1;
                int count = 1;
                expandedToken = preprocess(answer.start, &changeFlag);
                while (changeFlag && count++ < MAX_SUBSTITUTION_LEVEL)
                {
                    // Basic and Double Quote tokens can have variable substitutions
                    //     Repeat as many times as needed until all recursive levels are done.
                    //     Capped at a large level to prevent INFINITE LOOP!
                    char* previousExpansion = expandedToken;
                    expandedToken = preprocess(previousExpansion, &changeFlag);
                    free(previousExpansion);  // Free up old expanded token.
                }
            }
            else
            {
                // Otherwise just duplicate the token
                expandedToken = strdup(answer.start);
            }

            if (processMode == CMD)
            {
                // This is a new command
                assert (cmd == NULL);
                cmd = newCommand(expandedToken);
                processMode = ARGS;  // Switch modes
            }
            else if (processMode == PIPED_CMD)
            {
                // This is a new command after a pipe
                cmd = newCommand(expandedToken);
                cmd->input = PIPE_IN;
                processMode = ARGS;        // Switch modes
            }
            else if (processMode == ARGS)
            {
                // This is a new argument
                assert(cmd != NULL);
                addArg(cmd, expandedToken);
            }
            free(expandedToken);  // Don't forget - we OWN this REFERENCE
            expandedToken = NULL;
            break;

        case PIPE:
            // We have a pipe, so command is now completed and ready to be executed

            if (processMode == CMD || processMode == PIPED_CMD)
            {
                // A pipe while waiting for a command!
                // Empty (blank) statements for pipes are not allowed
                fprintf(stderr, "Error: Missing command\n");
                assert (cmd == NULL);  // Otherwise some programming error occurred! (Mem leak maybe?)
                freeStatement(stmt);
                stmt = NULL;
                return;
            }
            else
            {
                assert(cmd != NULL);       // Otherwise some prog. error - entered ARGS mode w/o a Command!
                cmd->output = PIPE_OUT;    // Set its output stream to that of a PIPE
                addCommand(stmt, cmd);     // Add command to statement list (REFERENCE is STOLEN)
                cmd = NULL;
                processMode = PIPED_CMD;  // Next command uses a piped command
            }
            break;

        case EOL:
            // EOL is nearly same as SEMICOLON - just flag done as well
            doneFlag = 1;

        case SEMICOLON:
            // We have a statement terminator
            if (processMode == PIPED_CMD)
            {
                // We are in a piped command mode (without having gotten any new command)
                // An empty statement - not allowed after a pipe
                fprintf(stderr, "Error: Broken pipe\n");
                assert (cmd == NULL);
                freeStatement(stmt);
                stmt = NULL;
                return;
            }
            else if (processMode == CMD)
            {
                assert (cmd == NULL);
                // An empty statement - is allowed but ignored
            }
            else
            {
                assert (cmd != NULL);
                addCommand(stmt, cmd);   // Add the finished command to the statement list (cmd REF is STOLEN)
                cmd = NULL;
                processStatement(stmt);
                freeStatement(stmt);     // Free this old statement
                stmt = newStatement();   // Create a new statement
            }
            processMode = CMD;  // Switch back to processing mode
            break;

        default:
            fprintf(stderr, "Programming Error: Unrecognized type returned!!!\n");
            if (cmd != NULL )
            {
                freeCommand(cmd);
                cmd = NULL;
            }
            if (stmt != NULL)
            {
                freeStatement(stmt);
                stmt = NULL;
            }
            return;
        }
        answer = getNextToken();
    }

    // Should only happen once doneFlag is set and SEMICOLON process is executed
    assert(cmd == NULL);

    if (stmt != NULL)
    {
        freeStatement(stmt);
        stmt = NULL;
    }
}

void printPrompt()
{
    fprintf(stdout, "$$ ");
}

int main(int argc, char* argv[])
{
    FILE* inStream;

    if (argc <= 1)
    {
        // No arguments given (in interactive mode)
        inStream = stdin;
        interactiveFlag = 1;
    }
    else
    {
        // Argument 1 is the script to run (non-interactive mode)
        interactiveFlag = 0;
        int localErr;
        inStream = fopen(argv[1], "r");
        localErr = errno;
        if (inStream == NULL)
        {
            // Unable to open the file
            fprintf(stderr, "Error: %s\n", strerror(localErr));
            exit(localErr);
        }
    }

    varList = createVarSet();

    char line[MAX_LINE_LENGTH+1];

    if (interactiveFlag != 0)
    {
        // Print out a prompt
        printPrompt();
    }

    while (fgets(line, MAX_LINE_LENGTH+1, inStream) != NULL)
    {
        // We have our current line
        processLine(line);

        if (interactiveFlag != 0)
        {
            // Print out a prompt
            printPrompt();
        }
    }

    // Everything ran smoothly
    return 0;
}
