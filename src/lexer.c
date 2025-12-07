#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

//#include "stack.h"
#include "lexer.h"
// --- Function Prototypes for Tokenizer ---
char** split_string(char* str, const char* delimiter, int* num_tokens);
struct Token* tokenize_line(char line[], const int line_size);
void free_tokens(struct Token* tokens);
int is_reserved(const char* word);
const char* get_token_type_name(tokenType type);

const char* get_token_type_name(tokenType type) {
    switch (type) {
        case Keyw:    return "Keyw";
        case Num:     return "Num";
        case oBrace:  return "oBrace";
        case cBrace:  return "cBrace";
        case oSBrace: return "oSBrace";
        case cSBrace: return "cSBrace";
        case funcName:return "funcName";
        case Colon:   return "Colon";
        case TOKEN_END: return "TOKEN_END";
        default:      return "UNKNOWN";
    }
}


extern char* reserved[] = {
    "push", "ps",
    "discard", "rm"
    "dup", "swap", "reverse", "add", "sub", "jump",
    "out", "outint", "in", "inint", 
    "rotright", "rotr", "rotleft", "rotl",
    "func", "endfunc",
    NULL // Null terminator
};

// --- Helper Implementation (is_reserved) ---
int is_reserved(const char* word) {
    for (int i = 0; reserved[i] != NULL; i++) {
        if (strcmp(word, reserved[i]) == 0) {
            return 1; // Is reserved
        }
    }
    return 0; // Not reserved
}

// splits a string by delimeter
char** split_string(char* str, const char* delimiter, int* num_tokens) {
    char* temp_str = strdup(str);
    if (temp_str == NULL) {
        perror("strdup failed");
        return NULL;
    }

    // First pass to count tokens
    int count = 0;
    char* token = strtok(temp_str, delimiter);
    while (token != NULL) {
        count++;
        token = strtok(NULL, delimiter);
    }
    free(temp_str);

    // Allocate memory for the array of strings
    char** tokens = (char**)malloc(sizeof(char*) * (count + 1));
    if (tokens == NULL) {
        perror("malloc failed");
        return NULL;
    }

    // Second pass to store tokens
    int i = 0;
    temp_str = strdup(str);
    if (temp_str == NULL) {
        perror("strdup failed");
        free(tokens);
        return NULL;
    }

    token = strtok(temp_str, delimiter);
    while (token != NULL) {
        tokens[i] = strdup(token);
        if (tokens[i] == NULL) {
            perror("strdup failed");
            for (int j = 0; j < i; j++) {
                free(tokens[j]);
            }
            free(tokens);
            free(temp_str);
            return NULL;
        }
        i++;
        token = strtok(NULL, delimiter);
    }
    tokens[i] = NULL;

    *num_tokens = count;
    free(temp_str);
    return tokens;
}

// turns one line into an array of Tokens
struct Token* tokenize_line(char line[], const int line_size){
    int max_tokens = line_size / 2 + 10;
    struct Token* tokens = (struct Token*)malloc(sizeof(struct Token) * max_tokens);
    if (tokens == NULL) {
        perror("malloc failed for tokens");
        return NULL;
    }
    int token_count = 0;
    
    // Split the input line into commands based on the ';' delimiter
    int split_col_size;
    char** split_col = split_string(line, ";", &split_col_size);
    if (split_col == NULL) {
        free(tokens);
        return NULL;
    }
    int comment = 0;
    // Process each command string
    for (int i = 0; i < split_col_size; i++){
        char* command = split_col[i];
        int len = strlen(command);
        
        for (int j = 0; j < len; j++) {
            char curr = command[j];
            
            if (isspace(curr)) {
                continue;
            }
            if (curr == '#') {
                comment = 1;
                break; // Ignore the rest of this command string (comment)
            }

            // Single-Character Tokens
            if (curr == '(' || curr == ')' || curr == '[' || curr == ']') {
                tokens[token_count].val = (char*)malloc(2);
                tokens[token_count].val[0] = curr;
                tokens[token_count].val[1] = '\0';

                if (curr == '(') tokens[token_count].type = oBrace;
                else if (curr == ')') tokens[token_count].type = cBrace;
                else if (curr == '[') tokens[token_count].type = oSBrace;
                else if (curr == ']') tokens[token_count].type = cSBrace;
                
                token_count++;
            }
            
            // Multi-Character Tokens (Numbers and Words/Keywords)
            else if (isdigit(curr) || (curr == '-' && isdigit(command[j+1]))) {
                // Start of a number
                int k = j;
                // Handle optional sign
                if (command[k] == '-') k++; 
                // Read digits
                int start = k;
                while (k < len && isdigit(command[k])) {
                    k++;
                }
                
                // Extract the substring
                int sub_len = k - j;
                tokens[token_count].val = (char*)malloc(sub_len + 1);
                strncpy(tokens[token_count].val, command + j, sub_len);
                tokens[token_count].val[sub_len] = '\0';
                
                tokens[token_count].type = Num;
                token_count++;
                j = k - 1; 
            }
            else if (isalpha(curr) || curr == '_') {
                // Start of a word
                int k = j;
                while (k < len && (isalnum(command[k]) || command[k] == '_')) {
                    k++;
                }
                
                // Extract the substring
                int sub_len = k - j;
                char* word_str = (char*)malloc(sub_len + 1);
                strncpy(word_str, command + j, sub_len);
                word_str[sub_len] = '\0';
                tokens[token_count].val = word_str;
                
                // Check if it's reserved
                if (is_reserved(word_str)){
                    tokens[token_count].type = Keyw;
                }
                else {
                    tokens[token_count].type = funcName;
                    // printf("asdfjsdfjal");
                }
                
                token_count++;
                j = k - 1;
            }
            else {
                 fprintf(stderr, "Error: Unexpected character '%c' in command '%s'\n", curr, command);
            }

            // Dynamic memory re-allocation check
            if (token_count >= max_tokens) {
                max_tokens += 10;
                struct Token* new_tokens = (struct Token*)realloc(tokens, sizeof(struct Token) * max_tokens);
                if (new_tokens == NULL) {
                    perror("realloc failed for tokens");
                    free(split_col); // Free command array
                    return NULL; 
                }
                tokens = new_tokens;
            }
        }
        
        if (!comment){
            // Add the Colon token (which split_string removed)
            tokens[token_count].type = Colon;
            tokens[token_count].val = strdup(";");
            token_count++;
        }
        comment = 0;

        // Realloc check after adding Colon token
        if (token_count >= max_tokens) {
            max_tokens += 10;
            struct Token* new_tokens = (struct Token*)realloc(tokens, sizeof(struct Token) * max_tokens);
            if (new_tokens == NULL) {
                perror("realloc failed after colon");
                free_tokens(tokens);
                free(split_col);
                return NULL; 
            }
            tokens = new_tokens;
        }

        // Free the command string duplicated by split_string
        free(split_col[i]); 
    }
    
    // Free the array of command pointers
    free(split_col); 

    // Null-terminate the token array with the sentinel value
    if (token_count < max_tokens) {
        tokens[token_count].type = TOKEN_END; 
        tokens[token_count].val = NULL;
    }
    
    return tokens;
}

// memory cleanup
void free_tokens(struct Token* tokens) {
    if (tokens == NULL) return;
    /* for (int i = 0; tokens[i].type != TOKEN_END; i++) {
        free(tokens[i].val); // Free the duplicated string value
    }*/
    free(tokens); // Free the token array itself
}

int test(void){
    // Test input string
    char test_line[] = "#comment;";
    int line_size = (int)strlen(test_line);

    printf("--- Tokenizing Test ---\n");
    printf("Input: \"%s\"\n\n", test_line);

    // Call the tokenizer
    struct Token* t = tokenize_line(test_line, line_size);

    if (t == NULL) {
        fprintf(stderr, "Tokenization failed.\n");
        return EXIT_FAILURE;
    }

    // Print the resulting tokens
    printf("| %-4s | %-10s | %-10s |\n", "Idx", "Type", "Value");
    printf("|------|------------|------------|\n");
    
    int i = 0;
    while (t[i].type != TOKEN_END && t[i].val != NULL) {
        printf("| %-4d | %-10s | %-10s |\n", 
               i, 
               get_token_type_name(t[i].type), 
               t[i].val);
        i++;
    }
    printf("| %-4d | %-10s | %-10s |\n", i, "END", "NULL");

    printf("\nTokenization complete. Total tokens: %d\n", i);
    
    free_tokens(t);
    printf("Memory freed successfully.\n");

    return EXIT_SUCCESS;
}
