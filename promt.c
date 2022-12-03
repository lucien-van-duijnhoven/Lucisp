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

enum
{
    LVAL_NUM,
    LVAL_ERR
};
enum
{
    LERR_DIV_ZERO,
    LERR_BAD_OP,
    LERR_BAD_NUM
};

typedef struct
{
    int type;
    long value;
    int error;
} LVal;

LVal lvalError(int type)
{
    LVal v;
    v.type = LVAL_ERR;
    v.error = type;
    return v;
};

LVal lvalValue(long value)
{
    LVal v;
    v.type = LVAL_NUM;
    v.value = value;
    return v;
};

void lvalPrint(LVal v)
{
    switch (v.type)
    {
    case LVAL_NUM:
        printf(") %li", v.value);
        break;

    case LVAL_ERR:
        /* Check what type of error it is and print it */
        if (v.error == LERR_DIV_ZERO)
        {
            printf("Error: Division By Zero!");
        }
        if (v.error == LERR_BAD_OP)
        {
            printf("Error: Invalid Operator!");
        }
        if (v.error == LERR_BAD_NUM)
        {
            printf("Error: Invalid Number!");
        }
        break;
    }
};

void lvalPrintln(LVal v)
{
    lvalPrint(v);
    putchar('\n');
}

LVal eval_op(LVal x, char *op, LVal y)
{
    if (x.type == LVAL_ERR)
    {
        return x;
    }
    if (y.type == LVAL_ERR)
    {
        return y;
    }

    if (strcmp(op, "+") == 0)
    {
        return lvalValue(x.value + y.value);
    }
    if (strcmp(op, "-") == 0)
    {
        return lvalValue(x.value - y.value);
    }
    if (strcmp(op, "*") == 0)
    {
        return lvalValue(x.value * y.value);
    }
    if (strcmp(op, "/") == 0)
    {

        return y.value == 0 ? lvalError(LERR_DIV_ZERO) : lvalValue(x.value / y.value);
    }
    return lvalError(LERR_BAD_OP);
}

LVal eval(mpc_ast_t *t)
{
    if (strstr(t->tag, "number"))
    {
        errno = 0;
        long contentNum = strtol(t->contents, NULL, 10);
        return errno != ERANGE ? lvalValue(contentNum) : lvalError(LERR_BAD_NUM);
    }

    char *op = t->children[1]->contents;

    // Childern(s) Evaluation Results
    LVal CER = eval(t->children[2]);

    int i = 3;
    while (strstr(t->children[i]->tag, "expr"))
    {
        CER = eval_op(CER, op, eval(t->children[i]));
        ++i;
    }

    return CER;
}

int main(int argc, char **argv)
{
    /* Create Some Parsers */
    mpc_parser_t *Number = mpc_new("number");
    mpc_parser_t *Operator = mpc_new("operator");
    mpc_parser_t *Expr = mpc_new("expr");
    mpc_parser_t *Lucisp = mpc_new("lucisp");

    /* Define them with the following Language */
    mpca_lang(MPCA_LANG_DEFAULT,
              "                                                     \
    number   : /-?[0-9]+/ ;                             \
    operator : '+' | '-' | '*' | '/' ;                  \
    expr     : <number> | '(' <operator> <expr>+ ')' ;  \
    lucisp   : /^/ <operator> <expr>+ /$/ ;             \
  ",
              Number, Operator, Expr, Lucisp);
    puts("Welcome to Lucisp");
    while (1)
    {
        char *input = readline(">");
        add_history(input);

        if (strncmp(input, "exit", 4) == 0)
        {
            break;
        }

        mpc_result_t mpcResult;
        int mpcParseValid = mpc_parse("stdin", input, Lucisp, &mpcResult);
        if (mpcParseValid)
        {
            mpc_ast_print(mpcResult.output);
            LVal evalResult = eval(mpcResult.output);
            lvalPrintln(evalResult);
            mpc_ast_delete(mpcResult.output);
        }
        else
        {
            mpc_err_print(mpcResult.error);
            mpc_err_delete(mpcResult.error);
            printf(") Unable to evaluate the results.");
        }

        free(input);
    }
    puts("Bye");
    /* Undefine and Delete our Parsers */
    mpc_cleanup(4, Number, Operator, Expr, Lucisp);
    return 0;
}
