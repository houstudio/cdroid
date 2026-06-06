#if 0
#include <text/boringlayout.h>
namespace cdroid{

BoringLayout* BoringLayout::make(CharSequence* source, TextPaint* paint, int outerWidth, Alignment align,
        float spacingMult, float spacingAdd, BoringLayout::Metrics& metrics, bool includePad) {
    return new BoringLayout(source, paint, outerWidth, align, spacingMult, spacingAdd, metrics,
            includePad);
}

BoringLayout* BoringLayout::make(CharSequence* source, TextPaint* paint, int outerWidth, Alignment align,
        float spacingmult, float spacingadd, BoringLayout::Metrics& metrics,
        bool includePad, TextUtils::TruncateAt ellipsize, int ellipsizedWidth) {
    return new BoringLayout(source, paint, outerWidth, align, spacingmult, spacingadd, metrics,
            includePad, ellipsize, ellipsizedWidth);
}

BoringLayout* BoringLayout::make(CharSequence* source, TextPaint* paint,int outerWidth, Alignment align,
        BoringLayout::Metrics& metrics, bool includePad, TextUtils::TruncateAt ellipsize,
        int ellipsizedWidth, bool useFallbackLineSpacing) {
    return new BoringLayout(source, paint, outerWidth, align, 1.f, 0.f, metrics, includePad,
            ellipsize, ellipsizedWidth, useFallbackLineSpacing);
}

BoringLayout* BoringLayout::replaceOrMake(CharSequence* source, TextPaint* paint, int outerwidth,
        Alignment align, float spacingMult, float spacingAdd, BoringLayout::Metrics& metrics, bool includePad) {
    replaceWith(source, paint, outerwidth, align, spacingMult, spacingAdd);

    mEllipsizedWidth = outerwidth;
    mEllipsizedStart = 0;
    mEllipsizedCount = 0;
    mUseFallbackLineSpacing = false;

    init(source, paint, align, metrics, includePad, true, false /* useFallbackLineSpacing */);
    return this;
}

BoringLayout* BoringLayout::replaceOrMake(CharSequence* source, TextPaint* paint, int outerWidth,Alignment align,
        BoringLayout::Metrics& metrics, bool includePad, TextUtils::TruncateAt ellipsize, int ellipsizedWidth, bool useFallbackLineSpacing) {
    return replaceOrMake(source, paint, outerWidth, align, 1.0f, 0.0f, metrics, includePad,
            ellipsize, ellipsizedWidth, useFallbackLineSpacing, false /* useBoundsForWidth */,
            nullptr /* minimumFontMetrics */);
}

/** @hide */
BoringLayout* BoringLayout::replaceOrMake(CharSequence* source, TextPaint* paint, int outerWidth, Alignment align,
        float spacingMultiplier, float spacingAmount, BoringLayout::Metrics& metrics, bool includePad,
        TextUtils::TruncateAt ellipsize, int ellipsizedWidth, bool useFallbackLineSpacing, bool useBoundsForWidth,
        Paint::FontMetrics* minimumFontMetrics) {
    bool trust;

    if (ellipsize == nullptr || ellipsize == TextUtils::TruncateAt::MARQUEE) {
        replaceWith(source, paint, outerWidth, align, 1.f, 0.f);

        mEllipsizedWidth = outerWidth;
        mEllipsizedStart = 0;
        mEllipsizedCount = 0;
        trust = true;
    } else {
        replaceWith(TextUtils::ellipsize(source, paint, ellipsizedWidth, ellipsize, true, this),
                paint, outerWidth, align, spacingMultiplier, spacingAmount);

        mEllipsizedWidth = ellipsizedWidth;
        trust = false;
    }

    mUseFallbackLineSpacing = useFallbackLineSpacing;

    init(getText(), paint, align, metrics, includePad, trust,
            useFallbackLineSpacing);
    return this;
}

BoringLayout* BoringLayout::replaceOrMake(CharSequence* source, TextPaint* paint, int outerWidth,
        Alignment align, float spacingMult, float spacingAdd, BoringLayout::Metrics& metrics,
        bool includePad, TextUtils::TruncateAt ellipsize, int ellipsizedWidth) {
    return replaceOrMake(source, paint, outerWidth, align, metrics,
            includePad, ellipsize, ellipsizedWidth, false /* useFallbackLineSpacing */);
}

BoringLayout::BoringLayout(CharSequence* source, TextPaint* paint, int outerwidth, Alignment align,
        float spacingMult, float spacingAdd, BoringLayout::Metrics &metrics, bool includePad)
    :TextLayout(source, paint, outerwidth, align, TextDirectionHeuristics.LTR, spacingMult,
            spacingAdd, includePad, false /* fallbackLineSpacing */,
            outerwidth /* ellipsizedWidth */, nullptr /* ellipsize */, 1 /* maxLines */,
            BREAK_STRATEGY_SIMPLE, HYPHENATION_FREQUENCY_NONE, nullptr /* leftIndents */,
            nullptr /* rightIndents */, JUSTIFICATION_MODE_NONE, LineBreakConfig::NONE, false,
            false /* shiftDrawingOffsetForStartOverhang */, nullptr){

    mEllipsizedWidth = outerwidth;
    mEllipsizedStart = 0;
    mEllipsizedCount = 0;
    mUseFallbackLineSpacing = false;

    init(source, paint, align, metrics, includePad, true, false /* useFallbackLineSpacing */);
}

BoringLayout::BoringLayout(CharSequence* source, TextPaint* paint, int outerWidth, Alignment align,
        float spacingMult, float spacingAdd, BoringLayout::Metrics& metrics, bool includePad,
        TextUtils::TruncateAt ellipsize, int ellipsizedWidth)
    :BoringLayout(source, paint, outerWidth, align, spacingMult, spacingAdd, metrics, includePad,
            ellipsize, ellipsizedWidth, false /* fallbackLineSpacing */){
}

BoringLayout::BoringLayout(CharSequence* source, TextPaint* paint, int outerWidth, Alignment align, float spacingMult,
        float spacingAdd, BoringLayout::Metrics& metrics, bool includePad, TextUtils::TruncateAt ellipsize, int ellipsizedWidth,
        bool useFallbackLineSpacing)
    :BoringLayout(source, paint, outerWidth, align, TextDirectionHeuristics::LTR, spacingMult,
            spacingAdd, includePad, useFallbackLineSpacing,
            ellipsizedWidth, ellipsize, 1 /* maxLines */,
            BREAK_STRATEGY_SIMPLE, HYPHENATION_FREQUENCY_NONE, nullptr /* leftIndents */,
            nullptr /* rightIndents */, JUSTIFICATION_MODE_NONE,
            LineBreakConfig::NONE, metrics, false /* useBoundsForWidth */,
            false /* shiftDrawingOffsetForStartOverhang */, nullptr){
}

BoringLayout::BoringLayout(CharSequence* text, TextPaint* paint, int width, Alignment align,
        float spacingMult, float spacingAdd, bool includePad, bool fallbackLineSpacing,
        int ellipsizedWidth, TextUtils::TruncateAt ellipsize, Metrics& metrics,
        bool useBoundsForWidth, bool shiftDrawingOffsetForStartOverhang,
        Paint::FontMetrics minimumFontMetrics)
    :BoringLayout(text, paint, width, align, TextDirectionHeuristics::LTR,
            spacingMult, spacingAdd, includePad, fallbackLineSpacing, ellipsizedWidth,
            ellipsize, 1 /* maxLines */, TextLayout::BREAK_STRATEGY_SIMPLE,
            TextLayout::HYPHENATION_FREQUENCY_NONE, nullptr, nullptr, TextLayout::JUSTIFICATION_MODE_NONE,
            LineBreakConfig::NONE, metrics, useBoundsForWidth,
            shiftDrawingOffsetForStartOverhang, minimumFontMetrics){
}

BoringLayout::BoringLayout(CharSequence* text, TextPaint* paint, int width, Alignment align,
        const TextDirectionHeuristic* textDir, float spacingMult, float spacingAdd, bool includePad,
        bool fallbackLineSpacing, int ellipsizedWidth, TextUtils::TruncateAt ellipsize,
        int maxLines, int breakStrategy, int hyphenationFrequency,
        const std::vector<int>& leftIndents, const std::vector<int>& rightIndents,
        int justificationMode, LineBreakConfig lineBreakConfig, Metrics& metrics,
        bool useBoundsForWidth, bool shiftDrawingOffsetForStartOverhang,
        Paint::FontMetrics minimumFontMetrics)
    :TextLayout(text, paint, width, align, textDir, spacingMult, spacingAdd, includePad,
            fallbackLineSpacing, ellipsizedWidth, ellipsize, maxLines, breakStrategy,
            hyphenationFrequency, leftIndents, rightIndents, justificationMode,
            lineBreakConfig, useBoundsForWidth, shiftDrawingOffsetForStartOverhang,
            minimumFontMetrics){


    bool trust;

    if (ellipsize == TextUtils::TruncateAt::NONE || ellipsize == TextUtils::TruncateAt::MARQUEE) {
        mEllipsizedWidth = width;
        mEllipsizedStart = 0;
        mEllipsizedCount = 0;
        trust = true;
    } else {
        replaceWith(TextUtils::ellipsize(text, paint, ellipsizedWidth, ellipsize, true, this),
                    paint, width, align, spacingMult, spacingAdd);

        mEllipsizedWidth = ellipsizedWidth;
        trust = false;
    }

    mUseFallbackLineSpacing = fallbackLineSpacing;
    init(getText(), paint, align, metrics, includePad, trust, fallbackLineSpacing);
}

void BoringLayout::init(CharSequence* source, TextPaint* paint, Alignment align,
        BoringLayout::Metrics& metrics, bool includePad, bool trustWidth, bool useFallbackLineSpacing) {
    int spacing;

    if (source instanceof String && align == Layout::Alignment::ALIGN_NORMAL) {
        mDirect = source.toString();
    } else {
        mDirect = nullptr;
    }

    mPaint = paint;

    if (includePad) {
        spacing = metrics.bottom - metrics.top;
        mDesc = metrics.bottom;
    } else {
        spacing = metrics.descent - metrics.ascent;
        mDesc = metrics.descent;
    }

    mBottom = spacing;

    if (trustWidth) {
        mMax = metrics.width;
    } else {
        /*
         * If we have ellipsized, we have to actually calculate the
         * width because the width that was passed in was for the
         * full text, not the ellipsized form.
         */
        TextLine* line = TextLine::obtain();
        line->set(paint, source, 0, source->length(), TextLayout::DIR_LEFT_TO_RIGHT,
                TextLayout::DIRS_ALL_LEFT_TO_RIGHT, false, nullptr,
                mEllipsizedStart, mEllipsizedStart + mEllipsizedCount, useFallbackLineSpacing);
        mMax = (int) std::ceil(line->metrics(null, null, false, null));
        TextLine::recycle(line);
    }

    if (includePad) {
        mTopPadding = metrics.top - metrics.ascent;
        mBottomPadding = metrics.bottom - metrics.descent;
    }

    mDrawingBounds.set(metrics.mDrawingBounds);
    mDrawingBounds.offset(0, mBottom - mDesc);
}

BoringLayout::Metrics* BoringLayout::isBoring(CharSequence* text, TextPaint* paint) {
    return isBoring(text, paint, TextDirectionHeuristics::FIRSTSTRONG_LTR, nullptr);
}

BoringLayout::Metrics* BoringLayout::isBoring(CharSequence* text, TextPaint* paint, Metrics* metrics) {
    return isBoring(text, paint, TextDirectionHeuristics::FIRSTSTRONG_LTR, metrics);
}

bool BoringLayout::hasAnyInterestingChars(CharSequence* text, int textLength) {
    const int MAX_BUF_LEN = 500;
    char32_t* buffer = TextUtils::obtain(MAX_BUF_LEN);
    for (int start = 0; start < textLength; start += MAX_BUF_LEN) {
        const int end = std::min(start + MAX_BUF_LEN, textLength);

        // No need to worry about getting half codepoints, since we consider surrogate code
        // units "interesting" as soon we see one.
        TextUtils::getChars(text, start, end, buffer, 0);

        const int len = end - start;
        for (int i = 0; i < len; i++) {
            const char32_t c = buffer[i];
            if (c == '\n' || c == '\t' || TextUtils::couldAffectRtl(c)) {
                return true;
            }
        }
    }
    TextUtils::recycle(buffer);
    return false;
}

BoringLayout::Metrics* BoringLayout::isBoring(CharSequence* text, TextPaint* paint,
        const TextDirectionHeuristic* textDir, Metrics* metrics) {
    return isBoring(text, paint, textDir, false /* useFallbackLineSpacing */, metrics);
}

BoringLayout::Metrics* BoringLayout::isBoring(CharSequence* text, TextPaint* paint,
        const TextDirectionHeuristic* textDir, bool useFallbackLineSpacing, Metrics* metrics) {
    return isBoring(text, paint, textDir, useFallbackLineSpacing, nullptr, metrics);
}

BoringLayout::Metrics* BoringLayout::isBoring(CharSequence* text, TextPaint* paint,
        const TextDirectionHeuristic* textDir, bool useFallbackLineSpacing,
        Paint::FontMetrics* minimumFontMetrics, Metrics* metrics) {
    const int textLength = text->length();
    if (hasAnyInterestingChars(text, textLength)) {
       return nullptr;  // There are some interesting characters. Not boring.
    }
    if (textDir != nullptr && textDir->isRtl(text, 0, textLength)) {
       return nullptr;  // The heuristic considers the whole text RTL. Not boring.
    }
    if (dynamic_cast<Spanned*>(text)) {
        Spanned* sp = (Spanned*) text;
        auto styles = sp->getSpans(0, textLength, ParagraphStyle.class);
        if (styles.size() > 0) {
            return nullptr;  // There are some ParagraphStyle spans. Not boring.
        }
    }

    Metrics* fm = metrics;
    if (fm == nullptr) {
        fm = new Metrics();
    } else {
        fm->reset();
    }

    if (ClientFlags.fixLineHeightForLocale()) {
        if (minimumFontMetrics != nullptr) {
            fm->set(minimumFontMetrics);
            // Because the font metrics is provided by APIs, adjust the top/bottom with
            // ascent/descent: top must be smaller than ascent, bottom must be larger than
            // descent.
            fm->top = std::min(fm->top, fm->ascent);
            fm->bottom = std::max(fm->bottom, fm->descent);
        }
    }

    TextLine* line = TextLine::obtain();
    line->set(paint, text, 0, textLength, TextLayout::DIR_LEFT_TO_RIGHT,
            TextLayout::DIRS_ALL_LEFT_TO_RIGHT, false, nullptr,
            0 /* ellipsisStart, 0 since text has not been ellipsized at this point */,
            0 /* ellipsisEnd, 0 since text has not been ellipsized at this point */,
            useFallbackLineSpacing);
    fm->width = (int) std::ceil(line->metrics(fm, fm->mDrawingBounds, false, nullptr));
    TextLine::recycle(line);

    return fm;
}

int BoringLayout::getHeight() const{
    return mBottom;
}

int BoringLayout::getLineCount() const{
    return 1;
}

int BoringLayout::getLineTop(int line) const{
    if (line == 0)
        return 0;
    else
        return mBottom;
}

int BoringLayout::getLineDescent(int line) const{
    return mDesc;
}

int BoringLayout::getLineStart(int line) const{
    if (line == 0)
        return 0;
    else
        return getText()->length();
}

int BoringLayout::getParagraphDirection(int line) const{
    return DIR_LEFT_TO_RIGHT;
}

bool BoringLayout::getLineContainsTab(int line) const{
    return false;
}

float BoringLayout::getLineMax(int line) const{
    if (getUseBoundsForWidth()) {
        return TextLayout::getLineMax(line);
    } else {
        return mMax;
    }
}

float BoringLayout::getLineWidth(int line) const{
    if (getUseBoundsForWidth()) {
        return TextLayout::getLineWidth(line);
    } else {
        return (line == 0 ? mMax : 0);
    }
}

const Directions* BoringLayout::getLineDirections(int line) const{
    return &TextLayout::DIRS_ALL_LEFT_TO_RIGHT;
}

int BoringLayout::getTopPadding() const{
    return mTopPadding;
}

int BoringLayout::getBottomPadding() const{
    return mBottomPadding;
}

int BoringLayout::getEllipsisCount(int line) const{
    return mEllipsizedCount;
}

int BoringLayout::getEllipsisStart(int line) const{
    return mEllipsizedStart;
}

int BoringLayout::getEllipsizedWidth() const{
    return mEllipsizedWidth;
}

bool BoringLayout::isFallbackLineSpacingEnabled() const{
    return mUseFallbackLineSpacing;
}

RectF BoringLayout::computeDrawingBoundingBox() const{
    return mDrawingBounds;
}

void BoringLayout::draw(Canvas& c, Path* highlight, Paint* highlightpaint, int cursorOffset) {
    if (mDirect != nullptr && highlight == nullptr) {
        float leftShift = 0;
        if (getUseBoundsForWidth() && getShiftDrawingOffsetForStartOverhang()) {
            RectF drawingRect = computeDrawingBoundingBox();
            if (drawingRect.left < 0) {
                leftShift = -drawingRect.left;
                c.translate(leftShift, 0);
            }
        }

        c.drawText(mDirect, 0, mBottom - mDesc, mPaint);

        if (leftShift != 0) {
            // Manually translate back to the original position because of b/324498002, using
            // save/restore disappears the toggle switch drawables.
            c.translate(-leftShift, 0);
        }
    } else {
        TextLayout::draw(c, highlight, highlightpaint, cursorOffset);
    }
}

void BoringLayout::ellipsized(int start, int end) {
    mEllipsizedStart = start;
    mEllipsizedCount = end - start;
}

///////////////////////////////////////////////////////////////////////////////////////////////

RectF BoringLayout::Metrics::getDrawingBoundingBox() const{
    return mDrawingBounds;
}

void BoringLayout::Metrics::reset() {
    top = 0;
    bottom = 0;
    ascent = 0;
    descent = 0;
    width = 0;
    leading = 0;
    mDrawingBounds.setEmpty();
}
}/*endof namespace*/
#endif
