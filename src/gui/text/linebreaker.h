#ifndef __CDROID_LINE_BREAKER_H__
#define __CDROID_LINE_BREAKER_H__
namespace cdroid{
class LineBreaker {
public:
    enum BreakStrategy{
        BREAK_STRATEGY_SIMPLE = 0,
        BREAK_STRATEGY_HIGH_QUALITY = 1,
        BREAK_STRATEGY_BALANCED = 2
    };
    enum HyphenationFrequency{
        HYPHENATION_FREQUENCY_NONE = 0,
        HYPHENATION_FREQUENCY_NORMAL = 1,
        HYPHENATION_FREQUENCY_FULL = 2
    };
    enum JustficationMode{
        JUSTIFICATION_MODE_NONE = 0,
        JUSTIFICATION_MODE_INTER_WORD = 1
    }
public:
    class Builder {
    private:
        int mBreakStrategy = BREAK_STRATEGY_SIMPLE;
        int mHyphenationFrequency = HYPHENATION_FREQUENCY_NONE;
        int mJustificationMode = JUSTIFICATION_MODE_NONE;
        std::vector<int> mIndents;
    public:
        Builder& setBreakStrategy(int breakStrategy) {
            mBreakStrategy = breakStrategy;
            return *this;
        }

        Builder& setHyphenationFrequency(int hyphenationFrequency) {
            mHyphenationFrequency = hyphenationFrequency;
            return *this;
        }

        Builder& setJustificationMode(int justificationMode) {
            mJustificationMode = justificationMode;
            return *this;
        }

        Builder& setIndents(const std::vector<int>& indents) {
            mIndents = indents;
            return *this;
        }

        LineBreaker* build() {
            return new LineBreaker(mBreakStrategy, mHyphenationFrequency, mJustificationMode,
                    mIndents);
        }
    };

    class ParagraphConstraints {
    private:
        float mWidth = 0;
        float mFirstWidth = 0;
        int mFirstWidthLineCount = 0;
        std::vector<float> mVariableTabStops;
        float mDefaultTabStop = 0;
    public:
        ParagraphConstraints() =default;

        void setWidth(float width) {
            mWidth = width;
        }

        void setIndent(float firstWidth,int firstWidthLineCount) {
            mFirstWidth = firstWidth;
            mFirstWidthLineCount = firstWidthLineCount;
        }

        void setTabStops(const std::vector<float> tabStops,float defaultTabStop) {
            mVariableTabStops = tabStops;
            mDefaultTabStop = defaultTabStop;
        }

        float getWidth() const{
            return mWidth;
        }

        float getFirstWidth() const{
            return mFirstWidth;
        }

        int getFirstWidthLineCount() const{
            return mFirstWidthLineCount;
        }

        std::vector<float> getTabStops() const{
            return mVariableTabStops;
        }

        float getDefaultTabStop() const{
            return mDefaultTabStop;
        }
    };

    class Result {
    private:
        static constexpr int TAB_MASK = 0x20000000;
        static constexpr int HYPHEN_MASK = 0xFF;
        static constexpr int START_HYPHEN_MASK = 0x18;  // 0b11000
        static constexpr int END_HYPHEN_MASK = 0x7;  // 0b00111
        static constexpr int START_HYPHEN_BITS_SHIFT = 3;
        void* mPtr;/*minikin's internal*/ 
    public:
        Result(long ptr) {
            mPtr = ptr;
            sRegistry.registerNativeAllocation(this, mPtr);
        }

        int getLineCount() const{
            return nGetLineCount(mPtr);
        }

        int getLineBreakOffset( int lineIndex) {
            return nGetLineBreakOffset(mPtr, lineIndex);
        }

        float getLineWidth(int lineIndex) const{
            return nGetLineWidth(mPtr, lineIndex);
        }

        float getLineAscent(int lineIndex) const{
            return nGetLineAscent(mPtr, lineIndex);
        }

        float getLineDescent(int lineIndex) const{
            return nGetLineDescent(mPtr, lineIndex);
        }

        bool hasLineTab(int lineIndex) const{
            return (nGetLineFlag(mPtr, lineIndex) & TAB_MASK) != 0;
        }

        int getStartLineHyphenEdit(int lineIndex) {
            return (nGetLineFlag(mPtr, lineIndex) & START_HYPHEN_MASK) >> START_HYPHEN_BITS_SHIFT;
        }

        int getEndLineHyphenEdit(int lineIndex) {
            return nGetLineFlag(mPtr, lineIndex) & END_HYPHEN_MASK;
        }
    };
private:
    void* mNativePtr;
public:
    LineBreaker(int breakStrategy, int hyphenationFrequency, int justify, const std::vector<int>& indents);
    Result computeLineBreaks( MeasuredText* measuredPara, ParagraphConstraints* constraints, int lineNumber);
};
}/*endof namespace*/
#endif/*__CDROID_LINE_BREAKER_H__*/
