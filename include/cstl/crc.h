/*!
 * @file
 */

#ifndef CSTL_CRC_H
#define CSTL_CRC_H

#include <stdint.h>
#include <stddef.h>

/*
 * crc functions come in two varieties: big and little endian.
 * the difference is that the big endian variety processes messages
 * starting with the high order bit of each input byte, and little
 * endian starts with the low order.
 *
 * in documentation around the internet, some specifications mention
 * "reflection" of the input. this tends to correspond with the little
 * endian family of functions.
 *
 * the same polynomial should be passed to both varieties of functions
 * for a given CRC. if any special handling is necessary to accommodate
 * the differences between the big and little endian versions of the
 * functions, it is handled internally.
 *
 * the functions are byte oriented but must process those bytes one bit
 * at a time to perform the crc calculations. functions are provided to
 * pre-generate calculations for all possible input bytes, and these
 * tables can be passed to the corresponding crc functions to enable
 * processing at the byte (as opposed to bit) level. this is highly
 * recommended for callers that need to do repeated crc calculations
 * and/or are sensitive to the time required for each calculation.
 */

/*!
 * @brief Generate values for byte-oriented CRC calculation
 *
 * @param[out] tab An array large enough to hold the generated values
 * @param[in] poly The polynomial associated with the CRC function
 */
void cstl_crc32be_table(uint32_t[256], uint32_t);

/*! @copydoc cstl_crc32be_table */
void cstl_crc32le_table(uint32_t[256], uint32_t);

/*!
 * @brief Calculate the CRC over a given memory region
 *
 * @param[in] tab A pre-generated table of values for byte-oriented processing.
 *                This value may be NULL.
 * @param[in] poly The polynomial associated with the CRC function. If @p tab
 *                 is not @p NULL, this parameter is ignored
 * @param[in] init The initial value for the CRC calculation. If the CRC is
 *                 to be calculated over multiple, separate memory regions,
 *                 this parameter should be set to the result of the previous
 *                 call to this function
 * @param[in] buf A pointer to a memory region
 * @param[in] len The number of bytes pointed to by @p buf
 *
 * @return The calculated CRC value
 *
 * @note If the defined CRC requires an XOR with the final result, that
 *       operation must be carried out by the caller on the result of final
 *       call to this function.
 */
uint32_t cstl_crc32be(uint32_t[256], uint32_t, uint32_t, const void *, size_t);

/*! @copydoc cstl_crc32be */
uint32_t cstl_crc32le(uint32_t[256], uint32_t, uint32_t, const void *, size_t);

#endif
