#include <stdlib.h>
#include <stdio.h>    // Include for debugging/error handling
#include <ctype.h>    // For tolower

#include "interpreter.h"
#include "stack.h"

void interpret(struct Stack *stack, const char *commands) {
    const char *cmd = commands;
    char debug_msg[64];
    // We use a manual loop structure here because the 'p' case changes 'cmd' directly.
    while (*cmd != '\0') {
        
        // 1. Convert to lowercase and process the command character
        switch (tolower(*cmd)) {
            
            // P: Push Literal (Reads an integer following 'p')
            case 'p': { 
                cmd++; // Move past 'p'
                
                // Skip leading whitespace/separators
                while (*cmd != '\0' && isspace(*cmd)) {
                    cmd++;
                }
                
                int val = 0;
                
                // Read all subsequent digits until a non-digit is found
                while (*cmd >= '0' && *cmd <= '9') {
                    val = val * 10 + (*cmd - '0');
                    cmd++;
                }
                
                push_stack(stack, val);
                continue; 
            }
            
            // A: Add (a b -> a+b)
            case 'a': 
                add_stack(stack);
                break;
                
            // S: Subtract (a b -> a-b)
            case 's': 
                sub_stack(stack);
                break;
            
            // J: Jump (if top == 0, jump to address @ top after pop)
            case 'j': 
                execute_jump(stack, &cmd, commands);
                // snprintf(debug_msg, sizeof(debug_msg), "PC %zu: JUMP (%c)", cmd - commands, *(cmd - 1));
                // print_stack(stack, debug_msg);
                // execute_jump modifies cmd, so we handle the loop control inside it.
                continue;
            
            // D: Duplicate (a -> a a)
            case 'd': 
                dup_stack(stack);
                break;
                
            // W: Swap (a b -> b a)
            case 'w': 
                swap_stack(stack);
                break;
                
            // X: Pop/Discard (a -> )
            case 'x': 
                discard_stack(stack);
                break;
            
            // O: Output (a -> )
            case 'o': 
                out_stack(stack);
                break;
            
            default:
                // Ignore unknown characters (e.g., spaces, comments, illegal commands)
                break;
        }
        
        // 2. Advance the program counter to the next command
        cmd++;
        // snprintf(debug_msg, sizeof(debug_msg), "PC %zu: %c", cmd - commands, *(cmd - 1));
        // print_stack(stack, debug_msg);
    }
}
