#!/bin/bash
#
# CalculatorApp Build Script for ARM64 (aarch64) with box64
# This script handles building Android apps when the SDK build tools are x86-64
# binaries that need box64 emulation on ARM64.
#

set -e

echo "=== 开始构建 CalculatorApp ==="

# 设置环境变量
export ANDROID_HOME=/opt/android-sdk
export ANDROID_SDK_ROOT=/opt/android-sdk
PROJECT_DIR="/root/a"
cd "$PROJECT_DIR"

# 确保 local.properties 存在
if [ ! -f "local.properties" ]; then
    echo "创建 local.properties..."
    echo "sdk.dir=$ANDROID_HOME" > local.properties
fi

# 停止 Gradle 守护进程
echo "停止 Gradle 守护进程..."
gradle --stop 2>/dev/null || true

# 包装 Gradle 缓存中的 AAPT2（如果需要）
echo "检查并包装 AAPT2 二进制文件..."
wrap_aapt2_if_needed() {
    local aapt2_dir="$1"
    if [ -d "$aapt2_dir" ] && [ -f "$aapt2_dir/aapt2" ]; then
        local file_type=$(file "$aapt2_dir/aapt2" 2>/dev/null || echo "")
        if echo "$file_type" | grep -q "ELF 64-bit LSB.*x86-64"; then
            if [ ! -f "$aapt2_dir/aapt2.orig" ]; then
                echo "  包装 $aapt2_dir/aapt2"
                mv "$aapt2_dir/aapt2" "$aapt2_dir/aapt2.orig"
                cat > "$aapt2_dir/aapt2" << 'EOF'
#!/bin/bash
box64 "$0.orig" "$@"
EOF
                chmod +x "$aapt2_dir/aapt2"
            fi
        fi
    fi
}

# 查找并包装 Gradle 缓存中的 AAPT2
find /data/data/com.termux/files/home/.gradle/caches -type d -name "aapt2-*linux" 2>/dev/null | while read -r dir; do
    wrap_aapt2_if_needed "$dir"
done

# 清理构建
echo "清理旧的构建..."
gradle clean --quiet

# 构建
echo "开始构建 Debug APK..."
gradle assembleDebug --quiet

# 复制 APK
echo "复制 APK 到项目根目录..."
cp app/build/outputs/apk/debug/app-debug.apk "$PROJECT_DIR/CalculatorApp.apk"

echo "=== 构建完成 ==="
echo "APK 位置: $PROJECT_DIR/CalculatorApp.apk"
ls -lh "$PROJECT_DIR/CalculatorApp.apk"
