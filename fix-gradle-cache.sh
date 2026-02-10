#!/bin/bash
set -e

GRADLE_CACHE="/data/data/com.termux/files/home/.gradle/caches"

# 使用 box64 而不是 qemu-x86_64
EMULATOR="box64"

echo "======================================"
echo "包装 Gradle 缓存中的 AAPT2 (使用 $EMULATOR)"
echo "======================================"

wrapped_count=0
skipped_count=0

while IFS= read -r aapt2_path; do
  if [ -f "$aapt2_path" ] && file "$aapt2_path" 2>/dev/null | grep -q "ELF 64-bit.*x86-64"; then
    if [ ! -f "${aapt2_path}.real.bin" ]; then
      echo "包装: $aapt2_path"
      mv "$aapt2_path" "${aapt2_path}.real.bin"
      cat > "$aapt2_path" << EOF
#!/bin/bash
SCRIPT_DIR=\$(cd "\$(dirname "\$0")" && pwd)
exec $EMULATOR "\$SCRIPT_DIR/aapt2.real.bin" "\$@"
EOF
      chmod +x "$aapt2_path" "${aapt2_path}.real.bin"
      wrapped_count=$((wrapped_count + 1))
      echo "  ✓ 已包装"
    else
      skipped_count=$((skipped_count + 1))
    fi
  fi
done < <(find "$GRADLE_CACHE" -path "*/transformed/aapt2-*/aapt2" -type f 2>/dev/null)

echo ""
echo "包装统计:"
echo "  新包装: $wrapped_count"
echo "  已跳过: $skipped_count"
echo ""

# 更新 AAPT2 JAR
jar_file=$(find "$GRADLE_CACHE/modules-2/files-2.1/com.android.tools.build/aapt2" -name "aapt2-8.2.0-*.jar" 2>/dev/null | head -1)

if [ -f "$jar_file" ]; then
  echo "更新 AAPT2 JAR..."
  extract_dir="/tmp/aapt2-jar-$$"
  rm -rf "$extract_dir"
  mkdir -p "$extract_dir"
  cd "$extract_dir"
  unzip -q "$jar_file"

  if [ -f "aapt2.real.bin" ] || [ -f "aapt2" ] && file "aapt2" 2>/dev/null | grep -q "ELF 64-bit.*x86-64"; then
    if [ ! -f "aapt2.real.bin" ]; then
      mv aapt2 aapt2.real.bin
    fi

    cat > aapt2 << EOF
#!/bin/bash
SCRIPT_DIR=\$(cd "\$(dirname "\$0")" && pwd)
exec $EMULATOR "\$SCRIPT_DIR/aapt2.real.bin" "\$@"
EOF
    chmod +x aapt2 aapt2.real.bin

    # 重新打包
    jar cf "$jar_file" *
    echo "  ✓ AAPT2 JAR 已更新"
  else
    echo "  ⊘ AAPT2 JAR 已是正确的版本"
  fi

  rm -rf "$extract_dir"
fi

echo ""
echo "完成！可以运行: ./gradlew assembleDebug"
