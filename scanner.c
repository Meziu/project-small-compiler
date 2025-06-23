#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdarg.h>
#include <assert.h>

#include "scanner.h"
#include "error.h"

#define BUCKETS 131

#define MAX_TOKEN_NAME 15 
#define MAX_COMMENT_DELIMITER 2
#define MAX_TOKEN_SIZE 512
#define NUM_CHARS 256

/*------------------------------------------------------------------
 * DEFINIZIONI DI TIPO
 ------------------------------------------------------------------*/
typedef struct HTNode HTNode;
struct HTNode {
    char *name;
    int  code;
    HTNode *link;
};

/*------------------------------------------------------------------
 * VARIABILI STATIC DEL MODULO
 ------------------------------------------------------------------*/
static HTNode *id_table[BUCKETS];

static bool op1[NUM_CHARS];
static bool op2begin[NUM_CHARS];
static char cbegin[MAX_COMMENT_DELIMITER+1];
static char cend[MAX_COMMENT_DELIMITER+1];
static char token_name[MAX_TOKEN_NAME+1];

static FILE *file=NULL;
static char buffer[MAX_TOKEN_SIZE+1];
static int current_char=EOF;
static int current_token=TOK_EOF;
static int current_line=1;
static int current_value;
static float current_float_value;
static char *current_str;
static char *current_id;

/*------------------------------------------------------------------
 * PROTOTIPI DELLE FUNZIONI STATIC DEL MODULO
 ------------------------------------------------------------------*/
static unsigned hash(char *name);
static HTNode *lookup(char *name);
static HTNode *reverse_lookup(int code);
static HTNode *insert(char *name, int code);


static void next_char(void);
static void prev_char(int c);
static void skip_spaces(void);
static bool check_and_skip(char c1, char c2);
static bool skip_comment(void);

static void scan_num(void);
static void scan_digits(int *pos);
static void insert_current_char(int *pos);
static void scan_id(void);
static void scan_str(void);
static void scan_other(void);


/*------------------------------------------------------------------
 * IMPLEMENTAZIONE DELLE FUNZIONI PUBBLICHE DEL MODULO
 ------------------------------------------------------------------*/


void sc_init(TokenDefinition keywords[], int num_keywords,
        TokenDefinition op2chars[], int num_op2chars,
        char op1char[],
        char *comment_begin,
        char *comment_end) {
    int i;
    for(i=0; i<num_keywords; i++) {
        if (lookup(keywords[i].text)!=NULL)
            error(-1, "Keyword %s defined twice!", keywords[i].text);
        insert(keywords[i].text, keywords[i].code);
    }
    for(i=0; i<num_op2chars; i++) {
        TokenDefinition *t=&op2chars[i];
        if (strlen(t->text)!=2)
            error(-1, "Operator '%s' is not 2 characters!", t->text);
        if (lookup(t->text)!=NULL)
            error(-1, "Operator %s defined twice!", keywords[i].text);
        insert(t->text, t->code);
        op2begin[(unsigned char)t->text[0]]=true;
    }
    for(i=0; op1char[i]!='\0'; i++)
        op1[(unsigned char)op1char[i]]=true;
    
    if (strlen(comment_begin)>MAX_COMMENT_DELIMITER)
        error(-1, "Comment begin delimiter '%s' is too long!", comment_begin);
    strcpy(cbegin, comment_begin);
    if (strlen(comment_end)>MAX_COMMENT_DELIMITER)
        error(-1, "Comment end delimiter '%s' is too long!", comment_end);
    strcpy(cend, comment_end);
}

void sc_open(char *fname) {
    if (file!=NULL) {
        fclose(file);
        set_error_filename(NULL);
    }
    file=fopen(fname, "r");
    if (file==NULL)
        error(-1, "Cannot open file: %s", fname);
    set_error_filename(fname);
    current_line=1;
    current_char='\0';
    next_char();
    sc_advance();
}
        
void sc_close(void) {
    fclose(file);
    file=NULL;
    set_error_filename(NULL);
    current_char=EOF;
}

int sc_advance(void) {
    do {
        skip_spaces();
    } while (skip_comment());
    if (current_char==EOF) {
        current_token=TOK_EOF;
    } else if (isdigit(current_char)) {
        /* A numeric literal */
        scan_num();
    } else if (isalpha(current_char) || current_char=='_') {
        /* An identifier or a keyword */
        scan_id();
    } else if (current_char=='\"') {
        /* A string literal */
        scan_str();
    } else {
        /* A 1- or 2-character operator/delimiter */
        scan_other();
    }
    
    return current_token;
}

int sc_current(void) {
    return current_token;
}

int sc_current_line(void) {
    return current_line;
}

int sc_current_value(void) {
    if (current_token==TOK_NUM)
        return current_value;
    else
        return 0;
}

float sc_current_float_value(void) {
    if (current_token==TOK_NUMF)
        return current_float_value;
    else
        return 0.0;
}


char *sc_current_string(void) {
    if (current_token==TOK_STR)
        return current_str;
    else
        return NULL;
}

char *sc_current_id(void) {
    if (current_token==TOK_ID)
        return current_id;
    else
        return NULL;
}

char *sc_make_id(char *name) {
    HTNode *h=lookup(name);
    if (h==NULL)
        h=insert(name, TOK_ID);
    return h->name;
}

int sc_match(int code) {
    char expected[MAX_TOKEN_NAME+1];
    if (current_token != code) {
        strcpy(expected, sc_token_name(code));
        error(current_line, "Found '%s', expected '%s'",
                sc_token_name(current_token),
                expected);
    }
    return sc_advance();
}

int sc_match_n(int n, ...) {
    va_list ap;
    va_start(ap, n);
    int i;
    bool found=false;
    for(i=0; i<n && !found; i++) {
        int code=va_arg(ap,int);
        if (current_token==code)
            found=true;
    } 
    va_end(ap);
    
    if (found) {
        return sc_advance();
    }
    
    int lim=MAX_TOKEN_SIZE / (MAX_TOKEN_NAME+1);
    if (n>lim)
        n=lim;
    int j=0;
    va_start(ap, n);
    for(i=0; i<n && !found; i++) {
        int code=va_arg(ap,int);
        char *t=sc_token_name(code);
        int len=strlen(t);
        strcpy(buffer+j, t);
        buffer[j+len]=' ';
        j+=len+1;
    } 
    va_end(ap);
    buffer[j]='\0';
    error(current_line, "Found '%s', expected one of %s",
                sc_token_name(current_token), buffer);
    return TOK_EOF; /* Never executed */
}

char* sc_token_name(int code) {
    if (code==TOK_EOF)
        return "EOF";
    else if (code==TOK_NUM)
        return "NUM";
    else if (code==TOK_NUMF)
        return "NUMF";
    else if (code==TOK_ID)
        return "ID";
    else if (code==TOK_STR)
        return "STR";
    HTNode *h=reverse_lookup(code);
    if (h!=NULL) {
        strncpy(token_name, h->name, MAX_TOKEN_NAME);
        token_name[MAX_TOKEN_NAME]='\0';
        return token_name;
    }
    token_name[0]=code;
    token_name[1]='\0';
    return token_name;
}



/*------------------------------------------------------------------
 * IMPLEMENTAZIONE DELLE FUNZIONI STATIC DEL MODULO
 ------------------------------------------------------------------*/
static unsigned hash(char *name) {
    unsigned h=0;
    int i;
    for(i=0; name[i]!='\0'; i++) {
        h=h*31+name[i];
    }
    return h;
}

static HTNode *lookup(char *name) {
    int i=hash(name) % BUCKETS;
    HTNode *p=id_table[i];
    while (p!=NULL && strcmp(p->name,name)!=0)
        p=p->link;
    return p;
}

static HTNode *reverse_lookup(int code) {
    int i;
    for(i=0; i<BUCKETS; i++) {
        HTNode *p=id_table[i];
        while (p!=NULL && p->code != code)
            p=p->link;
        if (p!=NULL)
            return p;
    }
    return NULL;
}


static HTNode* insert(char *name, int code) {
    int i=hash(name) % BUCKETS;
    HTNode *p=malloc(sizeof(HTNode));
    if (p==NULL)
        error(-1, "Out of memory!");
    int n=strlen(name)+1;
    p->name=malloc(n);
    if (p->name==NULL)
        error(-1, "Out of memory!");
    strcpy(p->name, name);
    p->code=code;
    p->link=id_table[i];
    id_table[i]=p;
    return p;
}


static void next_char(void) {
    if (current_char==EOF)
        return;
    bool new_line=(current_char == '\n');
    current_char=fgetc(file);
    if (new_line)
        current_line++;
}

static void prev_char(int c) {
    ungetc(current_char, file);
    if (c=='\n')
        current_line--;
    current_char=c;
}


static void skip_spaces(void) {
    while (current_char!=EOF && isspace(current_char))
        next_char();
}

static bool check_and_skip(char c1, char c2) {
    if (current_char==c1) {
        next_char();
        if (c2=='\0')
            return true;
        if (current_char==c2) {
            next_char();
            return true;
        }
        prev_char(c1);
    }
    return false;
}


static bool skip_comment(void) {
    if (!check_and_skip(cbegin[0], cbegin[1]))
        return false;
        
    /* Now we are inside a comment */
    
    while (current_char != EOF) {
        if (check_and_skip(cend[0], cend[1]))
            return true;
        next_char();
    }
    error(current_line, "EOF while inside a comment");
    return false; /* Never executed */
}


static void scan_num(void) {
    int i=0;
    bool is_float=false;
    buffer[MAX_TOKEN_SIZE]='\0';
    scan_digits(&i);
    if (current_char=='.') {
        insert_current_char(&i);
        scan_digits(&i);
        is_float=true;
    }
    if (current_char=='e' || current_char=='E') {
        insert_current_char(&i);
        if (current_char=='+' || current_char=='-')
            insert_current_char(&i);
        if (!isdigit(current_char)) {
            buffer[i]='\0';
            error(current_line, "Invalid number: %s", buffer);
        }
        scan_digits(&i);
        is_float=true;
    }
    buffer[i]='\0';
    if (is_float) {
        current_float_value=(float)atof(buffer);
        current_token=TOK_NUMF;
    } else {
        current_value=atoi(buffer);
        current_token=TOK_NUM;
    }
}

static void scan_digits(int *pos) {
    while (isdigit(current_char)) {
        insert_current_char(pos);
    }
}

static void insert_current_char(int *pos) {
    int i=*pos;
    if (i<MAX_TOKEN_SIZE) {
        buffer[i]=current_char;
        next_char();
        ++ *pos;
    } else {
        buffer[MAX_TOKEN_SIZE]='\0';
        error(current_line, "Token too long: %s", buffer);
    }

}


static void scan_id(void) {
    int i=0;
    buffer[MAX_TOKEN_SIZE]='\0';
    while (isalnum(current_char) || current_char=='_') {
        insert_current_char(&i);
    }
    buffer[i]='\0';
    HTNode *h=lookup(buffer);
    if (h==NULL)
        h=insert(buffer, TOK_ID);
    current_id=h->name;
    current_token=h->code;
}


static void scan_str(void) {
    next_char();
    int i=0;
    buffer[MAX_TOKEN_SIZE]='\0';
    while (current_char!=EOF && current_char!='\"' && current_char!='\n') {
        char c=current_char;
        if (check_and_skip('\\', '\\'))
            c='\\';
        else if (check_and_skip('\\', '\"'))
            c='\"';
        else if (check_and_skip('\\', 'n'))
            c='\n';
        else
            next_char();
        if (i<MAX_TOKEN_SIZE)
            buffer[i++]=c;
        else
            error(current_line, "String literal too long: '%s'", buffer);
    }
    buffer[i]='\0';
    if (current_char!='\"')
        error(current_line, "String literal unterminated: '%s'", buffer);
    next_char();
    current_str=malloc(i+1);
    if (current_str==NULL)
        error(current_line, "Out of memory!");
    strcpy(current_str, buffer);
    current_token=TOK_STR;
}

static void scan_other(void) {
    assert(current_char!=EOF);
    
    if (op2begin[current_char]) {
        int c1=current_char;
        next_char();
        if (current_char!=EOF) {
            buffer[0]=c1;
            buffer[1]=current_char;
            buffer[2]='\0';
            HTNode *h=lookup(buffer);
            if (h!=NULL) {
                next_char();
                current_token=h->code;
                return;
            }
        } 
        prev_char(c1);    
    }
    
    if (op1[current_char]) {
        current_token=current_char;
        next_char();
    } else {
        error(current_line, "Unvalid character '%c' (0x%02X)",
                current_char, current_char);
    }
}
