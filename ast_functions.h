/* Questo modulo definisce le funzioni di analisi semantica
 * e di generazione del codice per i vari tipi di nodo dell'AST.
 */


#ifndef AST_FUNCTIONS_H
#define AST_FUNCTIONS_H

#include "ast.h"

void empty_check(AST ast, SymbolTable *sym);
void empty_gen(AST ast, SymbolTable *sym);

void seq_check(AST ast, SymbolTable *sym);
void seq_gen(AST ast, SymbolTable *sym);

void program_check(AST ast, SymbolTable *sym);
void program_gen(AST ast, SymbolTable *sym);

void func_def_check(AST ast, SymbolTable *sym);
void func_def_gen(AST ast, SymbolTable *sym);

void type_check(AST ast, SymbolTable *sym);
void type_gen(AST ast, SymbolTable *sym);

void formals_check(AST ast, SymbolTable *sym);
void formals_gen(AST ast, SymbolTable *sym);

void formal_check(AST ast, SymbolTable *sym);
void formal_gen(AST ast, SymbolTable *sym);

void body_check(AST ast, SymbolTable *sym);
void body_gen(AST ast, SymbolTable *sym);

void const_check(AST ast, SymbolTable *sym);
void const_gen(AST ast, SymbolTable *sym);

void def_check(AST ast, SymbolTable *sym);
void def_gen(AST ast, SymbolTable *sym);

void assignment_check(AST ast, SymbolTable *sym);
void assignment_gen(AST ast, SymbolTable *sym);

void if_check(AST ast, SymbolTable *sym);
void if_gen(AST ast, SymbolTable *sym);

void while_check(AST ast, SymbolTable *sym);
void while_gen(AST ast, SymbolTable *sym);

void do_while_check(AST ast, SymbolTable *sym);
void do_while_gen(AST ast, SymbolTable *sym);

void for_check(AST ast, SymbolTable *sym);
void for_gen(AST ast, SymbolTable *sym);

void write_check(AST ast, SymbolTable *sym);
void write_gen(AST ast, SymbolTable *sym);

void write_str_check(AST ast, SymbolTable *sym);
void write_str_gen(AST ast, SymbolTable *sym);

void write_nl_check(AST ast, SymbolTable *sym);
void write_nl_gen(AST ast, SymbolTable *sym);

void read_check(AST ast, SymbolTable *sym);
void read_gen(AST ast, SymbolTable *sym);

void return_check(AST ast, SymbolTable *sym);
void return_gen(AST ast, SymbolTable *sym);

void call_check(AST ast, SymbolTable *sym);
void call_gen(AST ast, SymbolTable *sym);

void or_check(AST ast, SymbolTable *sym);
void or_gen(AST ast, SymbolTable *sym);

void and_check(AST ast, SymbolTable *sym);
void and_gen(AST ast, SymbolTable *sym);

void not_check(AST ast, SymbolTable *sym);
void not_gen(AST ast, SymbolTable *sym);

void eq_check(AST ast, SymbolTable *sym);
void eq_gen(AST ast, SymbolTable *sym);

void lt_check(AST ast, SymbolTable *sym);
void lt_gen(AST ast, SymbolTable *sym);

void add_check(AST ast, SymbolTable *sym);
void add_gen(AST ast, SymbolTable *sym);

void sub_check(AST ast, SymbolTable *sym);
void sub_gen(AST ast, SymbolTable *sym);

void mul_check(AST ast, SymbolTable *sym);
void mul_gen(AST ast, SymbolTable *sym);

void div_check(AST ast, SymbolTable *sym);
void div_gen(AST ast, SymbolTable *sym);

void uminus_check(AST ast, SymbolTable *sym);
void uminus_gen(AST ast, SymbolTable *sym);

void intnum_check(AST ast, SymbolTable *sym);
void intnum_gen(AST ast, SymbolTable *sym);

void realnum_check(AST ast, SymbolTable *sym);
void realnum_gen(AST ast, SymbolTable *sym);

void id_check(AST ast, SymbolTable *sym);
void id_gen(AST ast, SymbolTable *sym);

void func_call_check(AST ast, SymbolTable *sym);
void func_call_gen(AST ast, SymbolTable *sym);

void actuals_check(AST ast, SymbolTable *sym);
void actuals_gen(AST ast, SymbolTable *sym);

void actual_check(AST ast, SymbolTable *sym);
void actual_gen(AST ast, SymbolTable *sym);

void int2real_check(AST ast, SymbolTable *sym);
void int2real_gen(AST ast, SymbolTable *sym);


#endif
