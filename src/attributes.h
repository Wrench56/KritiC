#ifndef KRITIC_ATTRIBUTES_H
#define KRITIC_ATTRIBUTES_H

#ifdef __cplusplus
extern "C" {
#endif

struct kritic_test_t;
struct kritic_test_index_t;

typedef enum {
    KRITIC_ATTR_UNKNOWN = 0,
    KRITIC_ATTR_DEPENDS_ON,
    KRITIC_ATTR_PARAMETERIZED
} kritic_attr_type_t;

typedef enum {
    KRITIC_SIMPLE_PARAMETERIZED,
} kritic_parameterized_type_t;

typedef struct {
    const char* suite;
    const char* test;
} kritic_attr_depends_on_t;

typedef struct {
    const char* varname;
    void* array;
    size_t size;
    size_t elem_size;
    kritic_parameterized_type_t kind;
    bool is_static_array;
} kritic_attr_parameterized_t;

typedef union {
    kritic_attr_depends_on_t depends_on;
    kritic_attr_parameterized_t parameterized;
} kritic_attr_union;

typedef struct kritic_attribute_t {
    kritic_attr_type_t type;
    kritic_attr_union attribute;
} kritic_attribute_t;

void kritic_parse_attr_data(struct kritic_test_t* test, size_t attr_count, kritic_attribute_t** attrs);
void kritic_free_attributes(struct kritic_test_t* test);
void* kritic_get_param_value(const char* varname);

#define KRITIC_DEPENDS_ON(_suite, _name)                                                                      \
    &(kritic_attribute_t){                                                                                    \
        .type = KRITIC_ATTR_DEPENDS_ON,                                                                       \
        .attribute.depends_on = { .suite = #_suite, .test = #_name }                                          \
    }

#define KRITIC_PARAMETERIZED(_varname, _type, _array, _size)                                                  \
    &(kritic_attribute_t){                                                                                    \
        .type = KRITIC_ATTR_PARAMETERIZED,                                                                    \
        .attribute.parameterized = {                                                                          \
            .varname = #_varname,                                                                             \
            .array = _array,                                                                                  \
            .size = _size,                                                                                    \
            .elem_size = sizeof(_type),                                                                       \
            .kind = KRITIC_SIMPLE_PARAMETERIZED,                                                              \
            .is_static_array = true                                                                           \
        }                                                                                                     \
    }                                                                                                         \

#define KRITIC_GET_PARAMETER(_type, _varname)                                                                 \
    (*(_type *) kritic_get_param_value(#_varname))

#ifdef __cplusplus
} // extern "C"
#endif

#endif // KRITIC_ATTRIBUTES_H
