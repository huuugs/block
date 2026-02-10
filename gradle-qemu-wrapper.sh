#!/bin/bash
#
# Gradle QEMU 包装脚本
# 在 Gradle 运行时监控并自动包装新提取的 AAPT2
#

set -e

# 颜色
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

export ANDROID_HOME=/root/Android/sdk
export ANDROID_SDK_ROOT=/root/Android/sdk

# 创建包装函数
wrap_aapt2() {
    local aapt2_file="$1"

    if [ -f "$aapt2_file" ] && [ ! -f "${aapt2_file}.real.bin" ]; then
        # 检查是否是 x86-64 二进制
        if file "$aapt2_file" 2>/dev/null | grep -q "ELF 64-bit.*x86-64"; then
            mv "$aapt2_file" "${aapt2_file}.real.bin"
            cat > "$aapt2_file" << 'WRAPPERSCRIPT'
#!/bin/bash
SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)
exec qemu-x86_64 -L /usr/x86_64-linux-gnu "$SCRIPT_DIR/aapt2.real.bin" "$@"
WRAPPERSCRIPT
            chmod +x "$aapt2_file" "${aapt2_file}.real.bin"
            echo -e "${GREEN}✓${NC} 自动包装: $aapt2_file"
        fi
    fi
}

# 预包装 Gradle 缓存中的现有 AAPT2
echo "预包装 Gradle 缓存中的 AAPT2..."
find /data/data/com.termux/files/home/.gradle/caches -name "aapt2" -type f -exec file {} \; 2>/dev/null | grep "ELF 64-bit.*x86-64" | cut -d: -f1 | while read aapt2; do
    wrap_aapt2 "$aapt2"
done

# 启动后台监控进程
monitor_gradle_cache() {
    local gradle_cache="/data/data/com.termux/files/home/.gradle/caches/8.11.1/transforms"

    while true; do
        # 查找新提取的 AAPT2
        find "$gradle_cache" -name "aapt2" -type f -mmin -1 2>/dev/null | while read aapt2; do
            if [ -f "$aapt2" ]; then
                file_info=$(file "$aapt2" 2>/dev/null || echo "")
                if echo "$file_info" | grep -q "ELF 64-bit.*x86-64"; then
                    wrap_aapt2 "$aapt2" 2>/dev/null || true
                fi
            fi
        done
        sleep 2
    done
}

# 启动监控
echo -e "${YELLOW}启动 AAPT2 监控...${NC}"
monitor_gradle_cache &
MONITOR_PID=$!

# 清理函数
cleanup() {
    kill $MONITOR_PID 2>/dev/null || true
    wait $MONITOR_PID 2>/dev/null || true
}

trap cleanup EXIT INT TERM

# 运行 Gradle
echo ""
echo "运行 Gradle 构建..."
echo ""

./gradlew "$@"
RESULT=$?

echo ""
if [ $RESULT -eq 0 ]; then
    echo -e "${GREEN}构建成功!${NC}"
else
    echo -e "${YELLOW}构建失败${NC}"
fi

exit $RESULT
