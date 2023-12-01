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

#endif
