#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "parser.h"
#include "stack.h"
#include "helpers.h"
#include "interpreter.h"

#define cmd(x) else if (strcmp(func, x) == 0)
#define loop_params(i) for (int i = 0; i < params_len; i++)
#define ic (*ip)++


#define trace(...) fprintf(stderr, __VA_ARGS__)

// so many parameters...
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
) {
    trace("\n=== interpret_line BEGIN ===\n");
    #define ret_trace(x) trace("=== interpret_line END ===\n"); return x

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


        trace("=== interpret_line END (skipped) ===\n");
        ret_trace(NORMAL);
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
        trace("[func] start ip=%d, file=%s\n", f->start, strdup(*curr_file));

        *in_function = true;
        *curr_function = f;

        trace("[state] in_function = true\n");



        ret_trace(NORMAL);
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

            // continue after endfunc definition

            ret_trace(NORMAL);
        }

        // end call
        trace("[func caller] function call ended\n");
        int file_idx = pop_stack(&call_stk->file_stk);
        *curr_file = call_stk->paths[file_idx];
        trace("[func caller] returning file=%s\n", *curr_file);
        char** tmp = realloc(call_stk->paths, (call_stk->file_stk.top + 1) * sizeof(char*));
        if (!tmp && call_stk->file_stk.top > 0) {
            perror("realloc");
            exit(EXIT_FAILURE);
        }
        call_stk->paths = tmp;
        
        ret_trace(NORMAL);
    }

    /* ==========================
   include
========================== */
    cmd("_inc") {
        char fullpath[512];
        snprintf(fullpath, sizeof(fullpath), "%s%s",
            include_paths[0], params_char);

        // push file
        push_stack(&call_stk->file_stk, call_stk->file_stk.top + 1);
        int idx = call_stk->file_stk.top;

        call_stk->paths = realloc(call_stk->paths, (idx + 1) * sizeof(char*));
        call_stk->paths[idx] = strdup(fullpath);

        trace("included file %s, %d pushed to file_idx stack\n", fullpath, get_stack(&call_stk->file_stk));

        ret_trace(INCLUDE_CALL);
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

        ret_trace(NORMAL);
    }

    /* ==========================
       discard
    ========================== */
    cmd("discard") {
        trace("[exec] discard\n");

        discard_stack(stack);
        trace("[stack] new top=%d\n", stack->top);

        ret_trace(NORMAL);
    }

    /* ===========================
                no-op
    ==============================*/
    cmd("noop") {
        trace("[exec] no-op\n");


        ret_trace(NORMAL);
    }

    /* ==========================
       user-def'ed function
    ========================== */
    else {
        int i;
        for (i = 0; i < *function_count; i++) {
            if (strcmp((*functions)[i].name, func) == 0)
                break;
        }
        if (i == *function_count) {
            fprintf(stderr, "unknown function %s\n", func);
            exit(1);
        }
        *curr_function = &(*functions)[i];
        ret_trace(FUNCTION_CALL);
    }

    trace("=== interpret_line END ===\n");
}

// not again...
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
) {
    int ip = 0;

    // push current file
    push_stack(&cs->file_stk, cs->file_stk.top + 1);
    int file_idx = cs->file_stk.top;

    cs->paths = realloc(cs->paths, (file_idx + 1) * sizeof(char*));
    cs->paths[file_idx] = strdup(fn);

    while (ip < in_size) {
        lexTok* lex;
        int lex_size;
        Call par;

        lex = lex_line(code[ip], strlen(code[ip]) + 1, &lex_size);
        par = parse_line(lex, lex_size);

        interpStatus st = interpret_line(
            par,
            &ip,
            functions,
            in_function,
            function_count,
            curr_function,
            stack,
            include_paths,
            &cs->paths[file_idx],
            cs
        );

        if (st == NORMAL) {
            ip++;
        }

        else if (st == INCLUDE_CALL) {
            const char* fname = cs->paths[cs->file_stk.top];
            char* text = read_file(fname);

            int n;
            char** lines = split_lines(text, &n);

            run_str(
                lines,
                stack,
                n,
                fname,
                functions,
                in_function,
                function_count,
                curr_function,
                include_paths,
                cs
            );

            ip++;  // resume after include
        }

        else if (st == FUNCTION_CALL) {
            Function* f = *curr_function;  // already selected

            char* text = read_file(f->fname);
            int n;
            char** lines = split_lines(text, &n);

            run_str(
                lines + f->start,
                stack,
                f->end - f->start,
                f->fname,
                functions,
                in_function,
                function_count,
                curr_function,
                include_paths,
                cs
            );

            ip++;  // resume after function call
        }
    }

    // pop file
    free(cs->paths[file_idx]);
    cs->paths[file_idx] = NULL;
    discard_stack(&cs->file_stk);
}

int main(void) {
    char* code[] = {
        "@io.pcs;",
        "f();"
    };
    int prog_size = sizeof(code) / sizeof(code[0]);

    struct Stack stack;
    init_stack(&stack);

    callStack cs = init_callStack();

    Function* functions = NULL;
    Function* curr_function = NULL;
    bool in_function = false;
    int function_count = 0;

    const char* include_paths[] = { "./include/" };

    run_str(
        code,
        &stack,
        prog_size,
        "testf.pcs",
        &functions,
        &in_function,
        &function_count,
        &curr_function,
        include_paths,
        &cs
    );

    printf("\nAfter program run:\n");
    printf("Stack top = %d\n", stack.top);
    printf("Stack contents (top -> bottom): ");
    for (int i = stack.top; i >= 0; i--) {
        printf("%d ", stack.arr[i]);
    }
    printf("\n");

    printf("Call stack top = %d\n", cs.file_stk.top);
    printf("Call stack files (top -> bottom): ");
    for (int i = cs.file_stk.top; i >= 0; i--) {
        printf("%s ", cs.paths[i]);
    }
    printf("\n");

    free(cs.paths);
    free(stack.arr);
    free(functions);
    return 0;
}