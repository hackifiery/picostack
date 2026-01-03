#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "lexer.h"
#include "helpers.h"

#define _CRT_SECURE_NO_WARNINGS

char Funcs[][10] = {
    "push",
    "discard",
    "startfunc",
    "endfunc"
};

lexTok* lex_line(char* code_raw_, int size, int* toks_size_out) {
    if (size <= 0 || code_raw_[size - 2] != ';') {
        fprintf(stderr, "Lexer Error: Line must end with ';'\n");
        exit(EXIT_FAILURE);
    }

    // Copy input and remove trailing semicolon
    char* code_raw = strdup(code_raw_);
    code_raw[size - 2] = '\0';

    int split_size = 0;

    // do NOT split on quotes
    char** code = split_str(code_raw, " (),", &split_size);

    int toksSize = 0;
    lexTok* toks = NULL;

    for (int i = 0; i < split_size; i++) {
        char* curr = code[i];
        if (!curr || *curr == '\0') continue;

        lexTok tok;
        tok.val = NULL;

        if (*curr == '@') {
            tok.type = Include;
            tok.val = strdup(curr + 1);
        }
        else if (*curr == '?') {
            tok.type = Spec;
            tok.val = strdup(curr + 1);
        }
        else if (isKeyw(curr)) {
            tok.type = Keyw;
            tok.val = strdup(curr);
        }
        else if (isInt(curr)) {
            tok.type = Num;
            tok.val = strdup(curr);
        }
        else if (isString(curr)) {
            tok.type = String;
            tok.val = strdup(curr);
            process_str_tok(tok.val);
        }
        else {
            tok.type = Func;
            tok.val = strdup(curr);
        }

        toks = safe_realloc(toks, (toksSize + 1) * sizeof(lexTok));
        toks[toksSize++] = tok;
    }

    // End-of-call token
    lexTok end = { .type = endCall, .val = NULL };
    toks = safe_realloc(toks, (toksSize + 1) * sizeof(lexTok));
    toks[toksSize++] = end;

    // Cleanup
    for (int i = 0; i < split_size; i++) free(code[i]);
    free(code);
    free(code_raw);

    *toks_size_out = toksSize;
    return toks;
}
