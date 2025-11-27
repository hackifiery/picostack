#pragma once
#include "stack.h"

void interpret(struct Stack *stack, const char *commands, size_t *pc);
void preprocess_program(const char *src, char **cleaned_out, int **map_out);