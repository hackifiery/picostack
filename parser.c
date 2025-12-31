#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <stdio.h>

#include "lexer.h"
#include "helpers.h"
#include "parser.h"

/*typedef struct {
    char* name;
    int paramCount;
    bool isExtern; // if it's defined in the source rather than in a picostack file
} Function;

typedef struct {
    Function func;
    int* params;
    int params_len;
    char* params_char; // used for includes and specs
} Call;
*/
Call parse_line(const lexTok* code, const int in_size) {
    Call out;
    memset(&out, 0, sizeof(Call)); // Initialize to zero

    // Validate input
    if (code == NULL || in_size == 0) {
        fprintf(stderr, "Parser Error: Invalid input\n");
        exit(EXIT_FAILURE);
    }

    lexTokType type = code[0].type;

    // Validate first token type
    assert(type == Func || type == Keyw || type == Spec || type == Include);

    // Set function name based on type
    if (type == Func || type == Keyw) {
        out.func.name = strdup(code[0].val); // Must duplicate!
        out.func.isExtern = (type == Keyw);
    }
    else if (type == Spec) {
        out.func.name = strdup("_spec");
        out.func.isExtern = false;
    }
    else if (type == Include) {
        out.func.name = strdup("_inc");
        out.func.isExtern = false;
    }

    // Parse parameters based on type
    if (type == Include || type == Spec) {
        // For Include/Spec, store the identifier as string
        out.params = NULL;
        out.params_len = strlen(code[0].val);
        out.params_char = strdup(code[0].val);
        out.func.paramCount = 1; // One string parameter
    }
    else if (type == Func || type == Keyw) {
        // Parse function parameters
        out.params_char = NULL;

        // Count and collect parameters
        int param_capacity = 10;
        out.params = (int*)malloc(param_capacity * sizeof(int));
        out.params_len = 0;

        // Iterate through tokens until we hit endCall
        for (int i = 1; i < in_size && code[i].type != endCall; i++) {
            if (code[i].type == Num) {
                // Integer parameter
                int val = atoi(code[i].val);

                // Ensure capacity
                if (out.params_len >= param_capacity) {
                    param_capacity *= 2;
                    out.params = (int*)realloc(out.params, param_capacity * sizeof(int));
                    if (out.params == NULL) {
                        fprintf(stderr, "Parser Error: Memory allocation failed\n");
                        exit(EXIT_FAILURE);
                    }
                }

                out.params[out.params_len++] = val;
            }
            else if (code[i].type == String) {
                // String parameter - convert to int array (character codes)
                int str_len = 0;
                int* str_params = string_to_params(code[i].val, &str_len);

                // Ensure capacity for all characters
                while (out.params_len + str_len > param_capacity) {
                    param_capacity *= 2;
                    out.params = (int*)realloc(out.params, param_capacity * sizeof(int));
                    if (out.params == NULL) {
                        fprintf(stderr, "Parser Error: Memory allocation failed\n");
                        exit(EXIT_FAILURE);
                    }
                }

                // Copy string characters as integers
                memcpy(out.params + out.params_len, str_params, str_len * sizeof(int));
                out.params_len += str_len;

                free(str_params);
            }
            else if (code[i].type == Func || code[i].type == Keyw) {
                fprintf(stderr, "Parser Error: Nested function calls not supported\n");
                exit(EXIT_FAILURE);
            }
        }

        out.func.paramCount = out.params_len;

        // If no parameters, free the array
        if (out.params_len == 0) {
            free(out.params);
            out.params = NULL;
        }
    }

    return out;
}

void free_call(Call* call) {
    if (call == NULL) return;

    if (call->func.name) {
        free(call->func.name);
        call->func.name = NULL;
    }
    if (call->params) {
        free(call->params);
        call->params = NULL;
    }
    if (call->params_char) {
        free(call->params_char);
        call->params_char = NULL;
    }
}

void print_call(const Call* call) {
    if (call == NULL) return;

    printf("Function: %s\n", call->func.name);
    printf("  isExtern: %s\n", call->func.isExtern ? "true" : "false");
    printf("  paramCount: %d\n", call->func.paramCount);

    if (call->params_char) {
        printf("  params_char: %s\n", call->params_char);
    }

    if (call->params && call->params_len > 0) {
        printf("  params: [");
        for (int i = 0; i < call->params_len; i++) {
            printf("%d", call->params[i]);
            if (i < call->params_len - 1) printf(", ");
        }
        printf("]\n");
    }
}

// Test the parser
int main(void) {
    printf("=== Parser Tests ===\n\n");

    // Test 1: Include
    {
        int sz = 0;
        char input[] = "@stdlib;";
        lexTok* tokens = lex_line(input, (int)strlen(input), &sz);
        Call call = parse_line(tokens, sz);

        printf("Test 1 - Include:\n");
        print_call(&call);
        printf("\n");

        free_call(&call);
        for (int i = 0; i < sz; i++) free(tokens[i].val);
        free(tokens);
    }

    // Test 2: Function with numbers
    {
        int sz = 0;
        char input[] = "push(42, 100);";
        lexTok* tokens = lex_line(input, (int)strlen(input), &sz);
        Call call = parse_line(tokens, sz);

        printf("Test 2 - Function with numbers:\n");
        print_call(&call);
        printf("\n");

        free_call(&call);
        for (int i = 0; i < sz; i++) free(tokens[i].val);
        free(tokens);
    }

    // Test 3: Keyword (builtin function)
    {
        int sz = 0;
        char input[] = "discard();";
        lexTok* tokens = lex_line(input, (int)strlen(input), &sz);
        Call call = parse_line(tokens, sz);

        printf("Test 3 - Keyword:\n");
        print_call(&call);
        printf("\n");

        free_call(&call);
        for (int i = 0; i < sz; i++) free(tokens[i].val);
        free(tokens);
    }

    // Test 4: Function with string
    {
        int sz = 0;
        char input[] = "push(\"Hello\n\");";
        lexTok* tokens = lex_line(input, (int)strlen(input), &sz);
        Call call = parse_line(tokens, sz);

        printf("Test 4 - Function with string:\n");
        print_call(&call);
        printf("  params as chars: ");
        for (int i = 0; i < call.params_len; i++) {
            if (call.params[i] == 0) printf("\\0");
            else printf("%c", (char)call.params[i]);
        }
        printf("\n\n");

        free_call(&call);
        for (int i = 0; i < sz; i++) free(tokens[i].val);
        free(tokens);
    }

    return 0;
}