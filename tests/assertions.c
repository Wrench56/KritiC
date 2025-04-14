#include <stdio.h>
#include <stdlib.h>

#include "../kritic.h"

/* KRITIC_ASSERT */
KRITIC_TEST(assertions, assert_pass) {
    KRITIC_ASSERT(1);
}

KRITIC_TEST(assertions, assert_fail) {
    KRITIC_ASSERT(0);
}

KRITIC_TEST(assertions, assert_expr_pass) {
    int var = 2;
    KRITIC_ASSERT(var);
}

KRITIC_TEST(assertions, assert_expr_fail) {
    int var = 0;
    KRITIC_ASSERT(var);
}

/* KRITIC_ASSERT_NOT */
KRITIC_TEST(assertions, assert_not_pass) {
    KRITIC_ASSERT_NOT(0);
}

KRITIC_TEST(assertions, assert_not_fail) {
    KRITIC_ASSERT_NOT(1);
}

KRITIC_TEST(assertions, assert_not_expr_pass) {
    int var = 0;
    KRITIC_ASSERT_NOT(var);
}

KRITIC_TEST(assertions, assert_not_expr_fail) {
    int var = 2;
    KRITIC_ASSERT_NOT(var);
}

/* KRITIC_ASSERT_EQ */
KRITIC_TEST(assertions, assert_eq_pass) {
    KRITIC_ASSERT_EQ(42, 42);
}

KRITIC_TEST(assertions, assert_eq_fail) {
    KRITIC_ASSERT_EQ(1, 2);
}

KRITIC_TEST(assertions, assert_eq_expr_pass) {
    int a = 10, b = 10;
    KRITIC_ASSERT_EQ(a, b);
}

KRITIC_TEST(assertions, assert_eq_expr_fail) {
    int a = 5, b = 6;
    KRITIC_ASSERT_EQ(a, b);
}

/* KRITIC_ASSERT_NE */
KRITIC_TEST(assertions, assert_ne_pass) {
    KRITIC_ASSERT_NE(1, 2);
}

KRITIC_TEST(assertions, assert_ne_fail) {
    KRITIC_ASSERT_NE(5, 5);
}

KRITIC_TEST(assertions, assert_ne_expr_pass) {
    int a = 10, b = 20;
    KRITIC_ASSERT_NE(a, b);
}

KRITIC_TEST(assertions, assert_ne_expr_fail) {
    int a = 7, b = 7;
    KRITIC_ASSERT_NE(a, b);
}

/* KRITIC_FAIL */
KRITIC_TEST(assertions, assert_fail_fail) {
    KRITIC_FAIL();
}
