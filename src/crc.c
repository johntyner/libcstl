/*!
 * @file
 */

#include <stdint.h>
#include <stddef.h>

#include "cstl/bits.h"

#ifndef NO_DOC
#define CRCbe_MASK_BIT(WIDTH, MASK, POLY)                       \
    do {                                                        \
        const uint##WIDTH##_t __POLY[] = { 0, POLY };           \
        MASK = (MASK << 1) ^                                    \
            __POLY[(MASK >> ((8 * sizeof(MASK)) - 1)) & 1];     \
    } while (0)
#define CRCle_MASK_BIT(WIDTH, MASK, POLY)               \
    do {                                                \
        const uint##WIDTH##_t __POLY[] = { 0, POLY };   \
        MASK = (MASK >> 1) ^ __POLY[MASK & 1];          \
    } while (0)

#define CRC_MASK_BITS(WIDTH, MASKBIT, MASK, POLY)       \
    do {                                                \
        unsigned int __i;                               \
        for (__i = 0; __i < 8; __i++) {                 \
            MASKBIT(WIDTH, MASK, POLY);                 \
        }                                               \
    } while (0)

#define CRCbe_MASK(WIDTH, MASK, POLY, VAL)                              \
    do {                                                                \
        MASK = ((uint##WIDTH##_t)VAL) << ((8 * sizeof(MASK)) - 8);      \
        CRC_MASK_BITS(WIDTH, CRCbe_MASK_BIT, MASK, POLY);               \
    } while (0)
#define CRCle_MASK(WIDTH, MASK, POLY, VAL)                      \
    do {                                                        \
        MASK = VAL;                                             \
        CRC_MASK_BITS(WIDTH, CRCle_MASK_BIT, MASK, POLY);       \
    } while (0)

#define DEFINE_CRC_MASK_FUNCTION(WIDTH, END)            \
    uint##WIDTH##_t crc##WIDTH##END##_mask(             \
        const uint##WIDTH##_t poly, const uint8_t v)    \
    {                                                   \
        uint##WIDTH##_t m;                              \
        CRC##END##_MASK(WIDTH, m, poly, v);             \
        return m;                                       \
    } struct __swallow_semicolon

#define DEFINE_CRC_TABLE_FUNCTION(WIDTH)                        \
    void crc##WIDTH##_table(uint##WIDTH##_t TAB[256],           \
                            const uint##WIDTH##_t POLY,         \
                            uint##WIDTH##_t (* const MASK)(     \
                                uint##WIDTH##_t, uint8_t))      \
    {                                                           \
        unsigned int i;                                         \
        for (i = 0; i < 256; i++)                               \
            TAB[i] = MASK(POLY, i);                             \
    } struct __swallow_semicolon

#define DEFINE_CRC_FUNCTION(WIDTH)                                      \
    uint##WIDTH##_t crc##WIDTH(                                         \
        const uint##WIDTH##_t tab[256],                                 \
        const uint##WIDTH##_t poly,                                     \
        uint##WIDTH##_t crc,                                            \
        const void * buf, size_t len,                                   \
        uint8_t (* const upd)(uint##WIDTH##_t *, uint8_t),              \
        uint##WIDTH##_t (* const mask)(uint##WIDTH##_t, uint8_t))       \
    {                                                                   \
        while (len > 0) {                                               \
            const uint8_t v = upd(&crc, *(uint8_t *)buf);               \
            uint##WIDTH##_t m;                                          \
            if (tab != NULL) {                                          \
                m = tab[v];                                             \
            } else {                                                    \
                m = mask(poly, v);                                      \
            }                                                           \
            crc ^= m;                                                   \
            buf = ((uint8_t *)buf) + 1;                                 \
            len--;                                                      \
        }                                                               \
        return crc;                                                     \
    } struct __swallow_semicolon

#define CRCbe_UPD(RES, CRC, VAL)                        \
    do {                                                \
        RES = (CRC >> ((8 * sizeof(CRC)) - 8)) ^ VAL;   \
        CRC <<= 8;                                      \
    } while (0)
#define CRCle_UPD(RES, CRC, VAL)                \
    do {                                        \
        RES = CRC ^ VAL;                        \
        CRC >>= 8;                              \
    } while (0)

#define DEFINE_CRC_UPD_FUNCTION(WIDTH, END)             \
    uint8_t crc##WIDTH##END##_upd(                      \
        uint##WIDTH##_t * const crc, const uint8_t b)   \
    {                                                   \
        uint8_t v;                                      \
        CRC##END##_UPD(v, *crc, b);                     \
        return v;                                       \
    } struct __swallow_semicolon

#define CALL_CRC_TABLE(WIDTH, END, TAB, POLY)                   \
    crc##WIDTH##_table(TAB, POLY, crc##WIDTH##END##_mask)

#define CALL_CRC(WIDTH, END, TAB, POLY, INIT, BUF, LEN)         \
    crc##WIDTH(TAB, POLY, INIT, BUF, LEN,                       \
               crc##WIDTH##END##_upd, crc##WIDTH##END##_mask)

#define DEFINE_CRC_SUPPORT(WIDTH)               \
    static DEFINE_CRC_MASK_FUNCTION(WIDTH, be); \
    static DEFINE_CRC_MASK_FUNCTION(WIDTH, le); \
    static DEFINE_CRC_UPD_FUNCTION(WIDTH, be);  \
    static DEFINE_CRC_UPD_FUNCTION(WIDTH, le);  \
    static DEFINE_CRC_TABLE_FUNCTION(WIDTH);    \
    static DEFINE_CRC_FUNCTION(WIDTH)

DEFINE_CRC_SUPPORT(32);
DEFINE_CRC_SUPPORT(64);
#endif /* NO_DOC */

void cstl_crc32be_table(uint32_t tab[256], const uint32_t poly)
{
    CALL_CRC_TABLE(32, be, tab, poly);
}

void cstl_crc32le_table(uint32_t tab[256], const uint32_t poly)
{
    CALL_CRC_TABLE(32, le, tab, cstl_reflect32(poly));
}

uint32_t cstl_crc32be(const uint32_t tab[256], const uint32_t poly,
                      const uint32_t init, const void * buf, size_t len)
{
    return CALL_CRC(32, be, tab, poly, init, buf, len);
}

uint32_t cstl_crc32le(const uint32_t tab[256], const uint32_t poly,
                      const uint32_t init, const void * buf, size_t len)
{
    return CALL_CRC(32, le, tab, cstl_reflect32(poly), init, buf, len);
}

void cstl_crc64be_table(uint64_t tab[256], const uint64_t poly)
{
    CALL_CRC_TABLE(64, be, tab, poly);
}

void cstl_crc64le_table(uint64_t tab[256], const uint64_t poly)
{
    CALL_CRC_TABLE(64, le, tab, cstl_reflect64(poly));
}

uint64_t cstl_crc64be(const uint64_t tab[256], const uint64_t poly,
                      const uint64_t init, const void * buf, size_t len)
{
    return CALL_CRC(64, be, tab, poly, init, buf, len);
}

uint64_t cstl_crc64le(const uint64_t tab[256], const uint64_t poly,
                      const uint64_t init, const void * buf, size_t len)
{
    return CALL_CRC(64, le, tab, cstl_reflect64(poly), init, buf, len);
}

#ifdef __cfg_test__
// GCOV_EXCL_START
#include <check.h>

/*
 * creates crcXX_test(poly, init, xor_out, check_val,
 *                    crc_func *, crc_tab_func *)
 */
#define DEFINE_CRC_TEST(WIDTH)                                  \
    void crc##WIDTH##_test(                                     \
        const uint##WIDTH##_t poly,                             \
        const uint##WIDTH##_t init, const uint##WIDTH##_t xout, \
        const uint##WIDTH##_t chck,                             \
        uint##WIDTH##_t (* const crcf)(                         \
            const uint##WIDTH##_t [256], uint##WIDTH##_t,       \
            uint##WIDTH##_t, const void *, size_t),             \
        void (* const crct)(                                    \
            uint##WIDTH##_t [256], uint##WIDTH##_t))            \
    {                                                           \
        static const char * inpt = "123456789";                 \
        uint##WIDTH##_t tab[256];                               \
        crct(tab, poly);                                        \
        ck_assert_uint_eq(                                      \
            chck,                                               \
            crcf(NULL, poly,                                    \
                 init,                                          \
                 inpt, strlen(inpt)) ^ xout);                   \
        ck_assert_uint_eq(                                      \
            chck,                                               \
            crcf(tab, poly,                                     \
                 init,                                          \
                 inpt, strlen(inpt)) ^ xout);                   \
    } struct __swallow_semicolon

static DEFINE_CRC_TEST(32); // creates crc32_test()
static DEFINE_CRC_TEST(64); // creates crc64_test()

#define CALL_CRC_TEST(WIDTH, END, POLY, INIT, XOUT, CHK)        \
    crc##WIDTH##_test(POLY, INIT, XOUT, CHK,                    \
                      cstl_crc##WIDTH##END,                     \
                      cstl_crc##WIDTH##END##_table)

START_TEST(crc32_hdlc)
{
    CALL_CRC_TEST(32, le, 0x04c11db7, ~0, ~0, 0xcbf43926);
}
END_TEST

START_TEST(crc32_iscsi)
{
    CALL_CRC_TEST(32, le, 0x1edc6f41, ~0, ~0, 0xe3069283);
}
END_TEST

START_TEST(crc32_cdrom)
{
    CALL_CRC_TEST(32, le, 0x8001801b, 0, 0, 0x6ec2edc4);
}
END_TEST

START_TEST(crc32_cksum)
{
    CALL_CRC_TEST(32, be, 0x04c11db7, 0, ~0, 0x765e7680);
}
END_TEST

START_TEST(crc32_bzip2)
{
    CALL_CRC_TEST(32, be, 0x04c11db7, ~0, ~0, 0xfc891918);
}
END_TEST

START_TEST(crc64_goiso)
{
    CALL_CRC_TEST(64, le, 0x1b, ~0, ~0, 0xb90956c775a41001);
}
END_TEST

START_TEST(crc64_ecma182)
{
    CALL_CRC_TEST(64, be, 0x42f0e1eba9ea3693, 0, 0, 0x6c40df5f0b497347);
}
END_TEST

Suite * crc_suite(void)
{
    Suite * const s = suite_create("crc");

    TCase * tc;

    tc = tcase_create("crc32");
    tcase_add_test(tc, crc32_hdlc);
    tcase_add_test(tc, crc32_iscsi);
    tcase_add_test(tc, crc32_cdrom);
    tcase_add_test(tc, crc32_cksum);
    tcase_add_test(tc, crc32_bzip2);
    suite_add_tcase(s, tc);

    tc = tcase_create("crc64");
    tcase_add_test(tc, crc64_goiso);
    tcase_add_test(tc, crc64_ecma182);
    suite_add_tcase(s, tc);

    return s;
}

// GCOV_EXCL_STOP
#endif
