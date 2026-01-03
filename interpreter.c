#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "parser.h"
#include "stack.h"
#include "helpers.h"

#define cmd(x) else if (strcmp(func, x) == 0)
#define loop_params(i) for (int i = 0; i < params_len; i++)
#define ic (*ip)++
#define trace(...) fprintf(stderr, __VA_ARGS__)


typedef struct {
    struct Stack ip_stk;
    struct Stack file_stk; // all numbers pointing to indexes in paths (since our struct Stack doesn't support strings & im lazy to reimplement it)
    char** paths;
}callStack;

callStack init_callStack(void) {
    callStack cs;
    init_stack(&cs.ip_stk);
    init_stack(&cs.file_stk);
    cs.paths = NULL;
    return cs;
}


void interpret_line(
    Call code,
    const int in_size,
    int* ip,
    Function** functions,
    bool* in_function,
    int* function_count,
    Function** curr_function,
    struct Stack* stack,
    const char** include_paths,
    char** curr_file,
    callStack* call_stk
) {
    trace("\n=== interpret_line BEGIN ===\n");

    if (call_stk->ip_stk.arr == NULL) {
        trace("[init] first run, initializing call stack\n");
        trace("[init] file=%s ip=%d\n", *curr_file, *ip);

        // push a new file index into the file stack
        push_stack(&call_stk->file_stk, call_stk->file_stk.top + 1);

        // ensure paths array is large enough
        int path_idx = call_stk->file_stk.top; // top after push
        call_stk->paths = realloc(call_stk->paths, (path_idx + 1) * sizeof(char*));
        if (!call_stk->paths) {
            fprintf(stderr, "Memory allocation failed for paths\n");
            exit(EXIT_FAILURE);
        }

        // store a copy of the file name
        call_stk->paths[path_idx] = strdup(*curr_file);
    }
    trace("[ip] current ip = %d\n", *ip);
    trace("[call] func='%s'\n", code.func.name);
    trace("[call] params_len=%d\n", code.params_len);

    if (code.params_char) {
        trace("[call] params_char='%s'\n", code.params_char);
    }

    // Skip execution inside func
    if (*in_function && strcmp(code.func.name, "endfunc") != 0) {
        trace("[skip] inside function definition\n");
        trace("[skip] ignoring '%s'\n", code.func.name);
        (*ip)++;
        trace("[ip] advanced to %d\n", *ip);
        trace("=== interpret_line END (skipped) ===\n");
        return;
    }

    char* func = code.func.name;
    int* params = code.params;
    int params_len = code.params_len;
    char* params_char = code.params_char;

    if (false); // dummy for macros

    /* ==========================
       startfunc
    ========================== */
    cmd("startfunc") {
        trace("[exec] startfunc\n");

        (*function_count)++;
        trace("[func] function_count -> %d\n", *function_count);

        Function* tmp =
            (Function*)realloc(*functions,
                               (*function_count) * sizeof(Function));

        if (!tmp) {
            trace("[fatal] realloc failed\n");
            exit(EXIT_FAILURE);
        }

        *functions = tmp;

        Function* f = &(*functions)[(*function_count) - 1];

        f->name = strdup(params_char);
        f->isExtern = false;
        f->fname = strdup(*curr_file);
        f->paramCount = -1;
        f->start = *ip + 1;
        f->end = -1;

        trace("[func] defined function '%s'\n", f->name);
        trace("[func] start ip=%d, file=%s\n", f->start, strdup(curr_file));

        *in_function = true;
        *curr_function = f;

        trace("[state] in_function = true\n");

        (*ip)++;
        trace("[ip] advanced to %d\n", *ip);
    }

    /* ==========================
           endfunc
    ========================== */
    cmd("endfunc") {
        trace("[exec] endfunc\n");

        if (*in_function) { // end def
            (*curr_function)->end = *ip;
            trace("[func] function '%s' end ip = %d\n", (*curr_function)->name, (*curr_function)->end);

            *in_function = false;
            *curr_function = NULL;
            trace("[state] in_function = false\n");

            (*ip)++;  // continue after endfunc definition
            trace("[ip] advanced to %d\n", *ip);
            return;
        }

        // end call
        trace("[func caller] function call ended\n");
        int ret_ip = pop_stack(&call_stk->ip_stk);
        int file_idx = pop_stack(&call_stk->file_stk);
        *curr_file = call_stk->paths[file_idx];
        trace("[func caller] returning to ip=%d, file=%s\n", ret_ip, *curr_file);
        char** tmp = realloc(call_stk->paths, call_stk->file_stk.top * sizeof(char*));
        if (!tmp && call_stk->file_stk.top > 0) {
            perror("realloc");
            exit(EXIT_FAILURE);
        }
        call_stk->paths = tmp;

        *ip = ret_ip;  // DO NOT increment
        trace("[ip] set to %d\n", *ip);
    }
    

    /* ==========================
       push
    ========================== */
    cmd("push") {
        trace("[exec] push\n");

        loop_params(i) {
            trace("[stack] push %d\n", params[i]);
            push_stack(stack, params[i]);
            trace("[stack] new top=%d\n", stack->top);
        }

        (*ip)++;
        trace("[ip] advanced to %d\n", *ip);
    }

    /* ==========================
       discard
    ========================== */
    cmd("discard") {
        trace("[exec] discard\n");

        discard_stack(stack);
        trace("[stack] new top=%d\n", stack->top);

        (*ip)++;
        trace("[ip] advanced to %d\n", *ip);
    }

    /* ===========================
                no-op
    ==============================*/
    cmd("noop") {
        trace("[exec] no-op\n");
        (*ip)++;
        trace("[ip] advanced to %d\n", *ip);
    }

    /* ==========================
       user-def'ed function
    ========================== */
    else {
        trace("[func caller] checking if \"%s\" exists...", func);
        int i; // global since we need it afterwards
        bool found = false;
        for (i = 0; i < *function_count; i++){
            if (strcmp(functions[i]->name, func) == 0){
                trace("yep.\n");
                found = true;
                break;
            }
        }
        if (!found) {
            trace("nah.\n");
            trace("[fatal] Error: unknown function %s.\n", func);
            exit(EXIT_FAILURE);
        }
        // function exists
        trace("[func caller] advancing ip to %d, which is the start addr of \"%s\", defined in %s.\n", functions[i]->start, functions[i]->name, functions[i]->fname);
        /*trace("[debug] not implemented yet, exiting\n");
        exit(EXIT_SUCCESS);*/
        push_stack(&call_stk->ip_stk, *ip + 1); // +1 so it advances and doesn't inf loop
        (*ip) = functions[i]->start;
        push_stack(&call_stk->file_stk, call_stk->file_stk.top);
        call_stk->paths = realloc(call_stk->paths, (call_stk->file_stk.top)*sizeof(char*));
        call_stk->paths[call_stk->file_stk.top - 1] = strdup(functions[i]->fname);
    }

    trace("=== interpret_line END ===\n");
}

void run_string_code(
    const char* source,
    const char* virtual_fname,
    Function** functions,
    int* function_count,
    struct Stack* stack,
    const char** include_paths
) {
    trace("\n=== run_string_code BEGIN (%s) ===\n", virtual_fname);

    int line_count = 0;
    char** lines = split_lines(source, &line_count);

    // Lex + parse
    lexTok** tokens = malloc(line_count * sizeof(lexTok*));
    int* tok_sizes = malloc(line_count * sizeof(int));
    Call* calls = malloc(line_count * sizeof(Call));

    if (!tokens || !tok_sizes || !calls) exit(EXIT_FAILURE);

    for (int i = 0; i < line_count; i++) {
        trace("[runner] lexing line %d: %s\n", i + 1, lines[i]);
        tokens[i] = lex_line(lines[i], (int)strlen(lines[i]) + 1, &tok_sizes[i]);
        calls[i] = parse_line(tokens[i], tok_sizes[i]);
    }

    // Runtime state
    int ip = 0;
    bool in_function = false;
    Function* curr_function = NULL;
    char* curr_file = strdup(virtual_fname);
    callStack cs = init_callStack();

    // Execute
    while (ip < line_count) {
        trace("\n[runner] executing ip=%d (%s)\n", ip, curr_file);
        interpret_line(
            calls[ip],
            line_count,
            &ip,
            functions,
            &in_function,
            function_count,
            &curr_function,
            stack,
            include_paths,
            &curr_file,
            &cs
        );
    }

    // Cleanup
    for (int i = 0; i < line_count; i++) {
        free_call(&calls[i]);
        for (int j = 0; j < tok_sizes[i]; j++) {
            free(tokens[i][j].val);
        }
        free(tokens[i]);
        free(lines[i]);
    }

    free(tokens);
    free(tok_sizes);
    free(calls);
    free(lines);
    free(curr_file);

    trace("=== run_string_code END ===\n");
}

int main(void) {
    struct Stack stack;
    init_stack(&stack);

    Function* functions = NULL;
    int function_count = 0;

    const char* include_paths[] = { "./include/" };

    const char* code =
        "startfunc \"f\";\n"
        "push 3;\n"
        "endfunc;\n"
        "f;\n";

    run_string_code(
        code,
        "<string>",
        &functions,
        &function_count,
        &stack,
        include_paths
    );

    printf("Stack top = %d\n\n", get_stack(&stack));
    print_stack(&stack, "full stack:");
    return 0;
}
