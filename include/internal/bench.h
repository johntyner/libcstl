#ifndef INTERNAL_BENCH_H
#define INTERNAL_BENCH_H

struct bench_context;

unsigned long bench_context_count(const struct bench_context *);

void bench_start_timer(struct bench_context *);
void bench_stop_timer(struct bench_context *);

typedef void bench_runner_func_t(struct bench_context *);

#endif
