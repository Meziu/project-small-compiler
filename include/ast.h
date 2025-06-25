/*
 * Questo modulo definisce i tipi e le funzioni di base per
 * rappresentare un AST.
 */

#ifndef AST_H
#define AST_H

#include "symtab.h"
#include "type.h"

#define MAX_CHILDREN 3

typedef struct ASTNode ASTNode;
typedef ASTNode *AST;

/* Puntatore a funzione; usato per le funzioni di
 * analisi semantica e generazione del codice, che
 * sono specifiche per ciascun tipo di nodo dell'AST.
 */
typedef void (*ASTNodeFunction)(AST ast, SymbolTable *sym);

/* Struttura di un nodo dell'AST
 */
struct ASTNode {
    ASTNodeFunction check;    /* Puntatore alla funzione di analisi semantica */
    ASTNodeFunction gen;      /* Puntatore alla funzione di gen. del codice */
    AST child[MAX_CHILDREN];
    int line;                 /* Numero di linea del codice sorgente */
   
    ValueType value_type; 
    int value;
    float real_value;
    char *id;
    char *str;
};

/*------------------------------------------------------------------
 * Crea un nodo di un AST, senza figli.
 * Parametri di ingresso
 *    check    Puntatore alla funzione di analisi semantica da usare
 *             per il nodo.
 *    gen      Puntatore alla funzione di generazione del codice da
 *             usare per il nodo.
 *    line     Numero di linea del codice sorgente associata al nodo;
 *             è utilizzato per i messaggi di errore.
 * Valore di ritorno
 *    Il puntatore al nodo creato. Il nodo è allocato dinamicamente.
 -----------------------------------------------------------------*/
AST make_ast(ASTNodeFunction check, ASTNodeFunction gen, int line);

/*------------------------------------------------------------------
 * Crea un nodo di un AST, con un figlio.
 * Parametri di ingresso
 *    check    Puntatore alla funzione di analisi semantica da usare
 *             per il nodo.
 *    gen      Puntatore alla funzione di generazione del codice da
 *             usare per il nodo.
 *    line     Numero di linea del codice sorgente associata al nodo;
 *             è utilizzato per i messaggi di errore.
 *    child1   Puntatore al nodo che sarà usato come primo figlio del
 *             nodo creato dalla funzione.
 * Valore di ritorno
 *    Il puntatore al nodo creato. Il nodo è allocato dinamicamente.
 -----------------------------------------------------------------*/
AST make_ast1(ASTNodeFunction check, ASTNodeFunction gen, int line,
              AST child1);


/*------------------------------------------------------------------
 * Crea un nodo di un AST, con due figli.
 * Parametri di ingresso
 *    check    Puntatore alla funzione di analisi semantica da usare
 *             per il nodo.
 *    gen      Puntatore alla funzione di generazione del codice da
 *             usare per il nodo.
 *    line     Numero di linea del codice sorgente associata al nodo;
 *             è utilizzato per i messaggi di errore.
 *    child1   Puntatore al nodo che sarà usato come primo figlio del
 *             nodo creato dalla funzione.
 *    child2   Puntatore al nodo che sarà usato come secondo figlio del
 *             nodo creato dalla funzione.
 * Valore di ritorno
 *    Il puntatore al nodo creato. Il nodo è allocato dinamicamente.
 -----------------------------------------------------------------*/
AST make_ast2(ASTNodeFunction check, ASTNodeFunction gen, int line,
              AST child1, AST child2);

/*------------------------------------------------------------------
 * Crea un nodo di un AST, con tre figli.
 * Parametri di ingresso
 *    check    Puntatore alla funzione di analisi semantica da usare
 *             per il nodo.
 *    gen      Puntatore alla funzione di generazione del codice da
 *             usare per il nodo.
 *    line     Numero di linea del codice sorgente associata al nodo;
 *             è utilizzato per i messaggi di errore.
 *    child1   Puntatore al nodo che sarà usato come primo figlio del
 *             nodo creato dalla funzione.
 *    child2   Puntatore al nodo che sarà usato come secondo figlio del
 *             nodo creato dalla funzione.
 *    child3   Puntatore al nodo che sarà usato come terzo figlio del
 *             nodo creato dalla funzione.
 * Valore di ritorno
 *    Il puntatore al nodo creato. Il nodo è allocato dinamicamente.
 -----------------------------------------------------------------*/
AST make_ast3(ASTNodeFunction check, ASTNodeFunction gen, int line,
              AST child1, AST child2, AST child3);

/*------------------------------------------------------------------
 * Richiama la funzione di analisi semantica di un nodo AST.
 * La funzione richiamata è determinata dalle informazioni
 * contenute all'interno del nodo.
 * Parametri di ingresso/uscita.
 *   ast     Puntatore al nodo di cui si vuole effettuare 
 *           l'analisi semantica.
 *   sym     Puntatore alla tabella dei simboli, che sarà
 *           passata come parametro alla funzione di
 *           analisi semantica (insieme al puntatore al nodo).
 -----------------------------------------------------------------*/
void ast_check(AST ast, SymbolTable *sym);

/*------------------------------------------------------------------
 * Richiama la funzione di generazione del codice di un nodo AST.
 * La funzione richiamata è determinata dalle informazioni
 * contenute all'interno del nodo.
 * Parametri di ingresso/uscita.
 *   ast     Puntatore al nodo di cui si vuole effettuare 
 *           la generazione del codice.
 *   sym     Puntatore alla tabella dei simboli, che sarà
 *           passata come parametro alla funzione di
 *           generazione del codice (insieme al puntatore al nodo).
 -----------------------------------------------------------------*/
void ast_gen(AST ast, SymbolTable *sym);



#endif /* AST_H */

