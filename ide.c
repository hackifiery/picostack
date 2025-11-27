#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "interpreter.h"
#include "stack.h"
// hello world: p33p100p108p114p111p87p32p111p108p108p101p72oooooooooooo
void ide(){
    printf("Welcome to PicoStack IDE!\n");
    printf("This is a simple stack-based interpreter written in c.\n");
    printf("Available commands:\n");
    printf(" p <num> : Push literal number onto the stack\n");
    printf(" a       : Add top two numbers on the stack\n");
    printf(" s       : Subtract top two numbers on the stack\n");
    printf(" j       : Jump to address if top of stack is 0\n");
    printf(" d       : Duplicate top number on the stack\n");
    printf(" w       : Swap top two numbers\n");
    printf(" o       : Output top number as character\n");
    printf(" x       : Discard top number on the stack\n");
    printf(" Enjoy coding with PicoStack!\n");

    struct Stack stack; 
    init_stack(&stack);
    char input[1024];

    while (1) {
        printf("\nEnter commands (or 'exit' to quit): ");
        if (!fgets(input, sizeof(input), stdin)) {
            break; // EOF
        }
        
        // Check for 'exit'
        if (strncmp(input, "exit", 4) == 0) {
            free(stack.arr);
            break; 
        }
        interpret(&stack, input);
    }
}

int main() {
    ide();
    return 0;
}