/* Questo modulo definisce le funzioni di parsing del linguaggio */

#ifndef PARSER_H
#define PARSER_H

#include "ast.h"

/*---------------------------------------------------------
 * Effettua l'analisi sintattica di un programma.
 * Valore di ritorno
 *    L'AST che rappresenta la struttura del programma.
 --------------------------------------------------------*/
AST parse_program(void);





#endif /* PARSER_H */


