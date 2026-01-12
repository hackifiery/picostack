#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <stdio.h>

#include "lexer.h"
#include "helpers.h"
#include "parser.h"

/*typedef struct {
    char* name;
    int paramCount;
    bool isExtern; // if it's defined in the source rather than in a picostack file
} Function;

typedef struct {
    Function func;
    int* params;
    int params_len;
    char* params_char; // used for includes and specs
} Call;
*/

Call parse_line(const lexTok* code, const int in_size, bool verbose) {
    trace("\n=== parse_line BEGIN ===\n");

    Call out;
    memset(&out, 0, sizeof(Call));
    trace("[init] Call struct zeroed\n");

    /* --------------------------
       Input validation
    -------------------------- */
    if (!code) {
        error("code == NULL\n");
        exit(EXIT_FAILURE);
    }
    if (in_size == 0) {
        error("in_size == 0\n");
        exit(EXIT_FAILURE);
    }

    trace("[input] token count = %d\n", in_size);
    trace("[input] head token: type=%d val='%s'\n",
          code[0].type, code[0].val);

    lexTokType head = code[0].type;
    assert(head == Func || head == Spec || head == Include);

    /* --------------------------
       Determine call kind
    -------------------------- */
    bool is_startfunc = false;

    if (head == Func ) {
        trace("[call] function/keyword detected\n");

        out.func.name = strdup(code[0].val);
        trace("[call] function name = '%s'\n", out.func.name);

        is_startfunc = (strcmp(code[0].val, "startfunc") == 0);
        trace("[call] is_startfunc = %s\n",
              is_startfunc ? "true" : "false");

    } else if (head == Spec) {
        trace("[call] spec directive detected\n");
        out.func.name = strdup("_spec");

    } else if (head == Include) {
        trace("[call] include directive detected\n");
        out.func.name = strdup("_inc");
    }

    /* --------------------------
       Include / Spec handling
    -------------------------- */
    if (head == Include || head == Spec) {
        trace("[include/spec] capturing string payload\n");

        out.params_char = strdup(code[0].val);
        out.params_len = (int)strlen(out.params_char);
        out.func.paramCount = 1;

        trace("[include/spec] params_char='%s'\n", out.params_char);
        trace("[include/spec] paramCount=1\n");
        trace("=== parse_line END (early return) ===\n");

        return out;
    }

    /* --------------------------
       Parameter parsing setup
    -------------------------- */
    int cap = 8;
    out.params = malloc(cap * sizeof(int));
    out.params_len = 0;
    out.params_char = NULL;

    trace("[params] initial capacity = %d\n", cap);

    /* --------------------------
       Parse tokens
    -------------------------- */
    for (int i = 1; i < in_size && code[i].type != endCall; i++) {
        lexTok tok = code[i];

        trace("[token %d] type=%d val='%s'\n",
              i, tok.type, tok.val);

        switch (tok.type) {

        case Num: {
            if (out.params_len == cap) {
                cap *= 2;
                trace("[params] realloc to %d\n", cap);
                out.params = realloc(out.params, cap * sizeof(int));
            }

            int v = atoi(tok.val);
            out.params[out.params_len++] = v;

            trace("[params] pushed int %d (len=%d)\n",
                  v, out.params_len);
            break;
        }

        case String:
            if (is_startfunc) {
                trace("[string] startfunc name detected\n");

                if (out.params_char != NULL) {
                    error("startfunc already has a name\n");
                    exit(EXIT_FAILURE);
                }

                out.params_char = strdup(tok.val);
                trace("[string] startfunc name='%s'\n",
                      out.params_char);
            } else {
                trace("[string] converting string to char codes\n");

                int slen = 0;
                int* chars = string_to_params(tok.val, &slen);

                trace("[string] string length=%d\n", slen);

                while (out.params_len + slen > cap) {
                    cap *= 2;
                    trace("[params] realloc to %d\n", cap);
                    out.params = realloc(out.params, cap * sizeof(int));
                }

                memcpy(out.params + out.params_len,
                       chars,
                       slen * sizeof(int));

                out.params_len += slen;
                trace("[params] appended %d chars (len=%d)\n",
                      slen, out.params_len);

                free(chars);
            }
            break;

        case Func:
            error("nested call '%s' not supported\n", tok.val);
            exit(EXIT_FAILURE);

        default:
            trace("[skip] token ignored\n");
            break;
        }
    }

    /* --------------------------
       Finalization
    -------------------------- */
    out.func.paramCount =
        is_startfunc ? 1 : out.params_len;

    trace("[final] paramCount=%d\n", out.func.paramCount);

    if (out.params_len == 0) {
        trace("[final] no numeric params, freeing array\n");
        free(out.params);
        out.params = NULL;
    }

    trace("=== parse_line END ===\n");
    return out;
}
