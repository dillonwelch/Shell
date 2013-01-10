/*******
 * Dillon Welch
 * Original code from Dr. Duncan
 * Did CSC 222 Group Project with Nathan Lapp and James Poore
 *
 *
 * Builtins
 *    A set of functions to process various built-in
 *    commands.
 *    Commands supported:
 *       SET
 *       LIST
 *       EXIT
 *       STATUS
 *       CD
 *       PWD
 *******/

#ifndef __BUILTINS_H
#define __BUILTINS_H

#include "command.h"
#include <stdio.h>

int processBuiltin(Command* cmd);
char *stringCopy(char *dest, const char *src, size_t n, size_t homeLength);
void findDir();
#endif
