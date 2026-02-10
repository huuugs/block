#!/bin/bash
#
# 通用 Android App 编译脚本 (Termux PRoot-Distro ARM64 环境)
# 使用 box64 模拟 x86_64 Android build tools
#
# 使用方法:
#   cd /path/to/your/android/project
#   bash /path/to/termux-android-build.sh [gradle_task]
#
# 示例:
#   bash termux-android-build.sh assembleDebug
#   bash termux-android-build.sh assembleRelease
#   bash termux-android-build.sh clean assembleDebug
#

set -e

GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
BLUE='\033[0;34m'
NC='\033[0m'

# ============================================================================
# 配置区域 - 根据需要修改
# ============================================================================

# Android SDK 路径 (自动检测或手动设置)
ANDROID_SDK_ROOT="${ANDROID_HOME:-/root/Android/sdk}"

# Gradle 任务 (默认: assembleDebug)
GRADLE_TASKS="${*:-assembleDebug}"

# 仿真器 (box64 或 qemu-x86_64)
EMULATOR="box64"

# Java 版本 (推荐 1.8 以避免 jlink 兼容性问题)
JAVA_TARGET_VERSION="1.8"

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
    if command -v "$1" >/dev/null 2>&1; then
        return 0
    else
        return 1
    fi
}

# ============================================================================
# 步骤 1: 环境检查
# ============================================================================

print_header "步骤 1: 环境检查"

# 检查是否在正确的目录
if [ ! -f "build.gradle" ] && [ ! -f "build.gradle.kts" ]; then
    print_error "错误: 未找到 build.gradle 或 build.gradle.kts"
    print_error "请在 Android 项目根目录下运行此脚本"
    exit 1
fi
print_success "找到 Android 项目"

# 检查 Android SDK
if [ ! -d "$ANDROID_SDK_ROOT" ]; then
    print_error "错误: Android SDK 不存在: $ANDROID_SDK_ROOT"
    print_error "请设置 ANDROID_HOME 环境变量或修改脚本中的 ANDROID_SDK_ROOT"
    exit 1
fi
print_success "Android SDK: $ANDROID_SDK_ROOT"

# 检查 Java
if check_command java; then
    JAVA_VERSION=$(java -version 2>&1 | head -1 | awk -F '"' '{print $2}')
    print_success "Java: $JAVA_VERSION"
else
    print_error "错误: Java 未安装"
    print_error "请运行: apt install openjdk-17"
    exit 1
fi

# 检查仿真器
if check_command "$EMULATOR"; then
    EMULATOR_VERSION=$($EMULATOR --version 2>/dev/null | head -1 || echo "$EMULATOR")
    print_success "$EMULATOR: $EMULATOR_VERSION"
else
    print_error "错误: $EMULATOR 未安装"
    print_error "请运行: apt install box64"
    exit 1
fi

# 检查 Gradle wrapper
if [ ! -f "gradlew" ]; then
    print_error "错误: 未找到 gradlew"
    print_error "请确保项目包含 Gradle wrapper"
    exit 1
fi
chmod +x gradlew
print_success "Gradle wrapper 已准备"

export ANDROID_HOME="$ANDROID_SDK_ROOT"
export ANDROID_SDK_ROOT="$ANDROID_SDK_ROOT"

echo ""
echo "环境配置:"
echo "  ANDROID_HOME=$ANDROID_HOME"
echo "  EMULATOR=$EMULATOR"
echo "  JAVA_TARGET=$JAVA_TARGET_VERSION"
echo "  GRADLE_TASKS=$GRADLE_TASKS"
echo ""

# ============================================================================
# 步骤 2: 检查并包装 build-tools
# ============================================================================

print_header "步骤 2: 包装 Build Tools"

# 查找所有 build-tools 版本
BUILD_TOOLS_VERSIONS=$(find "$ANDROID_SDK_ROOT/build-tools" -maxdepth 1 -type d -name "[0-9]*" | sort -V | tail -1)

if [ -z "$BUILD_TOOLS_VERSIONS" ]; then
    print_warning "未找到 build-tools"
else
    BUILD_TOOLS_DIR="$BUILD_TOOLS_VERSIONS"
    print_success "找到 build-tools: $BUILD_TOOLS_DIR"

    # 需要包装的 x86_64 二进制工具
    tools="aapt aapt2 zipalign aidl dexdump apksigner"

    for tool in $tools; do
        tool_path="$BUILD_TOOLS_DIR/$tool"

        if [ -f "$tool_path" ]; then
            # 检查是否是 x86-64 二进制
            if file "$tool_path" 2>/dev/null | grep -q "ELF 64-bit.*x86-64"; then
                if [ ! -f "${tool_path}.real.bin" ]; then
                    print_step "包装: $tool"
                    mv "$tool_path" "${tool_path}.real.bin"
                    cat > "$tool_path" << EOF
#!/bin/bash
SCRIPT_DIR=\$(cd "\$(dirname "\$0")" && pwd)
exec $EMULATOR "\$SCRIPT_DIR/\$(basename "\$0").real.bin" "\$@"
EOF
                    chmod +x "$tool_path"
                    print_success "  ✓ 已包装"
                else
                    print_success "  ✓ $tool 已包装"
                fi
            else
                print_warning "  ! $tool 不是 x86_64 二进制"
            fi
        else
            print_warning "  ! $tool 不存在"
        fi
    done
fi

echo ""

# ============================================================================
# 步骤 3: 包装 Gradle 缓存中的 AAPT2
# ============================================================================

print_header "步骤 3: 包装 Gradle 缓存中的 AAPT2"

GRADLE_CACHE="$HOME/.gradle/caches"

if [ -d "$GRADLE_CACHE" ]; then
    wrapped_count=0

    find "$GRADLE_CACHE" -path "*/transformed/aapt2-*/aapt2" -type f 2>/dev/null | while read aapt2_path; do
        if [ -f "$aapt2_path" ] && file "$aapt2_path" 2>/dev/null | grep -q "ELF 64-bit.*x86-64"; then
            if [ ! -f "${aapt2_path}.real.bin" ]; then
                print_step "包装: $aapt2_path"
                mv "$aapt2_path" "${aapt2_path}.real.bin"
                cat > "$aapt2_path" << EOF
#!/bin/bash
SCRIPT_DIR=\$(cd "\$(dirname "\$0")" && pwd)
exec $EMULATOR "\$SCRIPT_DIR/aapt2.real.bin" "\$@"
EOF
                chmod +x "$aapt2_path" "${aapt2_path}.real.bin"
                print_success "  ✓ 已包装"
                wrapped_count=$((wrapped_count + 1))
            fi
        fi
    done

    if [ $wrapped_count -eq 0 ]; then
        print_success "AAPT2 缓存已是正确的状态"
    fi
else
    print_warning "Gradle 缓存目录不存在: $GRADLE_CACHE"
fi

echo ""

# ============================================================================
# 步骤 4: 检查和修复 build.gradle 配置
# ============================================================================

print_header "步骤 4: 检查 build.gradle 配置"

# 检查 Java 版本配置
if grep -q "jvmTarget = \"17\"" build.gradle* 2>/dev/null; then
    print_warning "检测到 jvmTarget = \"17\""
    print_warning "建议修改为 jvmTarget = \"1.8\" 以避免兼容性问题"

    # 自动检测 app 模块
    if [ -f "app/build.gradle.kts" ]; then
        print_warning "app/build.gradle.kts 中可能有 jvmTarget = \"17\""
        print_warning "请手动检查并修改为 jvmTarget = \"1.8\""
    fi
fi

# 检查 gradle.properties
GRADLE_PROPS="gradle.properties"
if [ -f "$GRADLE_PROPS" ]; then
    print_success "找到 gradle.properties"

    # 检查关键配置
    if ! grep -q "org.gradle.daemon=false" "$GRADLE_PROPS"; then
        print_step "添加 org.gradle.daemon=false 到 gradle.properties"
        echo "" >> "$GRADLE_PROPS"
        echo "# Termux PRoot 环境优化" >> "$GRADLE_PROPS"
        echo "org.gradle.daemon=false" >> "$GRADLE_PROPS"
        echo "org.gradle.caching=false" >> "$GRADLE_PROPS"
        print_success "  ✓ 已添加"
    fi

    if ! grep -q "android.aapt2FromMavenEnabled=false" "$GRADLE_PROPS"; then
        print_step "添加 android.aapt2FromMavenEnabled=false"
        echo "android.aapt2FromMavenEnabled=false" >> "$GRADLE_PROPS"
        echo "android.enableAapt2Daemon=false" >> "$GRADLE_PROPS"
        print_success "  ✓ 已添加"
    fi
fi

echo ""

# ============================================================================
# 步骤 5: 执行构建
# ============================================================================

print_header "步骤 5: 执行构建"

print_step "运行: ./gradlew $GRADLE_TASKS"
echo ""

# 执行构建
./gradlew $GRADLE_TASKS --no-daemon
BUILD_RESULT=$?

echo ""
echo "======================================"
if [ $BUILD_RESULT -eq 0 ]; then
    print_success "构建成功！"

    # 查找生成的 APK
    echo ""
    echo "APK 文件:"
    find . -name "*.apk" -path "*/build/outputs/apk/*" -type f -exec ls -lh {} \; 2>/dev/null || echo "未找到 APK 文件"
else
    print_error "构建失败"
    echo ""
    echo "常见问题排查:"
    echo "1. 检查 app/build.gradle.kts 中的 jvmTarget 是否为 \"1.8\""
    echo "2. 检查是否有编译错误日志"
    echo "3. 尝试运行: ./gradlew clean --no-daemon"
fi
echo "======================================"

exit $BUILD_RESULT
