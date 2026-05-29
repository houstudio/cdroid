#include <cstdint>
#include <text/layout.h>
#include <text/measuredparagraph.h>
namespace cdroid{

const Directions TextLayout::DIRS_ALL_LEFT_TO_RIGHT({ 0, RUN_LENGTH_MASK });
const Directions TextLayout::DIRS_ALL_RIGHT_TO_LEFT({ 0, RUN_LENGTH_MASK | RUN_RTL_FLAG });

const auto TabStopSpanFilter=Predicate<const ParcelableSpan*>([](const ParcelableSpan* span){return dynamic_cast<const TabStopSpan*>(span) != nullptr;});
const auto LeadingMarginSpanFilter=Predicate<const ParcelableSpan*>([](const ParcelableSpan* span){return dynamic_cast<const LeadingMarginSpan*>(span) != nullptr;});
const auto ReplacementSpanFilter=Predicate<const ParcelableSpan*>([](const ParcelableSpan* span){return dynamic_cast<const ReplacementSpan*>(span) != nullptr;});
const auto AlignmentSpanFilter=Predicate<const ParcelableSpan*>([](const ParcelableSpan* span){return dynamic_cast<const AlignmentSpan*>(span) != nullptr;});
const auto StyleSpanFilter=Predicate<const ParcelableSpan*>([](const ParcelableSpan* span){return dynamic_cast<const StyleSpan*>(span);});

float TextLayout::getDesiredWidth(CharSequence* source, TextPaint& paint) {
    return getDesiredWidth(source, 0, source->length(), paint);
}

float TextLayout::getDesiredWidth(CharSequence* source, int start, int end, TextPaint& paint) {
    return getDesiredWidth(source, start, end, paint, TextDirectionHeuristics::FIRSTSTRONG_LTR);
}

float TextLayout::getDesiredWidth(CharSequence* source, int start, int end, TextPaint& paint,const TextDirectionHeuristic* textDir) {
    return getDesiredWidthWithLimit(source, start, end, paint, textDir, FLT_MAX);
}

float TextLayout::getDesiredWidthWithLimit(CharSequence* source, int start, int end, TextPaint& paint, const TextDirectionHeuristic* textDir, float upperLimit) {
    float need = 0;
    int next;
    for (int i = start; i <= end; i = next) {
        next = TextUtils.indexOf(source, '\n', i, end);

        if (next < 0)
            next = end;
        // note, omits trailing paragraph char
        float w = measurePara(paint, source, i, next, textDir);
        if (w > upperLimit) {
            return upperLimit;
        }
        if (w > need)
            need = w;
        next++;
    }
    return need;
}

TextLayout::TextLayout(CharSequence* text, TextPaint* paint, int width, Alignment align, float spacingMult, float spacingAdd)
    :TextLayout(text, paint, width, align, TextDirectionHeuristics::FIRSTSTRONG_LTR, spacingMult, spacingAdd){
}

TextLayout::TextLayout(CharSequence* text, TextPaint* paint, int width, Alignment align,const TextDirectionHeuristic* textDir, float spacingMult, float spacingAdd) {
    //FATAL_ERROR("Layout: " + width + " < 0");

    // Ensure paint doesn't have baselineShift set.
    // While normally we don't modify the paint the user passed in,
    // we were already doing this in Styled.drawUniformRun with both
    // baselineShift and bgColor.  We probably should reevaluate bgColor.
    if (paint != nullptr) {
        paint->bgColor = 0;
        paint->baselineShift = 0;
    }

    mText = text;
    mPaint = *paint;
    mWidth = width;
    mAlignment = align;
    mSpacingMult = spacingMult;
    mSpacingAdd = spacingAdd;
    mSpannedText = (dynamic_cast<Spanned*>(text)!=nullptr);//text instanceof Spanned;
    mTextDir = textDir;
}

void TextLayout::setJustificationMode(int justificationMode) {
    mJustificationMode = justificationMode;
}

void TextLayout::replaceWith(CharSequence* text, TextPaint& paint,int width, Alignment align, float spacingmult, float spacingadd) {
    if (width < 0) {
        //throw new IllegalArgumentException("Layout: " + width + " < 0");
    }

    mText = text;
    mPaint = paint;
    mWidth = width;
    mAlignment = align;
    mSpacingMult = spacingmult;
    mSpacingAdd = spacingadd;
    mSpannedText = (dynamic_cast<Spanned*>(text)!=nullptr);//text instanceof Spanned;
}

void TextLayout::draw(Canvas& c) {
    draw(c, nullptr, nullptr, 0);
}

void TextLayout::draw(Canvas& canvas, Path* highlight, Paint* highlightPaint, int cursorOffsetVertical) {
    const int64_t lineRange = getLineRangeForDraw(canvas);
    int firstLine = (int)(lineRange>>32);
    int lastLine = (int)(lineRange&0xFFFFFFFF);
    if (lastLine < 0) return;

    drawBackground(canvas, highlight, highlightPaint, cursorOffsetVertical, firstLine, lastLine);
    drawText(canvas, firstLine, lastLine);
}

bool TextLayout::isJustificationRequired(int lineNum) const{
    if (mJustificationMode == JUSTIFICATION_MODE_NONE) return false;
    const int lineEnd = getLineEnd(lineNum);
    return lineEnd < mText->length() && mText->charAt(lineEnd - 1) != '\n';
}

float TextLayout::getJustifyWidth(int lineNum) const{
    Alignment paraAlign = mAlignment;

    int left = 0;
    int right = mWidth;
    const int dir = getParagraphDirection(lineNum);

    std::vector<ParcelableSpan*> spans;
    if (mSpannedText) {
        Spanned* sp = (Spanned*) mText;
        const int start = getLineStart(lineNum);
        const bool isFirstParaLine = (start == 0 || mText->charAt(start - 1) == '\n');

        if (isFirstParaLine) {
            const int spanEnd = sp->nextSpanTransition(start, mText->length(), ParagraphStyleFilter);
            spans = getParagraphSpans(sp, start, spanEnd, ParagraphStyleFilter);

            for (int n = spans.size() - 1; n >= 0; n--) {
                if (dynamic_cast<AlignmentSpan*>(spans[n])) {
                    paraAlign = (Alignment)((AlignmentSpan*) spans[n])->getAlignment();
                    break;
                }
            }
        }

        const int length = spans.size();
        bool useFirstLineMargin = isFirstParaLine;
        for (int n = 0; n < length; n++) {
            if (dynamic_cast<LeadingMarginSpan2*>(spans[n])) {
                int count = ((LeadingMarginSpan2*) spans[n])->getLeadingMarginLineCount();
                int startLine = getLineForOffset(sp->getSpanStart(spans[n]));
                if (lineNum < startLine + count) {
                    useFirstLineMargin = true;
                    break;
                }
            }
        }
        for (int n = 0; n < length; n++) {
            if (dynamic_cast<LeadingMarginSpan*>(spans[n])) {
                LeadingMarginSpan* margin = (LeadingMarginSpan*) spans[n];
                if (dir == DIR_RIGHT_TO_LEFT) {
                    right -= margin->getLeadingMargin(useFirstLineMargin);
                } else {
                    left += margin->getLeadingMargin(useFirstLineMargin);
                }
            }
        }
    }

    Alignment align;
    if (paraAlign == Alignment::ALIGN_LEFT) {
        align = (dir == DIR_LEFT_TO_RIGHT) ?  Alignment::ALIGN_NORMAL : Alignment::ALIGN_OPPOSITE;
    } else if (paraAlign == Alignment::ALIGN_RIGHT) {
        align = (dir == DIR_LEFT_TO_RIGHT) ?  Alignment::ALIGN_OPPOSITE : Alignment::ALIGN_NORMAL;
    } else {
        align = paraAlign;
    }

    int indentWidth;
    if (align == Alignment::ALIGN_NORMAL) {
        if (dir == DIR_LEFT_TO_RIGHT) {
            indentWidth = getIndentAdjust(lineNum, Alignment::ALIGN_LEFT);
        } else {
            indentWidth = -getIndentAdjust(lineNum, Alignment::ALIGN_RIGHT);
        }
    } else if (align == Alignment::ALIGN_OPPOSITE) {
        if (dir == DIR_LEFT_TO_RIGHT) {
            indentWidth = -getIndentAdjust(lineNum, Alignment::ALIGN_RIGHT);
        } else {
            indentWidth = getIndentAdjust(lineNum, Alignment::ALIGN_LEFT);
        }
    } else { // Alignment::ALIGN_CENTER
        indentWidth = getIndentAdjust(lineNum, Alignment::ALIGN_CENTER);
    }

    return right - left - indentWidth;
}

void TextLayout::drawText(Canvas& canvas, int firstLine, int lastLine) {
    int previousLineBottom = getLineTop(firstLine);
    int previousLineEnd = getLineStart(firstLine);
    std::vector<ParcelableSpan*> spans;
    int spanEnd = 0;
    TextPaint paint = mWorkPaint;
    paint.set(mPaint);
    CharSequence* buf = mText;

    Alignment paraAlign = mAlignment;
    TabStops* tabStops = nullptr;
    bool tabStopsIsInitialized = false;

    TextLine* tl = TextLine::obtain();

    // Draw the lines, one at a time.
    // The baseline is the top of the following line minus the current line's descent.
    for (int lineNum = firstLine; lineNum <= lastLine; lineNum++) {
        int start = previousLineEnd;
        previousLineEnd = getLineStart(lineNum + 1);
        const bool justify = isJustificationRequired(lineNum);
        const int end = getLineVisibleEnd(lineNum, start, previousLineEnd);
        paint.setStartHyphenEdit(getStartHyphenEdit(lineNum));
        paint.setEndHyphenEdit(getEndHyphenEdit(lineNum));

        const int ltop = previousLineBottom;
        const int lbottom = getLineTop(lineNum + 1);
        previousLineBottom = lbottom;
        const int lbaseline = lbottom - getLineDescent(lineNum);

        const int dir = getParagraphDirection(lineNum);
        int left = 0;
        int right = mWidth;

        if (mSpannedText) {
            Spanned* sp = (Spanned*) buf;
            int textLength = buf->length();
            bool isFirstParaLine = (start == 0 || buf->charAt(start - 1) == '\n');

            // New batch of paragraph styles, collect into spans array.
            // Compute the alignment, last alignment style wins.
            // Reset tabStops, we'll rebuild if we encounter a line with
            // tabs.
            // We expect paragraph spans to be relatively infrequent, use
            // spanEnd so that we can check less frequently.  Since
            // paragraph styles ought to apply to entire paragraphs, we can
            // just collect the ones present at the start of the paragraph.
            // If spanEnd is before the end of the paragraph, that's not
            // our problem.
            if (start >= spanEnd && (lineNum == firstLine || isFirstParaLine)) {
                spanEnd = sp->nextSpanTransition(start, textLength,ParagraphStyleFilter);
                spans = getParagraphSpans(sp, start, spanEnd, ParagraphStyleFilter);

                paraAlign = mAlignment;
                for (int n = spans.size() - 1; n >= 0; n--) {
                    if (dynamic_cast<AlignmentSpan*>(spans[n])) {
                        paraAlign = (Alignment)((AlignmentSpan*)spans[n])->getAlignment();
                        break;
                    }
                }

                tabStopsIsInitialized = false;
            }

            // Draw all leading margin spans.  Adjust left or right according
            // to the paragraph direction of the line.
            const int length = spans.size();
            bool useFirstLineMargin = isFirstParaLine;
            for (int n = 0; n < length; n++) {
                if (dynamic_cast<LeadingMarginSpan2*>(spans[n])) {
                    int count = ((LeadingMarginSpan2*) spans[n])->getLeadingMarginLineCount();
                    int startLine = getLineForOffset(sp->getSpanStart(spans[n]));
                    // if there is more than one LeadingMarginSpan2, use
                    // the count that is greatest
                    if (lineNum < startLine + count) {
                        useFirstLineMargin = true;
                        break;
                    }
                }
            }
            for (int n = 0; n < length; n++) {
                if (dynamic_cast<LeadingMarginSpan*>(spans[n])) {
                    LeadingMarginSpan* margin = (LeadingMarginSpan*) spans[n];
                    if (dir == DIR_RIGHT_TO_LEFT) {
                        margin->drawLeadingMargin(canvas, paint, right, dir, ltop,
                                lbaseline, lbottom, buf, start, end, isFirstParaLine, this);
                        right -= margin->getLeadingMargin(useFirstLineMargin);
                    } else {
                        margin->drawLeadingMargin(canvas, paint, left, dir, ltop,
                                lbaseline, lbottom, buf, start, end, isFirstParaLine, this);
                        left += margin->getLeadingMargin(useFirstLineMargin);
                    }
                }
            }
        }

        const bool hasTab = getLineContainsTab(lineNum);
        // Can't tell if we have tabs for sure, currently
        if (hasTab && !tabStopsIsInitialized) {
            if (tabStops == nullptr) {
                tabStops = new TabStops(TAB_INCREMENT, spans);
            } else {
                tabStops->reset(TAB_INCREMENT, spans);
            }
            tabStopsIsInitialized = true;
        }

        // Determine whether the line aligns to normal, opposite, or center.
        Alignment align = paraAlign;
        if (align == Alignment::ALIGN_LEFT) {
            align = (dir == DIR_LEFT_TO_RIGHT) ?
                Alignment::ALIGN_NORMAL : Alignment::ALIGN_OPPOSITE;
        } else if (align == Alignment::ALIGN_RIGHT) {
            align = (dir == DIR_LEFT_TO_RIGHT) ?
                Alignment::ALIGN_OPPOSITE : Alignment::ALIGN_NORMAL;
        }

        int x;
        int indentWidth;
        if (align == Alignment::ALIGN_NORMAL) {
            if (dir == DIR_LEFT_TO_RIGHT) {
                indentWidth = getIndentAdjust(lineNum, Alignment::ALIGN_LEFT);
                x = left + indentWidth;
            } else {
                indentWidth = -getIndentAdjust(lineNum, Alignment::ALIGN_RIGHT);
                x = right - indentWidth;
            }
        } else {
            int max = (int)getLineExtent(lineNum, *tabStops, false);
            if (align == Alignment::ALIGN_OPPOSITE) {
                if (dir == DIR_LEFT_TO_RIGHT) {
                    indentWidth = -getIndentAdjust(lineNum, Alignment::ALIGN_RIGHT);
                    x = right - max - indentWidth;
                } else {
                    indentWidth = getIndentAdjust(lineNum, Alignment::ALIGN_LEFT);
                    x = left - max + indentWidth;
                }
            } else { // Alignment::ALIGN_CENTER
                indentWidth = getIndentAdjust(lineNum, Alignment::ALIGN_CENTER);
                max = max & ~1;
                x = ((right + left - max) >> 1) + indentWidth;
            }
        }

        Directions directions = getLineDirections(lineNum);
        if (directions == DIRS_ALL_LEFT_TO_RIGHT && !mSpannedText && !hasTab && !justify) {
            // XXX: assumes there's nothing additional to be done
            //canvas.drawText(buf, start, end, x, lbaseline, paint);
        } else {
            tl->set(&paint, buf, start, end, dir, directions, hasTab, tabStops,
                    getEllipsisStart(lineNum), getEllipsisStart(lineNum) + getEllipsisCount(lineNum));
            if (justify) {
                tl->justify(right - left - indentWidth);
            }
            tl->draw(canvas, x, ltop, lbaseline, lbottom);
        }
    }

    TextLine::recycle(tl);
}

void TextLayout::drawBackground(Canvas& canvas, Path* highlight, Paint* highlightPaint,
        int cursorOffsetVertical, int firstLine, int lastLine) {
    // First, draw LineBackgroundSpans.
    // LineBackgroundSpans know nothing about the alignment, margins, or
    // direction of the layout or line.  XXX: Should they?
    // They are evaluated at each line.
    if (mSpannedText) {
        /*if (mLineBackgroundSpans == null) {
            mLineBackgroundSpans = new SpanSet<LineBackgroundSpan>(LineBackgroundSpan.class);
        }*/

        Spanned* buffer = (Spanned*) mText;
        int textLength = buffer->length();
        mLineBackgroundSpans.init(buffer, 0, textLength);

        if (mLineBackgroundSpans.numberOfSpans > 0) {
            int previousLineBottom = getLineTop(firstLine);
            int previousLineEnd = getLineStart(firstLine);
            std::vector<ParcelableSpan*> spans ;
            int spansLength = 0;
            TextPaint paint = mPaint;
            int spanEnd = 0;
            const int width = mWidth;
            for (int i = firstLine; i <= lastLine; i++) {
                int start = previousLineEnd;
                int end = getLineStart(i + 1);
                previousLineEnd = end;

                int ltop = previousLineBottom;
                int lbottom = getLineTop(i + 1);
                previousLineBottom = lbottom;
                int lbaseline = lbottom - getLineDescent(i);

                if (end >= spanEnd) {
                    // These should be infrequent, so we'll use this so that
                    // we don't have to check as often.
                    spanEnd = mLineBackgroundSpans.getNextTransition(start, textLength);
                    // All LineBackgroundSpans on a line contribute to its background.
                    spansLength = 0;
                    // Duplication of the logic of getParagraphSpans
                    if (start != end || start == 0) {
                        // Equivalent to a getSpans(start, end), but filling the 'spans' local
                        // array instead to reduce memory allocation
                        for (int j = 0; j < mLineBackgroundSpans.numberOfSpans; j++) {
                            // equal test is valid since both intervals are not empty by
                            // construction
                            if (mLineBackgroundSpans.spanStarts[j] >= end ||
                                    mLineBackgroundSpans.spanEnds[j] <= start) continue;
                            spans.push_back(mLineBackgroundSpans.spans[j]);// = GrowingArrayUtils.append( spans, spansLength, mLineBackgroundSpans.spans[j]);
                            spansLength++;
                        }
                    }
                }

                for (int n = 0; n < spansLength; n++) {
                    LineBackgroundSpan* lineBackgroundSpan = (LineBackgroundSpan*) spans[n];
                    lineBackgroundSpan->drawBackground(canvas, paint, 0, width,
                            ltop, lbaseline, lbottom, buffer, start, end, i);
                }
            }
        }
        mLineBackgroundSpans.recycle();
    }

    // There can be a highlight even without spans if we are drawing
    // a non-spanned transformation of a spanned editing buffer.
    if (highlight != nullptr) {
        if (cursorOffsetVertical != 0) canvas.translate(0, cursorOffsetVertical);
        //canvas.drawPath(highlight, highlightPaint);
        highlight->append_to_context(&canvas);
        canvas.fill();
        if (cursorOffsetVertical != 0) canvas.translate(0, -cursorOffsetVertical);
    }
}

static bool getClipBounds(Canvas& cr, Rect&r){
    double x1,  y1, x2, y2;
    cr.get_clip_extents(x1, y1, x2, y2);

    if (x1 >= x2 || y1 >= y2) {
        return false;
    }
    r.left = static_cast<int>(std::floor(x1));
    r.top = static_cast<int>(std::floor(y1));
    r.width  = static_cast<int>(std::ceil(x2) - std::floor(x1));
    r.height = static_cast<int>(std::ceil(y2) - std::floor(y1));
    return true;
}
int64_t TextLayout::getLineRangeForDraw(Canvas& canvas) const{
    int dtop, dbottom;
    Rect sTempRect;
    if (!getClipBounds(canvas,sTempRect)) {
        // Negative range end used as a special flag
        return int32_t(-1);//TextUtils.packRangeInLong(0, -1);
    }
    dtop = sTempRect.top;
    dbottom = sTempRect.bottom();

    const int top = std::max(dtop, 0);
    const int bottom = std::min(getLineTop(getLineCount()), dbottom);

    if (top >= bottom) return (int32_t)(-1);//TextUtils.packRangeInLong(0, -1);
    return (int64_t)getLineForVertical(top)<<32| getLineForVertical(bottom);
}

int TextLayout::getLineStartPos(int line, int left, int right) const{
    // Adjust the point at which to start rendering depending on the
    // alignment of the paragraph.
    Alignment align = getParagraphAlignment(line);
    const int dir = getParagraphDirection(line);

    if (align == Alignment::ALIGN_LEFT) {
        align = (dir == DIR_LEFT_TO_RIGHT) ? Alignment::ALIGN_NORMAL : Alignment::ALIGN_OPPOSITE;
    } else if (align == Alignment::ALIGN_RIGHT) {
        align = (dir == DIR_LEFT_TO_RIGHT) ? Alignment::ALIGN_OPPOSITE : Alignment::ALIGN_NORMAL;
    }

    int x;
    if (align == Alignment::ALIGN_NORMAL) {
        if (dir == DIR_LEFT_TO_RIGHT) {
            x = left + getIndentAdjust(line, Alignment::ALIGN_LEFT);
        } else {
            x = right + getIndentAdjust(line, Alignment::ALIGN_RIGHT);
        }
    } else {
        TabStops* tabStops = nullptr;
        if (mSpannedText && getLineContainsTab(line)) {
            Spanned* spanned = (Spanned*) mText;
            int start = getLineStart(line);
            int spanEnd = spanned->nextSpanTransition(start, spanned->length(), TabStopSpanFilter);
            auto tabSpans = getParagraphSpans(spanned, start, spanEnd, TabStopSpanFilter);
            if (tabSpans.size() > 0) {
                tabStops = new TabStops(TAB_INCREMENT, tabSpans);
            }
        }
        int max = (int)getLineExtent(line, *tabStops, false);
        if (align == Alignment::ALIGN_OPPOSITE) {
            if (dir == DIR_LEFT_TO_RIGHT) {
                x = right - max + getIndentAdjust(line, Alignment::ALIGN_RIGHT);
            } else {
                // max is negative here
                x = left - max + getIndentAdjust(line, Alignment::ALIGN_LEFT);
            }
        } else { // Alignment::ALIGN_CENTER
            max = max & ~1;
            x = (left + right - max) >> 1 + getIndentAdjust(line, Alignment::ALIGN_CENTER);
        }
    }
    return x;
}

void TextLayout::increaseWidthTo(int wid) {
    if (wid < mWidth) {
        //throw new RuntimeException("attempted to reduce Layout width");
    }
    mWidth = wid;
}

int TextLayout::getLineBounds(int line, Rect* bounds) const{
    if (bounds != nullptr) {
        bounds->left = 0;     // ???
        bounds->top = getLineTop(line);
        bounds->width = mWidth;   // ???
        bounds->height = getLineTop(line + 1);
    }
    return getLineBaseline(line);
}

bool TextLayout::isLevelBoundary(int offset) const{
    int line = getLineForOffset(offset);
    Directions dirs = getLineDirections(line);
    if (dirs == DIRS_ALL_LEFT_TO_RIGHT || dirs == DIRS_ALL_RIGHT_TO_LEFT) {
        return false;
    }

    auto& runs = dirs.mDirections;
    int lineStart = getLineStart(line);
    int lineEnd = getLineEnd(line);
    if (offset == lineStart || offset == lineEnd) {
        int paraLevel = getParagraphDirection(line) == 1 ? 0 : 1;
        int runIndex = offset == lineStart ? 0 : runs.size() - 2;
        return ((runs[runIndex + 1] >> RUN_LEVEL_SHIFT) & RUN_LEVEL_MASK) != paraLevel;
    }

    offset -= lineStart;
    for (int i = 0; i < runs.size(); i += 2) {
        if (offset == runs[i]) {
            return true;
        }
    }
    return false;
}

bool TextLayout::isRtlCharAt(int offset) const{
    int line = getLineForOffset(offset);
    Directions dirs = getLineDirections(line);
    if (dirs == DIRS_ALL_LEFT_TO_RIGHT) {
        return false;
    }
    if (dirs == DIRS_ALL_RIGHT_TO_LEFT) {
        return  true;
    }
    auto& runs = dirs.mDirections;
    int lineStart = getLineStart(line);
    for (int i = 0; i < runs.size(); i += 2) {
        int start = lineStart + runs[i];
        int limit = start + (runs[i+1] & RUN_LENGTH_MASK);
        if (offset >= start && offset < limit) {
            int level = (runs[i+1] >> RUN_LEVEL_SHIFT) & RUN_LEVEL_MASK;
            return ((level & 1) != 0);
        }
    }
    // Should happen only if the offset is "out of bounds"
    return false;
}

int64_t TextLayout::getRunRange(int offset) const{
    int line = getLineForOffset(offset);
    Directions dirs = getLineDirections(line);
    if (dirs == DIRS_ALL_LEFT_TO_RIGHT || dirs == DIRS_ALL_RIGHT_TO_LEFT) {
        return getLineEnd(line);//TextUtils.packRangeInLong(0, getLineEnd(line));
    }
    auto& runs = dirs.mDirections;
    int lineStart = getLineStart(line);
    for (int i = 0; i < runs.size(); i += 2) {
        int start = lineStart + runs[i];
        int limit = start + (runs[i+1] & RUN_LENGTH_MASK);
        if (offset >= start && offset < limit) {
            return (int64_t)start<<32|limit;//TextUtils.packRangeInLong(start, limit);
        }
    }
    // Should happen only if the offset is "out of bounds"
    return getLineEnd(line);//TextUtils.packRangeInLong(0, getLineEnd(line));
}

bool TextLayout::primaryIsTrailingPrevious(int offset) const{
    int line = getLineForOffset(offset);
    int lineStart = getLineStart(line);
    int lineEnd = getLineEnd(line);
    auto runs = getLineDirections(line).mDirections;

    int levelAt = -1;
    for (int i = 0; i < runs.size(); i += 2) {
        int start = lineStart + runs[i];
        int limit = start + (runs[i+1] & RUN_LENGTH_MASK);
        if (limit > lineEnd) {
            limit = lineEnd;
        }
        if (offset >= start && offset < limit) {
            if (offset > start) {
                // Previous character is at same level, so don't use trailing.
                return false;
            }
            levelAt = (runs[i+1] >> RUN_LEVEL_SHIFT) & RUN_LEVEL_MASK;
            break;
        }
    }
    if (levelAt == -1) {
        // Offset was limit of line.
        levelAt = getParagraphDirection(line) == 1 ? 0 : 1;
    }

    // At level boundary, check previous level.
    int levelBefore = -1;
    if (offset == lineStart) {
        levelBefore = getParagraphDirection(line) == 1 ? 0 : 1;
    } else {
        offset -= 1;
        for (int i = 0; i < runs.size(); i += 2) {
            int start = lineStart + runs[i];
            int limit = start + (runs[i+1] & RUN_LENGTH_MASK);
            if (limit > lineEnd) {
                limit = lineEnd;
            }
            if (offset >= start && offset < limit) {
                levelBefore = (runs[i+1] >> RUN_LEVEL_SHIFT) & RUN_LEVEL_MASK;
                break;
            }
        }
    }

    return levelBefore < levelAt;
}

std::vector<bool> TextLayout::primaryIsTrailingPreviousAllLineOffsets(int line) const{
    const int lineStart = getLineStart(line);
    const int lineEnd = getLineEnd(line);
    auto runs = getLineDirections(line).mDirections;

    std::vector<bool> trailing (lineEnd - lineStart + 1);

    std::vector<uint8_t> level (lineEnd - lineStart + 1);
    for (int i = 0; i < runs.size(); i += 2) {
        int start = lineStart + runs[i];
        int limit = start + (runs[i + 1] & RUN_LENGTH_MASK);
        if (limit > lineEnd) {
            limit = lineEnd;
        }
        if (limit == start) {
            continue;
        }
        level[limit - lineStart - 1] =
                (uint8_t) ((runs[i + 1] >> RUN_LEVEL_SHIFT) & RUN_LEVEL_MASK);
    }

    for (int i = 0; i < runs.size(); i += 2) {
        const int start = lineStart + runs[i];
        uint8_t currentLevel = (uint8_t) ((runs[i + 1] >> RUN_LEVEL_SHIFT) & RUN_LEVEL_MASK);
        trailing[start - lineStart] = currentLevel > (start == lineStart
                ? (getParagraphDirection(line) == 1 ? 0 : 1)
                : level[start - lineStart - 1]);
    }

    return trailing;
}

float TextLayout::getHorizontal(int offset, bool primary) const{
    return primary ? getPrimaryHorizontal(offset) : getSecondaryHorizontal(offset);
}

float TextLayout::getHorizontal(int offset, bool trailing, bool clamped) const{
    int line = getLineForOffset(offset);
    return getHorizontal(offset, trailing, line, clamped);
}

float TextLayout::getHorizontal(int offset, bool trailing, int line, bool clamped) const{
    int start = getLineStart(line);
    int end = getLineEnd(line);
    int dir = getParagraphDirection(line);
    bool hasTab = getLineContainsTab(line);
    Directions directions = getLineDirections(line);

    TabStops* tabStops = nullptr;
    if (hasTab && dynamic_cast<Spanned*>(mText)) {
        // Just checking this line should be good enough, tabs should be
        // consistent across all lines in a paragraph.
        auto tabs = getParagraphSpans((Spanned*) mText, start, end, TabStopSpanFilter);
        if (tabs.size() > 0) {
            tabStops = new TabStops(TAB_INCREMENT, tabs); // XXX should reuse
        }
    }

    TextLine* tl = TextLine::obtain();
    tl->set(&mPaint, mText, start, end, dir, directions, hasTab, tabStops,
            getEllipsisStart(line), getEllipsisStart(line) + getEllipsisCount(line));
    float wid = tl->measure(offset - start, trailing, nullptr);
    TextLine::recycle(tl);

    if (clamped && wid > mWidth) {
        wid = mWidth;
    }
    int left = getParagraphLeft(line);
    int right = getParagraphRight(line);

    return getLineStartPos(line, left, right) + wid;
}

std::vector<float> TextLayout::getLineHorizontals(int line, bool clamped, bool primary) {
    int start = getLineStart(line);
    int end = getLineEnd(line);
    int dir = getParagraphDirection(line);
    bool hasTab = getLineContainsTab(line);
    Directions directions = getLineDirections(line);

    TabStops* tabStops = nullptr;
    if (hasTab && dynamic_cast<Spanned*>(mText)) {
        // Just checking this line should be good enough, tabs should be
        // consistent across all lines in a paragraph.
        auto tabs = getParagraphSpans((Spanned*) mText, start, end, TabStopSpanFilter);
        if (tabs.size() > 0) {
            tabStops = new TabStops(TAB_INCREMENT, tabs); // XXX should reuse
        }
    }

    TextLine* tl = TextLine::obtain();
    tl->set(&mPaint, mText, start, end, dir, directions, hasTab, tabStops,
            getEllipsisStart(line), getEllipsisStart(line) + getEllipsisCount(line));
    auto trailings = primaryIsTrailingPreviousAllLineOffsets(line);
    if (!primary) {
        for (int offset = 0; offset < trailings.size(); ++offset) {
            trailings[offset] = !trailings[offset];
        }
    }
    std::vector<float> wid = tl->measureAllOffsets(trailings, nullptr);
    TextLine::recycle(tl);

    if (clamped) {
        for (int offset = 0; offset < wid.size(); ++offset) {
            if (wid[offset] > mWidth) {
                wid[offset] = mWidth;
            }
        }
    }
    int left = getParagraphLeft(line);
    int right = getParagraphRight(line);

    int lineStartPos = getLineStartPos(line, left, right);
    std::vector<float> horizontal(end - start + 1);
    for (int offset = 0; offset < horizontal.size(); ++offset) {
        horizontal[offset] = lineStartPos + wid[offset];
    }
    return horizontal;
}

float TextLayout::getLineLeft(int line) const{
    const int dir = getParagraphDirection(line);
    Alignment align = getParagraphAlignment(line);
    // Before Q, StaticLayout.Builder.setAlignment didn't check whether the input alignment
    // is null. And when it is null, the old behavior is the same as ALIGN_CENTER.
    // To keep consistency, we convert a null alignment to ALIGN_CENTER.
    if (align == Alignment::NONE) {
        align = Alignment::ALIGN_CENTER;
    }

    // First convert combinations of alignment and direction settings to
    // three basic cases: ALIGN_LEFT, ALIGN_RIGHT and ALIGN_CENTER.
    // For unexpected cases, it will fallback to ALIGN_LEFT.
    Alignment resultAlign;
    switch(align) {
        case ALIGN_NORMAL:
            resultAlign =
                    dir == DIR_RIGHT_TO_LEFT ? Alignment::ALIGN_RIGHT : Alignment::ALIGN_LEFT;
            break;
        case ALIGN_OPPOSITE:
            resultAlign =
                    dir == DIR_RIGHT_TO_LEFT ? Alignment::ALIGN_LEFT : Alignment::ALIGN_RIGHT;
            break;
        case ALIGN_CENTER:
            resultAlign = Alignment::ALIGN_CENTER;
            break;
        case ALIGN_RIGHT:
            resultAlign = Alignment::ALIGN_RIGHT;
            break;
        default: /* align == Alignment::ALIGN_LEFT */
            resultAlign = Alignment::ALIGN_LEFT;
    }

    // Here we must use getLineMax() to do the computation, because it maybe overridden by
    // derived class. And also note that line max equals the width of the text in that line
    // plus the leading margin.
    switch (resultAlign) {
        case ALIGN_CENTER:
            // This computation only works when mWidth equals leadingMargin plus
            // the width of text in this line. If this condition doesn't meet anymore,
            // please change here too.
            return (float) std::floor(getParagraphLeft(line) + (mWidth - getLineMax(line)) / 2);
        case ALIGN_RIGHT:
            return mWidth - getLineMax(line);
        default: /* resultAlign == Alignment::ALIGN_LEFT */
            return 0;
    }
    return 0;
}

float TextLayout::getLineRight(int line) const {
    const int dir = getParagraphDirection(line);
    Alignment align = getParagraphAlignment(line);
    // Before Q, StaticLayout.Builder.setAlignment didn't check whether the input alignment
    // is null. And when it is null, the old behavior is the same as ALIGN_CENTER.
    // To keep consistency, we convert a null alignment to ALIGN_CENTER.
    if (align == Alignment::NONE) {
        align = Alignment::ALIGN_CENTER;
    }
    Alignment resultAlign;
    switch(align) {
        case ALIGN_NORMAL:
            resultAlign = dir == DIR_RIGHT_TO_LEFT ? Alignment::ALIGN_RIGHT : Alignment::ALIGN_LEFT;
            break;
        case ALIGN_OPPOSITE:
            resultAlign = dir == DIR_RIGHT_TO_LEFT ? Alignment::ALIGN_LEFT : Alignment::ALIGN_RIGHT;
            break;
        case ALIGN_CENTER:
            resultAlign = Alignment::ALIGN_CENTER;
            break;
        case ALIGN_RIGHT:
            resultAlign = Alignment::ALIGN_RIGHT;
            break;
        default: /* align == Alignment::ALIGN_LEFT */
            resultAlign = Alignment::ALIGN_LEFT;
    }
    switch (resultAlign) {
        case ALIGN_CENTER:
            // This computation only works when mWidth equals leadingMargin plus width of the
            // text in this line. If this condition doesn't meet anymore, please change here.
            return (float) std::ceil(getParagraphRight(line) - (mWidth - getLineMax(line)) / 2);
        case ALIGN_RIGHT:
            return mWidth;
        default: /* resultAlign == Alignment::ALIGN_LEFT */
            return getLineMax(line);
    }
    return 0;
}

float TextLayout::getLineMax(int line) const {
    float margin = getParagraphLeadingMargin(line);
    float signedExtent = getLineExtent(line, false);
    return margin + (signedExtent >= 0 ? signedExtent : -signedExtent);
}

float TextLayout::getLineWidth(int line) const {
    float margin = getParagraphLeadingMargin(line);
    float signedExtent = getLineExtent(line, true);
    return margin + (signedExtent >= 0 ? signedExtent : -signedExtent);
}

float TextLayout::getLineExtent(int line, bool full) const{
    const int start = getLineStart(line);
    const int end = full ? getLineEnd(line) : getLineVisibleEnd(line);

    const bool hasTabs = getLineContainsTab(line);
    TabStops* tabStops = nullptr;
    if (hasTabs && dynamic_cast<Spanned*>(mText) != nullptr) {
        // Just checking this line should be good enough, tabs should be
        // consistent across all lines in a paragraph.
        auto tabs = getParagraphSpans((Spanned*) mText, start, end, TabStopSpanFilter);
        if (tabs.size() > 0) {
            tabStops = new TabStops(TAB_INCREMENT, tabs); // XXX should reuse
        }
    }
    const Directions directions = getLineDirections(line);
    // Returned directions can actually be null
    if (directions.empty()){//directions == nullptr) {
        return 0.f;
    }
    const int dir = getParagraphDirection(line);

    TextLine* tl = TextLine::obtain();
    TextPaint paint = mWorkPaint;
    paint.set(mPaint);
    paint.setStartHyphenEdit(getStartHyphenEdit(line));
    paint.setEndHyphenEdit(getEndHyphenEdit(line));
    tl->set(&paint, mText, start, end, dir, directions, hasTabs, tabStops,
            getEllipsisStart(line), getEllipsisStart(line) + getEllipsisCount(line));
    if (isJustificationRequired(line)) {
        tl->justify(getJustifyWidth(line));
    }
    const float width = tl->metrics(nullptr);
    TextLine::recycle(tl);
    return width;
}

float TextLayout::getLineExtent(int line, TabStops& tabStops, bool full) const{
    const int start = getLineStart(line);
    const int end = full ? getLineEnd(line) : getLineVisibleEnd(line);
    const bool hasTabs = getLineContainsTab(line);
    const Directions directions = getLineDirections(line);
    const int dir = getParagraphDirection(line);

    TextLine* tl = TextLine::obtain();
    TextPaint paint = mWorkPaint;
    paint.set(mPaint);
    paint.setStartHyphenEdit(getStartHyphenEdit(line));
    paint.setEndHyphenEdit(getEndHyphenEdit(line));
    tl->set(&paint, mText, start, end, dir, directions, hasTabs, &tabStops,
            getEllipsisStart(line), getEllipsisStart(line) + getEllipsisCount(line));
    if (isJustificationRequired(line)) {
        tl->justify(getJustifyWidth(line));
    }
    const float width = tl->metrics(nullptr);
    TextLine::recycle(tl);
    return width;
}

int TextLayout::getLineForVertical(int vertical) const{
    int high = getLineCount(), low = -1, guess;

    while (high - low > 1) {
        guess = (high + low) / 2;

        if (getLineTop(guess) > vertical)
            high = guess;
        else
            low = guess;
    }

    if (low < 0)
        return 0;
    else
        return low;
}

int TextLayout::getLineForOffset(int offset) const{
    int high = getLineCount(), low = -1, guess;

    while (high - low > 1) {
        guess = (high + low) / 2;

        if (getLineStart(guess) > offset)
            high = guess;
        else
            low = guess;
    }

    if (low < 0) {
        return 0;
    } else {
        return low;
    }
}

int TextLayout::getOffsetForHorizontal(int line, float horiz, bool primary) const{
    // TODO: use Paint.getOffsetForAdvance to avoid binary search
    const int lineEndOffset = getLineEnd(line);
    const int lineStartOffset = getLineStart(line);

    Directions dirs = getLineDirections(line);

    TextLine* tl = TextLine::obtain();
    // XXX: we don't care about tabs as we just use TextLine#getOffsetToLeftRightOf here.
    tl->set(&mPaint, mText, lineStartOffset, lineEndOffset, getParagraphDirection(line), dirs,
            false, nullptr, getEllipsisStart(line), getEllipsisStart(line) + getEllipsisCount(line));
    HorizontalMeasurementProvider horizontal((TextLayout*)this,line, primary);

    int max;
    if (line == getLineCount() - 1) {
        max = lineEndOffset;
    } else {
        max = tl->getOffsetToLeftRightOf(lineEndOffset - lineStartOffset,
                !isRtlCharAt(lineEndOffset - 1)) + lineStartOffset;
    }
    int best = lineStartOffset;
    float bestdist = std::abs(horizontal.get(lineStartOffset) - horiz);

    for (int i = 0; i < dirs.mDirections.size(); i += 2) {
        int here = lineStartOffset + dirs.mDirections[i];
        int there = here + (dirs.mDirections[i+1] & RUN_LENGTH_MASK);
        bool isRtl = (dirs.mDirections[i+1] & RUN_RTL_FLAG) != 0;
        int swap = isRtl ? -1 : 1;

        if (there > max)
            there = max;
        int high = there - 1 + 1, low = here + 1 - 1, guess;

        while (high - low > 1) {
            guess = (high + low) / 2;
            int adguess = getOffsetAtStartOf(guess);

            if (horizontal.get(adguess) * swap >= horiz * swap) {
                high = guess;
            } else {
                low = guess;
            }
        }

        if (low < here + 1)
            low = here + 1;

        if (low < there) {
            int aft = tl->getOffsetToLeftRightOf(low - lineStartOffset, isRtl) + lineStartOffset;
            low = tl->getOffsetToLeftRightOf(aft - lineStartOffset, !isRtl) + lineStartOffset;
            if (low >= here && low < there) {
                float dist = std::abs(horizontal.get(low) - horiz);
                if (aft < there) {
                    float other = std::abs(horizontal.get(aft) - horiz);

                    if (other < dist) {
                        dist = other;
                        low = aft;
                    }
                }

                if (dist < bestdist) {
                    bestdist = dist;
                    best = low;
                }
            }
        }

        float dist = std::abs(horizontal.get(here) - horiz);

        if (dist < bestdist) {
            bestdist = dist;
            best = here;
        }
    }

    float dist = std::abs(horizontal.get(max) - horiz);

    if (dist <= bestdist) {
        best = max;
    }

    TextLine::recycle(tl);
    return best;
}

///////////////////////////////////////////////////////////////////////////////////

TextLayout::HorizontalMeasurementProvider::HorizontalMeasurementProvider(TextLayout* layout, int line, bool primary) {
    mLayout = layout;
    mLine = line;
    mPrimary = primary;
    init();
}

void TextLayout::HorizontalMeasurementProvider::init() {
    const Directions dirs = mLayout->getLineDirections(mLine);
    if (dirs == DIRS_ALL_LEFT_TO_RIGHT) {
        return;
    }

    mHorizontals = mLayout->getLineHorizontals(mLine, false, mPrimary);
    mLineStartOffset = mLayout->getLineStart(mLine);
}

float TextLayout::HorizontalMeasurementProvider::get(int offset) {
    const int index = offset - mLineStartOffset;
    if (mHorizontals.empty() || index < 0 || index >= mHorizontals.size()) {
        return mLayout->getHorizontal(offset, mPrimary);
    } else {
        return mHorizontals[index];
    }
}

///////////////////////////////////////////////////////////////////////////////////

int TextLayout::getLineVisibleEnd(int line, int start, int end) const{
    CharSequence* text = mText;
    char16_t ch;
    if (line == getLineCount() - 1) {
        return end;
    }

    for (; end > start; end--) {
        ch = text->charAt(end - 1);
        if (ch == '\n') {
            return end - 1;
        }
        if (!TextLine::isLineEndSpace(ch)) {
            break;
        }
    }
    return end;
}

/*int TextLayout::getLineBottom(int line) const{
    return getLineTop(line + 1);
}

int TextLayout::getLineBottomWithoutSpacing(int line) const{
    return getLineTop(line + 1) - getLineExtra(line);
}

int TextLayout::getLineBaseline(int line) const{
    // getLineTop(line+1) == getLineBottom(line)
    return getLineTop(line+1) - getLineDescent(line);
}

int TextLayout::getLineAscent(int line) const{
    // getLineTop(line+1) - getLineDescent(line) == getLineBaseLine(line)
    return getLineTop(line) - (getLineTop(line+1) - getLineDescent(line));
}

int TextLayout::getLineExtra(int line) const{
    return 0;
}

int TextLayout::getOffsetToLeftOf(int offset) const{
    return getOffsetToLeftRightOf(offset, true);
}

int TextLayout::getOffsetToRightOf(int offset) const{
    return getOffsetToLeftRightOf(offset, false);
}*/

int TextLayout::getOffsetToLeftRightOf(int caret, bool toLeft) const{
    int line = getLineForOffset(caret);
    int lineStart = getLineStart(line);
    int lineEnd = getLineEnd(line);
    int lineDir = getParagraphDirection(line);

    bool lineChanged = false;
    bool advance = toLeft == (lineDir == DIR_RIGHT_TO_LEFT);
    // if walking off line, look at the line we're headed to
    if (advance) {
        if (caret == lineEnd) {
            if (line < getLineCount() - 1) {
                lineChanged = true;
                ++line;
            } else {
                return caret; // at very end, don't move
            }
        }
    } else {
        if (caret == lineStart) {
            if (line > 0) {
                lineChanged = true;
                --line;
            } else {
                return caret; // at very start, don't move
            }
        }
    }

    if (lineChanged) {
        lineStart = getLineStart(line);
        lineEnd = getLineEnd(line);
        int newDir = getParagraphDirection(line);
        if (newDir != lineDir) {
            // unusual case.  we want to walk onto the line, but it runs
            // in a different direction than this one, so we fake movement
            // in the opposite direction.
            toLeft = !toLeft;
            lineDir = newDir;
        }
    }
    Directions directions = getLineDirections(line);
    TextLine* tl = TextLine::obtain();
    // XXX: we don't care about tabs
    tl->set(&mPaint, mText, lineStart, lineEnd, lineDir, directions, false, nullptr,
            getEllipsisStart(line), getEllipsisStart(line) + getEllipsisCount(line));
    caret = lineStart + tl->getOffsetToLeftRightOf(caret - lineStart, toLeft);
    TextLine::recycle(tl);
    return caret;
}

int TextLayout::getOffsetAtStartOf(int offset) const{
    // XXX this probably should skip local reorderings and
    // zero-width characters, look at callers
    if (offset == 0)
        return 0;
    CharSequence* text = mText;
    char16_t c = text->charAt(offset);

    if (c >= 0xDC00 && c <= 0xDFFF) {
        char16_t c1 = text->charAt(offset - 1);
        if (c1 >= 0xD800 && c1 <= 0xDBFF)
            offset -= 1;
    }
    if (mSpannedText) {
        auto spans = ((Spanned*) text)->getSpans(offset, offset, ReplacementSpanFilter);
        for (int i = 0; i < spans.size(); i++) {
            int start = ((Spanned*) text)->getSpanStart(spans[i]);
            int end = ((Spanned*) text)->getSpanEnd(spans[i]);
            if (start < offset && end > offset)
                offset = start;
        }
    }
    return offset;
}

bool TextLayout::shouldClampCursor(int line) {
    // Only clamp cursor position in left-aligned displays.
    switch (getParagraphAlignment(line)) {
        case ALIGN_LEFT:
            return true;
        case ALIGN_NORMAL:
            return getParagraphDirection(line) > 0;
        default:
            return false;
    }

}

void TextLayout::getCursorPath(int point, Path& dest, CharSequence* editingBuffer) {
    dest.reset();

    const int line = getLineForOffset(point);
    int top = getLineTop(line);
    int bottom = getLineBottomWithoutSpacing(line);

    const bool clamped = shouldClampCursor(line);
    float h1 = getPrimaryHorizontal(point, clamped) - 0.5f;

    const int caps = 0;//TextKeyListener.getMetaState(editingBuffer, TextKeyListener.META_SHIFT_ON) |
               //TextKeyListener.getMetaState(editingBuffer, TextKeyListener.META_SELECTING);
    const int fn = 0;//TextKeyListener.getMetaState(editingBuffer, TextKeyListener.META_ALT_ON);
    int dist = 0;

    if (caps != 0 || fn != 0) {
        dist = (bottom - top) >> 2;

        if (fn != 0)
            top += dist;
        if (caps != 0)
            bottom -= dist;
    }

    if (h1 < 0.5f)
        h1 = 0.5f;

    dest.move_to(h1, top);
    dest.line_to(h1, bottom);

    if (caps == 2) {
        dest.move_to(h1, bottom);
        dest.line_to(h1 - dist, bottom + dist);
        dest.line_to(h1, bottom);
        dest.line_to(h1 + dist, bottom + dist);
    } else if (caps == 1) {
        dest.move_to(h1, bottom);
        dest.line_to(h1 - dist, bottom + dist);

        dest.move_to(h1 - dist, bottom + dist - 0.5f);
        dest.line_to(h1 + dist, bottom + dist - 0.5f);

        dest.line_to(h1 + dist, bottom + dist);
        dest.line_to(h1, bottom);
    }

    if (fn == 2) {
        dest.move_to(h1, top);
        dest.line_to(h1 - dist, top - dist);
        dest.line_to(h1, top);
        dest.line_to(h1 + dist, top - dist);
    } else if (fn == 1) {
        dest.move_to(h1, top);
        dest.line_to(h1 - dist, top - dist);

        dest.move_to(h1 - dist, top - dist + 0.5f);
        dest.line_to(h1 + dist, top - dist + 0.5f);

        dest.line_to(h1 + dist, top - dist);
        dest.line_to(h1, top);
    }
}

void TextLayout::addSelection(int line, int start, int end,
        int top, int bottom,const SelectionRectangleConsumer& consumer) {
    int linestart = getLineStart(line);
    int lineend = getLineEnd(line);
    Directions dirs = getLineDirections(line);

    if (lineend > linestart && mText->charAt(lineend - 1) == '\n') {
        lineend--;
    }

    for (int i = 0; i < dirs.mDirections.size(); i += 2) {
        int here = linestart + dirs.mDirections[i];
        int there = here + (dirs.mDirections[i + 1] & RUN_LENGTH_MASK);

        if (there > lineend) {
            there = lineend;
        }

        if (start <= there && end >= here) {
            int st = std::max(start, here);
            int en = std::min(end, there);

            if (st != en) {
                float h1 = getHorizontal(st, false, line, false /* not clamped */);
                float h2 = getHorizontal(en, true, line, false /* not clamped */);

                float left = std::min(h1, h2);
                float right = std::max(h1, h2);

                const int layout = ((dirs.mDirections[i + 1] & RUN_RTL_FLAG) != 0)
                                ? TEXT_SELECTION_LAYOUT_RIGHT_TO_LEFT
                                : TEXT_SELECTION_LAYOUT_LEFT_TO_RIGHT;

                consumer/*.accept*/(left, top, right, bottom, layout);
            }
        }
    }
}

void TextLayout::getSelectionPath(int start, int end, Path& dest) {
    dest.reset();
    SelectionRectangleConsumer ff=[&dest](float left, float top, float right, float bottom, int textSelectionLayout) {
        //dest.addRect(left, top, right, bottom, Path.Direction.CW);
        dest.rectangle(left,top,right-left,bottom-top);
    };
    getSelection(start, end, ff);
}

void TextLayout::getSelection(int start, int end,const SelectionRectangleConsumer& consumer) {
    if (start == end) {
        return;
    }
    if (end < start) {
        int temp = end;
        end = start;
        start = temp;
    }
    const int startline = getLineForOffset(start);
    const int endline = getLineForOffset(end);

    int top = getLineTop(startline);
    int bottom = getLineBottomWithoutSpacing(endline);

    if (startline == endline) {
        addSelection(startline, start, end, top, bottom, consumer);
    } else {
        const float width = mWidth;
        addSelection(startline, start, getLineEnd(startline),
                top, getLineBottom(startline), consumer);

        if (getParagraphDirection(startline) == DIR_RIGHT_TO_LEFT) {
            consumer/*.accept*/(getLineLeft(startline), top, 0, getLineBottom(startline),
                    TEXT_SELECTION_LAYOUT_RIGHT_TO_LEFT);
        } else {
            consumer/*.accept*/(getLineRight(startline), top, width, getLineBottom(startline),
                    TEXT_SELECTION_LAYOUT_LEFT_TO_RIGHT);
        }

        for (int i = startline + 1; i < endline; i++) {
            top = getLineTop(i);
            bottom = getLineBottom(i);
            if (getParagraphDirection(i) == DIR_RIGHT_TO_LEFT) {
                consumer/*.accept*/(0, top, width, bottom, TEXT_SELECTION_LAYOUT_RIGHT_TO_LEFT);
            } else {
                consumer/*.accept*/(0, top, width, bottom, TEXT_SELECTION_LAYOUT_LEFT_TO_RIGHT);
            }
        }
        top = getLineTop(endline);
        bottom = getLineBottomWithoutSpacing(endline);
        addSelection(endline, getLineStart(endline), end, top, bottom, consumer);
        if (getParagraphDirection(endline) == DIR_RIGHT_TO_LEFT) {
            consumer/*.accept*/(width, top, getLineRight(endline), bottom,
                    TEXT_SELECTION_LAYOUT_RIGHT_TO_LEFT);
        } else {
            consumer/*.accept*/(0, top, getLineLeft(endline), bottom,
                    TEXT_SELECTION_LAYOUT_LEFT_TO_RIGHT);
        }
    }
}

TextLayout::Alignment TextLayout::getParagraphAlignment(int line) const {
    Alignment align = mAlignment;
    if (mSpannedText) {
        Spanned* sp = (Spanned*) mText;
        auto spans = getParagraphSpans(sp, getLineStart(line), getLineEnd(line),AlignmentSpanFilter);

        int spanLength = spans.size();
        if (spanLength > 0) {
            align = (Alignment)((AlignmentSpan*)spans[spanLength-1])->getAlignment();
        }
    }
    return align;
}

int TextLayout::getParagraphLeft(int line) const{
    const int left = 0;
    const int dir = getParagraphDirection(line);
    if (dir == DIR_RIGHT_TO_LEFT || !mSpannedText) {
        return left; // leading margin has no impact, or no styles
    }
    return getParagraphLeadingMargin(line);
}

int TextLayout::getParagraphRight(int line) const{
    const int right = mWidth;
    const int dir = getParagraphDirection(line);
    if (dir == DIR_LEFT_TO_RIGHT || !mSpannedText) {
        return right; // leading margin has no impact, or no styles
    }
    return right - getParagraphLeadingMargin(line);
}
int TextLayout::getParagraphLeadingMargin(int line) const{
    if (!mSpannedText) {
        return 0;
    }
    Spanned* spanned = (Spanned*) mText;

    int lineStart = getLineStart(line);
    int lineEnd = getLineEnd(line);
    int spanEnd = spanned->nextSpanTransition(lineStart, lineEnd, LeadingMarginSpanFilter);
    auto spans = getParagraphSpans(spanned, lineStart, spanEnd, LeadingMarginSpanFilter);
    if (spans.size() == 0) {
        return 0; // no leading margin span;
    }

    int margin = 0;

    bool useFirstLineMargin = lineStart == 0 || spanned->charAt(lineStart - 1) == '\n';
    for (int i = 0; i < spans.size(); i++) {
        if (dynamic_cast<LeadingMarginSpan2*>(spans[i])) {
            int spStart = spanned->getSpanStart(spans[i]);
            int spanLine = getLineForOffset(spStart);
            int count = ((LeadingMarginSpan2*) spans[i])->getLeadingMarginLineCount();
            // if there is more than one LeadingMarginSpan2, use the count that is greatest
            useFirstLineMargin |= line < spanLine + count;
        }
    }
    for (int i = 0; i < spans.size(); i++) {
        LeadingMarginSpan* span = (LeadingMarginSpan*)spans[i];
        margin += span->getLeadingMargin(useFirstLineMargin);
    }

    return margin;
}

float TextLayout::measurePara(TextPaint& paint, CharSequence* text, int start, int end, const TextDirectionHeuristic* textDir) {
    MeasuredParagraph* mt = nullptr;
    TextLine* tl = TextLine::obtain();
    mt = MeasuredParagraph::buildForBidi(text, start, end, textDir, mt);
    auto& chars = mt->getChars();
    const int len = chars.size();
    const Directions directions = mt->getDirections(0, len);
    const int dir = mt->getParagraphDir();
    bool hasTabs = false;
    TabStops* tabStops = nullptr;
    // leading margins should be taken into account when measuring a paragraph
    int margin = 0;
    if (dynamic_cast<Spanned*>(text)) {
        Spanned* spanned = (Spanned*) text;
        auto spans = getParagraphSpans(spanned, start, end, LeadingMarginSpanFilter);
        for (auto lms : spans) {
            margin += ((LeadingMarginSpan*)lms)->getLeadingMargin(true);
        }
    }
    for (int i = 0; i < len; ++i) {
        if (chars[i] == '\t') {
            hasTabs = true;
            if (dynamic_cast<Spanned*>(text)) {
                Spanned* spanned = (Spanned*) text;
                int spanEnd = spanned->nextSpanTransition(start, end, TabStopSpanFilter);
                auto spans = getParagraphSpans(spanned, start, spanEnd,TabStopSpanFilter);
                if (spans.size() > 0) {
                    tabStops = new TabStops(TAB_INCREMENT, spans);
                }
            }
            break;
        }
    }
    tl->set(&paint, text, start, end, dir, directions, hasTabs, tabStops,
            0 /* ellipsisStart */, 0 /* ellipsisEnd */);
    auto ret =margin + std::abs(tl->metrics(nullptr));
    TextLine::recycle(tl);
    return ret;
}

////////////////public static class TabStops////////////////////

TabStops::TabStops(float increment, const std::vector<ParcelableSpan*>& spans) {
    reset(increment, spans);
}

void TabStops::reset(float increment, const std::vector<ParcelableSpan*>& spans) {
    this->mIncrement = increment;
    int ns = 0;
    if (!spans.empty()) {
        auto& stops = this->mStops;
        for (auto o : spans) {
            if (dynamic_cast<TabStopSpan*>(o)) {
                if (stops.empty()) {
                    stops.resize(10);
                } else if (ns == stops.size()) {
                    stops.resize(ns * 2);
                }
                stops[ns++] = ((TabStopSpan*) o)->getTabStop();
            }
        }
        if (ns > 1) {
            std::sort(stops.begin(),stops.begin()+ns);//Arrays.sort(stops, 0, ns);
        }
    }
    this->mNumStops = ns;
}

float TabStops::nextTab(float h) {
    int ns = this->mNumStops;
    if (ns > 0) {
        for (int i = 0; i < ns; ++i) {
            float stop = mStops[i];
            if (stop > h) {
                return stop;
            }
        }
    }
    return nextDefaultStop(h, mIncrement);
}

//////////////////////////////////////////////////////////////////////

float TextLayout::nextTab(CharSequence* text, int start, int end,
                                   float h,std::vector<ParcelableSpan*>& tabs) {
    float nh = FLT_MAX;
    bool alltabs = false;
    if (dynamic_cast<Spanned*>(text)) {
        if (tabs.empty()) {
            tabs = getParagraphSpans((Spanned*) text, start, end, TabStopSpanFilter);
            alltabs = true;
        }
        for (int i = 0; i < tabs.size(); i++) {
            if (!alltabs) {
                if (!(dynamic_cast<TabStopSpan*>(tabs[i])))
                    continue;
            }
            int where = ((TabStopSpan*) tabs[i])->getTabStop();
            if (where < nh && where > h)
                nh = where;
        }
        if (nh != FLT_MAX)
            return nh;
    }
    return ((int) ((h + TAB_INCREMENT) / TAB_INCREMENT)) * TAB_INCREMENT;
}

 bool TextLayout::isSpanned() const{
    return mSpannedText;
}

std::vector<ParcelableSpan*> TextLayout::getParagraphSpans(Spanned* text, int start, int end, const SpanFilter& type) {
    if (start == end && start > 0) {
        return {};//ArrayUtils.emptyArray(type);
    }

    if(dynamic_cast<SpannableStringBuilder*>(text)) {
        return ((SpannableStringBuilder*) text)->getSpans(start, end, type, false);
    } else {
        return text->getSpans(start, end, type);
    }
}

static std::string getEllipsisString(TextUtils::TruncateAt method){
    return (method==TextUtils::TruncateAt::END_SMALL)?"\u2025":"\u2026";
}
void TextLayout::ellipsize(int start, int end, int line, std::vector<char16_t>& dest, int destoff, TextUtils::TruncateAt method) {
    const int ellipsisCount = getEllipsisCount(line);
    if (ellipsisCount == 0) {
        return;
    }
    const int ellipsisStart = getEllipsisStart(line);
    const int lineStart = getLineStart(line);

    std::string ellipsisString = /*TextUtils.*/getEllipsisString(method);
    const int ellipsisStringLen = ellipsisString.length();
    // Use the ellipsis string only if there are that at least as many characters to replace.
    const bool useEllipsisString = ellipsisCount >= ellipsisStringLen;
    const int min = std::max(0, start - ellipsisStart - lineStart);
    const int max = std::min(ellipsisCount, end - ellipsisStart - lineStart);

    for (int i = min; i < max; i++) {
        char c;
        if (useEllipsisString && i < ellipsisStringLen) {
            c = ellipsisString[i];//.charAt(i);
        } else {
            //c = TextUtils.ELLIPSIS_FILLER;
        }
        const int a = i + ellipsisStart + lineStart;
        dest[destoff + a - start] = c;
    }
}

/*class Directions {
    public int[] mDirections;

    public Directions(int[] dirs) {
        mDirections = dirs;
    }

    public int getRunCount() {
        return mDirections.length / 2;
    }

    public int getRunStart(int runIndex) {
        return mDirections[runIndex * 2];
    }

    public int getRunLength(int runIndex) {
        return mDirections[runIndex * 2 + 1] & RUN_LENGTH_MASK;
    }

    public bool isRunRtl(int runIndex) {
        return (mDirections[runIndex * 2 + 1] & RUN_RTL_FLAG) != 0;
    }
};*/

//static class Ellipsizer implements CharSequence, GetChars
int TextLayout::Ellipsizer::charAt(int off) const {
    std::vector<char16_t> buf(8) ;//= TextUtils.obtain(1);
    getChars(off, off + 1, buf, 0);
    char ret = buf[0];
    //TextUtils.recycle(buf);
    return ret;
}

void TextLayout::Ellipsizer::getChars(int start, int end, std::vector<char16_t>& dest, int destoff) const {
    const int line1 = mLayout->getLineForOffset(start);
    const int line2 = mLayout->getLineForOffset(end);
    //TextUtils.getChars(mText, start, end, dest, destoff);
    for (int i = line1; i <= line2; i++) {
        mLayout->ellipsize(start, end, i, dest, destoff, mMethod);
    }
}

CharSequence* TextLayout::Ellipsizer::subSequence(int start, int end) {
    std::vector<char16_t> s (end - start);
    getChars(start, end, s, 0);
    return new SpannedString("");//s.data());
}

std::string TextLayout::Ellipsizer::toString() const{
    //char[] s = new char[length()];
    //getChars(0, length(), s, 0);
    return "";//new String(s);
}

std::wstring TextLayout::Ellipsizer::toWString() const{
    return "";//new String(s);
}
}/*endof namespace*/
