/**
 * @file square_attack.h
 * @brief Square (Integral) attack on reduced-round AES
 * 
 * Implements the Square attack (also known as Integral or Saturation attack)
 * on 3.5-round AES-128. The attack exploits structural properties of the
 * AES substitution-permutation network to recover the secret key.
 */

#ifndef SQUARE_ATTACK_H
#define SQUARE_ATTACK_H

#include <stdint.h>

/**
 * @brief Configuration for the Square attack
 */
typedef struct {
    unsigned int num_additional_sets;  /* Number of extra lambda sets for filtering */
    unsigned int verbose;              /* Verbosity level (0=quiet, 1=normal, 2=debug) */
    unsigned int num_rounds;           /* Target number of rounds (typically 4 for 3.5-round attack) */
} square_attack_config_t;

/**
 * @brief Results from the Square attack
 */
typedef struct {
    uint8_t recovered_last_round_key[16];  /* Recovered last round key */
    uint8_t recovered_master_key[16];      /* Recovered master key */
    unsigned int candidates_per_byte[16];  /* Number of candidates for each byte */
    int success;                           /* 1 if attack succeeded, 0 otherwise */
    double time_elapsed;                   /* Time taken in seconds */
} square_attack_result_t;

/**
 * @brief Execute the Square attack on 3.5-round AES
 * 
 * This function implements the complete key-recovery attack:
 *   1. For each byte position (0-15):
 *      - Build lambda sets with that byte varying
 *      - Query oracle to get ciphertexts
 *      - Test all 256 key guesses
 *      - Filter candidates with additional lambda sets
 *   2. Invert key schedule to recover master key
 * 
 * @param oracle_key The secret key (only for testing - simulates oracle)
 * @param config Attack configuration
 * @param result Output structure for attack results
 * 
 * @return 0 on success, negative error code on failure
 */
int square_attack_execute(const uint8_t oracle_key[16],
                         const square_attack_config_t *config,
                         square_attack_result_t *result);

/**
 * @brief Verify attack results against known key
 * 
 * @param recovered_key Recovered master key
 * @param true_key True master key
 * @return 1 if keys match, 0 otherwise
 */
int square_attack_verify(const uint8_t recovered_key[16], 
                        const uint8_t true_key[16]);

#endif /* SQUARE_ATTACK_H */
