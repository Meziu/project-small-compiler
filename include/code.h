#ifndef CODE_H
#define CODE_H

/*---------------------------------------------------------------------
 * Questo modulo gestisce le operazioni per la compilazione e
 * l'esecuzione del codice nella macchina virtuale.
 --------------------------------------------------------------------*/

/* Codici operativi delle istruzioni */
enum {
    OP_HALT,
    OP_JUMP,
    OP_JUMPZ,
    OP_JUMPNZ,
    OP_CALL,
    OP_ENTER,
    OP_RET0,
    OP_RET1,
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_NEG,
    OP_ADDF,
    OP_SUBF,
    OP_MULF,
    OP_DIVF,
    OP_NEGF,
    OP_INT2REAL,
    OP_EQ,
    OP_LT,
    OP_EQF,
    OP_LTF,
    OP_PUSH8,
    OP_PUSH32,
    OP_DUP,
    OP_DROP,
    OP_LOAD,
    OP_STORE,
    OP_ADDR,
    OP_READ,
    OP_READF,
    OP_WRITE,
    OP_WRITEF,
    OP_WRITE_STR
};

/* Numero massimo di parametri formali che una funzione può avere */
#define MAX_FORMALS 255

/* Numero massimo di variabili locali che una funzione può avere */
#define MAX_LOCALS 32767

/*------------------------------------------------------------------
 * Restituisce l'indirizzo nella code memory della prossima
 * istruzione che verrà compilata.
 *
 * VALORE DI RITORNO
 *    indirizzo della prossima istruzione
 *----------------------------------------------------------------*/
int code_address(void);

/*-----------------------------------------------------------------
 * Inserisce un byte nella code memory, incrementando l'indirizzo
 * della prossima istruzione.
 *
 * PARAMETRI DI INGRESSO
 *    b   Il byte da inserire
 *---------------------------------------------------------------*/
void code_put(unsigned char b);

/*-----------------------------------------------------------------
 * Inserisce un byte nella code memory a un indirizzo specificato.
 *
 * PARAMETRI DI INGRESSO
 *    b    Il byte da inserire
 *    addr L'indirizzo in cui inserire il valore.
 *---------------------------------------------------------------*/
void code_put_at(unsigned char b, unsigned short addr);

/*-----------------------------------------------------------------
 * Inserisce un valore a 16 bit nella code memory,
 * incrementando di 2 l'indirizzo della prossima istruzione.
 *
 * PARAMETRI DI INGRESSO
 *    w   Il valore a 16 bit da inserire
 *---------------------------------------------------------------*/
void code_put16(short w);

/*-----------------------------------------------------------------
 * Inserisce un valore a 16 bit nella code memory,
 * a un indirizzo specificato
 * PARAMETRI DI INGRESSO
 *    w   Il valore a 16 bit da inserire
 *    addr L'indirizzo in cui inserire il valore.
 *---------------------------------------------------------------*/
void code_put16_at(short w, unsigned short addr);

/*-----------------------------------------------------------------
 * Inserisce un valore a 32 bit nella code memory,
 * incrementando di 4 l'indirizzo della prossima istruzione.
 *
 * PARAMETRI DI INGRESSO
 *    w   Il valore a 32 bit da inserire
 *---------------------------------------------------------------*/
void code_put32(int w);

/*-----------------------------------------------------------------
 * Inserisce un valore a 32 bit nella code memory,
 * a un indirizzo specificato
 * PARAMETRI DI INGRESSO
 *    w   Il valore a 32 bit da inserire
 *    addr L'indirizzo in cui inserire il valore.
 *---------------------------------------------------------------*/
void code_put32_at(int w, unsigned short addr);

/*-----------------------------------------------------------------
 * Inserisce un valore float a 32 bit nella code memory,
 * incrementando di 4 l'indirizzo della prossima istruzione.
 *
 * PARAMETRI DI INGRESSO
 *    w   Il valore a 32 bit da inserire
 *---------------------------------------------------------------*/
void code_putfloat(float w);

/*-----------------------------------------------------------------
 * Inserisce un valore float a 32 bit nella code memory,
 * a un indirizzo specificato
 * PARAMETRI DI INGRESSO
 *    w   Il valore a 32 bit da inserire
 *    addr L'indirizzo in cui inserire il valore.
 *---------------------------------------------------------------*/
void code_putfloat_at(float w, unsigned short addr);

/*----------------------------------------------------------------
 * Esegue il programma contenuto nella code memory.
 * L'esecuzione comincia all'indirizzo 0 della code memory;
 * il registro SP è inizializzato con l'indirizzo dell'ultima
 * cella della data memory, mentre FP è inizializzato a 0.
 *--------------------------------------------------------------*/
void code_run(void);

#endif
