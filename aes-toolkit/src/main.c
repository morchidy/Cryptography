/**
 * @file main.c
 * @brief Main program for AES Cryptanalysis Toolkit
 * 
 * Command-line interface for running the Square attack on 3.5-round AES.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "core/aes128_enc.h"
#include "attacks/square_attack.h"
#include "attacks/attack_utils.h"

/**
 * @brief Print usage information
 */
void print_usage(const char *progname)
{
    printf("AES Cryptanalysis Toolkit - Square Attack on 3.5-Round AES\n");
    printf("\nUsage: %s [options]\n\n", progname);
    printf("Options:\n");
    printf("  -k KEY      Use specified key (32 hex digits)\n");
    printf("  -r          Use random key\n");
    printf("  -s SEED     Random seed (default: time-based)\n");
    printf("  -f SETS     Number of additional lambda sets for filtering (default: 2)\n");
    printf("  -v          Verbose output\n");
    printf("  -q          Quiet mode (minimal output)\n");
    printf("  -h          Show this help\n");
    printf("\nExamples:\n");
    printf("  %s -r                                    # Attack with random key\n", progname);
    printf("  %s -k 0123456789ABCDEF0123456789ABCDEF  # Attack specific key\n", progname);
    printf("  %s -r -f 3 -v                           # Verbose with 3 filter sets\n", progname);
    printf("\n");
}

/**
 * @brief Parse hexadecimal string into byte array
 * 
 * @param hex Hexadecimal string (32 characters)
 * @param bytes Output byte array (16 bytes)
 * @return 0 on success, -1 on error
 */
int parse_hex_key(const char *hex, uint8_t bytes[16])
{
    if (strlen(hex) != 32) {
        fprintf(stderr, "Error: Key must be 32 hexadecimal digits\n");
        return -1;
    }

    for (int i = 0; i < 16; i++) {
        char byte_str[3] = {hex[2*i], hex[2*i+1], '\0'};
        char *endptr;
        long val = strtol(byte_str, &endptr, 16);
        
        if (*endptr != '\0' || val < 0 || val > 255) {
            fprintf(stderr, "Error: Invalid hexadecimal digit in key\n");
            return -1;
        }
        
        bytes[i] = (uint8_t)val;
    }

    return 0;
}

/**
 * @brief Main entry point
 */
int main(int argc, char *argv[])
{
    uint8_t target_key[16];
    int use_random_key = 0;
    int key_provided = 0;
    unsigned int seed = 0;
    int use_time_seed = 1;
    
    square_attack_config_t config = {
        .num_additional_sets = 2,
        .verbose = 1,
        .num_rounds = 4
    };

    /* Parse command-line arguments */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        }
        else if (strcmp(argv[i], "-k") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Error: -k requires a key argument\n");
                return 1;
            }
            if (parse_hex_key(argv[++i], target_key) != 0) {
                return 1;
            }
            key_provided = 1;
        }
        else if (strcmp(argv[i], "-r") == 0) {
            use_random_key = 1;
        }
        else if (strcmp(argv[i], "-s") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Error: -s requires a seed argument\n");
                return 1;
            }
            seed = (unsigned int)atoi(argv[++i]);
            use_time_seed = 0;
        }
        else if (strcmp(argv[i], "-f") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Error: -f requires a number argument\n");
                return 1;
            }
            config.num_additional_sets = (unsigned int)atoi(argv[++i]);
        }
        else if (strcmp(argv[i], "-v") == 0) {
            config.verbose = 2;
        }
        else if (strcmp(argv[i], "-q") == 0) {
            config.verbose = 0;
        }
        else {
            fprintf(stderr, "Error: Unknown option '%s'\n", argv[i]);
            print_usage(argv[0]);
            return 1;
        }
    }

    /* Initialize random number generator */
    if (use_time_seed) {
        seed = (unsigned int)time(NULL);
    }
    srand(seed);

    /* Generate or use provided key */
    if (use_random_key || !key_provided) {
        generate_random_block(target_key);
        if (config.verbose >= 1) {
            printf("Using random target key: ");
            print_hex_block(target_key);
            printf("(Random seed: %u)\n\n", seed);
        }
    } else {
        if (config.verbose >= 1) {
            printf("Using provided target key: ");
            print_hex_block(target_key);
            printf("\n");
        }
    }

    /* Execute the attack */
    square_attack_result_t result;
    int status = square_attack_execute(target_key, &config, &result);

    /* Print summary */
    if (config.verbose == 0) {
        /* Quiet mode: just print recovered key and status */
        printf("Recovered: ");
        print_hex_block(result.recovered_master_key);
        printf("Status: %s\n", result.success ? "SUCCESS" : "FAILED");
    }

    return status;
}
