#!/bin/bash
# Build script for AES Cryptanalysis Toolkit

set -e  # Exit on error

echo "========================================"
echo "AES Cryptanalysis Toolkit - Build Script"
echo "========================================"
echo ""

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Check for required tools
echo "Checking dependencies..."

if ! command -v gcc &> /dev/null; then
    echo -e "${RED}Error: gcc not found. Please install GCC compiler.${NC}"
    exit 1
fi

if ! command -v make &> /dev/null; then
    echo -e "${RED}Error: make not found. Please install Make build system.${NC}"
    exit 1
fi

echo -e "${GREEN}All dependencies found.${NC}"
echo ""

# Parse command line arguments
BUILD_TYPE="release"
VERBOSE=""

while [[ $# -gt 0 ]]; do
    case $1 in
        --debug)
            BUILD_TYPE="debug"
            shift
            ;;
        --sanitize)
            BUILD_TYPE="sanitize"
            shift
            ;;
        --verbose)
            VERBOSE="V=1"
            shift
            ;;
        --clean)
            echo "Cleaning previous build..."
            make clean
            echo -e "${GREEN}Clean complete.${NC}"
            exit 0
            ;;
        --help)
            echo "Usage: $0 [options]"
            echo ""
            echo "Options:"
            echo "  --debug      Build with debug symbols"
            echo "  --sanitize   Build with AddressSanitizer"
            echo "  --verbose    Show detailed build output"
            echo "  --clean      Clean build artifacts"
            echo "  --help       Show this help message"
            echo ""
            exit 0
            ;;
        *)
            echo -e "${RED}Unknown option: $1${NC}"
            echo "Use --help for usage information"
            exit 1
            ;;
    esac
done

# Build the project
echo "Building project (type: $BUILD_TYPE)..."
echo ""

case $BUILD_TYPE in
    debug)
        make debug $VERBOSE
        ;;
    sanitize)
        make sanitize $VERBOSE
        ;;
    *)
        make all $VERBOSE
        ;;
esac

if [ $? -eq 0 ]; then
    echo ""
    echo -e "${GREEN}========================================"
    echo "Build successful!"
    echo "========================================${NC}"
    echo ""
    echo "Executable location: bin/aes_attack"
    echo ""
    echo "Quick start:"
    echo "  ./bin/aes_attack -r          # Attack with random key"
    echo "  ./bin/aes_attack -h          # Show help"
    echo "  make run                     # Build and run"
    echo ""
else
    echo ""
    echo -e "${RED}========================================"
    echo "Build failed!"
    echo "========================================${NC}"
    exit 1
fi
