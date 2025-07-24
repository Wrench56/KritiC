#ifndef KRITIC_TIMER_H
#define KRITIC_TIMER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
typedef struct {
    uint64_t start;
    uint64_t frequency;
} kritic_timer_t;

#else // POSIX
typedef struct {
    uint64_t start_sec;
    uint64_t start_nsec;
} kritic_timer_t;

#endif // POSIX

void kritic_timer_start(kritic_timer_t* timer);
uint64_t kritic_timer_elapsed(const kritic_timer_t* timer);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // KRITIC_TIMER_H
