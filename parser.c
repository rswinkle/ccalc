
#include "parser.h"

CVEC_NEW_DEFS(exprptr, RESIZE)

CVEC_NEW_DEFS2(expr_block, RESIZE)

CVEC_NEW_DEFS2(cbigint, RESIZE)


// See comment in parser.h explaining the reason for expr_block
expression* make_expression(program_state* prog)
{
	expr_block* b = cvec_back_expr_block(&prog->expressions);
	if (b->n == b->used) {
		expr_block b_new;
		make_expression_block(32, &b_new);
		cvec_push_expr_block(&prog->expressions, &b_new);
		b = cvec_back_expr_block(&prog->expressions);
	}
	return &b->data[b->used++];
}

expression* copy_expr(program_state* prog, expression* e)
{
	expression* copy = make_expression(prog);
	memcpy(copy, e, sizeof(expression));
	return copy;
}


int make_expression_block(size_t n, expr_block* block)
{
	block->data = malloc(n * sizeof(expression)); assert(block->data);
	if (!block->data) {
		return 0;
	}
	block->n = n;
	block->used = 0;
	return 1;
}

void free_expr_block(void* block)
{
	expr_block* b = block;
	free(b->data);
}

token_value* peek_token(parsing_state* p, long offset)
{
	return &p->tokens.a[p->pos + offset];
}

token_value* get_token(parsing_state* p)
{
	token_value* tok = &p->tokens.a[p->pos++];
	//print_token(tok, stdout, 0); putchar('\n');

	//return &p->tokens.a[p->pos++].tok;
	return tok;
}

void parse_program(parsing_state* p, program_state* prog)
{
	expression* e;

	token_value* tok = peek_token(p, 0);
	while (tok->type != END) {
		e = make_expression(prog);
		assign_expr(p, prog, e);

		//puts("==");
		//print_token(&e->tok, stdout, 1);
		//puts("==");
		cvec_push_exprptr(&prog->stmt_list, e);

		do {
			tok = get_token(p);
		} while (tok->type == SEMICOLON);
	}
}

void assign_expr(parsing_state* p, program_state* prog, expression* e)
{
	token_value* tok;

	if (peek_token(p, 0)->type != ID) {
		add_expr(p, prog, e);

		return;
	}

	tok = peek_token(p, 0);

	if (!assignment_operator(peek_token(p, 1)->type)) {
		parse_error(&e->tok, "in assign_expr, assig_op expected\n");
		return;
	}

	get_token(p);  //match ID
	e->tok = *tok; //id from above

	tok = get_token(p); //get assignment op

	//assignment operators are right associative for obvious reasons
	//they're the only right associative binary operators in C
	//
	//well technically the comma is fully associative semantically
	//
	//strangely this looks like the same left associative code for other
	//operators.  The difference is the others aren't recursive
	//just loop always modifying the same top level expression
	//copying it into the left side (the recursion) as needed.
	//(apparently that method is called tail recursion ...)
	//
	//Here the top level left side never changes and only the right
	//side is expanded in the recursive call
	e->left = copy_expr(prog, e);
	e->tok.type = tok->type;

	e->right = make_expression(prog);

	assign_expr(p, prog, e->right);
}

int assignment_operator(Token tok)
{
	return (tok == EQUAL || tok == ADDEQUAL || tok == SUBEQUAL ||
	        tok == MULTEQUAL || tok == DIVEQUAL || tok == MODEQUAL);
}

/* add_expr -> mult_expr { '+'|'-' mult_expr } */
void add_expr(parsing_state* p, program_state* prog, expression* e)
{
	mult_expr(p, prog, e);

	token_value* tok = peek_token(p, 0);
	while (tok->type == ADD || tok->type == SUB) {
		get_token(p);

		e->left = copy_expr(prog, e);
		e->tok.type = tok->type;

		e->right = make_expression(prog);
		mult_expr(p, prog, e->right);

		tok = peek_token(p, 0);
	}
}

/* mult_expr ->  exp_expr { '*'|'/'|'%' exp_expr } */
void mult_expr(parsing_state* p, program_state* prog, expression* e)
{
	token_value* tok;

	exp_expr(p, prog, e);

	tok = peek_token(p, 0);
	while (tok->type == MULT || tok->type == DIV || tok->type == MOD) {
		get_token(p);

		e->left = copy_expr(prog, e);
		e->tok.type = tok->type;

		e->right = make_expression(prog);
		exp_expr(p, prog, e->right);

		tok = peek_token(p, 0);
	}
}

/* exp_expr ->  unary_expr { '**' unary_expr } */
void exp_expr(parsing_state* p, program_state* prog, expression* e)
{
	token_value* tok;

	unary_expr(p, prog, e);

	tok = peek_token(p, 0);
	if (tok->type == EXPONENT) {
		e->left = copy_expr(prog, e);
		e->tok.type = tok->type;

		e->right = make_expression(prog);

		exp_expr(p, prog, e->right);
	}
}

/* unary_expr -> unary+ | unary- | primary_expr */
void unary_expr(parsing_state* p, program_state* prog, expression* e)
{
	switch (peek_token(p, 0)->type) {
	case ADD:   //unary +, -
	case SUB:
		e->tok.type = get_token(p)->type; //match
		e->left = make_expression(prog);

		unary_expr(p, prog, e->left);
		break;

	default:
		primary_expr(p, prog, e);
	}
}

/* primary -> '(' expr ')' | ID | INT_LITERAL */
void primary_expr(parsing_state* p, program_state* prog, expression* e)
{
	token_value* tok = get_token(p);
	cbigint* check;

	switch (tok->type) {
	case LPAREN:
		e->tok.type = EXP;
		e->left = make_expression(prog);
		assert(e->left);
		assign_expr(p, prog, e->left);

		tok = get_token(p);
		if (tok->type != RPAREN) {
			parse_error(tok, "in primary_expr, RPAREN expected.\n");
		}
		break;
	case ID:
		check = look_up_value(prog, tok->v.id);
		if (!check) {
			parse_error(tok, "undeclared variable\n");
		}
		e->tok.type = ID;
		e->tok.v.id = tok->v.id;
		break;
	case INT_LITERAL:
		e->tok.type = INT_LITERAL;
		// TODO even though they're both strings, have a v.int_literal?
		// or even just do the conversion here or in lexing to cbigint?
		e->tok.v.id = tok->v.id;
		break;
	default:
		parse_error(tok, "in primary_expr, LPAREN or literal expected\n");
	}
}

void parse_program_file(program_state* prog, FILE* file)
{
	parsing_state p;
	cvec_token_value(&p.tokens, 0, 1000);

	cvec_str(&prog->string_db, 0, 100);
	int i;

	token_value tok = read_token(file);
	while (tok.type != END) {

		// could do this in lexing
		if (tok.type == ID) {
			for (i=0; i<prog->string_db.size; ++i) {
				if (!strcmp(tok.v.id, prog->string_db.a[i])) {
					free(tok.v.id);
					tok.v.id = prog->string_db.a[i];
					break;
				}
			}
			if (i == prog->string_db.size) {
				cvec_pushm_str(&prog->string_db, tok.v.id);
			}
		}

		// debugging
		print_token(&tok, stdout, 1);
		putchar('\n');

		cvec_push_token_value(&p.tokens, tok);
		tok = read_token(file);
	}
	cvec_push_token_value(&p.tokens, tok); //push END
	if (file != stdin)
		fclose(file);


	p.pos = 0;

	cvec_str(&prog->vars, 0, 20);
	cvec_cbigint(&prog->vals, 0, 20, cbi_free, NULL);
	cvec_exprptr(&prog->stmt_list, 0, 20);

	cvec_expr_block(&prog->expressions, 1, 1, free_expr_block, NULL);
	make_expression_block(50, cvec_back_expr_block(&prog->expressions));

	parse_program(&p, prog);

	cvec_free_token_value(&p.tokens);

	return;
}

cbigint* look_up_value(program_state* prog, const char* id)
{
	for (int i=0; i<prog->vars.size; ++i) {
		if (!strcmp(prog->vars.a[i], id)) {
			return &prog->vals.a[i];
		}
	}
	return NULL;
}

void parse_error(token_value* tok, char *str, ...)
{
	va_list args;
	va_start(args, str);
	fprintf(stderr, "Parse Error: ");
	vfprintf(stderr, str, args);
	if (tok) {
		if (str[strlen(str)-1] == '\n')
			fprintf(stderr, "Got ");
		else
			fprintf(stderr, ", got ");
		print_token(tok, stderr, 0);
	}
	va_end(args);

	exit(0);

}
