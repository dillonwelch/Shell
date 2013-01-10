/*******
 * Dillon Welch
 * Original code from Dr. Duncan
 * Did CSC 222 Group Project with Nathan Lapp and James Poore
 *
 * VarSet:
 *    Representing a set of variables
 *    Each entry in the set contains a name and value
 *    In our implementation this is a linked list (with dummy head node)
 *    Several functions are provided to access/use this set.
 *******/

#ifndef __VAR_SET
#define __VAR_SET

#include <stdio.h>

typedef struct varSet
{
    char* name;   // REFERENCE is OWNED
    char* value;  // REFERENCE is OWNED
    struct varSet *next;  // REFERENCE is OWNED
} VarSet;

VarSet* createVarSet();
void freeVarSet(VarSet* set);
void addToSet(VarSet* set, char* name, char* value, int tokenType);
VarSet* findInSet(VarSet* set, char* name);
void printSet(VarSet* set, FILE* stream);

#endif
