/*******
 * Christian Duncan
 *
 * VarSet: 
 *    See varSet.h for details.
 *******/

#include "varSet.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/***
 * createVarSet:
 *   Create a variable set (or one entry in the set)
 ***/
VarSet* createVarSet() {
  VarSet* ans = malloc(sizeof(VarSet));
  ans->name = NULL;
  ans->value = NULL;
  ans->next = NULL;
  return ans;
}

/***
 * freeVarSet:
 *    Free up the variable set (delete the linked list and contents)
 *    Must also free all OWNED references.
 ***/
void freeVarSet(VarSet* set) {
  VarSet* curr = set;
  while (curr != NULL) {
    if (curr->name != NULL) free(curr->name);
    if (curr->value != NULL) free(curr->value);
    VarSet* next = curr->next;
    free(curr);
    curr = next;
  }
}

/***
 * addToSet:
 *    Add the given name/value to the set
 *    If name exists - replace with new value
 *    If not, add the name/value to the list
 ***/
void addToSet(VarSet* set, char* name, char* value) {
  assert(set != NULL);  // Using a dummy head node - so verify it is created.

  // First lookup variable (if already exists)
  VarSet* locate = findInSet(set, name);
  if (locate == NULL) {
    // We have a new variable!
    locate = createVarSet();
    locate->name = strdup(name);
    locate->value = strdup(value);
    locate->next = set->next;
    set->next = locate;
  } else {
    // Replace
    if (locate->value != NULL) {
      free(locate->value);
    }
    locate->value = strdup(value);
  }
}

/***
 * findInSet:
 *    Searches for a given name in the set
 *    Returns the reference in the list for the matching name
 *    or NULL if not found.
 *    Matching is case sensitive.
 ***/
VarSet* findInSet(VarSet* set, char* name) {
  assert(set != NULL);   // Using a dummy head node - so verify it is created.

  VarSet* curr;
  for (curr = set->next; curr != NULL; curr = curr->next) {
    if (strcmp(name, curr->name) == 0) {
      // Found it
      return curr;
    }
  }

  // Nothing found
  return NULL;
}

/***
 * printSet:
 *    Print the given set to the stream
 ***/
void printSet(VarSet* set, FILE* stream) {
  assert(set != NULL);   // Using a dummy head node - so verify it is created.

  VarSet* curr;
  for (curr = set->next; curr != NULL; curr = curr->next) {
    fprintf(stream, "%s: %s\n", curr->name, curr->value);
  }
}

