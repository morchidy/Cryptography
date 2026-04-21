/**
 * @file attack_utils.c
 * @brief Implementation of attack utility functions
 */

#include "attack_utils.h"
#include "../core/aes128_enc.h"
#include "../core/key_schedule.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Build a lambda set
 * 
 * Creates 256 plaintexts where one byte varies through all values 0-255
 * and all other bytes remain constant.
 */
void build_lambda_set(const uint8_t base[16], 
                     int var_index, 
                     uint8_t out_plaintexts[256][16])
{
    for (int v = 0; v < 256; v++) {
        memcpy(out_plaintexts[v], base, 16);
        out_plaintexts[v][var_index] = (uint8_t)v;
    }
}

/**
 * @brief Query the 3.5-round AES oracle
 * 
 * Simulates querying an oracle by encrypting with our implementation.
 * Uses nrounds=4, lastfull=0 to get 3.5 rounds (last round without MixColumns).
 */
void query_oracle_3p5(const uint8_t key[16],
                     const uint8_t plaintexts[256][16],
                     uint8_t ciphertexts[256][16])
{
    uint8_t temp[16];
    
    for (int i = 0; i < 256; i++) {
        memcpy(temp, plaintexts[i], 16);
        aes128_enc(temp, key, 4, 0);  /* 4 rounds, last without MixColumns */
        memcpy(ciphertexts[i], temp, 16);
    }
}

/**
 * @brief Partially decrypt one byte
 * 
 * The last half-round applies SubBytes, ShiftRows, then AddRoundKey.
 * To invert:
 *   1. Undo AddRoundKey: XOR with key guess
 *   2. Apply inverse S-box
 * 
 * Note: ShiftRows just permutes byte positions, which we handle
 * by using the correct position in the caller.
 */
uint8_t partial_decrypt_byte(uint8_t c_byte, uint8_t key_guess)
{
    uint8_t after_key = c_byte ^ key_guess;  /* Undo AddRoundKey */
    return Sinv[after_key];                  /* Apply inverse S-box */
}

/**
 * @brief Recover master key from last round key
 * 
 * For num_rounds=4, we have:
 *   - Round 0: Initial AddRoundKey with master key
 *   - Rounds 1-3: Full rounds
 *   - Round 4: Final half-round
 * 
 * After round 4, we have the round key at index 4.
 * To get back to master key (index 0), we invert the key schedule
 * from round 3 down to round 0.
 */
void recover_master_from_last_round(const uint8_t last_round_key[16],
                                   uint8_t master_key[16],
                                   unsigned int num_rounds)
{
    uint8_t current_key[16];
    uint8_t previous_key[16];
    
    memcpy(current_key, last_round_key, 16);
    
    /* Invert key schedule from last round back to master key */
    for (int r = num_rounds - 1; r >= 0; r--) {
        prev_aes128_round_key(current_key, previous_key, r);
        memcpy(current_key, previous_key, 16);
    }
    
    memcpy(master_key, current_key, 16);
}

/**
 * @brief Generate a random block
 * 
 * Uses stdlib rand() - ensure srand() is called before first use.
 */
void generate_random_block(uint8_t block[16])
{
    for (int i = 0; i < 16; i++) {
        block[i] = (uint8_t)(rand() & 0xFF);
    }
}

/**
 * @brief Print block in hexadecimal
 */
void print_hex_block(const uint8_t block[16])
{
    for (int i = 0; i < 16; i++) {
        printf("%02X", block[i]);
    }
    printf("\n");
}

/**
 * @brief XOR two blocks
 */
void xor_blocks(uint8_t result[16], const uint8_t a[16], const uint8_t b[16])
{
    for (int i = 0; i < 16; i++) {
        result[i] = a[i] ^ b[i];
    }
}
