/******
 * Christian Duncan
 * Tokenizer:
 *    This collection of functions takes as input a single line of text
 *    and tokenizes that text.  Each call to getNextToken() returns the
 *    next token (as a string) in the line.  If the line is complete EOL
 *    is returned.
 *
 * Tokens are defined as follows:
 *    A collection of continuous non-whitespace characters.
 *
 *    Whitespace:
 *       Is defined as space (' '), tab ('\t') or newline ('\n')
 *       Everything else is considered non-whitespace.
 *
 *    String Tokens:
 *       If the token starts with a double (") or single (') quote then
 *       the token continues to the matching double or single quote or
 *       the end of the line.
 *       If the end of the line is reached (in a string) before the matching
 *       quote is found (unterminated string), an error should be returned.
 *       The Token returned should not include the quotes.
 *       The ending quote is considered a delimiter (an end to the token)
 *       Thus,
 *          Hi "there how" are you
 *             Has 3 tokens: Hi, there how, are, you
 *          Hi "there how"are you
 *             Also has 3 tokens: Hi, there how, are, you
 *          Hi there"how are"you
 *             Has 2 tokens (odd!!!)
 *                Hi, there"how, are"you
 *             Because quotes only mean something at START of token!!!
 *             We take this approach for SIMPLICITY really!
 *       
 * NOTE: 
 *    This is a very very simplistic tokenizer but will do for our basic needs
 *    for now.
 *
 *******/

#ifndef __TOKENIZER_H
#define __TOKENIZER_H
/***
 * A token: storing start of the token string
 *  and the type of the token.
 ***/
typedef struct {
  char *start;
  enum { BASIC, SINGLE_QUOTE, DOUBLE_QUOTE, PIPE, SEMICOLON, EOL, ERROR } type;
} aToken;

/***
 * startToken:
 *    Register the start of a new line to tokenize.
 *    The previous line (if still present) gets ignored.
 *    An error is printed if the line is NULL (but treated as an empty line)
 *
 *    line: A pointer to the start of the null-terminated string for this line.
 *          The string gets stored in a local copy so the string line can change
 *          without affecting the tokenizer.  Also, line is not altered in any way.
 ***/
void startToken(char *line);

/***
 * getNextToken:
 *    Return the next token in the current line as a struct (aToken).
 *    String tokens are handled as described above.
 *
 *    Returns aToken.type of: 
 *      EOL: If end-of-line reached
 *      ERROR: If some error occurred (namely, unterminated string)
 *      BASIC: If token is a regular token
 *      SINGLE_QUOTE: If token is 'single quoted string'
 *      DOUBLE_QUOTE: If token is "double quoted string"
 *      PIPE: If token is '|'
 *      SEMICOLON: If token is ';'
 *
 *    Returns aToken.start:
 *      If not EOL or ERROR, then start points to start of the string
 *      (and string is null-terminated)
 *
 **********************************************
 *    WARNING: This start string is ONLY temporary.  A subsequent call to 
 *      getNextToken/startToken will possibly erase it.  So caller MUST
 *      make a local copy if further use is needed!
 **********************************************
 ***/
aToken getNextToken();

#endif  /* __TOKENIZER_H */
