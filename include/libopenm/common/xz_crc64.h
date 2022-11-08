/*
 * File below is a small portion from original xz.h file:
 *
 * XZ decompressor
 *
 * Authors: Lasse Collin <lasse.collin@tukaani.org>
 *          Igor Pavlov <http://7-zip.org/>
 *
 * This file has been put into the public domain.
 * You can do whatever you want with this file.
 */

#ifndef XZ_CRC64_H
#define XZ_CRC64_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef XZ_EXTERN
#	define XZ_EXTERN extern
#endif

/*
 * This must be called before any other xz_* function (except xz_crc32_init())
 * to initialize the CRC64 lookup table.
 */
XZ_EXTERN void xz_crc64_init(void);

/*
 * Update CRC64 value using the polynomial from ECMA-182. To start a new
 * calculation, the third argument must be zero. To continue the calculation,
 * the previously returned value is passed as the third argument.
 */
XZ_EXTERN uint64_t xz_crc64(const uint8_t *buf, size_t size, uint64_t crc);

#ifdef __cplusplus
}
#endif

#endif  // XZ_CRC64_H
