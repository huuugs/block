#!/bin/bash
#
# 安装验证脚本 - 检查 Android 编译环境是否正确配置
#

set -e

GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'

print_success() {
    echo -e "${GREEN}✓${NC} $1"
}

print_error() {
    echo -e "${RED}✗${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}⚠${NC} $1"
}

print_header() {
    echo ""
    echo "======================================"
    echo "$1"
    echo "======================================"
    echo ""
}

ERRORS=0
WARNINGS=0

# ============================================================================
# 1. 环境检查
# ============================================================================

print_header "1. 环境检查"

# 检查是否在 PRoot 环境
if [ -f "/proc/version" ] && grep -qi "proot" /proc/version 2>/dev/null; then
    print_success "PRoot-Distro 环境"
else
    print_warning "未检测到 PRoot 环境"
    WARNINGS=$((WARNINGS + 1))
fi

# 检查架构
ARCH=$(uname -m)
if [ "$ARCH" = "aarch64" ] || [ "$ARCH" = "arm64" ]; then
    print_success "架构: ARM64 ($ARCH)"
else
    print_error "不支持的架构: $ARCH"
    ERRORS=$((ERRORS + 1))
fi

# ============================================================================
# 2. 系统工具检查
# ============================================================================

print_header "2. 系统工具检查"

# Java
if command -v java >/dev/null 2>&1; then
    JAVA_VERSION=$(java -version 2>&1 | head -1)
    print_success "Java: $JAVA_VERSION"
else
    print_error "Java 未安装"
    ERRORS=$((ERRORS + 1))
fi

# javac
if command -v javac >/dev/null 2>&1; then
    print_success "javac: $(javac -version 2>&1)"
else
    print_error "javac 未安装"
    ERRORS=$((ERRORS + 1))
fi

# box64 (ARM64 需要)
if [ "$ARCH" = "aarch64" ] || [ "$ARCH" = "arm64" ]; then
    if command -v box64 >/dev/null 2>&1; then
        BOX64_VERSION=$(box64 --version 2>/dev/null | head -1 || echo "box64")
        print_success "box64: $BOX64_VERSION"
    else
        print_error "box64 未安装 (ARM64 环境必需)"
        ERRORS=$((ERRORS + 1))
    fi
fi

# wget
if command -v wget >/dev/null 2>&1; then
    print_success "wget: $(wget --version | head -1)"
else
    print_error "wget 未安装"
    ERRORS=$((ERRORS + 1))
fi

# unzip
if command -v unzip >/dev/null 2>&1; then
    print_success "unzip: $(unzip -v | head -1)"
else
    print_error "unzip 未安装"
    ERRORS=$((ERRORS + 1))
fi

# ============================================================================
# 3. Android SDK 检查
# ============================================================================

print_header "3. Android SDK 检查"

# 检查环境变量
if [ -n "$ANDROID_HOME" ]; then
    print_success "ANDROID_HOME: $ANDROID_HOME"
else
    print_error "ANDROID_HOME 未设置"
    ERRORS=$((ERRORS + 1))
fi

if [ -n "$ANDROID_SDK_ROOT" ]; then
    print_success "ANDROID_SDK_ROOT: $ANDROID_SDK_ROOT"
else
    print_warning "ANDROID_SDK_ROOT 未设置"
    WARNINGS=$((WARNINGS + 1))
fi

# 检查 SDK 目录
SDK_DIR="${ANDROID_HOME:-$HOME/Android/sdk}"
if [ -d "$SDK_DIR" ]; then
    print_success "SDK 目录存在: $SDK_DIR"

    # 检查关键组件
    if [ -d "$SDK_DIR/cmdline-tools" ]; then
        print_success "  - cmdline-tools"
    else
        print_error "  - cmdline-tools 缺失"
        ERRORS=$((ERRORS + 1))
    fi

    if [ -d "$SDK_DIR/platform-tools" ]; then
        print_success "  - platform-tools"
    else
        print_error "  - platform-tools 缺失"
        ERRORS=$((ERRORS + 1))
    fi

    if [ -d "$SDK_DIR/build-tools" ]; then
        BUILD_TOOLS_COUNT=$(find "$SDK_DIR/build-tools" -maxdepth 1 -type d -name "[0-9]*" | wc -l)
        print_success "  - build-tools ($BUILD_TOOLS_COUNT 个版本)"
    else
        print_error "  - build-tools 缺失"
        ERRORS=$((ERRORS + 1))
    fi

    if [ -d "$SDK_DIR/platforms" ]; then
        PLATFORM_COUNT=$(find "$SDK_DIR/platforms" -maxdepth 1 -type d -name "android-*" | wc -l)
        print_success "  - platforms ($PLATFORM_COUNT 个版本)"
    else
        print_error "  - platforms 缺失"
        ERRORS=$((ERRORS + 1))
    fi
else
    print_error "SDK 目录不存在: $SDK_DIR"
    ERRORS=$((ERRORS + 1))
fi

# ============================================================================
# 4. Android 工具检查
# ============================================================================

print_header "4. Android 工具检查"

# sdkmanager
if command -v sdkmanager >/dev/null 2>&1; then
    print_success "sdkmanager: $(sdkmanager --version 2>&1 || echo 'available')"
else
    print_error "sdkmanager 不可用 (检查 PATH 是否包含 \$ANDROID_HOME/cmdline-tools/latest/bin)"
    ERRORS=$((ERRORS + 1))
fi

# adb
if command -v adb >/dev/null 2>&1; then
    print_success "adb: $(adb version | head -1)"
else
    print_warning "adb 不可用"
    WARNINGS=$((WARNINGS + 1))
fi

# ============================================================================
# 5. Build Tools 包装检查
# ============================================================================

print_header "5. Build Tools 包装检查"

if [ -d "$SDK_DIR/build-tools" ]; then
    # 查找最新的 build-tools
    LATEST_BUILD_TOOLS=$(find "$SDK_DIR/build-tools" -maxdepth 1 -type d -name "[0-9]*" | sort -V | tail -1)

    if [ -n "$LATEST_BUILD_TOOLS" ]; then
        echo "检查: $LATEST_BUILD_TOOLS"

        tools_to_check="aapt aapt2 zipalign"

        for tool in $tools_to_check; do
            tool_path="$LATEST_BUILD_TOOLS/$tool"

            if [ -f "$tool_path" ]; then
                if file "$tool_path" 2>/dev/null | grep -q "ELF 64-bit.*x86-64"; then
                    if [ -f "${tool_path}.real.bin" ]; then
                        print_success "$tool: 已包装"
                    else
                        print_error "$tool: 未包装 (x86_64 二进制)"
                        ERRORS=$((ERRORS + 1))
                    fi
                else
                    print_success "$tool: ARM64 原生"
                fi
            else
                print_warning "$tool: 不存在"
                WARNINGS=$((WARNINGS + 1))
            fi
        done
    fi
fi

# ============================================================================
# 6. Gradle 检查
# ============================================================================

print_header "6. Gradle 检查"

# 检查 gradle.properties
if [ -f "gradle.properties" ]; then
    print_success "找到 gradle.properties"

    # 检查关键配置
    if grep -q "org.gradle.daemon=false" gradle.properties; then
        print_success "  - daemon 已禁用"
    else
        print_warning "  - daemon 未禁用 (Termux 环境建议禁用)"
        WARNINGS=$((WARNINGS + 1))
    fi

    if grep -q "android.aapt2FromMavenEnabled=false" gradle.properties; then
        print_success "  - AAPT2 Maven 已禁用"
    else
        print_warning "  - AAPT2 Maven 未禁用"
        WARNINGS=$((WARNINGS + 1))
    fi
else
    print_warning "未在当前目录找到 gradle.properties"
    WARNINGS=$((WARNINGS + 1))
fi

# ============================================================================
# 7. PATH 检查
# ============================================================================

print_header "7. PATH 检查"

path_checks=0

# 检查 Android SDK 路径是否在 PATH 中
if [ -n "$ANDROID_HOME" ]; then
    if echo ":$PATH:" | grep -q ":$ANDROID_HOME/cmdline-tools/latest/bin:"; then
        print_success "cmdline-tools 在 PATH 中"
        path_checks=$((path_checks + 1))
    else
        print_warning "cmdline-tools 不在 PATH 中"
        WARNINGS=$((WARNINGS + 1))
    fi

    if echo ":$PATH:" | grep -q ":$ANDROID_HOME/platform-tools:"; then
        print_success "platform-tools 在 PATH 中"
        path_checks=$((path_checks + 1))
    else
        print_warning "platform-tools 不在 PATH 中"
        WARNINGS=$((WARNINGS + 1))
    fi
fi

# ============================================================================
# 总结
# ============================================================================

print_header "验证总结"

echo "错误: $ERRORS"
echo "警告: $WARNINGS"
echo ""

if [ $ERRORS -eq 0 ] && [ $WARNINGS -eq 0 ]; then
    print_success "所有检查通过！环境配置正确。"
    echo ""
    echo "可以开始编译 Android App:"
    echo "  bash termux-android-build.sh assembleDebug"
    exit 0
elif [ $ERRORS -eq 0 ]; then
    print_success "环境基本配置正确，但有 $WARNINGS 个警告。"
    echo ""
    echo "可以尝试编译，如果遇到问题请解决警告。"
    exit 0
else
    print_error "发现 $ERRORS 个错误，请修复后再编译。"
    echo ""
    echo "建议运行: bash setup-termux-android-build.sh"
    exit 1
fi
