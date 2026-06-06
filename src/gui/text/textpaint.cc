#include <memory>
#include <iostream>
#include <text/textpaint.h>
#include <text/textutils.h>
#include <text/spannablestring.h>
#include <minikin/GraphemeBreak.h>
#include <minikin/LocaleList.h>
#include <minikin/Measurement.h>
#include <minikin/MeasuredText.h>
#include <minikin/MinikinFont.h>
#include <core/typeface.h>
#include <core/canvas.h>
#include <porting/cdlog.h>
#include <hb.h>
#include <hb-ft.h>

namespace cdroid{

static int32_t gFontIdCounter = 0;

// MinikinFont implementation using Cairomm's FtFontFace
class FullMinikinFont : public minikin::MinikinFont {
public:
    FullMinikinFont(const Cairo::RefPtr<Cairo::FtFontFace>& fontFace)
        : MinikinFont(gFontIdCounter++),
          mFontFace(fontFace),
          mHbFace(nullptr),
          mCachedScaledFont(nullptr),
          mCachedFontSize(0.0f) {
    }

    ~FullMinikinFont() override {
        if (mHbFace != nullptr) {
            hb_face_destroy(mHbFace);
        }
    }

    float GetHorizontalAdvance(uint32_t glyph_id, const minikin::MinikinPaint& paint,
                               const minikin::FontFakery&) const override {
        Cairo::RefPtr<Cairo::FtScaledFont> scaledFont = getScaledFont(paint.size);        
        std::vector<Cairo::Glyph> glyphs(1);
        glyphs[0].index = glyph_id;
        //glyphs[0].x = 0;
        //glyphs[0].y = 0;
        Cairo::TextExtents extents;
        scaledFont->get_glyph_extents(glyphs, extents);
        return extents.x_advance;
    }

    void GetBounds(minikin::MinikinRect* bounds, uint32_t glyph_id,
                   const minikin::MinikinPaint& paint, const minikin::FontFakery&) const override {
        Cairo::RefPtr<Cairo::FtScaledFont> scaledFont = getScaledFont(paint.size);
        
        std::vector<Cairo::Glyph> glyphs(1);
        glyphs[0].index = glyph_id;
        //glyphs[0].x = 0;
        //glyphs[0].y = 0;
        Cairo::TextExtents extents;
        scaledFont->get_glyph_extents(glyphs, extents);
        bounds->mLeft = extents.x_bearing;
        bounds->mTop = extents.y_bearing;
        bounds->mRight = extents.x_bearing + extents.width;
        bounds->mBottom = extents.y_bearing + extents.height;
    }

    void GetFontExtent(minikin::MinikinExtent* extent, const minikin::MinikinPaint& paint,
                       const minikin::FontFakery&) const override {
        Cairo::RefPtr<Cairo::FtScaledFont> scaledFont = getScaledFont(paint.size);
        Cairo::FontExtents fontExtents;
        scaledFont->get_extents(fontExtents);        
        extent->ascent = fontExtents.ascent;
        extent->descent = fontExtents.descent;
    }

    const std::vector<minikin::FontVariation>& GetAxes() const override {
        static const std::vector<minikin::FontVariation> emptyAxes;
        return emptyAxes;
    }

    const void* GetFontData() const override {
        if (mHbFace == nullptr) {
            Cairo::RefPtr<Cairo::FtScaledFont> scaledFont = createScaledFont(12.0f);
            FT_Face ftFace = scaledFont->lock_face();
            if (ftFace != nullptr) {
                mHbFace = hb_ft_face_create(ftFace, 0);
            }
            scaledFont->unlock_face();
        }
        return mHbFace;
    }
    size_t GetFontSize() const override {
        return 0;//sizeof filedata
    }
    int GetFontIndex() const override {
        return 0;
    }
public:
    // 获取缓存的 ScaledFont，只有当 fontSize 改变时才重新创建
    Cairo::RefPtr<Cairo::FtScaledFont> getScaledFont(float size) const {
        if (mCachedScaledFont == nullptr || mCachedFontSize != size) {
            mCachedScaledFont = createScaledFont(size);
            mCachedFontSize = size;
        }
        return mCachedScaledFont;
    }

    Cairo::RefPtr<Cairo::FtScaledFont> createScaledFont(float size) const {
        Cairo::Matrix font_mtx(size, 0.0, 0.0, size, 0.0, 0.0);
        Cairo::FontOptions options;
        Cairo::Matrix ctm = Cairo::identity_matrix();
        options.set_hint_style(Cairo::FontOptions::HintStyle::MEDIUM);
        options.set_hint_metrics(Cairo::FontOptions::HintMetrics::OFF);        
        return Cairo::FtScaledFont::create(mFontFace, font_mtx, ctm, options);
    }
    Cairo::RefPtr<Cairo::FtFontFace> mFontFace;
    mutable hb_face_t* mHbFace = nullptr;
    mutable Cairo::RefPtr<Cairo::FtScaledFont> mCachedScaledFont;
    mutable float mCachedFontSize;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////

Paint::Paint(){
    mTypeface = Typeface::DEFAULT;
    mStartHyphenEdit=0;
    mEndHyphenEdit=0;
    mWordSpace=0;
    mAntialias=false;
    mLetterSpacing=0;
    mMinikinPaint=new minikin::MinikinPaint(mTypeface->getFontCollection());
    mMinikinPaint->size=32;
    mMinikinPaint->scaleX=1.0;
    mMinikinPaint->skewX=0;
}

Paint::Paint(int flags):Paint(){
}

Paint::Paint(const Paint&other){
    set(other);
}

Paint::~Paint(){
}
void Paint::set(const Paint&o){
    mTypeface=o.mTypeface;
    mColor = o.mColor;
    mTextSize=o.mTextSize;
    mTextSkewX=o.mTextSkewX;
    mTextScaleX=o.mTextScaleX;
    mAntialias = o.mAntialias;
    mStartHyphenEdit = o.mStartHyphenEdit;
    mEndHyphenEdit = o.mEndHyphenEdit;
    mWordSpace = o.mWordSpace;
    mLetterSpacing = o.mLetterSpacing;
}

bool Paint::hasEqualAttributes(const Paint&other)const{
    return false;
}

float Paint::measureText(const std::string& text)const{
    return 100;
}

float Paint::measureText(const std::string& text, int start, int end)const{
    return 100;
}

float Paint::measureText(const CharSequence* text, int start, int end)const{
    std::vector<char32_t>buf(end-start);
    TextUtils::getChars(text, start, end, buf, 0);
    return measureText(buf.data(),0,end-start);
}

float Paint::measureText(const char32_t* text, int index, int count)const{
    const minikin::U32StringPiece textBuf(text+index, count);
    const minikin::Range range(0, count);
    const minikin::StartHyphenEdit startHyphen = static_cast<minikin::StartHyphenEdit>(getStartHyphenEdit());
    const minikin::EndHyphenEdit endHyphen = static_cast<minikin::EndHyphenEdit>(getEndHyphenEdit());
    auto bidiFlags = false/*isRtl*/ ? minikin::Bidi::FORCE_RTL : minikin::Bidi::FORCE_LTR;
    return minikin::Layout::measureText(textBuf, range, bidiFlags, *mMinikinPaint, startHyphen,
                                        endHyphen, nullptr);
}
void Paint::getFontMetricsInt(const CharSequence* text, int start, int count,
    int contextStart, int contextCount,bool isRtl,FontMetricsInt& outMetrics)const{
}

int Paint::getFontMetricsInt(FontMetricsInt& fmi)const{
    std::shared_ptr<minikin::MinikinFont> minikinFont = mTypeface->getMinikinFont();
    minikin::MinikinExtent extent;
    minikinFont->GetFontExtent(&extent, *mMinikinPaint, minikin::FontFakery());
    fmi.ascent = extent.ascent;
    fmi.descent = extent.descent;
    fmi.leading = 0;
    fmi.top = fmi.ascent;           // top 等于 ascent
    fmi.bottom = fmi.descent;       // bottom 等于 descent
    return fmi.descent - fmi.ascent;
}

float Paint::getTextRunAdvances(const std::vector<char32_t>& chars, int index, int count, int contextIndex,
        int contextCount, bool isRtl, std::vector<float>* advances, int advancesIndex)const{
    const minikin::U32StringPiece textBuf(chars.data(), chars.size());
    const minikin::Range range(index, index + count);
    const minikin::StartHyphenEdit startHyphen = static_cast<minikin::StartHyphenEdit>(getStartHyphenEdit());
    const minikin::EndHyphenEdit endHyphen = static_cast<minikin::EndHyphenEdit>(getEndHyphenEdit());
    auto bidiFlags = isRtl ? minikin::Bidi::FORCE_RTL : minikin::Bidi::FORCE_LTR;
    return minikin::Layout::measureText(textBuf, range, bidiFlags, *mMinikinPaint, startHyphen,
                                        endHyphen, advances?advances->data():nullptr);
}

float Paint::getTextRunCursor(const CharSequence* text, int start, int count, bool isRtl, int offset, int cursorOpt)const{
    return 100.f;
}

float Paint::getRunAdvance(const CharSequence* text, int start, int end, int contextStart, int contextEnd, bool isRtl, int offset)const{
    std::vector<char32_t>buf(end-start);
    TextUtils::getChars(text, start, end, buf, 0);
    return getRunAdvance(buf,0,end-start,contextStart,contextEnd,isRtl,offset);;
}

float Paint::getTextRunCursor(const std::vector<char32_t>& text, int start, int count, bool isRtl, int offset, int cursorOpt)const{
    const minikin::Bidi bidiFlags = isRtl ? minikin::Bidi::FORCE_RTL : minikin::Bidi::FORCE_LTR;
    const minikin::U32StringPiece textBuf((const char32_t*)text.data(), text.size());
    const minikin::Range range(start, start + count);
    const minikin::StartHyphenEdit startHyphen = static_cast<minikin::StartHyphenEdit>(getStartHyphenEdit());
    const minikin::EndHyphenEdit endHyphen = static_cast<minikin::EndHyphenEdit>(getEndHyphenEdit());

    return minikin::Layout::measureText(textBuf, range, bidiFlags, *mMinikinPaint, startHyphen,
                                        endHyphen, nullptr);
}

float Paint::getRunAdvance(const std::vector<char32_t>& text, int start, int end, int contextStart, int contextEnd, bool isRtl, int offset)const{
    const minikin::U32StringPiece textBuf(text.data(), text.size());
    const minikin::Range range(start, end);
    const minikin::StartHyphenEdit startHyphen = static_cast<minikin::StartHyphenEdit>(getStartHyphenEdit());
    const minikin::EndHyphenEdit endHyphen = static_cast<minikin::EndHyphenEdit>(getEndHyphenEdit());
    auto bidiFlags = isRtl ? minikin::Bidi::FORCE_RTL : minikin::Bidi::FORCE_LTR;
    return minikin::Layout::measureText(textBuf, range, bidiFlags, *mMinikinPaint, startHyphen, endHyphen, nullptr);
}
void Paint::drawTextRun(Canvas&c,const std::vector<char32_t>&chars,int start,int count,
        int contextStart,int contextCount,float x,float y,bool runIsRtl)const{
    minikin::U32StringPiece lineTextPiece(chars.data() + start, count);
    minikin::Layout layout(lineTextPiece, minikin::Range(0, count),
                               minikin::Bidi::DEFAULT_LTR, *mMinikinPaint,
                               minikin::StartHyphenEdit::NO_EDIT,
                               minikin::EndHyphenEdit::NO_EDIT);
    const minikin::MinikinFont* currentFont = nullptr;
    Cairo::RefPtr<Cairo::FtScaledFont> currentCairoFontFace = nullptr;
    float fontSize = getTextSize();
    size_t glyphIdx=0;
    while(glyphIdx < layout.nGlyphs()) {
        const minikin::MinikinFont* glyphFont = layout.getFont(glyphIdx);
        if (glyphFont != currentFont) {

            currentFont = glyphFont;
            auto fullFont = dynamic_cast<const FullMinikinFont*>(glyphFont);
            if (fullFont) {
                Cairo::RefPtr<Cairo::FtScaledFont> scaledFont = fullFont->getScaledFont(fontSize);
                currentCairoFontFace = scaledFont;
                c.set_scaled_font(currentCairoFontFace);
                //LOGD("%d switch font %d",glyphIdx,fullFont->GetUniqueId());
            } else {
                glyphIdx++;
                continue;
            }
        }

        std::vector<cairo_glyph_t> cairoGlyphs;
        while (glyphIdx < layout.nGlyphs() && layout.getFont(glyphIdx) == currentFont) {
            cairo_glyph_t glyph;
            glyph.index = layout.getGlyphId(glyphIdx);
            glyph.x = x + layout.getX(glyphIdx);
            glyph.y = y + layout.getY(glyphIdx);
            cairoGlyphs.push_back(glyph);
            glyphIdx++;
        }
        c.show_glyphs(cairoGlyphs);
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TextPaint::TextPaint():Paint(){
}

TextPaint::TextPaint(int flags):Paint(flags){
}

TextPaint::TextPaint(const Paint& p):Paint(p){
}

void TextPaint::set(const Paint& tp) {
    Paint::set(tp);
    bgColor = ((TextPaint&)tp).bgColor;
    baselineShift = ((TextPaint&)tp).baselineShift;
    linkColor = ((TextPaint&)tp).linkColor;
    drawableState = ((TextPaint&)tp).drawableState;
    density = ((TextPaint&)tp).density;
    underlineColor = ((TextPaint&)tp).underlineColor;
    underlineThickness = ((TextPaint&)tp).underlineThickness;
}

bool TextPaint::hasEqualAttributes(const TextPaint& other) const{
    return bgColor == other.bgColor
            && baselineShift == other.baselineShift
            && linkColor == other.linkColor
            && drawableState == other.drawableState
            && density == other.density
            && underlineColor == other.underlineColor
            && underlineThickness == other.underlineThickness
            && Paint::hasEqualAttributes((Paint&) other);
}
/*void TextPaint::setUnderlineText(bool underline) {
}
void TextPaint::setUnderlineText(int color, float thickness) {
    underlineColor = color;
    underlineThickness = thickness;
}
bool TextPaint::isUnderlineText()const{return false;}
bool TextPaint::equalsForTextMeasurement(const TextPaint&)const{
    return false;
}*/

}/*endof namespace*/
