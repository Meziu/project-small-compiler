#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <assert.h>

#include "ast_functions.h"
#include "scanner.h"
#include "error.h"
#include "code.h"
#include "symtab.h"

/*--------------------------------------------------------
 * PROTOTIPI DELLE FUNZIONI STATIC DEL MODULO
 -------------------------------------------------------*/
static AST ensure_expr_type(AST expr, ValueType type);
static void check_operands(AST expr, SymbolTable *sym, bool allow_real);
static void gen_address(SymbolTable *sym, char *id);

/*--------------------------------------------------------
 * IMPLEMENTAZIONE DELLE FUNZIONI PUBBLICHE DEL MODULO
 -------------------------------------------------------*/

/* Un nodo 'empty' non ha figli e non fa niente
 * né in fase di analisi semantica né in fase
 * di generazione del codice.
 */
void empty_check(AST ast, SymbolTable *sym) {
    /* La funzione è deliberatamente vuota */
}

void empty_gen(AST ast, SymbolTable *sym) {
    /* La funzione è deliberatamente vuota */

}

/* Un nodo 'seq' rappresenta una sequenza di AST.
 */
void seq_check(AST ast, SymbolTable *sym) {
    int i;
    for(i=0; i<MAX_CHILDREN; i++) {
        if (ast->child[i]!=NULL)
            ast_check(ast->child[i], sym);
    }
}

void seq_gen(AST ast, SymbolTable *sym) {
    int i;
    for(i=0; i<MAX_CHILDREN; i++) {
        if (ast->child[i]!=NULL)
            ast_gen(ast->child[i], sym);
    }
}

/* Un nodo 'program' rappresenta la radice dell'intero programma.
 */
void program_check(AST ast, SymbolTable *sym) {
    /* Prima controlla tutti i figli */
    seq_check(ast, sym);

    /* Poi controlla che esista la funzione main,
     * senza parametri formali e di tipo void.
     */
    Entry *e=symtab_lookup(sym, sc_make_id("main"));
    if (e==NULL || e->entry_type!=ET_FUNCTION)
        error(ast->line, "Non è definita la funzione 'main'");
    SymbolTable *main_sym=e->sym;
    assert(main_sym!=NULL);
    if (symtab_get_first_formal(main_sym)!=NULL)
        error(e->line, "La funzione main non deve avere parametri");
    if (e->value_type!=TYPE_VOID)
        error(e->line, "La funzione main deve essere di tipo void");
}

void program_gen(AST ast, SymbolTable *sym) {
	/* Genera il codice per le costanti */
	if (ast->child[0] != NULL) {
    	ast_gen(ast->child[0], sym);
	}

    /* Compila una call per il main */
    code_put(OP_CALL);
    int call_addr=code_address();
    code_put16(0); /* Qui verrà messo l'indirizzo del main */
    code_put(OP_HALT);

    /* Genera il codice per le funzioni */
    ast_gen(ast->child[2], sym);

    Entry *e=symtab_lookup(sym, sc_make_id("main"));
    assert(e!=NULL);

    /* Ora inserisce l'indirizzo del main nella call */
    code_put16_at(e->address, call_addr);
}

/* Il prototipo di una funzione è una dichiarazione della sua interfaccia, senza codice */

void prototype_check(AST ast, SymbolTable *sym) {
	Entry *e = symtab_lookup(sym, ast->id);

	if (e == NULL) {
		// Funzione né dichiarata né definita
		e=symtab_define(sym, ast->id, ET_FUNCTION, ast->line);
    	e->value_type=ast->value_type;
    	SymbolTable *local_sym=make_symtab(sym);
    	e->sym=local_sym;
     	e->func_defined=false;
      	e->func_generated=false;
       	e->incomplete_addresses = make_address_list();
    	symtab_set_function_definition(local_sym, e);
    	ast_check(ast->child[0], local_sym); /* Parametri formali */
     	// Non è presente il corpo della funzione
	} else {
		// Già dichiarata o definita
        if (e->entry_type != ET_FUNCTION)
            error(ast->line, "'%s' è un identificatore già definito", ast->id);
        else
        	error(ast->line, "La funzione %s è già stata dichiarata", ast->id);
	}
}

void prototype_gen(AST ast, SymbolTable *sym) {
	// Vuoto
}

/* Un nodo 'func_def' rappresenta la definizione di una funzione */

void func_def_check(AST ast, SymbolTable *sym) {
	Entry *e = symtab_lookup(sym, ast->id);

	if (e == NULL) {
		e=symtab_define(sym, ast->id, ET_FUNCTION, ast->line);
    	e->value_type=ast->value_type;
     	SymbolTable *local_sym=make_symtab(sym);
     	e->sym=local_sym;
       	e->func_generated=false;
        e->incomplete_addresses = make_address_list();
      	symtab_set_function_definition(local_sym, e);
       	ast_check(ast->child[0], e->sym); /* Parametri formali */
	} else {
		// Esiste già una dichiarazione (o un'altro identificatore uguale)
		if (e->entry_type != ET_FUNCTION)
            error(ast->line, "'%s' è un identificatore già utilizzato", ast->id);

		// Funzione con due definizioni (due corpi di codice)
		if (e->func_defined) {
			error(ast->line, "Funzione '%s' definita due volte.", ast->id);
		}

		// Controlli del matching del prototipo
		if (e->value_type != ast->value_type) {
			error(ast->line, "Tipo della funzione '%s' diverso dal prototipo.", ast->id);
		}

		// Controllo di validità dei parametri.

		SymbolTable *local_sym=make_symtab(sym);
		ast_check(ast->child[0], local_sym);  // Parametri formali
		Entry *f=symtab_define(local_sym, ast->id, ET_FUNCTION, ast->line);
    	f->value_type=ast->value_type;
     	f->sym=local_sym;
		symtab_set_function_definition(local_sym, f);

		if (symtab_get_formals_counter(e->sym) != symtab_get_formals_counter(f->sym)) {
			error(ast->line, "Definizione della funzione '%s' ha un numero diverso di parametri formali rispetto al suo prototipo.", ast->id);
		}

		Entry* prototype_formal = symtab_get_first_formal(e->sym);
		Entry* definition_formal = symtab_get_first_formal(f->sym);
		int i = 0;
		while (prototype_formal != NULL) {
			i++;

			if (definition_formal->id != prototype_formal->id) {
				error(ast->line, "Parametro formale in posizione %d ('%s') ha un nome diverso dal suo corrispondente '%s' nel prototipo.",
					i,
					definition_formal->id,
					prototype_formal->id);
			}

			if (definition_formal->value_type != prototype_formal->value_type) {
				error(ast->line, "Parametro formale '%s' è di tipo diverso dal suo corrispondente nel prototipo.",
					definition_formal->id,
					prototype_formal->id);
			}

			Entry *formal = symtab_lookup(e->sym, prototype_formal->id);
			assert(formal != NULL);
			formal->id = definition_formal->id;
			//prototype_formal->id = definition_formal->id;

			prototype_formal = prototype_formal->next_formal;
			definition_formal = definition_formal->next_formal;
		}
	}

	e->func_defined=true;
    ast_check(ast->child[1], e->sym); /* Corpo della funzione */
}

void func_def_gen(AST ast, SymbolTable *sym) {
    Entry *e=symtab_lookup(sym, ast->id);
    assert(e!=NULL);
    e->address=code_address();
    SymbolTable *local_sym=e->sym;

    /* Genera il codice dei figli */
    seq_gen(ast, local_sym);

    // Risoluzione delle chiamate senza indirizzo
    e->func_generated=true;
    resolve_address_list(e->incomplete_addresses, e->address);
    e->incomplete_addresses = destroy_address_list(e->incomplete_addresses);
}

/* Un nodo 'type' rappresenta un tipo */
void type_check(AST ast, SymbolTable *sym) {
    /* La funzione è deliberatamente vuota */
}

void type_gen(AST ast, SymbolTable *sym) {
    /* La funzione è deliberatamente vuota */
}

/* Un nodo 'formals' rappresenta l'elenco dei parametri formali */
void formals_check(AST ast, SymbolTable *sym) {
    seq_check(ast, sym);

    /* Aggiusta gli indirizzi dei parametri formali,
     * calcolando l'offset rispetto al registro FP
     */
    int n=symtab_get_formals_counter(sym);
    Entry *e=symtab_get_first_formal(sym);
    while (e!=NULL) {
        e->address=n+1 - e->address;
        e=e->next_formal;
    }
}

void formals_gen(AST ast, SymbolTable *sym) {
    /* La funzione è deliberatamente vuota */
}

/* Un nodo 'formal' rappresenta un singolo parametro formale */
void formal_check(AST ast, SymbolTable *sym) {
    Entry *e=symtab_define(sym, ast->id, ET_FORMAL, ast->line);
    e->value_type=ast->value_type;
    if (e->value_type==TYPE_VOID)
        error(ast->line, "Il parametro '%s' non può essere void", ast->id);

    /* Pre-allocazione del parametro: segna nel campo address
     * l'offset rispetto al primo dei parametri formali.
     * L'indirizzo relativo a FP è calcolato in formals_check()
     */
    e->address=symtab_get_formals_counter(sym);
    symtab_increment_formals_counter(sym, 1);
}

void formal_gen(AST ast, SymbolTable *sym) {
    /* La funzione è deliberatamente vuota */
}


/* Un nodo 'body' rappresenta il corpo della funzione */
void body_check(AST ast, SymbolTable *sym) {
    seq_check(ast, sym);
}

void body_gen(AST ast, SymbolTable *sym) {
    int num_formals=symtab_get_formals_counter(sym);
    assert(num_formals<=MAX_FORMALS);
    int num_locals=symtab_get_locals_counter(sym);
    assert(num_locals<=MAX_LOCALS);

    /* Codice di inizio del body */
    code_put(OP_ENTER);
    code_put16(num_locals);

    /* Genera il codice dei figli */
    seq_gen(ast, sym);

    /* Alla fine aggiunge un return di default */
    Entry *f=symtab_get_function_definition(sym);
    assert(f!=NULL);
    if (f->value_type==TYPE_VOID) {
        code_put(OP_RET0);
        code_put(num_formals);
    } else {
        code_put(OP_PUSH8);
        code_put(0);
        code_put(OP_RET1);
        code_put(num_formals);
    }
}

/* Un nodo 'def' rappresenta una definizione di variabile locale */
void const_check(AST ast, SymbolTable *sym) {
    if (ast->child[0]!=NULL) {
        /* Controlla l'espressione di inizializzazione */
        ast_check(ast->child[0], sym);
    }
    Entry *e=symtab_define(sym, ast->id, ET_CONST, ast->line);
    e->value_type=ast->value_type;
    if (e->value_type==TYPE_VOID)
        error(ast->line, "La costante '%s' non può essere void", ast->id);

    /* Allocazione della costante */
    e->address = symtab_get_const_counter(sym); // dall'indirizzo 0
    symtab_increment_const_counter(sym, 1);

    /* Controlla se l'inizializzazione è coerente con il tipo
     * della costante
     */
    if (ast->child[0]!=NULL) {
        ast->child[0]=ensure_expr_type(ast->child[0], e->value_type);
    }
}


void const_gen(AST ast, SymbolTable *sym) {
    if (ast->child[0]!=NULL) {
        ast_gen(ast->child[0], sym);

        // L'indirizzo della costante è globale, per cui non usiamo l'istruzione ADDR
        code_put(OP_PUSH32);
        Entry *e=symtab_lookup(sym, ast->id);
        code_put32(e->address);

        code_put(OP_STORE);
    }
}

/* Un nodo 'def' rappresenta una definizione di variabile locale */
void def_check(AST ast, SymbolTable *sym) {
    if (ast->child[0]!=NULL) {
        /* Controlla l'espressione di inizializzazione */
        ast_check(ast->child[0], sym);
    }
    Entry *e=symtab_define(sym, ast->id, ET_VAR, ast->line);
    e->value_type=ast->value_type;
    if (e->value_type==TYPE_VOID)
        error(ast->line, "La variabile '%s' non può essere void", ast->id);

    /* Allocazione della variabile: segna nel campo address
     * l'indirizzo relativo a FP.
     */
    e->address= -1 - symtab_get_locals_counter(sym);
    symtab_increment_locals_counter(sym, 1);

    /* Controlla se l'inizializzazione è coerente con il tipo
     * della variabile
     */
    if (ast->child[0]!=NULL) {
        ast->child[0]=ensure_expr_type(ast->child[0], e->value_type);
    }
}


void def_gen(AST ast, SymbolTable *sym) {
    if (ast->child[0]!=NULL) {
        ast_gen(ast->child[0], sym);
        gen_address(sym, ast->id);
        code_put(OP_STORE);
    }
}

/* Un nodo 'assignment' rappresenta un'istruzione di assegnazione */
void assignment_check(AST ast, SymbolTable *sym) {
    Entry *e=symtab_lookup(sym, ast->id);
    if (e==NULL)
        error(ast->line, "Variabile non definita: '%s'", ast->id);
    if (e->entry_type!=ET_VAR)
        error(ast->line, "Assegnazione a '%s' che non è una variabile",
                ast->id);
    ast_check(ast->child[0], sym);
    ast->child[0]=ensure_expr_type(ast->child[0], e->value_type);
}

void assignment_gen(AST ast, SymbolTable *sym) {
    ast_gen(ast->child[0], sym);
    gen_address(sym, ast->id);
    code_put(OP_STORE);
}

/* Un nodo 'if' rappresenta un'istruzione if-else */
void if_check(AST ast, SymbolTable *sym) {
    seq_check(ast, sym);
    /* Controlla che la conzione sia di tipo int */
    ast->child[0]=ensure_expr_type(ast->child[0], TYPE_INT);
}

void if_gen(AST ast, SymbolTable *sym) {
    /* Genera il codice della condizione */
    ast_gen(ast->child[0], sym);

    /* Genera il salto per condizione falsa */
    code_put(OP_JUMPZ);
    int jump_end_addr=code_address();
    code_put16(0);

    /* Genera il codice del ramo "then" */
    ast_gen(ast->child[1], sym);

    /* Se c'è un ramo "else", genera il codice corrispondente */
    if (ast->child[2]!=NULL) {
        /* Genera un altro salto per andare alla fine dell'if */
        code_put(OP_JUMP);
        int temp_addr=code_address();
        code_put16(0);
        /* Sistema il primo salto per arrivare qui */
        code_put16_at(code_address(), jump_end_addr);
        /* Sistema le cose in modo che il secondo
         * salto vada alla fine dell'if */
        jump_end_addr=temp_addr;
        /* Genera il ramo "else" */
        ast_gen(ast->child[2], sym);
    }

    /* Sistema il salto che era a end_addr per arrivare a questo punto */
    code_put16_at(code_address(), jump_end_addr);
}


/* Un nodo 'while' rappresenta un'istruzione while */
void while_check(AST ast, SymbolTable *sym) {
    seq_check(ast, sym);
    /* Controlla che la conzione sia di tipo int */
    ast->child[0]=ensure_expr_type(ast->child[0], TYPE_INT);
}

void while_gen(AST ast, SymbolTable *sym) {
    /* Il codice generato compila prima il corpo, e poi
     * la condizione del while, in modo da dover fare un solo
     * salto a ogni iterazione. Questo richiede di aggiungere
     * un salto all'inizio verso il punto in cui sarà compilata
     * la condizione.
     */

    // Assegnazione di inizializzazione
    ast_gen(ast->child[0], sym);
    gen_address(sym, ast->id);
    code_put(OP_STORE);

    /* Genera un salto verso il punto in cui sarà generata la condizione */
    code_put(OP_JUMP);
    int jump_cond_addr=code_address();
    code_put16(0);

    /* Salva l'indirizzo di inizio del corpo */
    int body_start=code_address();
    /* Genera il corpo del ciclo */
    ast_gen(ast->child[1], sym);

    /* Sistema il salto alla condizione per arrivare qui */
    code_put16_at(code_address(), jump_cond_addr);

    /* Genera la condizione */
    ast_gen(ast->child[0], sym);

    /* Genera un salto all'inizio del corpo se la cond. è vera */
    code_put(OP_JUMPNZ);
    code_put16(body_start);
}

void do_while_check(AST ast, SymbolTable *sym) {
    seq_check(ast, sym);
    ast->child[0]=ensure_expr_type(ast->child[0], TYPE_INT);
}

void do_while_gen(AST ast, SymbolTable *sym) {
    /* Il codice generato sarà composto dal blocco,
     * seguito dal salto condizionato.
     */

    int body_start=code_address();
    ast_gen(ast->child[1], sym);

    ast_gen(ast->child[0], sym);

    code_put(OP_JUMPNZ);
    code_put16(body_start);
}

void for_check(AST ast, SymbolTable *sym) {
    seq_check(ast, sym);
    Entry* iter_var = symtab_lookup(sym, ast->id);
    if (iter_var->value_type != TYPE_INT) {
    	error(ast->line, "Variabile iterativa %s non è di tipo intero.", ast->id);
    }
    ast->child[0]=ensure_expr_type(ast->child[0], TYPE_INT); // init
    ast->child[1]=ensure_expr_type(ast->child[1], TYPE_INT); // cond
}

void for_gen(AST ast, SymbolTable *sym) {
	/* Simile al while, genera dapprima l'espressione di inizializzazione,
	 * poi il blocco (preceduto da un salto alla condizione), poi l'espressione di incremento
	 * e infine la condizione.
	 */

	// Generiamo il codice per l'espressione "init" e ne salviamo il risultato nella variabile iterativa.
	ast_gen(ast->child[0], sym); // init
	gen_address(sym, ast->id);
	code_put(OP_STORE);

    code_put(OP_JUMP);
    int jump_cond_addr=code_address();
    code_put16(0);

    /* Salva l'indirizzo di inizio del corpo */
    int body_start=code_address();
    /* Genera il corpo del ciclo */
    ast_gen(ast->child[2], sym);

    // Incrementa il valore della variabile iterativa
    gen_address(sym, ast->id);
    code_put(OP_LOAD);

    code_put(OP_PUSH32);
    code_put32(1); // incremento di 1

    code_put(OP_ADD); // Somma tra il valore precedente e la costante 1.

    gen_address(sym, ast->id);
    code_put(OP_STORE); // Salvataggio del nuovo valore incrementato.

    /* Sistema il salto alla condizione per arrivare qui */
    code_put16_at(code_address(), jump_cond_addr);

    // Generazione del limite superiore.
    ast_gen(ast->child[1], sym);
    // Ricarica la variabile per il controllo.
    gen_address(sym, ast->id);
    code_put(OP_LOAD);

    code_put(OP_LT); // Implementiamo il controllo <= come !(b < a), per cui l'ordine è invertito.

    code_put(OP_PUSH8);
    code_put(0);
    code_put(OP_EQ);

    /* Genera un salto all'inizio del corpo se la cond. è vera */
    code_put(OP_JUMPNZ);
    code_put16(body_start);
}

/* Un nodo 'write' rappresenta la write di un'espressione */
void write_check(AST ast, SymbolTable *sym) {
    ast_check(ast->child[0], sym);
    /* Controlla che l'espressione da scrivere sia int o real */
    ValueType t=ast->child[0]->value_type;
    if (t!=TYPE_INT && t!=TYPE_REAL)
        error(ast->line, "Attesa un'espressione int o real");
}

void write_gen(AST ast, SymbolTable *sym) {
    AST expr=ast->child[0];
    /* Genera l'espressione */
    ast_gen(expr, sym);
    /* Genera l'operazione write appropriata */
    if (expr->value_type==TYPE_INT)
        code_put(OP_WRITE);
    else
        code_put(OP_WRITEF);
}

/* Un nodo 'write_str' rappresenta la write di una stringa */
void write_str_check(AST ast, SymbolTable *sym) {
    /* La funzione è deliberatamente vuota */
}

void write_str_gen(AST ast, SymbolTable *sym) {
    code_put(OP_WRITE_STR);
    char *s;
    for(s=ast->str; *s!='\0'; s++)
        code_put(*s);
    code_put('\0');
}


/* Un nodo 'write_nl' rappresenta la write di un newline */
void write_nl_check(AST ast, SymbolTable *sym) {
    /* La funzione è deliberatamente vuota */
}

void write_nl_gen(AST ast, SymbolTable *sym) {
    code_put(OP_WRITE_STR);
    code_put('\n');
    code_put('\0');
}


/* Un nodo 'read' rappresenta la read di una variabile */
void read_check(AST ast, SymbolTable *sym) {
    Entry *e=symtab_lookup(sym, ast->id);
    if (e==NULL)
        error(ast->line, "Variabile non definita: '%s'", ast->id);
    if (e->entry_type!=ET_VAR)
        error(ast->line, "Read di '%s' che non è una variabile",
                ast->id);
}

void read_gen(AST ast, SymbolTable *sym) {
    Entry *e=symtab_lookup(sym, ast->id);

    if (e->value_type==TYPE_INT)
        code_put(OP_READ);
    else
        code_put(OP_READF);
    gen_address(sym, ast->id);
    code_put(OP_STORE);
}

/* Un nodo 'return' rappresenta un'istruzione return */
void return_check(AST ast, SymbolTable *sym) {
    seq_check(ast, sym);

    Entry *func=symtab_get_function_definition(sym);
    assert(func!=NULL);
    AST expr=ast->child[0];
    if (expr==NULL) {
        if (func->value_type != TYPE_VOID)
            error(ast->line,
                    "Return senza espressione in una funzione non-void");
    } else {
        if (func->value_type == TYPE_VOID)
            error(ast->line,
                    "Return con espressione in una funzione void");
        ast->child[0]=ensure_expr_type(ast->child[0], func->value_type);
    }
}

void return_gen(AST ast, SymbolTable *sym) {
    int num_formals=symtab_get_formals_counter(sym);
    AST expr=ast->child[0];
    if (expr==NULL) {
        code_put(OP_RET0);
        code_put(num_formals);
    } else {
        ast_gen(expr, sym);
        code_put(OP_RET1);
        code_put(num_formals);
    }
}

/* Un nodo 'call' rappresenta un'istruzione call */
void call_check(AST ast, SymbolTable *sym) {
    seq_check(ast, sym);
}

void call_gen(AST ast, SymbolTable *sym) {
    AST func_call=ast->child[0];
    ast_gen(func_call, sym);

    /* Se la funzione non è void, rimuove il risultato dallo stack */
    if (func_call->value_type != TYPE_VOID)
        code_put(OP_DROP);
}

/* Un nodo 'or' rappresenta un'operazione or */
void or_check(AST ast, SymbolTable *sym) {
    check_operands(ast, sym, false);
    ast->value_type=TYPE_INT;
}

void or_gen(AST ast, SymbolTable *sym) {
    /* Il codice generato usa la logica "short-circuit":
     * se il primo operando è vero, non valuta il secondo operando
     */
    ast_gen(ast->child[0], sym);

    /* Controlla il risultato del primo operando */
    code_put(OP_DUP);
    code_put(OP_JUMPNZ);
    int jump_end_addr=code_address();
    code_put(0);

    /* Rimuove dallo stack il risultato del primo operando
     * e valuta il secondo.
     */
    code_put(OP_DROP);
    ast_gen(ast->child[1], sym);

    /* Sistema l'indirizzo del salto precedente */
    code_put16_at(code_address(), jump_end_addr);
}

/* Un nodo 'and' rappresenta un'operazione and */
void and_check(AST ast, SymbolTable *sym) {
    check_operands(ast, sym, false);
    ast->value_type=TYPE_INT;
}

void and_gen(AST ast, SymbolTable *sym) {
    /* Il codice generato usa la logica "short-circuit":
     * se il primo operando è falso, non valuta il secondo operando
     */
    ast_gen(ast->child[0], sym);

    /* Controlla il risultato del primo operando */
    code_put(OP_DUP);
    code_put(OP_JUMPZ);
    int jump_end_addr=code_address();
    code_put(0);

    /* Rimuove dallo stack il risultato del primo operando
     * e valuta il secondo.
     */
    code_put(OP_DROP);
    ast_gen(ast->child[1], sym);

    /* Sistema l'indirizzo del salto precedente */
    code_put16_at(code_address(), jump_end_addr);
}

/* Un nodo 'not' rappresenta un'operazione not */
void not_check(AST ast, SymbolTable *sym) {
    check_operands(ast, sym, false);
    ast->value_type=TYPE_INT;
}

void not_gen(AST ast, SymbolTable *sym) {
    /* Not x significa x==0 */
    ast_gen(ast->child[0], sym);
    code_put(OP_PUSH8);
    code_put(0);
    code_put(OP_EQ);
}

/* Un nodo 'eq' rappresenta un'operazione == */
void eq_check(AST ast, SymbolTable *sym) {
    check_operands(ast, sym, true);
    ast->value_type=TYPE_INT;
}

void eq_gen(AST ast, SymbolTable *sym) {
    seq_gen(ast, sym); /* Genera gli operandi */
    if (ast->child[0]->value_type==TYPE_INT)
        code_put(OP_EQ);
    else
        code_put(OP_EQF);
}

/* Un nodo 'lt' rappresenta un'operazione < */
void lt_check(AST ast, SymbolTable *sym) {
    check_operands(ast, sym, true);
    ast->value_type=TYPE_INT;
}

void lt_gen(AST ast, SymbolTable *sym) {
    seq_gen(ast, sym); /* Genera gli operandi */
    if (ast->child[0]->value_type==TYPE_INT)
        code_put(OP_LT);
    else
        code_put(OP_LTF);
}

/* Un nodo 'add' rappresenta un'operazione + */
void add_check(AST ast, SymbolTable *sym) {
    check_operands(ast, sym, true);
    ast->value_type=ast->child[0]->value_type;
}

void add_gen(AST ast, SymbolTable *sym) {
    seq_gen(ast, sym); /* Genera gli operandi */
    if (ast->value_type==TYPE_INT)
        code_put(OP_ADD);
    else
        code_put(OP_ADDF);

}


/* Un nodo 'sub' rappresenta un'operazione + (sottrazione) */
void sub_check(AST ast, SymbolTable *sym) {
    check_operands(ast, sym, true);
    ast->value_type=ast->child[0]->value_type;

}

void sub_gen(AST ast, SymbolTable *sym) {
    seq_gen(ast, sym); /* Genera gli operandi */
    if (ast->value_type==TYPE_INT)
        code_put(OP_SUB);
    else
        code_put(OP_SUBF);
}


/* Un nodo 'mul' rappresenta un'operazione * */
void mul_check(AST ast, SymbolTable *sym) {
    check_operands(ast, sym, true);
    ast->value_type=ast->child[0]->value_type;
}

void mul_gen(AST ast, SymbolTable *sym) {
    seq_gen(ast, sym); /* Genera gli operandi */
    if (ast->value_type==TYPE_INT)
        code_put(OP_MUL);
    else
        code_put(OP_MULF);
}


/* Un nodo 'div' rappresenta un'operazione / */
void div_check(AST ast, SymbolTable *sym) {
    check_operands(ast, sym, true);
    ast->value_type=ast->child[0]->value_type;
}

void div_gen(AST ast, SymbolTable *sym) {
    seq_gen(ast, sym); /* Genera gli operandi */
    if (ast->value_type==TYPE_INT)
        code_put(OP_DIV);
    else
        code_put(OP_DIVF);
}


/* Un nodo 'uminus' rappresenta un'operazione - (cambio di segno) */
void uminus_check(AST ast, SymbolTable *sym) {
    check_operands(ast, sym, true);
    ast->value_type=ast->child[0]->value_type;
}

void uminus_gen(AST ast, SymbolTable *sym) {
    seq_gen(ast, sym); /* Genera l'operando */
    if (ast->value_type==TYPE_INT)
        code_put(OP_NEG);
    else
        code_put(OP_NEGF);
}

/* Un nodo 'intnum' rappresenta una costante intera */
void intnum_check(AST ast, SymbolTable *sym) {
  ast->value_type=TYPE_INT;
}

void intnum_gen(AST ast, SymbolTable *sym) {
    int n=ast->value;
    if (n>=-128 && n<128) {
        code_put(OP_PUSH8);
        code_put(n);
    } else {
        code_put(OP_PUSH32);
        code_put32(n);
    }
}


/* Un nodo 'realnum' rappresenta una costante real */
void realnum_check(AST ast, SymbolTable *sym) {
  ast->value_type=TYPE_REAL;
}

void realnum_gen(AST ast, SymbolTable *sym) {
    code_put(OP_PUSH32);
    code_putfloat(ast->real_value);
}

/* Un nodo 'id' rappresenta un identificatore usato come
 * riferimento a una variabile, un parametro formale o una costante.
 */
void id_check(AST ast, SymbolTable *sym) {
    Entry *e=symtab_lookup(sym, ast->id);
    if (e==NULL)
        error(ast->line, "Identificatore sconosciuto: '%s'", ast->id);
    EntryType et=e->entry_type;
    if (et!=ET_VAR && et!=ET_FORMAL && et!=ET_CONST)
        error(ast->line, "'%s' non è una costante, una variabile o un parametro", ast->id);
    ast->value_type=e->value_type;
}

void id_gen(AST ast, SymbolTable *sym) {
	Entry *e=symtab_lookup(sym, ast->id);

	// Se si tratta di una costante bisogna caricare il valore dall'indirizzo "globale",
	// e non relativamente ad FP
	if (e->entry_type == ET_CONST) {
		code_put(OP_PUSH32);
		code_put32(e->address);
	} else {
		gen_address(sym, ast->id);
	}

    code_put(OP_LOAD);
}

/* Un nodo 'func_call' rappresenta una chiamata di funzione */
void func_call_check(AST ast, SymbolTable *sym) {
    Entry *e=symtab_lookup(sym, ast->id);
    if (e==NULL)
        error(ast->line, "Identificatore sconosciuto: '%s'", ast->id);
    EntryType et=e->entry_type;
    if (et!=ET_FUNCTION)
        error(ast->line, "'%s' non è una funzione", ast->id);
    ast->value_type=e->value_type;

    /* Prepara il controllo dei parametri attuali */
    assert(e->sym!=NULL);
    Entry *first_formal=symtab_get_first_formal(e->sym);
    symtab_set_work_entry(sym, first_formal);
    ast_check(ast->child[0], sym);
    Entry *remaining_formal=symtab_get_work_entry(sym);
    if (remaining_formal!=NULL)
        error(ast->line, "Manca un valore per il parametro formale '%s'",
                remaining_formal->id);
}

void func_call_gen(AST ast, SymbolTable *sym) {
    Entry *e=symtab_lookup(sym, ast->id);
    assert(e!=NULL);
    if (!e->func_defined) {
   		error(ast->line, "Funzione chiamata '%s' non è mai stata definita.", ast->id);
    }

    /* Genera i parametri attuali */
    seq_gen(ast, sym);
    /* Genera la chiamata */
    code_put(OP_CALL);

    if (e->func_generated) {
    	// Caso 1: il corpo della funzione era scritto prima di questa chiamata
    	code_put16(e->address);
    } else {
    	// Caso 2: Bisogna tracciare la posizione della chiamata e risolverla più tardi
     	e->incomplete_addresses = address_list_append(e->incomplete_addresses, code_address());
      	code_put16(0);
    }
}

/* Un nodo 'actuals' rappresenta l'insieme dei parametri attuali */
void actuals_check(AST ast, SymbolTable *sym) {
    seq_check(ast, sym);
}

void actuals_gen(AST ast, SymbolTable *sym) {
    seq_gen(ast, sym);
}

/* Un nodo 'actual' rappresenta un singolo parametro attuale */
void actual_check(AST ast, SymbolTable *sym) {
    seq_check(ast, sym);
    Entry *formal=symtab_get_work_entry(sym);
    if (formal==NULL)
        error(ast->line, "Troppi parametri attuali nella chiamata");
    ast->child[0]=ensure_expr_type(ast->child[0], formal->value_type);

    /* Prepara il controllo del prossimo parametro */
    symtab_set_work_entry(sym, formal->next_formal);
}

void actual_gen(AST ast, SymbolTable *sym) {
    seq_gen(ast, sym);
}

/* Un nodo 'int2real' rappresenta la conversione da int a real */
void int2real_check(AST ast, SymbolTable *sym) {
    seq_check(ast, sym);
    ast->child[0]=ensure_expr_type(ast->child[0], TYPE_INT);
    ast->value_type=TYPE_REAL;
}

void int2real_gen(AST ast, SymbolTable *sym) {
    seq_gen(ast, sym);
    code_put(OP_INT2REAL);
}

/*--------------------------------------------------------
 * IMPLEMENTAZIONE DELLE FUNZIONI STATIC DEL MODULO
 -------------------------------------------------------*/
/* Controlla che il tipo di un'espressione sia quello desiderato;
 * se l'espressione è int e il tipo desiderato è real, inserisce
 * un nodo int2real per fare la conversione.
 * Il valore di ritorno deve essere usato al posto dell'AST
 * dell'espressione originaria.
 */
static AST ensure_expr_type(AST expr, ValueType type) {
    if (expr->value_type == type)
        return expr;
    if (expr->value_type == TYPE_INT && type==TYPE_REAL) {
        AST e=make_ast1(int2real_check, int2real_gen, expr->line, expr);
        e->value_type=TYPE_REAL;
        return e;
    }
    if (expr->value_type == TYPE_REAL && type==TYPE_INT)
        error(expr->line,
                "Espressione real dove era attesa un'espressione int");
    if (expr->value_type == TYPE_VOID)
        error(expr->line,
                "Espressione void dove era atteso un valore");
    error(expr->line, "Tipo dell'espressione non valido");
    return expr;
}

/*
 * Controlla gli operandi di un'espressione, assicurandosi che siano
 * tutti dello stesso tipo. Se allow_real è true, il tipo
 * può essere int o real, altrimenti deve essere necessariamente int.
 */
static void check_operands(AST expr, SymbolTable *sym, bool allow_real) {
    seq_check(expr, sym);
    int i;
    ValueType type=TYPE_INT;
    if (allow_real) {
        for(i=0; i<MAX_CHILDREN; i++) {
            if (expr->child[i]!=NULL && expr->child[i]->value_type==TYPE_REAL)
                type=TYPE_REAL;
        }
    }
    for(i=0; i<MAX_CHILDREN; i++) {
        if (expr->child[i]!=NULL)
            expr->child[i]=ensure_expr_type(expr->child[i], type);
    }
}

/* Genera il codice che mette sullo stack l'indirizzo di una
 * variabile (o di un parametro formale)
 */
static void gen_address(SymbolTable *sym, char *id) {
    Entry *e=symtab_lookup(sym, id);
    assert(e!=NULL);
    assert(e->entry_type==ET_VAR || e->entry_type==ET_FORMAL || e->entry_type==ET_CONST);
    code_put(OP_ADDR);
    code_put16(e->address);
}
