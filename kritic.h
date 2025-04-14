#ifndef __KRITIC_HEADER_GUARD
#define __KRITIC_HEADER_GUARD

#include <stdbool.h>
#include <stddef.h>

#include "src/redirect.h"

#define KRITIC_VERSION_MAJOR 0
#define KRITIC_VERSION_MINOR 1
#define KRITIC_VERSION_PATCH 0

#define KRITIC_MAX_TESTS 1024

#ifdef __cplusplus
extern "C" {
#endif

/* Structs, types, and enums */
typedef struct kritic_runtime_t kritic_runtime_t;

typedef enum {
    KRITIC_ASSERT_UNKNOWN = 0,
    KRITIC_ASSERT,
    KRITIC_ASSERT_NOT,
    KRITIC_ASSERT_EQ,
    KRITIC_ASSERT_NE,
    KRITIC_ASSERT_FAIL
} kritic_assert_type_t;

typedef struct {
    const char* file;
    const char* suite;
    const char* test;
    const int line;
} kritic_context_t;

typedef void (*kritic_test_fn)(void);
typedef void (*kritic_assert_printer_fn)(
    const kritic_context_t* ctx,
    bool passed,
    long long actual,
    long long expected,
    const char* actual_expr,
    const char* expected_expr,
    kritic_assert_type_t assert_type
);

typedef void (*kritic_init_printer_fn)(kritic_runtime_t* state);
typedef void (*kritic_summary_printer_fn)(kritic_runtime_t* state);
typedef void (*kritic_pre_test_printer_fn)(kritic_runtime_t* state);
typedef void (*kritic_post_test_printer_fn)(kritic_runtime_t* state);
typedef void (*kritic_stdout_printer_fn)(kritic_runtime_t* _, kritic_redirect_ctx_t* redir_ctx);


typedef struct {
    const char* file;
    const char* suite;
    const char* name;
    int line;
    kritic_test_fn fn;
} kritic_test_t;

typedef struct {
    const kritic_test_t* test;
    int asserts_failed;
    int assert_count;
} kritic_test_state_t;

typedef struct {
    kritic_assert_printer_fn assert_printer;
    kritic_pre_test_printer_fn pre_test_printer;
    kritic_post_test_printer_fn post_test_printer;
    kritic_summary_printer_fn summary_printer;
    kritic_init_printer_fn init_printer;
    kritic_stdout_printer_fn stdout_printer;
} kritic_printers_t;

// Globals struct
typedef struct kritic_runtime_t {
    // Pointer to a struct of printer functions
    kritic_printers_t* printers;
    // Number of registered tests
    int test_count;
    // Number of failed tests
    int fail_count;
    // Current test state
    kritic_test_state_t* test_state;
    // Array of registered tests
    kritic_test_t tests[KRITIC_MAX_TESTS];
} kritic_runtime_t;

/* API */
void kritic_register(const kritic_context_t* ctx, kritic_test_fn fn);
int kritic_run_all(void);
void kritic_assert_eq(
    const kritic_context_t* ctx,
    long long actual,
    long long expected,
    const char* actual_expr,
    const char* expected_expr,
    const kritic_assert_type_t assert_type
);
kritic_runtime_t* kritic_get_runtime_state(void);
void _kritic_default_assert_printer(
    const kritic_context_t* ctx,
    bool passed,
    long long actual,
    long long expected,
    const char* actual_expr,
    const char* expected_expr,
    kritic_assert_type_t assert_type
);
void _kritic_default_pre_test_printer(kritic_runtime_t* state);
void _kritic_default_post_test_printer(kritic_runtime_t* state);
void _kritic_default_summary_printer(kritic_runtime_t* state);
void _kritic_default_init_printer(kritic_runtime_t* state);
void _kritic_default_stdout_printer(kritic_runtime_t* _, kritic_redirect_ctx_t* redir_ctx);

#ifdef _WIN32
void kritic_enable_ansi(void);
#endif

/* Macros */
#define _KRITIC_TEST_NAME(suite, name) kritic_test_##suite##_##name
#define _KRITIC_REGISTER_NAME(suite, name) kritic_register_##suite##_##name

#define _KRITIC_GET_CURRENT_SUITE() kritic_get_runtime_state()->test_state->test->suite
#define _KRITIC_GET_CURRENT_TEST() kritic_get_runtime_state()->test_state->test->name

/* Defines and registers a test case function in the given suite */
#define KRITIC_TEST(suite, name)                                                                              \
    static void _KRITIC_TEST_NAME(suite, name)(void);                                                         \
    __attribute__((constructor)) static void _KRITIC_REGISTER_NAME(suite, name)(void) {                       \
        static const kritic_context_t ctx = {__FILE__, #suite, #name, __LINE__};                              \
        kritic_register(&ctx, _KRITIC_TEST_NAME(suite, name));                                                \
    }                                                                                                         \
    static void _KRITIC_TEST_NAME(suite, name)(void)

/* Asserts that the given expression is true */
#define KRITIC_ASSERT(expr)                                                                                   \
    do {                                                                                                      \
        kritic_context_t ctx = {__FILE__, _KRITIC_GET_CURRENT_SUITE(), _KRITIC_GET_CURRENT_TEST(), __LINE__}; \
        kritic_assert_eq(&ctx, (expr), 0, #expr, NULL, KRITIC_ASSERT);                                        \
    } while (0);

/* Asserts that the given expression is false */
#define KRITIC_ASSERT_NOT(expr)                                                                               \
    do {                                                                                                      \
    kritic_context_t ctx = {__FILE__, _KRITIC_GET_CURRENT_SUITE(), _KRITIC_GET_CURRENT_TEST(), __LINE__};     \
    kritic_assert_eq(&ctx, (expr), 0, #expr, NULL, KRITIC_ASSERT_NOT);                                        \
    } while (0);

/* Forces a test failure unconditionally */
#define KRITIC_FAIL()                                                                                         \
    do {                                                                                                      \
        kritic_context_t ctx = {__FILE__, _KRITIC_GET_CURRENT_SUITE(), _KRITIC_GET_CURRENT_TEST(), __LINE__}; \
        kritic_assert_eq(&ctx, 0, 1, "forced failure", NULL, KRITIC_ASSERT_FAIL);                             \
    } while (0);

/* Asserts that two values are equal */
#define KRITIC_ASSERT_EQ(x, y)                                                                                \
    do {                                                                                                      \
        kritic_context_t ctx = {__FILE__, _KRITIC_GET_CURRENT_SUITE(), _KRITIC_GET_CURRENT_TEST(), __LINE__}; \
        kritic_assert_eq(&ctx, (x), (y), #x, #y, KRITIC_ASSERT_EQ);                                           \
    } while (0);

/* Asserts that two values are not equal */
#define KRITIC_ASSERT_NE(x, y)                                                                                \
    do {                                                                                                      \
        kritic_context_t ctx = {__FILE__, _KRITIC_GET_CURRENT_SUITE(), _KRITIC_GET_CURRENT_TEST(), __LINE__}; \
        kritic_assert_eq(&ctx, (x), (y), #x, #y, KRITIC_ASSERT_NE);                                           \
    } while (0);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // __KRITIC_HEADER_GUARD
