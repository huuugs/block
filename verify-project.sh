#!/bin/bash
# Project Verification Script for Block Eater

echo "=== Block Eater Project Verification ==="
echo ""

# Colors
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m'

check_file() {
    if [ -f "$1" ]; then
        echo -e "${GREEN}✓${NC} $1"
        return 0
    else
        echo -e "${RED}✗${NC} $1 (missing)"
        return 1
    fi
}

check_dir() {
    if [ -d "$1" ]; then
        echo -e "${GREEN}✓${NC} $1/"
        return 0
    else
        echo -e "${RED}✗${NC} $1/ (missing)"
        return 1
    fi
}

errors=0

echo "1. Checking Gradle configuration..."
check_file "build.gradle.kts" || ((errors++))
check_file "settings.gradle.kts" || ((errors++))
check_file "gradle.properties" || ((errors++))
check_file "gradlew" || ((errors++))
check_dir "gradle/wrapper" || ((errors++))
echo ""

echo "2. Checking App module..."
check_file "app/build.gradle.kts" || ((errors++))
check_dir "app/src/main" || ((errors++))
echo ""

echo "3. Checking C++ source files..."
check_file "app/src/main/cpp/main.cpp" || ((errors++))
check_file "app/src/main/cpp/game.h" || ((errors++))
check_file "app/src/main/cpp/game.cpp" || ((errors++))
check_file "app/src/main/cpp/player.h" || ((errors++))
check_file "app/src/main/cpp/player.cpp" || ((errors++))
check_file "app/src/main/cpp/enemy.h" || ((errors++))
check_file "app/src/main/cpp/enemy.cpp" || ((errors++))
check_file "app/src/main/cpp/particles.h" || ((errors++))
check_file "app/src/main/cpp/particles.cpp" || ((errors++))
check_file "app/src/main/cpp/ui.h" || ((errors++))
check_file "app/src/main/cpp/ui.cpp" || ((errors++))
check_file "app/src/main/cpp/audio.h" || ((errors++))
check_file "app/src/main/cpp/audio.cpp" || ((errors++))
check_file "app/src/main/cpp/modes.h" || ((errors++))
check_file "app/src/main/cpp/modes.cpp" || ((errors++))
check_file "app/src/main/cpp/controls.h" || ((errors++))
check_file "app/src/main/cpp/controls.cpp" || ((errors++))
check_file "app/src/main/cpp/assets.h" || ((errors++))
check_file "app/src/main/cpp/assets.cpp" || ((errors++))
check_file "app/src/main/cpp/CMakeLists.txt" || ((errors++))
echo ""

echo "4. Checking Java/Kotlin files..."
check_file "app/src/main/java/com/blockeater/MainActivity.java" || ((errors++))
echo ""

echo "5. Checking Android resources..."
check_file "app/src/main/AndroidManifest.xml" || ((errors++))
check_file "app/src/main/res/values/strings.xml" || ((errors++))
echo ""

echo "6. Checking build tools..."
check_file "build-game.sh" || ((errors++))
[ -x "build-game.sh" ] && echo -e "${GREEN}✓${NC} build-game.sh is executable" || echo -e "${YELLOW}!${NC} build-game.sh not executable (run: chmod +x build-game.sh)"
echo ""

echo "7. Checking for required tools..."
command -v cmake &> /dev/null && echo -e "${GREEN}✓${NC} cmake" || echo -e "${RED}✗${NC} cmake (required)"
command -v gradle &> /dev/null && echo -e "${GREEN}✓${NC} gradle" || echo -e "${YELLOW}!${NC} gradle (will use wrapper)"
command -v git &> /dev/null && echo -e "${GREEN}✓${NC} git" || echo -e "${YELLOW}!${NC} git (needed for raylib download)"
echo ""

echo "8. Source file statistics..."
header_count=$(find app/src/main/cpp -name "*.h" 2>/dev/null | wc -l)
source_count=$(find app/src/main/cpp -name "*.cpp" 2>/dev/null | wc -l)
echo "   Header files: $header_count"
echo "   Source files: $source_count"
echo "   Total: $((header_count + source_count))"
echo ""

if [ $errors -eq 0 ]; then
    echo -e "${GREEN}=== All checks passed! ===${NC}"
    echo ""
    echo "Next steps:"
    echo "  1. Run: ./build-game.sh setup"
    echo "  2. Run: ./build-game.sh build"
    echo "  3. Run: ./build-game.sh install"
    echo ""
    echo "Or all at once:"
    echo "  ./build-game.sh all"
    exit 0
else
    echo -e "${RED}=== $errors error(s) found ===${NC}"
    exit 1
fi
