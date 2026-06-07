#include <memory>
#include <text/linebreaker.h>
#include <text/staticlayout.h>
#include <text/precomputedtext.h>
#include <porting/cdlog.h>
#include <minikin/LineBreaker.h>
namespace cdroid{
//public class StaticLayout extends Layout {
const auto TabStopSpanFilter=make_span_filter<TabStopSpan>();
const auto LineHeightSpanFilter=make_span_filter<LineHeightSpan>();
const auto LeadingMarginSpanFilter=make_span_filter<LeadingMarginSpan>();

class LineWidth :public minikin::LineWidth{
private:
    float mFirstWidth;
    int mFirstWidthLineCount;
    float mRestWidth;
public:
    LineWidth(float firstWidth, int firstWidthLineCount, float restWidth) {
        mFirstWidth = firstWidth;
        mFirstWidthLineCount = firstWidthLineCount;
        mRestWidth = restWidth;
    }
    float getAt(size_t line) const override{
            return (line < mFirstWidthLineCount) ? mFirstWidth : mRestWidth;
    }
    float getMin() const override {
        return mRestWidth;
    }
};

StaticLayout::Builder* StaticLayout::Builder::obtain(CharSequence* source, int start, int end, TextPaint* paint,int width) {
    Builder* b = sPool.acquire();
    if (b == nullptr) {
        b = new Builder();
    }

    // set default initial values
    b->mText = source;
    b->mStart = start;
    b->mEnd = end;
    b->mPaint = paint;
    b->mWidth = width;
    b->mAlignment = Alignment::ALIGN_NORMAL;
    b->mTextDir = TextDirectionHeuristics::FIRSTSTRONG_LTR;
    b->mSpacingMult = DEFAULT_LINESPACING_MULTIPLIER;
    b->mSpacingAdd = DEFAULT_LINESPACING_ADDITION;
    b->mIncludePad = true;
    b->mFallbackLineSpacing = false;
    b->mEllipsizedWidth = width;
    b->mEllipsize = TextUtils::TruncateAt::NONE;//nullptr;
    b->mMaxLines = INT_MAX;//Integer.MAX_VALUE;
    b->mBreakStrategy = TextLayout::BREAK_STRATEGY_SIMPLE;
    b->mHyphenationFrequency = TextLayout::HYPHENATION_FREQUENCY_NONE;
    b->mJustificationMode = TextLayout::JUSTIFICATION_MODE_NONE;
    return b;
}

void StaticLayout::Builder::recycle(Builder* b) {
    b->mPaint = nullptr;
    b->mText = nullptr;
    b->mLeftIndents.clear();
    b->mRightIndents.clear();
    sPool.release(b);
}

void StaticLayout::Builder::finish() {
    mText = nullptr; 
    mPaint = nullptr; 
    mLeftIndents.clear();
    mRightIndents.clear();
}

StaticLayout::Builder& StaticLayout::Builder::setText(CharSequence* source) {
    return setText(source, 0, source->length());
}

StaticLayout::Builder& StaticLayout::Builder::setText(CharSequence* source, int start, int end) {
    mText = source;
    mStart = start;
    mEnd = end;
    return *this;
}

StaticLayout::Builder& StaticLayout::Builder::setPaint(TextPaint* paint) {
    mPaint = paint;
    return *this;
}

StaticLayout::Builder& StaticLayout::Builder::setWidth(int width) {
    mWidth = width;
    if (mEllipsize == TextUtils::TruncateAt::NONE/*null*/) {
        mEllipsizedWidth = width;
    }
    return *this;
}

StaticLayout::Builder& StaticLayout::Builder::setAlignment(Alignment alignment) {
    mAlignment = alignment;
    return *this;
}

StaticLayout::Builder& StaticLayout::Builder::setTextDirection(const TextDirectionHeuristic* textDir) {
    mTextDir = textDir;
    return *this;
}

StaticLayout::Builder& StaticLayout::Builder::setLineSpacing(float spacingAdd, float spacingMult) {
    mSpacingAdd = spacingAdd;
    mSpacingMult = spacingMult;
    return *this;
}

StaticLayout::Builder& StaticLayout::Builder::setIncludePad(bool includePad) {
    mIncludePad = includePad;
    return *this;
}

StaticLayout::Builder& StaticLayout::Builder::setUseLineSpacingFromFallbacks(bool useLineSpacingFromFallbacks) {
    mFallbackLineSpacing = useLineSpacingFromFallbacks;
    return *this;
}

StaticLayout::Builder& StaticLayout::Builder::setEllipsizedWidth( int ellipsizedWidth) {
    mEllipsizedWidth = ellipsizedWidth;
    return *this;
}

StaticLayout::Builder& StaticLayout::Builder::setEllipsize(TextUtils::TruncateAt ellipsize) {
    mEllipsize = ellipsize;
    return *this;
}

StaticLayout::Builder& StaticLayout::Builder::setMaxLines( int maxLines) {
    mMaxLines = maxLines;
    return *this;
}

StaticLayout::Builder& StaticLayout::Builder::setBreakStrategy( int breakStrategy) {
    mBreakStrategy = breakStrategy;
    return *this;
}

StaticLayout::Builder& StaticLayout::Builder::setHyphenationFrequency(int hyphenationFrequency) {
    mHyphenationFrequency = hyphenationFrequency;
    return *this;
}

StaticLayout::Builder& StaticLayout::Builder::setIndents(const std::vector<int>& leftIndents,const std::vector<int>& rightIndents) {
    mLeftIndents = leftIndents;
    mRightIndents = rightIndents;
    return *this;
}

StaticLayout::Builder& StaticLayout::Builder::setJustificationMode(int justificationMode) {
    mJustificationMode = justificationMode;
    return *this;
}

StaticLayout::Builder& StaticLayout::Builder::setAddLastLineLineSpacing(bool value) {
    mAddLastLineLineSpacing = value;
    return *this;
}

StaticLayout* StaticLayout::Builder::build() {
    StaticLayout* result = new StaticLayout(*this);
    Builder::recycle(this);
    return result;
}

Pools::SynchronizedPool<StaticLayout::Builder> StaticLayout::Builder::sPool(3);// = new SynchronizedPool<>(3);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

StaticLayout::StaticLayout(const Builder& b):TextLayout((b.mEllipsize == TextUtils::TruncateAt::NONE/*nullptr*/)
            ? b.mText : [this, &b]() -> CharSequence* {
                Spanned* spanned = dynamic_cast<Spanned*>(b.mText);
                return spanned != nullptr ? new SpannedEllipsizer(spanned) : new Ellipsizer(b.mText);
            }(),
            b.mPaint, b.mWidth, b.mAlignment, b.mTextDir, b.mSpacingMult, b.mSpacingAdd){

    if (b.mEllipsize != TextUtils::TruncateAt::NONE/*nullptr*/) {
        Ellipsizer* e = dynamic_cast<Ellipsizer*>(getText());

        e->mLayout = this;
        e->mWidth = b.mEllipsizedWidth;
        e->mMethod = b.mEllipsize;
        mEllipsizedWidth = b.mEllipsizedWidth;
        mColumns = COLUMNS_ELLIPSIZE;
    } else {
        mColumns = COLUMNS_NORMAL;
        mEllipsizedWidth = b.mWidth;
    }

    mLineDirections.resize(2);// = ArrayUtils.newUnpaddedArray(Directions.class, 2);
    mLines.resize(2 * mColumns);
    mMaximumVisibleLineCount = b.mMaxLines;

    mLeftIndents = b.mLeftIndents;
    mRightIndents = b.mRightIndents;
    setJustificationMode(b.mJustificationMode);

    generate(b, b.mIncludePad, b.mIncludePad);
}

StaticLayout::~StaticLayout(){
    for(auto dir:mLineDirections){
        if((dir!=&TextLayout::DIRS_ALL_LEFT_TO_RIGHT)&&(dir!=&TextLayout::DIRS_ALL_RIGHT_TO_LEFT)){
            delete dir;
        }
    }
}

void StaticLayout::generate(const Builder& b, bool includepad, bool trackpad) {
    CharSequence* source = b.mText;
    const int bufStart = b.mStart;
    const int bufEnd = b.mEnd;
    TextPaint* paint = b.mPaint;
    const int outerWidth = b.mWidth;
    const TextDirectionHeuristic* textDir = b.mTextDir;
    const bool fallbackLineSpacing = b.mFallbackLineSpacing;
    float spacingmult = b.mSpacingMult;
    float spacingadd = b.mSpacingAdd;
    float ellipsizedWidth = b.mEllipsizedWidth;
    TextUtils::TruncateAt ellipsize = b.mEllipsize;
    const bool addLastLineSpacing = b.mAddLastLineLineSpacing;

    int lineBreakCapacity = 0;
    std::vector<int> breaks;
    std::vector<float> lineWidths;
    std::vector<float> ascents;
    std::vector<float> descents;
    std::vector<bool> hasTabs;
    std::vector<int> hyphenEdits;

    mLineCount = 0;
    mEllipsized = false;
    mMaxLineHeight = mMaximumVisibleLineCount < 1 ? 0 : DEFAULT_MAX_LINE_HEIGHT;

    int v = 0;
    bool needMultiply = (spacingmult != 1 || spacingadd != 0);

    Paint::FontMetricsInt fm = b.mFontMetricsInt;
    std::vector<int> chooseHtv;

    std::vector<int> indents;
    if (mLeftIndents.size() || mRightIndents.size()) {
        const int leftLen = mLeftIndents.empty() ? 0 : mLeftIndents.size();
        const int rightLen = mRightIndents.empty() ? 0 : mRightIndents.size();
        const int indentsLen = std::max(leftLen, rightLen);
        indents.resize(indentsLen);
        for (int i = 0; i < leftLen; i++) {
            indents[i] = mLeftIndents[i];
        }
        for (int i = 0; i < rightLen; i++) {
            indents[i] += mRightIndents[i];
        }
    } else {
        indents.clear();// = null;
    }

    LineBreaker lineBreaker(b.mBreakStrategy,b.mHyphenationFrequency,b.mJustificationMode,indents);

    std::vector<PrecomputedText::ParagraphInfo> paragraphInfo;
    Spanned* spanned = dynamic_cast<Spanned*>(source);
    if (dynamic_cast<PrecomputedText*>(source)) {
        PrecomputedText* precomputed = dynamic_cast<PrecomputedText*>(source);
        const int checkResult =  precomputed->checkResultUsable(bufStart, bufEnd, textDir, *paint,
                        b.mBreakStrategy, b.mHyphenationFrequency);
        switch (checkResult) {
        case PrecomputedText::Params::UNUSABLE:
            break;
        case PrecomputedText::Params::NEED_RECOMPUTE:
            precomputed = PrecomputedText::create(precomputed, PrecomputedText::Params(
                        *paint,textDir,b.mBreakStrategy,b.mHyphenationFrequency));
            paragraphInfo = precomputed->getParagraphInfo();
            break;
        case PrecomputedText::Params::USABLE:
            // Some parameters are different from the ones when measured text is created.
            paragraphInfo = precomputed->getParagraphInfo();
            break;
        }
    }
    if (paragraphInfo.empty()){// == null) {
        const PrecomputedText::Params param(*paint, textDir, b.mBreakStrategy, b.mHyphenationFrequency);
        paragraphInfo = PrecomputedText::createMeasuredParagraphs(source, param, bufStart, bufEnd, false );
    }
    for (int paraIndex = 0; paraIndex < paragraphInfo.size(); paraIndex++) {
        const int paraStart = paraIndex == 0 ? bufStart : paragraphInfo[paraIndex - 1].paragraphEnd;
        const int paraEnd = paragraphInfo[paraIndex].paragraphEnd;

        int firstWidthLineCount = 1;
        int firstWidth = outerWidth;
        int restWidth = outerWidth;

        std::vector<ParcelableSpan*> chooseHt;
        if (spanned != nullptr) {
            auto sp = getParagraphSpans(spanned, paraStart, paraEnd, LeadingMarginSpanFilter);
            for (int i = 0; i < sp.size(); i++) {
                LeadingMarginSpan* lms = (LeadingMarginSpan*)sp[i];
                firstWidth -= lms->getLeadingMargin(true);
                restWidth -= lms->getLeadingMargin(false);

                // LeadingMarginSpan2 is odd.  The count affects all
                // leading margin spans, not just this particular one
                if (dynamic_cast<LeadingMarginSpan2*>(lms)) {
                    LeadingMarginSpan2* lms2 = (LeadingMarginSpan2*) lms;
                    firstWidthLineCount = std::max(firstWidthLineCount, lms2->getLeadingMarginLineCount());
                }
            }

            chooseHt = getParagraphSpans(spanned, paraStart, paraEnd, LineHeightSpanFilter);

            if (chooseHt.size() == 0) {
                chooseHt.clear(); // So that out() would not assume it has any contents
            } else {
                if (chooseHtv.empty() || chooseHtv.size() < chooseHt.size()) {
                    chooseHtv.resize(chooseHt.size());
                }

                for (int i = 0; i < chooseHt.size(); i++) {
                    const int o = spanned->getSpanStart(chooseHt[i]);
                    if (o < paraStart) {
                        // starts in this layout, before the
                        // current paragraph
                        chooseHtv[i] = getLineTop(getLineForOffset(o));
                    } else {
                        // starts in this paragraph
                        chooseHtv[i] = v;
                    }
                }
            }
        }
        // tab stop locations
        std::vector<float> variableTabStops;
        if (spanned != nullptr) {
            auto spans = getParagraphSpans(spanned, paraStart, paraEnd, TabStopSpanFilter);
            if (spans.size() > 0) {
                std::vector<float> stops(spans.size());
                for (int i = 0; i < spans.size(); i++) {
                    stops[i] = ((TabStopSpan*)spans[i])->getTabStop();
                }
                std::sort(stops.begin(),stops.end());
                variableTabStops = stops;
            }
        }
        MeasuredParagraph* measuredPara = paragraphInfo[paraIndex].measured;
        std::vector<char32_t>chs = measuredPara->getChars();
        auto spanEndCache = measuredPara->getSpanEndCache();
        auto fmCache = measuredPara->getFontMetrics();
        LineBreaker::ParagraphConstraints constraints;

        constraints.setWidth(restWidth);
        constraints.setIndent(firstWidth, firstWidthLineCount);
        constraints.setTabStops(variableTabStops, TAB_INCREMENT);

        LineBreaker::Result res = lineBreaker.computeLineBreaks( measuredPara->getMeasuredText(), constraints, mLineCount);
        int breakCount = res.getLineCount();
        if (lineBreakCapacity < breakCount) {
            lineBreakCapacity = breakCount;
            breaks.resize(lineBreakCapacity);
            lineWidths.resize(lineBreakCapacity);
            ascents.resize(lineBreakCapacity);
            descents.resize(lineBreakCapacity);
            hasTabs.resize(lineBreakCapacity);
            hyphenEdits.resize(lineBreakCapacity);
        }

        for (int i = 0; i < breakCount; ++i) {
            breaks[i] = res.getLineBreakOffset(i);
            lineWidths[i] = res.getLineWidth(i);
            ascents[i] = res.getLineAscent(i);
            descents[i] = res.getLineDescent(i);
            hasTabs[i] = res.hasLineTab(i);
            hyphenEdits[i] =
                packHyphenEdit(res.getStartLineHyphenEdit(i), res.getEndLineHyphenEdit(i));
        }

        const int remainingLineCount = mMaximumVisibleLineCount - mLineCount;
        const bool ellipsisMayBeApplied = ellipsize != TextUtils::TruncateAt::NONE
                && (ellipsize == TextUtils::TruncateAt::END
                    || (mMaximumVisibleLineCount == 1
                            && ellipsize != TextUtils::TruncateAt::MARQUEE));
        if (0 < remainingLineCount && remainingLineCount < breakCount
                && ellipsisMayBeApplied) {
            // Calculate width
            float width = 0;
            bool hasTab = false;  // XXX May need to also have starting hyphen edit
            for (int i = remainingLineCount - 1; i < breakCount; i++) {
                if (i == breakCount - 1) {
                    width += lineWidths[i];
                } else {
                    for (int j = (i == 0 ? 0 : breaks[i - 1]); j < breaks[i]; j++) {
                        width += measuredPara->getCharWidthAt(j);
                    }
                }
                hasTab |= hasTabs[i];
            }
            // Treat the last line and overflowed lines as a single line.
            breaks[remainingLineCount - 1] = breaks[breakCount - 1];
            lineWidths[remainingLineCount - 1] = width;
            hasTabs[remainingLineCount - 1] = hasTab;

            breakCount = remainingLineCount;
        }
        // here is the offset of the starting character of the line we are currently
        // measuring
        int here = paraStart;
        int fmTop = 0, fmBottom = 0, fmAscent = 0, fmDescent = 0;
        int fmCacheIndex = 0;
        int spanEndCacheIndex = 0;
        int breakIndex = 0;
        for (int spanStart = paraStart, spanEnd; spanStart < paraEnd; spanStart = spanEnd) {
            // retrieve end of span
            spanEnd = spanEndCache[spanEndCacheIndex++];

            // retrieve cached metrics, order matches above
            fm.top = fmCache[fmCacheIndex * 4 + 0];
            fm.bottom = fmCache[fmCacheIndex * 4 + 1];
            fm.ascent = fmCache[fmCacheIndex * 4 + 2];
            fm.descent = fmCache[fmCacheIndex * 4 + 3];
            fmCacheIndex++;

            if (fm.top < fmTop) {
                fmTop = fm.top;
            }
            if (fm.ascent < fmAscent) {
                fmAscent = fm.ascent;
            }
            if (fm.descent > fmDescent) {
                fmDescent = fm.descent;
            }
            if (fm.bottom > fmBottom) {
                fmBottom = fm.bottom;
            }
            // skip breaks ending before current span range
            while (breakIndex < breakCount && paraStart + breaks[breakIndex] < spanStart) {
                breakIndex++;
            }
            while (breakIndex < breakCount && paraStart + breaks[breakIndex] <= spanEnd) {
                int endPos = paraStart + breaks[breakIndex];
                bool moreChars = (endPos < bufEnd);
                const int ascent = fallbackLineSpacing
                        ? std::min(fmAscent, (int)std::round(ascents[breakIndex]))
                        : fmAscent;
                const int descent = fallbackLineSpacing
                        ? std::max(fmDescent, (int)std::round(descents[breakIndex]))
                        : fmDescent;

                v = out(source, here, endPos,
                        ascent, descent, fmTop, fmBottom,
                        v, spacingmult, spacingadd, chooseHt, &chooseHtv, fm,
                        hasTabs[breakIndex], hyphenEdits[breakIndex], needMultiply,
                        measuredPara, bufEnd, includepad, trackpad, addLastLineSpacing, chs,
                        paraStart, ellipsize, ellipsizedWidth, lineWidths[breakIndex],
                        paint, moreChars);
                if (endPos < spanEnd) {
                    // preserve metrics for current span
                    fmTop = fm.top;
                    fmBottom = fm.bottom;
                    fmAscent = fm.ascent;
                    fmDescent = fm.descent;
                } else {
                    fmTop = fmBottom = fmAscent = fmDescent = 0;
                }
                here = endPos;
                breakIndex++;
                if (mLineCount >= mMaximumVisibleLineCount && mEllipsized) {
                    LOGD("TODO free remained measuredPara");
                    return;
                }
            }
        }
        delete measuredPara;
        if (paraEnd == bufEnd) {
            break;
        }
    }
    if ((bufEnd == bufStart || source->charAt(bufEnd - 1) == CHAR_NEW_LINE)
            && mLineCount < mMaximumVisibleLineCount) {
        MeasuredParagraph* measuredPara = MeasuredParagraph::buildForBidi(source, bufEnd, bufEnd, textDir, nullptr);
        paint->getFontMetricsInt(fm);
        v = out(source, bufEnd, bufEnd, fm.ascent, fm.descent, fm.top, fm.bottom,
                v, spacingmult, spacingadd, {}, nullptr, fm, false, 0, needMultiply,
                measuredPara, bufEnd, includepad, trackpad, addLastLineSpacing, {},
                bufStart, ellipsize, ellipsizedWidth, 0, paint, false);
        delete measuredPara;
    }
}

int StaticLayout::out(CharSequence* text, int start, int end, int above, int below, int top, int bottom,
        int v, float spacingmult, float spacingadd, const std::vector<ParcelableSpan*>& chooseHt,
        const std::vector<int>* chooseHtv, Paint::FontMetricsInt& fm,bool hasTab, int hyphenEdit,
        bool needMultiply, MeasuredParagraph* measured, int bufEnd, bool includePad, bool trackPad,
        bool addLastLineLineSpacing,const std::vector<char32_t>& chs,int widthStart, TextUtils::TruncateAt ellipsize,
        float ellipsisWidth,float textWidth, TextPaint* paint, bool moreChars) {
    const int j = mLineCount;
    const int off = j * mColumns;
    const int want = off + mColumns + TOP;
    std::vector<int>& lines = mLines;
    const int dir = measured->getParagraphDir();

    if (want >= lines.size()) {
        mLines.resize(want+8);
        lines = mLines;
    }

    if (j >= mLineDirections.size()) {
        mLineDirections.resize(j+8);
    }

    if (!chooseHt.empty()) {
        fm.ascent = above;
        fm.descent = below;
        fm.top = top;
        fm.bottom = bottom;
        for (size_t i = 0; i < chooseHt.size(); i++) {
            int chooseHtvVal = chooseHtv != nullptr ? (*chooseHtv)[i] : 0;
            if (dynamic_cast<LineHeightSpan::WithDensity*>(chooseHt[i])) {
                ((LineHeightSpan::WithDensity*) chooseHt[i])
                        ->chooseHeight(text, start, end, chooseHtvVal, v, fm, paint);
            } else {
                dynamic_cast<LineHeightSpan*>(chooseHt[i])->chooseHeight(text, start, end, chooseHtvVal, v, fm);
            }
        }
        above = fm.ascent;
        below = fm.descent;
        top = fm.top;
        bottom = fm.bottom;
    }
    bool firstLine = (j == 0);
    bool currentLineIsTheLastVisibleOne = (j + 1 == mMaximumVisibleLineCount);

    if (ellipsize != TextUtils::TruncateAt::NONE) {
        // If there is only one line, then do any type of ellipsis except when it is MARQUEE
        // if there are multiple lines, just allow END ellipsis on the last line
        bool forceEllipsis = moreChars && (mLineCount + 1 == mMaximumVisibleLineCount);
        bool doEllipsis = (((mMaximumVisibleLineCount == 1 && moreChars) || (firstLine && !moreChars)) &&
                        ellipsize != TextUtils::TruncateAt::MARQUEE) ||
                (!firstLine && (currentLineIsTheLastVisibleOne || !moreChars) &&
                        ellipsize == TextUtils::TruncateAt::END);
        if (doEllipsis) {
            calculateEllipsis(start, end, measured, widthStart,
                    ellipsisWidth, ellipsize, j,
                    textWidth, paint, forceEllipsis);
        }
    }

    bool lastLine;
    if (mEllipsized) {
        lastLine = true;
    } else {
        const bool lastCharIsNewLine = widthStart != bufEnd && bufEnd > 0
                && text->charAt(bufEnd - 1) == CHAR_NEW_LINE;
        if (end == bufEnd && !lastCharIsNewLine) {
            lastLine = true;
        } else if (start == bufEnd && lastCharIsNewLine) {
            lastLine = true;
        } else {
            lastLine = false;
        }
    }
    if (firstLine) {
        if (trackPad) {
            mTopPadding = top - above;
        }
        if (includePad) {
            above = top;
        }
    }
    int extra;
    if (lastLine) {
        if (trackPad) {
            mBottomPadding = bottom - below;
        }
        if (includePad) {
            below = bottom;
        }
    }
    if (needMultiply && (addLastLineLineSpacing || !lastLine)) {
        double ex = (below - above) * (spacingmult - 1) + spacingadd;
        if (ex >= 0) {
            extra = (int)(ex + EXTRA_ROUNDING);
        } else {
            extra = -(int)(-ex + EXTRA_ROUNDING);
        }
    } else {
        extra = 0;
    }

    lines[off + START] = start;
    lines[off + TOP] = v;
    lines[off + DESCENT] = below + extra;
    lines[off + EXTRA] = extra;

    // special case for non-ellipsized last visible line when maxLines is set
    // store the height as if it was ellipsized
    if (!mEllipsized && currentLineIsTheLastVisibleOne) {
        // below calculation as if it was the last line
        int maxLineBelow = includePad ? bottom : below;
        // similar to the calculation of v below, without the extra.
        mMaxLineHeight = v + (maxLineBelow - above);
    }

    v += (below - above) + extra;
    lines[off + mColumns + START] = end;
    lines[off + mColumns + TOP] = v;

    // TODO: could move TAB to share same column as HYPHEN, simplifying this code and gaining
    // one bit for start field
    lines[off + TAB] |= hasTab ? TAB_MASK : 0;
    lines[off + HYPHEN] = hyphenEdit;
    lines[off + DIR] |= dir << DIR_SHIFT;
    mLineDirections[j] = measured->getDirections(start - widthStart, end - widthStart);

    mLineCount++;
    return v;
}

void StaticLayout::calculateEllipsis(int lineStart, int lineEnd, MeasuredParagraph* measured, int widthStart, float avail,
        TextUtils::TruncateAt where,int line, float textWidth, TextPaint* paint, bool forceEllipsis) {
    avail -= getTotalInsets(line);
    if (textWidth <= avail && !forceEllipsis) {
        // Everything fits!
        mLines[mColumns * line + ELLIPSIS_START] = 0;
        mLines[mColumns * line + ELLIPSIS_COUNT] = 0;
        return;
    }

    float ellipsisWidth = paint->measureText(TextUtils::getEllipsisString(where));
    int ellipsisStart = 0;
    int ellipsisCount = 0;
    int len = lineEnd - lineStart;

    // We only support start ellipsis on a single line
    if (where == TextUtils::TruncateAt::START) {
        if (mMaximumVisibleLineCount == 1) {
            float sum = 0;
            int i;
            for (i = len; i > 0; i--) {
                float w = measured->getCharWidthAt(i - 1 + lineStart - widthStart);
                if (w + sum + ellipsisWidth > avail) {
                    while (i < len
                            && measured->getCharWidthAt(i + lineStart - widthStart) == 0.0f) {
                        i++;
                    }
                    break;
                }
                sum += w;
            }
            ellipsisStart = 0;
            ellipsisCount = i;
        } else {
            LOGE("Start Ellipsis only supported with one line");
        }
    } else if (where == TextUtils::TruncateAt::END || where == TextUtils::TruncateAt::MARQUEE ||
            where == TextUtils::TruncateAt::END_SMALL) {
        float sum = 0;
        int i;
        for (i = 0; i < len; i++) {
            const float w = measured->getCharWidthAt(i + lineStart - widthStart);
            if (w + sum + ellipsisWidth > avail) {
                break;
            }
            sum += w;
        }
        ellipsisStart = i;
        ellipsisCount = len - i;
        if (forceEllipsis && ellipsisCount == 0 && len > 0) {
            ellipsisStart = len - 1;
            ellipsisCount = 1;
        }
    } else {
        // where = TextUtils.TruncateAt.MIDDLE We only support middle ellipsis on a single line
        if (mMaximumVisibleLineCount == 1) {
            float lsum = 0, rsum = 0;
            int left = 0, right = len;

            float ravail = (avail - ellipsisWidth) / 2;
            for (right = len; right > 0; right--) {
                float w = measured->getCharWidthAt(right - 1 + lineStart - widthStart);

                if (w + rsum > ravail) {
                    while (right < len
                            && measured->getCharWidthAt(right + lineStart - widthStart)
                                == 0.0f) {
                        right++;
                    }
                    break;
                }
                rsum += w;
            }

            float lavail = avail - ellipsisWidth - rsum;
            for (left = 0; left < right; left++) {
                float w = measured->getCharWidthAt(left + lineStart - widthStart);

                if (w + lsum > lavail) {
                    break;
                }

                lsum += w;
            }
            ellipsisStart = left;
            ellipsisCount = right - left;
        } else {
            LOGE("Middle Ellipsis only supported with one line");
        }
    }
    mEllipsized = true;
    mLines[mColumns * line + ELLIPSIS_START] = ellipsisStart;
    mLines[mColumns * line + ELLIPSIS_COUNT] = ellipsisCount;
}

float StaticLayout::getTotalInsets(int line) {
    int totalIndent = 0;
    if (!mLeftIndents.empty()) {
        totalIndent = mLeftIndents[std::min(line, (int)mLeftIndents.size() - 1)];
    }
    if (!mRightIndents.empty()) {
        totalIndent += mRightIndents[std::min(line, (int)mRightIndents.size() - 1)];
    }
    return totalIndent;
}

int StaticLayout::getLineForVertical(int vertical) const{
    int high = mLineCount;
    int low = -1;
    int guess;
    const auto& lines = mLines;
    while (high - low > 1) {
        guess = (high + low) >> 1;
        if (lines[mColumns * guess + TOP] > vertical){
            high = guess;
        } else {
            low = guess;
        }
    }
    if (low < 0) {
        return 0;
    } else {
        return low;
    }
}

const Directions* StaticLayout::getLineDirections(int line) const{
    if (line > getLineCount()) {
        //throw new ArrayIndexOutOfBoundsException();
    }
    return mLineDirections[line];
}

// To store into single int field, pack the pair of start and end hyphen edit.

int StaticLayout::getIndentAdjust(int line, Alignment align) const{
    if (align == Alignment::ALIGN_LEFT) {
        if (mLeftIndents.empty()) {
            return 0;
        } else {
            return mLeftIndents[std::min(line, (int)mLeftIndents.size() - 1)];
        }
    } else if (align == Alignment::ALIGN_RIGHT) {
        if (mRightIndents.empty()) {
            return 0;
        } else {
            return -mRightIndents[std::min(line, (int)mRightIndents.size() - 1)];
        }
    } else if (align == Alignment::ALIGN_CENTER) {
        int left = 0;
        if (!mLeftIndents.empty()) {
            left = mLeftIndents[std::min(line, (int)mLeftIndents.size() - 1)];
        }
        int right = 0;
        if (!mRightIndents.empty()) {
            right = mRightIndents[std::min(line, (int)mRightIndents.size() - 1)];
        }
        return (left - right) >> 1;
    } else {
        LOGE("unhandled alignment %d" , align);
    }
    return 0;
}

int StaticLayout::getEllipsisCount(int line) const{
    if (mColumns < COLUMNS_ELLIPSIZE) {
        return 0;
    }
    return mLines[mColumns * line + ELLIPSIS_COUNT];
}

int StaticLayout::getEllipsisStart(int line) const{
    if (mColumns < COLUMNS_ELLIPSIZE) {
        return 0;
    }
    return mLines[mColumns * line + ELLIPSIS_START];
}

int StaticLayout::getHeight(bool cap) const{
    if (cap && mLineCount > mMaximumVisibleLineCount && mMaxLineHeight == -1) {
        LOGW("maxLineHeight should not be -1. maxLines:%d lineCount: %d", mMaximumVisibleLineCount,mLineCount);
    }

    return cap && mLineCount > mMaximumVisibleLineCount && mMaxLineHeight != -1
            ? mMaxLineHeight : TextLayout::getHeight();
}

}
