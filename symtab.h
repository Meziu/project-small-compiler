/*
 * Questo modulo gestisce le funzioni per la gestione delle
 * tabelle dei simboli, che associano identificatori a definizioni.
 * 
 * Per supportare gli scope annidati per la definizione
 * delle variabili, una tabella dei simboli può avere
 * una tabella "madre"; l'operazione di ricerca di una 
 * definizione viene propagata alla tabella
 * madre.
 * 
 * Per facilitare l'analisi semantica e la compilazione,
 * una tabella dei simboli mantiene anche una "funzione corrente" 
 * (la funzione in corso di compilazione).
 * 
 * NOTA: L'implementazione si basa sul fatto che tutti gli
 * identificatori siano inseriti in una tabella dallo scanner,
 * che assicura che lo stesso identificatore è associato a un
 * unico indirizzo anche se appare più volte nel sorgente del 
 * programma (vedi scanner.h); quindi è possibile confrontare
 * due identificatori usando == invece di strcmp.
 */


#ifndef SYMTAB_H
#define SYMTAB_H

#include "type.h"

struct SymbolTable;
typedef struct SymbolTable SymbolTable;

/* Tipi di entry che è possibile definire in una symbol table */
typedef enum {
    ET_FUNCTION,
    ET_FORMAL,    /* Parametro formale di una funzione */
    ET_VAR        /* Variabile  */
} EntryType;

/* Il tipo Entry rappresenta le informazioni contenute in una
 * symbol table su una dichiarazione.
 */
typedef struct Entry Entry;
struct Entry {
    char *id;  /* Identificatore della dichiarazione */
    int line;  /* Linea del sorgente in cui è presente la dichiarazione */
    EntryType entry_type;  /* Tipo di dichiarazione */
    Entry *next_formal; /* Per i parametri formali, link al prossimo
                         * parametro formale (o NULL se il parametro
                         * formale è l'ultimo. Questo campo è inizializzato
                         * automaticamente da symtab_define()
                         */
    
    /* Le seguenti informazioni sono riempite durante l'analisi semantica 
     * o la generazione del codice */
    ValueType value_type;  /* Tipo di dato dell'oggetto dichiarato */

    int address; /* Indirizzo dell'entità definita 
                  * Per le variabili e i parametri è un offset
                  * relativi al Frame Pointer; mentre per le funzioni
                  * è un indirizzo nella Code Memory.
                  */

    SymbolTable *sym; /* Per le funzioni, tabella dei simboli locale */

                 
    
    /* Questo campo è usato internamente per mantenere la struttura dati */
    Entry *link;
};


/*----------------------------------------------------------
 * Crea una nuova symbol table
 * Parametri di ingresso
 *   parent     La tabella madre, oppure NULL.
 * Valore di ritorno
 *   La tabella creata.
 ----------------------------------------------------------*/
SymbolTable *make_symtab(SymbolTable *parent);


/*----------------------------------------------------------
 * Inserisce una definizione in una symbol table.
 * Viene segnalato un errore se si tenta di fornire una
 * definizione per un identificatore che è già usato da
 * un'altra definizione nella stessa symbol table.
 * 
 * Parametri di ingresso/uscita
 *   sym        La symbol table
 * Parametri di ingresso
 *   id         L'identificatore della definizione. 
 *   type       Tipo di definizione (vedi le costanti ET_* sopra)
 *   line       Numero di linea nel sorgente della definizione; usato
 *              per i messaggi di errore.
 * Valore di ritorno
 *   L'Entry corrispondente alla definizione creata. L'Entry
 *   è allocato nella tabella dei simboli, e NON deve essere
 *   deallocato.
 *
 * NOTA BENE:
 *   id deve essere un'id memorizzato nella tabella degli id dello
 *   scanner; quindi deve essere ottenuto tramite le funzioni
 *   sc_current_id() oppure sc_make_id().
 *
 * NOTA:
 *   Le Entry corrispondenti ai parametri formali (ET_FORMAL)
 *   DEVONO essere create in ordine di definizione, per la
 *   corretta impostazione dei campi next_formal e per il 
 *   funzionamento di symtab_get_first_formal.
 ----------------------------------------------------------*/
Entry *symtab_define(SymbolTable *sym, char *id, EntryType type, int line);

/*----------------------------------------------------------
 * Cerca una definizione in una symbol table.
 * Se la definizione non è presente nella symbol table 
 * corrente, viene cercata nella tabella madre, nella
 * madre della madre e così via.
 * 
 * Parametri di ingresso
 *   sym        La symbol table
 *   id         L'identificatore della definizione da cercare. 
 * Valore di ritorno
 *   L'Entry corrispondente alla definizione trovata, oppure
 *   NULL se non è presente una definizione per quell'identificatore.
 *
 * NOTA BENE:
 *   id deve essere un'id memorizzato nella tabella degli id dello
 *   scanner; quindi deve essere ottenuto tramite le funzioni
 *   sc_current_id() oppure sc_make_id().
 ----------------------------------------------------------*/
Entry *symtab_lookup(SymbolTable *sym, char *id);


/*----------------------------------------------------------
 * Se la tabella dei simboli è associata a una funzione,
 * restituisce l'Entry di definizione della funzione.
 *
 * Parametri di ingresso
 *   sym        La symbol table
 * Valore di ritorno
 *   L'Entry corrispondente alla definizione della funzione
 *   a cui appartiene la symbol table, oppure NULL se la symbol table
 *   non è locale a una funzione.
 ----------------------------------------------------------*/
Entry *symtab_get_function_definition(SymbolTable *sym);

/*----------------------------------------------------------
 * Associa la symbol table a una funzione. Viene usata
 * quando la symbol table contiene le definizioni locali a
 * una funzione.
 *
 * Parametri di ingresso
 *   sym        La symbol table
 *   func       L'Entry di definizione della funzione.
 ----------------------------------------------------------*/
void symtab_set_function_definition(SymbolTable *sym, Entry *func);


/*-----------------------------------------------------------
 * Restituisce il contatore del numero di variabil locali 
 * associato a una symbol table.
 *
 * Parametri di ingresso
 *   sym        La symbol table
 * Valore di ritorno
 *   Il valore del contatore di variabili locali.
 ----------------------------------------------------------*/
int symtab_get_locals_counter(SymbolTable *sym);

/*-----------------------------------------------------------
 * Incrementa il contatore del numero di variabil locali 
 * associato a una symbol table.
 *
 * Parametri di ingresso
 *   sym        La symbol table
 *   inc        L'incremento da applicare al contatore.
 ----------------------------------------------------------*/
void symtab_increment_locals_counter(SymbolTable *sym, int inc);

/*-----------------------------------------------------------
 * Restituisce il contatore del numero di parametri formali
 * associato a una symbol table.
 *
 * Parametri di ingresso
 *   sym        La symbol table
 * Valore di ritorno
 *   Il valore del contatore di parametri formali.
 ----------------------------------------------------------*/
int symtab_get_formals_counter(SymbolTable *sym);

/*-----------------------------------------------------------
 * Incrementa il contatore del numero di parametri formali 
 * associato a una symbol table.
 *
 * Parametri di ingresso
 *   sym        La symbol table
 *   inc        L'incremento da applicare al contatore.
 ----------------------------------------------------------*/
void symtab_increment_formals_counter(SymbolTable *sym, int inc);


/*----------------------------------------------------------
 * Se la tabella è la tabella locale di una funzione,
 * restituisce l'entry corrispondente al primo parametro 
 * formale della funzione.
 *
 * Parametri di ingresso
 *   sym        La symbol table
 *
 * Valore di ritorno
 *   l'Entry corrispondente al primo parametro formale della
 *   funzione, oppure NULL.
 *
 * NOTA: si considera come primo parametro formale quello che
 * è stato creato per primo mediante symtab_define().
 *--------------------------------------------------------*/
Entry *symtab_get_first_formal(SymbolTable *sym);


/*-----------------------------------------------------------
 * Associa a una tabella una "work Entry", ovvero una Entry
 * su cui il codice che usa la tabella deve lavorare. Questo
 * meccanismo è usato durante l'analisi semantica per tenere 
 * traccia di qual è il parametro formale corrispondente al
 * prossimo parametro attuale.
 *
 * Parametri di ingresso/uscita
 *   sym        La symbol table
 *
 * Parametri di ingresso
 *   entry      La nuova "work Entry" della tabella
 *--------------------------------------------------------*/
void symtab_set_work_entry(SymbolTable *sym, Entry *entry);

/*-----------------------------------------------------------
 * Restituisce la "work Entry" della tabella, ovvero l'Entry
 * su cui il codice che usa la tabella deve lavorare. Questo
 * meccanismo è usato durante l'analisi semantica per tenere 
 * traccia di qual è il parametro formale corrispondente al
 * prossimo parametro attuale.
 *
 * Parametri di ingresso
 *   sym        La symbol table
 *
 * Valore di ritorno
 *   La work Entry corrente.
 *--------------------------------------------------------*/
Entry *symtab_get_work_entry(SymbolTable *sym);

#endif /* SYMTAB_H */

