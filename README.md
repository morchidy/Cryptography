# Second Preimage Attack on Merkle-Damgård Hash Functions

![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)
![C](https://img.shields.io/badge/Language-C-blue.svg)
![Build](https://img.shields.io/badge/Build-Make-green.svg)
![Platform](https://img.shields.io/badge/Platform-Linux-lightgrey.svg)
![Security](https://img.shields.io/badge/Purpose-Educational-orange.svg)

## Table of Contents

- [Overview](#overview)
- [Attack Methodology](#attack-methodology)
  - [Theoretical Background](#theoretical-background)
  - [Attack Strategy](#attack-strategy)
  - [Complexity Analysis](#complexity-analysis)
- [Project Architecture](#project-architecture)
  - [Source Files](#source-files)
- [Compilation Instructions](#compilation-instructions)
  - [Requirements](#requirements)
  - [Build Commands](#build-commands)
  - [Manual Compilation](#manual-compilation)
- [Execution Instructions](#execution-instructions)
  - [Running the Full Attack](#running-the-full-attack)
  - [Expected Output](#expected-output)
  - [Performance Considerations](#performance-considerations)
- [References](#references)
- [Security Disclaimer](#security-disclaimer)
- [Authors](#authors)
- [License](#license)

## Overview

This project implements a second preimage attack on a Merkle-Damgård hash function based on the Speck48/96 block cipher using the Davies-Meyer compression function. The attack exploits the structure of the hash function to find a different message that produces the same hash value as a given target message.

## Attack Methodology

### Theoretical Background

The second preimage attack implemented here is based on Kelsey and Schneier's "long message attack" against iterated hash functions. The attack works against Merkle-Damgård constructions when the target message is long (2^k blocks) and exploits two main weaknesses:

1. **The sequential nature of Merkle-Damgård**: Each block is processed sequentially, creating intermediate chaining values
2. **Fixed-point behavior**: Given a message block m, we can find a chaining value h such that the compression function maps (m, h) back to h

### Attack Strategy

The attack consists of four main phases:

#### Phase 1: Expandable Message Construction
- Generate an **expandable message** (m₁, m₂) using a meet-in-the-middle approach
- m₁ is a single-block message
- m₂ is a message block that forms a fixed-point when repeated
- Property: H(m₁) = H(m₁ || m₂ || m₂ || ... || m₂) for any number of repetitions
- This gives us flexible message length without changing the hash output

**Complexity**: O(2^(n/2)) where n is the hash output size (48 bits here)

#### Phase 2: Precompute Chaining Values
- Hash the entire target message M (2^18 blocks in our implementation)
- Store all intermediate chaining values CV[0], CV[1], ..., CV[2^18]
- Build a hash table for efficient lookup of these values

**Complexity**: O(2^k) time and space, where k=18 (message length)

#### Phase 3: Bridge Block Search
- Starting from the fixed-point of the expandable message
- Try random blocks b until cs48_dm(b, fp) = CV[j] for some j ≥ 2
- This "bridge block" connects our expandable prefix to the target message

**Complexity**: O(2^(n-k)) ≈ O(2^30) for n=48, k=18

#### Phase 4: Message Assembly
- Construct second preimage as: m₁ || m₂^(j-2) || b || M[j+1..end]
- Where M[j+1..end] is the suffix of the original message starting from block j+1
- This gives us the same total length as the original message
- The padding will be identical, resulting in the same final hash

### Complexity Analysis

- **Time Complexity**: O(2^(n/2) + 2^(n-k))
  - For n=48 bits and k=18: O(2^24 + 2^30) ≈ O(2^30) operations
  - Much faster than brute force O(2^48)
  
- **Space Complexity**: O(2^k) for storing chaining values
  - ~2 MB for k=18

- **Success Condition**: Works when target message length 2^k > 2^(n/2+2)
  - For 48-bit hash: requires k > 26, we use k=18 as a demonstration

## Project Architecture

### Source Files

#### second_preim_48.c
Main implementation file containing:
- Speck48/96 block cipher encryption and decryption functions
- Davies-Meyer compression function (cs48_dm)
- Merkle-Damgård hash function with padding (hs48)
- Fixed-point computation for the compression function
- Full second preimage attack implementation
- Main entry point with test execution

#### helpers.c
Helper functions for the attack:
- Expandable message generation using meet-in-the-middle approach
- Hash table implementation for chaining value lookups
- Random block generation utilities
- Forward/backward phase implementations for finding collisions

#### tests.c
Comprehensive test suite:
- Speck48/96 cipher test against official test vectors
- Speck48/96 inverse function validation
- Davies-Meyer compression function correctness test
- Fixed-point computation verification
- Expandable message generation test

#### second_preim.h
Header file containing:
- Function prototypes for all cryptographic primitives
- Type definitions for hash table entries
- Constants (IV value)
- External declarations for helper functions

#### xoshiro.h
High-quality pseudorandom number generator implementation:
- xoshiro256plus and xoshiro256starstar PRNGs
- Initialization and state management
- Used for generating random message blocks during the attack

#### Makefile
Build configuration:
- Compilation flags: -O3 -march=native -std=c11 for optimized execution
- Dependency management for all source files
- Clean and run targets

## Compilation Instructions

### Requirements
- GCC compiler (version 7.0 or later recommended)
- Linux/Unix environment
- Standard C libraries and math library

### Build Commands

To compile the project:
```bash
make
```

This will generate the executable `second_preim` with full optimizations enabled.

To clean build artifacts:
```bash
make clean
```

To compile and run immediately:
```bash
make run
```

### Manual Compilation

If you prefer to compile manually without the Makefile:
```bash
gcc -O3 -march=native -std=c11 -c second_preim_48.c
gcc -O3 -march=native -std=c11 -c tests.c
gcc -O3 -march=native -std=c11 -c helpers.c
gcc second_preim_48.o tests.o helpers.o -o second_preim -lm
```

## Execution Instructions

### Running the Full Attack

Execute the compiled binary:
```bash
./second_preim
```

The program will:
1. Run validation tests for the cryptographic primitives
2. Execute the full second preimage attack
3. Display progress information and results

### Expected Output

The execution will show:
- Test results for Speck48/96 implementation
- Test results for inverse cipher function
- Davies-Meyer compression function validation
- Fixed-point computation verification
- Attack progress including:
  - Target message hash computation
  - Expandable message generation
  - Chaining value table construction
  - Bridge block search progress
  - Final verification and success/failure status

### Performance Considerations

- The attack complexity depends on the hash output size (48 bits)
- Expandable message generation: expected time less than 2 minutes with N=2^17
- Bridge block search: expected time varies (2-10 minutes depending on hardware)
- Total execution time: typically 5-15 minutes on modern hardware
- Memory usage: approximately 300-400 MB for hash tables and message storage

## References

- Kelsey, J., & Schneier, B. (2005). "Second Preimages on n-Bit Hash Functions for Much Less than 2^n Work." *EUROCRYPT 2005*.
- Beaulieu, R., Shors, D., Smith, J., Treatman-Clark, S., Weeks, B., & Wingers, L. (2013). "The SIMON and SPECK Families of Lightweight Block Ciphers." *IACR ePrint Archive*.

## Security Disclaimer

**IMPORTANT: EDUCATIONAL USE ONLY**

This implementation is provided **strictly for educational and research purposes** to demonstrate cryptographic attack techniques in an academic setting. 

## Authors

**Youssef MORCHID**
morchidy33@gmail.com

Implementation for M2 CySec - Cryptographic Engineering Lab, 2025

Course: Cryptographic Engineering  
Institution: ENSIMAG

---
