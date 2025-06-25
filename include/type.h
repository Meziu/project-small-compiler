/*
 * Questo modulo contiene le definizioni usate per rappresentare
 * i tipi nell'AST e nella tabella dei simboli.
 */


#ifndef TYPE_H
#define TYPE_H

/*
 * Un valore di tipo ValueType rappresenta il tipo di valore di un'espressione
 * aritmetica, di una variabile, di un parametro o del risultato di una 
 * funzione.
 */

typedef enum {
    TYPE_UNKNOWN = 0,  /* Indica che il tipo non è al momento noto */
    TYPE_INT = 1,      /* Il tipo è intero */
    TYPE_REAL = 2,     /* Il tipo è reale */
    TYPE_VOID = 3      /* Il tipo è void (solo per le funzioni) */
} ValueType;


#endif /* TYPE_H */

