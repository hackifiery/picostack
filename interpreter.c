#include <stdlib.h>
#include <stdbool.h>
#include <fstream>
#include <sys/stat.h>

#include "parser.h"
#include "stack.h"

#define _CRT_SECURE_NO_WARNINGS

#define cmd(x) else if (strcmp(func,x)==0)
#define loop_params(i) for (int i = 0; i < params_len; i++)
#define picostack_ext ".pcs"

// ip, in_function, function_count, and curr_function must be refs, functions must be malloc'ed
void interpret_line(Call code, const int in_size, int* ip, Function* functions, bool* in_function, int* function_count, Function* curr_function, struct Stack* stack, const char** include_paths) {
	// if (!(code.func.isExtern)) exit(EXIT_FAILURE);
	if (*in_function && strcmp(code.func.name, "endfunc") != 0) return; // don't run stuff in function defs, unless it ends the function

	char* func = code.func.name;
	int* params = code.params;
	int params_len = code.params_len;
	char* params_char = code.params_char;
	bool isExtern = code.func.isExtern;

	if (false); // so that the else if macros work
	cmd("startfunc") {
		(*function_count)++;
		functions = (Function*)realloc(functions, *function_count);
		functions[(*function_count) - 1].name = params[0];
		functions[(*function_count) - 1].isExtern = false;
		functions[(*function_count) - 1].paramCount = -1; // inf # of params (will be changed if a parameter spec is present)
		functions[(*function_count) - 1].start = *(ip)+1;
		*in_function = true;
		*curr_function = functions[(*function_count) - 1];
	}
	cmd("push") {
		loop_params(i) {
			push_stack(stack, i);
		}
	}
	cmd("discard") {
		discard_stack(stack);
	}
}