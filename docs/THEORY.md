# Mathematical Theory Behind the Square Attack

## 1. Finite Field GF(2^8) Arithmetic

### 1.1 Field Construction

AES operates in the finite field GF(2^8), which contains 256 elements. Each element is represented as a polynomial of degree at most 7 with binary coefficients:

```
a(x) = a7*x^7 + a6*x^6 + ... + a1*x + a0, where ai ∈ {0,1}
```

For implementation, we represent each element as a byte:
```
0x57 = 01010111₂ ↔ x^6 + x^4 + x^2 + x + 1
```

### 1.2 Irreducible Polynomial

The field is defined modulo the irreducible polynomial:
```
m(x) = x^8 + x^4 + x^3 + x + 1 = 0x11B
```

This polynomial cannot be factored over GF(2), ensuring proper field structure.

### 1.3 Addition and Multiplication

**Addition:** XOR operation (coefficient-wise addition mod 2)
```
(x^6 + x^4 + x^2) + (x^4 + x + 1) = x^6 + x^2 + x + 1
0x54 ⊕ 0x13 = 0x47
```

**Multiplication:** Polynomial multiplication followed by reduction modulo m(x)

Example: Multiply by x (xtime operation)
```
p(x) * x = (a7*x^7 + ... + a0) * x
         = a7*x^8 + a6*x^7 + ... + a0*x
         
If a7 = 1:
  x^8 ≡ x^4 + x^3 + x + 1 (mod m(x))
  Result = (a6*x^7 + ... + a0*x) ⊕ (x^4 + x^3 + x + 1)
```

Implementation:
```c
uint8_t xtime(uint8_t p) {
    uint8_t msb = p >> 7;           // Extract bit 7
    uint8_t shifted = p << 1;       // Multiply by x
    uint8_t reduction = msb ? 0x1B : 0x00;  // Conditional reduction
    return shifted ^ reduction;
}
```

## 2. AES Structure

### 2.1 Round Function

Each AES round consists of four operations:

1. **SubBytes:** Non-linear byte substitution using S-box
   - Input: state[i][j]
   - Output: S[state[i][j]]
   - S-box is a bijective function over GF(2^8)

2. **ShiftRows:** Cyclic shift of each row
   ```
   Row 0: no shift
   Row 1: left shift by 1
   Row 2: left shift by 2
   Row 3: left shift by 3
   ```

3. **MixColumns:** Linear transformation over GF(2^8)
   ```
   [c0]   [02 03 01 01] [s0]
   [c1] = [01 02 03 01] [s1]
   [c2]   [01 01 02 03] [s2]
   [c3]   [03 01 01 02] [s3]
   ```
   where multiplication is in GF(2^8)

4. **AddRoundKey:** XOR with round key
   ```
   state[i][j] ^= round_key[i][j]
   ```

### 2.2 Key Schedule

The master key K₀ is expanded into round keys K₁, K₂, ..., K₁₀:

```
K[i] = K[i-1] ⊕ F(K[i-1], i)

where F(K, i) involves:
  - RotWord: rotate last column
  - SubWord: apply S-box
  - XOR with round constant RC[i] = x^(i-1) in GF(2^8)
```

## 3. The Square Distinguisher

### 3.1 Lambda Sets

A **lambda set** (Λ-set) is a collection of 256 plaintexts where:
- One byte position takes all 256 possible values (0x00 to 0xFF)
- All other 15 bytes remain constant

Example:
```
P₀ = [00, 42, 7A, 3F, ...]
P₁ = [01, 42, 7A, 3F, ...]
P₂ = [02, 42, 7A, 3F, ...]
...
P₂₅₅ = [FF, 42, 7A, 3F, ...]
```

### 3.2 Byte State Types

After each round, we track the "type" of each byte position:
- **Constant (C):** Same value in all 256 texts
- **Balanced (B):** XOR of all 256 values = 0
- **All (A):** Takes all 256 values exactly once

### 3.3 Propagation Through AES Operations

**SubBytes:**
- C → C (constant stays constant)
- A → A (bijection maps all values to all values)
- B → B (balanced stays balanced through bijection)

**ShiftRows:**
- Only permutes positions, preserves types

**MixColumns:** (linear over GF(2^8))
```
If one input byte is A and others are C:
  Each output byte becomes B (balanced)

Reason: Linear combination of:
  - One byte taking all values (A)
  - Three constant bytes (C)
  Result: Each output takes 256 distinct values that XOR to 0
```

**AddRoundKey:**
- Preserves all types (XOR with constant)

### 3.4 Three-Round Property

Starting with a Λ-set (one byte A, others C):

```
Round 0 (initial):  [A, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C]

After SubBytes:     [A, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C]
After ShiftRows:    [A, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C]
After MixColumns:   [B, B, B, B, C, C, C, C, C, C, C, C, C, C, C, C]
After AddRoundKey:  [B, B, B, B, C, C, C, C, C, C, C, C, C, C, C, C]

Round 1:
After full round:   [B, B, B, B, B, B, B, B, B, B, B, B, B, B, B, B]

Round 2:
After full round:   [B, B, B, B, B, B, B, B, B, B, B, B, B, B, B, B]

Round 3:
After full round:   [B, B, B, B, B, B, B, B, B, B, B, B, B, B, B, B]
```

**Key property:** After 3 rounds, all bytes are balanced (B).

This means:
```
⊕(i=0 to 255) AES₃(k, pᵢ) = 0^128
```

## 4. Key Recovery Attack on 3.5 Rounds

### 4.1 Attack Setup

Target: 3.5-round AES = 3 full rounds + final partial round
- Final partial round: SubBytes + ShiftRows + AddRoundKey (no MixColumns)

### 4.2 Attack Algorithm

For each byte position j (0 to 15):

1. **Generate Λ-set:** Create 256 plaintexts with byte j varying

2. **Query Oracle:** Get 256 ciphertexts C₀, C₁, ..., C₂₅₅

3. **Key Guessing:** For each guess g ∈ {0x00, ..., 0xFF}:
   ```
   a. Partially decrypt: dᵢ[j] = S⁻¹(Cᵢ[j] ⊕ g)
   b. Compute: sum = ⊕(i=0 to 255) dᵢ[j]
   c. If sum = 0: g is a candidate for K₄[j]
   ```

4. **Filtering:** Use additional Λ-sets to eliminate false positives

5. **Key Schedule Inversion:** Once all bytes recovered, invert key schedule:
   ```
   K₄ → K₃ → K₂ → K₁ → K₀ (master key)
   ```

### 4.3 Why This Works

After partial decryption with correct key guess:
```
dᵢ[j] = S⁻¹(Cᵢ[j] ⊕ K₄[j])
      = state after 3 full rounds at position j
```

Since the distinguisher guarantees:
```
⊕(i=0 to 255) (state after 3 rounds) = 0
```

We have:
```
⊕(i=0 to 255) S⁻¹(Cᵢ[j] ⊕ K₄[j]) = 0  (if K₄[j] is correct)
                                  ≠ 0  (with high probability if wrong)
```

### 4.4 Complexity Analysis

**Time Complexity:**
- 16 bytes to recover
- 256 key guesses per byte
- 256 plaintexts per Λ-set
- Total: 16 × 256 × 256 = 2^20 operations (about 1 million)

**Data Complexity:**
- 16 Λ-sets × 256 plaintexts = 4,096 chosen plaintexts

**Success Probability:**
- False positive rate: ≈ 2^-8 per wrong guess
- With 2-3 additional Λ-sets: success rate > 99.9%

## 5. Mathematical Justification

### 5.1 Balanced Property Preservation

**Theorem:** If input bytes are balanced (B), and we apply a linear transformation over GF(2^8), the output bytes remain balanced.

**Proof:** Let L be a linear function. For balanced input x:
```
⊕(i=0 to 255) xᵢ = 0

Then:
⊕(i=0 to 255) L(xᵢ) = L(⊕(i=0 to 255) xᵢ)  (by linearity)
                     = L(0)
                     = 0
```

### 5.2 Why Random Permutation Fails

For a random permutation π:
```
P[⊕(i=0 to 255) π(pᵢ) = 0] ≈ 2^-128
```

This is because each bit position in the XOR behaves like a random coin flip, and we need all 128 bits to be 0 simultaneously.

## 6. Limitations and Extensions

### 6.1 Why Full AES is Secure

- Full AES-128 has 10 rounds
- Square distinguisher only works up to 3-4 rounds
- Remaining rounds provide sufficient diffusion
- No known attack better than brute force on full AES

### 6.2 Related Attacks

- **4-round attack:** Requires meet-in-the-middle techniques
- **5-round attack:** Partial key recovery with higher complexity
- **Impossible differential:** Alternative structural attack
- **Truncated differential:** Related to integral cryptanalysis

### 6.3 Robustness Against Modifications

The attack still works if:
- Different irreducible polynomial for GF(2^8)
- Different S-box (as long as it's bijective)
- Different MDS matrix for MixColumns (as long as it's linear)

This is because the attack exploits structural properties (bijectivity, linearity) rather than specific numerical values.

## References

1. Daemen, J., Knudsen, L., & Rijmen, V. (1997). "The Block Cipher Square"
2. Knudsen, L., & Wagner, D. (2002). "Integral Cryptanalysis"
3. Ferguson, N., et al. (2001). "Improved Cryptanalysis of Rijndael"
4. NIST FIPS 197 (2001). "Advanced Encryption Standard"
