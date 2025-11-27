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
void print_stack(struct Stack *stack);
