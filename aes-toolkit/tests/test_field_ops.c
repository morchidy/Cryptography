/**
 * @file test_field_ops.c
 * @brief Unit tests for GF(2^8) field operations
 */

#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include "../src/core/field_ops.h"

/**
 * @brief Test xtime function with known test vectors
 */
void test_xtime(void)
{
    printf("Testing xtime() function...\n");
    
    /* Test case 1: 0x57 * x = 0xAE */
    uint8_t input1 = 0x57;
    uint8_t expected1 = 0xAE;
    uint8_t result1 = xtime(input1);
    assert(result1 == expected1);
    printf("  xtime(0x%02X) = 0x%02X [PASS]\n", input1, result1);
    
    /* Test case 2: 0x83 * x = 0x1D (reduction needed) */
    uint8_t input2 = 0x83;
    uint8_t expected2 = 0x1D;
    uint8_t result2 = xtime(input2);
    assert(result2 == expected2);
    printf("  xtime(0x%02X) = 0x%02X [PASS]\n", input2, result2);
    
    /* Test case 3: 0x00 * x = 0x00 */
    uint8_t input3 = 0x00;
    uint8_t expected3 = 0x00;
    uint8_t result3 = xtime(input3);
    assert(result3 == expected3);
    printf("  xtime(0x%02X) = 0x%02X [PASS]\n", input3, result3);
    
    /* Test case 4: 0xFF * x = 0xE5 */
    uint8_t input4 = 0xFF;
    uint8_t expected4 = 0xE5;
    uint8_t result4 = xtime(input4);
    assert(result4 == expected4);
    printf("  xtime(0x%02X) = 0x%02X [PASS]\n", input4, result4);
    
    printf("xtime() tests: ALL PASSED\n\n");
}

/**
 * @brief Test gf_mul3 function
 */
void test_gf_mul3(void)
{
    printf("Testing gf_mul3() function...\n");
    
    /* 3 * a = xtime(a) XOR a */
    uint8_t a = 0x57;
    uint8_t result = gf_mul3(a);
    uint8_t expected = xtime(a) ^ a;  /* 0xAE ^ 0x57 = 0xF9 */
    assert(result == expected);
    printf("  gf_mul3(0x%02X) = 0x%02X [PASS]\n", a, result);
    
    printf("gf_mul3() tests: ALL PASSED\n\n");
}

/**
 * @brief Test general multiplication
 */
void test_gf_mul(void)
{
    printf("Testing gf_mul() function...\n");
    
    /* Test: 0x53 * 0xCA = 0x01 (multiplicative inverse) */
    uint8_t a1 = 0x53;
    uint8_t b1 = 0xCA;
    uint8_t result1 = gf_mul(a1, b1);
    uint8_t expected1 = 0x01;
    assert(result1 == expected1);
    printf("  gf_mul(0x%02X, 0x%02X) = 0x%02X [PASS]\n", a1, b1, result1);
    
    /* Test: a * 0 = 0 */
    uint8_t a2 = 0xAB;
    uint8_t b2 = 0x00;
    uint8_t result2 = gf_mul(a2, b2);
    uint8_t expected2 = 0x00;
    assert(result2 == expected2);
    printf("  gf_mul(0x%02X, 0x%02X) = 0x%02X [PASS]\n", a2, b2, result2);
    
    /* Test: a * 1 = a */
    uint8_t a3 = 0x7F;
    uint8_t b3 = 0x01;
    uint8_t result3 = gf_mul(a3, b3);
    uint8_t expected3 = 0x7F;
    assert(result3 == expected3);
    printf("  gf_mul(0x%02X, 0x%02X) = 0x%02X [PASS]\n", a3, b3, result3);
    
    printf("gf_mul() tests: ALL PASSED\n\n");
}

/**
 * @brief Test field properties (commutativity, associativity)
 */
void test_field_properties(void)
{
    printf("Testing field properties...\n");
    
    /* Test commutativity: a * b = b * a */
    uint8_t a = 0x12;
    uint8_t b = 0x34;
    uint8_t ab = gf_mul(a, b);
    uint8_t ba = gf_mul(b, a);
    assert(ab == ba);
    printf("  Commutativity: 0x%02X * 0x%02X = 0x%02X * 0x%02X [PASS]\n", 
           a, b, b, a);
    
    /* Test distributivity: a * (b + c) = a*b + a*c */
    uint8_t c = 0x56;
    uint8_t bc = b ^ c;  /* addition in GF(2^8) is XOR */
    uint8_t a_bc = gf_mul(a, bc);
    uint8_t ab_ac = gf_mul(a, b) ^ gf_mul(a, c);
    assert(a_bc == ab_ac);
    printf("  Distributivity: a*(b+c) = a*b + a*c [PASS]\n");
    
    printf("Field property tests: ALL PASSED\n\n");
}

/**
 * @brief Main test driver
 */
int main(void)
{
    printf("=== GF(2^8) Field Operations Test Suite ===\n\n");
    
    test_xtime();
    test_gf_mul3();
    test_gf_mul();
    test_field_properties();
    
    printf("=== ALL TESTS PASSED ===\n");
    
    return 0;
}
