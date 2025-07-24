#include <limits.h>

#include <kritic/kritic.h>

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

/* KRITIC_ASSERT_EQ_INT */
KRITIC_TEST(assertions, assert_eq_int_zero_pass) {
    KRITIC_ASSERT_EQ_INT(0, 0);
}

KRITIC_TEST(assertions, assert_eq_int_pass) {
    KRITIC_ASSERT_EQ_INT(42, 42);
}

KRITIC_TEST(assertions, assert_eq_int_fail) {
    KRITIC_ASSERT_EQ_INT(1, 2);
}

KRITIC_TEST(assertions, assert_eq_int_negative_pass) {
    KRITIC_ASSERT_EQ_INT(-42, -42);
}

KRITIC_TEST(assertions, assert_eq_int_min_max_fail) {
    KRITIC_ASSERT_EQ_INT(INT_MIN, INT_MAX);
}

KRITIC_TEST(assertions, assert_eq_int_expr_pass) {
    int a = 10, b = 10;
    KRITIC_ASSERT_EQ_INT(a, b);
}

KRITIC_TEST(assertions, assert_eq_int_expr_fail) {
    int a = 5, b = 6;
    KRITIC_ASSERT_EQ_INT(a, b);
}

/* KRITIC_ASSERT_EQ_FLOAT */
KRITIC_TEST(assertions, assert_eq_float_zero_pass) {
    KRITIC_ASSERT_EQ_FLOAT(0.0, 0.0);
}

KRITIC_TEST(assertions, assert_eq_float_exact_pass) {
    double a = 3.1415926535;
    double b = 3.1415926535;
    KRITIC_ASSERT_EQ_FLOAT(a, b);
}

KRITIC_TEST(assertions, assert_eq_float_exact_fail) {
    double a = 2.7182818284;
    double b = 3.1415926535;
    KRITIC_ASSERT_EQ_FLOAT(a, b);
}

KRITIC_TEST(assertions, assert_eq_float_pass) {
    double a = 1.0;
    double b = 1.0 + (KRITIC_FLOAT_DELTA_VALUE / 2.0);
    KRITIC_ASSERT_EQ_FLOAT(a, b);
}

KRITIC_TEST(assertions, assert_eq_float_fail) {
    double a = 1.0;
    double b = 1.0 + (KRITIC_FLOAT_DELTA_VALUE * 2.0);
    KRITIC_ASSERT_EQ_FLOAT(a, b);
}

KRITIC_TEST(assertions, assert_eq_float_exact_expr_pass) {
    double pi = 3.1415926535;
    double pi_copy = 3.1415926535;
    KRITIC_ASSERT_EQ_FLOAT(pi, pi_copy);
}

KRITIC_TEST(assertions, assert_eq_float_exact_expr_fail) {
    double e = 2.7182818284;
    double pi = 3.1415926535;
    KRITIC_ASSERT_EQ_FLOAT(e, pi);
}

KRITIC_TEST(assertions, assert_eq_float_expr_pass) {
    double a = 100.0;
    double b = 100.0 + (KRITIC_FLOAT_DELTA_VALUE / 2.0);
    KRITIC_ASSERT_EQ_FLOAT(a, b);
}

KRITIC_TEST(assertions, assert_eq_float_expr_fail) {
    double a = 100.0;
    double b = 100.0 + (KRITIC_FLOAT_DELTA_VALUE * 2.0);
    KRITIC_ASSERT_EQ_FLOAT(a, b);
}

KRITIC_TEST(assertions, assert_eq_float_negative_pass) {
    KRITIC_ASSERT_EQ_FLOAT(-1.5, -1.5);
}

KRITIC_TEST(assertions, assert_eq_float_negative_zero_pass) {
    KRITIC_ASSERT_EQ_FLOAT(-0.0, 0.0);
}

/* KRITIC_ASSERT_EQ_STR */
KRITIC_TEST(assertions, assert_eq_str_pass) {
    KRITIC_ASSERT_EQ_STR("Hello", "Hello");
}

KRITIC_TEST(assertions, assert_eq_str_fail) {
    KRITIC_ASSERT_EQ_STR("Hello", "World");
}

KRITIC_TEST(assertions, assert_eq_str_expr_pass) {
    const char* hello1 = "Hello";
    const char* hello2 = "Hello";
    KRITIC_ASSERT_EQ_STR(hello1, hello2);
}

KRITIC_TEST(assertions, assert_eq_str_expr_fail) {
    const char* hello = "Hello";
    const char* world = "World";
    KRITIC_ASSERT_EQ_STR(hello, world);
}

KRITIC_TEST(assertions, assert_eq_str_empty_pass) {
    KRITIC_ASSERT_EQ_STR("", "");
}

KRITIC_TEST(assertions, assert_eq_str_null_literal_fail) {
    const char *a = "";
    const char *b = NULL;
    KRITIC_ASSERT_EQ_STR(a, b);
}

KRITIC_TEST(assertions, assert_eq_str_nulls_pass) {
    const char *a = NULL, *b = NULL;
    KRITIC_ASSERT_EQ_STR(a, b);
}

/* KRITIC_ASSERT_EQ (Generic) */
KRITIC_TEST(assertions, assert_eq_generic_int_pass) {
    KRITIC_ASSERT_EQ(123, 123);
}

KRITIC_TEST(assertions, assert_eq_generic_int_fail) {
    KRITIC_ASSERT_EQ(1, 0);
}

KRITIC_TEST(assertions, assert_eq_generic_int_expr_pass) {
    int a = 5, b = 2 + 3;
    KRITIC_ASSERT_EQ(a, b);
}

KRITIC_TEST(assertions, assert_eq_generic_int_expr_fail) {
    int a = 7, b = 8;
    KRITIC_ASSERT_EQ(a, b);
}

KRITIC_TEST(assertions, assert_eq_generic_float_pass) {
    double x = 1.0;
    double y = 1.0 + (KRITIC_FLOAT_DELTA_VALUE / 2.0);
    KRITIC_ASSERT_EQ(x, y);
}

KRITIC_TEST(assertions, assert_eq_generic_float_fail) {
    KRITIC_ASSERT_EQ(3.0, 3.1);
}

KRITIC_TEST(assertions, assert_eq_generic_str_pass) {
    KRITIC_ASSERT_EQ("test", "test");
}

KRITIC_TEST(assertions, assert_eq_generic_str_fail) {
    KRITIC_ASSERT_EQ("foo", "bar");
}

KRITIC_TEST(assertions, assert_eq_generic_str_expr_pass) {
    const char* a = "abc";
    const char* b = "abc";
    KRITIC_ASSERT_EQ(a, b);
}

KRITIC_TEST(assertions, assert_eq_generic_str_expr_fail) {
    const char* a = "abc";
    const char* b = "def";
    KRITIC_ASSERT_EQ(a, b);
}

/* KRITIC_ASSERT_NE_INT */
KRITIC_TEST(assertions, assert_ne_int_zero_fail) {
    KRITIC_ASSERT_NE_INT(0, 0);
}

KRITIC_TEST(assertions, assert_ne_int_pass) {
    KRITIC_ASSERT_NE_INT(1, 2);
}

KRITIC_TEST(assertions, assert_ne_int_fail) {
    KRITIC_ASSERT_NE_INT(5, 5);
}

KRITIC_TEST(assertions, assert_ne_int_negative_fail) {
    KRITIC_ASSERT_NE_INT(-42, -42);
}

KRITIC_TEST(assertions, assert_ne_int_min_max_pass) {
    KRITIC_ASSERT_NE_INT(INT_MIN, INT_MAX);
}

KRITIC_TEST(assertions, assert_ne_int_expr_pass) {
    int a = 10, b = 20;
    KRITIC_ASSERT_NE_INT(a, b);
}

KRITIC_TEST(assertions, assert_ne_int_expr_fail) {
    int a = 7, b = 7;
    KRITIC_ASSERT_NE_INT(a, b);
}

/* KRITIC_ASSERT_NE_FLOAT */
KRITIC_TEST(assertions, assert_ne_float_zero_fail) {
    KRITIC_ASSERT_NE_FLOAT(0.0, 0.0);
}

KRITIC_TEST(assertions, assert_ne_float_exact_fail) {
    double a = 3.1415926535;
    double b = 3.1415926535;
    KRITIC_ASSERT_NE_FLOAT(a, b);
}

KRITIC_TEST(assertions, assert_ne_float_exact_pass) {
    double a = 2.7182818284;
    double b = 3.1415926535;
    KRITIC_ASSERT_NE_FLOAT(a, b);
}

KRITIC_TEST(assertions, assert_ne_float_pass) {
    double a = 1.0;
    double b = 1.0 + (KRITIC_FLOAT_DELTA_VALUE * 2.0);
    KRITIC_ASSERT_NE_FLOAT(a, b);
}

KRITIC_TEST(assertions, assert_ne_float_fail) {
    double a = 1.0;
    double b = 1.0 + (KRITIC_FLOAT_DELTA_VALUE / 2.0);
    KRITIC_ASSERT_NE_FLOAT(a, b);
}

KRITIC_TEST(assertions, assert_ne_float_exact_expr_fail) {
    double pi = 3.1415926535;
    double pi_copy = 3.1415926535;
    KRITIC_ASSERT_NE_FLOAT(pi, pi_copy);
}

KRITIC_TEST(assertions, assert_ne_float_exact_expr_pass) {
    double e = 2.7182818284;
    double pi = 3.1415926535;
    KRITIC_ASSERT_NE_FLOAT(e, pi);
}

KRITIC_TEST(assertions, assert_ne_float_expr_pass) {
    double a = 100.0;
    double b = 100.0 + (KRITIC_FLOAT_DELTA_VALUE * 2.0);
    KRITIC_ASSERT_NE_FLOAT(a, b);
}

KRITIC_TEST(assertions, assert_ne_float_expr_fail) {
    double a = 100.0;
    double b = 100.0 + (KRITIC_FLOAT_DELTA_VALUE / 2.0);
    KRITIC_ASSERT_NE_FLOAT(a, b);
}

KRITIC_TEST(assertions, assert_ne_float_negative_fail) {
    KRITIC_ASSERT_NE_FLOAT(-1.5, -1.5);
}

KRITIC_TEST(assertions, assert_ne_float_negative_zero_fail) {
    KRITIC_ASSERT_NE_FLOAT(-0.0, 0.0);
}

/* KRITIC_ASSERT_NE_STR */
KRITIC_TEST(assertions, assert_ne_str_pass) {
    KRITIC_ASSERT_NE_STR("Hello", "World");
}

KRITIC_TEST(assertions, assert_ne_str_fail) {
    KRITIC_ASSERT_NE_STR("Hello", "Hello");
}

KRITIC_TEST(assertions, assert_ne_str_expr_pass) {
    const char* hello = "Hello";
    const char* world = "World";
    KRITIC_ASSERT_NE_STR(hello, world);
}

KRITIC_TEST(assertions, assert_ne_str_expr_fail) {
    const char* hello1 = "Hello";
    const char* hello2 = "Hello";
    KRITIC_ASSERT_NE_STR(hello1, hello2);
}

KRITIC_TEST(assertions, assert_ne_str_empty_fail) {
    KRITIC_ASSERT_NE_STR("", "");
}

KRITIC_TEST(assertions, assert_ne_str_null_literal_pass) {
    const char *a = "";
    const char *b = NULL;
    KRITIC_ASSERT_NE_STR(a, b);
}

KRITIC_TEST(assertions, assert_ne_str_nulls_fail) {
    const char *a = NULL, *b = NULL;
    KRITIC_ASSERT_NE_STR(a, b);
}

/* KRITIC_ASSERT_NE (Generic) */
KRITIC_TEST(assertions, assert_ne_generic_int_pass) {
    KRITIC_ASSERT_NE(123, 456);
}

KRITIC_TEST(assertions, assert_ne_generic_int_fail) {
    KRITIC_ASSERT_NE(1, 1);
}

KRITIC_TEST(assertions, assert_ne_generic_int_expr_pass) {
    int a = 5, b = 6;
    KRITIC_ASSERT_NE(a, b);
}

KRITIC_TEST(assertions, assert_ne_generic_int_expr_fail) {
    int a = 7, b = 7;
    KRITIC_ASSERT_NE(a, b);
}

KRITIC_TEST(assertions, assert_ne_generic_float_pass) {
    double a = 1.0, b = 1.0 + KRITIC_FLOAT_DELTA_VALUE * 2.0;
    KRITIC_ASSERT_NE(a, b);
}

KRITIC_TEST(assertions, assert_ne_generic_float_fail) {
    double a = 3.14159, b = 3.14159;
    KRITIC_ASSERT_NE(a, b);
}

KRITIC_TEST(assertions, assert_ne_generic_str_pass) {
    KRITIC_ASSERT_NE("foo", "bar");
}

KRITIC_TEST(assertions, assert_ne_generic_str_fail) {
    KRITIC_ASSERT_NE("same", "same");
}

KRITIC_TEST(assertions, assert_ne_generic_str_expr_pass) {
    const char *a = "abc", *b = "def";
    KRITIC_ASSERT_NE(a, b);
}

KRITIC_TEST(assertions, assert_ne_generic_str_expr_fail) {
    const char *a = "match", *b = "match";
    KRITIC_ASSERT_NE(a, b);
}

/* KRITIC_FAIL */
KRITIC_TEST(assertions, assert_fail_fail) {
    KRITIC_FAIL();
}

/* KRITIC_SKIP */
KRITIC_TEST(assertions, simple_skip) {
    KRITIC_SKIP("Skip from assertions.skip test");
}

KRITIC_TEST(assertions, dual_skip) {
    KRITIC_SKIP("This should be shown on stdout");
    KRITIC_SKIP("This should NOT be shown on stdout");
}

KRITIC_TEST(assertions, pass_then_skip) {
    KRITIC_ASSERT(1);
    KRITIC_SKIP("Skipping after pass...");
}

KRITIC_TEST(assertions, fail_then_skip) {
    KRITIC_ASSERT(0);
    KRITIC_SKIP("Skipping after fail...");
}

KRITIC_TEST(assertions, pass_and_skip_fail) {
    KRITIC_ASSERT(1);
    KRITIC_SKIP("This test should never show fail");
    KRITIC_ASSERT(0);
}
