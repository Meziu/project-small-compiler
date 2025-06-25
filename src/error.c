#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>

#include "error.h"

/*------------------------------------
 * Variabili static
 *----------------------------------*/
static const char *error_filename=NULL;

/*--------------------------------------------
 * Implementazione delle funzioni pubbliche
 *------------------------------------------*/

void error(int line, const char *fmt, ...) {
    assert(fmt!=NULL);

    va_list ap; /* Per accedere agli argomenti dopo fmt */
    va_start(ap, fmt); /* Inizializza ap */
    if (error_filename) {
        if (line>0) 
            fprintf(stderr, "ERRORE in %s, linea %d: ",
                    error_filename, line);
        else
            fprintf(stderr, "ERRORE in %s: ",
                    error_filename);
    } else {
        if (line>0)
            fprintf(stderr, "ERRORE alla linea %d: ",
                    line);
        else
            fprintf(stderr, "ERRORE: ");
    }

    /* Stampa gli argomenti con formato fmt sul file stderr */
    vfprintf(stderr, fmt, ap);

    fprintf(stderr, "\n");

    va_end(ap);

    exit(1);
}



void set_error_filename(const char *filename) {
    if (error_filename!=NULL) {
        free((void *)error_filename);
        error_filename=NULL;
    }

    if (filename!=NULL) {
        error_filename=strdup(filename);
        if (error_filename==NULL) {
            fprintf(stderr, 
                    "ERRORE: set_error_filename: Memoria insufficiente\n");
            exit(1);
        }
    }
}
