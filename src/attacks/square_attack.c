/**
 * @file square_attack.c
 * @brief Implementation of the Square attack on 3.5-round AES
 */

#include "square_attack.h"
#include "attack_utils.h"
#include "../core/aes128_enc.h"
#include "../core/key_schedule.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

/**
 * @brief Recover one byte of the last round key
 * 
 * For a given byte position, this function:
 *   1. Generates a lambda set with the corresponding byte varying
 *   2. Queries the 3.5-round oracle on all 256 plaintexts
 *   3. For each key guess g (0-255):
 *      - Partially decrypts the ciphertext byte
 *      - Computes XOR of all 256 decrypted values
 *      - If XOR is 0, g is a candidate
 *   4. Filters candidates using additional lambda sets
 * 
 * @param oracle_key Secret key (for oracle simulation)
 * @param pos Byte position to recover (0-15)
 * @param var_index Which byte of plaintext varies in lambda set
 * @param additional_sets Number of additional lambda sets for filtering
 * @param verbose Verbosity level
 * @return Recovered key byte (best candidate)
 */
static uint8_t recover_key_byte(const uint8_t oracle_key[16],
                               int pos,
                               int var_index,
                               unsigned int additional_sets,
                               unsigned int verbose,
                               unsigned int *num_candidates)
{
    uint8_t base[16];
    uint8_t plaintexts[256][16];
    uint8_t ciphertexts[256][16];
    uint8_t candidates[256];
    int ncand = 0;

    /* Generate random base plaintext */
    generate_random_block(base);

    /* Build lambda set */
    build_lambda_set(base, var_index, plaintexts);

    /* Query 3.5-round oracle */
    query_oracle_3p5(oracle_key, plaintexts, ciphertexts);

    /* Test all 256 key guesses */
    for (int g = 0; g < 256; g++) {
        uint8_t xor_sum = 0;
        
        /* Partially decrypt and XOR all results */
        for (int i = 0; i < 256; i++) {
            uint8_t decrypted = partial_decrypt_byte(ciphertexts[i][pos], (uint8_t)g);
            xor_sum ^= decrypted;
        }
        
        /* If XOR is zero, this is a candidate */
        if (xor_sum == 0) {
            candidates[ncand++] = (uint8_t)g;
        }
    }

    /* Filter false positives with additional lambda sets */
    if (ncand > 1 && additional_sets > 0) {
        uint8_t filtered[256];
        int nfiltered = 0;

        for (int c = 0; c < ncand; c++) {
            uint8_t candidate = candidates[c];
            int valid = 1;

            /* Test candidate with additional lambda sets */
            for (unsigned int s = 0; s < additional_sets && valid; s++) {
                generate_random_block(base);
                build_lambda_set(base, var_index, plaintexts);
                query_oracle_3p5(oracle_key, plaintexts, ciphertexts);

                uint8_t xor_sum = 0;
                for (int i = 0; i < 256; i++) {
                    uint8_t decrypted = partial_decrypt_byte(ciphertexts[i][pos], candidate);
                    xor_sum ^= decrypted;
                }

                if (xor_sum != 0) {
                    valid = 0;  /* Candidate failed, not the correct key */
                }
            }

            if (valid) {
                filtered[nfiltered++] = candidate;
            }
        }

        ncand = nfiltered;
        if (ncand > 0) {
            memcpy(candidates, filtered, ncand);
        }
    }

    *num_candidates = ncand;

    if (verbose >= 1) {
        printf("Byte %2d: 0x%02X (candidates=%d)\n", pos, 
               ncand > 0 ? candidates[0] : 0x00, ncand);
    }

    /* Return best candidate (or 0 if none found) */
    return (ncand > 0) ? candidates[0] : 0x00;
}

/**
 * @brief Execute the complete Square attack
 */
int square_attack_execute(const uint8_t oracle_key[16],
                         const square_attack_config_t *config,
                         square_attack_result_t *result)
{
    clock_t start, end;
    
    /* Initialize result structure */
    memset(result, 0, sizeof(square_attack_result_t));
    
    start = clock();

    if (config->verbose >= 1) {
        printf("\n=== Square Attack on 3.5-Round AES-128 ===\n\n");
        printf("Configuration:\n");
        printf("  Additional lambda sets: %u\n", config->num_additional_sets);
        printf("  Target rounds: %u\n\n", config->num_rounds);
    }

    /* Recover each byte of the last round key */
    if (config->verbose >= 1) {
        printf("Recovering last round key bytes:\n");
    }

    for (int pos = 0; pos < 16; pos++) {
        /* Use byte position itself as the varying index */
        int var_index = pos;
        
        unsigned int num_cand = 0;
        result->recovered_last_round_key[pos] = 
            recover_key_byte(oracle_key, pos, var_index, 
                           config->num_additional_sets, 
                           config->verbose, &num_cand);
        
        result->candidates_per_byte[pos] = num_cand;
    }

    if (config->verbose >= 1) {
        printf("\nRecovered last round key: ");
        print_hex_block(result->recovered_last_round_key);
    }

    /* Invert key schedule to recover master key */
    recover_master_from_last_round(result->recovered_last_round_key,
                                   result->recovered_master_key,
                                   config->num_rounds);

    if (config->verbose >= 1) {
        printf("Recovered master key:     ");
        print_hex_block(result->recovered_master_key);
    }

    end = clock();
    result->time_elapsed = ((double)(end - start)) / CLOCKS_PER_SEC;

    /* Verify result */
    result->success = square_attack_verify(result->recovered_master_key, oracle_key);

    if (config->verbose >= 1) {
        printf("\nAttack result: %s\n", result->success ? "SUCCESS" : "FAILED");
        printf("Time elapsed: %.3f seconds\n\n", result->time_elapsed);
    }

    return result->success ? 0 : -1;
}

/**
 * @brief Verify recovered key against true key
 */
int square_attack_verify(const uint8_t recovered_key[16], 
                        const uint8_t true_key[16])
{
    /* Test encryption with a known plaintext */
    uint8_t test_plain[16] = {0};
    uint8_t cipher1[16], cipher2[16];

    memcpy(cipher1, test_plain, 16);
    memcpy(cipher2, test_plain, 16);

    /* Encrypt with both keys */
    aes128_enc(cipher1, true_key, 4, 0);
    aes128_enc(cipher2, recovered_key, 4, 0);

    /* Compare results */
    for (int i = 0; i < 16; i++) {
        if (cipher1[i] != cipher2[i]) {
            return 0;  /* Keys don't match */
        }
    }

    return 1;  /* Keys match */
}
