#include <stdlib.h>
#include <assert.h>
#include "ast.h"
#include "error.h"

AST make_ast(ASTNodeFunction check, ASTNodeFunction gen, int line) {
    return make_ast3(check, gen, line, NULL, NULL, NULL);
}

AST make_ast1(ASTNodeFunction check, ASTNodeFunction gen, int line,
              AST child1) {
    return make_ast3(check, gen, line, child1, NULL, NULL);
}

AST make_ast2(ASTNodeFunction check, ASTNodeFunction gen, int line,
              AST child1, AST child2) {
    return make_ast3(check, gen, line, child1, child2, NULL);
}

AST make_ast3(ASTNodeFunction check, ASTNodeFunction gen, int line,
              AST child1, AST child2, AST child3) {
    AST p=malloc(sizeof(ASTNode));
    if (p==NULL)
        error(line, "Out of memory!");
    p->check = check;
    p->gen = gen;
    p->child[0]=child1;
    p->child[1]=child2;
    p->child[2]=child3;
    p->line = line;
   
    p->value_type=TYPE_UNKNOWN; 
    p->value=0;
    p->real_value=0.0;
    p->id=NULL;
    p->str=NULL;
    return p;
}

void ast_check(AST ast, SymbolTable *sym) {
    assert(ast!=NULL);
    assert(ast->check!=NULL);
    ast->check(ast, sym);
}

void ast_gen(AST ast, SymbolTable *sym) {
    assert(ast!=NULL);
    assert(ast->gen!=NULL);
    ast->gen(ast, sym);
}
