#include <stddef.h>
#include <stdbool.h>
#include "scanner.h"
#include "token_codes.h"
#include "parser.h"
#include "ast_functions.h"
#include "error.h"

/*--------------------------------------------------------
 * PROTOTIPI DELLE FUNZIONI STATIC DEL MODULO
 -------------------------------------------------------*/

static AST parse_const(void);
static AST parse_func_def(void);
static AST parse_type(void);
static AST parse_formals(void);
static AST parse_formal(void);
static AST parse_body(void);
static AST parse_def(void);
static AST parse_statement(void);
static AST parse_assignment(void);
static AST parse_if(void);
static AST parse_else(void);
static AST parse_while(void);
static AST parse_do_while(void);
static AST parse_for(void);
static AST parse_block(void);
static AST parse_write(void);
static AST parse_write_item(void);
static AST parse_read(void);
static AST parse_return(void);
static AST parse_call(void);
static AST parse_expr(void);
static AST parse_and_expr(void);
static AST parse_not_expr(void);
static AST parse_rel_expr(void);
static AST parse_add_expr(void);
static AST parse_mul_expr(void);
static AST parse_factor(void);
static AST parse_func_call(void);
static AST parse_actuals(void);
static AST parse_actual(void);

/*--------------------------------------------------------
 * IMPLEMENTAZIONE DELLE FUNZIONI PUBBLICHE DEL MODULO
 -------------------------------------------------------*/
AST parse_program(void) {
	// Le costanti simboliche, come le variabili nelle funzioni, possono essere
	// definite solo all'inizio del programma prima delle funzioni.
	AST constants = NULL;
	while (sc_current() == TOK_CONST) {
		AST c = parse_const();
		constants = make_ast2(seq_check, seq_gen, sc_current_line(), constants, c);
	}

    AST defs=parse_func_def();
    while (sc_current() != TOK_EOF) {
        AST fd=parse_func_def();
        defs=make_ast2(seq_check, seq_gen, defs->line,
                           defs, fd);
    }
    sc_match(TOK_EOF);
    AST program=make_ast2(program_check, program_gen, sc_current_line(),
            constants, defs);
    return program;
}


/*--------------------------------------------------------
 * IMPLEMENTAZIONE DELLE FUNZIONI STATIC DEL MODULO
 -------------------------------------------------------*/

static AST parse_const(void) {
	sc_match(TOK_CONST);

	AST type=parse_type();

    char *id=sc_current_id();
    int line=sc_current_line();
    sc_match(TOK_ID);
    sc_match('=');
    AST expr=parse_expr();
    sc_match('!');
    AST st=make_ast1(const_check, const_gen, line, expr);
    st->id=id;
    st->value_type=type->value_type;
    return st;
}

static AST parse_func_def(void) {
    AST type=parse_type();
    char *id=sc_current_id();
    sc_match(TOK_ID);
    sc_match('(');
    AST formals=parse_formals();
    sc_match(')');
    AST body=parse_body();
    AST func_def=make_ast2(func_def_check, func_def_gen, type->line,
                     formals, body);
    func_def->id=id;
    func_def->value_type=type->value_type;
    return func_def;
}

static AST parse_type(void) {
    ValueType vt=TYPE_UNKNOWN;
    int tok=sc_current();
    if (tok==TOK_INT)
        vt=TYPE_INT;
    else if (tok==TOK_REAL)
        vt=TYPE_REAL;
    else if (tok==TOK_VOID)
        vt=TYPE_VOID;
    else
        error(sc_current_line(), "Expected the name of a type, found: '%s'",
                sc_token_name(tok));
    sc_advance();
    AST ast=make_ast(type_check, type_gen, sc_current_line());
    ast->value_type=vt;
    return ast;
}

static AST parse_formals(void) {
    int tok=sc_current();
    int line=sc_current_line();
    if (tok==')') {
        return make_ast(formals_check, formals_gen, line);
    } else {
        AST curr=parse_formal();
        while (sc_current() == ',') {
            sc_advance();
            AST next=parse_formal();
            curr=make_ast2(seq_check, seq_gen, curr->line, curr, next);
        }
        return make_ast1(formals_check, formals_gen, line, curr);
    }
}

static AST parse_formal(void) {
    AST type=parse_type();
    char *id=sc_current_id();
    sc_match(TOK_ID);
    AST formal=make_ast(formal_check, formal_gen, type->line);
    formal->id=id;
    formal->value_type=type->value_type;
    return formal;
}

static AST parse_body(void) {
    int line=sc_current_line();
    sc_match('{');
    AST defs=make_ast(empty_check, empty_gen, sc_current_line());
    int tok=sc_current();
    while (tok==TOK_INT || tok==TOK_REAL || tok==TOK_VOID) {
        AST def=parse_def();
        defs=make_ast2(seq_check, seq_gen, defs->line,
                defs, def);
        tok=sc_current();
    }
    AST statements=make_ast(empty_check, empty_gen, sc_current_line());
    while (sc_current()!='}') {
        AST st=parse_statement();
        statements=make_ast2(seq_check, seq_gen, statements->line,
                     statements, st);
    }
    sc_match('}');
    return make_ast2(body_check, body_gen, line, defs, statements);
}

static AST parse_def(void) {
    AST type=parse_type();
    char *id=sc_current_id();
    sc_match(TOK_ID);
    AST expr=NULL;
    if (sc_current()=='=') {
        sc_advance();
        expr=parse_expr();
    }
    sc_match('!');
    AST def=make_ast1(def_check, def_gen, type->line, expr);
    def->id=id;
    def->value_type=type->value_type;
    return def;
}

static AST parse_statement(void) {
    int tok=sc_current();
    if (tok==TOK_ID)
        return parse_assignment();
    else if (tok==TOK_IF)
        return parse_if();
    else if (tok==TOK_WHILE)
        return parse_while();
    else if (tok==TOK_DO)
        return parse_do_while();
    else if (tok==TOK_FOR)
        return parse_for();
    else if (tok==TOK_WRITE)
        return parse_write();
    else if (tok==TOK_READ)
        return parse_read();
    else if (tok==TOK_RETURN)
        return parse_return();
    else if (tok==TOK_CALL)
        return parse_call();
    else
        error(sc_current_line(),
                "Unvalid token for the start of a statement: '%s'",
                sc_token_name(tok));
    return NULL;
}

static AST parse_assignment(void) {
    char *id=sc_current_id();
    int line=sc_current_line();
    sc_match(TOK_ID);
    sc_match('=');
    AST expr=parse_expr();
    sc_match('!');
    AST st=make_ast1(assignment_check, assignment_gen, line, expr);
    st->id=id;
    return st;
}

static AST parse_if(void) {
    int line=sc_current_line();
    sc_match(TOK_IF);
    AST expr=parse_expr();
    AST then_part=parse_block();
    AST else_part=NULL;
    if (sc_current()==TOK_ELSE)
        else_part=parse_else();
    return make_ast3(if_check, if_gen, line, expr, then_part, else_part);
}

static AST parse_else(void) {
    sc_match(TOK_ELSE);
    if (sc_current()==TOK_IF)
        return parse_if();
    else
        return parse_block();
}

static AST parse_while(void) {
    int line=sc_current_line();
    sc_match(TOK_WHILE);
    AST expr=parse_expr();
    AST block=parse_block();
    return make_ast2(while_check, while_gen, line, expr, block);
}

static AST parse_do_while(void) {
    int line=sc_current_line();
    sc_match(TOK_DO);
    AST block=parse_block();
    sc_match(TOK_WHILE);
    AST expr=parse_expr();
    sc_match('!'); // La condizione non è seguita da un blocco, bensì è conclusiva.
    return make_ast2(do_while_check, do_while_gen, line, expr, block);
}

static AST parse_for(void) {
    int line=sc_current_line();
    sc_match(TOK_FOR);

    // Identificatore della variabile di iterazione utilizzata.
    char* iter_name = sc_current_id();
    sc_advance();

    sc_match(TOK_IN);

    // Espressione di inizializzazione
    AST init=parse_expr();

    sc_match(':'); // Separatore tra inizializzatore e condizione.

    // Espressione di condizione
    AST cond=parse_expr();
    // Blocco di istruzioni del for
    AST block=parse_block();

    AST node = make_ast3(for_check, for_gen, line, init, cond, block);
    node->id = iter_name; // Utilizziamo il campo "id" dell'AST per indicare l'identificatore della variabile iterativa.
    return node;
}

static AST parse_block(void) {
    int line=sc_current_line();
    sc_match('{');
    AST statements=make_ast(empty_check, empty_gen, line);
    while (sc_current()!='}') {
        AST st=parse_statement();
        statements=make_ast2(seq_check, seq_gen, statements->line,
                     statements, st);
    }
    sc_match('}');
    return statements;
}

static AST parse_write(void) {
    sc_match(TOK_WRITE);
    AST items=parse_write_item();
    while (sc_current()==',') {
        sc_advance();
        AST it=parse_write_item();
        items=make_ast2(seq_check, seq_gen, items->line,
                items, it);
    }

    /* Aggiunge un nodo per stampare un newline alla fine */
    AST nl=make_ast(write_nl_check, write_nl_gen, sc_current_line());
    items=make_ast2(seq_check, seq_gen, items->line,
                items, nl);
    sc_match('!');
    return items;
}

static AST parse_write_item(void) {
    int line=sc_current_line();
    if (sc_current()==TOK_STR) {
        char *str=sc_current_string();
        sc_advance();
        AST item=make_ast(write_str_check,write_str_gen, line);
        item->str=str;
        return item;
    } else {
        AST expr=parse_expr();
        return make_ast1(write_check, write_gen, line, expr);
    }
}

static AST parse_read(void) {
    sc_match(TOK_READ);
    int line1=sc_current_line();
    char *str=NULL;
    if (sc_current()==TOK_STR) {
        str=sc_current_string();
        sc_match(TOK_STR);
        sc_match(',');
    }
    char *id=sc_current_id();
    int line2=sc_current_line();
    sc_match(TOK_ID);
    sc_match('!');
    AST read=make_ast(read_check, read_gen, line2);
    read->id=id;
    if (str!=NULL) {
        AST write=make_ast(write_str_check, write_str_gen, line1);
        write->str=str;
        read=make_ast2(seq_check, seq_gen, line1, write, read);
    }
    return read;
}

static AST parse_return(void) {
    int line=sc_current_line();
    sc_match(TOK_RETURN);
    AST expr=NULL;
    if (sc_current()!='!')
        expr=parse_expr();
    sc_match('!');

    return make_ast1(return_check, return_gen, line, expr);
}

static AST parse_call(void) {
    int line=sc_current_line();
    sc_match(TOK_CALL);
    AST fc=parse_func_call();
    sc_match('!');
    return make_ast1(call_check, call_gen, line, fc);
}

static AST parse_expr(void) {
    AST e=parse_and_expr();
    while (sc_current()==TOK_OR) {
        sc_advance();
        AST e1=parse_and_expr();
        e=make_ast2(or_check, or_gen, e->line, e, e1);
    }
    return e;
}

static AST parse_and_expr(void) {
    AST e=parse_not_expr();
    while (sc_current()==TOK_AND) {
        sc_advance();
        AST e1=parse_not_expr();
        e=make_ast2(and_check, and_gen, e->line, e, e1);
    }
    return e;
}

static AST parse_not_expr(void) {
    int line=sc_current_line();
    if (sc_current()==TOK_NOT) {
        sc_advance();
        AST e1=parse_not_expr();
        return make_ast1(not_check, not_gen, line, e1);
    } else {
        return parse_rel_expr();
    }
}

static AST parse_rel_expr(void) {
    AST e=parse_add_expr();
    int t=sc_current();
    if (t==TOK_EQ || t==TOK_NE || t=='<' || t=='>' || t==TOK_LE || t==TOK_GE) {
        sc_advance();
        AST e1=parse_add_expr();
        if (t==TOK_EQ) {
            e=make_ast2(eq_check, eq_gen, e->line, e, e1);
        } else if (t==TOK_NE) {
            e=make_ast2(eq_check, eq_gen, e->line, e, e1);
            e=make_ast1(not_check, not_gen, e->line, e);
        } else if (t=='<') {
            e=make_ast2(lt_check, lt_gen, e->line, e, e1);
        } else if (t=='>') {
            e=make_ast2(lt_check, lt_gen, e->line, e1, e);
        } else if (t==TOK_LE) {
            e=make_ast2(lt_check, lt_gen, e->line, e1, e);
            e=make_ast1(not_check, not_gen, e->line, e);
        } else if (t==TOK_GE) {
            e=make_ast2(lt_check, lt_gen, e->line, e, e1);
            e=make_ast1(not_check, not_gen, e->line, e);
        }
    }
    return e;
}

static AST parse_add_expr(void) {
    AST e=parse_mul_expr();
    int t=sc_current();
    while (t=='+' || t=='-') {
        sc_advance();
        AST e1=parse_mul_expr();
        if (t=='+')
            e=make_ast2(add_check, add_gen, e->line, e, e1);
        else
            e=make_ast2(sub_check, sub_gen, e->line, e, e1);
        t=sc_current();
    }
    return e;
}

static AST parse_mul_expr(void) {
    AST e=parse_factor();
    int t=sc_current();
    while (t=='*' || t=='/') {
        sc_advance();
        AST e1=parse_factor();
        if (t=='*')
            e=make_ast2(mul_check, mul_gen, e->line, e, e1);
        else
            e=make_ast2(div_check, div_gen, e->line, e, e1);
        t=sc_current();
    }
    return e;
}

static AST parse_factor(void) {
    int t=sc_current();
    int line=sc_current_line();
    AST f=NULL;
    if (t==TOK_NUM) {
        f=make_ast(intnum_check, intnum_gen, line);
        f->value=sc_current_value();
        sc_advance();
    } else if (t==TOK_NUMF) {
        f=make_ast(realnum_check, realnum_gen, line);
        f->real_value=sc_current_float_value();
        sc_advance();
    } else if (t=='-') {
        sc_advance();
        f=parse_factor();
        f=make_ast1(uminus_check, uminus_gen, line, f);
    } else if (t=='(') {
        sc_advance();
        f=parse_expr();
        sc_match(')');
    } else if (t==TOK_ID) {
        char *id=sc_current_id();
        sc_advance();
        if (sc_current()=='(') {
            sc_advance();
            AST actuals=parse_actuals();
            sc_match(')');
            f=make_ast1(func_call_check, func_call_gen, line, actuals);
            f->id=id;
        } else {
            f=make_ast(id_check, id_gen, line);
            f->id=id;
        }
    } else {
        error(line, "Unexpected token in expression: '%s'",
                sc_token_name(t));
    }
    return f;
}

static AST parse_func_call(void) {
    int line=sc_current_line();
    char *id=sc_current_id();
    sc_match(TOK_ID);
    sc_match('(');
    AST actuals=parse_actuals();
    sc_match(')');
    AST f=make_ast1(func_call_check, func_call_gen, line, actuals);
    f->id=id;
    return f;
}

static AST parse_actuals(void) {
    int line=sc_current_line();
    if (sc_current()==')')
        return make_ast(actuals_check, actuals_gen, line);
    AST curr=parse_actual();
    while (sc_current()==',') {
        sc_advance();
        AST next=parse_actual();
        curr=make_ast2(seq_check, seq_gen, curr->line, curr, next);
    }
    return make_ast1(actuals_check, actuals_gen, line, curr);
}

static AST parse_actual(void) {
    AST expr=parse_expr();
    return make_ast1(actual_check, actual_gen, expr->line, expr);
}
