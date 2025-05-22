#include <stdio.h>
#include <stdlib.h>

#include "../kritic.h"

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

                /* Skip to the next attribute */
                next_attr:
                    continue;
            }
            case KRITIC_ATTR_UNKNOWN:
            default:
                fprintf(stderr, "[      ] Error: Unknown attribute type detected\n");
                break;
        }
    }
}

void kritic_free_attributes(struct kritic_test_t* test) {
    for (size_t i = 0; i < KRITIC_MAX_DEPENDENCIES; i++) {
        if (test->dependencies[i] == NULL) break;
        free(test->dependencies[i]);
    }
}
