#pragma once
#include <stdbool.h>

#include "lexer.h"

typedef struct {
    char* name;         // Function name (must be freed)
    int paramCount;     // Number of parameters
    char* fname;        // file it was defined in
    int start;          // addresses of start
    int end;            // and end of def (not including startfunc)
} Function;

typedef struct {
    Function func;      // Function metadata
    int* params;        // Array of integer parameters (must be freed)
    int params_len;     // Length of params array
    char* params_char;  // String parameter for includes/specs (must be freed)
} Call;

Call parse_line(const lexTok* code, const int in_size, bool verbose);