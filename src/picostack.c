/*
* Copyright (c) 2025 
* hackifiery. All rights reserved.
* All code licensed under the MIT License.
*/

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "interpreter.h"
#include "stack.h"


void cli(){
    printf("Welcome to PicoStack. Enter commands (or 'exit' to quit)\n");
    struct Stack stack;
    init_stack(&stack);

    // Persistent program buffer
    char *program = NULL;
    size_t program_len = 0;
    size_t program_cap = 0;
    size_t pc = 0;  // Persistent program counter

    char input[1024];

    while (1) {
        printf("\n> ");
        if (!fgets(input, sizeof(input), stdin)) {
            break; // EOF
        }

        // Check for 'exit'
        if (strncmp(input, "exit", 4) == 0) {
            break;
        }
        if (strncmp(input, "reset", 5) == 0) {
            // 1. Reset stack
            free(stack.arr);
            init_stack(&stack);

            // 2. Reset persistent program
            free(program);
            program = NULL;
            program_len = 0;
            program_cap = 0;

            // 3. Reset PC
            pc = 0;

            printf("[State reset]\n");
            continue;
        }

        // Preprocess input to remove comments and whitespace
        char *cleaned = NULL;
        preprocess_program(input, &cleaned);
        size_t new_len = strlen(cleaned);

        // Append new code to persistent program buffer
        if (program_len + new_len + 1 > program_cap) {
            program_cap = (program_len + new_len + 1) * 2;
            program = realloc(program, program_cap);
        }
        memcpy(program + program_len, cleaned, new_len);
        program_len += new_len;
        program[program_len] = '\0';

        free(cleaned);

        // Interpret using persistent program and PC
        interpret(&stack, program, &pc);
    }

    free(stack.arr);
    free(program);
}

int main(int argc, char** argv) {

    if (argc == 2) {
        // --- Run file mode ---
        const char* filename = argv[1];
        FILE* fp = fopen(filename, "r");
        if (!fp) {
            fprintf(stderr, "Error: Cannot open file '%s'\n", filename);
            return 1;
        }
        // Load entire file
        fseek(fp, 0, SEEK_END);
        long size = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        char* buffer = malloc(size + 1);
        fread(buffer, 1, size, fp);
        buffer[size] = '\0';
        fclose(fp);

        // Preprocess file contents
        char* cleaned = NULL;
        preprocess_program(buffer, &cleaned);
        free(buffer);
        // Run interpreter with fresh stack and pc=0
        struct Stack stack;
        init_stack(&stack);
        size_t pc = 0;

        interpret(&stack, cleaned, &pc);

        free(stack.arr);
        free(cleaned);
        return 0;
    }

    // --- No file provided = start CLI ---
    cli();
    return 0;
}
