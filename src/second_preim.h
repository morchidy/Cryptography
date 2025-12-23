#ifndef SECOND_PREIM_H
#define SECOND_PREIM_H

#include <stdint.h>
#include <stdbool.h>

// Constants
#define IV 0x010203040506ULL

// Speck48/96 cipher functions
void speck48_96(const uint32_t k[4], const uint32_t p[2], uint32_t c[2]);
void speck48_96_inv(const uint32_t k[4], const uint32_t c[2], uint32_t p[2]);

// Davies-Meyer compression function
uint64_t cs48_dm(const uint32_t m[4], const uint64_t h);

// Hash function
uint64_t hs48(const uint32_t *m, uint64_t fourlen, int padding, int verbose);

// Fixed point computation
uint64_t get_cs48_dm_fp(uint32_t m[4]);

// Expandable message generation
void find_exp_mess(uint32_t m1[4], uint32_t m2[4]);

// Helper functions for attack implementation
extern void build_cv_table(const uint64_t *cv, uint32_t n_entries);
extern int cv_table_find(uint64_t h, uint32_t *idx_out);
extern void random_block_attack(uint32_t m[4]);

// External variables from helpers.c
typedef struct {
    uint64_t h;     // 48-bit chaining value (stored in low bits)
    uint32_t idx;   // index of the chaining value (number of blocks processed)
    uint8_t  used;  // 0 = empty, 1 = used
} cv_entry_t;
extern cv_entry_t *cv_table;

// Test functions
bool test_vector_okay(void);
int test_sp48_inv(void);
int test_cs48_dm(void);
int test_cs48_dm_fp(void);
int test_em(void);

// Attack function
void attack(void);

#endif // SECOND_PREIM_H
