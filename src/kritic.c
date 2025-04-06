#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "../kritic.h"

#define KRITIC_MAX_TESTS 1024

static kritic_test_t kritic_tests[KRITIC_MAX_TESTS];
static int kritic_test_count = 0;
static int kritic_fail_count = 0;
const char* kritic_current_suite = NULL;
const char* kritic_current_test  = NULL;
static kritik_assert_printer_fn kritik_assert_printer_impl = &_kritic_default_assert_printer;


#ifdef _WIN32
#include <windows.h>

/* Enable ANSI support on Windows terminal */
void kritic_enable_ansi(void) {
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
    if (kritic_test_count >= KRITIC_MAX_TESTS) {
        fprintf(stderr, "[kritic] Too many registered tests.\n");
        exit(1);
    }

    kritic_tests[kritic_test_count++] = (kritic_test_t){
        .file  = ctx->file,
        .suite = ctx->suite,
        .name  = ctx->test,
        .line  = ctx->line,
        .fn    = fn
    };
}

/* Run all of the test suites and tests */
int kritic_run_all(void) {
    int failed = 0;

    if (kritic_test_count == 0) {
        printf("[kritic] No registered test found\n");
    } else {
        printf("[kritic] Running %d tests:\n", kritic_test_count);
    }

    for (int i = 0; i < kritic_test_count; ++i) {
        const kritic_test_t* t = &kritic_tests[i];

        kritic_current_suite = t->suite;
        kritic_current_test  = t->name;

        printf("-> %s.%s\n", kritic_current_suite, kritic_current_test);
        t->fn();
    }

    printf("[kritic] Finished running %d tests\n", kritic_test_count);

    return failed > 0 ? 1 : 0;
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
    if (!passed) kritic_fail_count++;
    kritik_assert_printer_impl(ctx, passed, actual, expected, actual_expr, expected_expr, assert_type);
}

/* =-=-=-=-=-=-=-=-=-=-=-= */
/* Default implementations */
/* =-=-=-=-=-=-=-=-=-=-=-= */

/* Default assert print implementation */
void _kritic_default_assert_printer(
    const kritic_context_t* ctx,
    bool passed,
    long long actual,
    long long expected,
    const char* actual_expr,
    const char* expected_expr,
    kritic_assert_type_t assert_type
) {
    const char* label = passed
        ? "[ \033[1;32mPASS\033[0m ]"
        : "[ \033[1;31mFAIL\033[0m ]";

    switch (assert_type) {
        case KRITIC_ASSERT_EQ:
            if (passed) {
                printf("%s  %s.%s\n", label, ctx->suite, ctx->test);
            } else {
                fprintf(stderr, "%s  %s.%s: %s == %s failed at %s:%d\n",
                        label, ctx->suite, ctx->test, actual_expr, expected_expr, ctx->file, ctx->line);
                fprintf(stderr, "          -> %s = %lld, %s = %lld\n",
                        actual_expr, actual, expected_expr, expected);
            }
            break;

        case KRITIC_ASSERT_NE:
            if (passed) {
                printf("%s  %s.%s\n", label, ctx->suite, ctx->test);
            } else {
                fprintf(stderr, "%s  %s.%s: %s != %s failed at %s:%d\n",
                        label, ctx->suite, ctx->test, actual_expr, expected_expr, ctx->file, ctx->line);
                fprintf(stderr, "          -> both = %lld\n", actual);
            }
            break;

        case KRITIC_ASSERT:
            if (passed) {
                printf("%s  %s.%s\n", label, ctx->suite, ctx->test);
            } else {
                fprintf(stderr, "%s  %s.%s: assertion failed: %s at %s:%d\n",
                        label, ctx->suite, ctx->test, actual_expr, ctx->file, ctx->line);
                fprintf(stderr, "          -> value = %lld\n", actual);
            }
            break;
        
        case KRITIC_ASSERT_NOT:
            if (passed) {
                printf("%s  %s.%s\n", label, ctx->suite, ctx->test);
            } else {
                fprintf(stderr, "%s  %s.%s: assertion expected to fail: %s at %s:%d\n",
                        label, ctx->suite, ctx->test, actual_expr, ctx->file, ctx->line);
                fprintf(stderr, "          -> value = %lld (was truthy)\n", actual);
            }
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

/* Default KritiC main(void) code used to initialize the framework */
int main(void) {
    kritic_enable_ansi();
    printf("Hello World!\n");
    return kritic_run_all();
}
