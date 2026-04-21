# Contributing to AES Cryptanalysis Toolkit

Thank you for your interest in contributing to this educational cryptography project!

## How to Contribute

### Reporting Issues

If you find a bug or have a suggestion:

1. Check existing issues to avoid duplicates
2. Create a new issue with:
   - Clear title and description
   - Steps to reproduce (for bugs)
   - Expected vs actual behavior
   - System information (OS, compiler version)

### Code Contributions

#### Getting Started

1. Fork the repository
2. Clone your fork:
   ```bash
   git clone https://github.com/morchidy/AES-Cryptanalysis-Toolkit.git
   cd AES-Cryptanalysis-Toolkit
   ```

3. Create a feature branch:
   ```bash
   git checkout -b feature/your-feature-name
   ```

4. Make your changes following our coding standards

#### Coding Standards

**C Code Style:**
- Use C99 standard
- Follow existing code formatting
- Use meaningful variable names
- Add comments for complex logic
- Document all public functions with Doxygen-style comments

**Example:**
```c
/**
 * @brief Short description
 * 
 * Detailed description of what the function does.
 * 
 * @param param1 Description of param1
 * @param param2 Description of param2
 * @return Description of return value
 */
int example_function(int param1, uint8_t *param2);
```

**Commit Messages:**
- Use present tense ("Add feature" not "Added feature")
- First line: brief summary (50 chars max)
- Blank line, then detailed description if needed
- Reference issue numbers when applicable

**Example:**
```
Add support for AES-192 key recovery

Implement the Square attack for AES-192 using the extended
key schedule and adjusted round count.

Fixes #42
```

#### Testing

Before submitting:

1. Build the project:
   ```bash
   make clean
   make all
   ```

2. Run existing tests:
   ```bash
   make test
   ```

3. Test your changes manually:
   ```bash
   ./bin/aes_attack -r -v
   ```

4. Check for memory leaks (if applicable):
   ```bash
   make sanitize
   ./bin/aes_attack -r
   ```

#### Submitting Changes

1. Commit your changes:
   ```bash
   git add .
   git commit -m "Your commit message"
   ```

2. Push to your fork:
   ```bash
   git push origin feature/your-feature-name
   ```

3. Create a Pull Request:
   - Provide clear description of changes
   - Reference related issues
   - Explain testing performed

## Project Areas for Contribution

### High Priority

- Additional test cases and test vectors
- Documentation improvements
- Performance optimizations
- Bug fixes

### Medium Priority

- Support for AES-192 and AES-256
- 4-round attack implementation
- Visualization tools (Python scripts)
- Benchmarking suite

### Advanced Topics

- Side-channel attack simulations
- GPU acceleration
- Alternative distinguishers
- Web-based demonstration

## Documentation

When adding new features:

1. Update relevant .md files in `docs/`
2. Add inline code documentation
3. Update README.md if adding user-facing features
4. Include examples in `examples/` directory

## Questions?

Feel free to:
- Open an issue for discussion
- Contact the maintainers
- Join discussions in existing issues

## Code of Conduct

### Our Standards

- Be respectful and inclusive
- Focus on constructive feedback
- Accept gracefully when mistakes happen
- Prioritize educational value

### Scope

This project is for educational purposes. Contributions should:
- Not promote malicious use
- Include proper security disclaimers
- Follow ethical research practices

## License

By contributing, you agree that your contributions will be licensed under the MIT License.

## Recognition

Contributors will be acknowledged in:
- README.md contributors section
- Release notes
- Commit history

Thank you for helping improve this educational resource!
