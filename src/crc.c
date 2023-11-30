#include <stdint.h>
#include <stddef.h>

static uint32_t crc32be_mask(const uint32_t poly, const uint8_t v)
{
    unsigned int i;
    uint32_t m;

    m = v << 24;
    for (i = 0; i < 8; i++) {
        const uint32_t p[] = { 0, poly };
        m = (m << 1) ^ p[(m >> 31) & 1];
    }

    return m;
}

static uint32_t crc32le_mask(const uint32_t poly, const uint8_t v)
{
    unsigned int i;
    uint32_t m;

    m = v;
    for (i = 0; i < 8; i++) {
        const uint32_t p[] = { 0, poly };
        m = (m >> 1) ^ p[m & 1];
    }

    return m;
}

static void crc32_table(uint32_t tab[256], const uint32_t poly,
                        uint32_t (* const mask)(uint32_t, uint8_t))
{
    unsigned int i;

    for (i = 0; i < 256; i++) {
        tab[i] = mask(poly, i);
    }
}

void crc32be_table(uint32_t tab[256], const uint32_t poly)
{
    crc32_table(tab, poly, crc32be_mask);
}

void crc32le_table(uint32_t tab[256], const uint32_t poly)
{
    crc32_table(tab, poly, crc32le_mask);
}

static uint32_t crc32(const uint32_t tab[256], const uint32_t poly,
                      uint32_t crc,
                      const void * buf, size_t len,
                      uint8_t (* const upd)(uint32_t *, uint8_t),
                      uint32_t (* const mask)(uint32_t, uint8_t))
{
    while (len > 0) {
        const uint8_t v = upd(&crc, *(uint8_t *)buf);
        uint32_t m;

        if (tab != NULL) {
            m = tab[v];
        } else {
            m = mask(poly, v);
        }

        crc ^= m;

        buf = ((uint8_t *)buf) + 1;
        len--;
    }

    return crc;
}

static uint8_t crc32be_upd(uint32_t * const crc, const uint8_t b)
{
    const uint8_t v = (*crc >> 24) ^ b;
    *crc <<= 8;
    return v;
}

uint32_t crc32be(const uint32_t tab[256], const uint32_t poly,
                 uint32_t crc, const void * buf, size_t len)
{
    return crc32(tab, poly, crc, buf, len, crc32be_upd, crc32be_mask);
}

static uint8_t crc32le_upd(uint32_t * const crc, const uint8_t b)
{
    const uint8_t v = *crc ^ b;
    *crc >>= 8;
    return v;
}

uint32_t crc32le(const uint32_t tab[256], const uint32_t poly,
                 uint32_t crc, const void * buf, size_t len)
{
    return crc32(tab, poly, crc, buf, len, crc32le_upd, crc32le_mask);
}

#ifdef __cfg_test__
// GCOV_EXCL_START
#include <check.h>

static void crc32_test(const uint32_t poly,
                       const uint32_t init, const uint32_t xout,
                       const uint32_t chck,
                       uint32_t (* const crcf)(const uint32_t [256], uint32_t,
                               uint32_t, const void *, size_t))
{
    static const char * inpt = "123456789";

    uint32_t tab[256];

    if (crcf == crc32le) {
        crc32le_table(tab, poly);
    } else {
        crc32be_table(tab, poly);
    }

    ck_assert_uint_eq(
        chck,
        crcf(NULL, poly,
             init,
             inpt, strlen(inpt)) ^ xout);
    ck_assert_uint_eq(
        chck,
        crcf(tab, poly,
             init,
             inpt, strlen(inpt)) ^ xout);
}

START_TEST(crc32_hdlc)
{
    crc32_test(0xedb88320, ~0, ~0, 0xcbf43926, crc32le);
}
END_TEST

START_TEST(crc32_iscsi)
{
    crc32_test(0x82f63b78, ~0, ~0, 0xe3069283, crc32le);
}
END_TEST

START_TEST(crc32_cdrom)
{
    crc32_test(0xd8018001, 0, 0, 0x6ec2edc4, crc32le);
}
END_TEST

START_TEST(crc32_cksum)
{
    crc32_test(0x04c11db7, 0, ~0, 0x765e7680, crc32be);
}
END_TEST

START_TEST(crc32_bzip2)
{
    crc32_test(0x04c11db7, ~0, ~0, 0xfc891918, crc32be);
}
END_TEST

Suite * crc_suite(void)
{
    Suite * const s = suite_create("crc");

    TCase * tc;

    tc = tcase_create("crc");
    tcase_add_test(tc, crc32_hdlc);
    tcase_add_test(tc, crc32_iscsi);
    tcase_add_test(tc, crc32_cdrom);
    tcase_add_test(tc, crc32_cksum);
    tcase_add_test(tc, crc32_bzip2);

    suite_add_tcase(s, tc);

    return s;
}

// GCOV_EXCL_STOP
#endif
