#ifndef KRITIC_CORE_H
#define KRITIC_CORE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "src/redirect.h"
#include "src/timer.h"
#include "src/attributes.h"
#include "src/scheduler.h"

#define KRITIC_VERSION_MAJOR 1
#define KRITIC_VERSION_MINOR 2
#define KRITIC_VERSION_PATCH 7

#define KRITIC_MAX_TESTS 1024
#define KRITIC_FLOAT_DELTA_VALUE 1e-6

#ifdef __cplusplus
extern "C" {
#endif

/* Structs, types, and enums */
typedef enum {
    KRITIC_ASSERT_UNKNOWN = 0,
    KRITIC_ASSERT,
    KRITIC_ASSERT_NOT,
    KRITIC_ASSERT_EQ_INT,
    KRITIC_ASSERT_EQ_FLOAT,
    KRITIC_ASSERT_EQ_STR,
    KRITIC_ASSERT_NE_INT,
    KRITIC_ASSERT_NE_FLOAT,
    KRITIC_ASSERT_NE_STR,
    KRITIC_ASSERT_FAIL
} kritic_assert_type_t;

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
typedef void (*kritic_skip_printer_fn)(kritic_runtime_t* state, const kritic_context_t* ctx);

typedef struct {
    const kritic_test_t* test;
    int asserts_failed;
    int assert_count;
    bool skipped;
    const char* skip_reason;
    uint64_t duration_ns;
    kritic_timer_t timer;
} kritic_test_state_t;

typedef struct {
    kritic_assert_printer_fn assert_printer;
    kritic_pre_test_printer_fn pre_test_printer;
    kritic_post_test_printer_fn post_test_printer;
    kritic_summary_printer_fn summary_printer;
    kritic_init_printer_fn init_printer;
    kritic_stdout_printer_fn stdout_printer;
    kritic_skip_printer_fn skip_printer;
} kritic_printers_t;

// Globals struct
struct kritic_runtime_t {
    // Struct of printer functions
    kritic_printers_t printers;
    // Number of registered tests
    uint32_t test_count;
    // Number of failed tests
    uint32_t fail_count;
    // Number of skipped tests
    uint32_t skip_count;
    // Current test state
    kritic_test_state_t* test_state;
    // Pointer to a null-terminated array of scheduled tests
    kritic_test_t** queue;
    // Pointer to the end of the node in the tests linked list
    kritic_node_t* last_node;
    // Pointer to the first node in the tests linked list
    kritic_node_t* first_node;
    // Current redirection struct
    kritic_redirect_t* redirect;
    // Global runtime timer
    kritic_timer_t timer;
    // Duration of KritiC run
    uint64_t duration_ns;
};

/* API */
void kritic_override_printers(const kritic_printers_t* overrides);
void kritic_set_default_printers(void);
void kritic_noop(void* _, ...);
int kritic_run_all(void);
void kritic_assert_eq(
    const kritic_context_t* ctx,
    long long actual,
    long long expected,
    const char* actual_expr,
    const char* expected_expr,
    const kritic_assert_type_t assert_type
);
void kritic_skip_test(const kritic_context_t* ctx, const char* reason);
kritic_runtime_t* kritic_get_runtime_state(void);
void kritic_default_assert_printer(
    const kritic_context_t* ctx,
    bool passed,
    long long actual,
    long long expected,
    const char* actual_expr,
    const char* expected_expr,
    kritic_assert_type_t assert_type
);
void kritic_default_pre_test_printer(kritic_runtime_t* state);
void kritic_default_post_test_printer(kritic_runtime_t* state);
void kritic_default_summary_printer(kritic_runtime_t* state);
void kritic_default_init_printer(kritic_runtime_t* state);
void kritic_default_stdout_printer(kritic_runtime_t* _, kritic_redirect_ctx_t* redir_ctx);
void kritic_default_skip_printer(kritic_runtime_t* state, const kritic_context_t* ctx);

#ifdef _WIN32
    void kritic_enable_ansi_(void);

    /* Wrapper for Windows-specific kritic_enable_ansi_() */
    #define kritic_enable_ansi() kritic_enable_ansi_()
#else // POSIX
    #include <unistd.h>

    /* Aliases */
    #define _write  write

    /* Wrapper for Windows-specific kritic_enable_ansi_() */
    #define kritic_enable_ansi()

#endif // POSIX

/* Macros */
#define KRITIC_NOOP(type) ((type)(void*)(kritic_noop))
#define KRITIC_GET_CURRENT_SUITE() kritic_get_runtime_state()->test_state->test->suite
#define KRITIC_GET_CURRENT_TEST() kritic_get_runtime_state()->test_state->test->name

#define PP_NARG(...) PP_NARG_(_,__VA_ARGS__,PP_RSEQ_N())
#define PP_NARG_(_, ...) PP_ARG_N(__VA_ARGS__)

#define PP_ARG_N( \
     _1, _2, _3, _4, _5, _6, _7, _8, _9,_10, \
    _11,_12,_13,_14,_15,_16,_17,_18,_19,_20, \
    _21,_22,_23,_24,_25,_26,_27,_28,_29,_30, \
    _31,_32,_33,_34,_35,_36,_37,_38,_39,_40, \
    _41,_42,_43,_44,_45,_46,_47,_48,_49,_50, \
    _51,_52,_53,_54,_55,_56,_57,_58,_59,_60, \
    _61,_62,_63,  N, ...) N

#define PP_RSEQ_N() \
    3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3, \
    3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3, \
    3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3, \
    3,2,0,0,0

#define KRITIC_TEST_NAME(suite, name) kritic_test_##suite##_##name
#define KRITIC_REGISTER_NAME(suite, name) kritic_register_##suite##_##name

#define CONCAT2(a, b) a##b
#define EXPAND_CONCAT2(a, b) CONCAT2(a, b)

/* Defines and registers a test case function in the given suite */
#define KRITIC_TEST(...) EXPAND_CONCAT2(KRITIC_TEST, PP_NARG(__VA_ARGS__))(__VA_ARGS__)
#define KRITIC_TEST2 KRITIC_TEST_WITH_NO_ARGS
#define KRITIC_TEST3 KRITIC_TEST_WITH_ARGS

/* Defines a test with no arguments */
#define KRITIC_TEST_WITH_NO_ARGS(suite, name)                                                                 \
static void KRITIC_TEST_NAME(suite, name)(void);                                                              \
    __attribute__((constructor)) static void KRITIC_REGISTER_NAME(suite, name)(void) {                        \
        static const kritic_context_t ctx = { __FILE__, #suite, #name, __LINE__ };                            \
        kritic_register(&ctx, KRITIC_TEST_NAME(suite, name), 0, NULL);                                        \
    }                                                                                                         \
    static void KRITIC_TEST_NAME(suite, name)(void)

/* Defines a test with arguments */
#define KRITIC_TEST_WITH_ARGS(suite, name, ...)                                                               \
    static void KRITIC_TEST_NAME(suite, name)(void);                                                          \
    __attribute__((constructor)) static void KRITIC_REGISTER_NAME(suite, name)(void) {                        \
        kritic_attribute_t* kritic_attrs[] = { __VA_ARGS__, NULL };                                           \
        size_t count = 0;                                                                                     \
        while (kritic_attrs[count]) ++count;                                                                  \
        static const kritic_context_t ctx = { __FILE__, #suite, #name, __LINE__ };                            \
        kritic_register(&ctx, KRITIC_TEST_NAME(suite, name), count, kritic_attrs);                            \
    }                                                                                                         \
    static void KRITIC_TEST_NAME(suite, name)(void)

/* Asserts that the given expression is true */
#define KRITIC_ASSERT(expr)                                                                                   \
    do {                                                                                                      \
        kritic_context_t ctx = {__FILE__, KRITIC_GET_CURRENT_SUITE(), KRITIC_GET_CURRENT_TEST(), __LINE__};   \
        kritic_assert_eq(&ctx, (expr), 0, #expr, NULL, KRITIC_ASSERT);                                        \
    } while (0);

/* Asserts that the given expression is false */
#define KRITIC_ASSERT_NOT(expr)                                                                               \
    do {                                                                                                      \
    kritic_context_t ctx = {__FILE__, KRITIC_GET_CURRENT_SUITE(), KRITIC_GET_CURRENT_TEST(), __LINE__};       \
    kritic_assert_eq(&ctx, (expr), 0, #expr, NULL, KRITIC_ASSERT_NOT);                                        \
    } while (0);

/* Forces a test failure unconditionally */
#define KRITIC_FAIL()                                                                                         \
    do {                                                                                                      \
        kritic_context_t ctx = {__FILE__, KRITIC_GET_CURRENT_SUITE(), KRITIC_GET_CURRENT_TEST(), __LINE__};   \
        kritic_assert_eq(&ctx, 0, 1, "forced failure", NULL, KRITIC_ASSERT_FAIL);                             \
    } while (0);

/* Asserts that two values are equal (generic macro for ints, floats, and strings) */
#define KRITIC_ASSERT_EQ(actual, expected)                                                                    \
    do {                                                                                                      \
        kritic_context_t ctx = {__FILE__, KRITIC_GET_CURRENT_SUITE(), KRITIC_GET_CURRENT_TEST(), __LINE__};   \
        __typeof__(actual) _a = (actual);                                                                     \
        __typeof__(expected) _b = (expected);                                                                 \
        if (                                                                                                  \
            __builtin_types_compatible_p(__typeof__(_a), const char *) ||                                     \
            __builtin_types_compatible_p(__typeof__(_a), char *) ||                                           \
            __builtin_types_compatible_p(__typeof__(_a), const char[]) ||                                     \
            __builtin_types_compatible_p(__typeof__(_a), char[])                                              \
        ) {                                                                                                   \
            kritic_assert_eq(&ctx, (long long)(uintptr_t) _a, (long long)(uintptr_t) _b,                      \
                #actual, #expected, KRITIC_ASSERT_EQ_STR);                                                    \
        } else if (                                                                                           \
            __builtin_types_compatible_p(__typeof__(_a), float) ||                                            \
            __builtin_types_compatible_p(__typeof__(_a), double)                                              \
        ) {                                                                                                   \
            long long _a_bits, _b_bits;                                                                       \
            memcpy(&_a_bits, &_a, sizeof(_a));                                                                \
            memcpy(&_b_bits, &_b, sizeof(_b));                                                                \
            kritic_assert_eq(&ctx, _a_bits, _b_bits, #actual, #expected, KRITIC_ASSERT_EQ_FLOAT);             \
        } else {                                                                                              \
            kritic_assert_eq(&ctx, (long long) _a, (long long) _b, #actual, #expected,                        \
                KRITIC_ASSERT_EQ_INT);                                                                        \
        }                                                                                                     \
    } while (0);

/* Asserts that two integer values are equal */
#define KRITIC_ASSERT_EQ_INT(x, y)                                                                            \
    do {                                                                                                      \
        kritic_context_t ctx = {__FILE__, KRITIC_GET_CURRENT_SUITE(), KRITIC_GET_CURRENT_TEST(), __LINE__};   \
        kritic_assert_eq(&ctx, (x), (y), #x, #y, KRITIC_ASSERT_EQ_INT);                                       \
    } while (0);

/* Asserts that two floats/doubles are approximately equal */
#define KRITIC_ASSERT_EQ_FLOAT(actual, expected) \
    do {                                                                                                      \
        kritic_context_t ctx = {__FILE__, KRITIC_GET_CURRENT_SUITE(), KRITIC_GET_CURRENT_TEST(), __LINE__};   \
        double actual_val = (actual);                                                                         \
        double expected_val = (expected);                                                                     \
        kritic_assert_eq(&ctx, *(long long*)&actual_val, *(long long*)&expected_val, #actual, #expected,      \
            KRITIC_ASSERT_EQ_FLOAT);                                                                          \
    } while (0);

/* Asserts that two strings are equal */
#define KRITIC_ASSERT_EQ_STR(actual, expected) \
    do {                                                                                                      \
        kritic_context_t ctx = {__FILE__, KRITIC_GET_CURRENT_SUITE(), KRITIC_GET_CURRENT_TEST(), __LINE__};   \
        kritic_assert_eq(&ctx, (long long)(uintptr_t)(actual), (long long)(uintptr_t)(expected), #actual,     \
            #expected, KRITIC_ASSERT_EQ_STR);                                                                 \
    } while (0);

/* Asserts that two values are not equal (generic macro for ints, floats, and strings) */
#define KRITIC_ASSERT_NE(actual, expected)                                                                    \
    do {                                                                                                      \
        kritic_context_t ctx = {__FILE__, KRITIC_GET_CURRENT_SUITE(), KRITIC_GET_CURRENT_TEST(), __LINE__};   \
        __typeof__(actual) _a = (actual);                                                                     \
        __typeof__(expected) _b = (expected);                                                                 \
        if (                                                                                                  \
            __builtin_types_compatible_p(__typeof__(_a), const char *) ||                                     \
            __builtin_types_compatible_p(__typeof__(_a), char *) ||                                           \
            __builtin_types_compatible_p(__typeof__(_a), const char[]) ||                                     \
            __builtin_types_compatible_p(__typeof__(_a), char[])                                              \
        ) {                                                                                                   \
            kritic_assert_eq(&ctx, (long long)(uintptr_t) _a, (long long)(uintptr_t) _b,                      \
                #actual, #expected, KRITIC_ASSERT_NE_STR);                                                    \
        } else if (                                                                                           \
            __builtin_types_compatible_p(__typeof__(_a), float) ||                                            \
            __builtin_types_compatible_p(__typeof__(_a), double)                                              \
        ) {                                                                                                   \
            long long _a_bits, _b_bits;                                                                       \
            memcpy(&_a_bits, &_a, sizeof(_a));                                                                \
            memcpy(&_b_bits, &_b, sizeof(_b));                                                                \
            kritic_assert_eq(&ctx, _a_bits, _b_bits, #actual, #expected, KRITIC_ASSERT_NE_FLOAT);             \
        } else {                                                                                              \
            kritic_assert_eq(&ctx, (long long) _a, (long long) _b, #actual, #expected,                        \
                KRITIC_ASSERT_NE_INT);                                                                        \
        }                                                                                                     \
    } while (0);

/* Asserts that two integer values are not equal */
#define KRITIC_ASSERT_NE_INT(x, y)                                                                            \
    do {                                                                                                      \
        kritic_context_t ctx = {__FILE__, KRITIC_GET_CURRENT_SUITE(), KRITIC_GET_CURRENT_TEST(), __LINE__};   \
        kritic_assert_eq(&ctx, (x), (y), #x, #y, KRITIC_ASSERT_NE_INT);                                       \
    } while (0);

/* Asserts that two floats/doubles are approximately not equal */
#define KRITIC_ASSERT_NE_FLOAT(actual, expected)                                                              \
    do {                                                                                                      \
        kritic_context_t ctx = {__FILE__, KRITIC_GET_CURRENT_SUITE(), KRITIC_GET_CURRENT_TEST(), __LINE__};   \
        double actual_val = (actual);                                                                         \
        double expected_val = (expected);                                                                     \
        kritic_assert_eq(&ctx, *(long long*)&actual_val, *(long long*)&expected_val, #actual, #expected,      \
            KRITIC_ASSERT_NE_FLOAT);                                                                          \
    } while (0);

/* Asserts that two strings are not equal */
#define KRITIC_ASSERT_NE_STR(actual, expected) \
    do {                                                                                                      \
        kritic_context_t ctx = {__FILE__, KRITIC_GET_CURRENT_SUITE(), KRITIC_GET_CURRENT_TEST(), __LINE__};   \
        kritic_assert_eq(&ctx, (long long)(uintptr_t)(actual), (long long)(uintptr_t)(expected), #actual,     \
            #expected, KRITIC_ASSERT_NE_STR);                                                                 \
    } while (0);

/* Skip a unit test with an optional reason string */
#define KRITIC_SKIP(reason)                                                                                   \
    do {                                                                                                      \
        kritic_context_t ctx = {__FILE__, KRITIC_GET_CURRENT_SUITE(), KRITIC_GET_CURRENT_TEST(), __LINE__};   \
        kritic_skip_test(&ctx, reason);                                                                       \
        return;                                                                                               \
    } while (0);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // KRITIC_CORE_H
