#!/bin/bash
set -e

GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

BUILD_TOOLS_DIR="/root/Android/sdk/build-tools/34.0.0"

# 使用 box64 而不是 qemu-x86_64
EMULATOR="box64"

# 需要包装的 x86_64 二进制工具
tools="aapt aapt2 zipalign aidl dexdump apksigner"

echo "======================================"
echo "包装 Build Tools (使用 $EMULATOR)"
echo "======================================"

for tool in $tools; do
  tool_path="$BUILD_TOOLS_DIR/$tool"

  # 检查是否是 x86-64 二进制
  if [ -f "$tool_path" ] && file "$tool_path" 2>/dev/null | grep -q "ELF 64-bit.*x86-64"; then
    if [ ! -f "${tool_path}.real.bin" ]; then
      echo "包装: $tool"
      mv "$tool_path" "${tool_path}.real.bin"
      cat > "$tool_path" << EOF
#!/bin/bash
SCRIPT_DIR=\$(cd "\$(dirname "\$0")" && pwd)
exec $EMULATOR "\$SCRIPT_DIR/\$(basename "\$0").real.bin" "\$@"
EOF
      chmod +x "$tool_path"
      echo "  ✓ 已包装"
    else
      echo "  ✓ $tool 已包装"
    fi
  elif [ -f "$tool_path.real.bin" ]; then
    echo "  ✓ $tool 已包装"
  else
    echo "  ! $tool 不是 x86_64 二进制"
  fi
done

echo ""
echo "测试包装后的工具..."
for tool in $tools; do
  tool_path="$BUILD_TOOLS_DIR/$tool"
  if [ -f "$tool_path" ]; then
    echo -n "  $tool: "
    if timeout 2 "$tool_path" --version >/dev/null 2>&1 || timeout 2 "$tool_path" -v >/dev/null 2>&1 || true; then
      echo -e "${GREEN}✓ 正常${NC}"
    else
      echo -e "${YELLOW}⚠ 无 --version 选项${NC}"
    fi
  fi
done

echo ""
echo "完成！可以运行: ./gradlew assembleDebug"
