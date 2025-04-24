#include <stdio.h>

#include "../kritic.h"

KRITIC_TEST(core, registration) {}

int main(void) {
    kritic_enable_ansi();
    printf("Running from custom main()...\n");

    return kritic_run_all();
}
