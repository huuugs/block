#!/bin/bash
#
# Termux Android 编译环境一键配置脚本
# 适用于 Termux PRoot-Distro ARM64 环境
#
# 使用方法:
#   bash setup-termux-android-build.sh
#

set -e

GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
BLUE='\033[0;34m'
NC='\033[0m'

# ============================================================================
# 配置区域
# ============================================================================

# 安装目录
INSTALL_DIR="${ANDROID_HOME:-$HOME/Android/sdk}"
CMDLINE_TOOLS_VERSION="11076708"
BUILD_TOOLS_VERSION="34.0.0"
PLATFORM_VERSION="34"

# 下载镜像 (可选: 使用国内镜像加速)
# USE_CHINA_MIRROR="true"

# ============================================================================
# 工具函数
# ============================================================================

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

check_command() {
    command -v "$1" >/dev/null 2>&1
}

# 检测架构
detect_arch() {
    ARCH=$(uname -m)
    case "$ARCH" in
        aarch64|arm64)
            print_success "检测到架构: ARM64"
            ;;
        x86_64)
            print_warning "检测到架构: x86_64 (不需要 box64)"
            NEEDS_BOX64=false
            ;;
        *)
            print_error "不支持的架构: $ARCH"
            exit 1
            ;;
    esac
}

# ============================================================================
# 步骤 1: 环境检查
# ============================================================================

print_header "步骤 1: 环境检查"

# 检查是否在 Termux PRoot 环境中
if [ -f "/proc/version" ] && grep -qi "proot" /proc/version 2>/dev/null; then
    print_success "检测到 PRoot-Distro 环境"
else
    print_warning "未检测到 PRoot-Distro 环境"
    print_warning "此脚本专为 PRoot-Distro 设计，在其他环境可能无法正常工作"
fi

# 检测架构
NEEDS_BOX64=true
detect_arch

# 检查可用空间
AVAILABLE_SPACE=$(df -h "$HOME" | tail -1 | awk '{print $4}')
print_step "可用磁盘空间: $AVAILABLE_SPACE"

echo ""

# ============================================================================
# 步骤 2: 安装系统依赖
# ============================================================================

print_header "步骤 2: 安装系统依赖"

# 更新包列表
print_step "更新包列表..."
apt update

# 安装基本工具
print_step "安装基本工具..."
apt install -y \
    wget \
    curl \
    unzip \
    git \
    rsync \
    bc \
    file

# 安装 Java
print_step "安装 Java (OpenJDK 17)..."
if check_command java; then
    JAVA_VERSION=$(java -version 2>&1 | head -1)
    print_success "Java 已安装: $JAVA_VERSION"
else
    apt install -y openjdk-17-jdk
    print_success "Java 安装完成"
fi

# 安装 box64 (仅 ARM64 需要)
if [ "$NEEDS_BOX64" = true ]; then
    print_step "安装 box64..."
    if check_command box64; then
        BOX64_VERSION=$(box64 --version 2>/dev/null | head -1 || echo "box64")
        print_success "box64 已安装: $BOX64_VERSION"
    else
        apt install -y box64
        print_success "box64 安装完成"
    fi
fi

# 安装其他有用的工具
print_step "安装其他工具..."
apt install -y \
    zip \
    pkg-config \
    build-essential \
    python3 \
    python3-pip \
    2>/dev/null || true

print_success "系统依赖安装完成"
echo ""

# ============================================================================
# 步骤 3: 下载并安装 Android SDK
# ============================================================================

print_header "步骤 3: 安装 Android SDK"

# 创建 SDK 目录
mkdir -p "$INSTALL_DIR"
print_step "SDK 安装目录: $INSTALL_DIR"

# 下载 commandlinetools
CMDLINE_TOOLS_DIR="$INSTALL_DIR/cmdline-tools"
CMDLINE_TOOLS_ZIP="/tmp/commandlinetools-linux.zip"

print_step "下载 Android Command Line Tools..."

if [ ! -f "$CMDLINE_TOOLS_ZIP" ]; then
    DOWNLOAD_URL="https://dl.google.com/android/repository/commandlinetools-linux-${CMDLINE_TOOLS_VERSION}_latest.zip"

    echo "下载地址: $DOWNLOAD_URL"
    wget -O "$CMDLINE_TOOLS_ZIP" "$DOWNLOAD_URL"
    print_success "下载完成"
else
    print_success "已存在: $CMDLINE_TOOLS_ZIP"
fi

# 解压 commandlinetools
print_step "解压 Command Line Tools..."
mkdir -p "$CMDLINE_TOOLS_DIR"
cd "$CMDLINE_TOOLS_DIR"

# 创建 latest 目录 (Android SDK 要求的结构)
mkdir -p latest
cd latest
unzip -q -o "$CMDLINE_TOOLS_ZIP"
rm -f "$CMDLINE_TOOLS_ZIP"

# 创建目录结构: cmdline-tools/latest/bin/...
print_success "Command Line Tools 解压完成"

cd "$INSTALL_DIR"
echo ""

# ============================================================================
# 步骤 4: 安装 Android SDK 组件
# ============================================================================

print_header "步骤 4: 安装 Android SDK 组件"

# 设置环境变量
export ANDROID_HOME="$INSTALL_DIR"
export ANDROID_SDK_ROOT="$INSTALL_DIR"
export PATH="$PATH:$ANDROID_HOME/cmdline-tools/latest/bin:$ANDROID_HOME/platform-tools"

# 接受许可
print_step "接受 Android SDK 许可证..."
yes | "$ANDROID_HOME/cmdline-tools/latest/bin/sdkmanager" --licenses 2>/dev/null || true

# 安装必需组件
print_step "安装 Android SDK 组件..."

SDK_COMPONENTS=(
    "platform-tools"
    "platforms;android-$PLATFORM_VERSION"
    "build-tools;$BUILD_TOOLS_VERSION"
    "cmdline-tools;latest"
)

for component in "${SDK_COMPONENTS[@]}"; do
    print_step "安装 $component ..."
    "$ANDROID_HOME/cmdline-tools/latest/bin/sdkmanager" "$component" 2>/dev/null || true
done

print_success "SDK 组件安装完成"
echo ""

# ============================================================================
# 步骤 5: 配置环境变量
# ============================================================================

print_header "步骤 5: 配置环境变量"

# 检测 shell 配置文件
SHELL_CONFIG=""
if [ -n "$ZSH_VERSION" ]; then
    SHELL_CONFIG="$HOME/.zshrc"
elif [ -n "$BASH_VERSION" ]; then
    SHELL_CONFIG="$HOME/.bashrc"
else
    SHELL_CONFIG="$HOME/.profile"
fi

print_step "Shell 配置文件: $SHELL_CONFIG"

# 检查是否已配置
if grep -q "ANDROID_HOME" "$SHELL_CONFIG" 2>/dev/null; then
    print_warning "环境变量已存在于 $SHELL_CONFIG"
    print_warning "跳过环境变量配置"
else
    print_step "添加环境变量到 $SHELL_CONFIG..."

    cat >> "$SHELL_CONFIG" << 'EOF'

# Android SDK 环境变量 (由 setup-termux-android-build.sh 添加)
export ANDROID_HOME=$HOME/Android/sdk
export ANDROID_SDK_ROOT=$ANDROID_HOME
export PATH=$PATH:$ANDROID_HOME/cmdline-tools/latest/bin
export PATH=$PATH:$ANDROID_HOME/platform-tools
export PATH=$PATH:$ANDROID_HOME/build-tools/34.0.0
EOF

    print_success "环境变量已添加"
    print_warning "请运行: source $SHELL_CONFIG"
fi

echo ""

# ============================================================================
# 步骤 6: 包装 build-tools
# ============================================================================

print_header "步骤 6: 包装 Build Tools"

if [ "$NEEDS_BOX64" = true ]; then
    BUILD_TOOLS_DIR="$INSTALL_DIR/build-tools/$BUILD_TOOLS_VERSION"

    if [ -d "$BUILD_TOOLS_DIR" ]; then
        print_step "包装 build-tools 中的 x86_64 二进制..."

        tools="aapt aapt2 zipalign aidl dexdump apksigner"
        wrapped_count=0

        for tool in $tools; do
            tool_path="$BUILD_TOOLS_DIR/$tool"

            if [ -f "$tool_path" ]; then
                if file "$tool_path" 2>/dev/null | grep -q "ELF 64-bit.*x86-64"; then
                    if [ ! -f "${tool_path}.real.bin" ]; then
                        print_step "包装: $tool"
                        mv "$tool_path" "${tool_path}.real.bin"
                        cat > "$tool_path" << EOF
#!/bin/bash
SCRIPT_DIR=\$(cd "\$(dirname "\$0")" && pwd)
exec box64 "\$SCRIPT_DIR/\$(basename "\$0").real.bin" "\$@"
EOF
                        chmod +x "$tool_path"
                        wrapped_count=$((wrapped_count + 1))
                        print_success "  ✓ 已包装"
                    else
                        print_success "  ✓ $tool 已包装"
                    fi
                fi
            fi
        done

        if [ $wrapped_count -gt 0 ]; then
            print_success "已包装 $wrapped_count 个工具"
        else
            print_warning "没有需要包装的工具"
        fi
    else
        print_warning "build-tools 目录不存在: $BUILD_TOOLS_DIR"
    fi
else
    print_success "x86_64 环境，无需包装工具"
fi

echo ""

# ============================================================================
# 步骤 7: 验证安装
# ============================================================================

print_header "步骤 7: 验证安装"

export ANDROID_HOME="$INSTALL_DIR"
export ANDROID_SDK_ROOT="$INSTALL_DIR"
export PATH="$PATH:$ANDROID_HOME/cmdline-tools/latest/bin:$ANDROID_HOME/platform-tools"

# 验证 Java
if check_command java; then
    JAVA_VERSION=$(java -version 2>&1 | head -1)
    print_success "Java: $JAVA_VERSION"
else
    print_error "Java 未安装"
fi

# 验证 box64
if [ "$NEEDS_BOX64" = true ]; then
    if check_command box64; then
        BOX64_VERSION=$(box64 --version 2>/dev/null | head -1 || echo "box64")
        print_success "box64: $BOX64_VERSION"
    else
        print_error "box64 未安装"
    fi
fi

# 验证 Android SDK
if [ -d "$INSTALL_DIR" ]; then
    print_success "Android SDK: $INSTALL_DIR"

    # 检查关键组件
    [ -d "$INSTALL_DIR/cmdline-tools/latest" ] && print_success "  - cmdline-tools" || print_error "  - cmdline-tools 缺失"
    [ -d "$INSTALL_DIR/platform-tools" ] && print_success "  - platform-tools" || print_error "  - platform-tools 缺失"
    [ -d "$INSTALL_DIR/build-tools/$BUILD_TOOLS_VERSION" ] && print_success "  - build-tools $BUILD_TOOLS_VERSION" || print_error "  - build-tools 缺失"
    [ -d "$INSTALL_DIR/platforms/android-$PLATFORM_VERSION" ] && print_success "  - platform $PLATFORM_VERSION" || print_error "  - platform 缺失"
else
    print_error "Android SDK 未安装"
fi

# 验证 adb
if check_command adb; then
    ADB_VERSION=$(adb version | head -1)
    print_success "ADB: $ADB_VERSION"
else
    print_warning "ADB 不可用 (可能需要安装 platform-tools)"
fi

echo ""

# ============================================================================
# 完成
# ============================================================================

print_header "安装完成"

echo "环境配置:"
echo "  ANDROID_HOME=$ANDROID_HOME"
echo "  ANDROID_SDK_ROOT=$ANDROID_SDK_ROOT"
echo ""

echo "下一步操作:"
echo "  1. 重新加载 shell 配置:"
echo "     source $SHELL_CONFIG"
echo ""
echo "  2. 或者手动设置环境变量:"
echo "     export ANDROID_HOME=$INSTALL_DIR"
echo "     export ANDROID_SDK_ROOT=$INSTALL_DIR"
echo "     export PATH=\$PATH:\$ANDROID_HOME/cmdline-tools/latest/bin:\$ANDROID_HOME/platform-tools"
echo ""
echo "  3. 在你的 Android 项目目录中使用编译脚本:"
echo "     bash /path/to/termux-android-build.sh assembleDebug"
echo ""
echo "  4. 或复制通用编译脚本到你的项目:"
echo "     cp /path/to/termux-android-build.sh /path/to/your/project/"
echo ""

# 检查当前会话是否已设置环境
if [ -z "$ANDROID_HOME" ] || [ "$ANDROID_HOME" != "$INSTALL_DIR" ]; then
    print_warning "当前会话环境变量未更新"
    print_warning "请运行: export ANDROID_HOME=$INSTALL_DIR"
fi

print_success "Termux Android 编译环境配置完成！"
echo ""
