#!/bin/bash
#
# Android Build Environment Setup for ARM64 (aarch64) with box64
# 此脚本用于设置在 ARM64 Linux (Termux/PRoot) 环境下编译 Android 应用的环境
#
# 系统要求：
# - ARM64 (aarch64) Linux (Termux/PRoot-Distro)
# - 网络连接
#
# 关键技术：
# - box64: 在 ARM64 上运行 x86-64 二进制文件
# - Android SDK build-tools 是 x86-64 二进制，需要 box64 仿真
#

set -e

echo "=== Android Build Environment Setup for ARM64 ==="
echo ""

# 颜色输出
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

# 配置变量
ANDROID_SDK_ROOT="${ANDROID_SDK_ROOT:-/opt/android-sdk}"
BUILD_TOOLS_VERSION="34.0.0"

echo "配置:"
echo "  ANDROID_SDK_ROOT: $ANDROID_SDK_ROOT"
echo "  BUILD_TOOLS_VERSION: $BUILD_TOOLS_VERSION"
echo ""

# 检查系统架构
ARCH=$(uname -m)
if [ "$ARCH" != "aarch64" ]; then
    echo "警告: 此脚本设计用于 aarch64 架构，当前架构: $ARCH"
fi

# 1. 检查 box64
echo "=== 1. 检查 box64 ==="
if ! command -v box64 &> /dev/null; then
    echo -e "${YELLOW}box64 未安装${NC}"
    echo "在 Termux 中安装 box64:"
    echo "  pkg install box64"
    echo ""
    echo "或从源码编译: https://github.com/ptitSeb/box64"
    exit 1
fi
echo -e "${GREEN}✓ box64 已安装${NC}: $(box64 --version 2>&1 | head -1)"
echo ""

# 2. 检查 Android SDK
echo "=== 2. 检查 Android SDK ==="
if [ ! -d "$ANDROID_SDK_ROOT" ]; then
    echo -e "${YELLOW}Android SDK 未找到: $ANDROID_SDK_ROOT${NC}"
    echo ""
    echo "请手动安装 Android SDK，建议使用命令行工具:"
    echo "  mkdir -p $ANDROID_SDK_ROOT"
    echo "  cd $ANDROID_SDK_ROOT"
    echo "  # 下载 commandlinetools-linux"
    echo "  # 使用 sdkmanager 安装 build-tools 和 platform"
    echo ""
    exit 1
fi
echo -e "${GREEN}✓ Android SDK 目录存在${NC}"
echo ""

# 3. 检查 build-tools
echo "=== 3. 检查 Build Tools ==="
BUILD_TOOLS_DIR="$ANDROID_SDK_ROOT/build-tools/$BUILD_TOOLS_VERSION"
if [ ! -d "$BUILD_TOOLS_DIR" ]; then
    echo -e "${YELLOW}Build Tools 未找到: $BUILD_TOOLS_DIR${NC}"
    echo ""
    echo "使用 sdkmanager 安装:"
    echo "  $ANDROID_SDK_ROOT/cmdline-tools/latest/bin/sdkmanager \"build-tools;$BUILD_TOOLS_VERSION\""
    echo ""
    exit 1
fi
echo -e "${GREEN}✓ Build Tools 存在${NC}: $BUILD_TOOLS_VERSION"
echo ""

# 4. 包装 x86-64 二进制工具
echo "=== 4. 包装 Build Tools 二进制文件 ==="
wrap_binary() {
    local binary_path="$1"
    local binary_name=$(basename "$binary_path")

    if [ ! -f "$binary_path" ]; then
        return
    fi

    # 检查是否是 x86-64 二进制
    local file_info=$(file "$binary_path" 2>/dev/null || echo "")
    if ! echo "$file_info" | grep -q "ELF 64-bit LSB.*x86-64"; then
        return
    fi

    # 如果已经是包装脚本，跳过
    if [ -f "$binary_path.orig" ]; then
        echo -e "${GREEN}✓${NC} $binary_name 已包装"
        return
    fi

    # 如果第一行是 shebang 且包含 box64，已包装
    if head -1 "$binary_path" 2>/dev/null | grep -q "box64"; then
        echo -e "${GREEN}✓${NC} $binary_name 已包装"
        return
    fi

    # 创建包装
    echo "  包装 $binary_name..."
    mv "$binary_path" "$binary_path.orig"
    cat > "$binary_path" << EOF
#!/bin/bash
box64 "$binary_path.orig" "\$@"
EOF
    chmod +x "$binary_path"
    echo -e "${GREEN}✓${NC} $binary_name 已包装"
}

# 包装关键工具
for tool in aapt2 apksigner zipalign; do
    wrap_binary "$BUILD_TOOLS_DIR/$tool"
done
echo ""

# 5. 检查 Java/JDK
echo "=== 5. 检查 Java 环境 ==="
if ! command -v java &> /dev/null; then
    echo -e "${YELLOW}Java 未安装${NC}"
    echo "在 Termux 中安装:"
    echo "  pkg install openjdk-21"
    exit 1
fi
JAVA_VERSION=$(java -version 2>&1 | head -1)
echo -e "${GREEN}✓ Java 已安装${NC}: $JAVA_VERSION"
echo ""

# 6. 检查 Gradle
echo "=== 6. 检查 Gradle ==="
if ! command -v gradle &> /dev/null; then
    echo -e "${YELLOW}Gradle 未安装${NC}"
    echo "在 Termux 中安装:"
    echo "  pkg install gradle"
    exit 1
fi
GRADLE_VERSION=$(gradle --version 2>/dev/null | grep "Gradle" | head -1)
echo -e "${GREEN}✓ Gradle 已安装${NC}: $GRADLE_VERSION"
echo ""

# 7. 配置 Gradle 镜像源
echo "=== 7. 配置 Gradle Maven 镜像 ==="
GRADLE_INIT="$HOME/.gradle/init.gradle"
mkdir -p "$(dirname "$GRADLE_INIT")"
if [ ! -f "$GRADLE_INIT" ]; then
    echo "创建 Gradle 镜像配置..."
    cat > "$GRADLE_INIT" << 'GRADLE_EOF'
allprojects {
    repositories {
        def ALIYUN_REPOSITORY_URL = 'https://maven.aliyun.com/repository/public'
        def ALIYUN_GOOGLE_URL = 'https://maven.aliyun.com/repository/google'
        def ALIYUN_GRADLE_PLUGIN_URL = 'https://maven.aliyun.com/repository/gradle-plugin'

        all { ArtifactRepository repo ->
            if (repo instanceof MavenArtifactRepository) {
                def url = repo.url.toString()
                if (url.startsWith('https://repo1.maven.org/maven2/') ||
                    url.startsWith('https://jcenter.bintray.com/')) {
                    remove repo
                }
            }
        }

        maven { url ALIYUN_REPOSITORY_URL }
        maven { url ALIYUN_GOOGLE_URL }
        maven { url ALIYUN_GRADLE_PLUGIN_URL }
        google()
        mavenCentral()
    }
}

buildscript {
    repositories {
        def ALIYUN_REPOSITORY_URL = 'https://maven.aliyun.com/repository/public'
        def ALIYUN_GOOGLE_URL = 'https://maven.aliyun.com/repository/google'
        def ALIYUN_GRADLE_PLUGIN_URL = 'https://maven.aliyun.com/repository/gradle-plugin'

        maven { url ALIYUN_REPOSITORY_URL }
        maven { url ALIYUN_GOOGLE_URL }
        maven { url ALIYUN_GRADLE_PLUGIN_URL }
        google()
        mavenCentral()
    }
}
GRADLE_EOF
    echo -e "${GREEN}✓${NC} 创建 $GRADLE_INIT"
else
    echo -e "${GREEN}✓${NC} Gradle 配置已存在"
fi
echo ""

# 8. 检查项目 local.properties
echo "=== 8. 检查项目配置 ==="
if [ -n "$1" ] && [ -d "$1" ]; then
    PROJECT_DIR="$1"
    LOCAL_PROPS="$PROJECT_DIR/local.properties"
    if [ ! -f "$LOCAL_PROPS" ]; then
        echo "创建 $LOCAL_PROPS..."
        echo "sdk.dir=$ANDROID_SDK_ROOT" > "$LOCAL_PROPS"
        echo -e "${GREEN}✓${NC} 创建 $LOCAL_PROPS"
    else
        echo -e "${GREEN}✓${NC} $LOCAL_PROPS 已存在"
    fi
fi
echo ""

# 总结
echo "=== 环境设置完成 ==="
echo ""
echo "关键路径:"
echo "  Android SDK:     $ANDROID_SDK_ROOT"
echo "  Build Tools:     $BUILD_TOOLS_DIR"
echo "  Gradle Config:   $GRADLE_INIT"
echo "  box64:           $(which box64)"
echo ""
echo "注意事项:"
echo "  - Build Tools 二进制已使用 box64 包装"
echo "  - 每次更新 build-tools 后需要重新包装"
echo "  - Gradle 缓存中的 AAPT2 也需要在首次使用时包装（build.sh 会处理）"
echo ""
echo "下一步:"
echo "  cd <项目目录>"
echo "  ./build.sh"
echo ""
