#ifndef __ANDROID_TEXT_LAYOUT_H__
#define __ANDROID_TEXT_LAYOUT_H__
#include <functional>
#include <cfloat>
#include <text/textutils.h>
#include <core/canvas.h>
#include <core/path.h>
#include <core/rect.h>
#include <text/spannablestring.h>
#include <text/spanset.h>
#include <text/textpaint.h>
#include <text/textline.h>
#include <text/linebreaker.h>
#include <text/textdirectionheuristics.h>
namespace cdroid{
class Directions;
class TabStops;
class Layout {
public:
    static constexpr int DIR_LEFT_TO_RIGHT = 1;
    static constexpr int DIR_RIGHT_TO_LEFT = -1;

    static constexpr int DIR_REQUEST_LTR = 1;
    static constexpr int DIR_REQUEST_RTL = -1;
    static constexpr int DIR_REQUEST_DEFAULT_LTR = 2;
    static constexpr int DIR_REQUEST_DEFAULT_RTL = -2;

    static constexpr int RUN_LENGTH_MASK = 0x03ffffff;
    static constexpr int RUN_LEVEL_SHIFT = 26;
    static constexpr int RUN_LEVEL_MASK = 0x3f;
    static constexpr int RUN_RTL_FLAG = 1 << RUN_LEVEL_SHIFT;

    static constexpr float TAB_INCREMENT = 20;

    static constexpr int TEXT_SELECTION_LAYOUT_RIGHT_TO_LEFT = 0;
    static constexpr int TEXT_SELECTION_LAYOUT_LEFT_TO_RIGHT = 1;
    static constexpr int BREAK_STRATEGY_SIMPLE = LineBreaker::BREAK_STRATEGY_SIMPLE;
    static constexpr int BREAK_STRATEGY_HIGH_QUALITY = LineBreaker::BREAK_STRATEGY_HIGH_QUALITY;
    static constexpr int BREAK_STRATEGY_BALANCED = LineBreaker::BREAK_STRATEGY_BALANCED;
    static constexpr int HYPHENATION_FREQUENCY_NONE = LineBreaker::HYPHENATION_FREQUENCY_NONE;
    static constexpr int HYPHENATION_FREQUENCY_NORMAL = LineBreaker::HYPHENATION_FREQUENCY_NORMAL;
    static constexpr int HYPHENATION_FREQUENCY_FULL = LineBreaker::HYPHENATION_FREQUENCY_FULL;
    static constexpr int HYPHENATION_FREQUENCY_NORMAL_FAST = LineBreaker::HYPHENATION_FREQUENCY_NORMAL_FAST;
    static constexpr int HYPHENATION_FREQUENCY_FULL_FAST = LineBreaker::HYPHENATION_FREQUENCY_FULL_FAST;

    static constexpr int JUSTIFICATION_MODE_NONE = LineBreaker::JUSTIFICATION_MODE_NONE;
    static constexpr int JUSTIFICATION_MODE_INTER_WORD = LineBreaker::JUSTIFICATION_MODE_INTER_WORD;
    static constexpr int JUSTIFICATION_MODE_INTER_CHARACTER=LineBreaker::JUSTIFICATION_MODE_INTER_CHARACTER;

    static constexpr float DEFAULT_LINESPACING_MULTIPLIER = 1.0f;
    static constexpr float DEFAULT_LINESPACING_ADDITION = 0.0f;


    using SelectionRectangleConsumer = std::function<void(float left, float top, float right, float bottom,int textSelectionLayout)>;

    enum Alignment {
        NONE=-1,
        ALIGN_NORMAL=0,
        ALIGN_OPPOSITE,
        ALIGN_CENTER,
        ALIGN_LEFT,
        ALIGN_RIGHT,
    };
    static const Directions DIRS_ALL_LEFT_TO_RIGHT;
    static const Directions DIRS_ALL_RIGHT_TO_LEFT;
private:
    bool isJustificationRequired(int lineNum)const;
    float getJustifyWidth(int lineNum)const;
    int getLineStartPos(int line, int left, int right)const;
    float getHorizontal(int offset, bool primary) const;

    float getHorizontal(int offset, bool trailing, bool clamped) const;
    float getHorizontal(int offset, bool trailing, int line, bool clamped)const;
    std::vector<float> getLineHorizontals(int line, bool clamped, bool primary);
    float getLineExtent(int line, bool full)const;
    float getLineExtent(int line, class TabStops& tabStops, bool full)const;
    int getLineVisibleEnd(int line, int start, int end)const;
    int getOffsetToLeftRightOf(int caret, bool toLeft)const;
    int getOffsetAtStartOf(int offset)const;
    void addSelection(int line, int start, int end,int top, int bottom,const SelectionRectangleConsumer& consumer);
    int getParagraphLeadingMargin(int line) const;
    static float measurePara(const TextPaint* paint, CharSequence* text, int start, int end,const TextDirectionHeuristic* textDir, bool useBoundsForWidth);
    void ellipsize(int start, int end, int line, char16_t* dest, int destoff, TextUtils::TruncateAt method);
protected:
    Layout(CharSequence* text, TextPaint* paint, int width, Alignment align, float spacingMult, float spacingAdd);
    Layout(CharSequence* text, TextPaint* paint, int width, Alignment align, const TextDirectionHeuristic* textDir, float spacingMult, float spacingAdd,
            bool includePad,bool fallbackLineSpacing,int ellipsizedWidth,TextUtils::TruncateAt ellipsize,int maxLines,int breakStrategy,
            int hyphenationFrequency,const std::vector<int>&leftIndents,const std::vector<int>&rightIndents,int justificationMode,
            const LineBreakConfig&,bool useBoundsForWidth,bool shiftDrawingOffsetForStartOverhang,const Paint::FontMetrics* minimumFontMetrics);
    bool isSpanned() const;
public:
    virtual ~Layout();
    static float getDesiredWidth(CharSequence* source, const TextPaint& paint);
    static float getDesiredWidth(CharSequence* source, int start, int end, const TextPaint& paint);
    static float getDesiredWidth(CharSequence* source, int start, int end, const TextPaint& paint, const TextDirectionHeuristic* textDir);
    static float getDesiredWidthWithLimit(CharSequence* source, int start, int end,
            const TextPaint& paint, const TextDirectionHeuristic* textDir, float upperLimit,
            bool useBoundsForWidth);

    // android-36 Layout.Builder: high-level facade that picks BoringLayout (single boring line)
    // vs StaticLayout (everything else). Implementation (build/isBoring) is in layout.cc since it
    // needs the BoringLayout/StaticLayout full definitions.
    class Builder {
    public:
        Builder(CharSequence* text, int start, int end, TextPaint* paint, int width)
            : mText(text), mStart(start), mEnd(end), mPaint(paint), mWidth(width),
              mEllipsizedWidth(width) {}

        Builder& setAlignment(Alignment alignment){ mAlignment = alignment; return *this; }
        Builder& setTextDirection(const TextDirectionHeuristic* textDir){ mTextDir = textDir; return *this; }
        Builder& setLineSpacingAmount(float amount){ mSpacingAdd = amount; return *this; }
        Builder& setLineSpacingMultiplier(float multiplier){ mSpacingMult = multiplier; return *this; }
        Builder& setFontPaddingIncluded(bool includeFontPadding){ mIncludePad = includeFontPadding; return *this; }
        Builder& setFallbackLineSpacingEnabled(bool fallbackLineSpacing){ mFallbackLineSpacing = fallbackLineSpacing; return *this; }
        Builder& setEllipsizedWidth(int ellipsizedWidth){ mEllipsizedWidth = ellipsizedWidth; return *this; }
        Builder& setEllipsize(TextUtils::TruncateAt ellipsize){ mEllipsize = ellipsize; return *this; }
        Builder& setMaxLines(int maxLines){ mMaxLines = maxLines; return *this; }
        Builder& setBreakStrategy(int breakStrategy){ mBreakStrategy = breakStrategy; return *this; }
        Builder& setHyphenationFrequency(int hyphenationFrequency){ mHyphenationFrequency = hyphenationFrequency; return *this; }
        Builder& setLeftIndents(const std::vector<int>& leftIndents){ mLeftIndents = leftIndents; return *this; }
        Builder& setRightIndents(const std::vector<int>& rightIndents){ mRightIndents = rightIndents; return *this; }
        Builder& setJustificationMode(int justificationMode){ mJustificationMode = justificationMode; return *this; }
        Builder& setLineBreakConfig(const LineBreakConfig& lineBreakConfig){ mLineBreakConfig = lineBreakConfig; return *this; }
        Builder& setUseBoundsForWidth(bool useBoundsForWidth){ mUseBoundsForWidth = useBoundsForWidth; return *this; }
        Builder& setShiftDrawingOffsetForStartOverhang(bool shift){ mShiftDrawingOffsetForStartOverhang = shift; return *this; }
        Builder& setMinimumFontMetrics(Paint::FontMetrics* minimumFontMetrics){ mMinimumFontMetrics = minimumFontMetrics; return *this; }

        Layout* build();
    private:
        CharSequence* mText;
        int mStart;
        int mEnd;
        TextPaint* mPaint;
        int mWidth;
        Alignment mAlignment = Alignment::ALIGN_NORMAL;
        float mSpacingMult = 1.0f;
        float mSpacingAdd = 0.0f;
        const TextDirectionHeuristic* mTextDir = TextDirectionHeuristics::FIRSTSTRONG_LTR;
        bool mIncludePad = true;
        bool mFallbackLineSpacing = false;
        int mEllipsizedWidth;
        TextUtils::TruncateAt mEllipsize = TextUtils::TruncateAt::NONE;
        int mMaxLines = INT_MAX;
        int mBreakStrategy = BREAK_STRATEGY_SIMPLE;
        int mHyphenationFrequency = HYPHENATION_FREQUENCY_NONE;
        std::vector<int> mLeftIndents;
        std::vector<int> mRightIndents;
        int mJustificationMode = JUSTIFICATION_MODE_NONE;
        LineBreakConfig mLineBreakConfig;
        bool mUseBoundsForWidth = false;
        bool mShiftDrawingOffsetForStartOverhang = false;
        Paint::FontMetrics* mMinimumFontMetrics = nullptr;
    };

    void replaceWith(CharSequence* text, TextPaint* paint,int width, Alignment align, float spacingmult, float spacingadd);

    void draw(Canvas& c);
    virtual void draw(Canvas& canvas, Path* highlight,const Paint* highlightPaint, int cursorOffsetVertical);
    void drawText(Canvas& canvas, int firstLine, int lastLine);

    void drawBackground(Canvas& canvas, Path* highlight,const Paint* highlightPaint, int cursorOffsetVertical, int firstLine, int lastLine);

    int64_t getLineRangeForDraw(Canvas& canvas) const;

    CharSequence* getText() const{
        return mText;
    }

    const TextPaint* getPaint() const{
        return mPaint;
    }
    int getWidth() const{
        return mWidth;
    }

    virtual int getEllipsizedWidth() const{
        return mEllipsizedWidth;
    }
    bool getUseBoundsForWidth() const{
        return mUseBoundsForWidth;
    }
    bool getShiftDrawingOffsetForStartOverhang() const{
        return mShiftDrawingOffsetForStartOverhang;
    }
    TextUtils::TruncateAt getEllipsize()const{
        return mEllipsize;
    }
    int getMaxLines() const{
        return mMaxLines;
    }
    int getBreakStrategy() const{
        return mBreakStrategy;
    }
    int getHyphenationFrequency() const{
        return mHyphenationFrequency;
    }
    int getJustificationMode() const{
        return mJustificationMode;
    }
    void increaseWidthTo(int wid);

    virtual int getHeight() const{
        return getLineTop(getLineCount());
    }

    virtual int getHeight(bool cap) const{
        return getHeight();
    }
    virtual RectF computeDrawingBoundingBox()const;
    Alignment getAlignment() const{
        return mAlignment;
    }

    float getSpacingMultiplier() const{
        return mSpacingMult;
    }

    float getSpacingAdd() const{
        return getLineSpacingAmount();
    }
    float getLineSpacingAmount() const{
        return mSpacingAdd;
    }
    bool isFontPaddingIncluded() const{
        return mIncludePad;
    }
    virtual bool isFallbackLineSpacingEnabled() const{
        return mFallbackLineSpacing;
    }
    const TextDirectionHeuristic* getTextDirectionHeuristic() const{
        return mTextDir;
    }

    virtual int getLineCount()const=0;

    int getLineBounds(int line, Rect* bounds)const;

    virtual int getLineTop(int line)const=0;
    virtual int getLineDescent(int line)const=0;
    virtual int getLineStart(int line)const=0;
    virtual int getParagraphDirection(int line)const=0;
    virtual bool getLineContainsTab(int line)const=0;
    virtual const Directions* getLineDirections(int line)const=0;
    virtual int getTopPadding()const=0;
    virtual int getBottomPadding()const=0;

    virtual int getStartHyphenEdit(int line) const{
        return Paint::START_HYPHEN_EDIT_NO_EDIT;
    }

    virtual int getEndHyphenEdit(int line) const{
        return Paint::END_HYPHEN_EDIT_NO_EDIT;
    }

    virtual int getIndentAdjust(int line, Alignment alignment) const{
        return 0;
    }

    bool isLevelBoundary(int offset) const;

    bool isRtlCharAt(int offset) const;

    int64_t getRunRange(int offset) const;

    bool primaryIsTrailingPrevious(int offset) const;

    std::vector<bool> primaryIsTrailingPreviousAllLineOffsets(int line) const;

    float getPrimaryHorizontal(int offset) const{
        return getPrimaryHorizontal(offset, false /* not clamped */);
    }

    float getPrimaryHorizontal(int offset, bool clamped) const{
        const bool trailing = primaryIsTrailingPrevious(offset);
        return getHorizontal(offset, trailing, clamped);
    }

    float getSecondaryHorizontal(int offset) const{
        return getSecondaryHorizontal(offset, false /* not clamped */);
    }

    float getSecondaryHorizontal(int offset, bool clamped) const{
        const bool trailing = primaryIsTrailingPrevious(offset);
        return getHorizontal(offset, !trailing, clamped);
    }

    float getLineLeft(int line) const;
    float getLineRight(int line) const;
    virtual float getLineMax(int line) const;
    virtual float getLineWidth(int line) const;

    virtual int getLineForVertical(int vertical) const;
    int getLineForOffset(int offset) const;

    int getOffsetForHorizontal(int line, float horiz) const{
        return getOffsetForHorizontal(line, horiz, true);
    }

    int getOffsetForHorizontal(int line, float horiz, bool primary) const;

    /*private */class HorizontalMeasurementProvider {
        Layout* mLayout;
        int mLine;
        int mLineStartOffset;
        bool mPrimary;
        std::vector<float> mHorizontals;
    public:
        HorizontalMeasurementProvider(Layout* layout, int line, bool primary);
        void init();
        float get(int offset)const;
    };

    int getLineEnd(int line) const{
        return getLineStart(line + 1);
    }

    int getLineVisibleEnd(int line) const{
        return getLineVisibleEnd(line, getLineStart(line), getLineStart(line+1));
    }

    int getLineBottom(int line) const{
        return getLineTop(line + 1);
    }

    int getLineBottomWithoutSpacing(int line) const{
        return getLineTop(line + 1) - getLineExtra(line);
    }

    int getLineBaseline(int line) const{
        // getLineTop(line+1) == getLineBottom(line)
        return getLineTop(line+1) - getLineDescent(line);
    }

    int getLineAscent(int line) const{
        // getLineTop(line+1) - getLineDescent(line) == getLineBaseLine(line)
        return getLineTop(line) - (getLineTop(line+1) - getLineDescent(line));
    }

    virtual int getLineExtra(int line) const{
        return 0;
    }

    int getOffsetToLeftOf(int offset) const{
        return getOffsetToLeftRightOf(offset, true);
    }

    int getOffsetToRightOf(int offset) const{
        return getOffsetToLeftRightOf(offset, false);
    }

    bool shouldClampCursor(int line);

    void getCursorPath(int point, Path& dest, CharSequence* editingBuffer);

    void getSelectionPath(int start, int end, Path& dest);

    void getSelection(int start, int end,const SelectionRectangleConsumer& consumer);
    Alignment getParagraphAlignment(int line) const;
    int getParagraphLeft(int line) const;
    int getParagraphRight(int line) const;

    static float nextTab(CharSequence* text, int start, int end, float h, std::vector<const ParcelableSpan*>& tabs);

    static std::vector<const ParcelableSpan*> getParagraphSpans(Spanned* text, int start, int end, const SpanFilter&type);

    virtual int getEllipsisStart(int line)const=0;
    virtual int getEllipsisCount(int line)const=0;

    class Ellipsizer : virtual public CharSequence{//, public GetChars {
    public:
        CharSequence* mText;
        Layout* mLayout = nullptr;
        int mWidth = 0;
        TextUtils::TruncateAt mMethod = TextUtils::TruncateAt::NONE;
    public:
        Ellipsizer(CharSequence* s) {
            mText = s;
        }
        int charAt(int off) const override;
        void getChars(int start, int end, char16_t* dest, int destoff) const override;
        size_t length() const override{
            return mText->length();
        }
        CharSequence* subSequence(int start, int end)const override;
        std::string toString()const override;
        std::u16string toU16String() const override;
    };

    class SpannedEllipsizer : public Ellipsizer, public Spanned {
    private:
        Spanned* mSpanned;
    public:
        SpannedEllipsizer(Spanned* display) : Ellipsizer(display), mSpanned(display) {
        }
        std::vector<const ParcelableSpan*> getSpans(int start, int end, const SpanFilter& type) const override {
            return mSpanned->getSpans(start, end, type);
        }
        int getSpanStart(const ParcelableSpan* tag) const override {
            return mSpanned->getSpanStart(tag);
        }
        int getSpanEnd(const ParcelableSpan* tag) const override {
            return mSpanned->getSpanEnd(tag);
        }
        int getSpanFlags(const ParcelableSpan* tag) const override {
            return mSpanned->getSpanFlags(tag);
        }
        int nextSpanTransition(int start, int limit, const SpanFilter& type) const override {
            return mSpanned->nextSpanTransition(start, limit, type);
        }
        CharSequence* subSequence(int start, int end) const override;
   };
private:
    CharSequence* mText;
    mutable TextPaint mWorkPaint;
    int mWidth;
    int mEllipsizedWidth;
    Alignment mAlignment = Alignment::ALIGN_NORMAL;
    float mSpacingMult;
    float mSpacingAdd; 
    bool mSpannedText;
    bool mIncludePad;
    bool mFallbackLineSpacing;
    bool mUseBoundsForWidth;
    bool mShiftDrawingOffsetForStartOverhang;
    TextUtils::TruncateAt mEllipsize;
    int mMaxLines;
    int mBreakStrategy;
    int mHyphenationFrequency;
    int mJustificationMode;

    std::vector<int>mLeftIndents;
    std::vector<int>mRightIndents;
    LineBreakConfig mLineBreakConfig;
    const TextDirectionHeuristic* mTextDir;
    const Paint::FontMetrics* mMinimumFontMetrics;
    cdroid::SpanSet* mLineBackgroundSpans;
protected:
    TextPaint* mPaint;
};

class Directions {
public:
    std::vector<int> mDirections;
    Directions(const std::vector<int>& dirs) {
        mDirections = dirs;
    }
    int getRunCount() const{
        return mDirections.size() / 2;
    }
    int getRunStart(int runIndex) const{
        return mDirections[runIndex * 2];
    }
    int getRunLength(int runIndex) const{
        return mDirections[runIndex * 2 + 1] & Layout::RUN_LENGTH_MASK;
    }
    int getRunLevel(int runIndex) const{
        return (mDirections[runIndex * 2 + 1] >> Layout::RUN_LEVEL_SHIFT) & Layout::RUN_LEVEL_MASK;
    }
    bool isRunRtl(int runIndex) const{
        return (mDirections[runIndex * 2 + 1] & Layout::RUN_RTL_FLAG) != 0;
    }
};

class TabStops {
    std::vector<float> mStops;
    int mNumStops;
    float mIncrement;
public:
    TabStops(float increment, const std::vector<const ParcelableSpan*>& spans);
    void reset(float increment, const std::vector<const ParcelableSpan*>& spans);
    float nextTab(float h)const;
    static float nextDefaultStop(float h, float inc) {
        return ((int) ((h + inc) / inc)) * inc;
    }
};
}/*endof namespace*/
#endif/*__ANDROID_TEXT_LAYOUT_H__*/
