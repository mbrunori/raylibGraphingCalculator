#ifndef SHUNTINGYARD_H
#define SHUNTINGYARD_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define MAX_TOKENS 256
#define MAX_OPS 256

typedef struct {
    char out[MAX_TOKENS];
    size_t len;
} Output;

void outAppend(Output* o, const char* s);
void outTrimTrailingSpace(Output* o);

typedef struct {
    char items[MAX_OPS];
    int top;
} OpStack;

void stackInit(OpStack *s);
int  stack_empty(const OpStack *s);
char stack_peek(const OpStack *s);
void stack_push(OpStack *s, char c);
char stack_pop(OpStack *s);

int isOp(char op);
int precedence(char op);
int rightAssociative(char op);
int isIdentStart(unsigned char c);
char* shuntingYard(const char* str);

#endif