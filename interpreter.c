#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "parser.h"
#include "stack.h"

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
        push_stack(&call_stk->file_stk, call_stk->file_stk.top);

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

    /* --------------------------
       Skip execution inside func
    -------------------------- */
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
        f->paramCount = -1;
        f->start = *ip + 1;
        f->end = -1;

        trace("[func] defined function '%s'\n", f->name);
        trace("[func] start ip = %d\n", f->start);

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

        if (*in_function) {
            (*curr_function)->end = *ip;
            trace("[func] function '%s' end ip = %d\n",
                  (*curr_function)->name,
                  (*curr_function)->end);

            *in_function = false;
            *curr_function = NULL;

            trace("[state] in_function = false\n");
        } else {
            trace("[warn] endfunc outside function\n");
        }

        (*ip)++;
        trace("[ip] advanced to %d\n", *ip);
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

    /* ==========================
       user-def'ed function
    ========================== */
    else {
        trace("[func caller] checking if \"%s\" exists...", func);
        int i; // global since we need it afterwards
        for (i = 0; i < *function_count; i++){
            if (strcmp(functions[i]->name, func) == 0){
                trace("yep.\n");
                break;
            }
            else {
                trace("nah.\n");
                trace("[fatal] Error: unknown function %s.\n", func);
                exit(EXIT_FAILURE);
            }
        }
        // function exists
        trace("[func caller] advancing ip to %d, which is the start addr of \"%s\".\n", functions[i]->start, functions[i]->name);
        trace("[debug] not implemented yet, exiting\n");
        exit(EXIT_SUCCESS);
        (*ip) = functions[i]->start;
    }

    trace("=== interpret_line END ===\n");
}



int main(void) {
    // ---- setup ----
    struct Stack stack;
    init_stack(&stack);

    Function* functions = NULL;
    int function_count = 0;
    bool in_function = false;
    Function* curr_function = NULL;

    int ip = 0; // instruction pointer

    // Lex tokens for test program: define f, push 3, endfunc, call f
    int l1, l2, l3, l4;
    lexTok* lines[4] = {
        lex_line("startfunc \"f\";", 15, &l1),
        lex_line("push 3;", 8, &l2),
        lex_line("endfunc;", 9, &l3),
        lex_line("f;", 3, &l4)
    };

    Call calls[4];
    calls[0] = parse_line(lines[0], l1);
    calls[1] = parse_line(lines[1], l2);
    calls[2] = parse_line(lines[2], l3);
    calls[3] = parse_line(lines[3], l4);

    const char* include_paths[] = { "./include/" };
    callStack cs = init_callStack();
    char* cf = "main.pcs";

    // ---- execute ----
    for (ip = 0; ip < 4;) {
        trace("\n[runner] running line %d.\n", ip+1);
        interpret_line(calls[ip], 0, &ip, &functions, &in_function, &function_count, &curr_function, &stack, include_paths, &cf, &cs);
        
    }

    // ---- test results ----
    printf("Stack size: %d\n", stack.top);
    if (stack.top > 0) {
        printf("Top of stack = %d (expected 3)\n", stack.arr[stack.top - 1]);
    }

    // ---- cleanup ----
    for (int i = 0; i < 4; i++) {
        free_call(&calls[i]);
        free(lines[i]);
    }

    for (int i = 0; i < function_count; i++) {
        free(functions[i].name);
    }
    free(functions);
    free(stack.arr);

    return 0;
}