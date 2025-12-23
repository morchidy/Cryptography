#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "xoshiro.h"
#include "second_preim.h"

#define IV 0x010203040506ULL

// ---------------- Expandable message helpers ----------------

typedef struct {
    uint64_t h;    // 48-bit chaining value
    uint32_t m[4]; // 1-block message
} em_entry_t;

static int em_cmp(const void *a, const void *b)
{
    const em_entry_t *ea = (const em_entry_t *)a;
    const em_entry_t *eb = (const em_entry_t *)b;
    if (ea->h < eb->h) return -1;
    if (ea->h > eb->h) return 1;
    return 0;
}

static const em_entry_t *em_find(uint64_t target,
                                 const em_entry_t *tab,
                                 size_t N)
{
    size_t lo = 0, hi = N;
    while (lo < hi) {
        size_t mid = (lo + hi) / 2;
        uint64_t hmid = tab[mid].h;
        if (hmid == target) return &tab[mid];
        if (hmid < target)  lo = mid + 1;
        else                hi = mid;
    }
    return NULL;
}

static inline void em_random_block(uint32_t m[4])
{
    for (int i = 0; i < 4; i++)
        m[i] = (uint32_t)(xoshiro256plus_random() & 0xFFFFFFu);
}

/*
 * find_exp_mess:
 *  - N forward states: h = cs48_dm(m1, IV)
 *  - then random m2, compute fp = get_cs48_dm_fp(m2)
 *    until fp collides with some h.
 *
 * With N = 2^17, this should normally be < 1 minute on a modern CPU.
 */
void find_exp_mess(uint32_t m1[4], uint32_t m2[4])
{
    // You can tune this: try 1<<16, 1<<17, 1<<18
    const size_t N = (size_t)1 << 19;      // 131072
    em_entry_t *table = malloc(N * sizeof(em_entry_t));
    if (!table) {
        fprintf(stderr, "malloc failed in find_exp_mess\n");
        exit(EXIT_FAILURE);
    }

    // -------- Forward phase --------
    for (size_t i = 0; i < N; i++) {
        em_random_block(table[i].m);
        table[i].h = cs48_dm(table[i].m, IV) & 0xFFFFFFFFFFFFULL;
    }

    qsort(table, N, sizeof(em_entry_t), em_cmp);

    // -------- Backward phase --------
    uint64_t tries = 0;
    for (;;) {
        uint32_t cand_m2[4];
        em_random_block(cand_m2);

        uint64_t fp = get_cs48_dm_fp(cand_m2) & 0xFFFFFFFFFFFFULL;
        tries++;

        // OPTIONAL: very light progress indicator every 1M trials
        // if ((tries & ((1u << 20) - 1)) == 0) {
        //     fprintf(stderr, "[find_exp_mess] tries = %llu\r",
        //             (unsigned long long)tries);
        // }

        const em_entry_t *e = em_find(fp, table, N);
        if (e) {
            for (int j = 0; j < 4; j++) {
                m1[j] = e->m[j];
                m2[j] = cand_m2[j];
            }
            fprintf(stderr,
                    "\n[find_exp_mess] success after %llu trials (N=%zu)\n",
                    (unsigned long long)tries, N);
            free(table);
            return;
        }
    }
}

// ===== Helpers for the attack() implementation =====

// Hash table size for chaining values (must be power of 2)
#define CV_TABLE_SIZE (1u << 20)

cv_entry_t *cv_table = NULL;

/* Build a hash table mapping chaining values -> index */
void build_cv_table(const uint64_t *cv, uint32_t n_entries)
{
    cv_table = calloc(CV_TABLE_SIZE, sizeof(cv_entry_t));
    if (!cv_table) {
        fprintf(stderr, "calloc failed in build_cv_table\n");
        exit(1);
    }

    const uint32_t mask = CV_TABLE_SIZE - 1;

    for (uint32_t i = 0; i < n_entries; i++) {
        uint64_t key = cv[i] & 0xFFFFFFFFFFFFULL;
        uint32_t pos = (uint32_t)((key ^ (key >> 24)) & mask);
        while (cv_table[pos].used) {
            pos = (pos + 1) & mask;
        }
        cv_table[pos].used = 1;
        cv_table[pos].h    = key;
        cv_table[pos].idx  = i;
    }
}

/* Lookup chaining value in hash table, return 1 if found and set *idx_out */
int cv_table_find(uint64_t h, uint32_t *idx_out)
{
    if (!cv_table) return 0;

    const uint32_t mask = CV_TABLE_SIZE - 1;
    uint64_t key = h & 0xFFFFFFFFFFFFULL;
    uint32_t pos = (uint32_t)((key ^ (key >> 24)) & mask);

    while (cv_table[pos].used) {
        if (cv_table[pos].h == key) {
            *idx_out = cv_table[pos].idx;
            return 1;
        }
        pos = (pos + 1) & mask;
    }
    return 0;
}

/* Draw a random 96-bit block (4 × 24-bit words) using xoshiro256plus_random */
void random_block_attack(uint32_t m[4])
{
    for (int i = 0; i < 4; i++) {
        m[i] = (uint32_t)(xoshiro256plus_random() & 0xFFFFFF);
    }
}
