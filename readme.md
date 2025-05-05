# KritiC

[![Unittest](https://github.com/Wrench56/KritiC/actions/workflows/unittest.yml/badge.svg)](https://github.com/Wrench56/KritiC/actions/workflows/unittest.yml)
[![Tag Workflow Chain](https://github.com/Wrench56/KritiC/actions/workflows/tag-chain.yml/badge.svg)](https://github.com/Wrench56/KritiC/actions)
[![Version](https://img.shields.io/github/v/tag/Wrench56/KritiC?label=version)](https://github.com/Wrench56/KritiC/releases)
[![License](https://img.shields.io/github/license/Wrench56/KritiC)](https://github.com/Wrench56/KritiC/blob/main/LICENSE)
[![Lines of Code](https://tokei.rs/b1/github/Wrench56/KritiC)](https://github.com/Wrench56/KritiC)

A unit testing framework written in C.

KritiC is a lightweight, fast, and portable unit testing framework for C, C++, and Assembly projects. It aims to provide clean syntax and minimal setup.

## Quick Start

Write your tests:

```c
#include "kritic.h"

KRITIC_TEST(math, addition) {
    KRITIC_ASSERT_EQ(2 + 2, 4);
}

KRITIC_TEST(math, subtraction) {
    KRITIC_ASSERT_EQ(5 - 3, 2);
}
```

---

## Core Concepts

- Define tests with `KRITIC_TEST(suite, name)`
- Make assertions:
  - `KRITIC_ASSERT(expr)`: asserts that `expr` is true
  - `KRITIC_ASSERT_NOT(expr)`: asserts that `expr` is false
  - `KRITIC_ASSERT_EQ(x, y)`: asserts that `x` equals `y` (generic: supports `int`, `long long`, `float`, `double`, and `const char*`)
  - `KRITIC_ASSERT_NE(x, y)`: asserts that `x` does not equal `y` (generic: supports `int`, `long long`, `float`, `double`, and `const char*`)
  - `KRITIC_ASSERT_EQ_INT(x, y)`: asserts that `x` and `y` are equal using integer comparison  
  - `KRITIC_ASSERT_NE_INT(x, y)`: asserts that `x` and `y` are not equal using integer comparison  
  - `KRITIC_ASSERT_EQ_FLOAT(x, y)`: asserts that `x` and `y` are approximately equal using float/double comparison (with delta tolerance)  
  - `KRITIC_ASSERT_NE_FLOAT(x, y)`: asserts that `x` and `y` are not approximately equal using float/double comparison  
  - `KRITIC_ASSERT_EQ_STR(x, y)`: asserts that `x` and `y` are equal using string comparison  
  - `KRITIC_ASSERT_NE_STR(x, y)`: asserts that `x` and `y` are not equal using string comparison
  - `KRITIC_FAIL()`: forces a test failure
- Skip tests conditionally with `KRITIC_SKIP(reason)`
- Tests are discovered automatically without manual registration
- All printers (assertions, summaries, pre/post test messages) can be overridden
- Standard output can be automatically formatted/redirected during test execution
- Measure the precise runtime (nanosecond precision) of individual tests

## Compilation and Usage

Currently, KritiC is designed to be added to your project as a **Git submodule**.

### Add KritiC to your project

```sh
git submodule add https://github.com/Wrench56/KritiC.git extern/kritic
cd extern/kritic
make static
```

### Use KritiC from your project

```c
#include "kritic.h"

KRITIC_TEST(hello, world) {
    KRITIC_ASSERT(1);
}

```

### Compile your project with libkritic.a

```sh
clang -Iextern/kritic -o tests tests.c extern/kritic/build/libkritic.a
```
