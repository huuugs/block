# 字体设置指南 - Font Setup Guide

## 中文乱码问题解决方案

要修复中文显示乱码的问题，需要下载支持中文的字体文件。

### 推荐字体

#### 方案 1: 凤凰点阵体 (Vonwaon Bitmap Font) - 推荐用于游戏
一个专为游戏开发设计的免费中文像素字体，文件较小，适合移动端。

**下载步骤：**
1. 访问: https://timothyqiu.itch.io/vonwaon-bitmap
2. 点击 "Download" 下载字体包
3. 解压下载的文件
4. 将字体文件复制到项目目录：
   ```bash
   cp path/to/vonwaon_bitmap_12px.ttf app/src/main/cpp/fonts/vonwaon_pixel_12px.ttf
   ```

#### 方案 2: 思源黑体 (Source Han Sans)
Adobe和Google联合开发的开源字体，字符集完整但文件较大。

**下载步骤：**
1. 访问: https://github.com/adobe-fonts/source-han-sans/tree/release
2. 下载Subset OTF版本（文件更小）
3. 将字体文件复制到项目目录：
   ```bash
   cp SourceHanSansCN-Regular.otf app/src/main/cpp/fonts/
   ```

### 字体文件位置

确保字体文件放置在以下目录结构：
```
app/src/main/cpp/
├── fonts/
│   ├── vonwaon_pixel_12px.ttf (推荐)
│   └── SourceHanSansCN-Regular.otf (备用)
```

### 自动加载机制

代码已修改为自动检测并加载字体：
1. 首先尝试加载 `fonts/vonwaon_pixel_12px.ttf`
2. 如果不存在，尝试加载 `fonts/SourceHanSansCN-Regular.otf`
3. 如果都不存在，回退到raylib默认字体（不支持中文）

### 验证字体加载

启动游戏后，在logcat中查看以下日志确认字体加载成功：
```
Font loaded from fonts/vonwaon_pixel_12px.ttf
```

如果看到：
```
Using default font (no Chinese support)
```
说明字体文件未找到，需要检查文件路径。

### 字体打包

字体文件通过 `app/build.gradle.kts` 配置自动打包到APK的assets目录，无需额外配置。

### 字体授权

- **凤凰点阵体**: CC0 1.0 公有领域
- **思源黑体**: SIL Open Font License 1.1

两者都可以免费用于商业项目。
