${CC:=gcc} -fsanitize=address -std=gnu99 -g -O0 calc.c lexer.c parser.c cbigint.c -o ccalc -lm -Wall
