#include <stdio.h>

#include "../kritic.h"

KRITIC_TEST(io, stdout_redirect) {
    printf("Redirection of stdout works!\n");
}

KRITIC_TEST(io, stdout_newline) {
    printf("This line should end with a newline");
}

KRITIC_TEST(io, stdout_multiline) {
    printf("Hello\n");
    printf("World");
}


KRITIC_TEST(io, stdout_monoline) {
    printf("Hello ");
    printf("World");
}

/* Edge Case: Whitespace only */
KRITIC_TEST(io, stdout_space_only) {
    printf(" \t \n");
}

/* Edge Case: Flush behavior */
KRITIC_TEST(io, stdout_flush) {
    printf("This will be flushed... ");
    fflush(stdout);
    printf("and this follows.");
}

/* Buffer boundary: exactly buffer size */
KRITIC_TEST(io, stdout_exact_buffer) {
    char buf[KRITIC_REDIRECT_BUFFER_SIZE];
    for (int i = 0; i < KRITIC_REDIRECT_BUFFER_SIZE - 1; ++i) {
        buf[i] = 'A';
    }
    buf[KRITIC_REDIRECT_BUFFER_SIZE - 1] = '\0';
    printf("%s", buf);
}

/* Buffer overflow: 2x buffer size */
KRITIC_TEST(io, stdout_long_line) {
    static char bigbuf[2 * KRITIC_REDIRECT_BUFFER_SIZE];
    for (int i = 0; i < 2 * KRITIC_REDIRECT_BUFFER_SIZE - 1; ++i) {
        bigbuf[i] = 'A' + (i % 26);
    }
    bigbuf[2 * KRITIC_REDIRECT_BUFFER_SIZE - 1] = '\0';
    printf("%s", bigbuf);
}

/* Mixed content */
KRITIC_TEST(io, stdout_mixed_lines) {
    printf("Line 1\n");
    printf("Line 2 without newline");
    printf("\nLine 3\twith tab and \033[36mcyan ANSI escape\033[0m\n");
}

/* Print a single char at a time */
KRITIC_TEST(io, stdout_char_by_char) {
    const char* msg = "Char-by-char\n";
    for (const char* p = msg; *p; ++p) {
        putchar(*p);
    }
}
