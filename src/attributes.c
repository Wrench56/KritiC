#include <stdio.h>
#include <stdlib.h>

#include "../kritic.h"

void kritic_parse_attr_data(kritic_test_t* test, size_t attr_count, kritic_attribute_t** attrs) {
    for (size_t i = 0; i < attr_count; ++i) {
        const kritic_attribute_t* attr = attrs[i];
        switch (attr->type) {
            case KRITIC_ATTR_DEPENDS_ON: {
                const kritic_attr_depends_on_t depends_on = attr->attribute.depends_on;
                char* expected = malloc(strlen(depends_on.suite) + strlen(depends_on.test) + 2);
                if (!expected) {
                    fprintf(stderr, "[      ] Error: malloc() failed for dependency string\n");
                    exit(1);
                }
                sprintf(expected, "%s.%s", depends_on.suite, depends_on.test);

                bool inserted = false;
                for (size_t j = 0; j < KRITIC_MAX_DEPENDENCIES; ++j) {
                    const char* dep = test->dependencies[j];
                    if (!dep) {
                        test->dependencies[j] = expected;
                        inserted = true;
                        break;
                    }
                    if (strcmp(dep, expected) == 0) {
                        fprintf(stderr,
                            "[      ] Warning: duplicate dependency \"%s\" for test \"%s.%s\" in %s:%d\n"
                            "[      ] Skipping duplicate\n",
                            expected, test->suite, test->name, test->file, test->line);
                        free(expected);
                        inserted = true;
                        break;
                    }
                }
                if (!inserted) {
                    fprintf(stderr,
                        "[      ] Error: Too many dependencies for test \"%s.%s\" in %s:%d\n",
                        test->suite, test->name, test->file, test->line);
                    free(expected);
                    exit(1);
                }
                break;
            }
            case KRITIC_ATTR_UNKNOWN:
            default:
                fprintf(stderr, "[      ] Error: Unknown attribute type detected\n");
                break;
        }
    }
}
