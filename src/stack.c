/*
* Copyright (c) 2025 
* hackifiery. All rights reserved.
* All code licensed under the MIT License.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "stack.h"

void init_stack(struct Stack *stack){
    stack->arr = (int *)calloc(1, sizeof(int));
    stack->top = -1;
}

// Push value onto stack
void push_stack(struct Stack *stack, int val){
    stack->top++;
    stack->arr = (int *)realloc(stack->arr, (stack->top + 1) * sizeof(int));
    stack->arr[stack->top] = val;
}

// Helper for popping value from stack
int pop_stack(struct Stack *stack){
    if (stack->top < 0) {
        fprintf(stderr, "Error: Attempt to pop from empty stack.\n");
        exit(EXIT_FAILURE);
    }
    int a = stack->arr[stack->top];
    stack->arr = (int *)realloc(stack->arr, stack->top * sizeof(int));
    stack->top--;
    return a;
}

// Duplicate top value on stack
void dup_stack(struct Stack *stack){
    int a = pop_stack(stack);
    push_stack(stack, a);
    push_stack(stack, a);
}

// Swap top 2 vals on stack
void swap_stack(struct Stack *stack){
    int a = pop_stack(stack);
    int b = pop_stack(stack);
    push_stack(stack, a);
    push_stack(stack, b);
}
// reverses whole stack
void reverse_stack(struct Stack *stack) {
    int start = 0;
    int end = stack->top;
    while (start < end) {
        int temp = stack->arr[start];
        stack->arr[start] = stack->arr[end];
        stack->arr[end] = temp;
        start++;
        end--;
    }
}

// discard top val on stack
void discard_stack(struct Stack *stack) {
    if (stack->top == 0) {
        exit(EXIT_FAILURE);
    }
    stack->arr = (int *)realloc(stack->arr, stack->top * sizeof(int));
    stack->top--;
}

// debug
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

// pops and adds top 2 vals & pushes result
void add_stack(struct Stack *stack) {
    if (stack->top < 1) {
        fprintf(stderr, "Error: Stack needs at least 2 items for ADD.\n");
        return;
    }
    int b = pop_stack(stack);
    int a = pop_stack(stack);
    push_stack(stack, a + b);
}

// pops and subtracts top 2 vals & pushes result
void sub_stack(struct Stack *stack) {
    if (stack->top < 1) {
        fprintf(stderr, "Error: Stack needs at least 2 items for SUBTRACT.\n");
        return;
    }
    int b = pop_stack(stack);
    int a = pop_stack(stack);
    push_stack(stack, a - b);
}

// jump to address (top val) if test val is 0 (2nd to top val)
void execute_jump(struct Stack *stack, size_t *pc, const char *program_start) {
    if (stack->top < 1) {
        fprintf(stderr, "Error: Stack needs 2 items (Address and Test Value) for JUMP.\n");
        return;
    }

    int target_address = pop_stack(stack);
    int test_value = pop_stack(stack);

    size_t program_length = strlen(program_start);

    if (test_value == 0) {
        if (target_address < 0 || target_address >= (int)program_length) {
            fprintf(stderr, "Error: Invalid jump address (%d).\n", target_address);
            *pc = program_length; // jump to end
            return;
        }
        *pc = (size_t)target_address;
    } else {
        (*pc)++;
    }
}

// output top val as char
void out_stack(struct Stack *stack) {
    char output_char = (char)pop_stack(stack);
    putchar(output_char);
}

// output top val as number
void out_int_stack(struct Stack *stack) {
    int output_int = pop_stack(stack);
    printf("%d", output_int);
}

// reads input string and pushes ascii to stack (max 64 bytes)
void in_stack(struct Stack *stack) { 
    char buf[64];
    scanf("%63s", buf);
    for (size_t i = 0; i < strlen(buf); i++) {
        push_stack(stack, (int)buf[i]);
    }
    return;
}

// reads input integer and pushes to stack
void in_int_stack(struct Stack *stack) {
    int val;
    scanf("%d", &val);
    push_stack(stack, val);
}

// rotate right the top n items right in the stack (top -> nth item)
void rot_right_stack(struct Stack *stack, int n) {
    if (n <= 0 || stack->top < n - 1) { 
        return; 
    }

    int boundary_idx = stack->top - n;
    int target_idx = boundary_idx + 1; 
    int moved_element = stack->arr[stack->top];

    for (int i = stack->top - 1; i >= target_idx; i--) {
        stack->arr[i + 1] = stack->arr[i];
    }

    stack->arr[target_idx] = moved_element;
}

// rotate left the top n items left in the stack (nth item -> top)
void rot_left_stack(struct Stack *stack, int n) {
    if (n <= 1 || stack->top < n - 1) {
        return;  // nothing to rotate
    }

    int start = stack->top - n + 1; // beginning of slice
    int first = stack->arr[start]; // first element to move left

    for (int i = start; i < stack->top; i++) {
        stack->arr[i] = stack->arr[i + 1];
    }

    stack->arr[stack->top] = first;
}