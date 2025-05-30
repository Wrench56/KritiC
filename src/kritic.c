#include <inttypes.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "../kritic.h"

static kritic_runtime_t* kritic_runtime_state = &(kritic_runtime_t) {
    .test_state     = NULL,
    .redirect       = NULL,
    .first_node     = NULL,
    .last_node      = NULL,
    .queue          = NULL,
    .timer          = { 0 },
    .fail_count     = 0,
    .test_count     = 0,
    .printers       = { 0 }
};

/* Getter for kritic_runtime_state() */
kritic_runtime_t* kritic_get_runtime_state(void) {
    return kritic_runtime_state;
}

/* Override the default printers */
void kritic_override_printers(const kritic_printers_t* overrides) {
    kritic_runtime_t* state = kritic_get_runtime_state();
    if (!state || !overrides) return;

    if (overrides->assert_printer)    state->printers.assert_printer    = overrides->assert_printer;
    if (overrides->pre_test_printer)  state->printers.pre_test_printer  = overrides->pre_test_printer;
    if (overrides->post_test_printer) state->printers.post_test_printer = overrides->post_test_printer;
    if (overrides->summary_printer)   state->printers.summary_printer   = overrides->summary_printer;
    if (overrides->init_printer)      state->printers.init_printer      = overrides->init_printer;
    if (overrides->stdout_printer)    state->printers.stdout_printer    = overrides->stdout_printer;
    if (overrides->skip_printer)      state->printers.skip_printer      = overrides->skip_printer;
}

void kritic_set_default_printers(void) {
    kritic_runtime_t* state = kritic_get_runtime_state();
    state->printers = (kritic_printers_t) {
        .assert_printer    = &kritic_default_assert_printer,
        .pre_test_printer  = &kritic_default_pre_test_printer,
        .post_test_printer = &kritic_default_post_test_printer,
        .summary_printer   = &kritic_default_summary_printer,
        .init_printer      = &kritic_default_init_printer,
        .stdout_printer    = &kritic_default_stdout_printer,
        .skip_printer      = &kritic_default_skip_printer,
        .dep_fail_printer  = &kritic_default_dep_fail_printer
    };
}

void kritic_noop(void* dummy, ...) {
    (void) dummy;
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

/* Run all of the test suites and tests */
int kritic_run_all(void) {
    kritic_runtime_t* kritic_state = kritic_get_runtime_state();

    kritic_timer_start(&kritic_state->timer);

    kritic_construct_queue(kritic_state);

    kritic_redirect_t* redir = &(kritic_redirect_t) { 0 };
    kritic_state->redirect = redir;

    kritic_state->printers.init_printer(kritic_state);
    kritic_redirect_init(kritic_state);

    for (kritic_test_t** t = kritic_state->queue; *t != NULL; ++t) {
        kritic_state->test_state = &(kritic_test_state_t) {
            .test           = *t,
            .assert_count   = 0,
            .asserts_failed = 0,
            .skipped        = false,
            .skip_reason    = "",
            .duration_ns    = 0,
            .timer          = { 0 },
        };

        kritic_state->printers.pre_test_printer(kritic_state);
        if ((*t)->status != KRITIC_QUEUED) {
            fprintf(stderr, "[      ] Error: Test found in queue that has not yet been queued!\n");
            continue;
        }

        /* Check if dependencies succeeded */
        for (kritic_test_index_t** ptr = (*t)->dependencies; *ptr != NULL; ++ptr) {
            switch ((*ptr)->test_ptr->status) {
                case KRITIC_REGISTERED:
                case KRITIC_QUEUED:
                case KRITIC_RUNNING:
                    fprintf(stderr, "[      ] Error: Dependency \"%s.%s\" for test \"%s.%s\" did not run yet!\n",
                        (*ptr)->suite, (*ptr)->name, (*t)->suite, (*t)->name);
                    return 2;
                case KRITIC_FAILED:
                case KRITIC_DEP_FAILED:
                    kritic_state->printers.dep_fail_printer(kritic_state, (*t), (*ptr)->test_ptr);
                    (*t)->status = KRITIC_DEP_FAILED;
                    goto skip_test;
                case KRITIC_PASSED:
                    continue;
                case KRITIC_UNKNOWN:
                default:
                    fprintf(stderr, "[      ] Error: Test with unknown state found: \"%s.%s\"\n",
                        (*t)->suite, (*t)->name);
                    return 3;
            }
        }

        (*t)->status = KRITIC_RUNNING;
        kritic_redirect_start(kritic_state);
        kritic_timer_start(&kritic_state->test_state->timer);
        (*t)->fn();
        kritic_state->test_state->duration_ns = kritic_timer_elapsed(&kritic_state->test_state->timer);
        kritic_redirect_stop(kritic_state);

        if (kritic_state->test_state->skipped) {
            ++kritic_state->skip_count;
            (*t)->status = KRITIC_SKIPPED;
        } else if (kritic_state->test_state->asserts_failed > 0) {
            ++kritic_state->fail_count;
            (*t)->status = KRITIC_FAILED;
        } else {
            (*t)->status = KRITIC_PASSED;
        }

        kritic_state->printers.post_test_printer(kritic_state);

        // Label for test skip
        skip_test:
            continue;
    }

    fflush(stdout);
    kritic_state->duration_ns = kritic_timer_elapsed(&kritic_state->timer);
    kritic_state->printers.summary_printer(kritic_state);

    kritic_redirect_teardown(kritic_state);
    kritic_state->redirect = NULL;
    kritic_free_queue(kritic_state);

    return kritic_state->fail_count > 0;
}

void kritic_assert_eq(
    const kritic_context_t* ctx,
    long long actual,
    long long expected,
    const char* actual_expr,
    const char* expected_expr,
    const kritic_assert_type_t assert_type
) {
    bool passed = false;
    double actual_f, expected_f, delta;
    const char* actual_s;
    const char* expected_s;
    union {
        long long i;
        double f;
    } u_actual, u_expected;


    switch (assert_type) {
        case KRITIC_ASSERT:
            passed = actual;
            break;
        case KRITIC_ASSERT_FAIL:
            break;
        case KRITIC_ASSERT_EQ_INT:
            passed = (actual == expected);
            break;
        case KRITIC_ASSERT_EQ_FLOAT:
            u_actual.i = actual;
            u_expected.i = expected;
            actual_f = u_actual.f;
            expected_f = u_expected.f;
            delta = fabs(actual_f - expected_f);
            passed = (delta <= KRITIC_FLOAT_DELTA_VALUE);
            break;
        case KRITIC_ASSERT_EQ_STR:
            actual_s = (const char*)(uintptr_t)actual;
            expected_s = (const char*)(uintptr_t)expected;
            passed = (actual_s && expected_s) ? (strcmp(actual_s, expected_s) == 0) : (actual_s == expected_s);
            break;
        case KRITIC_ASSERT_NE_INT:
            passed = !(actual == expected);
            break;
        case KRITIC_ASSERT_NE_FLOAT:
            u_actual.i = actual;
            u_expected.i = expected;
            actual_f = u_actual.f;
            expected_f = u_expected.f;
            delta = fabs(actual_f - expected_f);
            passed = !(delta <= KRITIC_FLOAT_DELTA_VALUE);
            break;
        case KRITIC_ASSERT_NE_STR:
            actual_s = (const char*)(uintptr_t)actual;
            expected_s = (const char*)(uintptr_t)expected;
            passed = (actual_s && expected_s) ? (strcmp(actual_s, expected_s) != 0) : (actual_s != expected_s);
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
    kritic_state->printers.assert_printer(ctx, passed, actual, expected, actual_expr, expected_expr, assert_type);
}

void kritic_skip_test(const kritic_context_t* ctx, const char* reason) {
    kritic_runtime_t* kritic_state = kritic_get_runtime_state();

    kritic_state->test_state->skipped = true;
    kritic_state->test_state->skip_reason = reason;

    kritic_state->printers.skip_printer(kritic_state, ctx);
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
    double actual_f = 0, expected_f = 0, delta = 0;
    const char* actual_s = NULL;
    const char* expected_s = NULL;
    union {
        long long i;
        double f;
    } u_actual, u_expected;

    switch (assert_type) {
        case KRITIC_ASSERT_EQ_INT:
            fprintf(stderr, "%s  %s.%s: %s == %s failed at %s:%d\n",
                    label, ctx->suite, ctx->test, actual_expr, expected_expr, ctx->file, ctx->line);
            fprintf(stderr, "[      ]  -> %s = %lld, %s = %lld\n",
                    actual_expr, actual, expected_expr, expected);
            break;

        case KRITIC_ASSERT_EQ_FLOAT:
            u_actual.i = actual;
            u_expected.i = expected;
            actual_f = u_actual.f;
            expected_f = u_expected.f;
            delta = fabs(actual_f - expected_f);

            fprintf(stderr, "%s  %s.%s: %s = %s failed at %s:%d\n",
                    label, ctx->suite, ctx->test, actual_expr, expected_expr, ctx->file, ctx->line);
            fprintf(stderr, "[      ]  -> %s = %.10f, %s = %.10f\n",
                    actual_expr, actual_f, expected_expr, expected_f);
            fprintf(stderr, "[      ]  -> delta = %.10f\n", delta);
            break;

        case KRITIC_ASSERT_NE_INT:
            fprintf(stderr, "%s  %s.%s: %s != %s failed at %s:%d\n",
                    label, ctx->suite, ctx->test, actual_expr, expected_expr, ctx->file, ctx->line);
            fprintf(stderr, "[      ]  -> both = %lld\n", actual);
            break;

        case KRITIC_ASSERT_NE_FLOAT:
            u_actual.i = actual;
            u_expected.i = expected;
            actual_f = u_actual.f;
            expected_f = u_expected.f;
            delta = fabs(actual_f - expected_f);

            fprintf(stderr, "%s  %s.%s: %s != %s failed at %s:%d\n",
                    label, ctx->suite, ctx->test, actual_expr, expected_expr, ctx->file, ctx->line);
            fprintf(stderr, "[      ]  -> %s = %.10f, %s = %.10f\n",
                    actual_expr, actual_f, expected_expr, expected_f);
            fprintf(stderr, "[      ]  -> delta = %.10f\n", delta);
            break;

        case KRITIC_ASSERT_EQ_STR:
        case KRITIC_ASSERT_NE_STR:
            actual_s = (const char*)(uintptr_t)actual;
            expected_s = (const char*)(uintptr_t)expected;

            const char* op = (assert_type == KRITIC_ASSERT_EQ_STR) ? "==" : "!=";

            fprintf(stderr, "%s  %s.%s: %s %s %s failed at %s:%d\n",
                    label, ctx->suite, ctx->test, actual_expr, op, expected_expr, ctx->file, ctx->line);
            fprintf(stderr, "[      ]  -> %s = \"%s\", %s = \"%s\"\n",
                    actual_expr, actual_s ? actual_s : "(null)",
                    expected_expr, expected_s ? expected_s : "(null)");
            break;

        case KRITIC_ASSERT:
            fprintf(stderr, "%s  %s.%s: assertion failed: %s at %s:%d\n",
                    label, ctx->suite, ctx->test, actual_expr, ctx->file, ctx->line);
            fprintf(stderr, "[      ]  -> value = %lld\n", actual);
            break;

        case KRITIC_ASSERT_NOT:
            fprintf(stderr, "%s  %s.%s: assertion expected to fail: %s at %s:%d\n",
                    label, ctx->suite, ctx->test, actual_expr, ctx->file, ctx->line);
            fprintf(stderr, "[      ]  -> value = %lld (was truthy)\n", actual);
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
    if (state->test_state->skipped) return;
    double duration_ms = (double) state->test_state->duration_ns / 1000000.0;
    int total_asserts = state->test_state->assert_count;
    int failed_asserts = state->test_state->asserts_failed;
    int passed_asserts = total_asserts - failed_asserts;
    const char* color = (failed_asserts > 0) ? "\033[1;31m" : "\033[1;32m";
    const char* label = (failed_asserts > 0) ? "FAIL" : "PASS";

    if (duration_ms < 0.001) {
        printf("[ %s%s\033[0m ] %s.%s (%s%d\033[0m/%d) in less than 0.001ms\n",
            color,
            label,
            KRITIC_GET_CURRENT_SUITE(),
            KRITIC_GET_CURRENT_TEST(),
            color,
            passed_asserts,
            total_asserts
        );
    } else if (state->test_state->duration_ns == UINT64_MAX) {
        /* Timer is disabled (or the test ran for 584.5 years) */
        printf("[ %s%s\033[0m ] %s.%s (%s%d\033[0m/%d)\n",
            color,
            label,
            KRITIC_GET_CURRENT_SUITE(),
            KRITIC_GET_CURRENT_TEST(),
            color,
            passed_asserts,
            total_asserts
        );
    } else {
        printf("[ %s%s\033[0m ] %s.%s (%s%d\033[0m/%d) in %.3fms\n",
            color,
            label,
            KRITIC_GET_CURRENT_SUITE(),
            KRITIC_GET_CURRENT_TEST(),
            color,
            passed_asserts,
            total_asserts,
            duration_ms
        );
    }
}

void kritic_default_summary_printer(kritic_runtime_t* state) {
    const char* RESET  = "\033[0m";
    const char* GREEN  = "\033[32m";
    const char* RED    = "\033[31m";
    const char* CYAN   = "\033[36m";

    uint32_t passed = state->test_count - state->fail_count;
    float pass_rate = state->test_count > 0
        ? 100.0f * (float) passed / (float) state->test_count
        : 0.0f;

    double duration_ms = (double) state->duration_ns / 1000000.0;

    char buffer[512];
    int len = snprintf(buffer, sizeof(buffer),
        "[      ] Finished running %d tests!\n"
        "[      ]\n"
        "[      ] Statistics:\n"
        "[      ]   Total  : %d\n"
        "[      ]   Passed : %s%d%s\n"
        "[      ]   Failed : %s%d%s\n"
        "[      ]   Rate   : %s%.1f%%%s\n"
        "[      ]   Time   : %.3fms\n"
        "[      ]\n"
        "%s\n",
        state->test_count,
        state->test_count,
        GREEN, passed, RESET,
        RED, state->fail_count, RESET,
        CYAN, (double) pass_rate, RESET,
        duration_ms,
        state->fail_count > 0
            ? "[ \033[1;31m!!!!\033[0m ] Some tests failed!"
            : "[ \033[1;32m****\033[0m ] All tests passed!"
    );

    if (len > 0 && len < (int) sizeof(buffer)) {
        _write(1, buffer, (uint32_t) len);
    }
}

void kritic_default_init_printer(kritic_runtime_t* state) {
    printf(
        "[      ]\n"
        "[      ] KritiC v%i.%i.%i\n"
        "[      ]\n",
        KRITIC_VERSION_MAJOR,
        KRITIC_VERSION_MINOR,
        KRITIC_VERSION_PATCH
    );
    if (state->test_count == 0) {
        printf("[      ] No registered test found\n");
    } else {
        printf("[      ] Running %d tests:\n", state->test_count);
    }
}

void kritic_default_stdout_printer(kritic_runtime_t* state, kritic_redirect_ctx_t* redir_ctx) {
    (void) state;
    if (!redir_ctx->is_part_of_split) {
        _write(redir_ctx->stdout_copy, "[ \033[34mINFO\033[0m ] ", 18);
    }
    _write(redir_ctx->stdout_copy, redir_ctx->string, redir_ctx->length);
}

void kritic_default_skip_printer(kritic_runtime_t* state, const kritic_context_t* ctx) {
    char buffer[4096];
    int len;
    double duration_ms = (double) state->test_state->duration_ns / 1000000.0;

    if (duration_ms < 0.001) {
        len = snprintf(buffer, sizeof(buffer),
            "[ SKIP ] Reason: %s at %s:%d after less than 0.001ms\n",
            state->test_state->skip_reason,
            ctx->file,
            ctx->line
        );
    } else {
        len = snprintf(buffer, sizeof(buffer),
            "[ SKIP ] Reason: %s at %s:%d after %.3fms\n",
            state->test_state->skip_reason,
            ctx->file,
            ctx->line,
            duration_ms
        );
    }

    if (len > 0 && len < (int) sizeof(buffer)) {
        _write(state->redirect->stdout_copy, buffer, (uint32_t) len);
    }
}

void kritic_default_dep_fail_printer(kritic_runtime_t* state, kritic_test_t* test, kritic_test_t* dep_test) {
    (void) state;
    printf("[ SKIP ] Test \"%s.%s\" at %s:%d is being skipped because underlying dependency \"%s.%s\" failed\n",
        test->suite, test->name, test->file, test->line, dep_test->suite, dep_test->name);
}

/* Default KritiC main(void) code used to initialize the framework */
int __attribute__((weak)) main(void) {
    kritic_enable_ansi();
    kritic_set_default_printers();
    return kritic_run_all();
}
