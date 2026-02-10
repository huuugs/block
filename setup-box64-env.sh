#!/bin/bash
# Setup box64 environment for Android NDK

set -e

GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'

echo "=== Setting up box64 for NDK ==="
echo ""

NDK_DIR="/root/block/android-ndk"
X86_ROOT="/usr/x86_64-linux-gnu"

# 1. Install required x86-64 libraries
echo -e "${YELLOW}[1/4] Installing x86-64 libraries...${NC}"

# Install static box64-wrapped libraries
apt install -y \
    libstdc++6:amd64 \
    libgcc-s1:amd64 \
    libz1:amd64 \
    libtinfo6:amd64 \
    libxml2:amd64 \
    2>/dev/null || echo "  Some packages may not be available"

echo "  ✓ Libraries installed"

# 2. Create library symlinks for box64
echo -e "${YELLOW}[2/4] Setting up library symlinks...${NC}"

mkdir -p /emul/86-linux
for lib in libc.so.6 libm.so.6 libdl.so.2 librt.so.1 libpthread.so.0; do
    if [ -f "$X86_ROOT/lib64/$lib" ] && [ ! -L "/emul/86-linux/$lib" ]; then
        ln -sf "$X86_ROOT/lib64/$lib" "/emul/86-linux/$lib"
    fi
done

# Also check /usr/x86_64-linux-gnu/lib
for lib in libz.so.1 libtinfo.so.6; do
    found=$(find $X86_ROOT -name "$lib" 2>/dev/null | head -1)
    if [ -n "$found" ] && [ ! -L "/emul/86-linux/$(basename $lib)" ]; then
        ln -sf "$found" "/emul/86-linux/$(basename $lib)"
    fi
done

echo "  ✓ Symlinks created"

# 3. Restore and re-wrap NDK binaries
echo -e "${YELLOW}[3/4] Re-wrapping NDK binaries...${NC}"

cd "$NDK_DIR/toolchains/llvm/prebuilt/linux-x86_64/bin"

# Function to wrap a binary
wrap_binary() {
    local bin_name="$1"
    local bin_path="$2"

    if [ ! -f "$bin_path" ]; then
        return
    fi

    # Check if it's an x86-64 binary
    local file_info=$(file "$bin_path" 2>/dev/null)
    if ! echo "$file_info" | grep -q "ELF 64-bit LSB.*x86-64"; then
        return
    fi

    # Restore original if needed
    if [ -f "${bin_path}.orig" ]; then
        bin_path="${bin_path}.orig"
    fi

    # Already wrapped?
    if head -1 "$bin_path" 2>/dev/null | grep -q "box64"; then
        return
    fi

    # Create wrapper
    echo "  Wrapping: $bin_name"
    mv "$bin_path" "${bin_path}.tmp"
    cat > "$bin_path" << WRAPPER_EOF
#!/bin/bash
# Box64 wrapper for $bin_name
export QEMU_LD_PREFIX=/usr/x86_64-linux-gnu
export LD_LIBRARY_PATH=/emul/86-linux:$X86_ROOT/lib:$X86_ROOT/lib64:\$LD_LIBRARY_PATH
box64 "${bin_path}.tmp" "\$@"
WRAPPER_EOF
    chmod +x "$bin_path"
}

# Wrap key tools
for tool in clang clang++ clang-14 clang++-14 lld ld.lld llvm-ar llvm-ranlib llvm-strip; do
    wrap_binary "$tool" "./$tool"
done

echo "  ✓ Binaries wrapped"

# 4. Test the setup
echo -e "${YELLOW}[4/4] Testing setup...${NC}"

export QEMU_LD_PREFIX=/usr/x86_64-linux-gnu
export LD_LIBRARY_PATH=/emul/86-linux:$X86_ROOT/lib:$X86_ROOT/lib64:$LD_LIBRARY_PATH

if timeout 5 box64 ./clang-14 --version >/dev/null 2>&1; then
    echo -e "${GREEN}✓ box64 setup successful!${NC}"
    box64 ./clang-14 --version | head -3
    exit 0
else
    echo -e "${RED}✗ box64 setup failed${NC}"
    echo ""
    echo "Troubleshooting:"
    echo "  1. Check box64 version: box64 --version"
    echo "  2. Check libraries: ls -la /emul/86-linux/"
    echo "  3. Test manually: QEMU_LD_PREFIX=/usr/x86_64-linux-gnu box64 ./clang-14 --version"
    exit 1
fi
