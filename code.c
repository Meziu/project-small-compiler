#include <stdio.h>
#include <assert.h>
#include "error.h"
#include "code.h"

/*------------------------------------------------------------------
 * VARIABILI STATIC DEL MODULO
 *----------------------------------------------------------------*/
#define MAX_CODE 8192
#define MAX_DATA 16384

static unsigned char code_memory[MAX_CODE];
static int data_memory[MAX_DATA];
static float *data_memory_as_float=(float *)data_memory;

static int pc, sp, fp;
static int code_limit=0;

/*------------------------------------------------------------------
 * PROTOTIPI DELLE FUNZIONI STATIC
 *----------------------------------------------------------------*/
static inline int fetch_u8(void);
static inline int fetch_i8(void);
static inline int fetch_u16(void);
static inline int fetch_i16(void);
static inline int fetch_i32(void);
static inline void execute(int op_code);

/*------------------------------------------------------------------
 * IMPLEMENTAZIONE DELLE FUNZIONI PUBBLICHE DEL MODULO
 *----------------------------------------------------------------*/
int code_address(void) {
    return code_limit;
}

void code_put(unsigned char b) {
    if (code_limit+1>MAX_CODE)
        error(-1, "Code memory exhausted");
    code_memory[code_limit++]=b;
}

void code_put_at(unsigned char b, unsigned short addr) {
    code_memory[addr]=b;
}


void code_put16(short w) {
    code_put(w >> 8);
    code_put(w);
}


void code_put16_at(short w, unsigned short addr) {
    code_put_at(w >> 8, addr);
    code_put_at(w, addr+1);
}


void code_put32(int w) {
    code_put(w >> 24);
    code_put(w >> 16);
    code_put(w >> 8);
    code_put(w);
}

void code_put32_at(int w, unsigned short addr) {
    code_put_at(w >> 24, addr);
    code_put_at(w >> 16, addr+1);
    code_put_at(w >> 8, addr+2);
    code_put_at(w, addr+3);
}


void code_putfloat(float w) {
    assert(sizeof(int)==sizeof(float));
    int iw=*(int *)&w;
    code_put32(iw);
}

void code_putfloat_at(float w, unsigned short addr) {
    assert(sizeof(int)==sizeof(float));
    int iw=*(int *)&w;
    code_put32_at(iw, addr);
}


void code_run(void) {
    pc=0;
    fp=0;
    sp=MAX_DATA-1;
    int op=fetch_u8();
    while (op!=OP_HALT) {
        execute(op);
        if (sp<-1)
            error(-1, "Stack overflow!");
        else if (sp>=MAX_DATA)
            error(-1, "Stack underflow!");
        op=fetch_u8();
    }
}

/*------------------------------------------------------------------
 * IMPLEMENTAZIONE DELLE FUNZIONI STATIC
 *----------------------------------------------------------------*/
static inline int fetch_u8(void) {
    assert(pc>=0 && pc<code_limit);
    return (unsigned char)code_memory[pc++];
}

static inline int fetch_i8(void) {
    assert(pc>=0 && pc<code_limit);
    return (signed char)code_memory[pc++];
}

static inline int fetch_u16(void) {
    assert(pc>=0 && pc+1<code_limit);
    unsigned short w=code_memory[pc++];
    w=(w<<8)+code_memory[pc++];
    return w;
}

static inline int fetch_i16(void) {
    assert(pc>=0 && pc+1<code_limit);
    short w=code_memory[pc++];
    w=(w<<8)+code_memory[pc++];
    return w;
}

static inline int fetch_i32(void) {
    assert(pc>=0 && pc+3<code_limit);
    int w=code_memory[pc++];
    w=(w<<8)+code_memory[pc++];
    w=(w<<8)+code_memory[pc++];
    w=(w<<8)+code_memory[pc++];
    return w;
}

static inline void execute(int op_code) {
    int a, b, n;
    float fa, fb;
    unsigned short addr;
    assert(sizeof(int)==sizeof(float));
    switch (op_code) {
    case OP_HALT:
        break;
    case OP_JUMP:
        pc=fetch_u16();
        break;
    case OP_JUMPZ:
        addr=fetch_u16();
        a=data_memory[++sp];
        if (a==0)
            pc=addr;
        break;
    case OP_JUMPNZ:
        addr=fetch_u16();
        a=data_memory[++sp];
        if (a!=0)
            pc=addr;
        break;
    case OP_CALL:
        addr=fetch_u16();
        data_memory[sp--]=pc;
        pc=addr;
        break;
    case OP_ENTER:
        n=fetch_u16();
        data_memory[sp--]=fp;
        fp=sp+1;
        sp-=n;
        break;
    case OP_RET0:
        n=fetch_u8();
        sp=fp;
        fp=data_memory[sp];
        addr=data_memory[++sp];
        sp+=n;
        pc=addr;
        break;
    case OP_RET1:
        n=fetch_u8();
        a=data_memory[++sp];
        sp=fp;
        fp=data_memory[sp];
        addr=data_memory[++sp];
        sp+=n;
        data_memory[sp--]=a;
        pc=addr;
        break;
    case OP_ADD:
        b=data_memory[++sp];
        a=data_memory[++sp];
        data_memory[sp--]=a+b;
        break;
    case OP_ADDF:
        fb=data_memory_as_float[++sp];
        fa=data_memory_as_float[++sp];
        data_memory_as_float[sp--]=fa+fb;
        break;
    case OP_SUB:
        b=data_memory[++sp];
        a=data_memory[++sp];
        data_memory[sp--]=a-b;
        break;
    case OP_SUBF:
        fb=data_memory_as_float[++sp];
        fa=data_memory_as_float[++sp];
        data_memory_as_float[sp--]=fa-fb;
        break;
    case OP_MUL:
        b=data_memory[++sp];
        a=data_memory[++sp];
        data_memory[sp--]=a*b;
        break;
    case OP_MULF:
        fb=data_memory_as_float[++sp];
        fa=data_memory_as_float[++sp];
        data_memory_as_float[sp--]=fa*fb;
        break;
    case OP_DIV:
        b=data_memory[++sp];
        a=data_memory[++sp];
        data_memory[sp--]=a/b;
        break;
    case OP_DIVF:
        fb=data_memory_as_float[++sp];
        fa=data_memory_as_float[++sp];
        data_memory_as_float[sp--]=fa/fb;
        break;
    case OP_NEG:
        a=data_memory[++sp];
        data_memory[sp--]=-a;
        break;
    case OP_NEGF:
        fa=data_memory_as_float[++sp];
        data_memory_as_float[sp--]=-fa;
        break;
    case OP_INT2REAL:
        a=data_memory[++sp];
        data_memory_as_float[sp--]=(float)a;
        break;
    case OP_EQ:
        b=data_memory[++sp];
        a=data_memory[++sp];
        data_memory[sp--]=(a==b);
        break;
    case OP_EQF:
        fb=data_memory_as_float[++sp];
        fa=data_memory_as_float[++sp];
        data_memory[sp--]=(fa==fb);
        break;
    case OP_LT:
        b=data_memory[++sp];
        a=data_memory[++sp];
        data_memory[sp--]=(a<b);
        break;
    case OP_LTF:
        fb=data_memory_as_float[++sp];
        fa=data_memory_as_float[++sp];
        data_memory[sp--]=(fa<fb);
        break;
    case OP_PUSH8:
        a=(signed char)fetch_i8();
        data_memory[sp--]=a;
        break;
    case OP_PUSH32:
        a=fetch_i32();
        data_memory[sp--]=a;
        break;
    case OP_DUP:
        a=data_memory[sp+1];
        data_memory[sp--]=a;
        break;
    case OP_DROP:
        ++sp;
        break;
    case OP_LOAD:
        a=data_memory[++sp];
        if (a<0 || a>=MAX_DATA)
            error(-1, "Unvalid load address: %d", a);
        data_memory[sp--]=data_memory[a];
        break;
    case OP_STORE:
        b=data_memory[++sp];
        a=data_memory[++sp];
        if (b<0 || b>=MAX_DATA)
            error(-1, "Unvalid store address: %d", b);
        data_memory[b]=a;
        break;
    case OP_ADDR:
        n=fetch_i16();
        data_memory[sp--]=fp+n;
        break;
    case OP_READ:
        scanf("%d", &a);
        data_memory[sp--]=a;
        break;
    case OP_READF:
        scanf("%f", &fa);
        data_memory_as_float[sp--]=fa;
        break;
    case OP_WRITE:
        a=data_memory[++sp];
        printf("%d", a);
        break;
    case OP_WRITEF:
        fa=data_memory_as_float[++sp];
        printf("%g", fa);
        break;
    case OP_WRITE_STR:
        a=fetch_u8();
        while (a!=0) {
            putchar(a);
            a=fetch_u8();
        }
        break;
    default:
        error(-1, "Unvalid opcode: %d", op_code);
        break;
    }
}
