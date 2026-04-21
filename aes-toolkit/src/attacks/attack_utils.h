/**
 * @file attack_utils.h
 * @brief Utility functions for cryptanalytic attacks
 */

#ifndef ATTACK_UTILS_H
#define ATTACK_UTILS_H

#include <stdint.h>

/**
 * @brief Build a lambda set for the Square attack
 * 
 * A lambda set consists of 256 plaintexts where:
 *   - One byte (at index var_index) takes all 256 possible values
 *   - All other bytes remain constant (equal to base plaintext)
 * 
 * @param base Base plaintext (16 bytes)
 * @param var_index Index of the byte to vary (0-15)
 * @param out_plaintexts Output buffer for 256 plaintexts (256x16 bytes)
 */
void build_lambda_set(const uint8_t base[16], 
                     int var_index, 
                     uint8_t out_plaintexts[256][16]);

/**
 * @brief Query the 3.5-round AES oracle
 * 
 * Encrypts 256 plaintexts using 3.5-round AES (4 rounds, last without MixColumns).
 * In a real attack scenario, this would query an external oracle.
 * For testing, we simulate the oracle using our AES implementation.
 * 
 * @param key Secret key (16 bytes)
 * @param plaintexts Input plaintexts (256x16 bytes)
 * @param ciphertexts Output ciphertexts (256x16 bytes)
 */
void query_oracle_3p5(const uint8_t key[16],
                     const uint8_t plaintexts[256][16],
                     uint8_t ciphertexts[256][16]);

/**
 * @brief Partially decrypt one byte through the last half-round
 * 
 * The last half-round of 3.5-round AES consists of:
 *   1. SubBytes (S-box)
 *   2. ShiftRows
 *   3. AddRoundKey (XOR with last round key)
 * 
 * To partially decrypt byte at position pos:
 *   1. Undo AddRoundKey: c_byte XOR key_guess
 *   2. Apply inverse S-box: Sinv[result]
 * 
 * This gives us the state after 3 full rounds.
 * 
 * @param c_byte Ciphertext byte
 * @param key_guess Guessed key byte
 * @return State byte after partial decryption
 */
uint8_t partial_decrypt_byte(uint8_t c_byte, uint8_t key_guess);

/**
 * @brief Recover master key from last round key
 * 
 * Applies the inverse key schedule repeatedly to go from the
 * last round key back to the master key.
 * 
 * @param last_round_key Last round key (16 bytes)
 * @param master_key Output master key (16 bytes)
 * @param num_rounds Total number of rounds (4 for 3.5-round attack)
 */
void recover_master_from_last_round(const uint8_t last_round_key[16],
                                   uint8_t master_key[16],
                                   unsigned int num_rounds);

/**
 * @brief Generate a random block (for lambda set base)
 * 
 * @param block Output block (16 bytes)
 */
void generate_random_block(uint8_t block[16]);

/**
 * @brief Print a block in hexadecimal
 * 
 * @param block Block to print (16 bytes)
 */
void print_hex_block(const uint8_t block[16]);

/**
 * @brief XOR two blocks
 * 
 * @param result Output block (result = a XOR b)
 * @param a First block (16 bytes)
 * @param b Second block (16 bytes)
 */
void xor_blocks(uint8_t result[16], const uint8_t a[16], const uint8_t b[16]);

#endif /* ATTACK_UTILS_H */
