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
    LVAL_ERR,
    LVAL_SYM,
    LVAL_SEXR
};
enum
{
    LERR_DIV_ZERO,
    LERR_BAD_OP,
    LERR_BAD_NUM
};

typedef struct LVal
{
    int type;
    long value;
    char *error;
    char *sym;
    int count;
    struct LVal **cell;
} LVal;

LVal *lvalValue(long m)
{
    printf("lvalValue %lo\n", m);
    LVal *v = malloc(sizeof(LVal));
    v->type = LVAL_NUM;
    v->value = m;
    printf("LVal in lvalValue: %i %lo\n", v->type, v->value);
    return v;
};

LVal *lvalError(char *m)
{
    printf("lvalError %s\n", m);
    LVal *v = malloc(sizeof(LVal));
    v->type = LVAL_ERR;
    v->error = malloc(sizeof(m) + 1);
    strcpy(v->error, m);
    return v;
};

LVal *lvalSymbol(char *s)
{
    puts(s);
    LVal *v = malloc(sizeof(LVal));
    v->type = LVAL_SYM;
    v->sym = malloc(sizeof(s) + 1);
    strcpy(v->sym, s);
    return v;
};

LVal *lvalSexr(void)
{
    LVal *v = malloc(sizeof(LVal));
    v->type = LVAL_SEXR;
    v->count = 0;
    v->cell = NULL;
    return v;
};

LVal *lvalValueMiddleware(mpc_ast_t *t)
{
    puts("lvalValueMiddleware");
    errno = 0;
    long number = strtol(t->contents, NULL, 10);
    return errno != ERANGE ? lvalValue(number) : lvalError("Invalid number.");
}

LVal *lvalConcat(LVal *p, LVal *c)
{
    p->count++;
    p->cell = realloc(p->cell, sizeof(LVal *) * p->count);
    p->cell[p->count - 1] = c;
    return p;
}
LVal *lvalRead(mpc_ast_t *t)
{
    puts("lvalRead start");
    if (strstr(t->tag, "number"))
    {
        LVal *v = lvalValueMiddleware(t);
        printf("number struct: %i %lo\n", v->type, v->value);
        return v;
    };
    if (strstr(t->tag, "symbol"))
    {
        return lvalSymbol(t->contents);
    }
    puts("lvalRead => After 2 guards returns");
    LVal *p = NULL;
    if (strstr(t->tag, ">") == 0)
    {
        p = lvalSexr();
    }
    if (strstr(t->tag, "sexpr"))
    {
        p = lvalSexr();
    }

    for (size_t i = 0; i < t->children_num; i++)
    {
        printf("Loop for %llu\n", i);
        if (strcmp(t->children[i]->contents, "(") == 0)
        {
            puts("Continue (");
            continue;
        }
        if (strcmp(t->children[i]->contents, ")") == 0)
        {
            puts("Continue )");
            continue;
        }
        if (strcmp(t->children[i]->tag, "regex") == 0)
        {
            puts("Continue regex");
            continue;
        }

        p = lvalConcat(p, lvalRead(t->children[i]));

        printf("Loop struct: %i %s %lo\n", p->type, p->sym, p->value);
        printf("End of loop %llu\n", i);
    }
    printf("return struct: %i %s %lo\n", p->type, p->sym, p->value);
    return p;
}

void lvalDelete(LVal *v)
{
    switch (v->type)
    {
    case LVAL_NUM:
        break;
    case LVAL_ERR:
        free(v->error);
        break;
    case LVAL_SYM:
        free(v->sym);
        break;
    case LVAL_SEXR:
        for (size_t i = 0; i < v->count; i++)
        {
            lvalDelete(v->cell[i]);
        }
        free(v->cell);
        break;
    default:
        break;
    }
    free(v);
}
void lvalPrint(LVal *v);
void lvalExprPrint(LVal *v, char begin, char end)
{
    putchar(begin);
    for (size_t i = 0; i < v->count; i++)
    {
        lvalPrint(v->cell[i]);
        if (i != (v->count - 1))
        {
            putchar(' ');
        }
    }
    putchar(end);
}

void lvalPrint(LVal *v)
{
    switch (v->type)
    {
    case LVAL_NUM:
        printf("%li", v->value);
        break;

    case LVAL_ERR:
        printf("Error: %s", v->error);
        /* Check what type of error it is and print it */
        // if (v.error == LERR_DIV_ZERO)
        // {
        //     printf("Error: Division By Zero!");
        // }
        // if (v.error == LERR_BAD_OP)
        // {
        //     printf("Error: Invalid Operator!");
        // }
        // if (v.error == LERR_BAD_NUM)
        // {
        //     printf("Error: Invalid Number!");
        // }
        break;
    case LVAL_SYM:
        printf("%s", v->sym);
        break;
    case LVAL_SEXR:
        lvalExprPrint(v, '(', ')');
    }
};

void lvalPrintln(LVal *v)
{
    lvalPrint(v);
    putchar('\n');
}

// LVal eval_op(LVal x, char *op, LVal y)
// {
//     if (x.type == LVAL_ERR)
//     {
//         return x;
//     }
//     if (y.type == LVAL_ERR)
//     {
//         return y;
//     }

//     if (strcmp(op, "+") == 0)
//     {
//         return lvalValue(x.value + y.value);
//     }
//     if (strcmp(op, "-") == 0)
//     {
//         return lvalValue(x.value - y.value);
//     }
//     if (strcmp(op, "*") == 0)
//     {
//         return lvalValue(x.value * y.value);
//     }
//     if (strcmp(op, "/") == 0)
//     {

//         return y.value == 0 ? lvalError(LERR_DIV_ZERO) : lvalValue(x.value / y.value);
//     }
//     return lvalError(LERR_BAD_OP);
// }

// LVal eval(mpc_ast_t *t)
// {
//     if (strstr(t->tag, "number"))
//     {
//         errno = 0;
//         long contentNum = strtol(t->contents, NULL, 10);
//         return errno != ERANGE ? lvalValue(contentNum) : lvalError(LERR_BAD_NUM);
//     }

//     char *op = t->children[1]->contents;

//     // Childern(s) Evaluation Results
//     LVal CER = eval(t->children[2]);

//     int i = 3;
//     while (strstr(t->children[i]->tag, "expr"))
//     {
//         CER = eval_op(CER, op, eval(t->children[i]));
//         ++i;
//     }

//     return CER;
// }

int main(int argc, char **argv)
{
    mpc_parser_t *Number = mpc_new("number");
    mpc_parser_t *Symbol = mpc_new("symbol");
    mpc_parser_t *Sexpr = mpc_new("sexpr");
    mpc_parser_t *Expr = mpc_new("expr");
    mpc_parser_t *Lucisp = mpc_new("lucisp");

    mpca_lang(MPCA_LANG_DEFAULT,
              "                                          \
    number : /-?[0-9]+/ ;                    \
    symbol : '+' | '-' | '*' | '/' ;         \
    sexpr  : '(' <expr>* ')' ;               \
    expr   : <number> | <symbol> | <sexpr> ; \
    lucisp  : /^/ <expr>* /$/ ;               \
  ",
              Number, Symbol, Sexpr, Expr, Lucisp);
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
            // LVal evalResult = eval(mpcResult.output);
            // lvalPrintln(evalResult);
            puts("before lvalRead");
            LVal *SXTree = lvalRead(mpcResult.output);
            puts("after lvalRead");
            lvalPrintln(SXTree);
            puts("after lvalPrintLn");
            lvalDelete(SXTree);
            puts("after lvalDelete");
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
    mpc_cleanup(5, Number, Symbol, Sexpr, Expr, Lucisp);
    return 0;
}
