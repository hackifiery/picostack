#include <stdbool.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>
#include <stdlib.h>

#include "helpers.h"
#include "lexer.h"

// LEXER.C


bool isInt(const char* str) {
    char* endptr;
    long val;
    if (str == NULL || *str == '\0') return false;

    errno = 0;
    val = strtol(str, &endptr, 10);

    if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN)) || (errno != 0 && val == 0)) {
        return false;
    }
    if (endptr == str) return false;

    while (*endptr != '\0') {
        if (!isspace((unsigned char)*endptr)) return false;
        endptr++;
    }
    return true;
}


char* safe_realloc(char* original_ptr, size_t new_size) {
    char* temp_ptr = (char*)realloc(original_ptr, new_size);
    if (temp_ptr == NULL) {
        fprintf(stderr, "Error: Memory reallocation failed!\n");
        free(original_ptr);
        exit(EXIT_FAILURE);
    }
    return temp_ptr;
}

bool isKeyw(char* c) {
    for (int i = 0; i < 4; i++) { // Using 4 for the 4 keywords defined
        if (strcmp(c, Funcs[i]) == 0) return true;
    }
    return false;
}


void strip_char(char* str, const char stripval) {
    if (str == NULL) return NULL;

    char* read = str;
    char* write = str;

    while (*read != '\0') {
        if (*read != stripval) {
            *write = *read;
            write++;
        }
        read++;
    }
    *write = '\0'; // Null-terminate the "new" end of the string
}


void process_str_tok(char* str) {
    if (str == NULL) return;
    size_t len = strlen(str);
    if (len < 2) return;

    // Check if wrapped in matching quotes
    bool double_quoted = (str[0] == '"' && str[len - 1] == '"');
    bool single_quoted = (str[0] == '\'' && str[len - 1] == '\'');

    if (double_quoted || single_quoted) {
        char quote_char = str[0];
        char* read = str + 1; // Skip start quote
        char* write = str;

        while (*read != '\0') {
            // Stop if we hit the closing quote at the very end
            if (*read == quote_char && *(read + 1) == '\0') {
                break;
            }

            // Handle escapes: if we see \, write the next char instead
            if (*read == '\\' && (*(read + 1) == '"' || *(read + 1) == '\'' || *(read + 1) == '\\')) {
                read++;
            }

            *write = *read;
            write++;
            read++;
        }
        *write = '\0';
    }
}

char** split_str(const char* str, const char* delims, int* count_out) {
    if (!str || !delims || !count_out) return NULL;

    char** tokens = NULL;
    int capacity = 10;
    int count = 0;

    tokens = malloc(capacity * sizeof(char*));
    if (!tokens) return NULL;

    const char* start = str;
    const char* current = str;
    bool in_string = false;
    bool escape = false;

    while (true) {
        if (!escape && *current == '\\' && in_string) escape = true;
        else {
            if (!escape && (*current == '"' || *current == '\'')) in_string = !in_string;
            escape = false;
        }

        if (!in_string && (strchr(delims, *current) || *current == '\0')) {
            size_t len = current - start;
            if (len > 0) {
                if (count >= capacity - 1) {
                    capacity *= 2;
                    char** temp = realloc(tokens, capacity * sizeof(char*));
                    if (!temp) goto error_cleanup;
                    tokens = temp;
                }
                tokens[count] = malloc(len + 1);
                if (!tokens[count]) goto error_cleanup;

                strncpy(tokens[count], start, len);
                tokens[count][len] = '\0';
                count++;
            }
            if (*current == '\0') break;
            start = current + 1;
        }
        current++;
    }

    tokens[count] = NULL; // null-termination
    *count_out = count;
    // printf("after lexer split: %s\n", strdup(tokens));
    return tokens;

error_cleanup:
    for (int i = 0; i < count; i++) free(tokens[i]);
    free(tokens);
    return NULL;
}


bool isString(char* str) {
    int c = 0;
    while (!(str[c] == '\0')) c++;
    return (str[0] == '"' && str[c - 1] == '"') || (str[0] == '\'' && str[c - 1] == '\'');
}


// PARSER.C


// note: INCLUDES NULL CHARACTER!!!
int* string_to_params(char* str, int* out_len) {
    *out_len = (int)strlen(str) + 1; // add 1 to include the \0
    int* out = (int*)calloc(*out_len, sizeof(int));
    char* curr = str;
    if (!out) return NULL;
    for (int i = 0; *curr != '\0'; curr++, i++) out[i] = (int)(*curr);
    return out;
}

void insert_int_array(int** arr, int* arr_len, const int* insert, const int insert_len, int index) {
    if (index < 0 || index > *arr_len || insert_len <= 0) {
        return;
    }

    int new_len = *arr_len + insert_len;
    *arr = realloc(*arr, new_len * sizeof(int));
    if (!*arr) {
        perror("realloc");
        exit(EXIT_FAILURE);
    }

    // Move tail to make space
    memmove(*arr + index + insert_len, *arr + index, (*arr_len - index) * sizeof(int));

    // Copy inserted data
    memcpy(*arr + index, insert, insert_len * sizeof(int));

    *arr_len = new_len;
}

int* slice_int_array(const int* arr, int arr_len, int start, int end, int* out_len) {
    // Handle NULL inputs
    if (arr == NULL || out_len == NULL) {
        return NULL;
    }

    // Handle negative end index (means "to end of array")
    if (end < 0) {
        end = arr_len;
    }

    // Validate indices
    if (start < 0 || start > arr_len || end < start || end > arr_len) {
        *out_len = 0;
        return NULL;
    }

    // Calculate slice length
    int slice_len = end - start;
    *out_len = slice_len;

    // Handle empty slice
    if (slice_len == 0) {
        return NULL;
    }

    // Allocate new array
    int* result = (int*)malloc(slice_len * sizeof(int));
    if (result == NULL) {
        *out_len = 0;
        return NULL;
    }

    // Copy the slice
    memcpy(result, arr + start, slice_len * sizeof(int));

    return result;
}

// INTERPRETER.C
char** split_lines(const char* src, int* out_count) {
    char* buf = strdup(src);
    if (!buf) exit(EXIT_FAILURE);

    int cap = 8;
    int count = 0;
    char** lines = malloc(cap * sizeof(char*));
    if (!lines) exit(EXIT_FAILURE);

    char* tok = strtok(buf, "\n");
    while (tok) {
        if (count == cap) {
            cap *= 2;
            lines = realloc(lines, cap * sizeof(char*));
            if (!lines) exit(EXIT_FAILURE);
        }
        lines[count++] = strdup(tok);
        tok = strtok(NULL, "\n");
    }

    free(buf);
    *out_count = count;
    return lines;
}
