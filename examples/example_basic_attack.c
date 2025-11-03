/**
 * @file example_basic_attack.c
 * @brief Simple example of using the Square attack API
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "../src/core/aes128_enc.h"
#include "../src/attacks/square_attack.h"
#include "../src/attacks/attack_utils.h"

int main(void)
{
    printf("=== Basic Square Attack Example ===\n\n");
    
    /* Initialize random seed */
    srand((unsigned int)time(NULL));
    
    /* Generate a random target key */
    uint8_t target_key[16];
    generate_random_block(target_key);
    
    printf("Target key: ");
    print_hex_block(target_key);
    printf("\n");
    
    /* Configure the attack */
    square_attack_config_t config = {
        .num_additional_sets = 2,   /* Use 2 additional lambda sets for filtering */
        .verbose = 1,               /* Normal verbosity */
        .num_rounds = 4             /* 3.5 rounds (4 rounds with last partial) */
    };
    
    /* Execute the attack */
    square_attack_result_t result;
    int status = square_attack_execute(target_key, &config, &result);
    
    /* Display results */
    printf("\n=== Attack Summary ===\n");
    printf("Status: %s\n", status == 0 ? "SUCCESS" : "FAILED");
    printf("Time elapsed: %.3f seconds\n", result.time_elapsed);
    printf("Candidates per byte: ");
    for (int i = 0; i < 16; i++) {
        printf("%d ", result.candidates_per_byte[i]);
    }
    printf("\n\n");
    
    if (result.success) {
        printf("Successfully recovered master key!\n");
        printf("Recovered: ");
        print_hex_block(result.recovered_master_key);
        printf("Original:  ");
        print_hex_block(target_key);
    } else {
        printf("Attack failed. This should be rare.\n");
    }
    
    return status;
}
