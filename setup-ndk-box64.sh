#!/bin/bash
# Wrap NDK x86-64 binaries with box64 for ARM64

NDK_DIR="/root/block/android-ndk"
TOOLCHAIN_DIR="${NDK_DIR}/toolchains/llvm/prebuilt/linux-x86_64"

echo "=== Wrapping NDK binaries with box64 ==="
echo ""

# Check if box64 is available
if ! command -v box64 &> /dev/null; then
    echo "Error: box64 not found. Install with: pkg install box64"
    exit 1
fi

# Wrap binary function
wrap_binary() {
    local bin_path="$1"
    local bin_name=$(basename "$bin_path")

    if [ ! -f "$bin_path" ]; then
        return
    fi

    # Check if it's an x86-64 binary
    local file_info=$(file "$bin_path" 2>/dev/null || echo "")
    if ! echo "$file_info" | grep -q "ELF 64-bit LSB.*x86-64"; then
        return
    fi

    # Skip if already wrapped
    if [ -f "${bin_path}.orig" ]; then
        echo "  ✓ $bin_name already wrapped"
        return
    fi

    # Create wrapper
    echo "  Wrapping $bin_name..."
    mv "$bin_path" "${bin_path}.orig"
    cat > "$bin_path" << EOF
#!/bin/bash
box64 "${bin_path}.orig" "\$@"
EOF
    chmod +x "$bin_path"
}

# Wrap clang and related tools
echo "Wrapping clang toolchain..."
for bin in clang clang++ clang-14 clang++-14 llvm-ar llvm-strip llvm-ranlib; do
    wrap_binary "${TOOLCHAIN_DIR}/bin/$bin"
done

# Wrap build tools
echo "Wrapping build tools..."
for bin in aapt zipalign; do
    wrap_binary "${NDK_DIR}/$bin" 2>/dev/null || true
done

echo ""
echo "Done! NDK binaries are now wrapped with box64."
