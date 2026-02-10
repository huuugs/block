#!/bin/bash
#
# 将测试脚本推送到设备Termux环境
#

DEVICE_HOME="/data/data/com.termux/files/home"
DEVICE_TEST_DIR="$DEVICE_HOME/npu_test"

echo "=================================================="
echo "   推送测试脚本到Termux"
echo "=================================================="
echo ""

# 检查Termux是否安装
if ! adb shell "pm list packages" | grep -q "com.termux"; then
    echo "错误: Termux未安装"
    echo "请从F-Droid或GitHub安装Termux"
    exit 1
fi

echo "1. 创建测试目录..."
adb shell "mkdir -p $DEVICE_TEST_DIR"

echo ""
echo "2. 推送测试脚本..."
TEST_FILES=(
    "python_client/quick_test.py"
    "python_client/npu_client.py"
    "python_client/npu_bridge.py"
    "python_client/termux_npu_test.py"
    "python_client/example.py"
    "python_client/test_connection.py"
)

for file in "${TEST_FILES[@]}"; do
    if [ -f "$file" ]; then
        echo "   推送: $file"
        adb push "$file" "$DEVICE_TEST_DIR/"
    fi
done

echo ""
echo "3. 创建便捷启动脚本..."
adb shell "cat > $DEVICE_TEST_DIR/test.sh << 'EOF'
#!/data/data/com.termux/files/usr/bin/bash
echo \"=== NPU Bridge 测试 ===\"
echo \"\"
cd ~/npu_test

# 检查依赖
echo \"检查依赖...\"
python3 -c \"import numpy\" 2>/dev/null || {
    echo \"安装numpy...\"
    pkg install -y python numpy
}

echo \"\"
echo \"运行快速测试...\"
python3 quick_test.py
EOF
chmod +x $DEVICE_TEST_DIR/test.sh"

echo ""
echo "=================================================="
echo "   推送完成!"
echo "=================================================="
echo ""
echo "在设备Termux中运行:"
echo "  cd ~/npu_test"
echo "  bash test.sh"
echo ""
echo "或直接运行:"
echo "  cd ~/npu_test && python3 quick_test.py"
echo ""
