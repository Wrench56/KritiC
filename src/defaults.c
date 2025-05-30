#include <math.h>
#include <stdio.h>

#include "../kritic.h"

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
        case KRITIC_ASSERT_EQ_INT:
            kritic_error_printerf("%s  %s.%s: %s == %s failed at %s:%d\n",
                    label, ctx->suite, ctx->test, actual_expr, expected_expr, ctx->file, ctx->line);
            kritic_error_printerf("[      ]  -> %s = %lld, %s = %lld\n",
                    actual_expr, actual, expected_expr, expected);
            break;

        case KRITIC_ASSERT_NE_INT:
            kritic_error_printerf("%s  %s.%s: %s != %s failed at %s:%d\n",
                    label, ctx->suite, ctx->test, actual_expr, expected_expr, ctx->file, ctx->line);
            kritic_error_printerf("[      ]  -> both = %lld\n", actual);
            break;

        case KRITIC_ASSERT_EQ_FLOAT:
        case KRITIC_ASSERT_NE_FLOAT: {
            union { long long i; double f; } u_actual, u_expected;
            u_actual.i = actual;
            u_expected.i = expected;

            double actual_f = u_actual.f;
            double expected_f = u_expected.f;
            double delta = fabs(actual_f - expected_f);

            const char* op = (assert_type == KRITIC_ASSERT_EQ_FLOAT) ? "=" : "!=";

            kritic_error_printerf("%s  %s.%s: %s %s %s failed at %s:%d\n",
                    label, ctx->suite, ctx->test, actual_expr, op, expected_expr, ctx->file, ctx->line);
            kritic_error_printerf("[      ]  -> %s = %.10f, %s = %.10f\n",
                    actual_expr, actual_f, expected_expr, expected_f);
            kritic_error_printerf("[      ]  -> delta = %.10f\n", delta);
            break;
        }

        case KRITIC_ASSERT_EQ_STR:
        case KRITIC_ASSERT_NE_STR: {
            const char* actual_s = (const char*)(uintptr_t)actual;
            const char* expected_s = (const char*)(uintptr_t)expected;

            const char* op = (assert_type == KRITIC_ASSERT_EQ_STR) ? "==" : "!=";

            kritic_error_printerf("%s  %s.%s: %s %s %s failed at %s:%d\n",
                    label, ctx->suite, ctx->test, actual_expr, op, expected_expr, ctx->file, ctx->line);
            kritic_error_printerf("[      ]  -> %s = \"%s\", %s = \"%s\"\n",
                    actual_expr, actual_s ? actual_s : "(null)",
                    expected_expr, expected_s ? expected_s : "(null)");
            break;
        }

        case KRITIC_ASSERT:
            kritic_error_printerf("%s  %s.%s: assertion failed: %s at %s:%d\n",
                    label, ctx->suite, ctx->test, actual_expr, ctx->file, ctx->line);
            kritic_error_printerf("[      ]  -> value = %lld\n", actual);
            break;

        case KRITIC_ASSERT_NOT:
            kritic_error_printerf("%s  %s.%s: assertion expected to fail: %s at %s:%d\n",
                    label, ctx->suite, ctx->test, actual_expr, ctx->file, ctx->line);
            kritic_error_printerf("[      ]  -> value = %lld (was truthy)\n", actual);
            break;

        case KRITIC_ASSERT_FAIL:
            kritic_error_printerf("%s  %s.%s: forced failure at %s:%d\n",
                    label, ctx->suite, ctx->test, ctx->file, ctx->line);
            break;

        default:
            kritic_error_printerf("%s  %s.%s: unknown assertion type at %s:%d\n",
                    label, ctx->suite, ctx->test, ctx->file, ctx->line);
            break;
    }
}

void kritic_default_pre_test_printer(kritic_runtime_t* state) {
    kritic_printerf("[ \033[1;36mEXEC\033[0m ] %s.%s at %s:%i\n", KRITIC_GET_CURRENT_SUITE(), KRITIC_GET_CURRENT_TEST(),
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
        kritic_printerf("[ %s%s\033[0m ] %s.%s (%s%d\033[0m/%d) in less than 0.001ms\n",
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
        kritic_printerf("[ %s%s\033[0m ] %s.%s (%s%d\033[0m/%d)\n",
            color,
            label,
            KRITIC_GET_CURRENT_SUITE(),
            KRITIC_GET_CURRENT_TEST(),
            color,
            passed_asserts,
            total_asserts
        );
    } else {
        kritic_printerf("[ %s%s\033[0m ] %s.%s (%s%d\033[0m/%d) in %.3fms\n",
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
    int len = kritic_snprintf(buffer, sizeof(buffer),
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
    kritic_printerf(
        "[      ]\n"
        "[      ] KritiC v%i.%i.%i\n"
        "[      ]\n",
        KRITIC_VERSION_MAJOR,
        KRITIC_VERSION_MINOR,
        KRITIC_VERSION_PATCH
    );
    if (state->test_count == 0) {
        kritic_printer("[      ] No registered test found\n");
    } else {
        kritic_printerf("[      ] Running %d tests:\n", state->test_count);
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
        len = kritic_snprintf(buffer, sizeof(buffer),
            "[ SKIP ] Reason: %s at %s:%d after less than 0.001ms\n",
            state->test_state->skip_reason,
            ctx->file,
            ctx->line
        );
    } else {
        len = kritic_snprintf(buffer, sizeof(buffer),
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
    kritic_printerf("[ SKIP ] Test \"%s.%s\" at %s:%d is being skipped because underlying dependency \"%s.%s\" failed\n",
        test->suite, test->name, test->file, test->line, dep_test->suite, dep_test->name);
}
