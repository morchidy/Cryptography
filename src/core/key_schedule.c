/**
 * @file key_schedule.c
 * @brief AES-128 key schedule implementation
 */

#include "key_schedule.h"
#include "aes128_enc.h"
#include <string.h>

/**
 * @brief Round constants for AES key expansion
 * 
 * These are powers of x in GF(2^8): RC[i] = x^i mod m(x)
 */
static const uint8_t RC[10] = {
    0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1B, 0x36
};

/**
 * @brief Generate next round key from previous round key
 * 
 * The AES key expansion applies:
 *   1. RotWord: Rotate last column left by 1 byte
 *   2. SubWord: Apply S-box to each byte of rotated column
 *   3. XOR with round constant (only first byte)
 *   4. XOR with previous column to generate new columns
 * 
 * Structure for 16-byte key (4 columns):
 *   next[0..3] = prev[0..3] XOR SubWord(RotWord(prev[12..15])) XOR RC[round]
 *   next[4..7] = prev[4..7] XOR next[0..3]
 *   next[8..11] = prev[8..11] XOR next[4..7]
 *   next[12..15] = prev[12..15] XOR next[8..11]
 * 
 * @param prev_key Previous round key
 * @param next_key Output next round key
 * @param round Round number (0-9)
 */
void next_aes128_round_key(const uint8_t prev_key[16], uint8_t next_key[16], int round)
{
    int i;

    /* First column: apply RotWord + SubWord + XOR with round constant */
    next_key[0] = prev_key[0] ^ S[prev_key[13]] ^ RC[round];
    next_key[1] = prev_key[1] ^ S[prev_key[14]];
    next_key[2] = prev_key[2] ^ S[prev_key[15]];
    next_key[3] = prev_key[3] ^ S[prev_key[12]];

    /* Remaining columns: XOR with previous column */
    for (i = 4; i < 16; i++) {
        next_key[i] = prev_key[i] ^ next_key[i - 4];
    }
}

/**
 * @brief Recover previous round key from next round key (inverse key schedule)
 * 
 * This is the inverse of next_aes128_round_key().
 * Since XOR is self-inverse (a XOR b XOR b = a), we can reverse the process:
 * 
 * Key insight:
 *   - next[i] = prev[i] XOR next[i-4] for i >= 4
 *   - Therefore: prev[i] = next[i] XOR next[i-4]
 * 
 * We must recover bytes 4..15 first (simple XOR), then bytes 0..3
 * (which depend on prev[12..15] that we just recovered).
 * 
 * @param next_key Current round key (16 bytes)
 * @param prev_key Output previous round key (16 bytes)
 * @param round Round number that was used to generate next_key
 */
void prev_aes128_round_key(const uint8_t next_key[16], uint8_t prev_key[16], int round)
{
    int i;

    /* Step 1: Recover bytes 4..15 using linear XOR relation */
    for (i = 4; i < 16; i++) {
        prev_key[i] = next_key[i] ^ next_key[i - 4];
    }

    /* Step 2: Recover bytes 0..3 using S-box and round constant
     * 
     * From forward key schedule:
     *   next[0] = prev[0] ^ S[prev[13]] ^ RC[round]
     * Therefore:
     *   prev[0] = next[0] ^ S[prev[13]] ^ RC[round]
     * 
     * Since we already have prev[13] from step 1, we can compute this.
     */
    prev_key[0] = next_key[0] ^ S[prev_key[13]] ^ RC[round];
    prev_key[1] = next_key[1] ^ S[prev_key[14]];
    prev_key[2] = next_key[2] ^ S[prev_key[15]];
    prev_key[3] = next_key[3] ^ S[prev_key[12]];
}

/**
 * @brief Expand master key into all 11 round keys
 * 
 * @param master_key Input master key (16 bytes)
 * @param round_keys Output buffer for 176 bytes (11 keys)
 */
void expand_key(const uint8_t master_key[16], uint8_t round_keys[176])
{
    int round;
    
    /* Round 0 key is the master key itself */
    memcpy(round_keys, master_key, 16);
    
    /* Generate rounds 1-10 */
    for (round = 0; round < 10; round++) {
        next_aes128_round_key(round_keys + (round * 16), 
                             round_keys + ((round + 1) * 16), 
                             round);
    }
}
