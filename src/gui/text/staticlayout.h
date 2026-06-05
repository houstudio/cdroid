#ifndef __STATIC_LAYOUT_H__
#define __STATIC_LAYOUT_H__
#include <core/pools.h>
#include <text/layout.h>
#include <text/textutils.h>
#include <text/measuredparagraph.h>
namespace cdroid{
class StaticLayout :public TextLayout {
public:
    class Builder;
private:
    StaticLayout(Builder& b);
    StaticLayout(CharSequence* text);
    
    int out(CharSequence* text, int start, int end, int above, int below, int top, int bottom,
            int v, float spacingmult, float spacingadd, const std::vector<ParcelableSpan*>& chooseHt,
            const std::vector<int>* chooseHtv, Paint::FontMetricsInt& fm, bool hasTab, int hyphenEdit,
            bool needMultiply, MeasuredParagraph* measured, int bufEnd, bool includePad, bool trackPad,
            bool addLastLineLineSpacing, const std::vector<char32_t>& chs, int widthStart, TextUtils::TruncateAt ellipsize,
            float ellipsisWidth, float textWidth, TextPaint* paint, bool moreChars);

    void calculateEllipsis(int lineStart, int lineEnd, MeasuredParagraph* measured, int widthStart, float avail,
            TextUtils::TruncateAt where,int line, float textWidth, TextPaint* paint, bool forceEllipsis);

    float getTotalInsets(int line);
public:
    StaticLayout(CharSequence* source, TextPaint* paint, int width, Alignment align, float spacingmult, float spacingadd, bool includepad);
    StaticLayout(CharSequence* source, int bufstart, int bufend, TextPaint* paint, int outerwidth,
            Alignment align, float spacingmult, float spacingadd, bool includepad);
    StaticLayout(CharSequence* source, int bufstart, int bufend, TextPaint* paint, int outerwidth, Alignment align,float spacingmult,
            float spacingadd, bool includepad, TextUtils::TruncateAt ellipsize, int ellipsizedWidth);
    StaticLayout(CharSequence* source, int bufstart, int bufend, TextPaint* paint, int outerwidth, Alignment align, const TextDirectionHeuristic* textDir,
            float spacingmult, float spacingadd, bool includepad, TextUtils::TruncateAt ellipsize, int ellipsizedWidth, int maxLines);

    void generate(Builder& b, bool includepad, bool trackpad);
    int getLineForVertical(int vertical)const override;

    int getLineCount() const override{
        return mLineCount;
    }

    int getLineTop(int line) const override{
        return mLines[mColumns * line + TOP];
    }

    int getLineExtra(int line) const override{
        return mLines[mColumns * line + EXTRA];
    }

    int getLineDescent(int line) const override{
        return mLines[mColumns * line + DESCENT];
    }

    int getLineStart(int line) const override{
        return mLines[mColumns * line + START] & START_MASK;
    }

    int getParagraphDirection(int line) const override{
        return mLines[mColumns * line + DIR] >> DIR_SHIFT;
    }

    bool getLineContainsTab(int line) const override{
        return (mLines[mColumns * line + TAB] & TAB_MASK) != 0;
    }

    const Directions* getLineDirections(int line) const override;

    int getTopPadding() const override{
        return mTopPadding;
    }

    int getBottomPadding() const override{
        return mBottomPadding;
    }

    static int packHyphenEdit(int start, int end) {
        return start << START_HYPHEN_BITS_SHIFT | end;
    }

    static int unpackStartHyphenEdit(int packedHyphenEdit) {
        return (packedHyphenEdit & START_HYPHEN_MASK) >> START_HYPHEN_BITS_SHIFT;
    }

    static int unpackEndHyphenEdit(int packedHyphenEdit) {
        return packedHyphenEdit & END_HYPHEN_MASK;
    }

    int getStartHyphenEdit(int lineNumber)const override{
        return unpackStartHyphenEdit(mLines[mColumns * lineNumber + HYPHEN] & HYPHEN_MASK);
    }

    int getEndHyphenEdit(int lineNumber)const override{
        return unpackEndHyphenEdit(mLines[mColumns * lineNumber + HYPHEN] & HYPHEN_MASK);
    }

    int getIndentAdjust(int line, Alignment align) const override;
    int getEllipsisCount(int line) const override;

    int getEllipsisStart(int line) const override;

    int getEllipsizedWidth() const override{
        return mEllipsizedWidth;
    }

    int getHeight(bool cap)const override;
private:
    int mLineCount;
    int mTopPadding, mBottomPadding;
    int mColumns;
    int mEllipsizedWidth;

    bool mEllipsized;
    int mMaxLineHeight = DEFAULT_MAX_LINE_HEIGHT;

    static constexpr int COLUMNS_NORMAL = 5;
    static constexpr int COLUMNS_ELLIPSIZE = 7;
    static constexpr int START = 0;
    static constexpr int DIR = START;
    static constexpr int TAB = START;
    static constexpr int TOP = 1;
    static constexpr int DESCENT = 2;
    static constexpr int EXTRA = 3;
    static constexpr int HYPHEN = 4;
    static constexpr int ELLIPSIS_START = 5;
    static constexpr int ELLIPSIS_COUNT = 6;

    std::vector<int> mLines;
    std::vector<const Directions*> mLineDirections;
    int mMaximumVisibleLineCount = INT_MAX;

    static constexpr int START_MASK = 0x1FFFFFFF;
    static constexpr int DIR_SHIFT  = 30;
    static constexpr int TAB_MASK   = 0x20000000;
    static constexpr int HYPHEN_MASK = 0xFF;
    static constexpr int START_HYPHEN_BITS_SHIFT = 3;
    static constexpr int START_HYPHEN_MASK = 0x18; // 0b11000
    static constexpr int END_HYPHEN_MASK = 0x7;  // 0b00111

    static constexpr float TAB_INCREMENT = 20; // same as Layout, but that's private
    static constexpr char CHAR_NEW_LINE = '\n';
    static constexpr double EXTRA_ROUNDING = 0.5;
    static constexpr int DEFAULT_MAX_LINE_HEIGHT = -1;

    class LineBreaks {
    private:
        static constexpr int INITIAL_SIZE = 16;
    public:
        int breaks[INITIAL_SIZE];
        float widths[INITIAL_SIZE];
        float ascents[INITIAL_SIZE];
        float descents[INITIAL_SIZE];
        int flags[INITIAL_SIZE]; // hasTab
        // breaks, widths, and flags should all have the same length
    };
    std::vector<int> mLeftIndents;
    std::vector<int> mRightIndents;
};

class StaticLayout::Builder {
private:
    friend class StaticLayout;
    Builder()=default;
public:
    static Builder* obtain(CharSequence* source, int start, int end, TextPaint* paint,int width);
    static void recycle(Builder* b);
public:
    void finish();
    Builder& setText(CharSequence* source);
    Builder& setText(CharSequence* source, int start, int end);
    Builder& setPaint(TextPaint* paint);
    Builder& setWidth(int width);
    Builder& setAlignment(Alignment alignment);
    Builder& setTextDirection(const TextDirectionHeuristic* textDir);
    Builder& setLineSpacing(float spacingAdd, float spacingMult);
    Builder& setIncludePad(bool includePad);
    Builder& setUseLineSpacingFromFallbacks(bool useLineSpacingFromFallbacks);
    Builder& setEllipsizedWidth(int ellipsizedWidth);
    Builder& setEllipsize(TextUtils::TruncateAt ellipsize);
    Builder& setMaxLines(int maxLines);
    Builder& setBreakStrategy(int breakStrategy);
    Builder& setHyphenationFrequency(int hyphenationFrequency);
    Builder& setIndents(const std::vector<int>&leftIndents,const std::vector<int>& rightIndents);
    Builder& setJustificationMode(int justificationMode);
    Builder& setAddLastLineLineSpacing(bool value);
    StaticLayout* build();
private:
    CharSequence* mText;
    int mStart;
    int mEnd;
    TextPaint* mPaint;
    int mWidth;
    Alignment mAlignment;
    const TextDirectionHeuristic* mTextDir;
    float mSpacingMult;
    float mSpacingAdd;
    bool mIncludePad;
    bool mFallbackLineSpacing;
    bool mAddLastLineLineSpacing;
    int mEllipsizedWidth;
    TextUtils::TruncateAt mEllipsize;
    int mMaxLines;
    int mBreakStrategy;
    int mHyphenationFrequency;
    std::vector<int> mLeftIndents;
    std::vector<int> mRightIndents;
    int mJustificationMode;

    Paint::FontMetricsInt mFontMetricsInt;// = new Paint.FontMetricsInt();
    static Pools::SynchronizedPool<Builder> sPool;// = new SynchronizedPool<>(3);
};

}/*endof namespace*/
#endif/*__STATIC_LAYOUT_H__*/
