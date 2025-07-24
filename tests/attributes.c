#include <stdio.h>
#include <string.h>

#include <kritic/kritic.h>

KRITIC_TEST(attributes, target_simple) {
    KRITIC_ASSERT(1);
}

KRITIC_TEST(attributes, depends_on_simple,
    KRITIC_DEPENDS_ON(attributes, target_simple)) {
    KRITIC_ASSERT(1);
}

/* Duplicate dependency (warning) */
KRITIC_TEST(attributes, depends_on_duplicate,
    KRITIC_DEPENDS_ON(attributes, target_simple),
    KRITIC_DEPENDS_ON(attributes, target_simple)) {
    KRITIC_ASSERT(1);
}

/* Cross-suite dependency */
KRITIC_TEST(dependency_suite, target_cross_suite) {
    KRITIC_ASSERT(1);
}

KRITIC_TEST(attributes, depends_on_cross_suite,
    KRITIC_DEPENDS_ON(dependency_suite, target_cross_suite)) {
    KRITIC_ASSERT(1);
}

/* Dependency chain */
KRITIC_TEST(attributes, dep_c) {
    KRITIC_ASSERT(1);
}

KRITIC_TEST(attributes, dep_b,
    KRITIC_DEPENDS_ON(attributes, dep_c)) {
    KRITIC_ASSERT(1);
}

KRITIC_TEST(attributes, dep_a,
    KRITIC_DEPENDS_ON(attributes, dep_b)) {
    KRITIC_ASSERT(1);
}

/* Diamond dependency */
KRITIC_TEST(attributes, diamond_a) {
    KRITIC_ASSERT(1);
}

KRITIC_TEST(attributes, diamond_b,
    KRITIC_DEPENDS_ON(attributes, diamond_a)) {
    KRITIC_ASSERT(1);
}

KRITIC_TEST(attributes, diamond_c,
    KRITIC_DEPENDS_ON(attributes, diamond_a)) {
    KRITIC_ASSERT(1);
}

KRITIC_TEST(attributes, diamond_d,
    KRITIC_DEPENDS_ON(attributes, diamond_b),
    KRITIC_DEPENDS_ON(attributes, diamond_c)) {
    KRITIC_ASSERT(1);
}

KRITIC_TEST(attributes, fail_leaf) {
    KRITIC_ASSERT(0);
}

KRITIC_TEST(attributes, fail_mid,
    KRITIC_DEPENDS_ON(attributes, fail_leaf)) {
    KRITIC_ASSERT(1);
}

KRITIC_TEST(attributes, fail_top,
    KRITIC_DEPENDS_ON(attributes, fail_mid)) {
    KRITIC_ASSERT(1);
}

KRITIC_TEST(attributes, diamond_fail_b,
    KRITIC_DEPENDS_ON(attributes, diamond_a)) {
    KRITIC_ASSERT(0);
}

KRITIC_TEST(attributes, diamond_fail_c,
    KRITIC_DEPENDS_ON(attributes, diamond_a)) {
    KRITIC_ASSERT(1);
}

KRITIC_TEST(attributes, diamond_fail_d,
    KRITIC_DEPENDS_ON(attributes, diamond_fail_b),
    KRITIC_DEPENDS_ON(attributes, diamond_fail_c)) {
    KRITIC_ASSERT(1);
}

typedef struct { size_t x, y, z; } Point;
typedef struct { int x, y; } Pair;
#define ARRAY_LEN(arr) (sizeof(arr) / sizeof((arr)[0]))

static int32_t nums_p[] = {1, 2, 3, 5, 8, 13, 21};
KRITIC_TEST(attributes, param_primitives_pass,
    KRITIC_PARAMETERIZED(n, int, nums_p, ARRAY_LEN(nums_p))) {
    int v = KRITIC_GET_PARAMETER(int, n);
    KRITIC_ASSERT(v >= 0);
}

static int32_t nums_f[] = {1, 2, 3, 5, 8, 13, 21, -1};
KRITIC_TEST(attributes, param_primitives_fail,
    KRITIC_PARAMETERIZED(n, int, nums_f, ARRAY_LEN(nums_f))) {
    int v = KRITIC_GET_PARAMETER(int, n);
    KRITIC_ASSERT(v >= 0);
}

static Point pts[] = {{1,2,3}, {4,5,6}, {7,8,9}};
KRITIC_TEST(attributes, param_struct_pass,
    KRITIC_PARAMETERIZED(p, Point, pts, ARRAY_LEN(pts))) {
    Point val = KRITIC_GET_PARAMETER(Point, p);
    KRITIC_ASSERT(val.x < 10);
}

static Point bad[] = {{1,2,3}, {100,100,100}};
KRITIC_TEST(attributes, param_struct_fail,
    KRITIC_PARAMETERIZED(p, Point, bad, ARRAY_LEN(bad))) {
    Point val = KRITIC_GET_PARAMETER(Point, p);
    KRITIC_ASSERT(val.x < 10);
}

KRITIC_TEST(attributes, param_null,
    KRITIC_PARAMETERIZED(p, Point, NULL, 0)) {
    printf("Should not run!");
    KRITIC_ASSERT(0);
}

static Pair pairs[] = {
    {1, 2}, {3, 4}, {-5, 5}
};
static int expected[] = {3, 7, 0};
KRITIC_TEST(attributes, param_double,
    KRITIC_PARAMETERIZED(p, Pair, pairs, ARRAY_LEN(pairs)),
    KRITIC_PARAMETERIZED(result, int, expected, ARRAY_LEN(expected))) {
    Pair a = KRITIC_GET_PARAMETER(Pair, p);
    int expected_sum = KRITIC_GET_PARAMETER(int, result);

    KRITIC_ASSERT_EQ_INT(a.x + a.y, expected_sum);
}

static const char *words[] = {"hello", "hi", "", "test"};
static int lengths[] = {5, 2, 0, 4};
KRITIC_TEST(attributes, param_diff,
    KRITIC_PARAMETERIZED(word, const char*, words, ARRAY_LEN(words)),
    KRITIC_PARAMETERIZED(len, int, lengths, ARRAY_LEN(lengths))) {
    const char* w = KRITIC_GET_PARAMETER(const char*, word);
    int l = KRITIC_GET_PARAMETER(int, len);

    KRITIC_ASSERT_EQ_INT((int)strlen(w), l);
}
