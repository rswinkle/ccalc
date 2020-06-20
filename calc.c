
#include "cbigint.h"

#include "parser.h"
#include "lexer.h"

#include <stdio.h>

#define CVECTOR_IMPLEMENTATION
#include "cvector.h"


int main(int argc, char** argv)
{

	/*
	token_value tok;

	cvector_token_value tokens;
	cvec_token_value(&tokens, 0, 100);

	cvector_str ids = { 0 };
	cvector_cbigint vals = { 0 };

	do {
		tok = read_token(stdin);

		cvec_push_token_value(&tokens, tok);

		//if (tok.type == ID)
	} while (tok.type != END);
	*/


	program_state prog;

	parse_program_file(&prog, stdin);


	for (int i=0; i<prog.stmt_list.size; ++i) {
		print_token(&prog.stmt_list.a[i]->tok, stdout, 1);
	}

	//cvec_free_str(&ids);
	//cvec_free_token_value(&tokens);


	cvec_free_str(&prog.string_db);
	cvec_free_expr_block(&prog.expressions);

	cvec_free_exprptr(&prog.stmt_list);

	cvec_free_str(&prog.vars);
	cvec_free_cbigint(&prog.vals);



	return 0;
}







