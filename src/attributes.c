#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <kritic/kritic.h>

void kritic_parse_attr_data(kritic_test_t* test, size_t attr_count, kritic_attribute_t** attrs) {
    for (size_t i = 0; i < attr_count; ++i) {
        const kritic_attribute_t* attr = attrs[i];
        switch (attr->type) {
            case KRITIC_ATTR_DEPENDS_ON: {
                const kritic_attr_depends_on_t depends_on = attr->attribute.depends_on;
                for (size_t j = 0; j < KRITIC_MAX_DEPENDENCIES; ++j) {
                    kritic_test_index_t* dep = test->dependencies[j];

                    if (dep == NULL) {
                        kritic_test_index_t* new_dep = malloc(sizeof(kritic_test_index_t));
                        if (new_dep == NULL) {
                            fprintf(stderr, "[      ] Error: malloc() failed in kritic_parse_attr_data()");
                            exit(1);
                        }
                        new_dep->suite = depends_on.suite;
                        new_dep->name = depends_on.test;

                        test->dependencies[j] = new_dep;
                        goto next_attr;
                    }

                    if (strcmp(dep->suite, depends_on.suite) == 0 &&
                        strcmp(dep->name, depends_on.test) == 0) {
                        fprintf(stderr,
                            "[      ] Warning: duplicate dependency \"%s.%s\" for test \"%s.%s\" in %s:%d\n"
                            "[      ] Skipping duplicate\n",
                            depends_on.suite, depends_on.test, test->suite, test->name, test->file, test->line);
                        goto next_attr;
                    }
                }

                fprintf(stderr,
                    "[      ] Error: Too many dependencies for test \"%s.%s\" in %s:%d\n",
                    test->suite, test->name, test->file, test->line);
                exit(1);
            }
            case KRITIC_ATTR_PARAMETERIZED: {
                const kritic_attr_parameterized_t param = attr->attribute.parameterized;
                for (size_t j = 0; j < KRITIC_MAX_PARAMETERIZED; ++j) {
                    /* Reached the end of the array */
                    if (test->parameterized[j] == NULL) {
                        kritic_attr_parameterized_t* new_param = malloc(sizeof(kritic_attr_parameterized_t));
                        if (new_param == NULL) {
                            fprintf(stderr, "[      ] Error: malloc() failed in kritic_parse_attr_data()");
                            exit(2);
                        }
                        memcpy(new_param, &param, sizeof(kritic_attr_parameterized_t));

                        test->parameterized[j] = new_param;
                        goto next_attr;
                    }

                    /* Duplicate varname found */
                    if (strcmp(test->parameterized[j]->varname, param.varname) == 0) {
                        fprintf(stderr, "[      ] Error: Parameterized attribute defined with same variable name \"%s\"",
                            param.varname);
                        exit(2);
                    }
                }

                fprintf(stderr,
                    "[      ] Error: Too many parameterized attributes for test \"%s.%s\" in %s:%d\n",
                    test->suite, test->name, test->file, test->line);
                exit(2);
            }
            case KRITIC_ATTR_UNKNOWN:
            default:
                fprintf(stderr, "[      ] Error: Unknown attribute type detected\n");
                break;
        }

        /* Skip to the next attribute */
        next_attr:
            continue;
    }
}

void* kritic_get_param_value(const char* varname) {
    kritic_runtime_t* runtime = kritic_get_runtime_state();
    kritic_test_state_t* state = runtime->test_state;
    const kritic_test_t* test = state->test;

    /* Get PARAMETERIZED attribute count */
    size_t param_count = 0;
    for (size_t i = 0; i < KRITIC_MAX_PARAMETERIZED; ++i) {
        if (test->parameterized[i] == NULL) break;
        param_count++;
    }

    if (param_count == 0) {
        fprintf(stderr, "[      ] Error: No parameterized attributes defined\n");
        exit(1);
    }

    /* Lookup varname and return value */
    for (size_t j = 0; j < param_count; ++j) {
        const kritic_attr_parameterized_t* param = test->parameterized[j];
        if (strcmp(param->varname, varname) == 0) {
            return (((uint8_t*) param->array) + state->iteration * param->elem_size);
        }
    }

    fprintf(stderr, "[      ] Error: Parameter \"%s\" not found\n", varname);
    exit(1);
}

void kritic_free_attributes(struct kritic_test_t* test) {
    for (size_t i = 0; i < KRITIC_MAX_DEPENDENCIES; i++){
        if (test->dependencies[i] == NULL) break;
        free(test->dependencies[i]); 
    }
    for (size_t i = 0; i < KRITIC_MAX_PARAMETERIZED; i++) {
        if (test->parameterized[i] == NULL) break;
        if (test->parameterized[i]->is_static_array) break;
        free(test->parameterized[i]); 
    }
}
