# Cryptography — Cryptanalysis & Attack Implementations

![Language](https://img.shields.io/badge/Language-C-blue)
![Domain](https://img.shields.io/badge/Domain-Cryptanalysis-red)
![Level](https://img.shields.io/badge/Level-Advanced-black)
![License](https://img.shields.io/badge/License-Educational-lightgrey)
![Status](https://img.shields.io/badge/Status-Active-success)
![Repo Size](https://img.shields.io/github/repo-size/morchidy/Cryptography)
![Last Commit](https://img.shields.io/github/last-commit/morchidy/Cryptography)

This repository gathers a set of low-level implementations of classical and modern cryptographic attacks, written in C with a focus on algorithmic understanding, performance, and reproducibility.

The objective is to explore how real-world cryptographic constructions behave under attack, and to bridge theory (cryptanalysis, number theory, hash constructions) with practical engineering.

---

## Overview

This project is structured as a monorepo containing independent implementations of:

- Hash function cryptanalysis (Merkle–Damgård constructions)
- Discrete logarithm attacks in finite fields
- Block cipher cryptanalysis (AES)

Each module is self-contained and focuses on a specific attack model or cryptographic primitive.

---

## Projects

### 1. Merkle–Damgård Second Preimage Attack
**Location:** `merkle-damgard/`

Implementation of the Kelsey–Schneier second preimage attack on Merkle–Damgård hash constructions.

- Construction: Davies–Meyer compression
- Block cipher: Speck48/96
- Focus: structural weaknesses of iterative hash designs

**Key concepts:**
- Second preimage resistance
- Expandable messages
- Long message attacks
- Hash function construction flaws

---

### 2. Pollard Kangaroo — Discrete Logarithm
**Location:** `pollard-kangaroo/`

Implementation of Pollard’s Kangaroo (Lambda) algorithm for solving discrete logarithms in bounded intervals.

- Domain: finite fields
- Includes parameter exploration and performance considerations

**Key concepts:**
- Discrete logarithm problem (DLP)
- Time-memory tradeoffs
- Random walks and collision search
- Algorithm optimization

---

### 3. AES Square Attack Toolkit
**Location:** `aes-toolkit/`

Educational framework demonstrating the Square attack on AES.

- Focus: understanding internal diffusion and structural weaknesses
- Designed as a cryptanalysis learning tool

**Key concepts:**
- Integral cryptanalysis
- AES structure (SubBytes, ShiftRows, MixColumns)
- Attack surface in reduced-round ciphers

---

## Technical Stack

- Language: C
- Paradigm: low-level, performance-oriented implementations
- Environment: Linux
- Tooling: GCC, Makefiles (depending on module)

---

## Skills Demonstrated

- Cryptanalysis and attack modeling
- Implementation of advanced algorithms from research papers
- Finite fields and number theory
- Hash function and block cipher internals
- Performance-aware C programming
- Experimental evaluation and parameter tuning

---

## How to Use

Each module is independent. Navigate to a subdirectory and follow its instructions:

## Author

Youssef MORCHID
Engineering student — Cybersecurity, Embedded Systems & AI
