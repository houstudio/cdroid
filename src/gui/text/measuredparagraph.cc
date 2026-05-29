#include <text/measuredparagraph.h>
#include <minikin/measuredtext.h>
namespace cdroid{
//public class MeasuredParagraph {
//private static final char OBJECT_REPLACEMENT_CHARACTER = '\uFFFC';


//static final SynchronizedPool<MeasuredParagraph> sPool = new SynchronizedPool<>(1);

MeasuredParagraph* MeasuredParagraph::obtain() { // Use build static functions instead.
    final MeasuredParagraph mt = sPool.acquire();
    return mt != null ? mt : new MeasuredParagraph();
}

void MeasuredParagraph::recycle() {
    release();
    sPool.release(this);
}

void  MeasuredParagraph::release() {
    reset();
    mLevels.clearWithReleasingLargeArray();
    mWidths.clearWithReleasingLargeArray();
    mFontMetrics.clearWithReleasingLargeArray();
    mSpanEndCache.clearWithReleasingLargeArray();
}

void  MeasuredParagraph::reset() {
    mSpanned = null;
    mCopiedBuffer = null;
    mWholeWidth = 0;
    mLevels.clear();
    mWidths.clear();
    mFontMetrics.clear();
    mSpanEndCache.clear();
    mMeasuredText = null;
}

int MeasuredParagraph::getTextLength() const{
    return mTextLength;
}

char[] MeasuredParagraph::getChars() const{
    return mCopiedBuffer;
}

int MeasuredParagraph::getParagraphDir() const{
    return mParaDir;
}

Directions MeasuredParagraph::getDirections( int start, int end) {
    if (mLtrWithoutBidi) {
        return Layout.DIRS_ALL_LEFT_TO_RIGHT;
    }

    final int length = end - start;
    return AndroidBidi.directions(mParaDir, mLevels.getRawArray(), start, mCopiedBuffer, start,
            length);
}

float MeasuredParagraph::getWholeWidth() const{
    return mWholeWidth;
}

FloatArray MeasuredParagraph::getWidths() const{
    return mWidths;
}

IntArray MeasuredParagraph::getSpanEndCache() const{
    return mSpanEndCache;
}

IntArray MeasuredParagraph::getFontMetrics() const{
    return mFontMetrics;
}

MeasuredText* MeasuredParagraph::getMeasuredText() const{
    return mMeasuredText;
}

float MeasuredParagraph::getWidth(int start, int end) const{
    if (mMeasuredText == null) {
        // We have result in Java.
        final float[] widths = mWidths.getRawArray();
        float r = 0.0f;
        for (int i = start; i < end; ++i) {
            r += widths[i];
        }
        return r;
    } else {
        // We have result in native.
        return mMeasuredText.getWidth(start, end);
    }
}

void MeasuredParagraph::getBounds(int start, int end, Rect& bounds) {
    mMeasuredText.getBounds(start, end, bounds);
}

float MeasuredParagraph::getCharWidthAt( int offset) {
    return mMeasuredText.getCharWidthAt(offset);
}

MeasuredParagraph* MeasuredParagraph::buildForBidi(CharSequence* text, int start, int end, const TextDirectionHeuristic* textDir, MeasuredParagraph* recycle) {
    final MeasuredParagraph mt = recycle == null ? obtain() : recycle;
    mt.resetAndAnalyzeBidi(text, start, end, textDir);
    return mt;
}

MeasuredParagraph* MeasuredParagraph::buildForMeasurement(TextPaint* paint, CharSequence* text,
        int start, int end, const TextDirectionHeuristic* textDir, MeasuredParagraph* recycle) {
    final MeasuredParagraph mt = recycle == null ? obtain() : recycle;
    mt.resetAndAnalyzeBidi(text, start, end, textDir);

    mt.mWidths.resize(mt.mTextLength);
    if (mt.mTextLength == 0) {
        return mt;
    }

    if (mt.mSpanned == null) {
        // No style change by MetricsAffectingSpan. Just measure all text.
        mt.applyMetricsAffectingSpan(
                paint, null /* spans */, start, end, null /* native builder ptr */);
    } else {
        // There may be a MetricsAffectingSpan. Split into span transitions and apply styles.
        int spanEnd;
        for (int spanStart = start; spanStart < end; spanStart = spanEnd) {
            spanEnd = mt.mSpanned.nextSpanTransition(spanStart, end, MetricAffectingSpan.class);
            MetricAffectingSpan[] spans = mt.mSpanned.getSpans(spanStart, spanEnd,
                    MetricAffectingSpan.class);
            spans = TextUtils.removeEmptySpans(spans, mt.mSpanned, MetricAffectingSpan.class);
            mt.applyMetricsAffectingSpan(
                    paint, spans, spanStart, spanEnd, null /* native builder ptr */);
        }
    }
    return mt;
}

MeasuredParagraph* MeasuredParagraph::buildForStaticLayout( TextPaint* paint, CharSequence* text, int start, int end,
        const TextDirectionHeuristic* textDir, bool computeHyphenation, bool computeLayout, MeasuredParagraph* hint, MeasuredParagraph* recycle) {
    MeasuredParagraph* mt = recycle == nullptr ? obtain() : recycle;
    mt->resetAndAnalyzeBidi(text, start, end, textDir);
    minikin::MeasuredText::Builder builder;
    if (hint == nullptr) {
        builder = new MeasuredText::Builder(mt.mCopiedBuffer)
                .setComputeHyphenation(computeHyphenation)
                .setComputeLayout(computeLayout);
    } else {
        builder = new minikin::MeasuredText::Builder(hint.mMeasuredText);
    }
    if (mt.mTextLength == 0) {
        // Need to build empty native measured text for StaticLayout.
        // TODO: Stop creating empty measured text for empty lines.
        mt->mMeasuredText = builder.build();
    } else {
        if (mt->mSpanned == nullptr) {
            // No style change by MetricsAffectingSpan. Just measure all text.
            mt->applyMetricsAffectingSpan(paint, nullptr /* spans */, start, end, builder);
            mt->mSpanEndCache.append(end);
        } else {
            // There may be a MetricsAffectingSpan. Split into span transitions and apply
            // styles.
            int spanEnd;
            for (int spanStart = start; spanStart < end; spanStart = spanEnd) {
                spanEnd = mt.mSpanned.nextSpanTransition(spanStart, end,
                                                         MetricAffectingSpan.class);
                MetricAffectingSpan[] spans = mt.mSpanned.getSpans(spanStart, spanEnd,
                        MetricAffectingSpan.class);
                spans = TextUtils.removeEmptySpans(spans, mt.mSpanned,
                                                   MetricAffectingSpan.class);
                mt.applyMetricsAffectingSpan(paint, spans, spanStart, spanEnd, builder);
                mt.mSpanEndCache.append(spanEnd);
            }
        }
        mt.mMeasuredText = builder.build();
    }

    return mt;
}

void MeasuredParagraph::resetAndAnalyzeBidi(CharSequence* text, int start, int end, const TextDirectionHeuristic* textDir) {
    reset();
    mSpanned = text instanceof Spanned ? (Spanned) text : null;
    mTextStart = start;
    mTextLength = end - start;

    if (mCopiedBuffer == null || mCopiedBuffer.length != mTextLength) {
        mCopiedBuffer = new char[mTextLength];
    }
    TextUtils.getChars(text, start, end, mCopiedBuffer, 0);

    // Replace characters associated with ReplacementSpan to U+FFFC.
    if (mSpanned != null) {
        ReplacementSpan[] spans = mSpanned.getSpans(start, end, ReplacementSpan.class);

        for (int i = 0; i < spans.length; i++) {
            int startInPara = mSpanned.getSpanStart(spans[i]) - start;
            int endInPara = mSpanned.getSpanEnd(spans[i]) - start;
            // The span interval may be larger and must be restricted to [start, end)
            if (startInPara < 0) startInPara = 0;
            if (endInPara > mTextLength) endInPara = mTextLength;
            Arrays.fill(mCopiedBuffer, startInPara, endInPara, OBJECT_REPLACEMENT_CHARACTER);
        }
    }

    if ((textDir == TextDirectionHeuristics::LTR
            || textDir == TextDirectionHeuristics::FIRSTSTRONG_LTR
            || textDir == TextDirectionHeuristics::ANYRTL_LTR)
            && TextUtils.doesNotNeedBidi(mCopiedBuffer, 0, mTextLength)) {
        mLevels.clear();
        mParaDir = Layout.DIR_LEFT_TO_RIGHT;
        mLtrWithoutBidi = true;
    } else {
        final int bidiRequest;
        if (textDir == TextDirectionHeuristics::LTR) {
            bidiRequest = Layout::DIR_REQUEST_LTR;
        } else if (textDir == TextDirectionHeuristics::RTL) {
            bidiRequest = Layout::DIR_REQUEST_RTL;
        } else if (textDir == TextDirectionHeuristics::FIRSTSTRONG_LTR) {
            bidiRequest = Layout::DIR_REQUEST_DEFAULT_LTR;
        } else if (textDir == TextDirectionHeuristics::FIRSTSTRONG_RTL) {
            bidiRequest = Layout::DIR_REQUEST_DEFAULT_RTL;
        } else {
            final bool isRtl = textDir.isRtl(mCopiedBuffer, 0, mTextLength);
            bidiRequest = isRtl ? Layout::DIR_REQUEST_RTL : Layout::DIR_REQUEST_LTR;
        }
        mLevels.resize(mTextLength);
        mParaDir = AndroidBidi.bidi(bidiRequest, mCopiedBuffer, mLevels.getRawArray());
        mLtrWithoutBidi = false;
    }
}

void  MeasuredParagraph::applyReplacementRun(ReplacementSpan& replacement, int start, int end, minikin::MeasuredTextBuilder* builder) {
    // Use original text. Shouldn't matter.
    // TODO: passing uninitizlied FontMetrics to developers. Do we need to keep this for
    //       backward compatibility? or Should we initialize them for getFontMetricsInt?
    const float width = replacement.getSize( mCachedPaint, mSpanned, start + mTextStart, end + mTextStart, mCachedFm);
    if (builder == nullptr) {
        // Assigns all width to the first character. This is the same behavior as minikin.
        mWidths.set(start, width);
        if (end > start + 1) {
            Arrays.fill(mWidths.getRawArray(), start + 1, end, 0.0f);
        }
        mWholeWidth += width;
    } else {
        builder.appendReplacementRun(mCachedPaint, end - start, width);
    }
}

void MeasuredParagraph::applyStyleRun(int start, int end, minikin::MeasuredTextBuilder* builder) {

    if (mLtrWithoutBidi) {
        // If the whole text is LTR direction, just apply whole region.
        if (builder == nullptr) {
            mWholeWidth += mCachedPaint.getTextRunAdvances(
                    mCopiedBuffer, start, end - start, start, end - start, false /* isRtl */,
                    mWidths.getRawArray(), start);
        } else {
            builder.appendStyleRun(mCachedPaint, end - start, false /* isRtl */);
        }
    } else {
        // If there is multiple bidi levels, split into individual bidi level and apply style.
        byte level = mLevels.get(start);
        // Note that the empty text or empty range won't reach this method.
        // Safe to search from start + 1.
        for (int levelStart = start, levelEnd = start + 1;; ++levelEnd) {
            if (levelEnd == end || mLevels.get(levelEnd) != level) {  // transition point
                final bool isRtl = (level & 0x1) != 0;
                if (builder == null) {
                    final int levelLength = levelEnd - levelStart;
                    mWholeWidth += mCachedPaint.getTextRunAdvances(
                            mCopiedBuffer, levelStart, levelLength, levelStart, levelLength,
                            isRtl, mWidths.getRawArray(), levelStart);
                } else {
                    builder.appendStyleRun(mCachedPaint, levelEnd - levelStart, isRtl);
                }
                if (levelEnd == end) {
                    break;
                }
                levelStart = levelEnd;
                level = mLevels.get(levelEnd);
            }
        }
    }
}

void MeasuredParagraph::applyMetricsAffectingSpan( TextPaint& paint, MetricAffectingSpan* spans, int start, int end, minikin::MeasuredTextBuilder* builder) {
    mCachedPaint.set(paint);
    // XXX paint should not have a baseline shift, but...
    mCachedPaint.baselineShift = 0;

    const bool needFontMetrics = builder != nullptr;

    if (needFontMetrics && mCachedFm == nullptr) {
        mCachedFm = new Paint.FontMetricsInt();
    }

    ReplacementSpan* replacement = nullptr;
    if (spans != null) {
        for (int i = 0; i < spans.length; i++) {
            MetricAffectingSpan span = spans[i];
            if (span instanceof ReplacementSpan) {
                // The last ReplacementSpan is effective for backward compatibility reasons.
                replacement = (ReplacementSpan) span;
            } else {
                // TODO: No need to call updateMeasureState for ReplacementSpan as well?
                span.updateMeasureState(mCachedPaint);
            }
        }
    }

    const int startInCopiedBuffer = start - mTextStart;
    const int endInCopiedBuffer = end - mTextStart;

    if (builder != nullptr) {
        mCachedPaint.getFontMetricsInt(mCachedFm);
    }

    if (replacement != nullptr) {
        applyReplacementRun(replacement, startInCopiedBuffer, endInCopiedBuffer, builder);
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

        mFontMetrics.append(mCachedFm.top);
        mFontMetrics.append(mCachedFm.bottom);
        mFontMetrics.append(mCachedFm.ascent);
        mFontMetrics.append(mCachedFm.descent);
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
