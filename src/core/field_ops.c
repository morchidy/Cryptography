/**
 * @file field_ops.c
 * @brief Implementation of GF(2^8) arithmetic
 */

#include "field_ops.h"

/**
 * @brief Constant-time multiplication by x in GF(2^8)
 * 
 * This implementation uses bitwise operations to avoid conditional
 * branches, ensuring constant execution time regardless of input values.
 * This property is crucial for side-channel attack resistance.
 * 
 * The AES field is defined by the irreducible polynomial:
 *   m(x) = x^8 + x^4 + x^3 + x + 1 = 0x11B
 * 
 * Multiplication by x corresponds to a left shift by 1 bit.
 * If the result exceeds degree 7 (i.e., bit 7 of input was 1),
 * we reduce modulo m(x) by XORing with 0x1B.
 * 
 * Algorithm:
 *   1. Extract MSB: m = p >> 7 (m is 0 or 1)
 *   2. Create mask: m ^= 1; m -= 1; (m is 0x00 or 0xFF)
 *   3. Select reduction: m &= 0x1B (m is 0x00 or 0x1B)
 *   4. Return shifted and reduced: (p << 1) ^ m
 * 
 * @param p Input byte representing a polynomial in GF(2^8)
 * @return Result of p*x mod m(x)
 */
uint8_t xtime(uint8_t p)
{
    uint8_t m = p >> 7;  /* Extract most significant bit */
    
    /* Convert 0/1 to 0x00/0xFF mask without branching */
    m ^= 1;              /* m = 1 - m */
    m -= 1;              /* m = 0 becomes 0xFF, m = 1 becomes 0x00 */
    m &= 0x1B;           /* Select reduction polynomial if needed */
    
    /* Shift left and reduce if necessary */
    return ((p << 1) ^ m);
}

/**
 * @brief General multiplication in GF(2^8) using peasant multiplication
 * 
 * Implements the "double-and-add" algorithm:
 *   a * b = a * (b7*x^7 + b6*x^6 + ... + b1*x + b0)
 *         = b7*(a*x^7) + b6*(a*x^6) + ... + b1*(a*x) + b0*a
 * 
 * @param a First operand
 * @param b Second operand
 * @return Product a*b in GF(2^8)
 */
uint8_t gf_mul(uint8_t a, uint8_t b)
{
    uint8_t result = 0;
    uint8_t temp_a = a;
    
    for (int i = 0; i < 8; i++) {
        /* If bit i of b is set, add temp_a to result */
        if (b & 1) {
            result ^= temp_a;
        }
        
        /* Multiply temp_a by x for next iteration */
        temp_a = xtime(temp_a);
        
        /* Shift b right to process next bit */
        b >>= 1;
    }
    
    return result;
}
