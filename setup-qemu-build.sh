#!/bin/bash
#
# QEMU x86 模拟环境 Android App 构建脚本
# 用于在 ARM64 (Termux/PRoot) 环境中编译 Android app
#

set -e

echo "======================================"
echo "QEMU x86 模拟 Android App 构建脚本"
echo "======================================"
echo ""

# 颜色定义
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# 检查 QEMU
if ! command -v qemu-x86_64 &> /dev/null; then
    echo -e "${RED}错误: qemu-x86_64 未安装${NC}"
    echo "请运行: apt install qemu-user-static"
    exit 1
fi

echo -e "${GREEN}✓${NC} QEMU x86_64 已安装"

# 检查 x86_64 库
if [ ! -d /usr/x86_64-linux-gnu/lib64 ]; then
    echo -e "${YELLOW}警告: x86_64 运行库未找到${NC}"
    echo "可能需要安装: apt install libc6:x86_64 libstdc++6:x86_64"
fi

# 设置环境变量
export ANDROID_HOME=/root/Android/sdk
export ANDROID_SDK_ROOT=/root/Android/sdk
export PATH="/root/qemu-bin:$PATH"

echo -e "${GREEN}✓${NC} 环境变量已设置"
echo "  ANDROID_HOME=$ANDROID_HOME"
echo ""

# 步骤 1: 包装 build-tools
echo "======================================"
echo "步骤 1: 包装 Build Tools"
echo "======================================"

BUILD_TOOLS_DIR="$ANDROID_HOME/build-tools/34.0.0"
tools="aapt zipalign aidl dexdump apksigner"

for tool in $tools; do
    real_bin="$BUILD_TOOLS_DIR/${tool}.real.bin"
    wrapper="$BUILD_TOOLS_DIR/$tool"

    if [ -f "$real_bin" ]; then
        # 更新包装脚本
        cat > "$wrapper" << EOF
#!/bin/bash
SCRIPT_DIR=\$(cd "\$(dirname "\$0")" && pwd)
exec qemu-x86_64 -L /usr/x86_64-linux-gnu "\$SCRIPT_DIR/\$(basename "\$0").real.bin" "\$@"
EOF
        chmod +x "$wrapper"
        echo -e "${GREEN}✓${NC} 已包装: $tool"
    fi
done

# 包装 aapt2
if [ -f "$BUILD_TOOLS_DIR/aapt2.real.bin" ]; then
    cat > "$BUILD_TOOLS_DIR/aapt2" << 'EOF'
#!/bin/bash
SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)
exec qemu-x86_64 -L /usr/x86_64-linux-gnu "$SCRIPT_DIR/aapt2.real.bin" "$@"
EOF
    chmod +x "$BUILD_TOOLS_DIR/aapt2"
    echo -e "${GREEN}✓${NC} 已包装: aapt2"
fi

echo ""

# 步骤 2: 包装 Gradle 缓存中的 AAPT2
echo "======================================"
echo "步骤 2: 包装 Gradle 缓存中的 AAPT2"
echo "======================================"

GRADLE_CACHE="/data/data/com.termux/files/home/.gradle/caches"
wrapped_count=0

find "$GRADLE_CACHE" -path "*/transformed/aapt2-*/aapt2.real.bin" | while read real_bin; do
    wrapper="${real_bin%.real.bin}"
    dir=$(dirname "$real_bin")

    cat > "$wrapper" << 'EOF'
#!/bin/bash
SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)
exec qemu-x86_64 -L /usr/x86_64-linux-gnu "$SCRIPT_DIR/aapt2.real.bin" "$@"
EOF
    chmod +x "$wrapper"
    chmod +x "$real_bin"
    wrapped_count=$((wrapped_count + 1))
    echo -e "${GREEN}✓${NC} 已包装: $wrapper"
done

echo ""

# 步骤 3: 更新 AAPT2 JAR
echo "======================================"
echo "步骤 3: 更新 AAPT2 JAR"
echo "======================================"

jar_file=$(find "$GRADLE_CACHE/modules-2/files-2.1/com.android.tools.build/aapt2" -name "aapt2-8.2.0-10154469-linux.jar" 2>/dev/null | head -1)

if [ -f "$jar_file" ]; then
    extract_dir="/tmp/aapt2-jar-fix"
    rm -rf "$extract_dir"
    mkdir -p "$extract_dir"
    cd "$extract_dir"
    unzip -q "$jar_file"

    if [ -f "aapt2.real.bin" ]; then
        # 创建包装脚本
        cat > aapt2 << 'EOF'
#!/bin/bash
SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)
exec qemu-x86_64 -L /usr/x86_64-linux-gnu "$SCRIPT_DIR/aapt2.real.bin" "$@"
EOF
        chmod +x aapt2 aapt2.real.bin

        # 重新打包
        jar cf "$jar_file" *
        echo -e "${GREEN}✓${NC} AAPT2 JAR 已更新"
    fi
fi

echo ""

# 步骤 4: 清理 Gradle 缓存
echo "======================================"
echo "步骤 4: 清理 Gradle 缓存"
echo "======================================"

rm -rf "$GRADLE_CACHE/8.11.1/transforms"
rm -rf /root/aaa/termux-npu-bridge/.gradle
rm -rf /root/aaa/termux-npu-bridge/build

echo -e "${GREEN}✓${NC} Gradle 缓存已清理"
echo ""

# 步骤 5: 开始构建
echo "======================================"
echo "步骤 5: 开始构建"
echo "======================================"
echo ""

cd /root/aaa/termux-npu-bridge

echo "运行: ./gradlew clean assembleDebug"
echo ""

./gradlew clean assembleDebug --no-daemon "$@"

echo ""
echo "======================================"
if [ $? -eq 0 ]; then
    echo -e "${GREEN}构建成功!${NC}"
    echo ""
    echo "APK 位置:"
    ls -lh app/build/outputs/apk/debug/*.apk 2>/dev/null || echo "未找到 APK"
else
    echo -e "${RED}构建失败${NC}"
    echo ""
    echo "提示:"
    echo "1. 检查 QEMU 是否正常工作: qemu-x86_64 --version"
    echo "2. 检查 x86_64 库是否存在: ls /usr/x86_64-linux-gnu/lib64"
    echo "3. 查看详细日志: ./gradlew assembleDebug --debug"
fi
echo "======================================"
