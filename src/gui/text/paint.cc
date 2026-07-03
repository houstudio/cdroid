#include <memory>
#include <iostream>
#include <text/paint.h>
#include <text/textutils.h>
#include <text/spannablestring.h>
#include <minikin/GraphemeBreak.h>
#include <minikin/LocaleList.h>
#include <minikin/Measurement.h>
#include <minikin/MeasuredText.h>
#include <minikin/MinikinFont.h>
#include <minikin/Layout.h>
#include <minikin/MinikinRect.h>
#include <cmath>
#include <core/typeface.h>
#include <core/canvas.h>
#include <core/path.h>
#include <core/pathmeasure.h>
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
    mFakeBoldText=false;
    mStrikeThruText=false;
    mUnderlineText=false;
    mUnderlinePosition=0;
    mUnderlineThickness=0;
    mStrikeThruPosition=0;
    mStrikeThruThickness=0;
    mMinikinPaint = std::make_shared<minikin::MinikinPaint>(mTypeface->getFontCollection());
    mMinikinPaint->size=12;
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
    mShader = o.mShader;
    mTextSize=o.mTextSize;
    mTextSkewX=o.mTextSkewX;
    mTextScaleX=o.mTextScaleX;
    mAntialias = o.mAntialias;
    mStartHyphenEdit = o.mStartHyphenEdit;
    mEndHyphenEdit = o.mEndHyphenEdit;
    mWordSpace = o.mWordSpace;
    mLetterSpacing = o.mLetterSpacing;
    mMinikinPaint=o.mMinikinPaint;
    mFakeBoldText = o.mFakeBoldText;
    mStrikeThruText = o.mStrikeThruText;
    mUnderlineText = o.mUnderlineText;
    mStyle = o.mStyle;
    mStrokeWidth = o.mStrokeWidth;
    mUnderlinePosition = o.mUnderlinePosition;
    mUnderlineThickness = o.mUnderlineThickness;
    mStrikeThruPosition = o.mStrikeThruPosition;
    mStrikeThruThickness = o.mStrikeThruThickness;
}

void Paint::setTextSize(float v){
    mTextSize = v;
    mMinikinPaint->size = v;
}

void Paint::setTextScaleX(float v){
    mTextScaleX =v;
    mMinikinPaint->scaleX = v;
}

void Paint::setLetterSpacing(float v){
    mLetterSpacing = v;
    mMinikinPaint->letterSpacing = v;   // Android: Paint.letterSpacing → MinikinPaint
}

void Paint::setTextSkewX(float v){
    mTextSkewX = v;
    mMinikinPaint->skewX = v;
}

float Paint::ascent()const{
    minikin::MinikinExtent extent;
    minikin::FontFakery  ffk;
    auto minikinFont = mTypeface->getMinikinFont();
    minikinFont->GetFontExtent(&extent,*mMinikinPaint,ffk);
    return extent.ascent;
}

float Paint::descent()const{
    minikin::MinikinExtent extent;
    minikin::FontFakery  ffk;
    auto minikinFont = mTypeface->getMinikinFont();
    minikinFont->GetFontExtent(&extent,*mMinikinPaint,ffk);
    return extent.descent;
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

void Paint::getTextBounds(const std::u16string& text, int start, int end, Rect& bounds) const {
    getTextBounds(text.c_str(), start, end - start, bounds);
}

void Paint::getTextBounds(const CharSequence* text, int start, int end, Rect& bounds) const {
    std::vector<char16_t> buf(end - start);
    TextUtils::getChars(text, start, end, buf.data(), 0);
    getTextBounds(buf.data(), 0, end - start, bounds);
}

void Paint::getTextBounds(const char16_t* text, int index, int count, Rect& bounds) const {
    const minikin::U16StringPiece textBuf((const uint16_t*)(text + index), count);
    const minikin::Range range(0, count);
    const minikin::StartHyphenEdit startHyphen = static_cast<minikin::StartHyphenEdit>(getStartHyphenEdit());
    const minikin::EndHyphenEdit endHyphen = static_cast<minikin::EndHyphenEdit>(getEndHyphenEdit());
    const minikin::Bidi bidiFlags = minikin::Bidi::FORCE_LTR;

    // Lay out the run, then union each glyph's ink bounds (offset by its pen position)
    // to form the smallest rectangle enclosing all characters, origin at (0,0).
    const minikin::Layout layout(textBuf, range, bidiFlags, *mMinikinPaint, startHyphen, endHyphen);

    minikin::MinikinRect r;
    for (size_t i = 0; i < layout.nGlyphs(); i++) {
        minikin::MinikinRect g;
        const std::shared_ptr<minikin::MinikinFont>& minikinFont = layout.getFontRef(i)->typeface();
        minikinFont->GetBounds(&g, layout.getGlyphId(i), *mMinikinPaint, layout.getFakery(i));
        g.offset(layout.getX(i), layout.getY(i));
        r.join(g);
    }

    if (r.isEmpty()) {
        bounds.setEmpty();
    } else {
        bounds = Rect::MakeLTRB((int) std::floor(r.mLeft), (int) std::floor(r.mTop),
                                (int) std::ceil(r.mRight), (int) std::ceil(r.mBottom));
    }
}

float Paint::getTextRunCursor(const CharSequence* text, int start, int count, bool isRtl, int offset, int cursorOpt)const{
    std::vector<char16_t>buf(count);
    TextUtils::getChars(text, start, start + count, buf.data(), 0);
    return getTextRunCursor(buf.data(), 0, count, isRtl, offset - start, cursorOpt);
}

float Paint::getRunAdvance(const CharSequence* text, int start, int end, int contextStart, int contextEnd, bool isRtl, int offset)const{
    // Copy the run [start, end) into a 0-based buffer and translate the absolute
    // offset/context into buffer-local coordinates before delegating. (Previously
    // the absolute offset was passed unchanged against a sliced buffer.)
    std::vector<char16_t>buf(end-start);
    TextUtils::getChars(text, start, end, buf.data(), 0);
    return getRunAdvance(buf.data(), /*start=*/0, /*end=*/end-start,
                         /*contextStart=*/0, /*contextEnd=*/end-start, isRtl,
                         /*offset=*/offset - start);
}

float Paint::getTextRunCursor(const char16_t* text, int start, int count, bool isRtl, int offset, int cursorOpt)const{
    // Grapheme-cluster cursor offset nearest to `offset` per `cursorOpt`, via
    // minikin's GraphemeBreak. (Previously returned a constant 100.f, which broke
    // cursor left/right movement and tap snapping for any non-trivial run.)
    std::vector<float> advances(count, 0.f);
    const minikin::Bidi bidiFlags = isRtl ? minikin::Bidi::FORCE_RTL : minikin::Bidi::FORCE_LTR;
    const minikin::U16StringPiece textBuf((const uint16_t*)text, start+count);
    const minikin::Range range(start, start + count);
    const minikin::StartHyphenEdit startHyphen = static_cast<minikin::StartHyphenEdit>(getStartHyphenEdit());
    const minikin::EndHyphenEdit endHyphen = static_cast<minikin::EndHyphenEdit>(getEndHyphenEdit());
    minikin::Layout::measureText(textBuf, range, bidiFlags, *mMinikinPaint, startHyphen, endHyphen, advances.data());
    const auto opt = static_cast<minikin::GraphemeBreak::MoveOpt>(cursorOpt);
    const size_t result = minikin::GraphemeBreak::getTextRunCursor(
            advances.data(), (const uint16_t*)text, start, count, (size_t)offset, opt);
    return (float)result;
}

float Paint::getRunAdvance(const char16_t* text, int start, int end, int contextStart, int contextEnd, bool isRtl, int offset)const{
    // Advance from `start` up to `offset` within the run [start, end). Measure the
    // per-code-unit advances over the run and sum the leading [start, offset)
    // slice. (Previously this ignored `offset` and returned the whole-run width,
    // which left the caret pinned to the line edge for every offset.)
    if (offset < start) offset = start;
    if (offset > end) offset = end;
    const int count = end - start;
    if (count <= 0) return 0.f;
    std::vector<float> advances(count, 0.f);
    const minikin::U16StringPiece textBuf((const uint16_t*)text, end);
    const minikin::Range range(start, end);
    const minikin::StartHyphenEdit startHyphen = static_cast<minikin::StartHyphenEdit>(getStartHyphenEdit());
    const minikin::EndHyphenEdit endHyphen = static_cast<minikin::EndHyphenEdit>(getEndHyphenEdit());
    auto bidiFlags = isRtl ? minikin::Bidi::FORCE_RTL : minikin::Bidi::FORCE_LTR;
    minikin::Layout::measureText(textBuf, range, bidiFlags, *mMinikinPaint, startHyphen, endHyphen, advances.data());
    float adv = 0.f;
    const int limit = offset - start;   // advances[] is 0-based within [start, end)
    for (int i = 0; i < limit; i++) adv += advances[i];
    return adv;
}

void Paint::drawTextRun(Canvas&c,const char16_t*chars,int start,int count,
        int contextStart,int contextCount,float x,float y,bool runIsRtl)const{
    minikin::U16StringPiece lineTextPiece((const uint16_t*)chars + start, count);
    auto bidiFlags = runIsRtl ? minikin::Bidi::FORCE_RTL : minikin::Bidi::FORCE_LTR;
    minikin::Layout layout(lineTextPiece, minikin::Range(0, count),
                               bidiFlags, *mMinikinPaint,
                               minikin::StartHyphenEdit::NO_EDIT,
                               minikin::EndHyphenEdit::NO_EDIT);
    // Honor Paint.Align on the run origin, matching Android's Canvas.drawText
    // (where Paint::alignText shifts x by the run advance). cdroid has no
    // separate drawText() wrapper -- every text path (Layout, TextView, ...) is
    // routed through drawTextRun -- so the align is applied here. Align::LEFT is
    // the default and a no-op, leaving the Layout/TextView path unchanged.
    const float runAdvance = layout.getAdvance();
    switch (mTextAlign) {
    case Align::CENTER: x -= runAdvance / 2.f; break;
    case Align::RIGHT:  x -= runAdvance;        break;
    case Align::LEFT:
    default: break;
    }
    std::shared_ptr<const minikin::Font> currentFontRef = nullptr;
    Cairo::RefPtr<Cairo::FtScaledFont> currentCairoFontFace = nullptr;
    size_t glyphIdx=0;
    if (mShader) {
        c.set_source(mShader);
    } else {
        c.set_color(getColor());
    }
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

void Paint::drawTextOnPath(Canvas& canvas, const std::string& text,
        const Path& path, float hOffset, float vOffset)const {
    std::u16string u16text(text.begin(), text.end());
    drawTextOnPath(canvas, u16text.c_str(), 0, u16text.length(), path, hOffset, vOffset);
}

void Paint::drawTextOnPath(Canvas& canvas, const char16_t* text, int index, int count,
        const Path& path, float hOffset, float vOffset)const {
    if (count <= 0) return;

    minikin::U16StringPiece lineTextPiece((const uint16_t*)text + index, count);
    minikin::Layout layout(lineTextPiece, minikin::Range(0, count),
                           minikin::Bidi::FORCE_LTR, *mMinikinPaint,
                           minikin::StartHyphenEdit::NO_EDIT,
                           minikin::EndHyphenEdit::NO_EDIT);

    const std::vector<float>& advances = layout.getAdvances();

    Cairo::RefPtr<cdroid::Path> pathRef = Cairo::RefPtr<cdroid::Path>(new cdroid::Path(path));
    PathMeasure measure(pathRef, false);
    double pathLen = measure.getLength();
    double currentPathPos = hOffset;

    for (size_t glyphIdx = 0; glyphIdx < layout.nGlyphs(); glyphIdx++) {
        const std::shared_ptr<const minikin::Font>& glyphFontRef = layout.getFontRef(glyphIdx);
        const minikin::MinikinFont* minikinFont = glyphFontRef->typeface().get();
        
        if (mTypeface != nullptr && mMinikinPaint != nullptr) {
            auto scaledFont = mTypeface->getScaledFont(*mMinikinPaint, minikinFont);
            Cairo::RefPtr<Cairo::FtScaledFont> cairoFont = std::dynamic_pointer_cast<Cairo::FtScaledFont>(scaledFont);
            if (cairoFont) {
                canvas.set_scaled_font(cairoFont);
            } else {
                continue;
            }
        }

        if (currentPathPos > pathLen) {
            break;
        }
        double pos[2], tan[2];
        const float glyphAdvance = advances[glyphIdx];
        if (measure.getPosTan(currentPathPos+glyphAdvance/2.0, pos, tan)) {
            double angle = atan2(tan[1], tan[0]);

            canvas.save();
            canvas.translate(pos[0], pos[1]);
            canvas.rotate(angle);
            canvas.translate(0, vOffset);

            cairo_glyph_t glyph;
            glyph.index = layout.getGlyphId(glyphIdx);
            glyph.x = -glyphAdvance/2.0;
            glyph.y = 0;
            canvas.show_glyphs(std::vector<cairo_glyph_t>{glyph});
            canvas.restore();
        }
        currentPathPos += glyphAdvance;
    }
}
}/*endof namespace*/
