#pragma once
struct Stack {
    int *arr;
    int top;
};

void init_stack(struct Stack *stack);
void push_stack(struct Stack *stack, int val);
int pop_stack(struct Stack *stack);
void dup_stack(struct Stack *stack);
void swap_stack(struct Stack *stack);
void discard_stack(struct Stack *stack);
void print_stack(struct Stack *stack, const char *msg);
void add_stack(struct Stack *stack);
void sub_stack(struct Stack *stack);
void execute_jump(struct Stack *stack, const char **program_counter, const char *program_start);
void out_stack(struct Stack *stack);