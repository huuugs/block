#!/bin/bash
#
# 使用 box64 在 ARM64 环境下编译 Android App
# 成功验证的配置 - Java 1.8 + box64
#

set -e

GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'

echo "======================================"
echo "使用 Box64 编译 Android App"
echo "======================================"
echo ""

# 检测 box64
if ! command -v box64 >/dev/null 2>&1; then
    echo -e "${RED}错误: box64 未安装${NC}"
    echo "请运行: apt install box64"
    exit 1
fi

echo -e "${GREEN}✓${NC} box64 已安装"
box64 --version 2>/dev/null | head -1 || echo "box64"

# 设置环境变量
export ANDROID_HOME=/root/Android/sdk
export ANDROID_SDK_ROOT=/root/Android/sdk

echo ""
echo "环境配置:"
echo "  ANDROID_HOME=$ANDROID_HOME"
echo "  EMULATOR=box64"
echo ""

# 步骤 1: 包装 build-tools
echo "======================================"
echo "步骤 1: 包装 Build Tools"
echo "======================================"
bash wrap-build-tools.sh
echo ""

# 步骤 2: 包装 Gradle 缓存
echo "======================================"
echo "步骤 2: 包装 Gradle 缓存中的 AAPT2"
echo "======================================"
bash fix-gradle-cache.sh
echo ""

# 步骤 3: 清理并构建
echo "======================================"
echo "步骤 3: 清理并构建"
echo "======================================"

rm -rf /root/aaa/termux-npu-bridge/build
rm -rf /root/aaa/termux-npu-bridge/app/build
rm -rf /data/data/com.termux/files/home/.gradle/caches/8.11.1/transforms/*/transformed/output

echo "运行: ./gradlew clean assembleDebug"
echo ""

# 执行构建
./gradlew clean assembleDebug --no-daemon "$@"
BUILD_RESULT=$?

echo ""
echo "======================================"
if [ $BUILD_RESULT -eq 0 ]; then
    echo -e "${GREEN}✓ 构建成功！${NC}"
    echo ""
    echo "APK 位置:"
    ls -lh app/build/outputs/apk/debug/*.apk 2>/dev/null || echo "未找到 APK"

    # 显示 APK 信息
    echo ""
    echo "APK 包信息:"
    /root/Android/sdk/build-tools/34.0.0/aapt2 dump badging app/build/outputs/apk/debug/app-debug.apk 2>&1 | grep -E "package:|versionCode:|versionName:|sdkVersion:" | head -5
else
    echo -e "${RED}✗ 构建失败${NC}"
    echo ""
    echo "提示:"
    echo "1. 检查 box64 是否正常工作"
    echo "2. 运行 bash wrap-build-tools.sh 检查工具包装"
    echo "3. 查看 BUILD_SUCCESS.md 了解成功的配置"
fi
echo "======================================"

exit $BUILD_RESULT
