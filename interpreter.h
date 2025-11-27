/*
* Copyright (c) 2025 
* hackifiery. All rights reserved.
* All code licensed under the MIT License.
*/

#pragma once
#include "stack.h"

void interpret(struct Stack *stack, const char *commands, size_t *pc);
void preprocess_program(const char *src, char **cleaned_out);