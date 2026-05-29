#include <text/textline.h>
namespace cdroid{
TextLine* TextLine::obtain() {
    TextLine tl;
    for (int i = sCached.length; --i >= 0;) {
        if (sCached[i] != null) {
            tl = sCached[i];
            sCached[i] = null;
            return tl;
        }
    }
    tl = new TextLine();
    return tl;
}

TextLine* TextLine::recycle(TextLine* tl) {
    tl->mText = null;
    tl->mPaint = null;
    tl->mDirections = null;
    tl->mSpanned = null;
    tl->mTabs = null;
    tl->mChars = null;
    tl->mComputed = null;

    tl->mMetricAffectingSpanSpanSet.recycle();
    tl->mCharacterStyleSpanSet.recycle();
    tl->mReplacementSpanSpanSet.recycle();

    for (int i = 0; i < sCached.length; ++i) {
        if (sCached[i] == null) {
            sCached[i] = tl;
            break;
        }
    }
    return nullptr;
}

void TextLine::set(const TextPaint* paint, CharSequence* text, int start, int limit, int dir,
        Directions directions, bool hasTabs, TabStops* tabStops, int ellipsisStart, int ellipsisEnd) {
    mPaint = paint;
    mText = text;
    mStart = start;
    mLen = limit - start;
    mDir = dir;
    mDirections = directions;
    if (mDirections == null) {
        throw new IllegalArgumentException("Directions cannot be null");
    }
    mHasTabs = hasTabs;
    mSpanned = null;

    bool hasReplacement = false;
    if (dynamic_cast<Spanned*>(text)) {
        mSpanned = (Spanned*) text;
        mReplacementSpanSpanSet.init(mSpanned, start, limit);
        hasReplacement = mReplacementSpanSpanSet.numberOfSpans > 0;
    }

    mComputed = null;
    if (dynamic_cast<PrecomputedText*>(text)) {
        // Here, no need to check line break strategy or hyphenation frequency since there is no
        // line break concept here.
        mComputed = (PrecomputedText*) text;
        if (!mComputed.getParams().getTextPaint().equalsForTextMeasurement(paint)) {
            mComputed = null;
        }
    }

    mCharsValid = hasReplacement;

    if (mCharsValid) {
        if (mChars == null || mChars.length < mLen) {
            mChars = ArrayUtils.newUnpaddedCharArray(mLen);
        }
        TextUtils.getChars(text, start, limit, mChars, 0);
        if (hasReplacement) {
            // Handle these all at once so we don't have to do it as we go.
            // Replace the first character of each replacement run with the
            // object-replacement character and the remainder with zero width
            // non-break space aka BOM.  Cursor movement code skips these
            // zero-width characters.
            char[] chars = mChars;
            for (int i = start, inext; i < limit; i = inext) {
                inext = mReplacementSpanSpanSet.getNextTransition(i, limit);
                if (mReplacementSpanSpanSet.hasSpansIntersecting(i, inext)
                        && (i - start >= ellipsisEnd || inext - start <= ellipsisStart)) {
                    // transition into a span
                    chars[i - start] = '\ufffc';
                    for (int j = i - start + 1, e = inext - start; j < e; ++j) {
                        chars[j] = '\ufeff'; // used as ZWNBS, marks positions to skip
                    }
                }
            }
        }
    }
    mTabs = tabStops;
    mAddedWidthForJustify = 0;
    mIsJustifying = false;

    mEllipsisStart = ellipsisStart != ellipsisEnd ? ellipsisStart : 0;
    mEllipsisEnd = ellipsisStart != ellipsisEnd ? ellipsisEnd : 0;
}

void TextLine::justify(float justifyWidth) {
    int end = mLen;
    while (end > 0 && isLineEndSpace(mText->charAt(mStart + end - 1))) {
        end--;
    }
    int spaces = countStretchableSpaces(0, end);
    if (spaces == 0) {
        // There are no stretchable spaces, so we can't help the justification by adding any
        // width.
        return;
    }
    const float width = std::abs(measure(end, false, nullptr));
    mAddedWidthForJustify = (justifyWidth - width) / spaces;
    mIsJustifying = true;
}

void TextLine::draw(Canvas& c, float x, int top, int y, int bottom) {
    float h = 0;
    const int runCount = mDirections.getRunCount();
    for (int runIndex = 0; runIndex < runCount; runIndex++) {
        const int runStart = mDirections.getRunStart(runIndex);
        if (runStart > mLen) break;
        const int runLimit = Math.min(runStart + mDirections.getRunLength(runIndex), mLen);
        const bool runIsRtl = mDirections.isRunRtl(runIndex);

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

float TextLine::metrics(FontMetricsInt& fmi) {
    return measure(mLen, false, fmi);
}

float TextLine::measure(int offset, bool trailing, FontMetricsInt* fmi) {
    if (offset > mLen) {
        //throw new IndexOutOfBoundsException("offset(" + offset + ") should be less than line limit(" + mLen + ")");
    }
    const int target = trailing ? offset - 1 : offset;
    if (target < 0) {
        return 0;
    }

    float h = 0;
    for (int runIndex = 0; runIndex < mDirections.getRunCount(); runIndex++) {
        const int runStart = mDirections.getRunStart(runIndex);
        if (runStart > mLen) break;
        const int runLimit = Math.min(runStart + mDirections.getRunLength(runIndex), mLen);
        const bool runIsRtl = mDirections.isRunRtl(runIndex);

        int segStart = runStart;
        for (int j = mHasTabs ? runStart : runLimit; j <= runLimit; j++) {
            if (j == runLimit || charAt(j) == TAB_CHAR) {
                const bool targetIsInThisSegment = target >= segStart && target < j;
                const bool sameDirection = (mDir == Layout.DIR_RIGHT_TO_LEFT) == runIsRtl;

                if (targetIsInThisSegment && sameDirection) {
                    return h + measureRun(segStart, offset, j, runIsRtl, fmi);
                }

                const float segmentWidth = measureRun(segStart, j, j, runIsRtl, fmi);
                h += sameDirection ? segmentWidth : -segmentWidth;

                if (targetIsInThisSegment) {
                    return h + measureRun(segStart, offset, j, runIsRtl, null);
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

std::vector<float> TextLine::measureAllOffsets(const std::vector<bool>& trailing, FontMetricsInt* fmi) {
    std::vector<float> measurement(mLen + 1);

    int[] target = new int[mLen + 1];
    for (int offset = 0; offset < target.length; ++offset) {
        target[offset] = trailing[offset] ? offset - 1 : offset;
    }
    if (target[0] < 0) {
        measurement[0] = 0;
    }

    float h = 0;
    for (int runIndex = 0; runIndex < mDirections.getRunCount(); runIndex++) {
        const int runStart = mDirections.getRunStart(runIndex);
        if (runStart > mLen) break;
        const int runLimit = Math.min(runStart + mDirections.getRunLength(runIndex), mLen);
        const bool runIsRtl = mDirections.isRunRtl(runIndex);

        int segStart = runStart;
        for (int j = mHasTabs ? runStart : runLimit; j <= runLimit; ++j) {
            if (j == runLimit || charAt(j) == TAB_CHAR) {
                const float oldh = h;
                const bool advance = (mDir == Layout.DIR_RIGHT_TO_LEFT) == runIsRtl;
                const float w = measureRun(segStart, j, j, runIsRtl, fmi);
                h += advance ? w : -w;

                const float baseh = advance ? oldh : h;
                FontMetricsInt crtfmi = advance ? fmi : null;
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

    if ((mDir == Layout.DIR_LEFT_TO_RIGHT) == runIsRtl) {
        float w = -measureRun(start, limit, limit, runIsRtl, null);
        handleRun(start, limit, limit, runIsRtl, c, x + w, top,
                y, bottom, null, false);
        return w;
    }

    return handleRun(start, limit, limit, runIsRtl, c, x, top,
            y, bottom, null, needWidth);
}

float TextLine::measureRun(int start, int offset, int limit, bool runIsRtl,FontMetricsInt& fmi) {
    return handleRun(start, offset, limit, runIsRtl, null, 0, 0, 0, 0, fmi, true);
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
    int[] runs = mDirections.mDirections;

    int runIndex, runLevel = 0, runStart = lineStart, runLimit = lineEnd, newCaret = -1;
    bool trailing = false;

    if (cursor == lineStart) {
        runIndex = -2;
    } else if (cursor == lineEnd) {
        runIndex = runs.length;
    } else {
      // First, get information about the run containing the character with
      // the active caret.
      for (runIndex = 0; runIndex < runs.length; runIndex += 2) {
        runStart = lineStart + runs[runIndex];
        if (cursor >= runStart) {
          runLimit = runStart + (runs[runIndex+1] & Layout.RUN_LENGTH_MASK);
          if (runLimit > lineEnd) {
              runLimit = lineEnd;
          }
          if (cursor < runLimit) {
            runLevel = (runs[runIndex+1] >>> Layout.RUN_LEVEL_SHIFT) &
                Layout.RUN_LEVEL_MASK;
            if (cursor == runStart) {
              // The caret is on a run boundary, see if we should
              // use the position on the trailing edge of the previous
              // logical character instead.
              int prevRunIndex, prevRunLevel, prevRunStart, prevRunLimit;
              int pos = cursor - 1;
              for (prevRunIndex = 0; prevRunIndex < runs.length; prevRunIndex += 2) {
                prevRunStart = lineStart + runs[prevRunIndex];
                if (pos >= prevRunStart) {
                  prevRunLimit = prevRunStart +
                      (runs[prevRunIndex+1] & Layout.RUN_LENGTH_MASK);
                  if (prevRunLimit > lineEnd) {
                      prevRunLimit = lineEnd;
                  }
                  if (pos < prevRunLimit) {
                    prevRunLevel = (runs[prevRunIndex+1] >>> Layout.RUN_LEVEL_SHIFT)
                        & Layout.RUN_LEVEL_MASK;
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
      if (runIndex != runs.length) {
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
      if (otherRunIndex >= 0 && otherRunIndex < runs.length) {
        int otherRunStart = lineStart + runs[otherRunIndex];
        int otherRunLimit = otherRunStart +
        (runs[otherRunIndex+1] & Layout.RUN_LENGTH_MASK);
        if (otherRunLimit > lineEnd) {
            otherRunLimit = lineEnd;
        }
        int otherRunLevel = (runs[otherRunIndex+1] >>> Layout.RUN_LEVEL_SHIFT) &
            Layout.RUN_LEVEL_MASK;
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
            return TextUtils.getOffsetAfter(mText, offset + mStart) - mStart;
        }
        return TextUtils.getOffsetBefore(mText, offset + mStart) - mStart;
    }

    TextPaint wp = mWorkPaint;
    wp.set(mPaint);
    if (mIsJustifying) {
        wp.setWordSpacing(mAddedWidthForJustify);
    }

    int spanStart = runStart;
    int spanLimit;
    if (mSpanned == null) {
        spanLimit = runLimit;
    } else {
        int target = after ? offset + 1 : offset;
        int limit = mStart + runLimit;
        while (true) {
            spanLimit = mSpanned.nextSpanTransition(mStart + spanStart, limit,
                    MetricAffectingSpan.class) - mStart;
            if (spanLimit >= target) {
                break;
            }
            spanStart = spanLimit;
        }

        MetricAffectingSpan[] spans = mSpanned.getSpans(mStart + spanStart,
                mStart + spanLimit, MetricAffectingSpan.class);
        spans = TextUtils.removeEmptySpans(spans, mSpanned, MetricAffectingSpan.class);

        if (spans.length > 0) {
            ReplacementSpan replacement = null;
            for (int j = 0; j < spans.length; j++) {
                MetricAffectingSpan span = spans[j];
                if (span instanceof ReplacementSpan) {
                    replacement = (ReplacementSpan)span;
                } else {
                    span.updateMeasureState(wp);
                }
            }

            if (replacement != null) {
                // If we have a replacement span, we're moving either to
                // the start or end of this span.
                return after ? spanLimit : spanStart;
            }
        }
    }

    int cursorOpt = after ? Paint.CURSOR_AFTER : Paint.CURSOR_BEFORE;
    if (mCharsValid) {
        return wp.getTextRunCursor(mChars, spanStart, spanLimit - spanStart,
                runIsRtl, offset, cursorOpt);
    } else {
        return wp.getTextRunCursor(mText, mStart + spanStart,
                mStart + spanLimit, runIsRtl, mStart + offset, cursorOpt) - mStart;
    }
}

void TextLine::expandMetricsFromPaint(FontMetricsInt& fmi, TextPaint& wp) {
    const int previousTop     = fmi.top;
    const int previousAscent  = fmi.ascent;
    const int previousDescent = fmi.descent;
    const int previousBottom  = fmi.bottom;
    const int previousLeading = fmi.leading;

    wp.getFontMetricsInt(fmi);

    updateMetrics(fmi, previousTop, previousAscent, previousDescent, previousBottom,
            previousLeading);
}

void TextLine::updateMetrics(FontMetricsInt& fmi, int previousTop, int previousAscent,
        int previousDescent, int previousBottom, int previousLeading) {
    fmi.top     = Math.min(fmi.top,     previousTop);
    fmi.ascent  = Math.min(fmi.ascent,  previousAscent);
    fmi.descent = Math.max(fmi.descent, previousDescent);
    fmi.bottom  = Math.max(fmi.bottom,  previousBottom);
    fmi.leading = Math.max(fmi.leading, previousLeading);
}

void TextLine::drawStroke(TextPaint& wp, Canvas& c, int color, float position,
        float thickness, float xleft, float xright, float baseline) {
    const float strokeTop = baseline + wp.baselineShift + position;

    const int previousColor = wp.getColor();
    const Paint.Style previousStyle = wp.getStyle();
    const bool previousAntiAlias = wp.isAntiAlias();

    wp.setStyle(Paint.Style.FILL);
    wp.setAntiAlias(true);

    wp.setColor(color);
    c.drawRect(xleft, strokeTop, xright, strokeTop + thickness, wp);

    wp.setStyle(previousStyle);
    wp.setColor(previousColor);
    wp.setAntiAlias(previousAntiAlias);
}

float TextLine::getRunAdvance(TextPaint& wp, int start, int end, int contextStart, int contextEnd,
        bool runIsRtl, int offset) {
    if (mCharsValid) {
        return wp.getRunAdvance(mChars, start, end, contextStart, contextEnd, runIsRtl, offset);
    } else {
        const int delta = mStart;
        if (mComputed == null) {
            return wp.getRunAdvance(mText, delta + start, delta + end,
                    delta + contextStart, delta + contextEnd, runIsRtl, delta + offset);
        } else {
            return mComputed.getWidth(start + delta, end + delta);
        }
    }
}

float TextLine::handleText(TextPaint& wp, int start, int end,
        int contextStart, int contextEnd, bool runIsRtl,
        Canvas& c, float x, int top, int y, int bottom,
        FontMetricsInt& fmi, bool needWidth, int offset,
        std::vector<DecorationInfo> decorations) {

    if (mIsJustifying) {
        wp.setWordSpacing(mAddedWidthForJustify);
    }
    // Get metrics first (even for empty strings or "0" width runs)
    if (fmi != null) {
        expandMetricsFromPaint(fmi, wp);
    }

    // No need to do anything if the run width is "0"
    if (end == start) {
        return 0f;
    }

    float totalWidth = 0;

    const int numDecorations = decorations == null ? 0 : decorations.size();
    if (needWidth || (c != null && (wp.bgColor != 0 || numDecorations != 0 || runIsRtl))) {
        totalWidth = getRunAdvance(wp, start, end, contextStart, contextEnd, runIsRtl, offset);
    }

    if (c != null) {
        const float leftX, rightX;
        if (runIsRtl) {
            leftX = x - totalWidth;
            rightX = x;
        } else {
            leftX = x;
            rightX = x + totalWidth;
        }

        if (wp.bgColor != 0) {
            int previousColor = wp.getColor();
            Paint.Style previousStyle = wp.getStyle();

            wp.setColor(wp.bgColor);
            wp.setStyle(Paint.Style.FILL);
            c.drawRect(leftX, top, rightX, bottom, wp);

            wp.setStyle(previousStyle);
            wp.setColor(previousColor);
        }

        drawTextRun(c, wp, start, end, contextStart, contextEnd, runIsRtl,
                leftX, y + wp.baselineShift);

        if (numDecorations != 0) {
            for (int i = 0; i < numDecorations; i++) {
                const DecorationInfo info = decorations.get(i);

                const int decorationStart = Math.max(info.start, start);
                const int decorationEnd = Math.min(info.end, offset);
                float decorationStartAdvance = getRunAdvance(
                        wp, start, end, contextStart, contextEnd, runIsRtl, decorationStart);
                float decorationEndAdvance = getRunAdvance(
                        wp, start, end, contextStart, contextEnd, runIsRtl, decorationEnd);
                const float decorationXLeft, decorationXRight;
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
                    drawStroke(wp, c, info.underlineColor, wp.getUnderlinePosition(),
                            info.underlineThickness, decorationXLeft, decorationXRight, y);
                }
                if (info.isUnderlineText) {
                    const float thickness =
                            Math.max(wp.getUnderlineThickness(), 1.0f);
                    drawStroke(wp, c, wp.getColor(), wp.getUnderlinePosition(), thickness,
                            decorationXLeft, decorationXRight, y);
                }

                if (info.isStrikeThruText) {
                    const float thickness =
                            Math.max(wp.getStrikeThruThickness(), 1.0f);
                    drawStroke(wp, c, wp.getColor(), wp.getStrikeThruPosition(), thickness,
                            decorationXLeft, decorationXRight, y);
                }
            }
        }

    }

    return runIsRtl ? -totalWidth : totalWidth;
}

float TextLine::handleReplacement(ReplacementSpan replacement, TextPaint& wp,
        int start, int limit, bool runIsRtl, Canvas& c,
        float x, int top, int y, int bottom, FontMetricsInt fmi,
        bool needWidth) {

    float ret = 0;

    int textStart = mStart + start;
    int textLimit = mStart + limit;

    if (needWidth || (c != null && runIsRtl)) {
        int previousTop = 0;
        int previousAscent = 0;
        int previousDescent = 0;
        int previousBottom = 0;
        int previousLeading = 0;

        bool needUpdateMetrics = (fmi != null);

        if (needUpdateMetrics) {
            previousTop     = fmi.top;
            previousAscent  = fmi.ascent;
            previousDescent = fmi.descent;
            previousBottom  = fmi.bottom;
            previousLeading = fmi.leading;
        }

        ret = replacement.getSize(wp, mText, textStart, textLimit, fmi);

        if (needUpdateMetrics) {
            updateMetrics(fmi, previousTop, previousAscent, previousDescent, previousBottom,
                    previousLeading);
        }
    }

    if (c != null) {
        if (runIsRtl) {
            x -= ret;
        }
        replacement.draw(c, mText, textStart, textLimit,
                x, top, y, bottom, wp);
    }

    return runIsRtl ? -ret : ret;
}

int TextLine::adjustStartHyphenEdit(int start, int startHyphenEdit) {
    // Only draw hyphens on first in line. Disable them otherwise.
    return start > 0 ? Paint.START_HYPHEN_EDIT_NO_EDIT : startHyphenEdit;
}

int TextLine::adjustEndHyphenEdit(int limit, int endHyphenEdit) {
    // Only draw hyphens on last run in line. Disable them otherwise.
    return limit < mLen ? Paint.END_HYPHEN_EDIT_NO_EDIT : endHyphenEdit;
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
}

float TextLine::handleRun(int start, int measureLimit,
        int limit, bool runIsRtl, Canvas& c, float x, int top, int y,
        int bottom, FontMetricsInt fmi, bool needWidth) {

    if (measureLimit < start || measureLimit > limit) {
        throw new IndexOutOfBoundsException("measureLimit (" + measureLimit + ") is out of "
                + "start (" + start + ") and limit (" + limit + ") bounds");
    }

    // Case of an empty line, make sure we update fmi according to mPaint
    if (start == measureLimit) {
        const TextPaint wp = mWorkPaint;
        wp.set(mPaint);
        if (fmi != null) {
            expandMetricsFromPaint(fmi, wp);
        }
        return 0f;
    }

    const bool needsSpanMeasurement;
    if (mSpanned == null) {
        needsSpanMeasurement = false;
    } else {
        mMetricAffectingSpanSpanSet.init(mSpanned, mStart + start, mStart + limit);
        mCharacterStyleSpanSet.init(mSpanned, mStart + start, mStart + limit);
        needsSpanMeasurement = mMetricAffectingSpanSpanSet.numberOfSpans != 0
                || mCharacterStyleSpanSet.numberOfSpans != 0;
    }

    if (!needsSpanMeasurement) {
        const TextPaint wp = mWorkPaint;
        wp.set(mPaint);
        wp.setStartHyphenEdit(adjustStartHyphenEdit(start, wp.getStartHyphenEdit()));
        wp.setEndHyphenEdit(adjustEndHyphenEdit(limit, wp.getEndHyphenEdit()));
        return handleText(wp, start, limit, start, limit, runIsRtl, c, x, top,
                y, bottom, fmi, needWidth, measureLimit, null);
    }

    // Shaping needs to take into account context up to metric boundaries,
    // but rendering needs to take into account character style boundaries.
    // So we iterate through metric runs to get metric bounds,
    // then within each metric run iterate through character style runs
    // for the run bounds.
    const float originalX = x;
    for (int i = start, inext; i < measureLimit; i = inext) {
        const TextPaint wp = mWorkPaint;
        wp.set(mPaint);

        inext = mMetricAffectingSpanSpanSet.getNextTransition(mStart + i, mStart + limit) -
                mStart;
        int mlimit = Math.min(inext, measureLimit);

        ReplacementSpan replacement = null;

        for (int j = 0; j < mMetricAffectingSpanSpanSet.numberOfSpans; j++) {
            // Both intervals [spanStarts..spanEnds] and [mStart + i..mStart + mlimit] are NOT
            // empty by construction. This special case in getSpans() explains the >= & <= tests
            if ((mMetricAffectingSpanSpanSet.spanStarts[j] >= mStart + mlimit)
                    || (mMetricAffectingSpanSpanSet.spanEnds[j] <= mStart + i)) continue;

            bool insideEllipsis =
                    mStart + mEllipsisStart <= mMetricAffectingSpanSpanSet.spanStarts[j]
                    && mMetricAffectingSpanSpanSet.spanEnds[j] <= mStart + mEllipsisEnd;
            const MetricAffectingSpan span = mMetricAffectingSpanSpanSet.spans[j];
            if (span instanceof ReplacementSpan) {
                replacement = !insideEllipsis ? (ReplacementSpan) span : null;
            } else {
                // We might have a replacement that uses the draw
                // state, otherwise measure state would suffice.
                span.updateDrawState(wp);
            }
        }

        if (replacement != null) {
            x += handleReplacement(replacement, wp, i, mlimit, runIsRtl, c, x, top, y,
                    bottom, fmi, needWidth || mlimit < measureLimit);
            continue;
        }

        const TextPaint activePaint = mActivePaint;
        activePaint.set(mPaint);
        int activeStart = i;
        int activeEnd = mlimit;
        const DecorationInfo decorationInfo = mDecorationInfo;
        mDecorations.clear();
        for (int j = i, jnext; j < mlimit; j = jnext) {
            jnext = mCharacterStyleSpanSet.getNextTransition(mStart + j, mStart + inext) -
                    mStart;

            const int offset = Math.min(jnext, mlimit);
            wp.set(mPaint);
            for (int k = 0; k < mCharacterStyleSpanSet.numberOfSpans; k++) {
                // Intentionally using >= and <= as explained above
                if ((mCharacterStyleSpanSet.spanStarts[k] >= mStart + offset) ||
                        (mCharacterStyleSpanSet.spanEnds[k] <= mStart + j)) continue;

                const CharacterStyle span = mCharacterStyleSpanSet.spans[k];
                span.updateDrawState(wp);
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
                activePaint.setStartHyphenEdit(
                        adjustStartHyphenEdit(activeStart, mPaint.getStartHyphenEdit()));
                activePaint.setEndHyphenEdit(
                        adjustEndHyphenEdit(activeEnd, mPaint.getEndHyphenEdit()));
                x += handleText(activePaint, activeStart, activeEnd, i, inext, runIsRtl, c, x,
                        top, y, bottom, fmi, needWidth || activeEnd < measureLimit,
                        Math.min(activeEnd, mlimit), mDecorations);

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
                const DecorationInfo copy = decorationInfo.copyInfo();
                copy.start = j;
                copy.end = jnext;
                mDecorations.add(copy);
            }
        }
        // Handle the final piece of text.
        activePaint.setStartHyphenEdit(
                adjustStartHyphenEdit(activeStart, mPaint.getStartHyphenEdit()));
        activePaint.setEndHyphenEdit(
                adjustEndHyphenEdit(activeEnd, mPaint.getEndHyphenEdit()));
        x += handleText(activePaint, activeStart, activeEnd, i, inext, runIsRtl, c, x,
                top, y, bottom, fmi, needWidth || activeEnd < measureLimit,
                Math.min(activeEnd, mlimit), mDecorations);
    }

    return x - originalX;
}

void TextLine::drawTextRun(Canvas& c, TextPaint& wp, int start, int end,
        int contextStart, int contextEnd, bool runIsRtl, float x, int y) {

    if (mCharsValid) {
        int count = end - start;
        int contextCount = contextEnd - contextStart;
        c.drawTextRun(mChars, start, count, contextStart, contextCount,
                x, y, runIsRtl, wp);
    } else {
        int delta = mStart;
        c.drawTextRun(mText, delta + start, delta + end,
                delta + contextStart, delta + contextEnd, x, y, runIsRtl, wp);
    }
}

float TextLine::nextTab(float h) {
    if (mTabs != nullptr) {
        return mTabs->nextTab(h);
    }
    return TabStops::nextDefaultStop(h, TAB_INCREMENT);
}

bool TextLine::isStretchableWhitespace(int ch) const{
    // TODO: Support NBSP and other stretchable whitespace (b/34013491 and b/68204709).
    return ch == 0x0020;
}

int TextLine::countStretchableSpaces(int start, int end) const{
    int count = 0;
    for (int i = start; i < end; i++) {
        char c = mCharsValid ? mChars[i] : mText.charAt(i + mStart);
        if (isStretchableWhitespace(c)) {
            count++;
        }
    }
    return count;
}

bool TextLine::isLineEndSpace(char ch) const{
    return ch == ' ' || ch == '\t' || ch == 0x1680
            || (0x2000 <= ch && ch <= 0x200A && ch != 0x2007)
            || ch == 0x205F || ch == 0x3000;
}


bool TextLine::equalAttributes(TextPaint& lp, TextPaint& rp) {
    return lp.getColorFilter() == rp.getColorFilter()
            && lp.getMaskFilter() == rp.getMaskFilter()
            && lp.getShader() == rp.getShader()
            && lp.getTypeface() == rp.getTypeface()
            && lp.getXfermode() == rp.getXfermode()
            && lp.getTextLocales().equals(rp.getTextLocales())
            && TextUtils.equals(lp.getFontFeatureSettings(), rp.getFontFeatureSettings())
            && TextUtils.equals(lp.getFontVariationSettings(), rp.getFontVariationSettings())
            && lp.getShadowLayerRadius() == rp.getShadowLayerRadius()
            && lp.getShadowLayerDx() == rp.getShadowLayerDx()
            && lp.getShadowLayerDy() == rp.getShadowLayerDy()
            && lp.getShadowLayerColor() == rp.getShadowLayerColor()
            && lp.getFlags() == rp.getFlags()
            && lp.getHinting() == rp.getHinting()
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
