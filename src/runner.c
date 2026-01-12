#include <stdlib.h>
#include <stdio.h>
#include "parser.h"
#include "interpreter.h"
#include "helpers.h"

void run_file(
    const char* filename,
    const char** include_paths
) {
    char* text = read_file(filename);
    if (!text) {
        fprintf(stderr, "failed to read %s\n", filename);
        exit(1);
    }

    int line_count = 0;
    char** lines = split_lines(text, &line_count);

    struct Stack stack;
    init_stack(&stack);

    callStack cs = init_callStack();

    Function* functions = NULL;
    Function* curr_function = NULL;
    bool in_function = false;
    int function_count = 0;

    run_str(
        lines,
        &stack,
        line_count,
        filename,
        &functions,
        &in_function,
        &function_count,
        &curr_function,
        include_paths,
        &cs
    );

    //cleanup

    for (int i = 0; i < line_count; i++) {
        free(lines[i]);
    }
    free(lines);
    free(text);

    for (int i = 0; i < function_count; i++) {
        free(functions[i].name);
        free(functions[i].fname);
    }
    free(functions);

    free(stack.arr);

    // call stack should be empty now
    free(cs.paths);
}

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "usage: %s <file.pcs>\n", argv[0]);
        return 1;
    }

    const char* include_paths[] = {
        "./include/",
        "/usr/local/include",
        NULL
    };

    run_file(argv[1], include_paths);
    return 0;
}