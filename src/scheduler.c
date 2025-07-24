#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <kritic/kritic.h>

#define KRITIC_SORT_STACK_LIMIT 1024

/* Register a test function to a specific suite with a specific name */
void kritic_register(const kritic_context_t* ctx, kritic_test_fn fn, size_t attr_count, kritic_attribute_t** attrs) {
    kritic_runtime_t* kritic_state = kritic_get_runtime_state();
    kritic_test_t* test_metadata = malloc(sizeof(kritic_test_t));
    kritic_node_t* test_node = malloc(sizeof(kritic_node_t));

    if (!test_metadata || !test_node) {
        fprintf(stderr, "[      ] Error: malloc() returned NULL\n");
        exit(1);
    }

    *test_metadata = (kritic_test_t) {
        .file         = ctx->file,
        .suite        = ctx->suite,
        .name         = ctx->test,
        .line         = ctx->line,
        .fn           = fn,
        .dependencies = { 0 },
        .status       = KRITIC_REGISTERED
    };

    kritic_parse_attr_data(test_metadata, attr_count, attrs);
    *test_node = (kritic_node_t) { .node = NULL, .data = test_metadata };

    if (kritic_state->first_node == NULL) {
        kritic_state->first_node = test_node;
    } else {
        kritic_state->last_node->node = test_node;
    }
    kritic_state->last_node = test_node;
    kritic_state->test_count++;
}

static inline size_t find_dep_index(const kritic_test_index_t* map, size_t count, kritic_test_index_t* dep, bool* invalid) {
    (*invalid) = false;
    for (size_t i = 0; i < count; ++i) {
        if (strcmp(map[i].suite, dep->suite) == 0
            && strcmp(map[i].name, dep->name) == 0) return map[i].index;
    }

    (*invalid) = true;
    return 0;
}

static void kritic_sort_queue(kritic_runtime_t* runtime) {
    size_t count = runtime->test_count;

    bool invalid;
    int* indegree            = malloc(count * sizeof(int));
    size_t* queue            = malloc(count * sizeof(size_t));
    kritic_test_index_t* map = malloc(count * sizeof(kritic_test_index_t));
    kritic_test_t** sorted   = malloc((count + 1) * sizeof(kritic_test_t*));

    if (!indegree || !queue || !sorted || !map) {
        fprintf(stderr, "[      ] Error: Memory allocation failed during queue sorting\n");
        exit(1);
    }

    for (size_t i = 0; i < count; ++i) {
        map[i].suite = runtime->queue[i]->suite;
        map[i].name = runtime->queue[i]->name;
        map[i].index = i;
        indegree[i] = 0;
    }

    /* Perform basic checks */
    for (size_t i = 0; i < count; ++i) {
        for (size_t d = 0; runtime->queue[i]->dependencies[d]; ++d) {
            kritic_test_index_t* dep = runtime->queue[i]->dependencies[d];
            if (strcmp(dep->suite, runtime->queue[i]->suite) == 0
                && strcmp(dep->name, runtime->queue[i]->name) == 0) {
                fprintf(stderr, "[      ] Error: Test \"%s.%s\" depends on itself\n",
                    runtime->queue[i]->suite, runtime->queue[i]->name);
                exit(1);
            }

            size_t dep_index = find_dep_index(map, count, dep, &invalid);
            if (invalid == true) {
                fprintf(stderr, "[      ] Error: Test \"%s.%s\" depends on unknown \"%s.%s\"\n",
                    runtime->queue[i]->suite, runtime->queue[i]->name, dep->suite, dep->name);
                exit(1);
            }

            /* Set test_ptr of kritic_test_index_t */
            dep->test_ptr = runtime->queue[dep_index];

            indegree[i]++;
        }
    }

    /* Queue tests with no dependencies */
    size_t front = 0, back = 0;
    for (size_t i = 0; i < count; ++i) {
        if (indegree[i] == 0) {
            queue[back++] = i;
        }
    }

    /* Kahn's algorithm */
    size_t sorted_index = 0;
    while (front < back) {
        size_t idx = queue[front++];
        sorted[sorted_index++] = runtime->queue[idx];

        for (size_t j = 0; j < count; ++j) {
            for (size_t d = 0; runtime->queue[j]->dependencies[d] != NULL; ++d) {
                if (strcmp(runtime->queue[j]->dependencies[d]->suite, runtime->queue[idx]->suite) == 0
                    && strcmp(runtime->queue[j]->dependencies[d]->name, runtime->queue[idx]->name) == 0) {
                    if (--indegree[j] == 0) {
                        queue[back++] = j;
                    }
                    break;
                }
            }
        }
    }

    /* Cycle detection */
    if (sorted_index != count) {
        fprintf(stderr, "[      ] Error: Cyclic dependency detected!\n");
        for (size_t i = 0; i < count; ++i) {
            if (indegree[i] > 0) {
                kritic_test_t* t = runtime->queue[i];
                fprintf(stderr, "[      ] Test \"%s\" in suite \"%s\" at %s:%d has unresolved dependencies:\n",
                        t->name, t->suite, t->file, t->line);
                for (size_t d = 0; t->dependencies[d] != NULL; ++d) {
                    kritic_test_index_t* dep = t->dependencies[d];
                    size_t dep_index = find_dep_index(map, count, dep, &invalid);
                    if (dep_index == (size_t)-1) {
                        fprintf(stderr, "[      ]   - (missing) %s.%s\n", dep->suite, dep->name);
                    } else {
                        fprintf(stderr, "[      ]   - %s.%s (possibly part of a cycle)\n", dep->suite, dep->name);
                    }
                }
            }
        }

        free(indegree);
        free(queue);
        free(map);
        free(sorted);
        exit(1);
    }

    /* Cleanup */
    sorted[count] = NULL;
    free(runtime->queue);
    runtime->queue = sorted;

    free(indegree);
    free(queue);
    free(map);
}


size_t kritic_construct_queue(kritic_runtime_t* runtime) {
    size_t count = runtime->test_count;
    runtime->queue = malloc((count + 1) * sizeof(kritic_test_t*));
    if (runtime->queue == NULL) {
        fprintf(stderr, "[      ] Error: malloc() for queue returned NULL\n");
        exit(1);
    }

    kritic_node_t* current = runtime->first_node;
    kritic_node_t* next;
    size_t i = 0;
    while (current != NULL) {
        runtime->queue[i++] = (kritic_test_t*) current->data;
        ((kritic_test_t*) current->data)->status = KRITIC_QUEUED;
        next = current->node;
        free(current);
        current = next;
    }

    runtime->first_node = NULL;
    runtime->last_node = NULL;
    runtime->queue[i] = NULL;
    kritic_sort_queue(runtime);

    return count;
}

void kritic_free_queue(kritic_runtime_t *runtime) {
    if (!runtime || !runtime->queue) return;
    for (kritic_test_t** t = runtime->queue; *t != NULL; ++t) {
        kritic_free_attributes((*t));
        free((*t));
    }

    free(runtime->queue);
    runtime->queue = NULL;
}
