#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "mpc/mpc.h"

#if _WIN32

static char buffer[2048];
char *readline(char *promt)
{
    fputs("> ", stdout);
    fgets(buffer, 2024, stdin);
    char *input = malloc(sizeof(buffer) + 1);

    strcpy(input, buffer);
    input[strlen(input) - 1] = '\n';
    return input;
}

void add_history(char *_){};
#else

#import <editline/readline.h>
#import <editline/history.h>

#endif

int main(int argc, char **argv)
{
    /* Create Some Parsers */
    mpc_parser_t *Number = mpc_new("number");
    mpc_parser_t *Operator = mpc_new("operator");
    mpc_parser_t *Expr = mpc_new("expr");
    mpc_parser_t *Lispy = mpc_new("lispy");

    /* Define them with the following Language */
    mpca_lang(MPCA_LANG_DEFAULT,
              "                                                     \
    number   : /-?[0-9]+/ ;                             \
    operator : '+' | '-' | '*' | '/' ;                  \
    expr     : <number> | '(' <operator> <expr>+ ')' ;  \
    lispy    : /^/ <operator> <expr>+ /$/ ;             \
  ",
              Number, Operator, Expr, Lispy);
    puts("Welcome to Lucisp");
    while (1)
    {
        char *input = readline(">");
        add_history(input);

        mpc_result_t mpcResult;
        if (mpc_parse("stdin", input, Lispy, &mpcResult))
        {
            mpc_ast_print(mpcResult.output);
            mpc_ast_delete(mpcResult.output);
        }
        else
        {
            mpc_err_print(mpcResult.error);
            mpc_err_delete(mpcResult.error);
        }

        printf(") %s", input);
        if (strncmp(input, "exit", 4) == 0)
        {
            break;
        }
        free(input);
    }
    puts("Bye");
    /* Undefine and Delete our Parsers */
    mpc_cleanup(4, Number, Operator, Expr, Lispy);
    return 0;
}