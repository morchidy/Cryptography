/**
 * @file key_schedule.h
 * @brief AES-128 key schedule operations
 * 
 * Provides functions for expanding the master key into round keys
 * and for inverting the key schedule (useful for cryptanalysis).
 */

#ifndef KEY_SCHEDULE_H
#define KEY_SCHEDULE_H

#include <stdint.h>

/**
 * @brief Generate next round key from previous round key
 * 
 * Implements the AES-128 key expansion algorithm for one step.
 * Uses S-box substitution, rotation, and XOR with round constant.
 * 
 * @param prev_key Previous round key (16 bytes)
 * @param next_key Output buffer for next round key (16 bytes)
 * @param round Round number (0-9 for AES-128)
 * 
 * @note Round 0: master key -> first round key
 */
void next_aes128_round_key(const uint8_t prev_key[16], 
                           uint8_t next_key[16], 
                           int round);

/**
 * @brief Recover previous round key from next round key
 * 
 * Inverts the AES-128 key expansion algorithm.
 * Useful for key-recovery attacks on reduced-round AES.
 * 
 * @param next_key Current round key (16 bytes)
 * @param prev_key Output buffer for previous round key (16 bytes)
 * @param round Round number that generated next_key (0-9)
 * 
 * @note Example: If next_key is round 4 key, use round=3 to get round 3 key
 */
void prev_aes128_round_key(const uint8_t next_key[16], 
                           uint8_t prev_key[16], 
                           int round);

/**
 * @brief Expand master key into all round keys
 * 
 * @param master_key Input master key (16 bytes)
 * @param round_keys Output buffer for 11 round keys (176 bytes)
 *                   round_keys[0..15] = master key
 *                   round_keys[16..31] = round 1 key, etc.
 */
void expand_key(const uint8_t master_key[16], uint8_t round_keys[176]);

#endif /* KEY_SCHEDULE_H */
