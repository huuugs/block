# 中文字体设置指南 - Chinese Font Setup Guide

## 问题说明 / Problem Description

默认情况下，raylib只包含ASCII字符字体（95个标准字符），**不支持中文显示**。如果不安装中文字体，游戏中所有中文会显示为**方块、问号或空白**。

By default, raylib only includes ASCII fonts (95 standard characters) and **does not support Chinese**. Without Chinese fonts, all Chinese text will appear as **squares, question marks, or blank spaces**.

---

## 解决方案 / Solution

### 方案1: 自动下载脚本 / Auto Download Script

```bash
chmod +x download_fonts.sh
./download_fonts.sh
```

按照提示选择要下载的字体。

### 方案2: 手动下载 / Manual Download

#### 推荐字体1: 凤凰点阵体 (Vonwaon Bitmap Font) ⭐推荐

**特点 / Features:**
- 像素风格，完美适合游戏 / Pixel style, perfect for games
- 文件小 (~1MB) / Small file size
- 包含6763个常用汉字 / 6763 commonly used Chinese characters
- 免费商用 (CC0 1.0) / Free for commercial use

**下载地址 / Download:**
https://timothyqiu.itch.io/vonwaon-bitmap

**安装步骤 / Installation:**
1. 访问链接并下载 / Visit link and download
2. 解压文件 / Extract the file
3. 复制字体到项目目录 / Copy font to project:
   ```bash
   cp vonwaon_bitmap_12px.ttf app/src/main/cpp/fonts/vonwaon_pixel_12px.ttf
   ```

#### 推荐字体2: 思源黑体 (Source Han Sans)

**特点 / Features:**
- 专业品质 / Professional quality
- 字符集完整 / Complete character set
- 多种字重 / Multiple weights
- 文件较大 (~5MB) / Larger file size

**下载地址 / Download:**
https://github.com/adobe-fonts/source-han-sans/releases

**推荐下载 / Recommended:**
`SourceHanSansCN-Regular.otf` (Subset OTF版本，文件更小)

**安装步骤 / Installation:**
```bash
cp SourceHanSansCN-Regular.otf app/src/main/cpp/fonts/
```

---

## 项目字体目录结构 / Project Font Directory Structure

```
app/src/main/cpp/
├── fonts/
│   ├── vonwaon_pixel_12px.ttf    (优先级1 - 游戏专用)
│   └── SourceHanSansCN-Regular.otf (优先级2 - 备用)
```

---

## 字体加载机制 / Font Loading Mechanism

代码会自动按以下顺序尝试加载字体：

The code automatically tries to load fonts in this order:

1. `fonts/vonwaon_pixel_12px.ttf` (游戏专用字体)
2. `fonts/SourceHanSansCN-Regular.otf` (思源黑体)
3. 回退到raylib默认字体 (无中文支持)

### Android Assets 路径

在Android平台上，字体文件需要从APK的assets中加载。`build.gradle.kts`已配置自动将`fonts/`目录打包到APK。

On Android, font files are loaded from APK assets. `build.gradle.kts` is configured to automatically package the `fonts/` directory into the APK.

---

## 验证字体加载 / Verify Font Loading

启动游戏后，查看logcat输出：

```bash
adb logcat | grep -E "Font|UIManager"
```

**成功加载 / Loaded successfully:**
```
Font loaded from fonts/vonwaon_pixel_12px.ttf
UIManager: Using custom font for Chinese support
```

**加载失败 / Failed to load:**
```
Using default font (no Chinese support)
UIManager: Using default font (limited Chinese support)
```

---

## 在GitHub Actions中使用字体 / Using Fonts in GitHub Actions

如果你想在GitHub Actions自动构建中包含字体：

### 方法1: 提交字体到仓库 (推荐小字体)

```bash
# 如果你选择凤凰点阵体（文件小）
git add app/src/main/cpp/fonts/vonwaon_pixel_12px.ttf
git commit -m "Add Chinese font support"
git push
```

### 方法2: 在工作流中下载

在`.github/workflows/build.yml`中添加下载步骤：

```yaml
- name: Download Chinese Font
  run: |
    mkdir -p app/src/main/cpp/fonts
    curl -L -o app/src/main/cpp/fonts/vonwaon_pixel_12px.ttf \
      "https://github.com/your-repo/fonts/raw/main/vonwaon_pixel_12px.ttf"
```

**注意**: 确保字体文件的许可证允许这样做。
**Note**: Ensure the font license permits this.

---

## 字体许可证 / Font Licenses

| 字体 / Font | 许可证 / License | 商用 / Commercial Use |
|------------|-----------------|---------------------|
| 凤凰点阵体 Vonwaon | CC0 1.0 | ✅ 允许 Allowed |
| 思源黑体 Source Han Sans | SIL Open Font 1.1 | ✅ 允许 Allowed |

---

## 故障排除 / Troubleshooting

### 中文显示为方块
**原因**: 字体未加载 / Font not loaded
**解决**: 
- 检查字体文件是否存在 / Check if font file exists
- 检查logcat中的字体加载日志 / Check logcat for font loading logs
- 确认字体文件名正确 / Verify font filename is correct

### 字体加载成功但中文不显示
**原因**: 字体文件可能损坏或不完整 / Font file may be corrupted
**解决**: 重新下载字体文件 / Re-download the font file

### APK文件过大
**原因**: 思源黑体文件较大 (~5MB) / Source Han Sans is large
**解决**: 使用凤凰点阵体 (~1MB) / Use Vonwaon Bitmap instead

---

## 快速检查清单 / Quick Checklist

- [ ] 字体文件已下载到 `app/src/main/cpp/fonts/`
- [ ] 字体文件名正确：`vonwaon_pixel_12px.ttf` 或 `SourceHanSansCN-Regular.otf`
- [ ] 重新构建APK后测试中文显示
- [ ] 检查logcat确认字体加载成功

---

## 技术支持 / Support

如有问题，请查看：
- GitHub Issues: https://github.com/huuugs/block/issues
- 构建日志: GitHub Actions → 最新构建 → Logs
