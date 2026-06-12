#include <text/boringlayout.h>
namespace cdroid{

BoringLayout* BoringLayout::make(CharSequence* source, TextPaint* paint, int outerWidth, Alignment align,
        float spacingMult, float spacingAdd, const BoringLayout::Metrics& metrics, bool includePad) {
    return new BoringLayout(source, paint, outerWidth, align, spacingMult, spacingAdd, metrics,
            includePad);
}

BoringLayout* BoringLayout::make(CharSequence* source, TextPaint* paint, int outerWidth, Alignment align,
        float spacingmult, float spacingadd, const BoringLayout::Metrics& metrics,
        bool includePad, TextUtils::TruncateAt ellipsize, int ellipsizedWidth) {
    return new BoringLayout(source, paint, outerWidth, align, spacingmult, spacingadd, metrics,
            includePad, ellipsize, ellipsizedWidth);
}

BoringLayout* BoringLayout::replaceOrMake(CharSequence* source, TextPaint* paint, int outerwidth,
        Alignment align, float spacingMult, float spacingAdd, const BoringLayout::Metrics& metrics, bool includePad) {
    replaceWith(source, paint, outerwidth, align, spacingMult, spacingAdd);

    mEllipsizedWidth = outerwidth;
    mEllipsizedStart = 0;
    mEllipsizedCount = 0;
    mUseFallbackLineSpacing = false;

    init(source, paint, align, metrics, includePad, true);
    return this;
}

BoringLayout* BoringLayout::replaceOrMake(CharSequence* source, TextPaint* paint, int outerWidth,
        Alignment align, float spacingMult, float spacingAdd,const BoringLayout::Metrics& metrics,
        bool includePad, TextUtils::TruncateAt ellipsize, int ellipsizedWidth) {
    bool trust;

    if (ellipsize == TextUtils::TruncateAt::NONE || ellipsize == TextUtils::TruncateAt::MARQUEE) {
        replaceWith(source, paint, outerWidth, align, spacingMult, spacingAdd);

        mEllipsizedWidth = outerWidth;
        mEllipsizedStart = 0;
        mEllipsizedCount = 0;
        trust = true;
    } else {
        replaceWith(TextUtils::ellipsize(source, *paint, ellipsizedWidth, ellipsize, true,
                    [this](int start,int end){ellipsized(start,end);}),
                paint, outerWidth, align, spacingMult, spacingAdd);

        mEllipsizedWidth = ellipsizedWidth;
        trust = false;
    }

    init(getText(), paint, align, metrics, includePad, trust);
    return this;
}

BoringLayout::BoringLayout(CharSequence* source, TextPaint* paint, int outerwidth, Alignment align,
        float spacingMult, float spacingAdd, const BoringLayout::Metrics &metrics, bool includePad)
    :Layout(source, paint, outerwidth, align, TextDirectionHeuristics::LTR, spacingMult,
            spacingAdd){

    mEllipsizedWidth = outerwidth;
    mEllipsizedStart = 0;
    mEllipsizedCount = 0;
    mUseFallbackLineSpacing = false;

    init(source, paint, align, metrics, includePad, true);
}

BoringLayout::BoringLayout(CharSequence* source, TextPaint* paint, int outerWidth, Alignment align,
        float spacingMult, float spacingAdd, const BoringLayout::Metrics& metrics, bool includePad,
        TextUtils::TruncateAt ellipsize, int ellipsizedWidth)
    :Layout(source, paint, outerWidth, align, spacingMult, spacingAdd){

    bool trust;

    if (ellipsize == TextUtils::TruncateAt::NONE || ellipsize == TextUtils::TruncateAt::MARQUEE) {
        mEllipsizedWidth = outerWidth;
        mEllipsizedStart = 0;
        mEllipsizedCount = 0;
        trust = true;
    } else {
        replaceWith(TextUtils::ellipsize(source, *paint, ellipsizedWidth, ellipsize, true,
                    [this](int start,int end){ellipsized(start,end);}),
                    paint, outerWidth, align, spacingMult, spacingAdd);

        mEllipsizedWidth = ellipsizedWidth;
        trust = false;
    }

    init(getText(), paint, align, metrics, includePad, trust);
}

void BoringLayout::init(CharSequence* source, TextPaint* paint, Alignment align,
        const BoringLayout::Metrics& metrics, bool includePad, bool trustWidth) {
    int spacing;

    /*if (source instanceof String && align == Layout::Alignment::ALIGN_NORMAL) {
        mDirect = source->toString();
    } else {
        mDirect = nullptr;
    }*/

    mPaint = *paint;

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
        line->set(paint, source, 0, source->length(), Layout::DIR_LEFT_TO_RIGHT,
                &DIRS_ALL_LEFT_TO_RIGHT, false/*hasTabs*/, nullptr/*TabStops*/,
                mEllipsizedStart, mEllipsizedStart + mEllipsizedCount);
        mMax = (int) std::ceil(line->metrics(nullptr));
        TextLine::recycle(line);
    }

    if (includePad) {
        mTopPadding = metrics.top - metrics.ascent;
        mBottomPadding = metrics.bottom - metrics.descent;
    }

    mDrawingBounds=metrics.mDrawingBounds;
    mDrawingBounds.offset(0, mBottom - mDesc);
}

BoringLayout::Metrics* BoringLayout::isBoring(CharSequence* text, TextPaint* paint) {
    return isBoring(text, paint, TextDirectionHeuristics::FIRSTSTRONG_LTR, nullptr);
}

BoringLayout::Metrics* BoringLayout::isBoring(CharSequence* text, TextPaint* paint, Metrics* metrics) {
    return isBoring(text, paint, TextDirectionHeuristics::FIRSTSTRONG_LTR, metrics);
}

bool BoringLayout::hasAnyInterestingChars(CharSequence* text, int textLength) {
    constexpr int MAX_BUF_LEN = 500;
    std::vector<char16_t> buffer(MAX_BUF_LEN);
    for (int start = 0; start < textLength; start += MAX_BUF_LEN) {
        const int end = std::min(start + MAX_BUF_LEN, textLength);

        // No need to worry about getting half codepoints, since we consider surrogate code
        // units "interesting" as soon we see one.
        TextUtils::getChars(text, start, end, buffer.data(), 0);

        const int len = end - start;
        for (int i = 0; i < len; i++) {
            const char16_t c = buffer[i];
            if (c == '\n' || c == '\t' || TextUtils::couldAffectRtl(c)) {
                return true;
            }
        }
    }
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
    Spanned* sp = dynamic_cast<Spanned*>(text);
    if (sp != nullptr) {
        auto styles = sp->getSpans(0, textLength, make_span_filter<ParagraphStyle>());
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

    TextLine* line = TextLine::obtain();
    line->set(paint, text, 0, textLength, Layout::DIR_LEFT_TO_RIGHT,
            &DIRS_ALL_LEFT_TO_RIGHT, false, nullptr,
            0 /* ellipsisStart, 0 since text has not been ellipsized at this point */,
            0 /* ellipsisEnd, 0 since text has not been ellipsized at this point */);
    fm->width = (int) std::ceil(line->metrics(fm));
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
    return mMax;
}

float BoringLayout::getLineWidth(int line) const{
    return (line == 0 ? mMax : 0);
}

const Directions* BoringLayout::getLineDirections(int line) const{
    return &Layout::DIRS_ALL_LEFT_TO_RIGHT;
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

/*bool BoringLayout::isFallbackLineSpacingEnabled() const{
    return mUseFallbackLineSpacing;
}*/

RectF BoringLayout::computeDrawingBoundingBox() const{
    return mDrawingBounds;
}

void BoringLayout::draw(Canvas& c, Path* highlight, Paint* highlightpaint, int cursorOffset) {
    /*if (!mDirect.empty() && highlight == nullptr) {
        c.draw_text(mDirect, 0, mBottom - mDesc, mPaint);
    } else */{
        Layout::draw(c, highlight, highlightpaint, cursorOffset);
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
