#include <inttypes.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <kritic/kritic.h>

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
            kritic_error_printer("[      ] Error: Test found in queue that has not yet been queued!\n");
            continue;
        }

        size_t iterations = 1;
        if ((*t)->parameterized[0] != NULL) {
            iterations = (*t)->parameterized[0]->size;
        }

        /* Check if dependencies succeeded */
        for (kritic_test_index_t** ptr = (*t)->dependencies; *ptr != NULL; ++ptr) {
            switch ((*ptr)->test_ptr->status) {
                case KRITIC_REGISTERED:
                case KRITIC_QUEUED:
                case KRITIC_RUNNING:
                    kritic_error_printerf("[      ] Error: Dependency \"%s.%s\" for test \"%s.%s\" did not run yet!\n",
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
                    kritic_error_printerf("[      ] Error: Test with unknown state found: \"%s.%s\"\n",
                        (*t)->suite, (*t)->name);
                    return 3;
            }
        }

        (*t)->status = KRITIC_RUNNING;
        kritic_redirect_start(kritic_state);
        kritic_timer_start(&kritic_state->test_state->timer);
        for (size_t i = 0; i < iterations; i++) {
            (*t)->fn();
            ++kritic_state->test_state->iteration;
        }
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

/* Default KritiC main(void) code used to initialize the framework */
int __attribute__((weak)) main(void) {
    kritic_enable_ansi();
    kritic_set_default_printers();
    return kritic_run_all();
}
