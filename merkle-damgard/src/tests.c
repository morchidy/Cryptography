#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <inttypes.h>
#include "second_preim.h"


/* Test against EP 2013/404, App. C */
bool test_vector_okay()
{
    uint32_t k[4] = {0x1a1918, 0x121110, 0x0a0908, 0x020100};
    uint32_t p[2] = {0x6d2073, 0x696874};
    uint32_t c[2];
    speck48_96(k, p, c);
    printf("%X %X\n", c[0], c[1]);

    return (c[0] == 0x735E10) && (c[1] == 0xB6445D);
}

/* Test that speck48_96_inv is the inverse of speck48_96 */
int test_sp48_inv(void){
	// Test with the known test vector
    uint32_t k[4] = {0x1a1918, 0x121110, 0x0a0908, 0x020100};
    uint32_t p_orig[2] = {0x6d2073, 0x696874};
    uint32_t c[2];
    uint32_t p_decrypted[2];

    // Encrypt
    speck48_96(k, p_orig, c);
    
    // Decrypt
    speck48_96_inv(k, c, p_decrypted);

    // Check if decryption gives back the original plaintext
    bool success = (p_decrypted[0] == p_orig[0]) && (p_decrypted[1] == p_orig[1]);
    
    printf("Inverse test: p_orig = (%06X, %06X), c = (%06X, %06X), p_dec = (%06X, %06X)\n",
           p_orig[0], p_orig[1], c[0], c[1], p_decrypted[0], p_decrypted[1]);
    
    if (success) {
        printf("Inverse test passed!\n");
    } else {
        printf("Inverse test failed!\n");
    }
	return success;
}

// Test of correctness of the function cs48_dm
int test_cs48_dm(void)
{
    uint32_t m[4] = {0, 1, 2, 3};
    uint64_t h = 0x010203040506ULL;

    uint64_t out = cs48_dm(m, h);

    printf("cs48_dm output = %012" PRIX64 "\n", out);

    if (out == 0x5DFD97183F91ULL) {
        printf("cs48_dm test passed!\n");
        return 1;
    } else {
        printf("cs48_dm test FAILED (expected 5DFD97183F91)\n");
        return 0;
    }
}

// Test cs48_dm_fp
int test_cs48_dm_fp(void)
{
    uint32_t m[4] = {0, 1, 2, 3};

    uint64_t fp = get_cs48_dm_fp(m);

    /* Check that fp is indeed a fixed point */
    uint64_t h2 = cs48_dm(m, fp);

    printf("fp = %012" PRIX64 "\n", fp & 0xFFFFFFFFFFFFULL);
    printf("cs48_dm(m, fp) = %012" PRIX64 "\n", h2 & 0xFFFFFFFFFFFFULL);

    if ((fp & 0xFFFFFFFFFFFFULL) == (h2 & 0xFFFFFFFFFFFFULL)) {
        printf("Fixed-point test passed!\n");
        return 1;
    } else {
        printf("Fixed-point test FAILED!\n");
        return 0;
    }
}

// test to validate the the function find_exp_mess
int test_em(void)
{
    uint32_t m1[4], m2[4];
    find_exp_mess(m1, m2);

    // h computed from a single m1 block
    uint64_t h = cs48_dm(m1, IV);

    // build message: m1 || m2^n  (e.g. n = 5)
    const int n = 5;
    uint32_t *msg = malloc((1 + n) * 4 * sizeof(uint32_t));
    if (!msg) {
        fprintf(stderr, "malloc failed in test_em\n");
        return 0;
    }

    for (int j = 0; j < 4; j++) msg[j] = m1[j];
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < 4; j++) {
            msg[4 * (1 + i) + j] = m2[j];
        }
    }

    uint64_t h2 = hs48(msg, (uint64_t)(1 + n), 0, 0);

    printf("Expandable message test:\n");
    printf("m1 = %06X %06X %06X %06X\n", m1[0], m1[1], m1[2], m1[3]);
    printf("m2 = %06X %06X %06X %06X\n", m2[0], m2[1], m2[2], m2[3]);
    printf("cs48_dm(m1, IV)       = %012" PRIX64 "\n", h & 0xFFFFFFFFFFFFULL);
    printf("hs48(m1 || m2^%d) = %012" PRIX64 "\n", n, h2 & 0xFFFFFFFFFFFFULL);

    free(msg);

    if ((h & 0xFFFFFFFFFFFFULL) == (h2 & 0xFFFFFFFFFFFFULL)) {
        printf("Expandable message test PASSED\n");
        return 1;
    } else {
        printf("Expandable message test FAILED\n");
        return 0;
    }
}
