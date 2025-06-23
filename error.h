#ifndef ERROR_H
#define ERROR_H

/*---------------------------------------------------------------
 * Questo modulo consente di stampare messaggi di errore;
 * dopo il messaggio, il programma viene terminato.
 * Il messaggio di errore viene costruito con una stringa di formato
 * e un insieme di parametri, in maniera analoga al funzionamento
 * della funzione printf.
 * Inoltre, il messaggio può contenere un'indicazione del 
 * numero di linea e del nome del file sorgente.
 *----------------------------------------------------------------*/


/*------------------------------------------------------------------
 * Stampa un messaggio di errore e termina il programma.
 * Parametri di ingresso:
 *    line    Numero di linea del sorgente a cui è riferito l'errore.
 *            Se <= 0, non viene inserito nel messaggio un numero di linea.
 *    fmt     Stringa di formato, usando lo stesso formato di printf.
 *            Nella stringa di formato non è necessario inserire un \n finale,
 *            che viene aggiunto automaticamente.
 *    ...     I parametri successivi a fmt dipendono dalla stringa di formato,
 *            in maniera analoga al funzionamento di printf.
 *----------------------------------------------------------------*/
void error(int line, const char *fmt, ...);


/*-------------------------------------------------------------------
 * Imposta il nome del file sorgente da visualizzare nei messaggi di
 * errore.
 * Parametri di ingresso:
 *    filename   Nome del file sorgente. Se NULL, nei messaggi di
 *    errore non viene inserito un nome di file sorgente.
 * NOTA: La funzione alloca una copia del nome del file; se l'allocazione
 * dell'area di memoria fallisce, la funzione termina il programma con
 * un messaggio di errore.
 *-------------------------------------------------------------------*/
void set_error_filename(const char *filename);


#endif
