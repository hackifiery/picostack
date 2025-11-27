#include <stdlib.h>
#include <stdio.h>    // Include for debugging/error handling
#include <ctype.h>    // For tolower
#include <string.h>

#include "interpreter.h"
#include "stack.h"

void preprocess_program(const char* src, char** cleaned_out) {
    size_t len = strlen(src);

    char* cleaned = malloc(len + 1);
    size_t w = 0; // write index

    for (size_t r = 0; r < len; r++) {

        // SKIP comments starting with '#'
        if (src[r] == '#') {
            while (r < len && src[r] != '\n') {
                r++;
            }
            continue;
        }

        // Skip whitespace
        if (isspace(src[r])) {
            continue;
        }

        // Keep everything else (commands)
        cleaned[w++] = src[r];
    }

    cleaned[w] = '\0';
    *cleaned_out = cleaned;
}


void interpret(struct Stack* stack, const char* commands, size_t* pc) {
    char debug_msg[64];
    size_t len = strlen(commands);

    while (*pc < len) {
        char cmd = tolower(commands[*pc]);

		// multi digit number handling
        if (isdigit(cmd)) {
            int value = 0;

            // Read full integer
            while (*pc < len && isdigit(commands[*pc])) {
                value = value * 10 + (commands[*pc] - '0');
                (*pc)++;
            }

            push_stack(stack, value);
            continue;   // IMPORTANT: skip (*pc)++ at end
        }

        switch (cmd) {
        case 'p': {
            (*pc)++;
            int val = 0;
            while (*pc < len && isdigit(commands[*pc])) {
                val = val * 10 + (commands[*pc] - '0');
                (*pc)++;
            }
            push_stack(stack, val);
            continue; // skip (*pc)++ at end
        }

        case 'a':
            add_stack(stack);
            break;

        case 's':
            sub_stack(stack);
            break;

        case 'j':
            execute_jump(stack, pc, commands);
            continue;

        case 'd':
            dup_stack(stack);
            break;

        case 'w':
            swap_stack(stack);
            break;

        case 'x':
            discard_stack(stack);
            break;

        case 'o':
            out_stack(stack);
            break;
        
        case 'n':
            out_int_stack(stack);
            break;

        case 'i':
            in_stack(stack);
            break;

        case 'r':
            reverse_stack(stack);
            break;

        case 'z':
            print_stack(stack, "Debug Stack:");
            break;

        case 'c':
            while (stack->top != -1) {
                discard_stack(stack);
            }
            break;

        default:
            break;
        }

        (*pc)++;
    }
}
