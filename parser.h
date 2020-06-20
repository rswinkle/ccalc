
#include "lexer.h"
#include "cbigint.h"



typedef struct parsing_state
{
	cvector_token_value tokens;
	unsigned int pos;
} parsing_state;


typedef struct expression expression;

struct expression
{
	token_value tok;
	expression* left;
	expression* right;
};

// This is solely so I can generate a vector with valid function names
// without having to wrap it in a struct.  I actually hate hiding pointers
// via typedefing.
typedef expression* exprptr;

// expr_block exists because I wanted to deal with expression pointers
// rather than indices.  If I just had a vector of expressions, I'd be forced
// to use the latter because everytime the vector is realloc'd the pointers
// from before would be invalid.
typedef struct expr_block
{
	expression* data;
	size_t n;
	size_t used;
} expr_block;



CVEC_NEW_DECLS(exprptr)

CVEC_NEW_DECLS2(expr_block)

CVEC_NEW_DECLS2(cbigint)

typedef struct program_state
{
	cvector_str vars;
	cvector_cbigint vals;

	cvector_exprptr stmt_list;

	cvector_expr_block expressions;
	cvector_str string_db;
} program_state;


expression* make_expression(program_state* prog);
expression* copy_expr(program_state* prog, expression* e);
int make_expression_block(size_t n, expr_block* block);
void free_expr_block(void* block);
token_value* peek_token(parsing_state* p, long offset);
token_value* get_token(parsing_state* p);

void parse_program_file(program_state* prog, FILE* file);



int assignment_operator(Token tok);
void assign_expr(parsing_state* p, program_state* prog, expression* e);
void add_expr(parsing_state* p, program_state* prog, expression* e);
void mult_expr(parsing_state* p, program_state* prog, expression* e);
void exp_expr(parsing_state* p, program_state* prog, expression* e);
void unary_expr(parsing_state* p, program_state* prog, expression* e);
void primary_expr(parsing_state* p, program_state* prog, expression* e);

cbigint* look_up_value(program_state* prog, const char* id);

void parse_error(token_value* tok, char *str, ...);
