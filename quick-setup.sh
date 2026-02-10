#!/bin/bash
#
# 快速配置脚本 - 适用于已安装 Termux PRoot 的环境
# 仅安装必要依赖和 SDK 组件
#
# 使用方法:
#   bash quick-setup.sh
#

set -e

GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
BLUE='\033[0;34m'
NC='\033[0m'

# 配置
ANDROID_SDK_ROOT="${ANDROID_HOME:-/root/Android/sdk}"
BUILD_TOOLS_VERSION="34.0.0"

print_header() {
    echo ""
    echo "======================================"
    echo "$1"
    echo "======================================"
    echo ""
}

print_step() {
    echo -e "${BLUE}▶${NC} $1"
}

print_success() {
    echo -e "${GREEN}✓${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}⚠${NC} $1"
}

print_error() {
    echo -e "${RED}✗${NC} $1"
}

# ============================================================================
# 步骤 1: 安装系统包
# ============================================================================

print_header "步骤 1: 安装系统包"

apt update

# 安装必需工具
apt install -y wget unzip curl box64 openjdk-17-jdk file

print_success "系统包安装完成"
echo ""

# ============================================================================
# 步骤 2: 安装 Android SDK (如果不存在)
# ============================================================================

print_header "步骤 2: 检查 Android SDK"

if [ ! -d "$ANDROID_SDK_ROOT" ]; then
    print_warning "Android SDK 不存在: $ANDROID_SDK_ROOT"
    print_step "请先运行完整配置脚本: bash setup-termux-android-build.sh"
    exit 1
fi

print_success "Android SDK: $ANDROID_SDK_ROOT"
echo ""

# ============================================================================
# 步骤 3: 包装 build-tools
# ============================================================================

print_header "步骤 3: 包装 Build Tools"

BUILD_TOOLS_DIR="$ANDROID_SDK_ROOT/build-tools/$BUILD_TOOLS_VERSION"

if [ ! -d "$BUILD_TOOLS_DIR" ]; then
    print_warning "build-tools 不存在: $BUILD_TOOLS_DIR"
    print_step "尝试安装..."

    export ANDROID_HOME="$ANDROID_SDK_ROOT"
    export ANDROID_SDK_ROOT="$ANDROID_SDK_ROOT"
    export PATH="$PATH:$ANDROID_HOME/cmdline-tools/latest/bin"

    yes | sdkmanager "build-tools;$BUILD_TOOLS_VERSION" 2>/dev/null || true
fi

if [ -d "$BUILD_TOOLS_DIR" ]; then
    print_step "包装 build-tools..."

    tools="aapt aapt2 zipalign aidl dexdump apksigner"

    for tool in $tools; do
        tool_path="$BUILD_TOOLS_DIR/$tool"

        if [ -f "$tool_path" ]; then
            if file "$tool_path" 2>/dev/null | grep -q "ELF 64-bit.*x86-64"; then
                if [ ! -f "${tool_path}.real.bin" ]; then
                    print_step "包装: $tool"
                    mv "$tool_path" "${tool_path}.real.bin"
                    cat > "$tool_path" << 'EOF'
#!/bin/bash
SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)
exec box64 "$SCRIPT_DIR/$(basename "$0").real.bin" "$@"
EOF
                    chmod +x "$tool_path"
                    print_success "  ✓ 已包装"
                fi
            fi
        fi
    done
fi

print_success "Build Tools 包装完成"
echo ""

# ============================================================================
# 步骤 4: 设置环境变量
# ============================================================================

print_header "步骤 4: 环境变量配置"

export ANDROID_HOME="$ANDROID_SDK_ROOT"
export ANDROID_SDK_ROOT="$ANDROID_SDK_ROOT"
export PATH="$PATH:$ANDROID_HOME/cmdline-tools/latest/bin:$ANDROID_HOME/platform-tools"

echo "当前环境:"
echo "  ANDROID_HOME=$ANDROID_HOME"
echo "  PATH=$PATH"
echo ""

# ============================================================================
# 步骤 5: 验证
# ============================================================================

print_header "步骤 5: 验证"

[ -x "$(command -v java)" ] && print_success "Java: $(java -version 2>&1 | head -1)"
[ -x "$(command -v box64)" ] && print_success "box64: $(box64 --version 2>/dev/null | head -1)"
[ -d "$ANDROID_HOME" ] && print_success "Android SDK"
[ -d "$BUILD_TOOLS_DIR" ] && print_success "Build Tools: $BUILD_TOOLS_VERSION"

echo ""
print_success "快速配置完成！"
echo ""
echo "现在可以编译 Android App:"
echo "  1. 进入项目目录: cd /path/to/project"
echo "  2. 运行编译脚本: bash termux-android-build.sh"
echo ""
