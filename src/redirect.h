#ifndef __KRITIC_REDIRECT_HEADER_GUARD
#define __KRITIC_REDIRECT_HEADER_GUARD

#define _KRITIC_REDIRECT_TIMEOUT_MS 1000
#define _KRITIC_REDIRECT_BUFFER_SIZE 4096

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct kritic_runtime_t kritic_runtime_t;

typedef struct {
    int stdout_copy;
    char* string;
    int length;
    bool is_part_of_split;
} kritic_redirect_ctx_t;

#ifdef _WIN32
#include <io.h>
#include <windows.h>

typedef struct {
    HANDLE thread;
    HANDLE event_start;
    HANDLE event_done;
    kritic_runtime_t* runtime;
    int running;
    int read_fd;
    int stdout_copy;
    int pipe_write_end;
} kritic_redirect_t;

#endif // _WIN32

kritic_redirect_t kritic_redirect_init(kritic_runtime_t* runtime);
void kritic_redirect_teardown(kritic_redirect_t* state);
void kritic_redirect_start(kritic_redirect_t* state);
void kritic_redirect_stop(kritic_redirect_t* state);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // __KRITIC_REDIRECT_HEADER_GUARD
