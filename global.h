/*******
 * Dillon Welch
 * Original code from Dr. Duncan
 * Did CSC 222 Group Project with Nathan Lapp and James Poore
 *
 * Global (external) variables
 *  for TechShell program.
 *******/

#include "varSet.h"

// These variables must be defined elsewhere, these are just declarations.

extern VarSet* varList; // Variable list
int comm[2]; // For piping
int status;  // Exit status.
int sFlag;   // Whether to print status or not.
char *dir;   // Current directory.
char *t;     // Tilde (for truncating directory with ~/).

int ioFlag;     // Redirect flags.
int inputFlag;
int outputFlag;
int errorFlag;

char *input;  // The names of the files for input, output, and error redirection.
char *output;
char *error;
