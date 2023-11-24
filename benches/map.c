#include "internal/bench.h"
#include "cstl/map.h"
#include <stdlib.h>

static int cmp_key(const void * const a, const void * const b, void * const p)
{
    (void)p;
    return (uintptr_t)a - (uintptr_t)b;
}

/*
 * at some point, I converted the map code to be able to use the
 * information it discovered during the search to prevent duplicate
 * keys to inform itself where the new key should be inserted. this
 * benchmark code didn't exist at the time so that I could compare
 * the before and after. this code isn't super useful now, on its
 * own, but there's also not much point in deleting it.
 *
 * fwiw, the change described above resulted in an approximately 30%
 * improvement in speed
 */
void bench_map_insert(struct bench_context * const ctx)
{
    const unsigned int n = 2000;
    unsigned int i;

    bench_stop_timer(ctx);

    for (i = 0; i < ctx->count; i++) {
	unsigned int j;
	cstl_map_t map;

	cstl_map_init(&map, cmp_key, NULL);

	bench_start_timer(ctx);
	for (j = 0; j < n; j++) {
	    cstl_map_insert(&map, (void *)(uintptr_t)rand(), NULL, NULL);
	}
	bench_stop_timer(ctx);

	cstl_map_clear(&map, NULL, NULL);
    }

    bench_start_timer(ctx);
}
