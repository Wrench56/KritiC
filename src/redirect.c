/* fileno() is not ISO C */
#define _POSIX_C_SOURCE 200112L

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
    #define READ_RET_T int
#else
    #include <unistd.h>
    #define READ_RET_T ssize_t
#endif

#include "../kritic.h"

static void kritic_read_pipe_lines(kritic_runtime_t* runtime, char* buffer, char* line_buffer) {
    kritic_redirect_t* state = runtime->redirect;
    uint32_t line_len = 0;
    READ_RET_T read_return;

    uint32_t bytes_read;
    bool is_part_of_split = false;

    while ((read_return = _read(state->read_fd, buffer, KRITIC_REDIRECT_BUFFER_SIZE - 1)) > 0) {
        bytes_read = (uint32_t) read_return;
        char* nl;
        for (uint32_t i = 0; i < bytes_read;) {
            nl = memchr(buffer + i, '\n', bytes_read - i);
            uint32_t chunk_len = nl ? (uint32_t) (nl - (buffer + i) + 1) : (bytes_read - i);

            if (line_len + chunk_len > KRITIC_REDIRECT_BUFFER_SIZE - 1) {
                line_buffer[line_len] = '\0';
                runtime->printers.stdout_printer(runtime, &(kritic_redirect_ctx_t) {
                    .stdout_copy      = state->stdout_copy,
                    .string           = line_buffer,
                    .length           = line_len,
                    .is_part_of_split = is_part_of_split
                });
                is_part_of_split = true;
                line_len = 0;
            }

            memcpy(line_buffer + line_len, buffer + i, chunk_len);
            line_len += chunk_len;
            i += chunk_len;

            if (nl) {
                line_buffer[line_len] = '\0';
                runtime->printers.stdout_printer(runtime, &(kritic_redirect_ctx_t) {
                    .stdout_copy      = state->stdout_copy,
                    .string           = line_buffer,
                    .length           = line_len,
                    .is_part_of_split = is_part_of_split
                });
                is_part_of_split = false;
                line_len = 0;
            }
        }
    }

    if (line_len == 0) return;
    if (line_len >= (KRITIC_REDIRECT_BUFFER_SIZE - 1)) {
        /* Send end fragment separately */
        line_buffer[line_len] = '\0';
        runtime->printers.stdout_printer(runtime, &(kritic_redirect_ctx_t) {
            .stdout_copy      = state->stdout_copy,
            .string           = line_buffer,
            .length           = line_len,
            .is_part_of_split = is_part_of_split
        });

        char end[] = {'\n', '\0'};
        runtime->printers.stdout_printer(runtime, &(kritic_redirect_ctx_t) {
            .stdout_copy      = state->stdout_copy,
            .string           = end,
            .length           = 1,
            .is_part_of_split = true
        });
    } else {
        line_buffer[line_len++] = '\n';
        line_buffer[line_len] = '\0';

        runtime->printers.stdout_printer(runtime, &(kritic_redirect_ctx_t) {
            .stdout_copy      = state->stdout_copy,
            .string           = line_buffer,
            .length           = line_len,
            .is_part_of_split = is_part_of_split
        });
    }
}

#ifdef _WIN32

static DWORD WINAPI kritic_pipe_reader_thread(LPVOID arg) {
    kritic_runtime_t* runtime = (kritic_runtime_t*)arg;
    kritic_redirect_t* state = runtime->redirect;

    char buffer[KRITIC_REDIRECT_BUFFER_SIZE];
    char line_buffer[KRITIC_REDIRECT_BUFFER_SIZE];

    for (;;) {
        WaitForSingleObject(state->event_start, INFINITE);

        if (!state->running) break;
        kritic_read_pipe_lines(runtime, buffer, line_buffer);

        SetEvent(state->event_done);
    }

    _close(state->read_fd);
    state->read_fd = -1;

    return 0;
}

void kritic_redirect_init(kritic_runtime_t* runtime) {
    kritic_redirect_t* state = runtime->redirect;

    state->event_start = CreateEvent(NULL, FALSE, FALSE, NULL);
    state->event_done = CreateEvent(NULL, FALSE, FALSE, NULL);
    state->thread = CreateThread(NULL, 0, kritic_pipe_reader_thread, runtime, 0, NULL);
    if (!state->thread) {
        perror("CreateThread() failed");
        exit(1);
    }
}

void kritic_redirect_teardown(kritic_runtime_t* runtime) {
    kritic_redirect_t* state = runtime->redirect;

    if (!state || !state->thread) return;
    state->running = 0;

    SetEvent(state->event_start);
    WaitForSingleObject(state->thread, KRITIC_REDIRECT_TIMEOUT_MS);
    CloseHandle(state->thread);
    CloseHandle(state->event_start);
    CloseHandle(state->event_done);
}

static void kritic_redirect_start_reader_thread(kritic_redirect_t* state) {
    ResetEvent(state->event_done);
    SetEvent(state->event_start);
}

static void kritic_redirect_wait_reader_thread(kritic_redirect_t* state) {
    WaitForSingleObject(state->event_done, KRITIC_REDIRECT_TIMEOUT_MS);
}

#else // POSIX

static void* kritic_pipe_reader_thread(void* arg) {
    kritic_runtime_t* runtime = (kritic_runtime_t*)arg;
    kritic_redirect_t* state = runtime->redirect;

    char buffer[KRITIC_REDIRECT_BUFFER_SIZE];
    char line_buffer[KRITIC_REDIRECT_BUFFER_SIZE];

    for (;;) {
        pthread_mutex_lock(&state->lock);
        while (!state->running && !state->shutting_down)
            pthread_cond_wait(&state->cond_start, &state->lock);

        if (state->shutting_down) {
            pthread_mutex_unlock(&state->lock);
            break;
        }

        pthread_mutex_unlock(&state->lock);

        kritic_read_pipe_lines(runtime, buffer, line_buffer);

        pthread_mutex_lock(&state->lock);
        state->running = false;
        pthread_cond_signal(&state->cond_done);
        pthread_mutex_unlock(&state->lock);
    }

    close(state->read_fd);
    state->read_fd = -1;

    return NULL;
}

void kritic_redirect_init(kritic_runtime_t* runtime) {
    kritic_redirect_t* state = runtime->redirect;

    pthread_mutex_init(&state->lock, NULL);
    pthread_cond_init(&state->cond_start, NULL);
    pthread_cond_init(&state->cond_done, NULL);

    pthread_create(&state->thread, NULL, kritic_pipe_reader_thread, runtime);
}

void kritic_redirect_teardown(kritic_runtime_t* runtime) {
    kritic_redirect_t* state = runtime->redirect;

    if (!state) return;
    pthread_mutex_lock(&state->lock);
    state->shutting_down = true;
    pthread_cond_signal(&state->cond_start);
    pthread_mutex_unlock(&state->lock);

    pthread_join(state->thread, NULL);

    pthread_mutex_destroy(&state->lock);
    pthread_cond_destroy(&state->cond_start);
    pthread_cond_destroy(&state->cond_done);
}

static void kritic_redirect_start_reader_thread(kritic_redirect_t* state) {
    pthread_mutex_lock(&state->lock);
    state->running = true;
    pthread_cond_signal(&state->cond_start);
    pthread_mutex_unlock(&state->lock);
}

static void kritic_redirect_wait_reader_thread(kritic_redirect_t* state) {
    pthread_mutex_lock(&state->lock);
    while (state->running)
        pthread_cond_wait(&state->cond_done, &state->lock);
    pthread_mutex_unlock(&state->lock);
}

#endif // POSIX

void kritic_redirect_start(kritic_runtime_t* runtime) {
    kritic_redirect_t* state = runtime->redirect;
    int pipefd[2];

    fflush(stdout);
    if (kritic_open_pipe(pipefd) == -1) {
        perror("pipe failed");
        exit(1);
    }

    int stdout_copy = _dup(_fileno(stdout));
    if (stdout_copy == -1) {
        perror("dup failed");
        exit(1);
    }

    state->read_fd        = pipefd[0];
    state->pipe_write_end = pipefd[1];
    state->stdout_copy    = stdout_copy;
    state->running        = 1;

    _dup2(pipefd[1], _fileno(stdout));
    setvbuf(stdout, NULL, _IONBF, 0);

    kritic_redirect_start_reader_thread(state);
}

void kritic_redirect_stop(kritic_runtime_t* runtime) {
    kritic_redirect_t* state = runtime->redirect;

    fflush(stdout);
    _close(state->pipe_write_end);
    _close(_fileno(stdout));

    kritic_redirect_wait_reader_thread(state);

    _dup2(state->stdout_copy, _fileno(stdout));
    setvbuf(stdout, NULL, _IOLBF, 1024);
    fflush(stdout);

    _close(state->stdout_copy);
}
