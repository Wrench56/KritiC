#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../kritic.h"

static void _kritic_read_pipe_lines(kritic_redirect_t* state, char* buffer, char* line_buffer) {
    size_t line_len = 0;
    size_t bytes_read;
    bool is_part_of_split = false;

    while ((bytes_read = _read(state->read_fd, buffer, _KRITIC_REDIRECT_BUFFER_SIZE)) > 0) {
        for (size_t i = 0; i < bytes_read;) {
            char* nl = memchr(buffer + i, '\n', bytes_read - i);
            size_t chunk_len = nl ? (nl - (buffer + i) + 1) : (bytes_read - i);

            if (line_len + chunk_len > _KRITIC_REDIRECT_BUFFER_SIZE) {
                line_buffer[line_len] = '\0';
                state->runtime->printers->stdout_printer(state->runtime, &(kritic_redirect_ctx_t) {
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
                state->runtime->printers->stdout_printer(state->runtime, &(kritic_redirect_ctx_t) {
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

    if (line_len > 0) {
        if (line_len >= (_KRITIC_REDIRECT_BUFFER_SIZE - 2)) {
            /* Override last two characters */
            line_len = _KRITIC_REDIRECT_BUFFER_SIZE - 2;
        }

        line_buffer[line_len++] = '\n';
        line_buffer[line_len] = '\0';

        state->runtime->printers->stdout_printer(state->runtime, &(kritic_redirect_ctx_t) {
            .stdout_copy      = state->stdout_copy,
            .string           = line_buffer,
            .length           = line_len,
            .is_part_of_split = is_part_of_split
        });
    }
}

#ifdef _WIN32

static DWORD WINAPI _kritic_pipe_reader_thread(LPVOID arg) {
    kritic_redirect_t* state = (kritic_redirect_t*)arg;
    char buffer[_KRITIC_REDIRECT_BUFFER_SIZE];
    char line_buffer[_KRITIC_REDIRECT_BUFFER_SIZE];

    for (;;) {
        WaitForSingleObject(state->event_start, INFINITE);

        if (!state->running) break;
        _kritic_read_pipe_lines(state, buffer, line_buffer);

        SetEvent(state->event_done);
    }

    return 0;
}


kritic_redirect_t kritic_redirect_init(kritic_runtime_t* runtime) {
    kritic_redirect_t state = { 0 };
    state.runtime = runtime;

    state.event_start = CreateEvent(NULL, FALSE, FALSE, NULL);
    state.event_done = CreateEvent(NULL, FALSE, FALSE, NULL);
    state.thread = CreateThread(NULL, 0, _kritic_pipe_reader_thread, &state, 0, NULL);
    if (!state.thread) {
        perror("CreateThread() failed");
        exit(1);
    }

    return state;
}

void kritic_redirect_teardown(kritic_redirect_t* state) {
    if (!state || !state->thread) return;
    state->running = 0;

    SetEvent(state->event_start);
    WaitForSingleObject(state->thread, _KRITIC_REDIRECT_TIMEOUT_MS);
    CloseHandle(state->thread);
    CloseHandle(state->event_start);
    CloseHandle(state->event_done);
}

static void _kritic_redirect_start_reader_thread(kritic_redirect_t* state) {
    ResetEvent(state->event_done);
    SetEvent(state->event_start);
}

static void _kritic_redirect_wait_reader_thread(kritic_redirect_t* state) {
    WaitForSingleObject(state->event_done, _KRITIC_REDIRECT_TIMEOUT_MS);
}

#endif // _WIN32

void kritic_redirect_start(kritic_redirect_t* state) {
    fflush(stdout);
    int pipefd[2];
    if (_pipe(pipefd, _KRITIC_REDIRECT_BUFFER_SIZE, O_BINARY) == -1) {
        perror("pipe failed");
        exit(1);
    }

    int stdout_copy = _dup(_fileno(stdout));
    if (stdout_copy == -1) {
        perror("dup failed");
        exit(1);
    }

    state->read_fd = pipefd[0];
    state->pipe_write_end = pipefd[1];
    state->stdout_copy = stdout_copy;
    state->running = 1;

    _dup2(pipefd[1], _fileno(stdout));
    setvbuf(stdout, NULL, _IONBF, 0);

    _kritic_redirect_start_reader_thread(state);    
}

void kritic_redirect_stop(kritic_redirect_t* state) {
    fflush(stdout);
    _close(state->pipe_write_end);
    _close(_fileno(stdout));

    _kritic_redirect_wait_reader_thread(state);

    _dup2(state->stdout_copy, _fileno(stdout));
    setvbuf(stdout, NULL, _IOLBF, 1024);
    fflush(stdout);

    _close(state->stdout_copy);
    _close(state->read_fd);
}
