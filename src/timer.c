/* clock_gettime() and co. is not ISO C */
#define _POSIX_C_SOURCE 199309L

#include "../kritic.h"

#ifdef KRITIC_DISABLE_TIMER

void kritic_timer_start(kritic_timer_t* timer) {
    (void) timer;
}

uint64_t kritic_timer_elapsed(const kritic_timer_t* timer) {
    (void) timer;
    return UINT64_MAX;
}

#else // !KRITIC_DISABLE_TIMER

#ifdef _WIN32
#include <windows.h>

void kritic_timer_start(kritic_timer_t* timer) {
    LARGE_INTEGER freq, counter;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&counter);
    timer->frequency = (uint64_t) freq.QuadPart;
    timer->start = (uint64_t) counter.QuadPart;
}

uint64_t kritic_timer_elapsed(const kritic_timer_t* timer) {
    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    uint64_t elapsed_counts = (uint64_t) counter.QuadPart - timer->start;
    return (elapsed_counts * 1000000000ULL) / timer->frequency;
}

#else // POSIX
#include <time.h>

void kritic_timer_start(kritic_timer_t* timer) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    timer->start_sec = (uint64_t) ts.tv_sec;
    timer->start_nsec = (uint64_t) ts.tv_nsec;
}

uint64_t kritic_timer_elapsed(const kritic_timer_t* timer) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);

    int64_t sec_diff = (int64_t)ts.tv_sec - (int64_t) timer->start_sec;
    int64_t nsec_diff = (int64_t)ts.tv_nsec - (int64_t) timer->start_nsec;

    if (nsec_diff < 0) {
        sec_diff -= 1;
        nsec_diff += 1000000000L;
    }

    return (uint64_t) (sec_diff * 1000000000L + nsec_diff);
}

#endif // POSIX

#endif // !KRITIC_DISABLE_TIMER
