#include "../kritic.h"

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
