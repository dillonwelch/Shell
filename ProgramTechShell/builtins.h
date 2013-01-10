/*******
 * Christian Duncan
 *
 * Builtins
 *    A set of functions to process various built-in
 *    commands.
 *    Commands supported:
 *       SET
 *       LIST
 *       ...
 *******/

#ifndef __BUILTINS_H
#define __BUILTINS_H

#include "command.h"
#include <stdio.h>

int processBuiltin(Command* cmd);

#endif
