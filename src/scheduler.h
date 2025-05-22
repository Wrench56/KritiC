#ifndef KRITIC_SCHEDULER_H
#define KRITIC_SCHEDULER_H

#define KRITIC_MAX_DEPENDENCIES 4

#ifdef __cplusplus
extern "C" {
#endif

struct kritic_runtime_t;

typedef enum {
    KRITIC_UNKNOWN = 0,
    KRITIC_REGISTERED,
    KRITIC_QUEUED,
    KRITIC_RUNNING,
    KRITIC_SKIPPED,
    KRITIC_FAILED,
    KRITIC_DEP_FAILED,
    KRITIC_PASSED,
} kritic_test_status_t;

struct kritic_runtime_t;
struct kritic_attribute_t;
struct kritic_test_t;
typedef void (*kritic_test_fn)(void);

typedef struct kritic_test_index_t {
    const char* suite;
    const char* name;
    size_t index;
    struct kritic_test_t* test_ptr;
} kritic_test_index_t;


typedef struct kritic_test_t {
    const char* file;
    const char* suite;
    const char* name;
    int line;
    kritic_test_fn fn;
    kritic_test_index_t* dependencies[KRITIC_MAX_DEPENDENCIES];
    kritic_test_status_t status;
} kritic_test_t;

typedef struct {
    const char* file;
    const char* suite;
    const char* test;
    const int line;
} kritic_context_t;

typedef struct kritic_linkedlist_node {
    struct kritic_linkedlist_node* node;
    void* data;
} kritic_node_t;

void kritic_register(const kritic_context_t* ctx, kritic_test_fn fn, size_t attr_count, kritic_attribute_t** attrs);
size_t kritic_construct_queue(struct kritic_runtime_t* runtime);
void kritic_free_queue(struct kritic_runtime_t* runtime);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // KRITIC_SCHEDULER_H
