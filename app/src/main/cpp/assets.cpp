#include "assets.h"
#include <cmath>
#include <cstdlib>

namespace BlockEater {

AssetManager::AssetManager() : pixelFont{0}, smallFont{0} {
}

AssetManager::~AssetManager() {
    UnloadExternalFonts();
}

void AssetManager::init() {
    LoadFonts();
}

void AssetManager::shutdown() {
    UnloadExternalFonts();
}

void AssetManager::LoadFonts() {
    TraceLog(LOG_INFO, "=== LoadFonts() START [TRY ZPIX FIRST] ===");

    // Try Zpix FIRST - it's smaller and more likely to work on Android
    bool loaded = false;

    TraceLog(LOG_INFO, "Attempting to load Zpix font first...");
    if (!loaded) {
        loaded = LoadExternalFont("fonts/zpix.ttf", 20);
    }

    // Fallback: Source Han Sans removed to reduce APK size
    // if (!loaded) {
    //     TraceLog(LOG_INFO, "Zpix failed, trying Source Han Sans...");
    //     loaded = LoadExternalFont("fonts/SourceHanSansCN-Regular.otf", 18);
    // }

    // Fallback: Vonwaon pixel font
    if (!loaded) {
        TraceLog(LOG_INFO, "Source Han Sans failed, trying vonwaon...");
        loaded = LoadExternalFont("fonts/vonwaon_pixel_12px.ttf", 12);
    }

    // Fallback: use default raylib font if external fonts not available
    if (!loaded) {
        TraceLog(LOG_ERROR, "All external fonts failed!");
        pixelFont = GetFontDefault();
        smallFont = GetFontDefault();
        TraceLog(LOG_WARNING, "Using default font (no Chinese support)");
    } else {
        TraceLog(LOG_INFO, TextFormat("FINAL: Font loaded with %d glyphs", pixelFont.glyphCount));
    }

    TraceLog(LOG_INFO, "=== LoadFonts() END ===");
}

Image AssetManager::CreatePixelBlockImage(Color color, int size) {
    Image img = {
        .data = malloc(size * size * 4),
        .width = size,
        .height = size,
        .mipmaps = 1,
        .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8
    };

    // Create pixel block with slight pixelation effect
    for (int y = 0; y < size; y++) {
        for (int x = 0; x < size; x++) {
            // Add pixelated edge effect
            int edgeSize = 2;
            bool isEdge = (x < edgeSize || x >= size - edgeSize ||
                          y < edgeSize || y >= size - edgeSize);

            Color pixelColor = color;
            if (isEdge) {
                pixelColor.r = (unsigned char)(color.r * 0.7f);
                pixelColor.g = (unsigned char)(color.g * 0.7f);
                pixelColor.b = (unsigned char)(color.b * 0.7f);
            }

            // Add highlight
            if (x < size / 3 && y < size / 3) {
                pixelColor.r = (unsigned char)fmin(255, color.r + 40);
                pixelColor.g = (unsigned char)fmin(255, color.g + 40);
                pixelColor.b = (unsigned char)fmin(255, color.b + 40);
            }

            int index = (y * size + x) * 4;
            ((unsigned char*)img.data)[index] = pixelColor.r;
            ((unsigned char*)img.data)[index + 1] = pixelColor.g;
            ((unsigned char*)img.data)[index + 2] = pixelColor.b;
            ((unsigned char*)img.data)[index + 3] = pixelColor.a;
        }
    }

    return img;
}

Texture2D AssetManager::GeneratePixelBlock(Color color, int size) {
    Image img = CreatePixelBlockImage(color, size);
    Texture2D tex = LoadTextureFromImage(img);
    UnloadImage(img);
    return tex;
}

Image AssetManager::CreateGridImage(int cellSize, Color gridColor, Color bgColor) {
    int width = SCREEN_WIDTH;
    int height = SCREEN_HEIGHT;

    Image img = {
        .data = malloc(width * height * 4),
        .width = width,
        .height = height,
        .mipmaps = 1,
        .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8
    };

    // Fill with background color
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            bool isGridLine = (x % cellSize == 0) || (y % cellSize == 0);

            int index = (y * width + x) * 4;
            if (isGridLine) {
                ((unsigned char*)img.data)[index] = gridColor.r;
                ((unsigned char*)img.data)[index + 1] = gridColor.g;
                ((unsigned char*)img.data)[index + 2] = gridColor.b;
                ((unsigned char*)img.data)[index + 3] = 40;  // Semi-transparent
            } else {
                ((unsigned char*)img.data)[index] = bgColor.r;
                ((unsigned char*)img.data)[index + 1] = bgColor.g;
                ((unsigned char*)img.data)[index + 2] = bgColor.b;
                ((unsigned char*)img.data)[index + 3] = bgColor.a;
            }
        }
    }

    return img;
}

Texture2D AssetManager::GeneratePixelGrid() {
    Image img = CreateGridImage(40, {100, 100, 150, 255}, {20, 20, 40, 255});
    Texture2D tex = LoadTextureFromImage(img);
    UnloadImage(img);
    return tex;
}

Texture2D AssetManager::GeneratePixelBackground() {
    // Generate space-themed background with stars instead of dizzying grid
    int width = SCREEN_WIDTH;
    int height = SCREEN_HEIGHT;

    Image img = {
        .data = malloc(width * height * 4),
        .width = width,
        .height = height,
        .mipmaps = 1,
        .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8
    };

    // Create space background with stars
    // Deep space gradient (dark blue to black)
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int index = (y * width + x) * 4;

            // Create subtle gradient from top to bottom
            float t = (float)y / (float)height;
            unsigned char r = (unsigned char)(10 * (1.0f - t * 0.5f));
            unsigned char g = (unsigned char)(10 * (1.0f - t * 0.3f));
            unsigned char b = (unsigned char)(25 * (1.0f - t * 0.2f));

            ((unsigned char*)img.data)[index] = r;
            ((unsigned char*)img.data)[index + 1] = g;
            ((unsigned char*)img.data)[index + 2] = b;
            ((unsigned char*)img.data)[index + 3] = 255;
        }
    }

    // Add random stars with varying brightness
    const int numStars = 200;
    for (int i = 0; i < numStars; i++) {
        int x = rand() % width;
        int y = rand() % height;
        int starSize = 1 + (rand() % 2);  // 1-2 pixels
        int brightness = 150 + (rand() % 106);  // 150-255

        for (int dy = 0; dy < starSize && y + dy < height; dy++) {
            for (int dx = 0; dx < starSize && x + dx < width; dx++) {
                int index = ((y + dy) * width + (x + dx)) * 4;
                ((unsigned char*)img.data)[index] = (unsigned char)brightness;
                ((unsigned char*)img.data)[index + 1] = (unsigned char)brightness;
                ((unsigned char*)img.data)[index + 2] = (unsigned char)brightness;
                ((unsigned char*)img.data)[index + 3] = 255;
            }
        }
    }

    // Add a few brighter "stars" with slight blue tint
    const int numBrightStars = 20;
    for (int i = 0; i < numBrightStars; i++) {
        int x = rand() % width;
        int y = rand() % height;

        int index = (y * width + x) * 4;
        ((unsigned char*)img.data)[index] = 200;      // R
        ((unsigned char*)img.data)[index + 1] = 220;  // G
        ((unsigned char*)img.data)[index + 2] = 255;  // B
        ((unsigned char*)img.data)[index + 3] = 255;

        // Add glow around bright stars
        for (int dy = -1; dy <= 1; dy++) {
            for (int dx = -1; dx <= 1; dx++) {
                if (dx == 0 && dy == 0) continue;
                int nx = x + dx;
                int ny = y + dy;
                if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
                    int nIndex = (ny * width + nx) * 4;
                    ((unsigned char*)img.data)[nIndex] = 150;
                    ((unsigned char*)img.data)[nIndex + 1] = 170;
                    ((unsigned char*)img.data)[nIndex + 2] = 200;
                    ((unsigned char*)img.data)[nIndex + 3] = 180;
                }
            }
        }
    }

    Texture2D tex = LoadTextureFromImage(img);
    UnloadImage(img);
    return tex;
}

// LoadExternalFont using LoadFileData + LoadFontFromMemory method (like the WeChat public account example)
bool AssetManager::LoadExternalFont(const char* fontPath, int fontSize) {
    // CRITICAL FIX: On Android, LoadFontEx() cannot access assets directory directly
    // We MUST use LoadFileData() + LoadFontFromMemory() method (like the working example)

    // Collect all Chinese text used in the game into one string
    // Expanded to ~3500 common Chinese characters
    const char* allChineseText =
        // Game UI text
        "方块吞噬者无尽模式关卡模式时间挑战设置返回开始继续退出重新开始"
        "生命能量等级分数时间"
        "主题蓝色深色绿色紫色红色"
        "语言中文英文"
        "控制虚拟摇杆触摸跟随"
        "字体Zpix默认像素"
        "普通困难专家"
        "第关"
        "暂停游戏结束胜利失败"
        "技能方向不能同时响应"
        "字体加载成功加载失败"
        "使用默认字体中文可能无法正确显示"
        "当前语言"
        "已经切换到"
        "主题"
        "控制方式"
        "日志查看器"
        "所有日志已同步"
        "当前等级"
        "当前分数"
        "剩余时间"
        "关卡选择"
        "请选择关卡"
        "无尽模式"
        "关卡模式"
        "时间挑战"
        "设置"
        "主菜单"
        "暂停"
        "游戏结束"
        "你赢了"
        "你死了"
        "再玩一次"
        "继续游戏"
        "返回主菜单"
        "音量静音取消"
        "触摸左半屏移动"
        "经验系统显示条日志"
        "选择关卡摇杆像素"
        "退出到菜单再试一次"
        "默认字体未知"
        "查看日志"
        // User System text
        "用户系统"
        "用户名"
        "登录注册"
        "请输入用户名"
        "确认登录"
        "取消返回"
        "最高分"
        "游戏次数"
        "总游戏时长"
        "通关记录"
        "无尽模式最高分"
        "时间挑战最好成绩"
        "成就系统"
        "个人资料"
        "保存成功"
        "保存失败"
        "已存在"
        "创建成功"
        "欢迎回来"
        "玩家"
        "数据统计"
        "历史记录"
        "清空数据"
        "删除账户"
        "确定"
        "警告"
        "无法恢复"
        // Top 100 most common Chinese characters
        "的一是在不了有和人这中大为上个国我以要他时来用们生到作地于出就分对成会可主发年动同工也能下过子说产种面而方后多定行学法所民得经十三之进着等部度家电力里如水化高自二理起小物现实加量都两体制机当使点从业本去把性好应开它合还因由其些然前外天政四日那社义事平形相全表间样与关各重新线内数正心反你明看原又么利比或但质气第向道命此变条只没结解问意建月公无系军很情者最立代想已通并提直题党程展五果料象员革位入常文总次品式活设及管特件长求老头基资边流路级少图山统接知较将组见计别她手角期根论运农指几九区强放决西被干做必战先回则任取完举科触广"
        // Common surnames and given names
        "李张陈刘杨黄赵王周吴徐孙马朱胡郭何高林罗郑梁谢宋唐许韩冯邓曹彭曾萧田董袁潘于蒋蔡余杜叶程苏魏吕丁任沈姚卢姜崔钟谭陆汪范金石廖贾夏韦付方白邹孟熊秦邱江尹薛闫段雷侯龙钱史陶贺"
        "伟芳秀英娜敏静丽强军平杰刚磊霞明辉洋勇艳霞萍娟涛红建军华国琴兰云玲海峰梅波萍燕敏丽霞军红平华建国"
        // Common verbs and adjectives
        "大小多少好坏新旧美丑黑白长短高低快慢胖瘦冷热深浅粗细软硬轻重"
        "吃喝玩乐睡醒坐站走跑跳飞游爬看听说读写想思做造制作买卖交易学习研究工作休息劳动运动比赛胜利失败成功"
        "天地人日月星山水火风云雨雪风雷电冰霜春夏秋冬早晚今昨明前后左右上下内外东西南北中"
        "一二三四五六七八九十百千万亿零"
        "年月日时分秒今天明天昨天现在过去未来刚才立刻马上永远一直"
        "爸爸妈妈兄弟姐妹儿子女儿爷爷奶奶外公外婆叔叔阿姨"
        "吃喝穿住用买卖市场商店超市餐厅酒店学校医院银行公司工厂政府军队警察"
        "道路街道城市乡村国家世界地球宇宙"
        "钱价格便宜贵便宜质量数量重量尺寸大小体积面积容量"
        "安全危险健康疾病生命死亡生活工作学习"
        "爱情感情友谊家庭婚姻亲戚朋友同事同学"
        "颜色红橙黄绿青蓝紫黑白灰金银粉彩色"
        "味道酸甜苦辣咸鲜淡香臭"
        "声音噪音安静大声小声说话唱歌音乐"
        "光线明亮黑暗白天黑夜夕阳日出日落阳光月光星光"
        "天气晴阴雨雪风雨雷电彩虹云"
        "动物猫狗鸟鱼牛马羊猪鸡鸭昆虫老虎狮子大象"
        "植物树草花叶果实根种子蔬菜水果"
        "房屋建筑房间门窗墙壁地板屋顶楼"
        "工具机器设备电脑手机网络电视收音机"
        "交通汽车火车飞机自行车地铁公交车船"
        "衣服裤子鞋帽子袜子"
        "书籍报纸杂志笔纸书包"
        // Common measure words
        "个件条只张本台辆座栋间层页篇封句段章节首张"
        "次回遍番趟遍阵场"
        "人个位口名家名"
        "天月年周季世纪时代年代"
        "点分秒小时分钟时刻"
        "米厘米毫米公里千米"
        "克公斤吨"
        "元角分块毛"
        "升毫升"
        // Directions and positions
        "东西南北中上下左右前后内外旁边周围中间中心"
        "地方位置地点场所区域"
        "远近高低深浅宽窄长短"
        // Time related
        "现在当时那时以前以后目前如今今天昨天明天前天后天今年去年明年"
        "早上上午中午下午傍晚晚上深夜凌晨"
        "春天夏天秋天冬天"
        "一月二月三月四月五月六月七月八月九月十月十一月十二月"
        "星期一星期二星期三星期四星期五星期六星期日"
        // Numbers and math
        "零一二三四五六七八九十百千万亿兆"
        "加减乘除等于大于小于"
        "整数小数分数百分比"
        // Common daily items
        "桌椅板凳门窗玻璃窗帘地毯床柜子抽屉"
        "锅碗瓢盆刀叉勺筷子杯盘碗碟"
        "手机电话电脑电视音响耳机充电器电池"
        "笔纸本子橡皮尺子剪刀胶水"
        "钱包钥匙身份证卡票证"
        "衣服裤子内衣外套鞋子袜子帽子围巾手套"
        "化妆品护肤品洗发水沐浴露牙膏牙刷毛巾"
        "药品药丸药片医院药店医生护士"
        // Common verbs in daily life
        "起床刷牙洗脸吃饭睡觉洗澡洗衣服打扫做饭洗碗工作学习休息运动"
        "买东西付钱找零退货取货送货"
        "打电话发短信聊天上网玩游戏看视频听音乐"
        "开车坐车骑车走路跑步坐船坐飞机"
        "开门关门开灯关灯打开关闭"
        "坐下站起来走过去跑回来跳下去爬上去"
        "看见听见闻到摸到想到说到做到"
        // Emotions and feelings
        "开心快乐高兴愉快幸福满足满意舒服轻松自在"
        "难过伤心痛苦悲伤失望沮丧焦虑紧张害怕担心烦恼生气愤怒"
        "喜欢讨厌热爱恨"
        "惊讶震惊意外"
        "骄傲自豪羞愧"
        "孤独寂寞孤单"
        "疲惫累困"
        "饿了渴了饱了"
        "冷暖热凉温"
        // Common questions and answers
        "什么怎么哪里哪个谁何时多少为什么"
        "是不是对不对好不好行不行可以不可以"
        "当然肯定一定确实真的"
        "也许可能大概或许"
        "已经还没有"
        // Other common characters
        "这种那个这样那样"
        "并且而且或者但是"
        "因为所以如果那么"
        "虽然但是即使"
        "只有才只要就无论都"
        "越来越越来";

    // Increase buffer size for more Chinese characters
    char allText[4096];
    snprintf(allText, sizeof(allText),
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"
        " !\"#$%%&'()*+,-./:;<=>?@[\\]^_`{|}~%s",
        allChineseText);

    TraceLog(LOG_INFO, "=== LoadExternalFont START (LoadFileData + LoadFontFromMemory method) ===");

    // Use LoadCodepoints to automatically extract characters from text
    int codepointCount = 0;
    int* codepoints = LoadCodepoints(allText, &codepointCount);

    TraceLog(LOG_INFO, TextFormat("LoadCodepoints extracted %d characters from text", codepointCount));

    // List of font paths to try (Source Han Sans removed to reduce APK size)
    const char* fontPaths[] = {
        "fonts/zpix.ttf",
        "fonts/vonwaon_pixel_12px.ttf",
        "zpix.ttf",
        "vonwaon_pixel_12px.ttf"
    };
    const int numPaths = sizeof(fontPaths) / sizeof(fontPaths[0]);

    // Try each font path
    for (int i = 0; i < numPaths; i++) {
        const char* path = fontPaths[i];

        TraceLog(LOG_INFO, TextFormat("Trying font: %s", path));

        // Load font file into memory (works on Android!)
        int fileSize = 0;
        unsigned char* fileData = LoadFileData(path, &fileSize);

        if (fileData == nullptr || fileSize == 0) {
            TraceLog(LOG_WARNING, TextFormat("LoadFileData failed for: %s", path));
            continue;
        }

        TraceLog(LOG_INFO, TextFormat("LoadFileData SUCCESS: %d bytes loaded from %s", fileSize, path));

        // Get file extension for font type detection
        const char* ext = (strstr(path, ".ttf") != nullptr) ? ".ttf" : ".otf";

        // Load font from memory using LoadFontFromMemory (like the WeChat example!)
        pixelFont = LoadFontFromMemory(ext, fileData, fileSize, fontSize, codepoints, codepointCount);

        // Free file data after loading
        UnloadFileData(fileData);

        if (pixelFont.texture.id != 0 && pixelFont.glyphCount > 100) {
            TraceLog(LOG_INFO, TextFormat("SUCCESS: Font loaded from memory! glyphs=%d (expected %d)",
                pixelFont.glyphCount, codepointCount));
            SetTextureFilter(pixelFont.texture, TEXTURE_FILTER_BILINEAR);
            GenTextureMipmaps(&pixelFont.texture);

            // Load small font
            fileData = LoadFileData(path, &fileSize);
            if (fileData != nullptr) {
                smallFont = LoadFontFromMemory(ext, fileData, fileSize, (int)(fontSize * 0.75f), codepoints, codepointCount);
                UnloadFileData(fileData);

                if (smallFont.texture.id != 0) {
                    SetTextureFilter(smallFont.texture, TEXTURE_FILTER_BILINEAR);
                    GenTextureMipmaps(&smallFont.texture);
                } else {
                    smallFont = pixelFont;
                }
            } else {
                smallFont = pixelFont;
            }

            UnloadCodepoints(codepoints);
            return true;
        } else {
            TraceLog(LOG_WARNING, TextFormat("Font loaded but glyphCount too low: %d (need > 100)", pixelFont.glyphCount));
        }
    }

    UnloadCodepoints(codepoints);
    TraceLog(LOG_ERROR, "All font loading attempts failed!");
    return false;
}

void AssetManager::UnloadExternalFonts() {
    // Only unload if it's not the default font
    if (pixelFont.texture.id != 0 && pixelFont.texture.id != GetFontDefault().texture.id) {
        UnloadFont(pixelFont);
    }
    if (smallFont.texture.id != 0 &&
        smallFont.texture.id != GetFontDefault().texture.id &&
        smallFont.texture.id != pixelFont.texture.id) {
        UnloadFont(smallFont);
    }
    pixelFont = GetFontDefault();
    smallFont = GetFontDefault();
}

bool AssetManager::LoadFontByType(int fontType) {
    // Unload current fonts first
    if (pixelFont.texture.id != 0 && pixelFont.texture.id != GetFontDefault().texture.id) {
        UnloadFont(pixelFont);
    }
    if (smallFont.texture.id != 0 &&
        smallFont.texture.id != GetFontDefault().texture.id &&
        smallFont.texture.id != pixelFont.texture.id) {
        UnloadFont(smallFont);
    }

    // Load new font based on type
    const char* fontPath = nullptr;
    int fontSize = 16;

    switch (fontType) {
        case 0:  // Zpix
            fontPath = "fonts/zpix.ttf";
            fontSize = 20;
            TraceLog(LOG_INFO, "Loading Zpix font");
            break;
        case 1:  // Vonwaon pixel font
            fontPath = "fonts/vonwaon_pixel_12px.ttf";
            fontSize = 12;
            TraceLog(LOG_INFO, "Loading Vonwaon pixel font");
            break;
        case 2:  // Default
            pixelFont = GetFontDefault();
            smallFont = GetFontDefault();
            TraceLog(LOG_INFO, "Using default font");
            return true;
        default:
            return false;
    }

    return LoadExternalFont(fontPath, fontSize);
}

} // namespace BlockEater
