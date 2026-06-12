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

//////////////////////////////////////////////////////////////////////////////////////////////////////////

Paint::Paint(){
    mTypeface = Typeface::DEFAULT;
    mStartHyphenEdit=0;
    mEndHyphenEdit=0;
    mWordSpace=0;
    mAntialias=false;
    mLetterSpacing=0;
    mTextScaleX=1.f;
    mTextSize=12;
    mMinikinPaint=std::make_shared<minikin::MinikinPaint>(mTypeface->getFontCollection());
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
    mMinikinPaint=o.mMinikinPaint;
}

bool Paint::hasEqualAttributes(const Paint&other)const{
    return true;
}

float Paint::measureText(const std::string& text)const{
    std::u16string u16=TextUtils::utf8_utf16(text);
    return measureText(u16.c_str(),0,u16.size());
}

float Paint::measureText(const std::string& text, int start, int end)const{
    std::u16string u16=TextUtils::utf8_utf16(text.substr(start,end-start));
    return measureText(u16.c_str(),0,u16.size());
}

float Paint::measureText(const CharSequence* text, int start, int end)const{
    std::vector<char16_t>buf(end-start);
    TextUtils::getChars(text, start, end, buf.data(), 0);
    return measureText(buf.data(),0,end-start);
}

float Paint::measureText(const char16_t* text, int index, int count)const{
    const minikin::U16StringPiece textBuf((const uint16_t*)(text+index), count);
    const minikin::Range range(0, count);
    const minikin::StartHyphenEdit startHyphen = static_cast<minikin::StartHyphenEdit>(getStartHyphenEdit());
    const minikin::EndHyphenEdit endHyphen = static_cast<minikin::EndHyphenEdit>(getEndHyphenEdit());
    auto bidiFlags = false/*isRtl*/ ? minikin::Bidi::FORCE_RTL : minikin::Bidi::FORCE_LTR;
    return minikin::Layout::measureText(textBuf, range, bidiFlags, *mMinikinPaint, startHyphen,
                                        endHyphen, nullptr);
}
void Paint::getFontMetricsInt(const CharSequence* text, int start, int count,
    int contextStart, int contextCount,bool isRtl,FontMetricsInt& outMetrics)const{
    LOGD("TODO");
}

Paint::FontMetricsInt Paint::getFontMetricsInt()const{
    FontMetricsInt fm;
    getFontMetricsInt(&fm);
    return fm;
}
int Paint::getFontMetricsInt(FontMetricsInt* fmi)const{
    std::shared_ptr<minikin::MinikinFont> minikinFont = mTypeface->getMinikinFont();
    minikin::MinikinExtent extent;
    minikinFont->GetFontExtent(&extent, *mMinikinPaint, minikin::FontFakery());
    if(fmi){
        fmi->ascent = extent.ascent;
        fmi->descent = extent.descent;
        fmi->leading = 0;
        fmi->top = fmi->ascent;           // top 等于 ascent
        fmi->bottom = fmi->descent;       // bottom 等于 descent
    }
    return extent.descent - extent.ascent;
}

float Paint::getTextRunAdvances(const char16_t* chars, int index, int count, int contextIndex,
        int contextCount, bool isRtl, float* advances, int advancesIndex)const{
    const minikin::U16StringPiece textBuf((const uint16_t*)chars, index+count);
    const minikin::Range range(index, index + count);
    const minikin::StartHyphenEdit startHyphen = static_cast<minikin::StartHyphenEdit>(getStartHyphenEdit());
    const minikin::EndHyphenEdit endHyphen = static_cast<minikin::EndHyphenEdit>(getEndHyphenEdit());
    auto bidiFlags = isRtl ? minikin::Bidi::FORCE_RTL : minikin::Bidi::FORCE_LTR;
    return minikin::Layout::measureText(textBuf, range, bidiFlags, *mMinikinPaint, startHyphen,
                                        endHyphen, advances);
}

float Paint::getTextRunCursor(const CharSequence* text, int start, int count, bool isRtl, int offset, int cursorOpt)const{
    return 100.f;
}

float Paint::getRunAdvance(const CharSequence* text, int start, int end, int contextStart, int contextEnd, bool isRtl, int offset)const{
    std::vector<char16_t>buf(end-start);
    TextUtils::getChars(text, start, end, buf.data(), 0);
    return getRunAdvance(buf.data(),0,end-start,contextStart,contextEnd,isRtl,offset);;
}

float Paint::getTextRunCursor(const char16_t* text, int start, int count, bool isRtl, int offset, int cursorOpt)const{
    const minikin::Bidi bidiFlags = isRtl ? minikin::Bidi::FORCE_RTL : minikin::Bidi::FORCE_LTR;
    const minikin::U16StringPiece textBuf((const uint16_t*)text, start+count);
    const minikin::Range range(start, start + count);
    const minikin::StartHyphenEdit startHyphen = static_cast<minikin::StartHyphenEdit>(getStartHyphenEdit());
    const minikin::EndHyphenEdit endHyphen = static_cast<minikin::EndHyphenEdit>(getEndHyphenEdit());

    return minikin::Layout::measureText(textBuf, range, bidiFlags, *mMinikinPaint, startHyphen,
                                        endHyphen, nullptr);
}

float Paint::getRunAdvance(const char16_t* text, int start, int end, int contextStart, int contextEnd, bool isRtl, int offset)const{
    const minikin::U16StringPiece textBuf((const uint16_t*)text, end);
    const minikin::Range range(start, end);
    const minikin::StartHyphenEdit startHyphen = static_cast<minikin::StartHyphenEdit>(getStartHyphenEdit());
    const minikin::EndHyphenEdit endHyphen = static_cast<minikin::EndHyphenEdit>(getEndHyphenEdit());
    auto bidiFlags = isRtl ? minikin::Bidi::FORCE_RTL : minikin::Bidi::FORCE_LTR;
    return minikin::Layout::measureText(textBuf, range, bidiFlags, *mMinikinPaint, startHyphen, endHyphen, nullptr);
}
void Paint::drawTextRun(Canvas&c,const char16_t*chars,int start,int count,
        int contextStart,int contextCount,float x,float y,bool runIsRtl)const{
    minikin::U16StringPiece lineTextPiece((const uint16_t*)chars + start, count);
    auto bidiFlags = runIsRtl ? minikin::Bidi::FORCE_RTL : minikin::Bidi::FORCE_LTR;
    minikin::Layout layout(lineTextPiece, minikin::Range(0, count),
                               bidiFlags, *mMinikinPaint,
                               minikin::StartHyphenEdit::NO_EDIT,
                               minikin::EndHyphenEdit::NO_EDIT);
    std::shared_ptr<const minikin::Font> currentFontRef = nullptr;
    Cairo::RefPtr<Cairo::FtScaledFont> currentCairoFontFace = nullptr;
    size_t glyphIdx=0;
    while(glyphIdx < layout.nGlyphs()) {
        // minikin14: getFontRef 返回 shared_ptr<Font>，通过 typeface() 获取 MinikinFont
        const std::shared_ptr<const minikin::Font>& glyphFontRef = layout.getFontRef(glyphIdx);
        if (glyphFontRef != currentFontRef) {
            currentFontRef = glyphFontRef;
            // 从 Font 获取底层的 MinikinFont
            const minikin::MinikinFont* minikinFont = glyphFontRef->typeface().get();
            if (mTypeface != nullptr && mMinikinPaint != nullptr) {
                // 使用 Typeface::getScaledFont，传入布局中实际使用的 MinikinFont
                auto scaledFont = mTypeface->getScaledFont(*mMinikinPaint, minikinFont);
                currentCairoFontFace = std::dynamic_pointer_cast<Cairo::FtScaledFont>(scaledFont);
                if (currentCairoFontFace) {
                    c.set_scaled_font(currentCairoFontFace);
                } else {
                    glyphIdx++;
                    continue;
                }
            } else {
                glyphIdx++;
                continue;
            }
        }

        std::vector<cairo_glyph_t> cairoGlyphs;
        while (glyphIdx < layout.nGlyphs() && layout.getFontRef(glyphIdx) == currentFontRef) {
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

}/*endof namespace*/
