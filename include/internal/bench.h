#ifndef INTERNAL_BENCH_H
#define INTERNAL_BENCH_H

#include <time.h>

#define CLOCK_BENCH       CLOCK_PROCESS_CPUTIME_ID

#define BENCH_CONTEXT_INITIALIZER               \
    {                                           \
        .count = 0,                             \
        .start = {                              \
            .tv_sec = -1,                       \
            .tv_nsec = 0,                       \
        },                                      \
        .accum = {                              \
            .tv_sec = 0,                        \
            .tv_nsec = 0,                       \
        },                                      \
    }
#define DECLARE_BENCH_CONTEXT(CTX)                              \
    struct bench_context CTX = BENCH_CONTEXT_INITIALIZER

struct bench_context
{
    unsigned long count;

    struct timespec start;
    struct timespec accum;
};

void bench_start_timer(struct bench_context *);
void bench_stop_timer(struct bench_context *);

typedef void bench_runner_func_t(struct bench_context *);

#endif
