#!/bin/bash
# 字体下载脚本 - Font Download Script
# 下载支持中文的字体文件

set -e

FONT_DIR="app/src/main/cpp/fonts"
mkdir -p "$FONT_DIR"

echo "========================================="
echo "  Block Eater 字体下载工具"
echo "  Font Download Tool"
echo "========================================="
echo ""

# 检查是否已有字体文件
if [ -f "$FONT_DIR/vonwaon_pixel_12px.ttf" ] || [ -f "$FONT_DIR/SourceHanSansCN-Regular.otf" ]; then
    echo "✓ 字体文件已存在 / Font files already exist:"
    ls -lh "$FONT_DIR/"
    echo ""
    echo "如需重新下载，请先删除现有字体文件。"
    echo "To re-download, please delete existing font files first."
    exit 0
fi

echo "请选择要下载的字体 / Please select font to download:"
echo ""
echo "1) 凤凰点阵体 (Vonwaon Bitmap) - 推荐 for games"
echo "   - 像素风格，文件小 (~1MB)"
echo "   - 适合游戏显示"
echo "   - 包含6763个常用汉字"
echo ""
echo "2) 思源黑体 (Source Han Sans) - 高质量"
echo "   - 专业字体，质量高 (~5MB)"
echo "   - 字符集完整"
echo "   - 适合各种场景"
echo ""
echo "3) 两个都下载 (Download both)"
echo ""
echo "0) 退出 (Exit)"
echo ""

read -p "请输入选项 / Enter option [0-3]: " choice

case $choice in
    1)
        echo ""
        echo "下载凤凰点阵体... / Downloading Vonwaon Bitmap..."
        echo ""
        echo "请手动下载: / Please download manually:"
        echo "https://timothyqiu.itch.io/vonwaon-bitmap"
        echo ""
        echo "下载步骤 / Download steps:"
        echo "1. 访问上述链接 / Visit the link above"
        echo "2. 点击 'Download' 按钮 / Click 'Download' button"
        echo "3. 解压下载的文件 / Extract the downloaded file"
        echo "4. 将字体文件复制到: / Copy font file to:"
        echo "   $FONT_DIR/vonwaon_pixel_12px.ttf"
        echo ""
        ;;
    2)
        echo ""
        echo "下载思源黑体... / Downloading Source Han Sans..."
        echo ""
        echo "请手动下载: / Please download manually:"
        echo "https://github.com/adobe-fonts/source-han-sans/releases"
        echo ""
        echo "推荐下载 / Recommended download:"
        echo "SourceHanSansCN-Regular.otf (Subset OTF)"
        echo ""
        echo "下载后复制到: / Copy to after download:"
        echo "   $FONT_DIR/SourceHanSansCN-Regular.otf"
        echo ""
        ;;
    3)
        echo ""
        echo "下载两个字体... / Downloading both fonts..."
        echo ""
        echo "=== 凤凰点阵体 Vonwaon Bitmap ==="
        echo "链接/Link: https://timothyqiu.itch.io/vonwaon-bitmap"
        echo "文件名/Filename: vonwaon_pixel_12px.ttf"
        echo ""
        echo "=== 思源黑体 Source Han Sans ==="
        echo "链接/Link: https://github.com/adobe-fonts/source-han-sans/releases"
        echo "文件名/Filename: SourceHanSansCN-Regular.otf"
        echo ""
        echo "复制到 / Copy to: $FONT_DIR/"
        echo ""
        ;;
    0)
        echo "退出 / Exit"
        exit 0
        ;;
    *)
        echo "无效选项 / Invalid option"
        exit 1
        ;;
esac

echo "========================================="
echo "字体安装说明 / Font Installation Guide"
echo "========================================="
echo ""
echo "当前字体目录 / Current font directory:"
ls -la "$FONT_DIR/"
echo ""
echo "字体下载后，重新构建APK即可使用中文显示。"
echo "After downloading fonts, rebuild APK to enable Chinese display."
echo ""
