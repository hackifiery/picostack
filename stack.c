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
    if (stack->top < 0) {
        fprintf(stderr, "Error: Attempt to pop from empty stack.\n");
        exit(EXIT_FAILURE);
    }
    int a = stack->arr[stack->top];
    stack->arr = (int *)realloc(stack->arr, stack->top * sizeof(int));
    stack->top--;
    return a;
}

void dup_stack(struct Stack *stack){
    int a = pop_stack(stack);
    push_stack(stack, a);
    push_stack(stack, a);
}

void swap_stack(struct Stack *stack){
    int a = pop_stack(stack);
    int b = pop_stack(stack);
    push_stack(stack, b);
    push_stack(stack, a);
}

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

void discard_stack(struct Stack *stack) {
    stack->arr = (int *)realloc(stack->arr, stack->top * sizeof(int));
    stack->top--;
}

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
    int b = pop_stack(stack);
    int a = pop_stack(stack);
    push_stack(stack, a + b);
}

void sub_stack(struct Stack *stack) {
    if (stack->top < 1) {
        fprintf(stderr, "Error: Stack needs at least 2 items for SUBTRACT.\n");
        return;
    }
    int b = pop_stack(stack);
    int a = pop_stack(stack);
    push_stack(stack, a - b);
}

void execute_jump(struct Stack *stack, size_t *pc, const char *program_start) {
    if (stack->top < 1) {
        fprintf(stderr, "Error: Stack needs 2 items (Address and Test Value) for JUMP.\n");
        return;
    }

    int test_value = pop_stack(stack);
    int target_address = pop_stack(stack);

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

void out_stack(struct Stack *stack) {
    char output_char = (char)pop_stack(stack);
    putchar(output_char);
}

void in_stack(struct Stack *stack) { 
    char buf[64];
    scanf("%63s", buf);
    for (size_t i = 0; i < strlen(buf); i++) {
        push_stack(stack, (int)buf[i]);
    }
    return;
}