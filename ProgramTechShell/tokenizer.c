/******
 * Christian Duncan
 * Tokenizer:
 *    This collection of functions takes as input a single line of text
 *    and tokenizes that text.  Each call to getNextToken() returns the
 *    next token (as a string) in the line.  If the line is complete a 
 *    NULL is returned.
 *
 * See Tokenizer.h for details...
 *******/
#include "tokenizer.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static char* tokLine = NULL;
static char* currTokPos;

void startToken(char* line) {
  if (line == NULL) {
    // Hey, no line even passed
    fprintf(stderr, "ERROR: Null line given.  Using empty line.\n");
    line = "";
  }

  // Make a copy of the line (so we have it safely)

  // Reallocated space
  if ((tokLine = realloc(tokLine, strlen(line)+1)) == NULL) {
    // Not enough memory???
    fprintf(stderr, "ERROR: Insufficient memory to tokenize!  Using empty space.\n");
    tokLine = NULL; currTokPos = NULL;
    return;
  }

  strcpy(tokLine, line);

  // Start the token pointing to the first position
  currTokPos = tokLine;
}

aToken getNextToken() {
  aToken res;
  if (currTokPos == NULL || *currTokPos == '\0') {
    // End of line reached.  (Nothing left to parse)
    res.type = EOL;
    res.start = NULL;
    return res;
  }

  // Find the first non-white space
  while (*currTokPos == ' ' ||
	 *currTokPos == '\t' ||
	 *currTokPos == '\n')
    currTokPos++;

  switch (*currTokPos) {
  case '\0':
    // We have reached the end of the line... 
    res.type = EOL;
    res.start = NULL;
    return res;

  case '#':
    // We have reached the beginning of a comment (ignore the remainder)
    //    (Basically same as end of line from caller's perspective)
    res.type = EOL;
    res.start = NULL;
    return res;

  case '\'':
    // We have a single quoted string
    res.start = ++currTokPos;  // Skipping the quotes
    res.type = SINGLE_QUOTE;   // Store type as SINGLE_QUOTE
    
    // Find end of token (using ' as delimiter)
    while (*currTokPos != '\'' &&
	   *currTokPos != '\0') currTokPos++;
    break;
    
  case '\"':
    // We have a double quoted string
    res.start = ++currTokPos;  // Skipping the quotes
    res.type = DOUBLE_QUOTE;   // Store type as DOUBLE_QUOTE

    // Find end of token (using ' as delimiter)
    while (*currTokPos != '\"' &&
	   *currTokPos != '\0') currTokPos++;
    break;

  case '|':
    // We have a pipe
    res.start = NULL;  // String is not needed
    res.type = PIPE;   // Store type as PIPE
    ++currTokPos;      // Skip the pipe
    break;

  case ';':
    // We have a semicolon
    res.start = NULL;  // String is not needed
    res.type = SEMICOLON;  // Store type as SEMICOLON
    ++currTokPos;      // Skip the semicolon
    break;

  default:
    // This is start of a basic string
    res.start = currTokPos;
    res.type = BASIC;

    // Find end of token (using regular delimiters)
    while (*currTokPos != ' ' &&
	   *currTokPos != '\t' &&
	   *currTokPos != '\n' &&
	   *currTokPos != '\0')
      currTokPos++;
  }
  
  if (*currTokPos != '\0') {
    // Haven't quite reached the end (mark it - and advance currTokPos)
    *(currTokPos++) = '\0';
  } else if (res.type == SINGLE_QUOTE || res.type == DOUBLE_QUOTE) {
    // Unterminatd string: End of line without matching quote found
    res.type = ERROR;
  }

  // Return the start of this token
  return res;
}
