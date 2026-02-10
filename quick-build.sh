#!/bin/bash
#
# 快速构建脚本 - 用于已初始化后的快速编译
#

set -e

export ANDROID_HOME=/root/Android/sdk
export ANDROID_SDK_ROOT=/root/Android/sdk

echo "======================================"
echo "快速编译 (已初始化)"
echo "======================================"
echo ""

# 确保 AAPT2 权限正确
find /data/data/com.termux/files/home/.gradle/caches/8.11.1/transforms -name "aapt2*" -type f -exec chmod +x {} \; 2>/dev/null || true

echo "运行: ./gradlew assembleDebug"
echo ""

./gradlew assembleDebug --no-daemon "$@"

if [ $? -eq 0 ]; then
    echo ""
    echo "✓ 构建成功!"
    ls -lh app/build/outputs/apk/debug/*.apk
else
    echo ""
    echo "✗ 构建失败"
    echo ""
    echo "如果持续失败，请运行: bash compile-with-qemu.sh"
    exit 1
fi
