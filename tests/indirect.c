#include "../kritic.h"

static void assert_zero(void) { KRITIC_ASSERT(0); }
static void assert_one(void)  { KRITIC_ASSERT(1); }

static void assert_nested_level_three(int pass) {
    if (pass)
        assert_one();
    else
        assert_zero();
}

static void assert_multiple_conditions(int a, int b) {
    KRITIC_ASSERT(a > 0);
    KRITIC_ASSERT(b > 0);
}

static void assert_recursive(int depth, int fail_at) {
    if (depth == 0) return;
    if (depth == fail_at)
        KRITIC_ASSERT(0);
    assert_recursive(depth - 1, fail_at);
}

static void assert_through_pointer(int pass) {
    if (pass)
        assert_one();
    else
        assert_zero();
}

static void assert_on_even(int value, int pass) {
    if (value % 2 == 0) {
        if (pass) {
            KRITIC_ASSERT(1);
        } else {
            KRITIC_ASSERT(0);
        }
    }
}

typedef struct {
    const char* name;
    int (*check)(void);
} fake_t;

static int always_fail(void) {
    KRITIC_ASSERT(0);
    return 1;
}

static int always_pass(void) {
    KRITIC_ASSERT(1);
    return 1;
}


/* Direct assertion tests */
KRITIC_TEST(indirect, direct_fail) {
    assert_zero();
}

KRITIC_TEST(indirect, direct_pass) {
    assert_one();
}

/* Nested function calls */
KRITIC_TEST(indirect, nested_fail) {
    assert_nested_level_three(0);
}

KRITIC_TEST(indirect, nested_pass) {
    assert_nested_level_three(1);
}

/* Multiple condition assertions */
KRITIC_TEST(indirect, multiple_fail) {
    assert_multiple_conditions(-1, 5);
}

KRITIC_TEST(indirect, multiple_pass) {
    assert_multiple_conditions(1, 1);
}

/* Recursive assertion paths */
KRITIC_TEST(indirect, recursion_fail) {
    assert_recursive(3, 1);
}

KRITIC_TEST(indirect, recursion_pass) {
    assert_recursive(3, -1);
}

/* Function pointer indirection */
KRITIC_TEST(indirect, pointer_fail) {
    assert_through_pointer(0);
}

KRITIC_TEST(indirect, pointer_pass) {
    assert_through_pointer(1);
}

/* Loop with helper assertions */
KRITIC_TEST(indirect, loop_helper_fail) {
    for (int i = 0; i < 4; ++i)
        assert_on_even(i, 0);
}

KRITIC_TEST(indirect, loop_helper_pass) {
    for (int i = 0; i < 4; ++i)
        assert_on_even(i, 1);
}

/* Struct-based dispatch */
KRITIC_TEST(indirect, struct_fail) {
    fake_t obj = { .name = "fail", .check = always_fail };
    obj.check();
}

KRITIC_TEST(indirect, struct_pass) {
    fake_t obj = { .name = "pass", .check = always_pass };
    obj.check();
}
