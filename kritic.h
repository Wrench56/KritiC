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

#ifdef __cplusplus
} // extern "C"
#endif

#endif // __KRITIC_HEADER_GUARD
