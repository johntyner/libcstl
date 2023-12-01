/*!
 * @file
 */

#ifndef CSTL_BITS_H
#define CSTL_BITS_H

/*!
 * @brief Find the last (highest order) bit set
 *
 * @return Zero-based index of the highest order set bit
 * @retval -1 No bits are set, i.e. the input value is zero
 */
int cstl_fls(unsigned long);

/*!
 * @brief Reverse the bits of an input value
 *
 * @return The value of the input with its bits in the reverse order
 */
uint8_t cstl_reflect8(uint8_t);
/*! @copydoc cstl_reflect8 */
uint16_t cstl_reflect16(uint16_t);
/*! @copydoc cstl_reflect8 */
uint32_t cstl_reflect32(uint32_t);
/*! @copydoc cstl_reflect8 */
uint64_t cstl_reflect64(uint64_t);

#endif
