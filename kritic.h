#ifndef __KRITIC_HEADER_GUARD
#define __KRITIC_HEADER_GUARD

#define KRITIC_VERSION_MAJOR 0
#define KRITIC_VERSION_MINOR 1
#define KRITIC_VERSION_PATCH 0

#ifdef __cplusplus
extern "C" {
#endif

/* Structs and types */
typedef void (*kritic_test_fn)(void);

typedef struct {
    const char* file;
    const char* suite;
    const char* name;
    int line;
    kritic_test_fn fn;
} kritic_test_t;

typedef struct {
    const char* file;
    const char* suite;
    const char* test;
    const int line;
} kritic_context_t;

/* API */
void kritic_register(const kritic_context_t* ctx, void (*fn)(void));
int kritic_run_all(void);

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
