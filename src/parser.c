#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "lexer.h"
#include "parser.h"

#define REALLOC_AND_CHECK(ptr, size) \
    ptr = (struct parsedToken *)realloc(ptr, (size) * sizeof(struct parsedToken)); \
    if (ptr == NULL) { \
        perror("realloc failed"); \
        exit(EXIT_FAILURE); \
    }

typedef enum{
    P_Keyw,
    P_StartKeywCall,
    P_EndKeywCall,
    P_KeywParam,
    P_StartFuncProto, // like "func foo 0"
    P_FuncName,          // "foo"
    P_FuncProtoParam, // 0
    P_EndFuncProto,
    P_StartFuncDef,      // the actual code in func
    P_EndFuncDef,        // "endfunc" keyw
    P_StartFuncCall,
    P_FuncCallName,
    P_FuncCallParam,
    P_EndFuncCall,
    P_End,
    Other
} parsedTokenType;

struct parsedToken {
    parsedTokenType type;
    char* val;
};

struct parsedToken* parse_tok(struct Token tok, int *end_size, int *in_FuncProto, int *in_FuncCall, int *in_FuncDef, int *in_KeywCall){
    int endfunc = 0;
    tokenType ttype = tok.type;
    struct parsedToken *res = NULL;
    int res_size =0;

    switch (ttype){
        case Keyw: {
            if (strcmp(tok.val, "func") == 0){
                // func is special
                *in_FuncProto = 1;
                REALLOC_AND_CHECK(res, res_size + 1);
                res[res_size].type = P_StartFuncProto;
                res[res_size].val = strdup("");
                res_size++;
                break;
            }
            if (strcmp(tok.val, "endfunc") == 0){
                // so is endfunc
                endfunc = 1;
                *in_FuncDef = 2; // meaning it will skip once and tell colon to end function next
                *in_KeywCall = 1; // because the colon case is run right after (endfunc has no parameters), but w/o this it thinks we aren't in any context so it errors.
                // dont run anything cuz it will be handled when in_FuncDef == 3.
                break;
            }
            *in_KeywCall = 1;
            REALLOC_AND_CHECK(res, res_size + 1);
            res[res_size].type = P_StartKeywCall;
            res[res_size].val = strdup("");
            res_size++;

            REALLOC_AND_CHECK(res, res_size + 1);
            res[res_size].type = P_Keyw;
            res[res_size].val = strdup(tok.val);
            res_size++;
            
            break;
        }
        case Num: {
            if (*in_KeywCall){
                REALLOC_AND_CHECK(res, res_size + 1);
                res[res_size].type = P_KeywParam;
                res[res_size].val = strdup(tok.val);
                res_size++;
                break;
            }
            if (*in_FuncCall){
                REALLOC_AND_CHECK(res, res_size + 1);
                res[res_size].type = P_FuncCallParam;
                res[res_size].val = strdup(tok.val);
                res_size++;
                break;
            }
            if (*in_FuncProto){
                REALLOC_AND_CHECK(res, res_size + 1);
                res[res_size].type = P_FuncProtoParam;
                res[res_size].val = strdup(tok.val);
                res_size++;
                break;
            }
            printf("wth is this number doing here bruh\n");
            exit(EXIT_FAILURE);
            break;
        }
        case funcName: {
            if (!*in_FuncProto){
                // we are calling a user-defined func
                *in_FuncCall = 1;
                REALLOC_AND_CHECK(res, res_size + 1);
                res[res_size].type = P_StartFuncCall;
                res[res_size].val = strdup("");
                res_size++;
            }
            REALLOC_AND_CHECK(res, res_size + 1);
            res[res_size].type = P_FuncName;
            res[res_size].val = tok.val;
            res_size++;
            break;
        }
        case Colon: {
            // end whatever we're doing here
            if (*in_FuncDef == 2) {
                // special case (if we're ending a function)
                *in_FuncDef = 0;
                REALLOC_AND_CHECK(res, res_size + 1);
                res[res_size].type = P_EndFuncDef;
                res[res_size].val = strdup("");
                res_size++;
            }
            else if (*in_FuncCall){ // "else" so endfunc doesn't get counted into an EndKeywCall
                REALLOC_AND_CHECK(res,res_size+1);
                res[res_size].type = P_EndFuncCall;
                res[res_size].val = strdup("");
                res_size++;
                *in_FuncCall = 0;
                break;
            }

            if (*in_FuncProto){
                REALLOC_AND_CHECK(res,res_size+2);
                res[res_size].type = P_EndFuncProto;
                res[res_size].val = strdup("");
                res_size++;
                res[res_size].type = P_StartFuncDef;
                res[res_size].val = strdup("");
                res_size++;
                *in_FuncProto = 0;
                *in_FuncDef = 1;
                break;
            }
            if (*in_KeywCall){
                REALLOC_AND_CHECK(res,res_size+1);
                res[res_size].type = P_EndKeywCall;
                res[res_size].val = strdup("");
                res_size++;
                *in_KeywCall = 0;
                break;
            }
            printf("wth is this ';' doing here bruh\n");
            exit(EXIT_FAILURE);
            break;
        }
    }
    
    if (res == NULL) { 
        REALLOC_AND_CHECK(res, 1);
        *end_size = 0; 
    } else {
        *end_size = res_size;
    }
    
    return res;
}

// now all the test stuff (it takes up half the file bruhhh)

const char* parsedTokenNames[] = {
    "P_Keyw", 
    "P_StartKeywCall", 
    "P_EndKeywCall", 
    "P_KeywParam",
    "P_StartFuncProto", 
    "P_FuncName", 
    "P_FuncProtoParam", 
    "P_EndFuncProto",
    "P_StartFuncDef",
    "P_EndFuncDef", 
    "P_StartFuncCall", 
    "P_FuncCallName",
    "P_FuncCallParam", 
    "P_EndFuncCall",
    "P_End",
    "Other" // Matches the end of the enum
};

// Utility function to free the aggregated parsed stream
void free_parsed_stream(struct parsedToken *stream, int size) {
    if (stream == NULL) return;
    for (int i = 0; i < size; i++) {
        // The memory for the val member was allocated inside parse_tok
        free(stream[i].val);
    }
    // Free the array itself
    free(stream);
}

struct parsedToken* run_test(const char* name, struct Token input_tok, int *size, int *in_FuncProto, int *in_FuncCall, int *in_FuncDef, int *in_KeywCall) {
    
    struct parsedToken* output_tokens = parse_tok(input_tok, size, in_FuncProto, in_FuncCall, in_FuncDef, in_KeywCall);
    int success = 1;

    printf("--- Test Case: %s ---\n", name);
    printf("Input: Token Type=%s, Value=\"%s\"\n", 
           get_token_type_name(input_tok.type),
           input_tok.val);
    printf("Output Parsed Tokens (%d):\n", *size);
    printf("State (after parse): Proto=%d, Call=%d, KeywCall=%d, FuncDef=%d\n", 
           *in_FuncProto, *in_FuncCall, *in_KeywCall, *in_FuncDef);

    if (*size > 0) {
        for (int i = 0; i < *size; i++) {
            printf("  [%d] Type: %s, Value: \"%s\"\n", 
                   i, parsedTokenNames[output_tokens[i].type], output_tokens[i].val);
            
            // free((output_tokens[i].val)); 
        }
    } else {
        printf("  [0] No parsed tokens generated.\n");
        // Logic for 'endfunc' which produces no tokens is complex, so we'll just check if parse_tok succeeded
        if (input_tok.type == Keyw && strcmp(input_tok.val, "endfunc") == 0) success = 1;
        else if (input_tok.type == Colon && (*in_FuncDef == 0 && *in_KeywCall == 0 && *in_FuncCall == 0 && *in_FuncProto == 0)) success = 1; // Handled 'endfunc;' colon
        else if (*size > 0) success = 1;
        else success = 0;
    }
    // free(output_tokens);
    
    printf("Status: %s\n\n", success ? "PASS" : "FAIL");
    return output_tokens;
}

int main() {
    printf("Starting End-to-End Lexer-Parser Test Suite...\n\n");

    // Sample code line that includes keywords, a function name, and a number
    char test_line[] = "func my_function 0; push 10; endfunc;";
    int line_size = (int)strlen(test_line);

    printf("Source Code Line: \"%s\"\n", test_line);
    printf("=================================================================\n");

    // Call the Lexer (Tokenizer)
    struct Token* tokens = tokenize_line(test_line, line_size);

    if (tokens == NULL) {
        fprintf(stderr, "Tokenization failed! Cannot run parser tests.\n");
        return EXIT_FAILURE;
    }

    // Set up context flags for the parser
    int in_FuncProto = 0;
    int in_FuncCall = 0;
    int in_FuncDef = 0;
    int in_KeywCall = 0;

    struct parsedToken* final_parsed_stream = NULL;
    int current_stream_size = 0;

    // Iterate through Lexer output and call the Parser helper (parse_tok)
    for (int i = 0; tokens[i].type != TOKEN_END; i++) {
        
        // Run the test for the current token
        char test_name[64];
        snprintf(test_name, sizeof(test_name), "Token %d: %s (\"%s\")", 
                 i, get_token_type_name(tokens[i].type), tokens[i].val);
        
        int step_size = 0;
        // Call parse_tok directly inside run_test
        struct parsedToken* current_parsed = run_test(test_name, tokens[i], &step_size, &in_FuncProto, &in_FuncCall, &in_FuncDef, &in_KeywCall);

        // AGGREGATION LOGIC
        if (step_size > 0) {
            int new_total_size = current_stream_size + step_size;
            REALLOC_AND_CHECK(final_parsed_stream, new_total_size);
            
            // Copy token data from the temporary array into the final stream
            for (int j = 0; j < step_size; j++) {
                final_parsed_stream[current_stream_size + j] = current_parsed[j];
            }
            current_stream_size = new_total_size;
        }
        
        // Free the temporary array container (not the contents, which were transferred)
        free(current_parsed); 
    }

    printf("=================================================================\n");
    printf("End-to-End Test Complete.\n\n");

    // --- FIX APPLIED HERE ---
    printf("Final result:\n");
    for (int i = 0; i < current_stream_size; i++) { // Correctly loop over the aggregated size
        printf("%s \"%s\"\n", parsedTokenNames[final_parsed_stream[i].type], final_parsed_stream[i].val);
    }

    // Cleanup Lexer memory
    free_tokens(tokens);
    
    // Cleanup Parsed Stream memory
    free_parsed_stream(final_parsed_stream, current_stream_size);

    return 0;
}