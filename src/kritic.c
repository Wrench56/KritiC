#include <stdio.h>
#include <stdlib.h>

#include "../kritic.h"

#define KRITIC_MAX_TESTS 1024

static kritic_test_t kritic_tests[KRITIC_MAX_TESTS];
static int kritic_test_count = 0;

/* Register a test function to a specific suite with a specific name */
void kritic_register(const char *suite, const char *name, kritic_test_fn fn) {
    if (kritic_test_count >= KRITIC_MAX_TESTS) {
        fprintf(stderr, "[kritic] Too many registered tests!\n");
        exit(1);
    }

    kritic_tests[kritic_test_count++] = (kritic_test_t) {
        .suite = suite,
        .name = name,
        .fn = fn
    };
}

/* Run all of the test suites and tests */
int kritic_run_all(void) {
    int failed = 0;

    if (kritic_test_count == 0) {
        printf("[kritic] No registered test found\n");
    } else {
        printf("[kritic] Running %d tests:\n", kritic_test_count);
    }

    for (int i = 0; i < kritic_test_count; ++i) {
        const kritic_test_t *t = &kritic_tests[i];

        printf("-> %s.%s\n", t->suite, t->name);

        t->fn();
    }

    printf("[kritic] Finished running %d tests\n", kritic_test_count);

    return failed > 0 ? 1 : 0;
}

int main(void) {
    printf("Hello World!\n");
    return kritic_run_all();
}
