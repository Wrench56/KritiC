#ifndef KRITIC_ATTRIBUTES_H
#define KRITIC_ATTRIBUTES_H

#ifdef __cplusplus
extern "C" {
#endif

struct kritic_test_t;
struct kritic_test_index_t;

typedef enum {
    KRITIC_ATTR_UNKNOWN = 0,
    KRITIC_ATTR_DEPENDS_ON
} kritic_attr_type_t;

typedef struct {
    const char* suite;
    const char* test;
} kritic_attr_depends_on_t;

typedef union {
    kritic_attr_depends_on_t depends_on;
} kritic_attr_union;

typedef struct kritic_attribute_t {
    kritic_attr_type_t type;
    kritic_attr_union attribute;
} kritic_attribute_t;

void kritic_parse_attr_data(struct kritic_test_t* test, size_t attr_count, kritic_attribute_t** attrs);
void kritic_free_attributes(struct kritic_test_t* test);

#define KRITIC_DEPENDS_ON(_suite, _name) \
    &(kritic_attribute_t){ \
        .type = KRITIC_ATTR_DEPENDS_ON, \
        .attribute.depends_on = { .suite = #_suite, .test = #_name } \
    }

#ifdef __cplusplus
} // extern "C"
#endif

#endif // KRITIC_ATTRIBUTES_H
