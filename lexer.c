#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "lexer.h"
#include "helpers.h"

#define _CRT_SECURE_NO_WARNINGS

extern char Funcs[10][10] = {
    "push",
    "discard",
    "startFunc",
    "endFunc"
};


/*

bool validName(const char* str) {
    if (str == NULL || *str == '\0') return false;
    if (!isalpha((unsigned char)str[0])) return false;
    for (size_t i = 1; i < strlen(str); i++) {
        if (!isalnum((unsigned char)str[i])) return false;
    }
    return true;
}
*/



lexTok* lex_line(char* code_raw_, int size, int* toks_size_out) {
    if (size <= 0 || code_raw_[size - 1] != ';') {
        fprintf(stderr, "Lexer Error: Line must end with ';'\n");
        exit(EXIT_FAILURE);
    }

    char* code_raw = strdup(code_raw_);
    code_raw[size - 1] = '\0'; // remove semicolon

    int split_size = 0;
    char** code = split_str(code_raw, " (),", &split_size);
    int toksSize = 0;
    lexTok* toks = NULL;

    for (int i = 0; i < split_size; i++) {
        char* curr = code[i];

        if (curr == NULL || strlen(curr) == 0) continue;

        lexTok new_tok;
        bool found = false;
        char* token_value = NULL;

        // Check for Include (@)
        if (*curr == '@') {
            new_tok.type = Include;
            // Skip the '@' character
            token_value = strdup(curr + 1);
            found = true;
        }
        // Check for Spec (?)
        else if (*curr == '?') {
            new_tok.type = Spec;
            // Skip the '?' character
            token_value = strdup(curr + 1);
            found = true;
        }
        // Check for Keyword
        else if (isKeyw(curr)) {
            new_tok.type = Keyw;
            token_value = strdup(curr);
            found = true;
        }
        // Check for Number
        else if (isInt(curr)) {
            new_tok.type = Num;
            token_value = strdup(curr);
            found = true;
        }
        // Check for String
        else if (isString(curr)) {
            char* curr_dup = strdup(curr);
            process_str_tok(curr_dup);
            new_tok.type = String;
            token_value = curr_dup;
            found = true;
        }
        // Otherwise it's a Function name
        else {
            new_tok.type = Func;
            token_value = strdup(curr);
            found = true;
        }

        if (found) {
            new_tok.val = token_value;
            toks = (lexTok*)safe_realloc((char*)toks, (toksSize + 1) * sizeof(lexTok));
            toks[toksSize] = new_tok;
            toksSize++;
        }
    }

    // Append the end call token
    lexTok end;
    end.type = endCall;
    end.val = NULL;
    toks = (lexTok*)safe_realloc((char*)toks, (toksSize + 1) * sizeof(lexTok));
    toks[toksSize] = end;
    toksSize++;

    // Cleanup
    if (code != NULL) {
        for (int i = 0; i < split_size; i++) {
            free(code[i]);
        }
        free(code);
    }
    free(code_raw);

    *toks_size_out = toksSize;
    return toks;
}

const char* get_token_type_string(lexTokType type) {
    switch (type) {
    case Keyw: return "Keyw";
    case String: return "String";
    case Num: return "Num";
    case Func: return "Func";
    case Include: return "Include";
    case Spec: return "Specifier";
    case endCall: return "Colon (end of call)";
    default: return "Unknown";
    }
}

int test(void) {
    int sz = 0;

    char input[] = "@stdlib;";
    lexTok* tokens = lex_line(input, (int)strlen(input), &sz);

    if (tokens != NULL) {
        for (int i = 0; i < sz; i++) {
            printf("Token %d: %s (Value: %s)\n", i, get_token_type_string(tokens[i].type), tokens[i].val);
        }

        // Cleanup
        for (int i = 0; i < sz; i++) {
            free(tokens[i].val);
        }
        free(tokens);
    }

    return 0;
}