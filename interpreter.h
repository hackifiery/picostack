#pragma once

#include <stdlib.h>
#include "stack.h"

typedef struct {
    struct Stack file_stk; // all numbers pointing to indexes in paths (since our struct Stack doesn't support strings & im lazy to reimplement it)
    char** paths;
}callStack;

callStack init_callStack(void) {
    callStack cs;
    init_stack(&cs.file_stk);
    cs.paths = NULL;
    return cs;
}

typedef enum {
    NORMAL,
    INCLUDE_CALL,
    FUNCTION_CALL
} interpStatus;

interpStatus interpret_line(
    Call code,
    int* ip, // note: manipulated by the runner, not this
    Function** functions,
    bool* in_function,
    int* function_count,
    Function** curr_function,
    struct Stack* stack,
    const char** include_paths,
    char** curr_file,
    callStack* call_stk
);

void run_str(
    char** code,
    struct Stack* stack,
    const int in_size,
    const char* fn,
    Function** functions,
    bool* in_function,
    int* function_count,
    Function** curr_function,
    const char** include_paths,
    callStack* cs
);