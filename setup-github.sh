#!/bin/bash
# Git 初始化和推送脚本

set -e

GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'

echo "=== Block Eater - GitHub 推送脚本 ==="
echo ""

PROJECT_DIR="/root/block"
cd "$PROJECT_DIR"

# Check if already a git repo
if [ -d ".git" ]; then
    echo -e "${YELLOW}Git repository already initialized${NC}"
else
    echo -e "${YELLOW}[1/4] Initializing Git repository...${NC}"
    git init
    echo "  ✓ Initialized"
fi

# Configure git if not configured
if ! git config user.name >/dev/null 2>&1; then
    echo ""
    echo -e "${YELLOW}[2/4] Configure Git...${NC}"
    echo "Enter your name:"
    read -r GIT_NAME
    echo "Enter your email:"
    read -r GIT_EMAIL
    git config user.name "$GIT_NAME"
    git config user.email "$GIT_EMAIL"
    echo "  ✓ Configured"
fi

# Create .gitignore if not exists
if [ ! -f ".gitignore" ]; then
    cat > .gitignore << 'EOF'
# Build outputs
app/build/
app/.cxx/
.gradle/
local.properties
*.apk
*.aab

# NDK
android-ndk/
ndk/
raylib/build-*/

# Temporary files
*.tmp
*.log
*.swp
*~

# OS
.DS_Store
Thumbs.db

# IDE
.idea/
.vscode/
*.iml
EOF
    echo "  ✓ Created .gitignore"
fi

echo ""
echo -e "${YELLOW}[3/4] Staging files...${NC}"
git add .
echo "  ✓ Files staged"

echo ""
echo -e "${YELLOW}[4/4] Creating commit...${NC}"
git commit -m "Initial Block Eater game implementation

- Complete Android game with raylib
- 3 game modes (Endless, Level, Time Challenge)
- 6-level player progression system
- 4 enemy types with different AI
- Virtual joystick and touch controls
- Procedural 8-bit audio generation
- Retro pixel art style
- ~3000 lines of C++17 code
"
echo "  ✓ Committed"

echo ""
echo -e "${GREEN}=== Git Setup Complete ===${NC}"
echo ""
echo "Next steps to push to GitHub:"
echo ""
echo "1. Create a new repository on GitHub: https://github.com/new"
echo "   - Name: block-eater (or your choice)"
echo "   - Don't initialize with README (we have one)"
echo ""
echo "2. Add remote and push:"
echo "   git remote add origin https://github.com/YOUR_USERNAME/YOUR_REPO.git"
echo "   git branch -M main"
echo "   git push -u origin main"
echo ""
echo "3. After pushing, go to:"
echo "   https://github.com/YOUR_USERNAME/YOUR_REPO/actions"
echo ""
echo "   The GitHub Action will automatically build the APK!"
echo ""
echo "4. Download the APK from Actions > Artifacts"
echo ""

# Ask if user wants to push now
read -p "Do you want to add remote URL now? (y/n): " add_remote
if [ "$add_remote" = "y" ]; then
    echo ""
    echo "Enter your GitHub username:"
    read -r username
    echo "Enter your repository name:"
    read -r repo_name

    git remote add origin "https://github.com/$username/$repo_name.git"
    git branch -M main

    echo ""
    echo "Remote added. Ready to push!"
    echo ""
    echo "Run this command to push:"
    echo "  git push -u origin main"
    echo ""
    echo "Note: First push may require GitHub authentication:"
    echo "  - Username: your GitHub username"
    echo "  - Password: use a Personal Access Token (not your password)"
    echo "  - Create token at: https://github.com/settings/tokens"
fi
