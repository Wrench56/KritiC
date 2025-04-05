#ifndef __KRITIC_HEADER_GUARD
#define __KRITIC_HEADER_GUARD

#ifdef __cplusplus
extern "C" {
#endif

/* Structs and types */
typedef struct {
    const char* suite;
    const char* name;
    void (*fn)(void);
} kritic_test_t;

typedef void (*kritic_test_fn)(void);

/* API */
void kritic_register(const char* suite, const char* name, void (*fn)(void));
int kritic_run_all(void);

/* Macros */
#define _KRITIC_TEST_NAME(suite, name) kritic_test_##suite##_##name
#define _KRITIC_REGISTER_NAME(suite, name) kritic_register_##suite##_##name

#define KRITIC_TEST(suite, name)                                                        \
    static void _KRITIC_TEST_NAME(suite, name)(void);                                   \
    __attribute__((constructor)) static void _KRITIC_REGISTER_NAME(suite, name)(void) { \
        kritic_register(#suite, #name, _KRITIC_TEST_NAME(suite, name));                 \
    }                                                                                   \
    static void _KRITIC_TEST_NAME(suite, name)(void)

#ifdef __cplusplus
} // extern "C"
#endif

#endif // __KRITIC_HEADER_GUARD
