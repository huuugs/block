#!/bin/bash
#
# 一键安装依赖并编译 Android APK
# 用于在全新的 Termux/PRoot-Distro (ARM64) 环境中快速部署构建环境
#
# 使用方法:
#   bash install-and-build.sh
#

set -e

# 颜色输出
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
BLUE='\033[0;34m'
NC='\033[0m'

# 全局变量
ANDROID_SDK_ROOT="/opt/android-sdk"
BUILD_TOOLS_VERSION="34.0.0"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$SCRIPT_DIR"
GRADLE_USER_HOME="${GRADLE_USER_HOME:-$HOME/.gradle}"

# Termux 检测
is_termux() {
    [ -d "/data/data/com.termux" ] 2>/dev/null
}

# 安全的包安装
safe_install_pkg() {
    local pkg="$1"

    if is_termux; then
        if ! pkg list-installed 2>/dev/null | grep -q "^$pkg/"; then
            echo "  安装 $pkg..."
            pkg install -y "$pkg" || {
                echo -e "${YELLOW}警告: $pkg 安装失败，请手动安装${NC}"
                return 1
            }
        else
            echo -e "${GREEN}✓${NC} $pkg 已安装"
        fi
    else
        echo -e "${YELLOW}  [跳过] $pkg (非 Termux 环境)${NC}"
    fi
}

# 检查命令存在
check_command() {
    local cmd="$1"
    local pkg="$2"

    if command -v "$cmd" >/dev/null 2>&1; then
        return 0
    fi

    echo -e "${YELLOW}警告: $cmd 未找到${NC}"
    if [ -n "$pkg" ]; then
        echo "  请安装: $pkg"
    fi
    return 1
}

# 包装 x86-64 二进制文件
wrap_x86_binary() {
    local binary_path="$1"
    local binary_name

    [ ! -f "$binary_path" ] && return 1

    binary_name=$(basename "$binary_path")

    # 已包装检查
    if [ -f "$binary_path.orig" ]; then
        return 0
    fi

    # 检查是否是脚本（已包装）
    if head -1 "$binary_path" 2>/dev/null | grep -q "box64"; then
        return 0
    fi

    # 检查是否是 x86-64
    local file_info
    file_info=$(file "$binary_path" 2>/dev/null || echo "")
    if ! echo "$file_info" | grep -q "ELF 64-bit LSB.*x86-64"; then
        return 1
    fi

    # 执行包装
    mv "$binary_path" "$binary_path.orig"
    cat > "$binary_path" << EOF
#!/bin/bash
box64 "$binary_path.orig" "\$@"
EOF
    chmod +x "$binary_path"
    echo -e "${GREEN}✓${NC} 包装: $binary_name"
    return 0
}

# 打印横幅
print_banner() {
    echo -e "${GREEN}========================================${NC}"
    echo -e "${GREEN}  Android 构建环境一键部署${NC}"
    echo -e "${GREEN}========================================${NC}"
    echo ""
    echo "配置:"
    echo "  项目目录: $PROJECT_DIR"
    echo "  Android SDK: $ANDROID_SDK_ROOT"
    echo "  Gradle 用户目录: $GRADLE_USER_HOME"
    echo ""
}

# 步骤1: 检查系统依赖
step1_check_dependencies() {
    echo -e "${GREEN}=== [1/7] 检查系统依赖 ===${NC}"

    local all_ok=true

    # box64
    if check_command "box64"; then
        echo -e "${GREEN}✓${NC} box64: $(box64 --version 2>&1 | head -1)"
    else
        echo -e "${RED}✗${NC} box64 未安装"
        all_ok=false
    fi

    # Java
    if check_command "java"; then
        local java_version
        java_version=$(java -version 2>&1 | head -1)
        echo -e "${GREEN}✓${NC} $java_version"
    else
        echo -e "${RED}✗${NC} Java 未安装"
        all_ok=false
    fi

    # Gradle
    if check_command "gradle"; then
        echo -e "${GREEN}✓${NC} Gradle: $(gradle --version 2>&1 | grep "Gradle" | head -1)"
    else
        echo -e "${RED}✗${NC} Gradle 未安装"
        all_ok=false
    fi

    if [ "$all_ok" = "false" ]; then
        echo ""
        if is_termux; then
            echo -e "${YELLOW}正在尝试安装缺失的包...${NC}"
            echo ""
            safe_install_pkg "box64"
            safe_install_pkg "openjdk-21"
            safe_install_pkg "gradle"
        else
            echo -e "${RED}错误: 缺少必需依赖${NC}"
            echo ""
            echo "请手动安装:"
            echo "  - box64: https://github.com/ptitSeb/box64"
            echo "  - Java JDK 21+"
            echo "  - Gradle 8.x+"
            exit 1
        fi
    fi

    echo ""
}

# 步骤2: 检查 Android SDK
step2_check_sdk() {
    echo -e "${GREEN}=== [2/7] 检查 Android SDK ===${NC}"

    if [ ! -d "$ANDROID_SDK_ROOT" ]; then
        echo -e "${RED}错误: Android SDK 未找到: $ANDROID_SDK_ROOT${NC}"
        echo ""
        echo "请先安装 Android SDK:"
        echo ""
        echo "  mkdir -p $ANDROID_SDK_ROOT"
        echo "  cd $ANDROID_SDK_ROOT"
        echo "  wget https://dl.google.com/android/repository/commandlinetools-linux-11076708_latest.zip"
        echo "  mkdir -p cmdline-tools/latest"
        echo "  unzip commandlinetools-linux-*.zip -d cmdline-tools"
        echo "  mv cmdline-tools/cmdline-tools/* cmdline-tools/latest/"
        echo "  sdkmanager \"build-tools;$BUILD_TOOLS_VERSION\" \"platforms;android-34\""
        exit 1
    fi
    echo -e "${GREEN}✓${NC} Android SDK: $ANDROID_SDK_ROOT"

    local build_tools_dir="$ANDROID_SDK_ROOT/build-tools/$BUILD_TOOLS_VERSION"
    if [ ! -d "$build_tools_dir" ]; then
        echo -e "${RED}错误: Build Tools $BUILD_TOOLS_VERSION 未找到${NC}"
        echo ""
        echo "使用 sdkmanager 安装:"
        echo "  sdkmanager \"build-tools;$BUILD_TOOLS_VERSION\""
        exit 1
    fi
    echo -e "${GREEN}✓${NC} Build Tools: $BUILD_TOOLS_VERSION"

    echo ""
}

# 步骤3: 包装 Build Tools
step3_wrap_build_tools() {
    echo -e "${GREEN}=== [3/7] 包装 Build Tools 二进制 ===${NC}"

    local build_tools_dir="$ANDROID_SDK_ROOT/build-tools/$BUILD_TOOLS_VERSION"
    local tool wrapped=false

    for tool in aapt2 apksigner zipalign; do
        if wrap_x86_binary "$build_tools_dir/$tool"; then
            wrapped=true
        fi
    done

    if [ "$wrapped" = "false" ]; then
        echo -e "${GREEN}✓${NC} Build Tools 已包装或无需包装"
    fi

    echo ""
}

# 步骤4: 配置 Gradle 镜像
step4_configure_gradle() {
    echo -e "${GREEN}=== [4/7] 配置 Gradle Maven 镜像 ===${NC}"

    local gradle_init="$GRADLE_USER_HOME/init.gradle"
    mkdir -p "$(dirname "$gradle_init")"

    if [ ! -f "$gradle_init" ]; then
        echo "创建镜像配置..."
        cat > "$gradle_init" << 'GRADLE_EOF'
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
        echo -e "${GREEN}✓${NC} 创建 $gradle_init"
    else
        echo -e "${GREEN}✓${NC} 配置已存在"
    fi

    echo ""
}

# 步骤5: 创建项目配置
step5_project_config() {
    echo -e "${GREEN}=== [5/7] 创建项目配置 ===${NC}"

    local local_props="$PROJECT_DIR/local.properties"

    if [ ! -f "$local_props" ]; then
        echo "创建 $local_props..."
        echo "sdk.dir=$ANDROID_SDK_ROOT" > "$local_props"
        echo -e "${GREEN}✓${NC} 创建 $local_props"
    else
        echo -e "${GREEN}✓${NC} $local_props 已存在"
    fi

    echo ""
}

# 步骤6: 准备 Gradle 缓存
step6_prepare_gradle_cache() {
    echo -e "${GREEN}=== [6/7] 准备 Gradle 缓存 ===${NC}"

    echo "停止 Gradle daemon..."
    gradle --stop 2>/dev/null || true

    # 包装现有的 AAPT2
    local wrapped=false
    local gradle_caches

    # 查找 Gradle 缓存目录
    if is_termux && [ -d "/data/data/com.termux/files/home/.gradle/caches" ]; then
        gradle_caches="/data/data/com.termux/files/home/.gradle/caches"
    elif [ -d "$HOME/.gradle/caches" ]; then
        gradle_caches="$HOME/.gradle/caches"
    fi

    if [ -n "$gradle_caches" ]; then
        while IFS= read -r -d '' dir; do
            if wrap_x86_binary "$dir/aapt2"; then
                wrapped=true
            fi
        done < <(find "$gradle_caches" -type d -name "aapt2-*linux" -print0 2>/dev/null)
    fi

    if [ "$wrapped" = "false" ]; then
        echo "  (首次构建时会自动包装 AAPT2)"
    fi

    echo ""
}

# 包装所有 Gradle 缓存中的 AAPT2
wrap_all_gradle_aapt2() {
    local gradle_caches wrapped=false

    if is_termux && [ -d "/data/data/com.termux/files/home/.gradle/caches" ]; then
        gradle_caches="/data/data/com.termux/files/home/.gradle/caches"
    elif [ -d "$HOME/.gradle/caches" ]; then
        gradle_caches="$HOME/.gradle/caches"
    fi

    if [ -n "$gradle_caches" ]; then
        while IFS= read -r -d '' dir; do
            if [ -d "$dir" ] && [ -f "$dir/aapt2" ]; then
                if wrap_x86_binary "$dir/aapt2"; then
                    wrapped=true
                fi
            fi
        done < <(find "$gradle_caches" -type d -name "aapt2-*linux" -print0 2>/dev/null)
    fi

    return 0
}

# 步骤7: 执行构建
step7_build() {
    echo -e "${GREEN}=== [7/7] 执行构建 ===${NC}"
    echo ""

    # 清理
    echo "清理..."
    gradle clean --quiet 2>/dev/null || true

    # 首次构建尝试
    echo "首次构建尝试..."
    if gradle assembleDebug --quiet 2>/dev/null; then
        build_success=true
    else
        build_success=false
        echo ""
        echo -e "${YELLOW}首次构建失败，尝试包装 AAPT2 后重试...${NC}"

        # 包装所有 AAPT2
        wrap_all_gradle_aapt2

        echo ""
        echo "重新构建..."
        if gradle assembleDebug; then
            build_success=true
        else
            build_success=false
        fi
    fi

    echo ""

    # 检查结果
    if [ "$build_success" = "true" ] && [ -f "app/build/outputs/apk/debug/app-debug.apk" ]; then
        cp app/build/outputs/apk/debug/app-debug.apk "$PROJECT_DIR/CalculatorApp.apk"
        echo -e "${GREEN}=== 编译成功！ ===${NC}"
        echo ""
        echo "APK 信息:"
        ls -lh "$PROJECT_DIR/CalculatorApp.apk"
        echo ""
        echo "安装命令:"
        echo "  adb install CalculatorApp.apk"
        return 0
    else
        echo -e "${RED}=== 编译失败 ===${NC}"
        echo ""
        echo "请检查错误信息，常见问题:"
        echo "  1. Android SDK 路径不正确"
        echo "  2. 网络连接问题（依赖下载失败）"
        echo "  3. box64 版本不兼容"
        return 1
    fi
}

# 主函数
main() {
    cd "$PROJECT_DIR"

    print_banner
    step1_check_dependencies
    step2_check_sdk
    step3_wrap_build_tools
    step4_configure_gradle
    step5_project_config
    step6_prepare_gradle_cache
    step7_build
}

# 运行
main "$@"
