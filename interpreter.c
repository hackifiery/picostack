#include <stdlib.h>

#include "interpreter.h"
#include "stack.h"

void interpret(struct Stack *stack, const char *commands) {
    for (const char *cmd = commands; *cmd != '\0'; cmd++) {
        switch (tolower(*cmd)) {
            case 'p': { // Push
                cmd++;
                int val = 0;
                while (*cmd >= '0' && *cmd <= '9') {
                    val = val * 10 + (*cmd - '0');
                    cmd++;
                }
                cmd--; // Adjust for the increment in the for loop
                push_stack(stack, val);
                break;
            }
            case 'd': // Duplicate
                dup_stack(stack);
                break;
            case 's': // Swap
                swap_stack(stack);
                break;
            case 'x': // Discard
                discard_stack(stack);
                break;
            default:
                // Ignore unknown commands
                break;
        }
    }
}