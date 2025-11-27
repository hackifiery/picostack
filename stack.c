#include <stdlib.h>
#include <stdio.h>

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
void print_stack(struct Stack *stack) {
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
