#!/bin/bash
#
# NPU Bridge 设备部署脚本
# 将测试脚本部署到Android设备并运行测试
#
# 使用方法:
#   bash deploy_to_device.sh              # 部署并交互式测试
#   bash deploy_to_device.sh --test       # 部署并自动运行测试
#   bash deploy_to_device.sh --install    # 仅安装APK
#

set -e

APK_PATH="app/build/outputs/apk/debug/app-debug.apk"
PACKAGE_NAME="com.npubridge"
DEVICE_TEMP_DIR="/data/local/tmp/npu_test"

echo "=================================================="
echo "   NPU Bridge 设备部署工具"
echo "=================================================="
echo ""

# 检查adb
if ! command -v adb &> /dev/null; then
    echo "错误: adb未安装"
    echo "请安装Android SDK Platform Tools"
    exit 1
fi

# 检查APK
if [ ! -f "$APK_PATH" ]; then
    echo "错误: APK文件不存在: $APK_PATH"
    echo "请先构建APK: ./gradlew assembleDebug"
    exit 1
fi

# 检查设备连接
echo "1. 检查设备连接..."
DEVICES=$(adb devices | grep -w "device" | wc -l)
if [ "$DEVICES" -eq 0 ]; then
    echo "   错误: 未检测到Android设备"
    echo "   请确保:"
    echo "     - 设备已通过USB连接"
    echo "     - 已启用USB调试"
    echo "     - 已授权此电脑"
    exit 1
fi
echo "   ✓ 检测到 $DEVICES 个设备"
echo ""

# 安装APK
echo "2. 安装NPU Bridge应用..."
adb install -r "$APK_PATH" 2>/dev/null || {
    echo "   正在首次安装..."
    adb install "$APK_PATH"
}
echo "   ✓ APK已安装"
echo ""

# 推送测试脚本
echo "3. 部署测试脚本到设备..."
adb shell "mkdir -p $DEVICE_TEMP_DIR"

# 推送Python测试脚本
TEST_FILES=(
    "python_client/quick_test.py"
    "python_client/npu_client.py"
    "python_client/npu_bridge.py"
    "python_client/termux_npu_test.py"
)

for file in "${TEST_FILES[@]}"; do
    if [ -f "$file" ]; then
        echo "   推送: $file"
        adb push "$file" "$DEVICE_TEMP_DIR/"
    fi
done

# 创建便捷启动脚本
adb shell "cat > $DEVICE_TEMP_DIR/run_test.sh << 'EOF'
#!/system/bin/sh
echo \"=== NPU Bridge 快速测试 ===\"
echo \"\"
cd $DEVICE_TEMP_DIR

# 检查Python
if ! command -v python3 >/dev/null 2>&1; then
    echo \"错误: 未找到python3\"
    echo \"请安装: pkg install python\"
    exit 1
fi

# 检查numpy
python3 -c \"import numpy\" 2>/dev/null || {
    echo \"错误: 未找到numpy模块\"
    echo \"请安装: pkg install numpy\"
    exit 1
}

# 运行测试
python3 quick_test.py
EOF
chmod +x $DEVICE_TEMP_DIR/run_test.sh"

echo "   ✓ 测试脚本已部署"
echo ""

# 检查Termux
echo "4. 检查Termux环境..."
if adb shell "pm list packages" | grep -q "com.termux"; then
    echo "   ✓ Termux已安装"

    # 安装numpy
    echo ""
    echo "5. 安装Python依赖..."
    echo "   在Termux中运行:"
    echo "   pkg update && pkg install python numpy"
else
    echo "   ⚠ Termux未安装"
    echo "   从F-Droid或GitHub安装Termux"
fi

echo ""
echo "=================================================="
echo "   部署完成!"
echo "=================================================="
echo ""
echo "下一步操作:"
echo ""
echo "方法A - 在设备Termux中运行:"
echo "  1. 打开NPU Bridge应用"
echo "  2. 点击'启动服务'"
echo "  3. 打开Termux"
echo "  4. 运行: cd $DEVICE_TEMP_DIR && python3 quick_test.py"
echo ""
echo "方法B - 使用adb直接测试:"
echo "  1. 在设备上打开NPU Bridge应用并启动服务"
echo "  2. 运行: bash android_test.sh"
echo ""
echo "方法C - 推送测试到Termux:"
echo "  bash push_to_termux.sh"
echo ""

# 如果指定了--test，自动运行测试
if [ "$1" = "--test" ]; then
    echo "自动运行测试..."
    bash android_test.sh
fi
