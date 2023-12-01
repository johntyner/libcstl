#include "internal/bench.h"
#include "cstl/crc.h"
#include <stddef.h>

#define BUF_LEN         3072
#define CRC32_POLY      0x04c11db7

void bench_crc32be_table(struct bench_context * const ctx,
                         const unsigned long count)
{
    uint32_t tab[256];
    unsigned int i;

    (void)ctx;

    for (i = 0; i < count; i++) {
        cstl_crc32be_table(tab, CRC32_POLY);
    }
}

void bench_crc32le_table(struct bench_context * const ctx,
                         const unsigned long count)
{
    uint32_t tab[256];
    unsigned int i;

    (void)ctx;

    for (i = 0; i < count; i++) {
        cstl_crc32le_table(tab, CRC32_POLY);
    }
}

void bench_crc32be_wtable(struct bench_context * const ctx,
                          const unsigned long count)
{
    uint32_t tab[256];
    uint8_t buf[BUF_LEN];
    unsigned int i;

    bench_stop_timer(ctx);
    cstl_crc32be_table(tab, CRC32_POLY);
    bench_start_timer(ctx);

    for (i = 0; i < count; i++) {
        cstl_crc32be(tab, CRC32_POLY, ~0, buf, BUF_LEN);
    }
}

void bench_crc32le_wtable(struct bench_context * const ctx,
                          const unsigned long count)
{
    uint32_t tab[256];
    uint8_t buf[BUF_LEN];
    unsigned int i;

    bench_stop_timer(ctx);
    cstl_crc32le_table(tab, CRC32_POLY);
    bench_start_timer(ctx);

    for (i = 0; i < count; i++) {
        cstl_crc32le(tab, CRC32_POLY, ~0, buf, BUF_LEN);
    }
}

void bench_crc32be_notable(struct bench_context * const ctx,
                           const unsigned long count)
{
    uint8_t buf[BUF_LEN];
    unsigned int i;

    (void)ctx;

    for (i = 0; i < count; i++) {
        cstl_crc32be(NULL, CRC32_POLY, ~0, buf, BUF_LEN);
    }
}

void bench_crc32le_notable(struct bench_context * const ctx,
                           const unsigned long count)
{
    uint8_t buf[BUF_LEN];
    unsigned int i;

    (void)ctx;

    for (i = 0; i < count; i++) {
        cstl_crc32le(NULL, CRC32_POLY, ~0, buf, BUF_LEN);
    }
}
