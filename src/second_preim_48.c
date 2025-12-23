#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <inttypes.h>
#include "second_preim.h"

#define ROTL24_16(x) ((((x) << 16) ^ ((x) >> 8)) & 0xFFFFFF) // = ROR24(x,8)
#define ROTL24_3(x) ((((x) << 3) ^ ((x) >> 21)) & 0xFFFFFF) // = ROL24(x,3)

#define ROTL24_8(x) ((((x) << 8) ^ ((x) >> 16)) & 0xFFFFFF) // = ROL24(x,8)
#define ROTL24_21(x) ((((x) << 21) ^ ((x) >> 3)) & 0xFFFFFF) // = ROR24(x,3)

#define IV 0x010203040506ULL

// External xoshiro PRNG (defined in helpers.c via xoshiro.h)
extern uint64_t xoshiro256plus_random(void);

/*
 * the 96-bit key is stored in four 24-bit chunks in the low bits of k[0]...k[3]
 * the 48-bit plaintext is stored in two 24-bit chunks in the low bits of p[0], p[1]
 * the 48-bit ciphertext is written similarly in c
 */
void speck48_96(const uint32_t k[4], const uint32_t p[2], uint32_t c[2])
{
	uint32_t rk[23];
	uint32_t ell[3] = {k[2], k[1], k[0]};

	rk[0] = k[3];

	c[0] = p[0];
	c[1] = p[1];

	/* full key schedule */
	for (unsigned i = 0; i < 22; i++)
	{
		uint32_t new_ell = ((ROTL24_16(ell[0]) + rk[i]) ^ i) & 0xFFFFFF; // addition (+) is done mod 2**24
		rk[i+1] = ROTL24_3(rk[i]) ^ new_ell;
		ell[0] = ell[1];
		ell[1] = ell[2];
		ell[2] = new_ell;
	}

	for (unsigned i = 0; i < 23; i++)
	{
		c[0] = ( (ROTL24_16(c[0]) + c[1]) ^ rk[i] ) & 0xFFFFFF;
    	c[1] = ( ROTL24_3(c[1]) ^ c[0] ) & 0xFFFFFF;

		// we could do it like this
	/*
		// We keep everything strictly within 24 bits.
		const uint32_t MASK24 = 0xFFFFFF;

		// Snapshot current state words (each is 24 bits stored in the low bits).
		uint32_t x = c[0] & MASK24;
		uint32_t y = c[1] & MASK24;

		// --- Round i ---

		// 1) x = ROR24(x, α) with α = 8.
		x = ROTL24_16(x);

		// 2) x = (x + y) mod 2^24.
		x = (x + y) & MASK24;

		// 3) x = x XOR rk[i].
		x ^= (rk[i] & MASK24);

		// 4) y = ROL24(y, β) with β = 3
		y = ROTL24_3(y);

		// 5) y = y XOR x.
		y ^= x;

		// Write back state for next round.
		c[0] = x & MASK24;
		c[1] = y & MASK24;
	*/
	}

	return;
}

/* the inverse cipher */
void speck48_96_inv(const uint32_t k[4], const uint32_t c[2], uint32_t p[2])
{
	uint32_t rk[23];
    uint32_t ell[3] = {k[2], k[1], k[0]};

    rk[0] = k[3];

    p[0] = c[0];
    p[1] = c[1];

    /* full key schedule (same as encryption) */
    for (unsigned i = 0; i < 22; i++)
    {
        uint32_t new_ell = ((ROTL24_16(ell[0]) + rk[i]) ^ i) & 0xFFFFFF;
        rk[i+1] = ROTL24_3(rk[i]) ^ new_ell;
        ell[0] = ell[1];
        ell[1] = ell[2];
        ell[2] = new_ell;
    }

    /* decrypt in reverse order */
    for (int i = 22; i >= 0; i--)
    {
        // Reverse step 6: y = y ^ x  =>  y = y ^ x
        p[1] = p[1] ^ p[0];
        
        // Reverse step 5: y = ROL(y, 3)  =>  y = ROR(y, 3)
        p[1] = ROTL24_21(p[1]);
        
        // Reverse step 3: x = x ^ rk[i]  =>  x = x ^ rk[i]
        p[0] = p[0] ^ rk[i];
        
        // Reverse step 2: x = x + y  =>  x = x - y
        p[0] = (p[0] - p[1]) & 0xFFFFFF;
        
        // Reverse step 1: x = ROR(x, 8)  =>  x = ROL(x, 8)
        p[0] = ROTL24_8(p[0]);
    }

    return;
}


/* The Davies-Meyer compression function based on speck48_96,
 * using an XOR feedforward
 * The input/output chaining value is given on the 48 low bits of a single 64-bit word,
 * whose 24 lower bits are set to the low half of the "plaintext"/"ciphertext" (p[0]/c[0])
 * and whose 24 higher bits are set to the high half (p[1]/c[1])
 */
uint64_t cs48_dm(const uint32_t m[4], const uint64_t h)
{
	uint32_t p[2];
    uint32_t c[2];

    /* extract 48-bit chaining value from h */
    p[0] = h & 0xFFFFFF;          // low 24 bits
    p[1] = (h >> 24) & 0xFFFFFF;  // next 24 bits

    /* encrypt h using message block m as the key */
    speck48_96(m, p, c);

    /* rebuild 48-bit output into a 64-bit word */
    uint64_t out = ((uint64_t)c[1] << 24) | c[0];

    /* DM feedforward */
    out ^= (h & 0xFFFFFFFFFFFFULL);

    return out;
}


/* Assumes message length is fourlen * four blocks of 24 bits, each stored as the low bits of 32-bit words
 * fourlen is stored on 48 bits (as the 48 low bits of a 64-bit word)
 * when padding is included, simply adds one block (96 bits) of padding with fourlen and zeros on higher bits
 * (The fourlen * four blocks must be considered “full”, otherwise trivial collisions are possible)
 */
uint64_t hs48(const uint32_t *m, uint64_t fourlen, int padding, int verbose)
{
	uint64_t h = IV;
	const uint32_t *mp = m;

	for (uint64_t i = 0; i < fourlen; i++)
	{
		h = cs48_dm(mp, h);
		if (verbose)
			printf("@%" PRIu64 " : %06X %06X %06X %06X => %06" PRIX64 "\n", i, mp[0], mp[1], mp[2], mp[3], h);
		mp += 4;
	}
	if (padding)
	{
		uint32_t pad[4];
		pad[0] = fourlen & 0xFFFFFF;
		pad[1] = (fourlen >> 24) & 0xFFFFFF;
		pad[2] = 0;
		pad[3] = 0;
		h = cs48_dm(pad, h);
		if (verbose)
			printf("@%" PRIu64 " : %06X %06X %06X %06X => %06" PRIX64 "\n", fourlen, pad[0], pad[1], pad[2], pad[3], h);
	}

	return h;
}

/* Computes the unique fixed-point for cs48_dm for the message m */
uint64_t get_cs48_dm_fp(uint32_t m[4])
{
	uint32_t zero[2] = {0, 0};
    uint32_t fp[2];

    /* We want fp such that E(m, fp) = 0.0 */
    speck48_96_inv(m, zero, fp);

    /* pack into 48-bit value inside 64-bit word */
    uint64_t h =
        ((uint64_t)(fp[1] & 0xFFFFFF) << 24) |
        ((uint64_t)(fp[0] & 0xFFFFFF));

    return h;
}


/* Finds a two-block expandable message for hs48, using a fixed-point
 * That is, computes m1, m2 s.t. hs48_nopad(m1||m2) = hs48_nopad(m1||m2^*),
 * where hs48_nopad is hs48 with no padding */
void find_exp_mess(uint32_t m1[4], uint32_t m2[4]);  // Implemented in helpers.c

void attack(void)
{
     const uint32_t NBLOCKS = 1u << 18;      // 2^18 blocks in the target message
    const uint64_t FOURLEN = (uint64_t)NBLOCKS;

    // --- 1) Build the target message mess as in the statement ---

    uint32_t *mess  = malloc(4 * NBLOCKS * sizeof(uint32_t));
    uint32_t *mess2 = malloc(4 * NBLOCKS * sizeof(uint32_t));
    if (!mess || !mess2) {
        fprintf(stderr, "malloc failed in attack()\n");
        exit(1);
    }

    // mess[i + 0] = i; mess[i+1..3] = 0; with i stepping by 4
    // There are (1<<20)/4 = 2^18 blocks.
    for (uint32_t i = 0; i < (1u << 20); i += 4) {
        uint32_t blk = i / 4;  // block index from 0 to NBLOCKS-1
        mess[4 * blk + 0] = i;
        mess[4 * blk + 1] = 0;
        mess[4 * blk + 2] = 0;
        mess[4 * blk + 3] = 0;
    }

    // Compute and print the original hash (with padding)
    uint64_t target_hash = hs48(mess, FOURLEN, 1, 0);
    printf("\n[attack] Target hash H(mess) = %012" PRIX64 "\n",
           target_hash & 0xFFFFFFFFFFFFULL);

    // --- 2) Compute an expandable message (m1, m2) and its fixed-point fp ---

    uint32_t m1[4], m2[4];
    find_exp_mess(m1, m2);

    // fp = cs48_dm(m1, IV) = get_cs48_dm_fp(m2)
    uint64_t fp = cs48_dm(m1, IV) & 0xFFFFFFFFFFFFULL;

    printf("[attack] Expandable message found.\n");
    printf("        m1 = %06X %06X %06X %06X\n",
           m1[0], m1[1], m1[2], m1[3]);
    printf("        m2 = %06X %06X %06X %06X\n",
           m2[0], m2[1], m2[2], m2[3]);
    printf("        fp = %012" PRIX64 "\n", fp);

    // --- 3) Precompute all chaining values cv[i] for the target message ---

    uint64_t *cv = malloc((NBLOCKS + 1) * sizeof(uint64_t));
    if (!cv) {
        fprintf(stderr, "malloc failed for cv in attack()\n");
        exit(1);
    }

    cv[0] = IV;
    for (uint32_t i = 0; i < NBLOCKS; i++) {
        cv[i + 1] = cs48_dm(&mess[4 * i], cv[i]) & 0xFFFFFFFFFFFFULL;
    }

    // Optional check: last chaining value before padding
    printf("[attack] Last chaining value before padding (target) = %012" PRIX64 "\n",
           cv[NBLOCKS] & 0xFFFFFFFFFFFFULL);

    // --- 4) Build a hash table of all chaining values cv[0..NBLOCKS] ---

    build_cv_table(cv, NBLOCKS + 1);
    printf("[attack] Hash table of chaining values built.\n");

    // --- 5) Search for a bridge block b s.t. cs48_dm(b, fp) = cv[j] for some j >= 2 ---

    uint32_t b[4];
    uint64_t h_candidate;
    uint32_t j = 0;

    printf("[attack] Searching for bridge block...\n");

    while (1) {
        // random candidate block
        random_block_attack(b);

        // chaining value after one block from fp
        h_candidate = cs48_dm(b, fp) & 0xFFFFFFFFFFFFULL;

        // check if it hits some intermediate chaining value cv[j]
        if (cv_table_find(h_candidate, &j)) {
            if (j >= 2) {    // we need j >= 2 so that k = j - 2 >= 0
                break;
            }
            // if j < 2, discard and continue
        }
    }

    uint32_t k = j - 2;  // number of repetitions of m2
    printf("[attack] Bridge found: cs48_dm(b, fp) = cv[%u]\n", j);
    printf("        k (repetitions of m2) = %u\n", k);
    printf("        b = %06X %06X %06X %06X\n",
           b[0], b[1], b[2], b[3]);

    // Sanity: resulting total length:
    // prefix length = 1 (m1) + k (m2^k) + 1 (b) = j
    // suffix length = NBLOCKS - j
    // total = NBLOCKS, so padding is identical.

    // --- 6) Build the actual second preimage mess2 ---

    uint32_t pos = 0;

    // 6.1) Block m1
    for (int w = 0; w < 4; w++) {
        mess2[4 * pos + w] = m1[w];
    }
    pos++;

    // 6.2) k copies of m2
    for (uint32_t t = 0; t < k; t++) {
        for (int w = 0; w < 4; w++) {
            mess2[4 * pos + w] = m2[w];
        }
        pos++;
    }

    // 6.3) Bridge block b
    for (int w = 0; w < 4; w++) {
        mess2[4 * pos + w] = b[w];
    }
    pos++;

    // 6.4) Suffix of original message: blocks j .. NBLOCKS-1
    for (uint32_t i = j; i < NBLOCKS; i++) {
        for (int w = 0; w < 4; w++) {
            mess2[4 * pos + w] = mess[4 * i + w];
        }
        pos++;
    }

    // pos should now be exactly NBLOCKS
    if (pos != NBLOCKS) {
        fprintf(stderr, "[attack] Internal error: pos = %u, expected %u\n", pos, NBLOCKS);
    }

    // --- 7) Verify the attack: check that hs48(mess2) == hs48(mess) ---

    uint64_t h_new = hs48(mess2, FOURLEN, 1, 0);
    printf("[attack] H(mess2) = %012" PRIX64 "\n", h_new & 0xFFFFFFFFFFFFULL);

    if ((h_new & 0xFFFFFFFFFFFFULL) == (target_hash & 0xFFFFFFFFFFFFULL)) {
        printf("[attack] SUCCESS: second preimage found.\n");
    } else {
        printf("[attack] FAILURE: hashes do not match.\n");
    }

    // Optionally, you can print where the branch occurs:
    printf("[attack] Branch point index j = %u (0-based block index in original message)\n", j);

    // --- 8) Cleanup ---

    free(mess);
    free(mess2);
    free(cv);
    free(cv_table);
    cv_table = NULL;
}

int main()
{
	// Test Speck48/96 implementation 
    printf("Testing Speck48/96 implementation...\n");
    if (test_vector_okay()) {
        printf("Test vector passed!\n");
    } else {
        printf("Test vector failed!\n");
        return 1;
    }

	// Test inverse Speck48/96
    printf("\nTesting Speck48/96 inverse...\n");
    if (!test_sp48_inv()) {
        printf("Inverse test failed!\n");
        return 1;
    }

	//Test cs48_dm
	printf("\nTesting cs48_dm...\n");
	test_cs48_dm();

	// Test cs48_dm_fp
	printf("\nTesting cs48_dm_fp...\n");
	test_cs48_dm_fp();

	// Test expandable message
	printf("\nTesting expandable message...\n");
	test_em();

	//Executing the attack
	attack();

	return 0;
}
