#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "stack.h"

void init_stack(struct Stack *stack){
    stack->arr = (int *)calloc(1, sizeof(int));
    stack->top = -1;
}

void push_stack(struct Stack *stack, int val){
    stack->top++;
    stack->arr = (int *)realloc(stack->arr, (stack->top + 1) * sizeof(int));
    stack->arr[stack->top] = val;
}

int pop_stack(struct Stack *stack){
    int a = stack->arr[stack->top];
    stack->arr = (int *)realloc(stack->arr, stack->top * sizeof(int));
    stack->top--;
    return a;
}

void dup_stack(struct Stack *stack){
    // start: {a}
    int a = pop_stack(stack); // {}
    push_stack(stack, a); // {a}
    push_stack(stack, a); // {a}
}

void swap_stack(struct Stack *stack){
    // start: {a b}
    int a = pop_stack(stack); // {b}
    int b = pop_stack(stack); // {}
    push_stack(stack, b); // {b}
    push_stack(stack, a); // {b a}
}

void discard_stack(struct Stack *stack) {
    stack->arr = (int *)realloc(stack->arr, stack->top * sizeof(int));
    stack->top--;
}

// Function to print the stack state for debugging
void print_stack(struct Stack *stack, const char *msg) {
    printf("--- %s ---\n", msg);
    if (stack->top == -1) {
        printf("Stack: [] (Empty)\n");
    } else {
        printf("Stack: [");
        for (int i = 0; i <= stack->top; i++) {
            printf("%d", stack->arr[i]);
            if (i < stack->top) {
                printf(", ");
            }
        }
        printf("] (Top: %d)\n", stack->arr[stack->top]);
    }
}

void add_stack(struct Stack *stack) {
    if (stack->top < 1) {
        fprintf(stderr, "Error: Stack needs at least 2 items for ADD.\n");
        return;
    }
    // Pop b, then a. Result is a + b.
    int b = pop_stack(stack);
    int a = pop_stack(stack);
    push_stack(stack, a + b);
}

void sub_stack(struct Stack *stack) {
    if (stack->top < 1) {
        fprintf(stderr, "Error: Stack needs at least 2 items for SUBTRACT.\n");
        return;
    }
    // Pop b, then a. Result is a - b.
    int b = pop_stack(stack);
    int a = pop_stack(stack);
    push_stack(stack, a - b);
}

void execute_jump(struct Stack *stack, const char **program_counter, const char *program_start) {
    if (stack->top < 1) {
        fprintf(stderr, "Error: Stack needs 2 items (Address and Test Value) for JUMP.\n");
        return;
    }
    
    // 1. Pop the Test Value (b) and Target Address (a)
    int test_value = pop_stack(stack); // b
    int target_address = pop_stack(stack); // a
    
    // 2. Check the jump condition: Jump if test_value (b) is 0
    if (test_value == 0) {
        // Condition met
    
        // Safety check for target address validity
        int program_length = strlen(program_start);
        if (target_address < 0 || target_address >= program_length) {
            fprintf(stderr, "Error: Invalid jump address (%d).\n", target_address);
            // Halt execution or set program_counter to end of string
            *program_counter = program_start + program_length; 
            return;
        }
        
        *program_counter = program_start + target_address;
        
    } else {
        // Condition NOT met: Continue sequentially.
        
        // We must manually advance the counter because the main loop's 'cmd++' 
        // is skipped when 'continue' is used for the 'j' case.
        (*program_counter)++; 
    }
}

void out_stack(struct Stack *stack) {
    char output_char = (char)pop_stack(stack);
    putchar(output_char);
}