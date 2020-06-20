
#include "lexer.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

CVEC_NEW_DEFS(token_value, RESIZE)

token_value read_token(FILE* file)
{
	static char token_buf[MAX_TOKEN_LEN];
	int c, i = 0;

	token_value tok = { 0 };

	do {
		c = getc(file);
		if (c == '\n') {
			tok.type = SEMICOLON;
			return tok;
		}
	} while (isspace(c));

	//printf("getting token starting with '%c'\n", c);
	switch (c) {
	case '=': tok.type = EQUAL;         break;
	case '!': tok.type = FACTORIAL;     break;
	case ';': tok.type = SEMICOLON;     break;

	case '+':
		c = getc(file);
		if (c == '=') {
			tok.type = ADDEQUAL;
		} else {
			ungetc(c, file);
			tok.type = ADD;
		}
		break;

	case '-':
		c = getc(file);
		if (c == '=') {
			tok.type = SUBEQUAL;
		} else {
			ungetc(c, file);
			tok.type = SUB;
		}
		break;

	case '*':
		c = getc(file);
		if (c == '=') {
			tok.type = MULTEQUAL;
		} else if (c == '*') {
			tok.type = EXPONENT;
		} else {
			ungetc(c, file);
			tok.type = MULT;
		}
		break;

	case '/':
		c = getc(file);
		if (c == '=') {
			tok.type = DIVEQUAL;
		} else {
			ungetc(c, file);
			tok.type = DIV;
		}
		break;

	case '%':
		c = getc(file);
		if (c == '=') {
			tok.type = MODEQUAL;
		} else {
			ungetc(c, file);
			tok.type = MOD;
		}
		break;

	default:

		//TODO should +532 and -12321 be handled
		//as literals or unary +/- and 2 positive literals?
		//and what about hex and oct literals?  crap
		if (isdigit(c)) {
			while (isdigit(c)) {
				token_buf[i++] = c;
				c = getc(file);

				if (i == MAX_TOKEN_LEN-1)
					goto token_length_error;
			}

			ungetc(c, file);
			token_buf[i] = '\0';

			tok.type = INT_LITERAL;
			tok.v.id = strdup(token_buf);

		} else if (isalpha(c)) {
			while (isalnum(c) || c == '_') {
				token_buf[i++] = c;
				c = getc(file);

				if (i == MAX_TOKEN_LEN-1)
					goto token_length_error;
			}

			ungetc(c, file);
			token_buf[i] = '\0';


			tok.type = ID;
			tok.v.id = strdup(token_buf);
			assert(tok.v.id);

		} else if (c == EOF) {
			tok.type = END;
		} else {
			lex_error("Unrecognized token \"%c\"", c);
		}
	}



	return tok;

token_length_error:
	lex_error("Token length is too long, max token length is %d,", MAX_TOKEN_LEN-1);
	
	// never gets here, gets rid of compiler warning
	return tok;
}

void lex_error(char *str, ...)
{
	va_list args;
	va_start(args, str);

	fprintf(stderr, "Lexer Error: ");
	vfprintf(stderr, str, args);

	va_end(args);

	exit(0);
}

void print_token(token_value* tok, FILE* file, int print_enum)
{
	if (print_enum) {
		switch (tok->type) {
			case END:              fprintf(file, "END");     break;
			case SEMICOLON:        fprintf(file, "SEMICOLON");     break;
			case ID:               fprintf(file, "ID = %s\n", tok->v.id);     break;
			case MOD:              fprintf(file, "MOD");     break;
			case LPAREN:           fprintf(file, "LPAREN");     break;
			case RPAREN:           fprintf(file, "RPAREN");     break;
			case MULT:             fprintf(file, "MULT");     break;
			case ADD:              fprintf(file, "ADD");     break;
			case SUB:              fprintf(file, "SUB");     break;
			case DIV:              fprintf(file, "DIV");     break;
			case EQUAL:            fprintf(file, "EQUAL");     break;

			case ADDEQUAL:         fprintf(file, "ADDEQUAL");     break;
			case SUBEQUAL:         fprintf(file, "SUBEQUAL");     break;
			case MULTEQUAL:        fprintf(file, "MULTEQUAL");     break;
			case DIVEQUAL:         fprintf(file, "DIVEQUAL");     break;
			case MODEQUAL:         fprintf(file, "MODEQUAL");     break;
			case INT_LITERAL:      fprintf(file, "INT_LITERAL = %s\n", tok->v.id);     break;
			case EXPONENT:         fprintf(file, "EXPONENT");     break;

			default:
				fprintf(file, "Error, unknown token: %d\n", tok->type);
		}
	} else {
		switch (tok->type) {
			case END:              fprintf(file, "END");     break;
			case SEMICOLON:        fprintf(file, ";");     break;
			case ID:               fprintf(file, "%s", tok->v.id);     break;
			case MOD:              fprintf(file, "%%");     break;
			case LPAREN:           fprintf(file, "(");     break;
			case RPAREN:           fprintf(file, ")");     break;
			case MULT:             fprintf(file, "*");     break;
			case ADD:              fprintf(file, "+");     break;
			case SUB:              fprintf(file, "-");     break;
			case DIV:              fprintf(file, "/");     break;
			case EQUAL:            fprintf(file, "=");     break;

			case ADDEQUAL:         fprintf(file, "+=");     break;
			case SUBEQUAL:         fprintf(file, "-=");     break;
			case MULTEQUAL:        fprintf(file, "*=");     break;
			case DIVEQUAL:         fprintf(file, "/=");     break;
			case MODEQUAL:         fprintf(file, "%%=");    break;

			case INT_LITERAL:      fprintf(file, "%s", tok->v.id);     break;
			case EXPONENT:         fprintf(file, "EXPONENT");     break;

			default:
				fprintf(file, "Error, unknown token: %d\n", tok->type);
		}
	}
}
