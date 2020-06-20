#ifndef LEXER_H
#define LEXER_H

#include "cvector.h"

#include <stdio.h>
#include <stdarg.h>

#define MAX_TOKEN_LEN 1025
#define PARSING (FILE*)1

//tokens pg 20
typedef enum Token
{
	// END is end of input, EOF
	// SEMICOLON is ; or '\n', end of expression/statement
	END, SEMICOLON,

	ID,

	MOD, LPAREN, RPAREN, MULT, ADD, SUB, DIV,
	EQUAL, EXPONENT, FACTORIAL,

	/* compound assignment operators */
	ADDEQUAL, SUBEQUAL, MULTEQUAL, DIVEQUAL, MODEQUAL,

	/* literals aka constants */
	INT_LITERAL,

	/* for internal use/hacks, not really tokens */
	DECLARATION, EXP, EXPR_LIST, FUNC_CALL,
	POST_INCREMENT, POST_DECREMENT, PRE_INCREMENT, PRE_DECREMENT
} Token;

typedef struct token_value
{
	Token type;
	union {
		char* id;
//		char* str_literal; just use id
	} v;
} token_value;

#define RESIZE(x) ((x+1)*2)

CVEC_NEW_DECLS(token_value)


void lex_error(char *str, ...);


token_value read_token(FILE* file);
//token_lex read_token_from_str(char* input);

void print_token(token_value* tok, FILE* file, int print_enum);


#endif
