#ifndef KRITIC_REDIRECT_H
#define KRITIC_REDIRECT_H

#define KRITIC_REDIRECT_TIMEOUT_MS 1000
#define KRITIC_REDIRECT_BUFFER_SIZE 4096

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

#define kritic_open_pipe(pipefd) _pipe(pipefd, KRITIC_REDIRECT_BUFFER_SIZE, O_BINARY)

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

#else // POSIX
#include <pthread.h>
#include <unistd.h>

#define kritic_open_pipe(pipefd) pipe(pipefd)

/* Aliases */
#define _close  close
#define _dup2   dup2
#define _dup    dup
#define _fileno fileno
#define _read   read

typedef struct {
    pthread_t thread;
    pthread_mutex_t lock;
    pthread_cond_t cond_start;
    pthread_cond_t cond_done;
    kritic_runtime_t* runtime;
    int read_fd;
    int pipe_write_end;
    int stdout_copy;
    bool running;
    bool shutting_down;
} kritic_redirect_t;

#endif // POSIX

void kritic_redirect_init(kritic_redirect_t* state, kritic_runtime_t* runtime);
void kritic_redirect_teardown(kritic_redirect_t* state);
void kritic_redirect_start(kritic_redirect_t* state);
void kritic_redirect_stop(kritic_redirect_t* state);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // KRITIC_REDIRECT_H
