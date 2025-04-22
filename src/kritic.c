#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "../kritic.h"

static kritic_runtime_t* kritic_runtime_state = &(kritic_runtime_t) {
    .test_state     = NULL,
    .redirect       = NULL,
    .fail_count     = 0,
    .test_count     = 0,
    .printers       = &(kritic_printers_t) {
        .assert_printer    = &kritic_default_assert_printer,
        .pre_test_printer  = &kritic_default_pre_test_printer,
        .post_test_printer = &kritic_default_post_test_printer,
        .summary_printer   = &kritic_default_summary_printer,
        .init_printer      = &kritic_default_init_printer,
        .stdout_printer    = &kritic_default_stdout_printer,
        .skip_printer      = &kritic_default_skip_printer
    }
};

/* Getter for kritic_runtime_state() */
kritic_runtime_t* kritic_get_runtime_state(void) {
    return kritic_runtime_state;
}

#ifdef _WIN32
#include <windows.h>

/* Enable ANSI support on Windows terminal */
void kritic_enable_ansi_(void) {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;

    if (hOut != INVALID_HANDLE_VALUE &&
        GetConsoleMode(hOut, &dwMode)) {
        SetConsoleMode(hOut, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }
}
#endif

/* Register a test function to a specific suite with a specific name */
void kritic_register(const kritic_context_t* ctx, kritic_test_fn fn) {
    kritic_runtime_t* kritic_state = kritic_get_runtime_state();
    if (kritic_state->test_count >= KRITIC_MAX_TESTS) {
        fprintf(stderr, "[kritic] Too many registered tests.\n");
        exit(1);
    }

    kritic_state->tests[kritic_state->test_count++] = (kritic_test_t) {
        .file  = ctx->file,
        .suite = ctx->suite,
        .name  = ctx->test,
        .line  = ctx->line,
        .fn    = fn
    };
}

/* Run all of the test suites and tests */
int kritic_run_all(void) {
    kritic_runtime_t* kritic_state = kritic_get_runtime_state();
    kritic_redirect_t* redir = &(kritic_redirect_t) { 0 };
    kritic_state->redirect = redir;

    kritic_state->printers->init_printer(kritic_state);
    kritic_redirect_init(kritic_state);

    for (int i = 0; i < kritic_state->test_count; ++i) {
        const kritic_test_t* t = &kritic_state->tests[i];

        kritic_state->test_state = &(kritic_test_state_t) {
            .test           = t,
            .assert_count   = 0,
            .asserts_failed = 0,
            .skipped        = false,
            .skip_reason    = "",
        };

        kritic_state->printers->pre_test_printer(kritic_state);
        kritic_redirect_start(kritic_state);
        t->fn();
        kritic_redirect_stop(kritic_state);
        if (kritic_state->test_state->asserts_failed > 0) {
            ++kritic_state->fail_count;
        }
        kritic_state->printers->post_test_printer(kritic_state);
    }

    fflush(stdout);
    kritic_state->printers->summary_printer(kritic_state);

    kritic_redirect_teardown(kritic_state);
    kritic_state->redirect = NULL;

    return kritic_state->fail_count > 0 ? 1 : 0;
}

void kritic_assert_eq(
    const kritic_context_t* ctx,
    long long actual,
    long long expected,
    const char* actual_expr,
    const char* expected_expr,
    const kritic_assert_type_t assert_type
) {
    bool passed = 0;
    switch (assert_type) {
        case KRITIC_ASSERT:
            passed = actual;
            break;
        case KRITIC_ASSERT_FAIL:
            break;
        case KRITIC_ASSERT_EQ:
            passed = (actual == expected);
            break;
        case KRITIC_ASSERT_NE:
            passed = !(actual == expected);
            break;
        case KRITIC_ASSERT_NOT:
            passed = !actual;
            break;
        case KRITIC_ASSERT_UNKNOWN:
        default:
            break;
    }

    kritic_runtime_t* kritic_state = kritic_get_runtime_state();
    ++kritic_state->test_state->assert_count;
    if (!passed) ++kritic_state->test_state->asserts_failed;
    kritic_state->printers->assert_printer(ctx, passed, actual, expected, actual_expr, expected_expr, assert_type);
}

void kritic_skip_test(const kritic_context_t* ctx, const char* reason) {
    kritic_runtime_t* kritic_state = kritic_get_runtime_state();

    kritic_state->test_state->skipped = true;
    kritic_state->test_state->skip_reason = reason;

    kritic_state->printers->skip_printer(kritic_state, ctx);
}

/* =-=-=-=-=-=-=-=-=-=-=-= */
/* Default implementations */
/* =-=-=-=-=-=-=-=-=-=-=-= */

/* Default assert print implementation */
void kritic_default_assert_printer(
    const kritic_context_t* ctx,
    bool passed,
    long long actual,
    long long expected,
    const char* actual_expr,
    const char* expected_expr,
    kritic_assert_type_t assert_type
) {
    if (passed) return;
    const char* label = "[ \033[1;31mFAIL\033[0m ]";

    switch (assert_type) {
        case KRITIC_ASSERT_EQ:
            fprintf(stderr, "%s  %s.%s: %s == %s failed at %s:%d\n",
                    label, ctx->suite, ctx->test, actual_expr, expected_expr, ctx->file, ctx->line);
            fprintf(stderr, "          -> %s = %lld, %s = %lld\n",
                    actual_expr, actual, expected_expr, expected);
            break;

        case KRITIC_ASSERT_NE:
            fprintf(stderr, "%s  %s.%s: %s != %s failed at %s:%d\n",
                    label, ctx->suite, ctx->test, actual_expr, expected_expr, ctx->file, ctx->line);
            fprintf(stderr, "          -> both = %lld\n", actual);
            break;

        case KRITIC_ASSERT:
            fprintf(stderr, "%s  %s.%s: assertion failed: %s at %s:%d\n",
                    label, ctx->suite, ctx->test, actual_expr, ctx->file, ctx->line);
            fprintf(stderr, "          -> value = %lld\n", actual);
            break;
        
        case KRITIC_ASSERT_NOT:
            fprintf(stderr, "%s  %s.%s: assertion expected to fail: %s at %s:%d\n",
                    label, ctx->suite, ctx->test, actual_expr, ctx->file, ctx->line);
            fprintf(stderr, "          -> value = %lld (was truthy)\n", actual);
            break;
            
        case KRITIC_ASSERT_FAIL:
            fprintf(stderr, "%s  %s.%s: forced failure at %s:%d\n",
                    label, ctx->suite, ctx->test, ctx->file, ctx->line);
            break;

        default:
            fprintf(stderr, "%s  %s.%s: unknown assertion type at %s:%d\n",
                    label, ctx->suite, ctx->test, ctx->file, ctx->line);
            break;
    }
}

void kritic_default_pre_test_printer(kritic_runtime_t* state) {
    printf("[ \033[1;36mEXEC\033[0m ] %s.%s at %s:%i\n", KRITIC_GET_CURRENT_SUITE(), KRITIC_GET_CURRENT_TEST(),
            state->test_state->test->file, state->test_state->test->line);
}

void kritic_default_post_test_printer(kritic_runtime_t* state) {
    if (state->test_state->asserts_failed > 0) {
        fprintf(stderr, "[ \033[1;31mFAIL\033[0m ] %s.%s\n", KRITIC_GET_CURRENT_SUITE(), KRITIC_GET_CURRENT_TEST());
    } else {
        printf("[ \033[1;32mPASS\033[0m ] %s.%s\n", KRITIC_GET_CURRENT_SUITE(), KRITIC_GET_CURRENT_TEST());
    }
}

void kritic_default_summary_printer(kritic_runtime_t* state) {
    const char* RESET  = "\033[0m";
    const char* GREEN  = "\033[32m";
    const char* RED    = "\033[31m";
    const char* CYAN   = "\033[36m";

    int passed = state->test_count - state->fail_count;
    float pass_rate = state->test_count > 0
        ? 100.0f * passed / state->test_count
        : 0.0f;
    char buffer[512];
    int len = snprintf(buffer, sizeof(buffer),
        "[      ] Finished running %d tests!\n"
        "[      ]\n"
        "[      ] Statistics:\n"
        "[      ]   Total  : %d\n"
        "[      ]   Passed : %s%d%s\n"
        "[      ]   Failed : %s%d%s\n"
        "[      ]   Rate   : %s%.1f%%%s\n"
        "[      ]\n"
        "%s\n",
        state->test_count,
        state->test_count,
        GREEN, passed, RESET,
        RED, state->fail_count, RESET,
        CYAN, pass_rate, RESET,
        state->fail_count > 0
            ? "[ \033[1;31m!!!!\033[0m ] Some tests failed!"
            : "[ \033[1;32m****\033[0m ] All tests passed!"
    );

    if (len > 0 && len < (int)sizeof(buffer)) {
        _write(1, buffer, (size_t)len);
    }
}

void kritic_default_init_printer(kritic_runtime_t* state) {
    if (state->test_count == 0) {
        printf("[kritic] No registered test found\n");
    } else {
        printf("[kritic] Running %d tests:\n", state->test_count);
    }
}

void kritic_default_stdout_printer(kritic_runtime_t* _, kritic_redirect_ctx_t* redir_ctx) {
    if (!redir_ctx->is_part_of_split) {
        _write(redir_ctx->stdout_copy, "[ \033[34mINFO\033[0m ] ", 18);
    }
    _write(redir_ctx->stdout_copy, redir_ctx->string, redir_ctx->length);
}

void kritic_default_skip_printer(kritic_runtime_t* state, const kritic_context_t* ctx) {
    char buffer[4096];

    int len = snprintf(buffer, sizeof(buffer),
        "[ SKIP ] Reason: %s at %s:%d\n",
        state->test_state->skip_reason,
        ctx->file,
        ctx->line
    );

    if (len > 0 && len < (int)sizeof(buffer)) {
        _write(state->redirect->stdout_copy, buffer, (size_t)len);
    }
}

/* Default KritiC main(void) code used to initialize the framework */
int main(void) {
    kritic_enable_ansi();
    return kritic_run_all();
}
