#pragma once
#include "stack.h"

typedef enum {
    Keyw, // unused, historic
    String,
    Func,
    Include,
    Num,
    Spec,
    endCall // aka a colon
} lexTokType;

typedef struct {
    lexTokType type;
    char* val;
} lexTok;

lexTok* lex_line(char* code_raw_, int size, int* toks_size_out, struct Stack *stack);