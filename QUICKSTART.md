# Quick Start Guide

Get started with the AES Cryptanalysis Toolkit in 5 minutes.

## Installation

### Step 1: Clone the Repository

```bash
git clone https://github.com/morchidy/AES-Cryptanalysis-Toolkit.git
cd AES-Cryptanalysis-Toolkit
```

### Step 2: Verify Prerequisites

You need:
- GCC compiler (or any C99-compatible compiler)
- Make build system
- Linux, macOS, or WSL

Check if you have them:
```bash
gcc --version
make --version
```

### Step 3: Build the Project

```bash
make all
```

Or use the build script:
```bash
./scripts/build.sh
```

That's it! The executable will be created at `bin/aes_attack`.

## Basic Usage

### Run Attack with Random Key

```bash
./bin/aes_attack -r
```

Expected output:
```
Using random target key: 4795ED3B61D0C1CE98F81B4119158ED5

=== Square Attack on 3.5-Round AES-128 ===

Recovering last round key bytes:
Byte  0: 0xE2 (candidates=1)
Byte  1: 0xEC (candidates=1)
...
Attack result: SUCCESS
Time elapsed: 0.022 seconds
```

### Run with Specific Key

```bash
./bin/aes_attack -k 0123456789ABCDEF0123456789ABCDEF
```

### Verbose Mode (See All Details)

```bash
./bin/aes_attack -r -v
```

### Reproducible Results (Use Seed)

```bash
./bin/aes_attack -r -s 12345
```

### Adjust Filtering (More Accurate, Slower)

```bash
./bin/aes_attack -r -f 3
```

## Understanding the Output

```
Using random target key: 4795ED3B61D0C1CE98F81B4119158ED5
                         ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
                         This is the secret key we're trying to recover

Byte  0: 0xE2 (candidates=1)
         ^^^^   ^^^^^^^^^^^^
         Recovered value    Number of candidates (1 = unique solution)

Recovered last round key: E2ECE6E25F081EF4AF6CC65AED3984F9
                          Key after 4 rounds of key schedule

Recovered master key:     4795ED3B61D0C1CE98F81B4119158ED5
                          ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
                          MATCHES the target key = SUCCESS!

Time elapsed: 0.022 seconds
              How long the attack took
```

## Common Tasks

### Run the Example Program

```bash
# First, compile the example
gcc examples/example_basic_attack.c \
    build/core/*.o \
    build/attacks/*.o \
    -Isrc -o bin/example

# Run it
./bin/example
```

### Run Tests

```bash
# Compile and run field operations tests
gcc tests/test_field_ops.c \
    build/core/field_ops.o \
    -Isrc -o bin/test_field_ops

./bin/test_field_ops
```

Expected output:
```
=== GF(2^8) Field Operations Test Suite ===

Testing xtime() function...
  xtime(0x57) = 0xAE [PASS]
  xtime(0x83) = 0x1D [PASS]
  ...
=== ALL TESTS PASSED ===
```

### Clean Build Artifacts

```bash
make clean
```

### Debug Build (With Symbols)

```bash
make debug
```

### Memory Check Build

```bash
make sanitize
./bin/aes_attack -r
```

## Troubleshooting

### "gcc: command not found"

Install GCC:
```bash
# Ubuntu/Debian
sudo apt-get install build-essential

# macOS
xcode-select --install

# Fedora/RHEL
sudo dnf install gcc make
```

### "make: command not found"

Install Make:
```bash
# Ubuntu/Debian
sudo apt-get install make

# macOS
xcode-select --install
```

### Build Fails

Try cleaning first:
```bash
make clean
make all
```

### Attack Returns "FAILED"

This is very rare (< 0.1% with default settings). Try:
```bash
./bin/aes_attack -r -f 3  # Use more filtering
```

## Next Steps

### Learn the Theory

Read the documentation:
```bash
# Mathematical foundations
cat docs/THEORY.md

# Step-by-step attack explanation
cat docs/ATTACK_EXPLANATION.md
```

### Experiment with Parameters

Try different configurations:
```bash
# Less filtering (faster, less accurate)
./bin/aes_attack -r -f 1

# More filtering (slower, more accurate)
./bin/aes_attack -r -f 5

# Quiet mode (just results)
./bin/aes_attack -r -q
```

### Modify the Code

1. Edit source files in `src/`
2. Rebuild: `make`
3. Test your changes

### Read the Full Documentation

- `README.md` - Project overview
- `docs/THEORY.md` - Mathematical background
- `docs/ATTACK_EXPLANATION.md` - Detailed walkthrough
- `CONTRIBUTING.md` - How to contribute
- `PROJECT_SUMMARY.md` - Complete project summary

## Command Reference

```bash
# Build commands
make all          # Build release version
make debug        # Build with debug symbols
make sanitize     # Build with memory checking
make clean        # Remove build artifacts
make run          # Build and run with random key
make run-verbose  # Build and run with verbose output

# Attack options
-k KEY    # Use specific 128-bit key (32 hex digits)
-r        # Use random key
-s SEED   # Set random seed for reproducibility
-f N      # Use N additional lambda sets (default: 2)
-v        # Verbose output (show all details)
-q        # Quiet output (minimal)
-h        # Show help message
```

## Example Workflows

### Quick Test
```bash
make run
```

### Detailed Analysis
```bash
make clean
make all
./bin/aes_attack -r -v -f 3 -s 12345
```

### Benchmarking
```bash
for i in {1..10}; do
    ./bin/aes_attack -r -q -s $i
done
```

### Development Cycle
```bash
# Edit code
vim src/core/aes128_enc.c

# Build with debug info
make debug

# Run with memory checking
make sanitize
./bin/aes_attack -r
```

## Getting Help

- Read the documentation in `docs/`
- Check `README.md` for detailed information
- Review examples in `examples/`
- Open an issue on GitHub
- Read the source code (it's well-commented!)

## Success!

If you see:
```
Attack result: SUCCESS
```

Congratulations! You've successfully:
1. Built the project
2. Executed a cryptanalytic attack
3. Recovered a secret AES key

You're now ready to explore the codebase and learn about AES internals!
