#include <stdio.h>

#include <kritic/kritic.h>

KRITIC_TEST(core, registration) {}

int main(void) {
    kritic_enable_ansi();
    kritic_set_default_printers();
    printf("Running from custom main()...\n");

    return kritic_run_all();
}
