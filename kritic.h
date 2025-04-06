#ifndef __KRITIC_HEADER_GUARD
#define __KRITIC_HEADER_GUARD

#include <stdbool.h>
#include <stddef.h>

#define KRITIC_VERSION_MAJOR 0
#define KRITIC_VERSION_MINOR 1
#define KRITIC_VERSION_PATCH 0

#ifdef __cplusplus
extern "C" {
#endif

extern const char* kritic_current_suite;
extern const char* kritic_current_test;

/* Structs, types, and enums */
typedef enum {
    KRITIC_ASSERT_UNKNOWN = 0,
    KRITIC_ASSERT,
    KRITIC_ASSERT_FAIL,
    KRITIC_ASSERT_EQ,
    KRITIC_ASSERT_NE
} kritic_assert_type_t;

typedef struct {
    const char* file;
    const char* suite;
    const char* test;
    const int line;
} kritic_context_t;

typedef void (*kritic_test_fn)(void);

typedef void (*kritik_assert_printer_fn)(
    const kritic_context_t* ctx,
    bool passed,
    long long actual,
    long long expected,
    const char* actual_expr,
    const char* expected_expr,
    kritic_assert_type_t assert_type
);

typedef struct {
    const char* file;
    const char* suite;
    const char* name;
    int line;
    kritic_test_fn fn;
} kritic_test_t;

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
void _kritic_default_assert_printer(
    const kritic_context_t* ctx,
    bool passed,
    long long actual,
    long long expected,
    const char* actual_expr,
    const char* expected_expr,
    kritic_assert_type_t assert_type
);

#ifdef _WIN32
void kritic_enable_ansi(void);
#endif

/* Macros */
#define _KRITIC_TEST_NAME(suite, name) kritic_test_##suite##_##name
#define _KRITIC_REGISTER_NAME(suite, name) kritic_register_##suite##_##name

#define KRITIC_TEST(suite, name)                                                              \
    static void _KRITIC_TEST_NAME(suite, name)(void);                                         \
    __attribute__((constructor)) static void _KRITIC_REGISTER_NAME(suite, name)(void) {       \
        static const kritic_context_t ctx = {__FILE__, #suite, #name, __LINE__};              \
        kritic_register(&ctx, _KRITIC_TEST_NAME(suite, name));                                \
    }                                                                                         \
    static void _KRITIC_TEST_NAME(suite, name)(void)

#ifdef __cplusplus
} // extern "C"
#endif

#endif // __KRITIC_HEADER_GUARD
