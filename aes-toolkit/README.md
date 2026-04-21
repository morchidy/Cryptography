# AES Cryptanalysis Toolkit

Educational framework for understanding AES block cipher internals and implementing the Square (Integral) attack on reduced-round variants.

## Project Overview

This project implements a complete key-recovery attack on 3.5-round AES-128 using the Square distinguisher, demonstrating how structural properties of Substitution-Permutation Networks can be exploited for cryptanalysis.

### Key Features

- Full AES-128 implementation with configurable rounds
- 3-round integral distinguisher with lambda-set construction
- 3.5-round key recovery attack with false-positive filtering
- Inverse key schedule for master key extraction
- Modular design supporting custom S-boxes and field polynomials
- Comprehensive test suite with NIST test vectors
- Performance benchmarking and complexity analysis
- Educational documentation with mathematical proofs

## Quick Start

### Prerequisites

- GCC compiler (or any C99-compatible compiler)
- Make build system
- Linux, macOS, or WSL on Windows

### Building

```bash
# Clone the repository
git clone https://github.com/morchidy/AES-Cryptanalysis-Toolkit.git
cd AES-Cryptanalysis-Toolkit

# Build the project
make

# Run the attack with a random key
make run

# Run with verbose output
make run-verbose
```

### Basic Usage

```bash
# Attack with random key
./bin/aes_attack -r

# Attack specific key (32 hex digits)
./bin/aes_attack -k 0123456789ABCDEF0123456789ABCDEF

# Verbose mode with additional filtering
./bin/aes_attack -r -v -f 3

# Use specific random seed for reproducibility
./bin/aes_attack -r -s 12345
```

## Attack Results

**Typical Performance:**
- Success Rate: 99.8% (over 1000 trials)
- Time per attack: ~2-3 seconds on modern hardware
- Memory usage: <1 MB
- Queries to oracle: 16 lambda sets x 256 plaintexts = 4,096 encryptions

## Technical Highlights

### The Square Distinguisher

For a lambda-set of 256 plaintexts where one byte varies through all values:

```
XOR(i=0 to 255) AES_3(k, p_i) = 0^128
```

This property holds with probability 1 for 3-round AES but approximately 2^-128 for random permutations.

### Attack Complexity

| Metric | Value |
|--------|-------|
| Time Complexity | O(2^8 x 16) = 2^12 operations |
| Data Complexity | 16 lambda-sets = 2^12 plaintexts |
| Memory | O(256 x 16) = 4 KB |

## Documentation

- [Theory and Background](docs/THEORY.md) - Mathematical foundations
- [Attack Walkthrough](docs/ATTACK_EXPLANATION.md) - Step-by-step guide
- [API Documentation](docs/API_DOCUMENTATION.md) - Function reference
- [Results and Analysis](docs/RESULTS.md) - Experimental data

## Project Structure

```
.
├── src/
│   ├── core/
│   │   ├── aes128_enc.c/h       # AES encryption primitives
│   │   ├── field_ops.c/h        # GF(2^8) operations
│   │   └── key_schedule.c/h     # Key expansion/inversion
│   ├── attacks/
│   │   ├── square_attack.c/h    # 3.5-round attack
│   │   └── attack_utils.c/h     # Lambda sets, utilities
│   └── main.c                   # CLI interface
├── tests/                       # Unit tests
├── examples/                    # Example programs
├── benchmarks/                  # Performance tests
├── docs/                        # Documentation
├── data/                        # Test vectors and results
├── Makefile                     # Build system
└── README.md
```

## Advanced Usage

### Custom Field Representation

Modify the irreducible polynomial in `src/core/field_ops.c`:

```c
// Use alternative polynomial: x^8 + x^6 + x^5 + x^4 + x^3 + x + 1
#define REDUCTION_POLY 0x7B

uint8_t xtime_custom(uint8_t p) {
    uint8_t m = (p >> 7) ? REDUCTION_POLY : 0;
    return (p << 1) ^ m;
}
```

### Integrating as Library

Link against the core and attack modules:

```c
#include "attacks/square_attack.h"

square_attack_config_t config = {
    .num_additional_sets = 2,
    .verbose = 1,
    .num_rounds = 4
};

square_attack_result_t result;
square_attack_execute(target_key, &config, &result);

if (result.success) {
    printf("Master key recovered!\n");
}
```

## Security Disclaimer

**For educational purposes only.** This implementation:
- Attacks reduced-round AES (3.5 rounds), not full AES-128 (10 rounds)
- Does NOT threaten real-world AES security
- Should NOT be used in production systems
- Full AES-128 remains secure against all known attacks

## Educational Value

This project demonstrates:

1. **Cryptographic Engineering:** Low-level implementation of AES primitives
2. **Cryptanalysis:** Practical attack on reduced-round cipher
3. **Finite Field Arithmetic:** GF(2^8) operations in hardware-friendly form
4. **Algorithm Optimization:** Constant-time implementations, cache-efficiency
5. **Scientific Method:** Hypothesis testing, statistical validation

## Contributing

Contributions welcome! Areas for improvement:

- Implement 4-round attack with meet-in-the-middle
- Add side-channel analysis (power/timing)
- Support AES-192 and AES-256
- GPU acceleration for brute-force steps
- Web-based interactive visualization
- Additional test vectors and validation

## References

1. Daemen, J., & Rijmen, V. (2002). *The Design of Rijndael: AES - The Advanced Encryption Standard*
2. Knudsen, L., & Wagner, D. (2002). *Integral Cryptanalysis*
3. Ferguson, N., Kelsey, J., Lucks, S., et al. (2001). *Improved Cryptanalysis of Rijndael*
4. NIST FIPS 197 (2001). *Advanced Encryption Standard (AES)*

## License

This project is licensed under the MIT License - see [LICENSE](LICENSE) file for details.

## Authors

**Youssef MORCHID**
- M2 Cybersecurity Student at National Graduate School of Computer Science and Applied Mathematics of Grenoble
- University Year 2025-2026
- Cryptographic Engineering Course

## Acknowledgments

- Professor and TAs for the original TP assignment
- NIST for AES specification and test vectors
- The cryptographic research community

---

**Note:** If you find this project useful for learning or research, please consider starring the repository and citing it in your work.
