#include <text/measuredparagraph.h>
#include <text/measuredtext.h>
#include <text/layout.h>
#include <text/androidbidi.h>
#include <minikin/MeasuredText.h>
namespace cdroid{

const auto TabStopSpanFilter=Predicate<const ParcelableSpan*>([](const ParcelableSpan* span){return dynamic_cast<const TabStopSpan*>(span) != nullptr;});
const auto MetricAffectingSpanFilter=Predicate<const ParcelableSpan*>([](const ParcelableSpan* span){return dynamic_cast<const MetricAffectingSpan*>(span) != nullptr;});
const auto ReplacementSpanFilter=Predicate<const ParcelableSpan*>([](const ParcelableSpan* span){return dynamic_cast<const ReplacementSpan*>(span) != nullptr;});

Pools::SynchronizedPool<MeasuredParagraph> MeasuredParagraph::sPool(1);// = new SynchronizedPool<>(1);

MeasuredParagraph* MeasuredParagraph::obtain() { // Use build static functions instead.
    MeasuredParagraph* mt = sPool.acquire();
    return mt != nullptr ? mt : new MeasuredParagraph();
}

void MeasuredParagraph::recycle() {
    release();
    sPool.release(this);
}

void  MeasuredParagraph::release() {
    reset();
    mLevels.clear();//clearWithReleasingLargeArray();
    mWidths.clear();//clearWithReleasingLargeArray();
    mFontMetrics.clear();//clearWithReleasingLargeArray();
    mSpanEndCache.clear();//clearWithReleasingLargeArray();
}

void  MeasuredParagraph::reset() {
    mSpanned = nullptr;
    mCopiedBuffer.clear();// = nullptr;
    mWholeWidth = 0;
    mLevels.clear();
    mWidths.clear();
    mFontMetrics.clear();
    mSpanEndCache.clear();
    delete mMeasuredText;
    mMeasuredText = nullptr;
}

/*int MeasuredParagraph::getTextLength() const{
    return mTextLength;
}

char[] MeasuredParagraph::getChars() const{
    return mCopiedBuffer;
}

int MeasuredParagraph::getParagraphDir() const{
    return mParaDir;
}*/

const Directions* MeasuredParagraph::getDirections( int start, int end) const{
    if (mLtrWithoutBidi) {
        return &Layout::DIRS_ALL_LEFT_TO_RIGHT;
    }

    const int length = end - start;
    return AndroidBidi::directions(mParaDir, mLevels, start, mCopiedBuffer, start,
            length);
}

/*float MeasuredParagraph::getWholeWidth() const{
    return mWholeWidth;
}

std::vector<float> MeasuredParagraph::getWidths() const{
    return mWidths;
}

std::vector<int> MeasuredParagraph::getSpanEndCache() const{
    return mSpanEndCache;
}

std::vector<int> MeasuredParagraph::getFontMetrics() const{
    return mFontMetrics;
}

MeasuredText* MeasuredParagraph::getMeasuredText() const{
    return mMeasuredText;
}*/

float MeasuredParagraph::getWidth(int start, int end) const{
    if (mMeasuredText == nullptr) {
        // We have result in Java.
        auto& widths = mWidths;
        float r = 0.0f;
        for (int i = start; i < end; ++i) {
            r += widths[i];
        }
        return r;
    } else {
        // We have result in native.
        return mMeasuredText->getWidth(start, end);
    }
}

void MeasuredParagraph::getBounds(int start, int end, Rect& bounds) const{
    mMeasuredText->getBounds(start, end, bounds);
}

float MeasuredParagraph::getCharWidthAt( int offset) const{
    return mMeasuredText->getCharWidthAt(offset);
}

MeasuredParagraph* MeasuredParagraph::buildForBidi(CharSequence* text, int start, int end, const TextDirectionHeuristic* textDir, MeasuredParagraph* recycle) {
    MeasuredParagraph* mt = recycle == nullptr ? obtain() : recycle;
    mt->resetAndAnalyzeBidi(text, start, end, textDir);
    return mt;
}

MeasuredParagraph* MeasuredParagraph::buildForMeasurement(TextPaint* paint,const CharSequence* text,
        int start, int end, const TextDirectionHeuristic* textDir, MeasuredParagraph* recycle) {
    MeasuredParagraph* mt = recycle == nullptr ? obtain() : recycle;
    mt->resetAndAnalyzeBidi(text, start, end, textDir);

    mt->mWidths.resize(mt->mTextLength);
    if (mt->mTextLength == 0) {
        return mt;
    }

    if (mt->mSpanned == nullptr) {
        // No style change by MetricsAffectingSpan. Just measure all text.
        mt->applyMetricsAffectingSpan(*paint, {/*nullptr*/} /* spans */, start, end, nullptr /* native builder ptr */);
    } else {
        // There may be a MetricsAffectingSpan. Split into span transitions and apply styles.
        int spanEnd;
        for (int spanStart = start; spanStart < end; spanStart = spanEnd) {
            spanEnd = mt->mSpanned->nextSpanTransition(spanStart, end, MetricAffectingSpanFilter);
            auto spans = mt->mSpanned->getSpans(spanStart, spanEnd, MetricAffectingSpanFilter);
            TextUtils::removeEmptySpans(spans, mt->mSpanned, MetricAffectingSpanFilter);
            mt->applyMetricsAffectingSpan(*paint, spans, spanStart, spanEnd, nullptr /* native builder ptr */);
        }
    }
    return mt;
}

MeasuredParagraph* MeasuredParagraph::buildForStaticLayout(const TextPaint* paint,const CharSequence* text, int start, int end,
        const TextDirectionHeuristic* textDir, bool computeHyphenation, bool computeLayout, MeasuredParagraph* hint, MeasuredParagraph* recycle) {
    MeasuredParagraph* mt = recycle == nullptr ? obtain() : recycle;
    mt->resetAndAnalyzeBidi(text, start, end, textDir);
    std::unique_ptr<MeasuredText::Builder> builder;
    if (hint == nullptr) {
        builder = std::make_unique<MeasuredText::Builder>(mt->mCopiedBuffer);
        builder->setComputeHyphenation(computeHyphenation)
                .setComputeLayout(computeLayout);
    } else {
        builder = std::make_unique<MeasuredText::Builder>(hint->mMeasuredText);
    }
    if (mt->mTextLength == 0) {
        // Need to build empty native measured text for StaticLayout.
        // TODO: Stop creating empty measured text for empty lines.
        mt->mMeasuredText = builder->build();
    } else {
        if (mt->mSpanned == nullptr) {
            // No style change by MetricsAffectingSpan. Just measure all text.
            mt->applyMetricsAffectingSpan(*paint, {/*nullptr*/} /* spans */, start, end, builder.get());
            mt->mSpanEndCache.emplace_back(end);
        } else {
            // There may be a MetricsAffectingSpan. Split into span transitions and apply
            // styles.
            int spanEnd;
            for (int spanStart = start; spanStart < end; spanStart = spanEnd) {
                spanEnd = mt->mSpanned->nextSpanTransition(spanStart, end, MetricAffectingSpanFilter);
                auto spans = mt->mSpanned->getSpans(spanStart, spanEnd, MetricAffectingSpanFilter);
                TextUtils::removeEmptySpans(spans, mt->mSpanned, MetricAffectingSpanFilter);
                mt->applyMetricsAffectingSpan(*paint, spans, spanStart, spanEnd, builder.get());
                mt->mSpanEndCache.emplace_back(spanEnd);
            }
        }
        mt->mMeasuredText = builder->build();
    }

    return mt;
}

void MeasuredParagraph::resetAndAnalyzeBidi(const CharSequence* text, int start, int end, const TextDirectionHeuristic* textDir) {
    reset();
    mSpanned = dynamic_cast<const Spanned*>(text);
    mTextStart = start;
    mTextLength = end - start;

    if (mCopiedBuffer.empty() || mCopiedBuffer.size() != mTextLength) {
        mCopiedBuffer.resize(mTextLength);
    }
    TextUtils::getChars(text, start, end, mCopiedBuffer.data(), 0);

    // Replace characters associated with ReplacementSpan to U+FFFC.
    if (mSpanned != nullptr) {
        auto spans = mSpanned->getSpans(start, end, ReplacementSpanFilter);

        for (int i = 0; i < spans.size(); i++) {
            int startInPara = mSpanned->getSpanStart(spans[i]) - start;
            int endInPara = mSpanned->getSpanEnd(spans[i]) - start;
            // The span interval may be larger and must be restricted to [start, end)
            if (startInPara < 0) startInPara = 0;
            if (endInPara > mTextLength) endInPara = mTextLength;
            //Arrays.fill(mCopiedBuffer, startInPara, endInPara, OBJECT_REPLACEMENT_CHARACTER);
            for(int j=startInPara;j<endInPara;j++)mCopiedBuffer[j]=OBJECT_REPLACEMENT_CHARACTER;
        }
    }

    if ((textDir == TextDirectionHeuristics::LTR
            || textDir == TextDirectionHeuristics::FIRSTSTRONG_LTR
            || textDir == TextDirectionHeuristics::ANYRTL_LTR)
            && TextUtils::doesNotNeedBidi(mCopiedBuffer, 0, mTextLength)) {
        mLevels.clear();
        mParaDir = Layout::DIR_LEFT_TO_RIGHT;
        mLtrWithoutBidi = true;
    } else {
        int bidiRequest;
        if (textDir == TextDirectionHeuristics::LTR) {
            bidiRequest = Layout::DIR_REQUEST_LTR;
        } else if (textDir == TextDirectionHeuristics::RTL) {
            bidiRequest = Layout::DIR_REQUEST_RTL;
        } else if (textDir == TextDirectionHeuristics::FIRSTSTRONG_LTR) {
            bidiRequest = Layout::DIR_REQUEST_DEFAULT_LTR;
        } else if (textDir == TextDirectionHeuristics::FIRSTSTRONG_RTL) {
            bidiRequest = Layout::DIR_REQUEST_DEFAULT_RTL;
        } else {
            const bool isRtl = textDir->isRtl(mCopiedBuffer.data(), 0, mTextLength);
            bidiRequest = isRtl ? Layout::DIR_REQUEST_RTL : Layout::DIR_REQUEST_LTR;
        }
        mLevels.resize(mTextLength);
        mParaDir = AndroidBidi::bidi(bidiRequest, mCopiedBuffer, mLevels);
        mLtrWithoutBidi = false;
    }
}

void  MeasuredParagraph::applyReplacementRun(ReplacementSpan& replacement, int start, int end, MeasuredText::Builder* builder) {
    // Use original text. Shouldn't matter.
    // TODO: passing uninitizlied FontMetrics to developers. Do we need to keep this for
    //       backward compatibility? or Should we initialize them for getFontMetricsInt?
    const float width = replacement.getSize( mCachedPaint, mSpanned, start + mTextStart, end + mTextStart, &mCachedFm);
    if (builder == nullptr) {
        // Assigns all width to the first character. This is the same behavior as minikin.
        mWidths[start]=width;//.set(start, width);
        if (end > start + 1) {
            //Arrays.fill(mWidths, start + 1, end, 0.0f);
            for(int i=start + 1;i<end;i++)mWidths[i]=0;
        }
        mWholeWidth += width;
    } else {
        builder->appendReplacementRun(mCachedPaint, end - start, width);
    }
}

void MeasuredParagraph::applyStyleRun(int start, int end, MeasuredText::Builder* builder) {

    if (mLtrWithoutBidi) {
        // If the whole text is LTR direction, just apply whole region.
        if (builder == nullptr) {
            mWholeWidth += mCachedPaint.getTextRunAdvances(
                    mCopiedBuffer.data(), start, end - start, start, end - start, false /* isRtl */,
                    mWidths.data(), start);
        } else {
            builder->appendStyleRun(mCachedPaint, end - start, false /* isRtl */);
        }
    } else {
        // If there is multiple bidi levels, split into individual bidi level and apply style.
        uint8_t level = mLevels.at(start);
        // Note that the empty text or empty range won't reach this method.
        // Safe to search from start + 1.
        for (int levelStart = start, levelEnd = start + 1;; ++levelEnd) {
            if (levelEnd == end || mLevels.at(levelEnd) != level) {  // transition point
                const bool isRtl = (level & 0x1) != 0;
                if (builder == nullptr) {
                    const int levelLength = levelEnd - levelStart;
                    mWholeWidth += mCachedPaint.getTextRunAdvances(
                            mCopiedBuffer.data(), levelStart, levelLength, levelStart, levelLength,
                            isRtl, mWidths.data(), levelStart);
                } else {
                    builder->appendStyleRun(mCachedPaint, levelEnd - levelStart, isRtl);
                }
                if (levelEnd == end) {
                    break;
                }
                levelStart = levelEnd;
                level = mLevels.at(levelEnd);
            }
        }
    }
}

void MeasuredParagraph::applyMetricsAffectingSpan(const TextPaint& paint,const std::vector<ParcelableSpan*>& spans, int start, int end, MeasuredText::Builder* builder) {
    mCachedPaint.set(paint);
    // XXX paint should not have a baseline shift, but...
    mCachedPaint.baselineShift = 0;

    const bool needFontMetrics = builder != nullptr;

    if (needFontMetrics /*&& mCachedFm == nullptr*/) {
        //mCachedFm = new Paint.FontMetricsInt();
    }

    ReplacementSpan* replacement = nullptr;
    if (!spans.empty()) {
        for (int i = 0; i < spans.size(); i++) {
            MetricAffectingSpan* span = dynamic_cast<MetricAffectingSpan*>(spans[i]);
            if (dynamic_cast<ReplacementSpan*>(span)) {
                // The last ReplacementSpan is effective for backward compatibility reasons.
                replacement = (ReplacementSpan*) span;
            } else {
                // TODO: No need to call updateMeasureState for ReplacementSpan as well?
                span->updateMeasureState(mCachedPaint);
            }
        }
    }

    const int startInCopiedBuffer = start - mTextStart;
    const int endInCopiedBuffer = end - mTextStart;

    if (builder != nullptr) {
        mCachedPaint.getFontMetricsInt(&mCachedFm);
    }

    if (replacement != nullptr) {
        applyReplacementRun(*replacement, startInCopiedBuffer, endInCopiedBuffer, builder);
    } else {
        applyStyleRun(startInCopiedBuffer, endInCopiedBuffer, builder);
    }

    if (needFontMetrics) {
        if (mCachedPaint.baselineShift < 0) {
            mCachedFm.ascent += mCachedPaint.baselineShift;
            mCachedFm.top += mCachedPaint.baselineShift;
        } else {
            mCachedFm.descent += mCachedPaint.baselineShift;
            mCachedFm.bottom += mCachedPaint.baselineShift;
        }

        mFontMetrics.emplace_back(mCachedFm.top);
        mFontMetrics.emplace_back(mCachedFm.bottom);
        mFontMetrics.emplace_back(mCachedFm.ascent);
        mFontMetrics.emplace_back(mCachedFm.descent);
    }
}

int MeasuredParagraph::breakText(int limit, bool forwards, float width) {
    const auto& w = mWidths;
    if (forwards) {
        int i = 0;
        while (i < limit) {
            width -= w[i];
            if (width < 0.0f) break;
            i++;
        }
        while (i > 0 && mCopiedBuffer[i - 1] == ' ') i--;
        return i;
    } else {
        int i = limit - 1;
        while (i >= 0) {
            width -= w[i];
            if (width < 0.0f) break;
            i--;
        }
        while (i < limit - 1 && (mCopiedBuffer[i + 1] == ' ' || w[i + 1] == 0.0f)) {
            i++;
        }
        return limit - i - 1;
    }
}

float MeasuredParagraph::measure(int start, int limit) const{
    float width = 0;
    const auto&  w = mWidths;
    for (int i = start; i < limit; ++i) {
        width += w[i];
    }
    return width;
}

int  MeasuredParagraph::getMemoryUsage() const{
    return mMeasuredText->getMemoryUsage();
}
}/*endof namespace*/
