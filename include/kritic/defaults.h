#ifndef KRITIC_DEFAULTS_H
#define KRITIC_DEFAULTS_H

#include <kritic/assert_types.h>

#include <kritic/scheduler.h>
#include <kritic/redirect.h>

struct kritic_runtime_t;
struct kritic_test_t;

typedef void (*kritic_assert_printer_fn)(
    const kritic_context_t* ctx,
    bool passed,
    long long actual,
    long long expected,
    const char* actual_expr,
    const char* expected_expr,
    kritic_assert_type_t assert_type
);
typedef void (*kritic_init_printer_fn)(struct kritic_runtime_t* state);
typedef void (*kritic_summary_printer_fn)(struct kritic_runtime_t* state);
typedef void (*kritic_pre_test_printer_fn)(struct kritic_runtime_t* state);
typedef void (*kritic_post_test_printer_fn)(struct kritic_runtime_t* state);
typedef void (*kritic_stdout_printer_fn)(struct kritic_runtime_t* _, kritic_redirect_ctx_t* redir_ctx);
typedef void (*kritic_skip_printer_fn)(struct kritic_runtime_t* state, const kritic_context_t* ctx);
typedef void (*kritic_dep_fail_printer_fn)(struct kritic_runtime_t* state, kritic_test_t* test, kritic_test_t* dep_test);

typedef struct {
    kritic_assert_printer_fn assert_printer;
    kritic_pre_test_printer_fn pre_test_printer;
    kritic_post_test_printer_fn post_test_printer;
    kritic_summary_printer_fn summary_printer;
    kritic_init_printer_fn init_printer;
    kritic_stdout_printer_fn stdout_printer;
    kritic_skip_printer_fn skip_printer;
    kritic_dep_fail_printer_fn dep_fail_printer;
} kritic_printers_t;

void kritic_default_assert_printer(
    const kritic_context_t* ctx,
    bool passed,
    long long actual,
    long long expected,
    const char* actual_expr,
    const char* expected_expr,
    kritic_assert_type_t assert_type
);
void kritic_default_pre_test_printer(struct kritic_runtime_t* state);
void kritic_default_post_test_printer(struct kritic_runtime_t* state);
void kritic_default_summary_printer(struct kritic_runtime_t* state);
void kritic_default_init_printer(struct kritic_runtime_t* state);
void kritic_default_stdout_printer(struct kritic_runtime_t* _, kritic_redirect_ctx_t* redir_ctx);
void kritic_default_skip_printer(struct kritic_runtime_t* state, const kritic_context_t* ctx);
void kritic_default_dep_fail_printer(struct kritic_runtime_t* state, kritic_test_t* test, kritic_test_t* dep_test);

/* These macros are used to print by default, can be overridden */
#ifndef kritic_error_printer
#define kritic_error_printer(f) fprintf(stderr, f)
#endif

#ifndef kritic_error_printerf
#define kritic_error_printerf(f, ...) fprintf(stderr, f __VA_OPT__(,) __VA_ARGS__)
#endif

#ifndef kritic_printer
#define kritic_printer(f) printf(f)
#endif

#ifndef kritic_printerf
#define kritic_printerf(f, ...) printf(f __VA_OPT__(,) __VA_ARGS__)
#endif

#ifndef kritic_snprintf
#define kritic_snprintf(buf, size, fmt, ...) snprintf(buf, size, fmt, __VA_ARGS__)
#endif

#endif // KRITIC_DEFAULTS_H
