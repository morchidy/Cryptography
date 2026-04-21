# Square Attack Walkthrough

A step-by-step explanation of how the Square attack recovers the AES master key.

## Overview

The Square attack exploits a structural property of AES that holds for 3 full rounds but breaks down in the 4th round. By adding a partial 4th round (without MixColumns), we can perform a "peeling" technique to recover the last round key, then invert the key schedule to get the master key.

## Prerequisites

**Target:** 3.5-round AES-128
- 3 full rounds (SubBytes, ShiftRows, MixColumns, AddRoundKey)
- 1 partial round (SubBytes, ShiftRows, AddRoundKey only)

**Key Property:** For any lambda set after 3 full rounds:
```
XOR(all 256 ciphertexts) = 0
```

## Step-by-Step Attack

### Step 1: Understanding Lambda Sets

A lambda set is a carefully constructed set of 256 plaintexts:

```
Position:  [0] [1] [2] [3] [4] [5] [6] [7] [8] [9] [10][11][12][13][14][15]
Plaintext 0:  00  42  7A  3F  B1  2C  88  ...  (other bytes constant)
Plaintext 1:  01  42  7A  3F  B1  2C  88  ...
Plaintext 2:  02  42  7A  3F  B1  2C  88  ...
...
Plaintext 255: FF  42  7A  3F  B1  2C  88  ...
```

**Key insight:** Position 0 takes ALL 256 values (0x00-0xFF), while all other positions stay constant.

### Step 2: The 3-Round Property

After encrypting these 256 plaintexts through 3 full rounds of AES:

```c
for (i = 0; i < 256; i++) {
    ciphertext_3rounds[i] = AES_3_rounds(key, plaintext[i]);
}

// This will equal zero for ALL bytes:
xor_sum = 0x00...00;
for (i = 0; i < 256; i++) {
    xor_sum ^= ciphertext_3rounds[i];
}
// xor_sum == 0x00...00 (guaranteed by Square property)
```

**Why this works:** The lambda set propagates through AES operations in a predictable way:
1. SubBytes: Permutes the "all values" property
2. ShiftRows: Just repositions bytes
3. MixColumns: Spreads the property across the column (creates balanced bytes)
4. After 3 rounds: All 16 bytes become "balanced" (XOR to 0)

### Step 3: Adding the Half-Round

But we don't have direct access to the 3-round output. The oracle gives us 3.5-round ciphertexts:

```
3-round output → SubBytes → ShiftRows → AddRoundKey → 3.5-round ciphertext
```

The problem: We can't directly XOR the 3.5-round ciphertexts because the final operations break the property.

**Solution:** Guess the last round key and "peel off" the final operations!

### Step 4: Byte-by-Byte Key Recovery

For each byte position (0-15), we independently recover one byte of the last round key:

```python
# Pseudocode for recovering byte j
for key_guess in range(256):
    xor_sum = 0
    
    for i in range(256):
        # Partially decrypt: undo final AddRoundKey and SubBytes
        temp = ciphertext_3p5[i][j] XOR key_guess  # Undo AddRoundKey
        decrypted = InverseS[temp]                 # Undo SubBytes
        xor_sum ^= decrypted
    
    if xor_sum == 0:
        # This key_guess might be correct!
        candidates.append(key_guess)
```

**Why this works:**
- If `key_guess` is correct, `decrypted` values are the state after 3 rounds
- We know XOR of all states after 3 rounds must be 0
- If `key_guess` is wrong, the XOR will be (essentially) random → non-zero

### Step 5: Dealing with False Positives

Sometimes, by chance, a wrong key guess also gives XOR = 0. 

**Solution:** Use multiple lambda sets!

```python
candidates = test_with_lambda_set_1(position_j)  # might return [0x42, 0x73]

# Filter with additional sets
for extra_set in range(num_additional_sets):
    new_candidates = []
    for candidate in candidates:
        if test_with_new_lambda_set(position_j, candidate):
            new_candidates.append(candidate)
    candidates = new_candidates

# Usually only 1 candidate survives
final_key_byte[j] = candidates[0]
```

**Statistics:**
- Probability of false positive: ~1/256 per wrong guess
- With 2 additional sets: ~1/256^3 ≈ negligible

### Step 6: Complete Example

Let's recover byte 0 of the last round key:

```
Lambda Set Construction:
  Base = [00, 42, 7A, 3F, B1, ...]
  Generate 256 plaintexts with byte 0 varying from 0x00 to 0xFF

Query Oracle (3.5 rounds):
  C[0] = Encrypt([00, 42, 7A, ...]) = [A3, 27, B1, ...]
  C[1] = Encrypt([01, 42, 7A, ...]) = [F2, 88, 3C, ...]
  ...
  C[255] = Encrypt([FF, 42, 7A, ...]) = [1D, C9, 47, ...]

Test Key Guesses:
  For g = 0x00:
    sum = InvS[A3 ^ 00] ^ InvS[F2 ^ 00] ^ ... ^ InvS[1D ^ 00]
    sum = 0x73 (non-zero) → WRONG
  
  For g = 0x01:
    sum = InvS[A3 ^ 01] ^ InvS[F2 ^ 01] ^ ... ^ InvS[1D ^ 01]
    sum = 0x2A (non-zero) → WRONG
  
  ...
  
  For g = 0xE2:
    sum = InvS[A3 ^ E2] ^ InvS[F2 ^ E2] ^ ... ^ InvS[1D ^ E2]
    sum = 0x00 (ZERO!) → CANDIDATE
  
  Continue testing...
  Final candidates: [0xE2]

Last Round Key byte 0 = 0xE2
```

Repeat for all 16 bytes → recover complete last round key.

### Step 7: Master Key Recovery

Once we have K₄ (the last round key), we invert the key schedule:

```
K₄ = E2ECE6E25F081EF4AF6CC65AED3984F9

Inverse key schedule (round 3 → 4):
K₃ = prev_round_key(K₄, round=3)
K₃ = 7D64A1F622629A1289122C48641EB8B1

Inverse key schedule (round 2 → 3):
K₂ = prev_round_key(K₃, round=2)
K₂ = D90154D8BF636BCA363E478252203BCB

Inverse key schedule (round 1 → 2):
K₁ = prev_round_key(K₂, round=1)
K₁ = 12941089AD9F7B4367A13B9237813FA6

Inverse key schedule (round 0 → 1):
K₀ = prev_round_key(K₁, round=0)
K₀ = 4795ED3B61D0C1CE98F81B4119158ED5

Master Key = 4795ED3B61D0C1CE98F81B4119158ED5
```

### Step 8: Verification

Verify the recovered key:

```c
// Encrypt test vector with recovered key
plaintext = [00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00]
recovered_cipher = AES_3p5(K₀, plaintext)

// Encrypt same plaintext with true oracle key
oracle_cipher = AES_3p5(K_true, plaintext)

// Compare
if (recovered_cipher == oracle_cipher):
    print("SUCCESS! Key recovered correctly!")
else:
    print("FAILURE! Attack did not work.")
```

## Complexity Analysis

### Time Complexity

For each byte:
- 256 key guesses
- 256 plaintexts per lambda set
- 1-2 S-box lookups per plaintext

Total: 16 bytes × 256 guesses × 256 texts × 2 ops = **2,097,152 operations**

On modern hardware: ~0.02 seconds

### Data Complexity

Minimum: 16 lambda sets (one per byte) = **4,096 chosen plaintexts**

With filtering: 16 × (1 + 2 additional sets) = 48 lambda sets = **12,288 plaintexts**

Still tiny compared to the full key space (2^128)!

### Success Probability

Without filtering: ~85-90% (false positives can occur)
With 2 additional sets: **>99.9%** (false positives extremely rare)

## Why This Attack Doesn't Work on Full AES

Full AES-128 has **10 full rounds**, not 3.5.

The Square property only holds for 3 rounds:
- After 4+ rounds, the XOR sum is essentially random
- Cannot distinguish from random permutation
- Attack complexity becomes equivalent to brute force

This is why adding more rounds is the primary defense against structural attacks!

## Visualizing the Attack

```
Plaintext Lambda Set (byte 0 varies)
  |
  | 3 Full Rounds
  v
State after 3 rounds (all bytes balanced, XOR = 0)
  |
  | SubBytes
  v
State after SubBytes (still balanced)
  |
  | ShiftRows
  v
State after ShiftRows (permuted, still balanced)
  |
  | AddRoundKey ← GUESS THIS KEY
  v
3.5-Round Ciphertext (we can query this)

Attack:
- Guess key byte
- Reverse AddRoundKey and SubBytes
- Check if XOR = 0 (balanced property)
- If yes → correct key guess
```

## Implementation Notes

**Optimization tricks used:**
1. Constant-time xtime() to prevent side channels
2. Byte-by-byte recovery (parallelizable)
3. Early termination when unique candidate found
4. Efficient XOR accumulation (no array storage needed)

**Potential pitfalls:**
1. Forgetting ShiftRows permutation (byte positions matter!)
2. Using wrong S-box direction (need inverse S-box)
3. Off-by-one errors in key schedule inversion
4. Not using enough filtering lambda sets

## Conclusion

The Square attack is an elegant example of exploiting structural properties rather than brute-forcing keys. It demonstrates:
- Power of chosen-plaintext attacks
- Importance of sufficient rounds
- How mathematical properties propagate through ciphers
- Why full AES is designed with 10 rounds (safety margin)

The attack works because:
1. We can construct special input sets (lambda sets)
2. We can predict output properties after 3 rounds
3. We can test key guesses independently per byte
4. The key schedule is invertible

All of these factors combined make 3.5-round AES vulnerable, while full AES remains secure.
