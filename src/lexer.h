#pragma once

typedef enum {
    Keyw,    // reserved keyword
    Num,     // a number
    oBrace,  // opening brace "("
    cBrace,  // closing brace ")"
    oSBrace, // opening square brace "["
    cSBrace, // closing square brace "]"
    funcName,// name of a function
    Colon,   // end of a command ";"
    TOKEN_END // Sentinel for the end of the token list
} tokenType;

struct Token {
    tokenType type;
    char* val;
};

extern char* reserved[];

struct Token* tokenize_line(char line[], const int line_size);
int test(void);
const char* get_token_type_name(tokenType type);