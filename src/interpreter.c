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

callStack init_callStack(void) {
    callStack cs;
    init_stack(&cs.file_stk);
    cs.paths = NULL;
    return cs;
}



// so many parameters...
interpStatus interpret_line(
    Call code,
    int* ip, // note: manipulated by the runner, not this
    Function** functions,
    bool* in_function,
    int* function_count,
    Function** curr_function,
    struct Stack* stack,
    struct Stack* pstack,
    const char** include_paths,
    char** curr_file,
    callStack* call_stk,
    bool verbose
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
        return NORMAL;
    }

    char* func = code.func.name;
    int* params = code.params;
    int params_len = code.params_len;
    char* params_char = code.params_char;

    if (false); // dummy for macros


    /*========= INTERNALS =================*/


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
            error("realloc failed\n");
            exit(EXIT_FAILURE);
        }

        *functions = tmp;

        Function* f = &(*functions)[(*function_count) - 1];

        f->name = strdup(params_char);
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

        init_stack(pstack); // reset parameters
        
        ret_trace(NORMAL);
    }

    /* ==========================
       include
    ========================== */
    cmd("_inc") {
        char *fullpath = NULL;
        for (int i = 0; include_paths[i] != NULL; i++) {
            char tmp[512];
            snprintf(tmp, sizeof(tmp), "%s%s",
                include_paths[0], params_char);
            if (file_exists(tmp)) {
                fullpath = strdup(tmp);
                break;
            }
        }
        if (fullpath == NULL) {
            error("%s not found.", params_char);
        }
        // push file
        push_stack(&call_stk->file_stk, call_stk->file_stk.top + 1);
        int idx = call_stk->file_stk.top;

        call_stk->paths = realloc(call_stk->paths, (idx + 1) * sizeof(char*));
        call_stk->paths[idx] = strdup(fullpath);

        trace("included file %s, %d pushed to file_idx stack\n", fullpath, get_stack(&call_stk->file_stk));

        ret_trace(INCLUDE_CALL);
    }

    /* ======== END INTERNALS ======= */



    /* ====== KEYWORDS =============*/


    #define ei else if
    #define ccmd(x) strcmp(func, x) == 0
    #define newtop trace("[stack] new top=%d\n", stack->top);

    /* ==========================
       push
    ========================== */
    ei(ccmd("ps") || ccmd("push")) {
        trace("[exec] push\n");

        loop_params(i) {
            trace("[stack] push %d\n", params[i]);
            push_stack(stack, params[i]);
            newtop;
        }

        ret_trace(NORMAL);
    }

    /* ========================
        param pop
    ========================== */

    cmd("ppop") {
        trace("[exec] param stack pop");
        int val = pop_stack(pstack);
        push_stack(stack, val);
        ret_trace(NORMAL);
    }

    /* ==========================
       discard
    ========================== */
    ei(ccmd("disc") || ccmd("discard")) {
        trace("[exec] discard\n");

        discard_stack(stack);
        newtop;

        ret_trace(NORMAL);
    }

    /* =========================
       duplicate
    ========================== */

    ei(ccmd("dup") || ccmd("duplicate")) {
        trace("[exec] duplicate\n");
        dup_stack(stack);
        newtop;
        ret_trace(NORMAL);
    }

    /* ========================
       rotate right
    ======================== */
    ei(ccmd("rotr") || ccmd("rr")) {
        trace("[exec] rotate r %d\n", params[0]);
        rot_right_stack(stack, params[0]);
        ret_trace(NORMAL);
    }

    ei(ccmd("protr") || ccmd("prr")) {
        trace("[exec] param rotate r %d\n", params[0]);
        rot_right_stack(pstack, params[0]);
        ret_trace(NORMAL);
    }

    /* =======================
        rotate left
    ========================= */

    ei(ccmd("rotl") || ccmd("rl")) {
        trace("[exec] rotate l %d\n", params[0]);
        rot_left_stack(stack, params[0]);
        ret_trace(NORMAL);
    }

    ei(ccmd("protl") || ccmd("prl")) {
        trace("[exec] param rotate l %d\n", params[0]);
        rot_left_stack(pstack, params[0]);
        ret_trace(NORMAL);
    }

    /* ========================
        get stack size
    ========================== */

    ei(ccmd("sl") || ccmd("stklen")) {
        trace("[exec] stack len\n");
        push_stack(stack, stack->top+1);
        ret_trace(NORMAL);
    }

    ei(ccmd("psl") || ccmd("pstklen")) {
        trace("[exec] param stack len\n");
        push_stack(stack, pstack->top+1);
        ret_trace(NORMAL);
    }

    /* =========================
        print char
    ============================ */

    cmd("putc") {
        trace("[exec] putc");
        out_stack(stack);
        ret_trace(NORMAL);
    }

    /* =========================
        jumps
    ============================ */

    cmd("jt") {
        trace("[exec] jt");
        if (params[1] == 1) *ip = params[0];
        ret_trace(JUMP);
    }

    cmd("jf") {
        trace("[exec] jf");
        if (params[1] != 1) *ip = params[0];
        ret_trace(JUMP);
    }

    /* ===========================
                no-op
    ==============================*/
    cmd("noop") {
        trace("[exec] no-op\n");

        ret_trace(NORMAL);
    }

    #undef ccmd
    #undef ei
    #undef newtop
    /* ============ END KEYWORDS ==================*/


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
        loop_params(i) push_stack(pstack, params[i]);
        ret_trace(FUNCTION_CALL);
    }

    trace("=== interpret_line END ===\n");
}

// not again...
void run_str(
    char** code,
    struct Stack* stack,
    struct Stack* pstk,
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

        lex = lex_line(code[ip], strlen(code[ip]) + 1, &lex_size, stack, *in_function);
        par = parse_line(lex, lex_size, false);

        interpStatus st = interpret_line(
            par,
            &ip,
            functions,
            in_function,
            function_count,
            curr_function,
            stack,
            pstk,
            include_paths,
            &cs->paths[file_idx],
            cs,
            false
            //true
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
                pstk,
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
                pstk,
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
        else if (st == JUMP); // handled by itself (no call stack-ing)
    }

    // pop file
    free(cs->paths[file_idx]);
    cs->paths[file_idx] = NULL;
    discard_stack(&cs->file_stk);
}