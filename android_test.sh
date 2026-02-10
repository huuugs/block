#!/bin/bash
# 在Android设备上通过adb测试NPU Bridge

echo "=== NPU Bridge Android设备测试 ==="
echo ""

# 检查设备连接
echo "检查设备连接..."
if ! adb devices | grep -q "device$"; then
    echo "错误: 未检测到Android设备"
    echo "请确保:"
    echo "  1. 设备已通过USB连接"
    echo "  2. 已启用USB调试"
    echo "  3. 已授权此电脑调试"
    exit 1
fi

echo "✓ 设备已连接"
echo ""

# 检查应用是否安装
echo "检查NPU Bridge应用..."
if adb shell pm list packages | grep -q "com.npubridge"; then
    echo "✓ NPU Bridge已安装"
else
    echo "✗ NPU Bridge未安装"
    echo "请先安装APK:"
    echo "  adb install app/build/outputs/apk/debug/app-debug.apk"
    exit 1
fi
echo ""

# 检查socket文件
echo "检查socket文件..."
if adb shell "[ -f /data/data/com.npubridge/socket/termux_npu_bridge ] && echo exists" | grep -q "exists"; then
    echo "✓ Socket文件存在"
else
    echo "✗ Socket文件不存在"
    echo "请在设备上:"
    echo "  1. 打开NPU Bridge应用"
    echo "  2. 点击'启动服务'"
    echo "  3. 确认服务状态为'运行中'"
    exit 1
fi
echo ""

# 推送测试脚本到设备
echo "推送测试脚本到设备..."
adb push python_client/quick_test.py /data/local/tmp/quick_test.py
adb push python_client/npu_client.py /data/local/tmp/npu_client.py
echo "✓ 脚本已推送"
echo ""

# 运行测试
echo "运行测试..."
echo "---"
adb shell "
    cd /data/local/tmp
    python3 quick_test.py
"
echo "---"
echo ""

echo "测试完成!"
