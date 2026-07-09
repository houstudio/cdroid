#include <text/layout.h>
#include <porting/cdlog.h>
#include <text/textline.h>
#include <text/precomputedtext.h>
#include <minikin/GraphemeBreak.h>

namespace cdroid{
const auto MetricAffectingSpanFilter=Predicate<const ParcelableSpan*>([](const ParcelableSpan* span){return dynamic_cast<const MetricAffectingSpan*>(span) != nullptr;});
TextLine* TextLine::sCached[3];
TextLine* TextLine::obtain() {
    TextLine* tl;
    for (int i = sizeof(sCached)/sizeof(sCached[0]); --i >= 0;) {
        if (sCached[i] != nullptr) {
            tl = sCached[i];
            sCached[i] = nullptr;
            return tl;
        }
    }
    tl = new TextLine();
    return tl;
}

TextLine* TextLine::recycle(TextLine* tl) {
    tl->mText = nullptr;
    tl->mPaint = nullptr;
    tl->mDirections = nullptr;
    tl->mSpanned = nullptr;
    delete tl->mTabs;
    tl->mTabs = nullptr;
    tl->mChars.clear();//mChars = nullptr;
    tl->mComputed = nullptr;

    tl->mMetricAffectingSpanSpanSet->recycle();
    tl->mCharacterStyleSpanSet->recycle();
    tl->mReplacementSpanSpanSet->recycle();

    for (int i = 0; i < sizeof(sCached)/sizeof(sCached[0]); ++i) {
        if (sCached[i] == nullptr) {
            sCached[i] = tl;
            break;
        }
    }
    return nullptr;
}

TextLine::TextLine(){
    mTabs = nullptr;
    mMetricAffectingSpanSpanSet=new SpanSet(make_span_filter<MetricAffectingSpan>());
    mCharacterStyleSpanSet=new SpanSet(make_span_filter<CharacterStyle>());
    mReplacementSpanSpanSet=new SpanSet(make_span_filter<ReplacementSpan>());
}

TextLine::~TextLine(){
    delete mTabs;
    delete mMetricAffectingSpanSpanSet;
    delete mCharacterStyleSpanSet;
    delete mReplacementSpanSpanSet;
}

void TextLine::set(const TextPaint* paint, CharSequence* text, int start, int limit, int dir, const Directions* directions,
        bool hasTabs, TabStops* tabStops, int ellipsisStart, int ellipsisEnd, bool useFallbackLineSpacing) {
    mPaint = paint;
    mText = text;
    mStart = start;
    mLen = limit - start;
    mDir = dir;
    mDirections = directions;
    mUseFallbackExtent = useFallbackLineSpacing;
    if (directions == nullptr) {
        //LOGE("Directions cannot be null");
    }
    mHasTabs = hasTabs;
    mSpanned = nullptr;

    bool hasReplacement = false;
    mSpanned = dynamic_cast<Spanned*>(text);
    if (mSpanned != nullptr) {
        mReplacementSpanSpanSet->init(mSpanned, start, limit);
        hasReplacement = mReplacementSpanSpanSet->numberOfSpans > 0;
    }

    mComputed = dynamic_cast<PrecomputedText*>(text);
    if (mComputed) {
        // Here, no need to check line break strategy or hyphenation frequency since there is no
        // line break concept here.
        if (!mComputed->getParams().getTextPaint().equalsForTextMeasurement(*paint)) {
            mComputed = nullptr;
        }
    }

    mCharsValid = hasReplacement;

    if (mCharsValid) {
        if (mChars.empty() || mChars.size() < mLen) {
            mChars.resize(mLen);// = ArrayUtils.newUnpaddedCharArray(mLen);
        }
        TextUtils::getChars(text, start, limit, mChars.data(), 0);
        text->getChars(start,limit,mChars.data(),0);
        if (hasReplacement) {
            // Handle these all at once so we don't have to do it as we go.
            // Replace the first character of each replacement run with the
            // object-replacement character and the remainder with zero width
            // non-break space aka BOM.  Cursor movement code skips these
            // zero-width characters.
            auto& chars = mChars;
            for (int i = start, inext; i < limit; i = inext) {
                inext = mReplacementSpanSpanSet->getNextTransition(i, limit);
                if (mReplacementSpanSpanSet->hasSpansIntersecting(i, inext)
                        && (i - start >= ellipsisEnd || inext - start <= ellipsisStart)) {
                    // transition into a span
                    chars[i - start] = 0xFFFC;//C'\ufffc';
                    for (int j = i - start + 1, e = inext - start; j < e; ++j) {
                        chars[j] = 0xFEFF;//'\ufeff'; // used as ZWNBS, marks positions to skip
                    }
                }
            }
        }
    }
    mTabs = tabStops;
    mAddedWordSpacingInPx = 0;
    mAddedLetterSpacingInPx = 0;
    mIsJustifying = false;

    mEllipsisStart = ellipsisStart != ellipsisEnd ? ellipsisStart : 0;
    mEllipsisEnd = ellipsisStart != ellipsisEnd ? ellipsisEnd : 0;
}

void TextLine::justify(int justificationMode, float justifyWidth) {
    int end = mLen;
    while (end > 0 && isLineEndSpace(mText->charAt(mStart + end - 1))) {
        end--;
    }
    if (justificationMode == Layout::JUSTIFICATION_MODE_INTER_WORD) {
        const float width = std::abs(measure(end, false, nullptr));
        const int spaces = countStretchableSpaces(0, end);
        if (spaces == 0) {
            // There are no stretchable spaces, so we can't help the justification by adding any
            // width.
            return;
        }
        mAddedWordSpacingInPx = (justifyWidth - width) / spaces;
        mAddedLetterSpacingInPx = 0;
    } else {  // justificationMode == Layout::JUSTIFICATION_MODE_INTER_CHARACTER
        LineInfo lineInfo;
        float width = std::abs(measure(end, false, nullptr, nullptr, &lineInfo));

        int lettersCount = lineInfo.getClusterCount();
        if (lettersCount < 2) {
            return;
        }
        mAddedLetterSpacingInPx = (justifyWidth - width) / (lettersCount - 1);
        if (mAddedLetterSpacingInPx > 0.03f) {
            // If the letter spacing is more than 0.03em, ligatures are automatically disabled,
            // so re-calculate everything without ligatures. mPaint points at the caller's mutable
            // TextPaint (Layout's mWorkPaint), so the const_cast below is safe.
            TextPaint* paint = const_cast<TextPaint*>(mPaint);
            const std::string oldFontFeatures = paint->getFontFeatureSettings();
            paint->setFontFeatureSettings(oldFontFeatures + ", \"liga\" off, \"cliga\" off");
            width = std::abs(measure(end, false, nullptr, nullptr, &lineInfo));
            lettersCount = lineInfo.getClusterCount();
            mAddedLetterSpacingInPx = (justifyWidth - width) / (lettersCount - 1);
            paint->setFontFeatureSettings(oldFontFeatures);
        }
        mAddedWordSpacingInPx = 0;
    }
    mIsJustifying = true;
}

void TextLine::draw(Canvas& c, float x, int top, int y, int bottom) {
    float h = 0;
    const int runCount = mDirections->getRunCount();
    for (int runIndex = 0; runIndex < runCount; runIndex++) {
        const int runStart = mDirections->getRunStart(runIndex);
        if (runStart > mLen) break;
        const int runLimit = std::min(runStart + mDirections->getRunLength(runIndex), mLen);
        const bool runIsRtl = mDirections->isRunRtl(runIndex);

        int segStart = runStart;
        for (int j = mHasTabs ? runStart : runLimit; j <= runLimit; j++) {
            if (j == runLimit || charAt(j) == TAB_CHAR) {
                h += drawRun(c, segStart, j, runIsRtl, x + h, top, y, bottom,
                        runIndex != (runCount - 1) || j != mLen);

                if (j != runLimit) {  // charAt(j) == TAB_CHAR
                    h = mDir * nextTab(h * mDir);
                }
                segStart = j + 1;
            }
        }
    }
}

float TextLine::metrics(Paint::FontMetricsInt* fmi, RectF* drawBounds, bool returnDrawWidth,
        LineInfo* lineInfo) {
    if (returnDrawWidth) {
        RectF tmp;
        RectF* db = drawBounds != nullptr ? drawBounds : &tmp;
        db->setEmpty();
        float w = measure(mLen, false, fmi, db, lineInfo);
        float boundsWidth;
        if (w >= 0) {
            boundsWidth = std::max(db->right(), w) - std::min(0.f, db->left);
        } else {
            boundsWidth = std::max(db->right(), 0.f) - std::min(w, db->left);
        }
        if (std::abs(w) > boundsWidth) {
            return w;
        }
        // bounds width is always positive but output of measure is signed width.
        // To be able to use bounds width as signed width, use the sign of the width.
        const float sign = (w < 0) ? -1.f : ((w > 0) ? 1.f : 0.f);
        return sign * boundsWidth;
    } else {
        return measure(mLen, false, fmi, drawBounds, lineInfo);
    }
}

float TextLine::measure(int offset, bool trailing, Paint::FontMetricsInt* fmi,
        RectF* drawBounds, LineInfo* lineInfo) {
    if (offset > mLen) {
        //throw new IndexOutOfBoundsException("offset(" + offset + ") should be less than line limit(" + mLen + ")");
    }
    const int target = trailing ? offset - 1 : offset;
    if (target < 0) {
        if (lineInfo) lineInfo->setClusterCount(0);
        return 0;
    }
    if (lineInfo) {
        // android counts shaping clusters via TextShaper (not ported). CDROID approximates with
        // grapheme clusters (minikin GraphemeBreak) over [0, target], using advances from
        // measureAllBounds. This unblocks INTER_CHARACTER justify's letter-spacing distribution.
        lineInfo->setClusterCount(countClusters(target));
    }

    float h = 0;
    for (int runIndex = 0; runIndex < mDirections->getRunCount(); runIndex++) {
        const int runStart = mDirections->getRunStart(runIndex);
        if (runStart > mLen) break;
        const int runLimit = std::min(runStart + mDirections->getRunLength(runIndex), mLen);
        const bool runIsRtl = mDirections->isRunRtl(runIndex);

        int segStart = runStart;
        for (int j = mHasTabs ? runStart : runLimit; j <= runLimit; j++) {
            if (j == runLimit || charAt(j) == TAB_CHAR) {
                const bool targetIsInThisSegment = target >= segStart && target < j;
                const bool sameDirection = (mDir == Layout::DIR_RIGHT_TO_LEFT) == runIsRtl;

                if (targetIsInThisSegment && sameDirection) {
                    return h + measureRun(segStart, offset, j, runIsRtl, fmi, drawBounds);
                }

                const float segmentWidth = measureRun(segStart, j, j, runIsRtl, fmi, drawBounds);
                h += sameDirection ? segmentWidth : -segmentWidth;

                if (targetIsInThisSegment) {
                    return h + measureRun(segStart, offset, j, runIsRtl, nullptr, drawBounds);
                }

                if (j != runLimit) {  // charAt(j) == TAB_CHAR
                    if (offset == j) {
                        return h;
                    }
                    h = mDir * nextTab(h * mDir);
                    if (target == j) {
                        return h;
                    }
                }

                segStart = j + 1;
            }
        }
    }

    return h;
}

void TextLine::measureAllBounds(float* bounds, float* advances) {
    std::vector<float> localAdvances;
    float* adv = advances;
    if (adv == nullptr) {
        localAdvances.assign(mLen, 0.f);
        adv = localAdvances.data();
    }

    const char16_t* buf;
    std::vector<char16_t> extracted;
    if (mCharsValid) {
        buf = mChars.data();
    } else {
        extracted.resize(mLen);
        mText->getChars(mStart, mStart + mLen, extracted.data(), 0);
        buf = extracted.data();
    }

    float h = 0;
    const int runCount = mDirections->getRunCount();
    for (int runIndex = 0; runIndex < runCount; runIndex++) {
        const int runStart = mDirections->getRunStart(runIndex);
        if (runStart > mLen) break;
        const int runLimit = std::min(runStart + mDirections->getRunLength(runIndex), mLen);
        const bool runIsRtl = mDirections->isRunRtl(runIndex);

        int segStart = runStart;
        for (int j = mHasTabs ? runStart : runLimit; j <= runLimit; j++) {
            if (j == runLimit || charAt(j) == TAB_CHAR) {
                const bool sameDirection = (mDir == Layout::DIR_RIGHT_TO_LEFT) == runIsRtl;

                const int segLen = j - segStart;
                float segmentWidth = 0;
                if (segLen > 0) {
                    segmentWidth = mPaint->getTextRunAdvances(buf, segStart, segLen, segStart, segLen,
                            runIsRtl, adv, segStart);
                }

                const float oldh = h;
                h += sameDirection ? segmentWidth : -segmentWidth;
                float currh = sameDirection ? oldh : h;
                for (int offset = segStart; offset < j && offset < mLen; ++offset) {
                    if (runIsRtl) {
                        bounds[2 * offset + 1] = currh;
                        currh -= adv[offset];
                        bounds[2 * offset] = currh;
                    } else {
                        bounds[2 * offset] = currh;
                        currh += adv[offset];
                        bounds[2 * offset + 1] = currh;
                    }
                }

                if (j != runLimit) {  // charAt(j) == TAB_CHAR
                    float leftX, rightX;
                    if (runIsRtl) {
                        rightX = h;
                        h = mDir * nextTab(h * mDir);
                        leftX = h;
                    } else {
                        leftX = h;
                        h = mDir * nextTab(h * mDir);
                        rightX = h;
                    }
                    bounds[2 * j] = leftX;
                    bounds[2 * j + 1] = rightX;
                    adv[j] = rightX - leftX;
                }

                segStart = j + 1;
            }
        }
    }
}

int TextLine::countClusters(int end) {
    if (end <= 0) return 0;
    const int n = std::min(end, mLen);

    // GraphemeBreak needs per-char advances (for emoji width); get them via measureAllBounds.
    std::vector<float> bounds(2 * mLen);
    std::vector<float> advances(mLen);
    measureAllBounds(bounds.data(), advances.data());

    std::vector<char16_t> extracted;
    const uint16_t* buf;
    if (mCharsValid) {
        buf = reinterpret_cast<const uint16_t*>(mChars.data());
    } else {
        extracted.resize(mLen);
        mText->getChars(mStart, mStart + mLen, extracted.data(), 0);
        buf = reinterpret_cast<const uint16_t*>(extracted.data());
    }

    int count = 1;  // the cluster beginning at offset 0
    for (int i = 1; i < n; i++) {
        if (minikin::GraphemeBreak::isGraphemeBreak(advances.data(), buf, 0, mLen, i)) {
            count++;
        }
    }
    return count;
}

std::vector<float> TextLine::measureAllOffsets(const std::vector<bool>& trailing, Paint::FontMetricsInt* fmi) {
    std::vector<float> measurement(mLen + 1);

    std::vector<int> target(mLen + 1);
    for (int offset = 0; offset < target.size(); ++offset) {
        target[offset] = trailing[offset] ? offset - 1 : offset;
    }
    if (target[0] < 0) {
        measurement[0] = 0;
    }

    float h = 0;
    for (int runIndex = 0; runIndex < mDirections->getRunCount(); runIndex++) {
        const int runStart = mDirections->getRunStart(runIndex);
        if (runStart > mLen) break;
        const int runLimit = std::min(runStart + mDirections->getRunLength(runIndex), mLen);
        const bool runIsRtl = mDirections->isRunRtl(runIndex);

        int segStart = runStart;
        for (int j = mHasTabs ? runStart : runLimit; j <= runLimit; ++j) {
            if (j == runLimit || charAt(j) == TAB_CHAR) {
                const float oldh = h;
                const bool advance = (mDir == Layout::DIR_RIGHT_TO_LEFT) == runIsRtl;
                const float w = measureRun(segStart, j, j, runIsRtl, fmi);
                h += advance ? w : -w;

                const float baseh = advance ? oldh : h;
                Paint::FontMetricsInt* crtfmi = advance ? fmi : nullptr;
                for (int offset = segStart; offset <= j && offset <= mLen; ++offset) {
                    if (target[offset] >= segStart && target[offset] < j) {
                        measurement[offset] =
                                baseh + measureRun(segStart, offset, j, runIsRtl, crtfmi);
                    }
                }

                if (j != runLimit) {  // charAt(j) == TAB_CHAR
                    if (target[j] == j) {
                        measurement[j] = h;
                    }
                    h = mDir * nextTab(h * mDir);
                    if (target[j + 1] == j) {
                        measurement[j + 1] =  h;
                    }
                }

                segStart = j + 1;
            }
        }
    }
    if (target[mLen] == mLen) {
        measurement[mLen] = h;
    }

    return measurement;
}

float TextLine::drawRun(Canvas& c, int start, int limit, bool runIsRtl, float x, int top, int y, int bottom, bool needWidth) {

    if ((mDir == Layout::DIR_LEFT_TO_RIGHT) == runIsRtl) {
        float w = -measureRun(start, limit, limit, runIsRtl, nullptr);
        handleRun(start, limit, limit, runIsRtl, &c, x + w, top, y, bottom, nullptr, nullptr, false);
        return w;
    }

    return handleRun(start, limit, limit, runIsRtl, &c, x, top, y, bottom, nullptr, nullptr, needWidth);
}

int TextLine::getOffsetToLeftRightOf(int cursor, bool toLeft) {
    // 1) The caret marks the leading edge of a character. The character
    // logically before it might be on a different level, and the active caret
    // position is on the character at the lower level. If that character
    // was the previous character, the caret is on its trailing edge.
    // 2) Take this character/edge and move it in the indicated direction.
    // This gives you a new character and a new edge.
    // 3) This position is between two visually adjacent characters.  One of
    // these might be at a lower level.  The active position is on the
    // character at the lower level.
    // 4) If the active position is on the trailing edge of the character,
    // the new caret position is the following logical character, else it
    // is the character.

    int lineStart = 0;
    int lineEnd = mLen;
    bool paraIsRtl = mDir == -1;
    auto& runs = mDirections->mDirections;

    int runIndex, runLevel = 0, runStart = lineStart, runLimit = lineEnd, newCaret = -1;
    bool trailing = false;

    if (cursor == lineStart) {
        runIndex = -2;
    } else if (cursor == lineEnd) {
        runIndex = runs.size();
    } else {
      // First, get information about the run containing the character with
      // the active caret.
      for (runIndex = 0; runIndex < runs.size(); runIndex += 2) {
        runStart = lineStart + runs[runIndex];
        if (cursor >= runStart) {
          runLimit = runStart + (runs[runIndex+1] & Layout::RUN_LENGTH_MASK);
          if (runLimit > lineEnd) {
              runLimit = lineEnd;
          }
          if (cursor < runLimit) {
            runLevel = (runs[runIndex+1] >> Layout::RUN_LEVEL_SHIFT) &
                Layout::RUN_LEVEL_MASK;
            if (cursor == runStart) {
              // The caret is on a run boundary, see if we should
              // use the position on the trailing edge of the previous
              // logical character instead.
              int prevRunIndex, prevRunLevel, prevRunStart, prevRunLimit;
              int pos = cursor - 1;
              for (prevRunIndex = 0; prevRunIndex < runs.size(); prevRunIndex += 2) {
                prevRunStart = lineStart + runs[prevRunIndex];
                if (pos >= prevRunStart) {
                  prevRunLimit = prevRunStart +
                      (runs[prevRunIndex+1] & Layout::RUN_LENGTH_MASK);
                  if (prevRunLimit > lineEnd) {
                      prevRunLimit = lineEnd;
                  }
                  if (pos < prevRunLimit) {
                    prevRunLevel = (runs[prevRunIndex+1] >> Layout::RUN_LEVEL_SHIFT)
                        & Layout::RUN_LEVEL_MASK;
                    if (prevRunLevel < runLevel) {
                      // Start from logically previous character.
                      runIndex = prevRunIndex;
                      runLevel = prevRunLevel;
                      runStart = prevRunStart;
                      runLimit = prevRunLimit;
                      trailing = true;
                      break;
                    }
                  }
                }
              }
            }
            break;
          }
        }
      }

      // caret might be == lineEnd.  This is generally a space or paragraph
      // separator and has an associated run, but might be the end of
      // text, in which case it doesn't.  If that happens, we ran off the
      // end of the run list, and runIndex == runs.length.  In this case,
      // we are at a run boundary so we skip the below test.
      if (runIndex != runs.size()) {
          bool runIsRtl = (runLevel & 0x1) != 0;
          bool advance = toLeft == runIsRtl;
          if (cursor != (advance ? runLimit : runStart) || advance != trailing) {
              // Moving within or into the run, so we can move logically.
              newCaret = getOffsetBeforeAfter(runIndex, runStart, runLimit,
                      runIsRtl, cursor, advance);
              // If the new position is internal to the run, we're at the strong
              // position already so we're finished.
              if (newCaret != (advance ? runLimit : runStart)) {
                  return newCaret;
              }
          }
      }
    }

    // If newCaret is -1, we're starting at a run boundary and crossing
    // into another run. Otherwise we've arrived at a run boundary, and
    // need to figure out which character to attach to.  Note we might
    // need to run this twice, if we cross a run boundary and end up at
    // another run boundary.
    while (true) {
      bool advance = toLeft == paraIsRtl;
      int otherRunIndex = runIndex + (advance ? 2 : -2);
      if (otherRunIndex >= 0 && otherRunIndex < runs.size()) {
        int otherRunStart = lineStart + runs[otherRunIndex];
        int otherRunLimit = otherRunStart +
        (runs[otherRunIndex+1] & Layout::RUN_LENGTH_MASK);
        if (otherRunLimit > lineEnd) {
            otherRunLimit = lineEnd;
        }
        int otherRunLevel = (runs[otherRunIndex+1] >> Layout::RUN_LEVEL_SHIFT) &
            Layout::RUN_LEVEL_MASK;
        bool otherRunIsRtl = (otherRunLevel & 1) != 0;

        advance = toLeft == otherRunIsRtl;
        if (newCaret == -1) {
            newCaret = getOffsetBeforeAfter(otherRunIndex, otherRunStart,
                    otherRunLimit, otherRunIsRtl,
                    advance ? otherRunStart : otherRunLimit, advance);
            if (newCaret == (advance ? otherRunLimit : otherRunStart)) {
                // Crossed and ended up at a new boundary,
                // repeat a second and final time.
                runIndex = otherRunIndex;
                runLevel = otherRunLevel;
                continue;
            }
            break;
        }

        // The new caret is at a boundary.
        if (otherRunLevel < runLevel) {
          // The strong character is in the other run.
          newCaret = advance ? otherRunStart : otherRunLimit;
        }
        break;
      }

      if (newCaret == -1) {
          // We're walking off the end of the line.  The paragraph
          // level is always equal to or lower than any internal level, so
          // the boundaries get the strong caret.
          newCaret = advance ? mLen + 1 : -1;
          break;
      }

      // Else we've arrived at the end of the line.  That's a strong position.
      // We might have arrived here by crossing over a run with no internal
      // breaks and dropping out of the above loop before advancing one final
      // time, so reset the caret.
      // Note, we use '<=' below to handle a situation where the only run
      // on the line is a counter-directional run.  If we're not advancing,
      // we can end up at the 'lineEnd' position but the caret we want is at
      // the lineStart.
      if (newCaret <= lineEnd) {
          newCaret = advance ? lineEnd : lineStart;
      }
      break;
    }

    return newCaret;
}

int TextLine::getOffsetBeforeAfter(int runIndex, int runStart, int runLimit,
        bool runIsRtl, int offset, bool after) {

    if (runIndex < 0 || offset == (after ? mLen : 0)) {
        // Walking off end of line.  Since we don't know
        // what cursor positions are available on other lines, we can't
        // return accurate values.  These are a guess.
        if (after) {
            return TextUtils::getOffsetAfter(mText, offset + mStart) - mStart;
        }
        return TextUtils::getOffsetBefore(mText, offset + mStart) - mStart;
    }

    TextPaint& wp = mWorkPaint;
    wp.set(*mPaint);
    if (mIsJustifying) {
        wp.setWordSpacing(mAddedWordSpacingInPx);
        wp.setLetterSpacing(mAddedLetterSpacingInPx / wp.getTextSize());  // px → em
    }

    int spanStart = runStart;
    int spanLimit;
    if (mSpanned == nullptr) {
        spanLimit = runLimit;
    } else {
        int target = after ? offset + 1 : offset;
        int limit = mStart + runLimit;
        while (true) {
            spanLimit = mSpanned->nextSpanTransition(mStart + spanStart, limit,
                    MetricAffectingSpanFilter) - mStart;
            if (spanLimit >= target) {
                break;
            }
            spanStart = spanLimit;
        }

        auto spans = mSpanned->getSpans(mStart + spanStart,
                mStart + spanLimit, MetricAffectingSpanFilter);
        TextUtils::removeEmptySpans(spans, mSpanned, MetricAffectingSpanFilter);

        if (spans.size() > 0) {
            const ReplacementSpan* replacement = nullptr;
            for (int j = 0; j < spans.size(); j++) {
                const MetricAffectingSpan* span = dynamic_cast<const MetricAffectingSpan*>(spans[j]);
                if (dynamic_cast<const ReplacementSpan*>(span)) {
                    replacement = (const ReplacementSpan*)span;
                } else {
                    span->updateMeasureState(wp);
                }
            }

            if (replacement != nullptr) {
                // If we have a replacement span, we're moving either to
                // the start or end of this span.
                return after ? spanLimit : spanStart;
            }
        }
    }

    int cursorOpt = after ? Paint::CURSOR_AFTER : Paint::CURSOR_BEFORE;
    if (mCharsValid) {
        return wp.getTextRunCursor(mChars.data(), spanStart, spanLimit - spanStart,
                runIsRtl, offset, cursorOpt);
    } else {
        // 3rd arg is COUNT (Paint::getTextRunCursor signature), not END — pass
        // spanLimit - spanStart. The overload now returns a text-absolute offset,
        // so -mStart lifts it to line-relative (matching the mCharsValid branch).
        return wp.getTextRunCursor(mText, mStart + spanStart,
                spanLimit - spanStart, runIsRtl, mStart + offset, cursorOpt) - mStart;
    }
}

void TextLine::expandMetricsFromPaint(Paint::FontMetricsInt& fmi,const TextPaint& wp) {
    const int previousTop     = fmi.top;
    const int previousAscent  = fmi.ascent;
    const int previousDescent = fmi.descent;
    const int previousBottom  = fmi.bottom;
    const int previousLeading = fmi.leading;

    wp.getFontMetricsInt(&fmi);

    updateMetrics(fmi, previousTop, previousAscent, previousDescent, previousBottom,
            previousLeading);
}

void TextLine::updateMetrics(Paint::FontMetricsInt& fmi, int previousTop, int previousAscent,
        int previousDescent, int previousBottom, int previousLeading) {
    fmi.top     = std::min(fmi.top,     previousTop);
    fmi.ascent  = std::min(fmi.ascent,  previousAscent);
    fmi.descent = std::max(fmi.descent, previousDescent);
    fmi.bottom  = std::max(fmi.bottom,  previousBottom);
    fmi.leading = std::max(fmi.leading, previousLeading);
}

void TextLine::drawStroke(TextPaint& wp, Canvas& c, int color, float position,
        float thickness, float xleft, float xright, float baseline) {
    const float strokeTop = baseline + wp.baselineShift + position;

    const int previousColor = wp.getColor();
    const Paint::Style previousStyle = wp.getStyle();
    const bool previousAntiAlias = wp.isAntiAlias();

    wp.setStyle(Paint::Style::FILL);
    wp.setAntiAlias(true);

    wp.setColor(color);
    c.set_color(color);
    //c.drawRect(xleft, strokeTop, xright, strokeTop + thickness, wp);
    c.rectangle(xleft,strokeTop,xright-xleft,thickness);
    c.fill();
    wp.setStyle(previousStyle);
    wp.setColor(previousColor);
    wp.setAntiAlias(previousAntiAlias);
}

float TextLine::getRunAdvance(TextPaint& wp, int start, int end, int contextStart, int contextEnd,
        bool runIsRtl, int offset) {
    if (mCharsValid) {
        return wp.getRunAdvance(mChars.data(), start, end, contextStart, contextEnd, runIsRtl, offset);
    } else {
        const int delta = mStart;
        if (mComputed == nullptr) {
            return wp.getRunAdvance(mText, delta + start, delta + end,
                    delta + contextStart, delta + contextEnd, runIsRtl, delta + offset);
        } else {
            return mComputed->getWidth(start + delta, end + delta);
        }
    }
}

float TextLine::handleText(TextPaint& wp, int start, int end,
        int contextStart, int contextEnd, bool runIsRtl,
        Canvas* c, float x, int top, int y, int bottom,
        Paint::FontMetricsInt* fmi, RectF* drawBounds, bool needWidth, int offset,
        const std::vector<DecorationInfo>* decorations) {

    if (mIsJustifying) {
        wp.setWordSpacing(mAddedWordSpacingInPx);
        wp.setLetterSpacing(mAddedLetterSpacingInPx / wp.getTextSize());  // px → em
    }
    // Get metrics first (even for empty strings or "0" width runs)
    if (fmi != nullptr) {
        expandMetricsFromPaint(*fmi, wp);
    }

    // No need to do anything if the run width is "0"
    if (end == start) {
        return 0.f;
    }

    float totalWidth = 0;

    const int numDecorations = decorations == nullptr ? 0 : decorations->size();
    if (needWidth || (c != nullptr && (wp.bgColor != 0 || numDecorations != 0 || runIsRtl))) {
        totalWidth = getRunAdvance(wp, start, end, contextStart, contextEnd, runIsRtl, offset);

        if (drawBounds != nullptr) {
            // Union this run's ink bounds (offset by the pen position) into drawBounds.
            Rect runRect;
            if (mCharsValid) {
                wp.getTextBounds(mChars.data(), start, end - start, runRect);
            } else if (mComputed != nullptr) {
                mComputed->getBounds(mStart + start, mStart + end, runRect);
            } else {
                wp.getTextBounds(mText, mStart + start, mStart + end, runRect);
            }
            const float dx = runIsRtl ? (x - totalWidth) : x;
            const float rl = runRect.left + dx;
            const float rt = runRect.top;
            const float rr = runRect.right() + dx;
            const float rb = runRect.bottom();
            if (drawBounds->empty()) {
                drawBounds->set(rl, rt, rr - rl, rb - rt);
            } else {
                const float nl = std::min(drawBounds->left, rl);
                const float nt = std::min(drawBounds->top, rt);
                const float nr = std::max(drawBounds->right(), rr);
                const float nb = std::max(drawBounds->bottom(), rb);
                drawBounds->set(nl, nt, nr - nl, nb - nt);
            }
        }
    }

    if (c != nullptr) {
        float leftX, rightX;
        if (runIsRtl) {
            leftX = x - totalWidth;
            rightX = x;
        } else {
            leftX = x;
            rightX = x + totalWidth;
        }

        if (wp.bgColor != 0) {
            int previousColor = wp.getColor();
            Paint::Style previousStyle = wp.getStyle();

            wp.setColor(wp.bgColor);
            wp.setStyle(Paint::Style::FILL);
            c->rectangle(leftX,top,totalWidth,bottom-top);//drawRect(leftX, top, rightX, bottom, wp);
            c->fill();
            wp.setStyle(previousStyle);
            wp.setColor(previousColor);
        }

        drawTextRun(*c, wp, start, end, contextStart, contextEnd, runIsRtl,
                leftX, y + wp.baselineShift);

        if (numDecorations != 0) {
            for (int i = 0; i < numDecorations; i++) {
                const DecorationInfo& info = decorations->at(i);

                const int decorationStart = std::max(info.start, start);
                const int decorationEnd = std::min(info.end, offset);
                float decorationStartAdvance = getRunAdvance(
                        wp, start, end, contextStart, contextEnd, runIsRtl, decorationStart);
                float decorationEndAdvance = getRunAdvance(
                        wp, start, end, contextStart, contextEnd, runIsRtl, decorationEnd);
                float decorationXLeft, decorationXRight;
                if (runIsRtl) {
                    decorationXLeft = rightX - decorationEndAdvance;
                    decorationXRight = rightX - decorationStartAdvance;
                } else {
                    decorationXLeft = leftX + decorationStartAdvance;
                    decorationXRight = leftX + decorationEndAdvance;
                }

                // Theoretically, there could be cases where both Paint's and TextPaint's
                // setUnderLineText() are called. For backward compatibility, we need to draw
                // both underlines, the one with custom color first.
                if (info.underlineColor != 0) {
                    drawStroke(wp, *c, info.underlineColor, wp.getUnderlinePosition(),
                            info.underlineThickness, decorationXLeft, decorationXRight, y);
                }
                if (info.isUnderlineText) {
                    const float thickness = std::max(wp.getUnderlineThickness(), 1.0f);
                    drawStroke(wp, *c, wp.getColor(), wp.getUnderlinePosition(), thickness,
                            decorationXLeft, decorationXRight, y);
                }

                if (info.isStrikeThruText) {
                    const float thickness = std::max(wp.getStrikeThruThickness(), 1.0f);
                    drawStroke(wp, *c, wp.getColor(), wp.getStrikeThruPosition(), thickness,
                            decorationXLeft, decorationXRight, y);
                }
            }
        }

    }

    return runIsRtl ? -totalWidth : totalWidth;
}

float TextLine::handleReplacement(const ReplacementSpan& replacement,const TextPaint& wp,
        int start, int limit, bool runIsRtl, Canvas* c, float x, int top,
        int y, int bottom, Paint::FontMetricsInt* fmi, bool needWidth) {

    float ret = 0;

    int textStart = mStart + start;
    int textLimit = mStart + limit;

    if (needWidth || (c != nullptr && runIsRtl)) {
        int previousTop = 0;
        int previousAscent = 0;
        int previousDescent = 0;
        int previousBottom = 0;
        int previousLeading = 0;

        bool needUpdateMetrics = (fmi != nullptr);

        if (needUpdateMetrics) {
            previousTop     = fmi->top;
            previousAscent  = fmi->ascent;
            previousDescent = fmi->descent;
            previousBottom  = fmi->bottom;
            previousLeading = fmi->leading;
        }

        ret = replacement.getSize(wp, mText, textStart, textLimit, fmi);

        if (needUpdateMetrics) {
            updateMetrics(*fmi, previousTop, previousAscent, previousDescent, previousBottom,
                    previousLeading);
        }
    }

    if (c != nullptr) {
        if (runIsRtl) {
            x -= ret;
        }
        replacement.draw(*c, mText, textStart, textLimit, x, top, y, bottom, wp);
    }

    return runIsRtl ? -ret : ret;
}

void TextLine::extractDecorationInfo(TextPaint& paint, DecorationInfo& info) {
    info.isStrikeThruText = paint.isStrikeThruText();
    if (info.isStrikeThruText) {
        paint.setStrikeThruText(false);
    }
    info.isUnderlineText = paint.isUnderlineText();
    if (info.isUnderlineText) {
        paint.setUnderlineText(false);
    }
    info.underlineColor = paint.underlineColor;
    info.underlineThickness = paint.underlineThickness;
    paint.setUnderlineText(0, 0.0f);

    if (info.isUnderlineText || info.underlineColor != 0) {
        const float ts = paint.getTextSize();
        if (paint.getUnderlinePosition() == 0.f)  paint.setUnderlinePosition(ts * 0.18f);
        if (paint.getUnderlineThickness() == 0.f) paint.setUnderlineThickness(std::max(ts * 0.06f, 1.0f));
    }
    if (info.isStrikeThruText) {
        const float ts = paint.getTextSize();
        if (paint.getStrikeThruPosition() == 0.f)  paint.setStrikeThruPosition(-(ts * 0.22f));
        if (paint.getStrikeThruThickness() == 0.f) paint.setStrikeThruThickness(std::max(ts * 0.06f, 1.0f));
    }
}

float TextLine::handleRun(int start, int measureLimit,
        int limit, bool runIsRtl, Canvas* c, float x, int top, int y,
        int bottom, Paint::FontMetricsInt* fmi, RectF* drawBounds, bool needWidth) {

    if (measureLimit < start || measureLimit > limit) {
        //("measureLimit (" + measureLimit + ") is out of "
        //        + "start (" + start + ") and limit (" + limit + ") bounds");
    }

    // Case of an empty line, make sure we update fmi according to mPaint
    if (start == measureLimit) {
        TextPaint& wp = mWorkPaint;
        wp.set(*mPaint);
        if (fmi != nullptr) {
            expandMetricsFromPaint(*fmi, wp);
        }
        return 0.f;
    }

    bool needsSpanMeasurement;
    if (mSpanned == nullptr) {
        needsSpanMeasurement = false;
    } else {
        mMetricAffectingSpanSpanSet->init(mSpanned, mStart + start, mStart + limit);
        mCharacterStyleSpanSet->init(mSpanned, mStart + start, mStart + limit);
        needsSpanMeasurement = mMetricAffectingSpanSpanSet->numberOfSpans != 0
                || mCharacterStyleSpanSet->numberOfSpans != 0;
    }

    if (!needsSpanMeasurement) {
        TextPaint& wp = mWorkPaint;
        wp.set(*mPaint);
        wp.setStartHyphenEdit(adjustStartHyphenEdit(start, wp.getStartHyphenEdit()));
        wp.setEndHyphenEdit(adjustEndHyphenEdit(limit, wp.getEndHyphenEdit()));
        return handleText(wp, start, limit, start, limit, runIsRtl, c, x, top,
                y, bottom, fmi, drawBounds, needWidth, measureLimit, nullptr);
    }

    // Shaping needs to take into account context up to metric boundaries,
    // but rendering needs to take into account character style boundaries.
    // So we iterate through metric runs to get metric bounds,
    // then within each metric run iterate through character style runs
    // for the run bounds.
    const float originalX = x;
    for (int i = start, inext; i < measureLimit; i = inext) {
        TextPaint& wp = mWorkPaint;
        wp.set(*mPaint);

        inext = mMetricAffectingSpanSpanSet->getNextTransition(mStart + i, mStart + limit) - mStart;
        int mlimit = std::min(inext, measureLimit);

        const ReplacementSpan* replacement = nullptr;

        for (int j = 0; j < mMetricAffectingSpanSpanSet->numberOfSpans; j++) {
            // Both intervals [spanStarts..spanEnds] and [mStart + i..mStart + mlimit] are NOT
            // empty by construction. This special case in getSpans() explains the >= & <= tests
            if ((mMetricAffectingSpanSpanSet->spanStarts[j] >= mStart + mlimit)
                    || (mMetricAffectingSpanSpanSet->spanEnds[j] <= mStart + i)) continue;

            const bool insideEllipsis =
                    mStart + mEllipsisStart <= mMetricAffectingSpanSpanSet->spanStarts[j]
                    && mMetricAffectingSpanSpanSet->spanEnds[j] <= mStart + mEllipsisEnd;
            const MetricAffectingSpan* span = dynamic_cast<const MetricAffectingSpan*>(mMetricAffectingSpanSpanSet->spans[j]);
            if (dynamic_cast<const ReplacementSpan*>(span)) {
                replacement = !insideEllipsis ? (const ReplacementSpan*) span : nullptr;
            } else {
                // We might have a replacement that uses the draw
                // state, otherwise measure state would suffice.
                span->updateDrawState(wp);
            }
        }

        if (replacement != nullptr) {
            x += handleReplacement(*replacement, wp, i, mlimit, runIsRtl, c, x, top, y,
                    bottom, fmi, needWidth || mlimit < measureLimit);
            continue;
        }

        TextPaint activePaint = mActivePaint;
        activePaint.set(*mPaint);
        int activeStart = i;
        int activeEnd = mlimit;
        DecorationInfo& decorationInfo = mDecorationInfo;
        mDecorations.clear();
        for (int j = i, jnext; j < mlimit; j = jnext) {
            jnext = mCharacterStyleSpanSet->getNextTransition(mStart + j, mStart + inext) - mStart;

            const int offset = std::min(jnext, mlimit);
            wp.set(*mPaint);
            for (int k = 0; k < mCharacterStyleSpanSet->numberOfSpans; k++) {
                // Intentionally using >= and <= as explained above
                if ((mCharacterStyleSpanSet->spanStarts[k] >= mStart + offset) ||
                        (mCharacterStyleSpanSet->spanEnds[k] <= mStart + j)) continue;

                const CharacterStyle* span = dynamic_cast<const CharacterStyle*>(mCharacterStyleSpanSet->spans[k]);
                span->updateDrawState(wp);
            }

            extractDecorationInfo(wp, decorationInfo);

            if (j == i) {
                // First chunk of text. We can't handle it yet, since we may need to merge it
                // with the next chunk. So we just save the TextPaint for future comparisons
                // and use.
                activePaint.set(wp);
            } else if (!equalAttributes(wp, activePaint)) {
                // The style of the present chunk of text is substantially different from the
                // style of the previous chunk. We need to handle the active piece of text
                // and restart with the present chunk.
                activePaint.setStartHyphenEdit(adjustStartHyphenEdit(activeStart, mPaint->getStartHyphenEdit()));
                activePaint.setEndHyphenEdit(adjustEndHyphenEdit(activeEnd, mPaint->getEndHyphenEdit()));
                x += handleText(activePaint, activeStart, activeEnd, i, inext, runIsRtl, c, x,
                        top, y, bottom, fmi, drawBounds, needWidth || activeEnd < measureLimit,
                        std::min(activeEnd, mlimit), &mDecorations);

                activeStart = j;
                activePaint.set(wp);
                mDecorations.clear();
            } else {
                // The present TextPaint is substantially equal to the last TextPaint except
                // perhaps for decorations. We just need to expand the active piece of text to
                // include the present chunk, which we always do anyway. We don't need to save
                // wp to activePaint, since they are already equal.
            }

            activeEnd = jnext;
            if (decorationInfo.hasDecoration()) {
                DecorationInfo copy = decorationInfo.copyInfo();
                copy.start = j;
                copy.end = jnext;
                mDecorations.emplace_back(copy);
            }
        }
        // Handle the final piece of text.
        activePaint.setStartHyphenEdit(adjustStartHyphenEdit(activeStart, mPaint->getStartHyphenEdit()));
        activePaint.setEndHyphenEdit(adjustEndHyphenEdit(activeEnd, mPaint->getEndHyphenEdit()));
        x += handleText(activePaint, activeStart, activeEnd, i, inext, runIsRtl, c, x,
                top, y, bottom, fmi, drawBounds, needWidth || activeEnd < measureLimit,
                std::min(activeEnd, mlimit), &mDecorations);
    }

    return x - originalX;
}

void TextLine::drawTextRun(Canvas& c, TextPaint& wp, int start, int end,
        int contextStart, int contextEnd, bool runIsRtl, float x, int y) {

    if (mCharsValid) {
        const int count = end - start;
        const int contextCount = contextEnd - contextStart;
        wp.drawTextRun(c,mChars.data(), mStart+start, count, contextStart, contextCount,
                x, y, runIsRtl);
    } else {
        const int delta = mStart;
        std::vector<char16_t>buf(end-start+1);
        mText->getChars(mStart+start,mStart+end,buf.data(),0);
        //LOGD("drawTextRun start=%d end=%d contextStart=%d contextEnd=%d",mStart+start,mStart+end,contextStart,contextEnd);
        wp.drawTextRun(c,buf.data(), 0, end-start, contextStart, delta + contextEnd, x, y, runIsRtl);
    }
}

float TextLine::nextTab(float h) {
    if (mTabs != nullptr) {
        return mTabs->nextTab(h);
    }
    return TabStops::nextDefaultStop(h, TAB_INCREMENT);
}

int TextLine::countStretchableSpaces(int start, int end) const{
    int count = 0;
    for (int i = start; i < end; i++) {
        char c = mCharsValid ? mChars[i] : mText->charAt(i + mStart);
        if (isStretchableWhitespace(c)) {
            count++;
        }
    }
    return count;
}

bool TextLine::isLineEndSpace(char16_t ch) {
    return ch == ' ' || ch == '\t' || ch == 0x1680
            || (0x2000 <= ch && ch <= 0x200A && ch != 0x2007)
            || ch == 0x205F || ch == 0x3000;
}


bool TextLine::equalAttributes(const TextPaint& lp, const TextPaint& rp) {
    return //lp.getColorFilter() == rp.getColorFilter()
           // && lp.getMaskFilter() == rp.getMaskFilter()
           // && lp.getShader() == rp.getShader()
            lp.getTypeface() == rp.getTypeface()
           // && lp.getXfermode() == rp.getXfermode()
           // && lp.getTextLocales().equals(rp.getTextLocales())
           // && TextUtils.equals(lp.getFontFeatureSettings(), rp.getFontFeatureSettings())
           // && TextUtils.equals(lp.getFontVariationSettings(), rp.getFontVariationSettings())
           // && lp.getShadowLayerRadius() == rp.getShadowLayerRadius()
           // && lp.getShadowLayerDx() == rp.getShadowLayerDx()
           // && lp.getShadowLayerDy() == rp.getShadowLayerDy()
           // && lp.getShadowLayerColor() == rp.getShadowLayerColor()
            && lp.getFlags() == rp.getFlags()
           // && lp.getHinting() == rp.getHinting()
            && lp.getStyle() == rp.getStyle()
            && lp.getColor() == rp.getColor()
            && lp.getStrokeWidth() == rp.getStrokeWidth()
            && lp.getStrokeMiter() == rp.getStrokeMiter()
            && lp.getStrokeCap() == rp.getStrokeCap()
            && lp.getStrokeJoin() == rp.getStrokeJoin()
            && lp.getTextAlign() == rp.getTextAlign()
            && lp.isElegantTextHeight() == rp.isElegantTextHeight()
            && lp.getTextSize() == rp.getTextSize()
            && lp.getTextScaleX() == rp.getTextScaleX()
            && lp.getTextSkewX() == rp.getTextSkewX()
            && lp.getLetterSpacing() == rp.getLetterSpacing()
            && lp.getWordSpacing() == rp.getWordSpacing()
            && lp.getStartHyphenEdit() == rp.getStartHyphenEdit()
            && lp.getEndHyphenEdit() == rp.getEndHyphenEdit()
            && lp.bgColor == rp.bgColor
            && lp.baselineShift == rp.baselineShift
            && lp.linkColor == rp.linkColor
            && lp.drawableState == rp.drawableState
            && lp.density == rp.density
            && lp.underlineColor == rp.underlineColor
            && lp.underlineThickness == rp.underlineThickness;
}
}/*endof namespace*/
