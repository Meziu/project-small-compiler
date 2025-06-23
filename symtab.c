#include <stdlib.h>
#include <assert.h>

#include "symtab.h"
#include "error.h"

/*--------------------------------------------------------
 * DEFINIZIONI DI COSTANTI E DI TIPI
 -------------------------------------------------------*/
#define BUCKETS 7

struct SymbolTable {
    SymbolTable *parent;
    Entry *bucket[BUCKETS];
    Entry *function;
    int num_formals;
    int num_locals;
    Entry *first_formal;
    Entry *last_formal;
    Entry *work_entry;
};

/*--------------------------------------------------------
 * PROTOTIPI DELLE FUNZIONI STATIC DEL MODULO
 -------------------------------------------------------*/
static unsigned hash(char *id);
static Entry *lookup(Entry *first, char *id);
static Entry *make_entry(char *id, EntryType type, int line);

/*--------------------------------------------------------
 * IMPLEMENTAZIONE DELLE FUNZIONI PUBBLICHE DEL MODULO
 -------------------------------------------------------*/
SymbolTable *make_symtab(SymbolTable *parent) {
    SymbolTable *sym=malloc(sizeof(SymbolTable));
    if (sym==NULL)
        error(-1, "Out of memory!");
    sym->parent=parent;
    int i;
    for(i=0; i<BUCKETS; i++)
        sym->bucket[i]=NULL;
    sym->function=NULL;
    sym->num_formals=0;
    sym->num_locals=0;
    sym->first_formal=NULL;
    sym->last_formal=NULL;
    sym->work_entry=NULL;
    return sym;
}

Entry *symtab_define(SymbolTable *sym, char *id, EntryType type, int line) {
    assert(sym!=NULL);
    assert(id!=NULL);
    int h=hash(id)%BUCKETS;
    Entry *e=lookup(sym->bucket[h], id);
    if (e!=NULL)
        error(line, "Redefinition of identifier %s. "
                "Previous definition was in line %d.", id, e->line);
    e=make_entry(id, type, line);
    e->link=sym->bucket[h];
    sym->bucket[h]=e;

    if (type==ET_FORMAL) {
        /* Connette il parametro formale agli altri parametri formali */
        if (sym->first_formal==NULL) {
            sym->first_formal=e;
        } else {
            sym->last_formal->next_formal=e;
        }
        sym->last_formal=e;
    }
    return e;
}

Entry *symtab_lookup(SymbolTable *sym, char *id) {
    assert(sym!=NULL);
    assert(id!=NULL);
    int h=hash(id)%BUCKETS;
    while (sym!=NULL) {
        Entry *e=lookup(sym->bucket[h], id);
        if (e!=NULL)
            return e;
        sym=sym->parent;
    }
    return NULL;
}

void symtab_set_function_definition(SymbolTable *sym, Entry *func) {
    assert(sym!=NULL);
    sym->function=func;
}

Entry *symtab_get_function_definition(SymbolTable *sym) {
    assert(sym!=NULL);
    while (sym!=NULL) {
        if (sym->function!=NULL)
            return sym->function;
        sym=sym->parent;
    }
    return NULL;
}

int symtab_get_locals_counter(SymbolTable *sym) {
    assert(sym!=NULL);
    return sym->num_locals;
}

void symtab_increment_locals_counter(SymbolTable *sym, int inc) {
    assert(sym!=NULL);
    sym->num_locals+=inc;
}

int symtab_get_formals_counter(SymbolTable *sym) {
    assert(sym!=NULL);
    return sym->num_formals;
}

void symtab_increment_formals_counter(SymbolTable *sym, int inc) {
    assert(sym!=NULL);
    sym->num_formals+=inc;
}


Entry *symtab_get_first_formal(SymbolTable *sym) {
    return sym->first_formal;
}

void symtab_set_work_entry(SymbolTable *sym, Entry *entry) {
    sym->work_entry=entry;
}

Entry *symtab_get_work_entry(SymbolTable *sym) {
    return sym->work_entry;
}


/*--------------------------------------------------------
 * IMPLEMENTAZIONE DELLE FUNZIONI STATIC DEL MODULO
 -------------------------------------------------------*/
static unsigned hash(char *id) {
    return (unsigned)(unsigned long)id;
}

static Entry *lookup(Entry *first, char *id) {
    Entry *p=first;
    while (p!=NULL && p->id !=id)
        p=p->link;
    return p;
}

static Entry *make_entry(char *id, EntryType type, int line) {
    Entry *e=malloc(sizeof(Entry));
    if (e==NULL)
        error(line, "Out of memory!");
    e->id=id;
    e->entry_type=type;
    e->line=line;
    e->next_formal=NULL;
    e->value_type=TYPE_UNKNOWN;
    e->sym=NULL;
    e->link=NULL;
    
    return e;
}

