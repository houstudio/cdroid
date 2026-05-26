#include <iostream>
#include <fstream>
#include <vector>
#include <memory>
#include <cstring>
#include <unistd.h>

// FreeType includes
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

// Cairo includes
#include <cairo.h>
#include <cairo-pdf.h>
#include <cairo-ft.h>

// HarfBuzz includes
#include <hb.h>
#include <hb-ft.h>

// Minikin includes
#include "minikin/Layout.h"
#include "minikin/MinikinPaint.h"
#include "minikin/FontCollection.h"
#include "minikin/FontFamily.h"
#include "minikin/FontStyle.h"
#include "minikin/U16StringPiece.h"
#include "minikin/Range.h"
#include "minikin/MinikinFont.h"
#include "minikin/FontVariation.h"
#include "minikin/MinikinExtent.h"
#include "minikin/Font.h"
#include "minikin/HbUtils.h"
#include "minikin/LineBreaker.h"
#include "minikin/LocaleList.h"
#include "minikin/GreedyLineBreaker.h"
#include "minikin/OptimalLineBreaker.h"
#include "minikin/LocaleListCache.h"
#include "minikin/MeasuredText.h"

// UTF-8 to UTF-16 conversion
std::vector<uint16_t> utf8ToUtf16(const std::string& str) {
    std::vector<uint16_t> result;
    for (size_t i = 0; i < str.size(); ) {
        unsigned char c = str[i];
        if (c < 0x80) {
            result.push_back(c);
            i++;
        } else if (c < 0xE0) {
            uint16_t val = ((c & 0x1F) << 6) | (str[i+1] & 0x3F);
            result.push_back(val);
            i += 2;
        } else if (c < 0xF0) {
            uint16_t val = ((c & 0x0F) << 12) | ((str[i+1] & 0x3F) << 6) | (str[i+2] & 0x3F);
            result.push_back(val);
            i += 3;
        } else {
            uint32_t val = ((c & 0x07) << 18) | ((str[i+1] & 0x3F) << 12) | 
                           ((str[i+2] & 0x3F) << 6) | (str[i+3] & 0x3F);
            val -= 0x10000;
            result.push_back(0xD800 | ((val >> 10) & 0x3FF));
            result.push_back(0xDC00 | (val & 0x3FF));
            i += 4;
        }
    }
    return result;
}

// UTF-16 to UTF-8 conversion
std::string utf16ToUtf8(const std::u16string& str) {
    std::string result;
    for (size_t i = 0; i < str.size(); ) {
        uint16_t c = str[i];
        if (c < 0x80) {
            result.push_back(static_cast<char>(c));
            i++;
        } else if (c < 0x800) {
            result.push_back(static_cast<char>(0xC0 | ((c >> 6) & 0x1F)));
            result.push_back(static_cast<char>(0x80 | (c & 0x3F)));
            i++;
        } else if (c >= 0xD800 && c <= 0xDFFF) {
            if (i + 1 < str.size()) {
                uint16_t c2 = str[i + 1];
                uint32_t codePoint = ((c - 0xD800) << 10) | (c2 - 0xDC00) + 0x10000;
                result.push_back(static_cast<char>(0xF0 | ((codePoint >> 18) & 0x07)));
                result.push_back(static_cast<char>(0x80 | ((codePoint >> 12) & 0x3F)));
                result.push_back(static_cast<char>(0x80 | ((codePoint >> 6) & 0x3F)));
                result.push_back(static_cast<char>(0x80 | (codePoint & 0x3F)));
                i += 2;
            } else {
                i++;
            }
        } else {
            result.push_back(static_cast<char>(0xE0 | ((c >> 12) & 0x0F)));
            result.push_back(static_cast<char>(0x80 | ((c >> 6) & 0x3F)));
            result.push_back(static_cast<char>(0x80 | (c & 0x3F)));
            i++;
        }
    }
    return result;
}

// 全局字体 ID 计数器
static int32_t gFontIdCounter = 0;

// MinikinFont implementation using FreeType
class FullMinikinFont : public minikin::MinikinFont {
public:
    FullMinikinFont(const std::string& fontPath) 
        : MinikinFont(gFontIdCounter++),  // 调用父类构造函数，传递 uniqueId
          mFontPath(fontPath), mFtLibrary(nullptr), mFtFace(nullptr), mHbFont(nullptr) {
        std::ifstream file(fontPath, std::ios::binary | std::ios::ate);
        if (file) {
            mFontSize = file.tellg();
            mFontData.resize(mFontSize);
            file.seekg(0);
            file.read(reinterpret_cast<char*>(mFontData.data()), mFontSize);
        }
        
        FT_Init_FreeType(&mFtLibrary);
        FT_New_Face(mFtLibrary, fontPath.c_str(), 0, &mFtFace);
        mHbFont = hb_ft_font_create(mFtFace, nullptr);
    }
    
    ~FullMinikinFont() {
        if (mHbFont) hb_font_destroy(mHbFont);
        if (mFtFace) FT_Done_Face(mFtFace);
        if (mFtLibrary) FT_Done_FreeType(mFtLibrary);
    }
    
    float GetHorizontalAdvance(uint32_t glyph_id, const minikin::MinikinPaint& paint,
                               const minikin::FontFakery&) const override {
        FT_Set_Pixel_Sizes(mFtFace, 0, static_cast<int>(paint.size));
        if (FT_Load_Glyph(mFtFace, glyph_id, FT_LOAD_NO_HINTING)) {
            return 0;
        }
        return static_cast<float>(mFtFace->glyph->advance.x) / 64.0f;
    }
    
    void GetBounds(minikin::MinikinRect* bounds, uint32_t glyph_id, 
                   const minikin::MinikinPaint& paint, const minikin::FontFakery&) const override {
        FT_Set_Pixel_Sizes(mFtFace, 0, static_cast<int>(paint.size));
        if (FT_Load_Glyph(mFtFace, glyph_id, FT_LOAD_NO_HINTING)) {
            bounds->mLeft = bounds->mTop = bounds->mRight = bounds->mBottom = 0;
            return;
        }
        bounds->mLeft = static_cast<float>(mFtFace->glyph->metrics.horiBearingX) / 64.0f;
        bounds->mTop = -static_cast<float>(mFtFace->glyph->metrics.horiBearingY) / 64.0f;
        bounds->mRight = bounds->mLeft + static_cast<float>(mFtFace->glyph->metrics.width) / 64.0f;
        bounds->mBottom = bounds->mTop + static_cast<float>(mFtFace->glyph->metrics.height) / 64.0f;
    }
    
    void GetFontExtent(minikin::MinikinExtent* extent, const minikin::MinikinPaint& paint,
                       const minikin::FontFakery&) const override {
        FT_Set_Pixel_Sizes(mFtFace, 0, static_cast<int>(paint.size));
        extent->ascent = static_cast<float>(mFtFace->size->metrics.ascender) / 64.0f;
        extent->descent = -static_cast<float>(mFtFace->size->metrics.descender) / 64.0f;
    }
    
    // 实现纯虚函数 GetAxes()
    const std::vector<minikin::FontVariation>& GetAxes() const override {
        static const std::vector<minikin::FontVariation> emptyAxes;
        return emptyAxes;
    }
    
    const std::string& GetFontPath() const { return mFontPath; }  // 移除 override
    const void* GetFontData() const override { return mFontData.data(); }
    size_t GetFontSize() const override { return mFontSize; }
    int GetFontIndex() const override { return 0; }
    
    hb_font_t* getHbFont() const { return mHbFont; }
    FT_Face getFace() const { return mFtFace; }
    
private:
    std::string mFontPath;
    std::vector<uint8_t> mFontData;
    size_t mFontSize = 0;
    FT_Library mFtLibrary;
    FT_Face mFtFace;
    hb_font_t* mHbFont;
};

// FixedLineWidth - 固定宽度的 LineWidth 实现
class FixedLineWidth : public minikin::LineWidth {
public:
    FixedLineWidth(float width) : mWidth(width) {}
    
    float getAt(size_t /* lineNo */) const override {
        return mWidth;
    }
    
    float getMin() const override {
        return mWidth;
    }
    
private:
    float mWidth;
};

// 存储字体映射
std::unordered_map<const minikin::MinikinFont*, std::shared_ptr<FullMinikinFont>> gFontMap;

// 微型排版模块
class MiniTypesetter {
public:
    MiniTypesetter(std::shared_ptr<minikin::FontCollection> fontCollection, float fontSize)
        : mFontCollection(fontCollection), mFontSize(fontSize) {}
    
    // 执行排版，返回断行结果
    minikin::LineBreakResult performLayout(const std::vector<uint16_t>& text, float maxWidth) {
        // 创建 MinikinPaint
        minikin::MinikinPaint paint(mFontCollection);
        paint.size = mFontSize;
        paint.scaleX = 1.0f;
        paint.skewX = 0.0f;
        paint.letterSpacing = 0.0f;
        paint.wordSpacing = 0.0f;
        paint.fontFlags = 0;
        paint.localeListId = minikin::LocaleListCache::getId("en-US");
        paint.fontStyle = minikin::FontStyle();
        paint.familyVariant = minikin::FamilyVariant::DEFAULT;
        
        // 构建 MeasuredText
        minikin::MeasuredTextBuilder builder;
        builder.addStyleRun(0, text.size(), std::move(paint), true /* is RTL */);
        
        minikin::U16StringPiece textPiece(text.data(), text.size());
        auto measuredText = builder.build(textPiece, false /* compute hyphenation */, 
                                          false /* compute layout */,
                                          nullptr /* no hint */);
        if (measuredText->widths.size() > 0) {
            float totalWidth = 0;
            for (float w : measuredText->widths) {
                totalWidth += w;
            }
        }
        
        // 设置断行参数
        FixedLineWidth lineWidth(maxWidth);
        minikin::TabStops tabStops(nullptr, 0, 10);  // 使用 10 作为 tabWidth
        
        // 执行断行 - cdroidMaster 版本的 breakLineGreedy 只有 5 个参数
        std::cout << std::endl;
        //breakLineGreedy breakLineOptimal
#if 10
        auto result = minikin::breakLineGreedy(textPiece, *measuredText, lineWidth, tabStops,false /* enableHyphenation */);
#else
        auto result = minikin::breakLineOptimal(textPiece, *measuredText, lineWidth,
                minikin::BreakStrategy::Greedy,/*Greedy HighQuality Balanced*/
                minikin::HyphenationFrequency::Normal/*None Mormal Full*/,false/*justified*/);
#endif
        // 打印断行结果
        if (result.breakPoints.empty()) {
            //std::cout << "  No break points found!" << std::endl;
        } else {
            int32_t start = 0;
            for (size_t i = 0; i < result.breakPoints.size(); i++) {
                int32_t end = result.breakPoints[i];
                std::u16string lineText(reinterpret_cast<const char16_t*>(&text[start]), end - start);
                std::string lineUtf8 = utf16ToUtf8(lineText);
                //std::cout << "  Line " << i << ": break at " << result.breakPoints[i] 
                //          << ", width: " << result.widths[i] << std::endl;
                start = end;
            }
        }
        return result;
    }
    
    // 获取字体集合
    std::shared_ptr<minikin::FontCollection> getFontCollection() const {
        return mFontCollection;
    }
    
    float getFontSize() const {
        return mFontSize;
    }
    
private:
    std::shared_ptr<minikin::FontCollection> mFontCollection;
    float mFontSize;
};

// 使用 Cairo 渲染布局结果到 PNG
void renderToPng(const std::vector<uint16_t>& text,
                 const minikin::LineBreakResult& lineBreakResult,
                 std::shared_ptr<minikin::FontCollection> fontCollection,
                 float fontSize, const std::string& outputPath) {
    // 从 fontCollection 获取基础字体信息
    minikin::FakedFont baseFont = fontCollection->baseFontFaked(minikin::FontStyle());
    const minikin::Font* fontPtr = baseFont.font;
    const minikin::MinikinFont* minikinFontPtr = fontPtr->typeface().get();
    
    // 从 gFontMap 获取 FullMinikinFont，然后获取 FT_Face
    FT_Face face = nullptr;
    auto it = gFontMap.find(minikinFontPtr);
    if (it != gFontMap.end()) {
        face = it->second->getFace();
        FT_Set_Pixel_Sizes(face, 0, static_cast<int>(fontSize));
    }
    
    // 使用 FT_Face 的真实 metrics
    float fontAscent, fontDescent;
    fontAscent = static_cast<float>(face->size->metrics.ascender) / 64.0f;
    fontDescent = -static_cast<float>(face->size->metrics.descender) / 64.0f;
    
    // 计算图像尺寸
    float maxWidth = 0;
    for (float w : lineBreakResult.widths) {
        if (w > maxWidth) maxWidth = w;
    } 
    float lineHeight = fontAscent + fontDescent + 8;
    int imgWidth = static_cast<int>(maxWidth) + 40;
    int imgHeight = static_cast<int>(lineHeight * lineBreakResult.breakPoints.size()) + 60;
    
    // 创建 Cairo surface 和 context
    cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, imgWidth, imgHeight);
    cairo_t* cr = cairo_create(surface);
    
    // 设置白色背景
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_paint(cr);
    
    // 绘制边界框
    cairo_set_source_rgba(cr, 0.8, 0.8, 0.8, 1.0);
    cairo_rectangle(cr, 19, 19, maxWidth + 2, imgHeight - 40);
    cairo_stroke(cr);
    
    // 设置字体颜色为黑色
    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
    
    // 创建 MinikinPaint 用于布局
    minikin::MinikinPaint paint(fontCollection);
    paint.size = fontSize;
    paint.scaleX = 1.0f;
    paint.skewX = 0.0f;
    paint.letterSpacing = 0.0f;
    paint.wordSpacing = 0.0f;
    
    // 渲染每一行
    float y = 20 + fontAscent;
    int prevBreak = 0;
    
    for (size_t lineIdx = 0; lineIdx < lineBreakResult.breakPoints.size(); lineIdx++) {
        int end = lineBreakResult.breakPoints[lineIdx];
        int length = end - prevBreak;
        if (length <= 0) {
            prevBreak = end;
            y += lineHeight;
            continue;
        }
        std::cout << "Line " << lineIdx << ": chars " << prevBreak << "-" << end 
                  << ", width: " << lineBreakResult.widths[lineIdx] << std::endl;
        
        // 使用 minikin Layout 进行字形布局
        // cdroidMaster 版本的 Layout 构造函数使用 U16StringPiece
        minikin::U16StringPiece lineTextPiece(text.data() + prevBreak, length);
        minikin::Layout layout(lineTextPiece, minikin::Range(0, length), 
                               minikin::Bidi::DEFAULT_LTR, paint,
                               minikin::StartHyphenEdit::NO_EDIT,
                               minikin::EndHyphenEdit::NO_EDIT);
        
        std::cout << "  Glyphs count: " << layout.nGlyphs() << std::endl;
        
        // 渲染字形
        float x = 20;
        const minikin::MinikinFont* currentFont = nullptr;
        cairo_font_face_t* currentCairoFontFace = nullptr;
        size_t glyphIdx=0;
        while(glyphIdx < layout.nGlyphs()) {
            // cdroidMaster 版本的 getFont() 返回 MinikinFont*
            const minikin::MinikinFont* glyphFont = layout.getFont(glyphIdx);
            
            // 检查是否需要切换字体
            if (glyphFont != currentFont) {
                if (currentCairoFontFace != nullptr) {
                    cairo_font_face_destroy(currentCairoFontFace);
                }
                
                currentFont = glyphFont;
                // 查找对应的 FullMinikinFont
                auto it = gFontMap.find(glyphFont);
                auto fullFont = dynamic_cast<const FullMinikinFont*>(glyphFont);
                if (fullFont){// != gFontMap.end()) {
                    FT_Face face = fullFont->getFace();
                    FT_Set_Pixel_Sizes(face, 0, static_cast<int>(fontSize));
                    currentCairoFontFace = cairo_ft_font_face_create_for_ft_face(face, 0);
                    cairo_set_font_face(cr, currentCairoFontFace);
                    cairo_set_font_size(cr, fontSize);
                    std::cout << glyphIdx<<":Switched to font: " << fullFont->GetFontPath() << std::endl;
                }else{
                    std::cerr << "  Warning: Could not get FullMinikinFont for glyph:"<<glyphIdx << std::endl;
                    glyphIdx++;
                    continue;
                }
            }
 
            std::vector<cairo_glyph_t> cairoGlyphs;
            float glyphsWidth = 0;
            while (glyphIdx < layout.nGlyphs() && layout.getFont(glyphIdx) == currentFont) {
                cairo_glyph_t glyph;
                glyph.index = layout.getGlyphId(glyphIdx);
                glyph.x = 20+layout.getX(glyphIdx);
                glyph.y = y+layout.getY(glyphIdx);
                cairoGlyphs.push_back(glyph);
                glyphsWidth += layout.getX(glyphIdx) - (glyphsWidth > 0 ? cairoGlyphs[cairoGlyphs.size()-2].x : 0);
                glyphIdx++;
            }
            // 渲染这组字形
            cairo_show_glyphs(cr, cairoGlyphs.data(), cairoGlyphs.size());
            x += glyphsWidth;
        }
        
        if (currentCairoFontFace != nullptr) {
            cairo_font_face_destroy(currentCairoFontFace);
        }
        
        prevBreak = end;
        y += lineHeight;
    }
    
    // 写入 PNG
    cairo_surface_write_to_png(surface, outputPath.c_str());
    
    // 清理资源
    cairo_destroy(cr);
    cairo_surface_destroy(surface);
}

int main() {
    // 查找可用字体
    const char* fontPaths[] = {
        "/home/houzh/.fonts/Alibaba_PuHuiTi_2.0_55_Regular_55_Regular.ttf",
        "/home/houzh/.fonts/yilang/HarmonyOS_Sans_SC_Regular.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/freefont/FreeSans.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans.ttf"
    };
 
    std::vector<std::string> availableFontPaths;
    for (const char* fontPath : fontPaths) {
        if (access(fontPath, R_OK) == 0) {
            availableFontPaths.push_back(fontPath);
        }
    }
    
    if (availableFontPaths.empty()) {
        std::cerr << "Failed to find any font" << std::endl;
        return 1;
    }
    
    std::cout << "Available fonts:" << std::endl;
    for (const auto& path : availableFontPaths) {
        std::cout << "  - " << path << std::endl;
    }
    
    // 创建 FontCollection，包含所有可用字体
    std::vector<std::shared_ptr<minikin::FontFamily>> families;
    
    for (const auto& fontPath : availableFontPaths) {
        std::shared_ptr<FullMinikinFont> minikinFont = std::make_shared<FullMinikinFont>(fontPath);
        auto font = minikin::Font::Builder(minikinFont).build();
        
        // 存储到全局字体映射
        gFontMap[minikinFont.get()] = minikinFont;
        
        std::vector<minikin::Font> fonts;
        fonts.push_back(std::move(font));
        
        // cdroidMaster 版本使用构造函数创建 FontFamily
        auto fontFamily = std::make_shared<minikin::FontFamily>(std::move(fonts));
        families.push_back(fontFamily);
    }
    
    // cdroidMaster 版本使用构造函数创建 FontCollection
    auto fontCollection = std::make_shared<minikin::FontCollection>(families);
    
    // 创建排版器
    const float fontSize = 32.0f;
    MiniTypesetter typesetter(fontCollection, fontSize);
    
    // 测试文本 - 包含英文、阿拉伯语和波斯语
    std::string testTextUTF8 = 
        "Hello World! السلام عليكم (Peace be upon you) مرحبا "
        "This is a test with multiple languages including "
        "Arabic: مرحبا بالعالم and Persian: سلام دنیا "
        "شكراً for testing. Thank you! متشکرم "
        "Line breaking should work properly with complex scripts."; 
    std::vector<uint16_t> text = utf8ToUtf16(testTextUTF8);
    std::cout << "Text length: " << text.size() << " characters" << std::endl;
    
    // 设置行宽
    const float lineWidth = 800.0f;
    std::cout << "\nTarget line width: " << lineWidth << " pixels" << std::endl;
    
    // 执行排版
    auto startTime = std::chrono::high_resolution_clock::now();
    minikin::LineBreakResult result = typesetter.performLayout(text, lineWidth);
    for(int i=0;i<999;i++){
        minikin::LineBreakResult result = typesetter.performLayout(text, lineWidth);
    }
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
    std::cout << "Number of lines: " << result.breakPoints.size() << std::endl;
    std::cout << "performLayout took: " << duration.count() << " microseconds" << std::endl;

    if (result.breakPoints.size() > 0) {
        int32_t start = 0;
        for (size_t i = 0; i < result.breakPoints.size(); i++) {
            int32_t end = result.breakPoints[i];
            std::u16string lineText(reinterpret_cast<const char16_t*>(&text[start]), end - start);
            std::string lineUtf8 = utf16ToUtf8(lineText);
            
            std::cout << "  Line " << i << ": [" << lineUtf8 << "]"
                      << ", break at " << end 
                      << ", width: " << result.widths[i] << std::endl;
            
            start = end;
        }
    } else {
        std::cout << "  Warning: No line breaks found!" << std::endl;
    }
    
    // 渲染到 PNG
    renderToPng(text, result, fontCollection, fontSize, 
                "minikin_cairo_output.png");
    
    std::cout << "\nPNG output saved to: /home/houzh/cdroidMaster/src/3rdparty/minikin/minikin_cairo_output.png" << std::endl;
    std::cout << "Successfully demonstrated multi-line layout with specified width using Minikin!" << std::endl;
    
    return 0;
}
