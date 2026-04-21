/**
 * @file field_ops.h
 * @brief Finite field GF(2^8) arithmetic operations
 * 
 * Implements operations in the finite field used by AES, defined by
 * the irreducible polynomial m(x) = x^8 + x^4 + x^3 + x + 1.
 */

#ifndef FIELD_OPS_H
#define FIELD_OPS_H

#include <stdint.h>

/**
 * @brief Multiplication by x in GF(2^8)
 * 
 * Constant-time implementation using broadcast-based masking.
 * Implements polynomial multiplication by x modulo the AES
 * irreducible polynomial 0x11B (x^8 + x^4 + x^3 + x + 1).
 * 
 * @param p Input byte
 * @return Result of p * x mod m(x)
 * 
 * @note This is constant-time to prevent timing side-channel attacks
 */
uint8_t xtime(uint8_t p);

/**
 * @brief Multiplication by 2 in GF(2^8) (alias for xtime)
 */
#define gf_mul2(p) xtime(p)

/**
 * @brief Multiplication by 3 in GF(2^8)
 * 
 * Computes 3*p = (x+1)*p = x*p + p
 * 
 * @param p Input byte
 * @return Result of 3*p in GF(2^8)
 */
static inline uint8_t gf_mul3(uint8_t p) {
    return xtime(p) ^ p;
}

/**
 * @brief General multiplication in GF(2^8)
 * 
 * @param a First operand
 * @param b Second operand
 * @return Product a*b in GF(2^8)
 */
uint8_t gf_mul(uint8_t a, uint8_t b);

#endif /* FIELD_OPS_H */
