
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "scanner.h"
#include "token_codes.h"
#include "parser.h"
#include "error.h"
#include "code.h"

/* Definizione delle keywords del linguaggio */
static TokenDefinition keywords[]={
    {"int", TOK_INT},
    {"real", TOK_REAL},
    {"void", TOK_VOID},
    {"if", TOK_IF},
    {"else", TOK_ELSE},
    {"while", TOK_WHILE},
    {"return", TOK_RETURN},
    {"write", TOK_WRITE},
    {"read", TOK_READ},
    {"call", TOK_CALL},
    {"and", TOK_AND},
    {"or", TOK_OR},
    {"not", TOK_NOT},
};
#define NUM_KEYWORDS (sizeof(keywords)/sizeof(keywords[0]))

/* Definizioni degli operatori di 2 caratteri del linguaggio */
static TokenDefinition op2[]={
    {"==", TOK_EQ},
    {"!=", TOK_NE},
    {"<=", TOK_LE},
    {">=", TOK_GE},
};
#define NUM_OP2 (sizeof(op2)/sizeof(op2[0]))

/* Delimitatori di inizio e fine commento */
#define CBEGIN "//"
#define CEND "\n"
        

int main(int argc, char** argv) {
    sc_init(keywords, NUM_KEYWORDS,  /* Keywords */
            op2, NUM_OP2,     /* Operatori di due caratteri */
            "+-*/(),!{}=<>",       /* Operatori di un carattere */
            CBEGIN,
            CEND);
    if (argc!=2) {
        error(-1, "Il programma richiede un parametro sulla linea di comando "
                  "(il nome del file sorgente da leggere).");
    }
    sc_open(argv[1]);
    AST program=parse_program();
    sc_close();
    printf("Analisi sintattica completata correttamente\n");
    SymbolTable *sym=make_symtab(NULL);
    ast_check(program, sym);
    printf("Analisi semantica completata correttamente\n");
    ast_gen(program, sym);
    printf("Generazione del codice completata correttamente (%d bytes)\n",
            code_address());
    code_run();
    return (EXIT_SUCCESS);
}

