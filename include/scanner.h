/*
 * Questo modulo implementa uno scanner generico, che può essere usato
 * per diversi linguaggi.
 * 
 * Lo scanner supporta i seguenti tipi di token:
 * - TOK_EOF - fine del file.
 * - TOK_NUM - costante numerica intera (senza segno); il valore
 *             numerico della costante è riportato in un attributo
 * - TOK_NUMF - costante numerica reale (senza segno); il valore
 *             numerico della costante è riportato in un attributo; 
 * - TOK_ID  - identificatore; il nome dell'identificatore (una stringa)
 *             è riportato come attributo (vedi nota sotto).
 * - TOK_STR - costante stringa, delimitata da doppi apici ("); la
 *             stringa non può contenere andata a capo. Lo scannere riconosce
 *             e gestisce le sequenze di escape \\ , \" , \n . La stringa
 *             è riportata come attributo (vedi nota sotto).
 * - parole chiave - è possibile personalizzare l'elenco delle parole chiave
 *             supportate (e i relativi codici) al momento della creazione
 *             dello scanner.
 * - operatori formati da 2 caratteri - è possibile personalizzare l'elenco
 *             degli operatori di 2 caratteri supportati (e i relativi codici)
 *             al momento della creazione dello scanner.
 * - operatori formati da 1 carattere - è possibile personalizzare l'elenco
 *             degli operatori/delimitatori di 1 carattere al momento della 
 *             creazione dello scanner; il token avrà come codice il codice 
 *             ASCII del carattere.
 * 
 * Gestione dei commenti
 * È possibile personalizzare la sequenza di inizio commento 
 * (formata da 1 o 2 caratteri) e la sequenza di fine commento
 * (formata da 1 o 2 caratteri) al momento della creazione dello scanner. I
 * commenti non sono riportati come token.
 * 
 * Gestione degli spazi
 * I caratteri di spazio, tab (\t), carriage return (\r) e line feed (\n) sono
 * considerati come separatori di token e non sono riportati come token;
 * i caratteri di spazio e tab possono però essere inseriti in una
 * costante stringa.
 * 
 * NOTA: i codici di TOK_ID, TOK_NUM, TOK_STR e TOK_EOF sono < 0; per
 * i token personalizzati dall'utilizzatore (es. parole chiave) è opportuno
 * usare codici >= 256 (in modo da evitare conflitti con gli operatori e
 * delimitatori di 1 carattere).
 * 
 * NOTA: per le costanti stringa (TOK_STR), l'attributo restituito è un
 * puntatore a una stringa allocata dinamicamente.
 * 
 * NOTA: per gli identificatori (TOK_ID), l'attributo restituito è un
 * puntatore a una stringa allocata in una tabella, e non deve essere
 * deallocato. Inoltre, è garantito che se lo stesso identificatore è
 * incontrato più volte nel testo del programma, il puntatore restituito
 * sarà lo stesso (quindi è possibile usare l'operatore == per confrontare
 * due identificatori, invece di strcmp).
 */

#ifndef SCANNER_H
#define SCANNER_H

/* Codici dei token predefiniti */
enum { TOK_EOF=  -1,
       TOK_NUM= -11,
       TOK_NUMF= -12,
       TOK_STR= -13,
       TOK_ID=  -14
};

/* Struttura usata per comunicare le definizioni dei token 
 * personalizzati (es. parole chiave e operatori di 2 caratteri)
 */
typedef struct  {
    char *text; /* Stringa che rappresenta il token */
    int   code;       /* Codice associato al token */
} TokenDefinition;

/*------------------------------------------------------------------
 * PROTOTIPI DELLE FUNZIONI PUBBLICHE DEL MODULO
 ------------------------------------------------------------------*/

/*--------------------------------------------------------------------
 * Inizializza lo scanner, completando le definizioni dei token
 * personalizzabili.
 * Parametri di ingresso
 *    keywords    Array di definizioni di token corrispondenti
 *                alle keyword del linguaggio; le keyword
 *                devono essere identificatori validi.
 *    num_keywords Numero di elementi di keywords[]
 *    op2chars    Array di definizioni di token per operatori/delimitatori
 *                di 2 caratteri; ogni operatore deve essere una
 *                stringa di 2 caratteri.
 *    num_op2chars  Numero di elementi di op2chars
 *    op1char     Array di operatori/delimitatori di 1 carattere;
 *                il codice dell'operatore coincide col codice ASCII
 *                del carattere. L'array è terminato da \0
 *    comment_begin  Delimitatore iniziale di commento. Deve essere
 *                una stringa di 1 o 2 caratteri.
 *    comment_end  Delimitatore finale di commento. Deve essere una
 *                stringa di 1 o 2 caratteri.
 --------------------------------------------------------------------*/
void sc_init(TokenDefinition keywords[], int num_keywords,
        TokenDefinition op2chars[], int num_op2chars,
        char op1char[],
        char *comment_begin,
        char *comment_end);


/*--------------------------------------------------------------------
 * Apre un file e legge il primo token del file, che diventa il
 * token corrente. I successivi token saranno letti dallo stesso file.
 * Parametri di ingresso
 *    fname    Nome del file
 * Nota: in caso di successo, imposta il nome del file per i
 * prossimi messaggi di errore con la funzione set_error_filename
 * (definita in error.h).
 --------------------------------------------------------------------*/
void sc_open(char *fname);
        
/*--------------------------------------------------------------------
 * Chiude il file correntemente aperto.
 --------------------------------------------------------------------*/
void sc_close(void);

/*--------------------------------------------------------------------
 * Legge il prossimo token, che diventa il token corrente.
 * Valore di ritorno
 *    Il codice del token appena letto
 --------------------------------------------------------------------*/
int sc_advance(void);

/*--------------------------------------------------------------------
 * Restituisce il codice del token corrente.
 --------------------------------------------------------------------*/
int sc_current(void);

/*--------------------------------------------------------------------
 * Restituisce il numero di linea del token corrente.
 --------------------------------------------------------------------*/
int sc_current_line(void);

/*--------------------------------------------------------------------
 * Se il token corrente è di tipo TOK_NUM, restituisce il valore
 * numerico del token. Altrimenti, restituisce 0.
 --------------------------------------------------------------------*/
int sc_current_value(void);

/*--------------------------------------------------------------------
 * Se il token corrente è di tipo TOK_NUMF, restituisce il valore
 * numerico del token. Altrimenti, restituisce 0.0
 --------------------------------------------------------------------*/
float sc_current_float_value(void);


 /*--------------------------------------------------------------------
  * Se il token corrente è di tipo TOK_STR, restituisce la stringa
  * di caratteri del token. Altrimenti, restituisce NULL.
  * NOTA: La stringa restituita è allocata dinamicamente.
  --------------------------------------------------------------------*/
char *sc_current_string(void);

 /*--------------------------------------------------------------------
  * Se il token corrente è di tipo TOK_ID, restituisce il nome 
  * dell'identificatore. Altrimenti, restituisce NULL.
  * NOTA: il puntatore restituito è un puntatore a una stringa allocata in 
  * una tabella, e non deve essere deallocato.
  * Inoltre, è garantito che se lo stesso identificatore è
  * incontrato più volte nel testo del programma, il puntatore restituito
  * sarà lo stesso (quindi è possibile usare l'operatore == per confrontare
  * due identificatori, invece di strcmp).
  --------------------------------------------------------------------*/
char *sc_current_id(void);

 /*--------------------------------------------------------------------
  * Memorizza una stringa nella tabella degli id.
  * NOTA: il puntatore restituito è un puntatore a una stringa allocata in 
  * una tabella, e non deve essere deallocato.
  * Inoltre, è garantito che se lo stesso identificatore è
  * incontrato più volte nel testo del programma, il puntatore restituito
  * sarà lo stesso (quindi è possibile usare l'operatore == per confrontare
  * due identificatori, invece di strcmp).
  --------------------------------------------------------------------*/
char *sc_make_id(char *name);

 /*--------------------------------------------------------------------
  * Se il token corrente ha il codice specificato dal parametro,
  * avanza al prossimo token; altrimenti segnala un messaggio di errore.
  * Parametri di ingresso
  *    code   Il codice del token che ci si aspetta sia il token corrente.
  * Valore di ritorno
  *    In caso di successo, il codice del nuovo token letto (che diventa
  *    il token corrente).
  --------------------------------------------------------------------*/
int sc_match(int code);

 /*--------------------------------------------------------------------
  * Se il token corrente ha uno dei codici specificati dai parametri,
  * avanza al prossimo token; altrimenti segnala un messaggio di errore.
  * Parametri di ingresso
  *    n   Numero di codici specificati dai parametri successivi
  *    code1,...,codeN   n valori interi che corrispondono ai codici
  *        che ci si aspettano per il token corrente.
  * Valore di ritorno
  *    In caso di successo, il codice del nuovo token letto (che diventa
  *    il token corrente).
  --------------------------------------------------------------------*/
int sc_match_n(int n, ...);


 /*--------------------------------------------------------------------
  * Restituisce una rappresentazione stampabile del codice 
  * associato a un token. Utile per la costruzione di messaggi di
  * errore.
  * NOTA: Il puntatore restituito punta a un'area di memoria allocata
  * staticamente (non deve essere deallocato).
  --------------------------------------------------------------------*/
char* sc_token_name(int code);



#endif /* SCANNER_H */

