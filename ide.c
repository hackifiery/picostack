#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "interpreter.h"
#include "stack.h"

// hello world: p33p100p108p114p111p87p32p111p108p108p101p72oooooooooooo

void ide(){
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

        // Preprocess input to remove comments and whitespace
        char *cleaned = NULL;
        int *map = NULL;
        preprocess_program(input, &cleaned, &map);
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
        free(map);

        // Interpret using persistent program and PC
        interpret(&stack, program, &pc);
    }

    free(stack.arr);
    free(program);
}


int main() {
    ide();
    return 0;
}
