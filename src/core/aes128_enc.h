/**
 * @file aes128_enc.h
 * @brief AES-128 encryption primitives and lookup tables
 * 
 * This header provides the core AES-128 encryption functionality with
 * configurable number of rounds. Supports reduced-round variants for
 * cryptanalysis research and education.
 */

#ifndef AES128_ENC_H
#define AES128_ENC_H

#include <stdint.h>

#define AES_BLOCK_SIZE 16
#define AES_128_KEY_SIZE 16
#define AES_NUM_ROUNDS 10

/**
 * @brief Encrypt a block with AES-128
 * 
 * @param block Input/output block (16 bytes, modified in place)
 * @param key Encryption key (16 bytes)
 * @param nrounds Number of rounds to perform (1-10)
 * @param lastfull If 1, last round includes MixColumns; if 0, it doesn't
 * 
 * @note For standard AES-128: nrounds=10, lastfull=0
 * @note For 3.5-round AES: nrounds=4, lastfull=0
 */
void aes128_enc(uint8_t block[AES_BLOCK_SIZE], 
                const uint8_t key[AES_128_KEY_SIZE], 
                unsigned nrounds, 
                int lastfull);

/**
 * @brief Perform one AES round
 * 
 * @param block State block (16 bytes)
 * @param round_key Round key (16 bytes)
 * @param lastround 0 for normal round, 16 to skip MixColumns
 */
void aes_round(uint8_t block[AES_BLOCK_SIZE], 
               uint8_t round_key[AES_BLOCK_SIZE], 
               int lastround);

/**
 * @brief AES forward S-box lookup table
 */
extern const uint8_t S[256];

/**
 * @brief AES inverse S-box lookup table
 */
extern const uint8_t Sinv[256];

#endif /* AES128_ENC_H */
