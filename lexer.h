#pragma once

typedef enum {
    Keyw,
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

char Funcs[10][10];

lexTok* lex_line(char* code_raw_, int size, int* toks_size_out);