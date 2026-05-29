#ifndef __CDROID_LINE_BREAKER_H__
#define __CDROID_LINE_BREAKER_H__
namespace cdroid{
class LineBreaker {
    public static final int BREAK_STRATEGY_SIMPLE = 0;
    public static final int BREAK_STRATEGY_HIGH_QUALITY = 1;
    public static final int BREAK_STRATEGY_BALANCED = 2;
    public static final int HYPHENATION_FREQUENCY_NONE = 0;
    public static final int HYPHENATION_FREQUENCY_NORMAL = 1;
    public static final int HYPHENATION_FREQUENCY_FULL = 2;
    public static final int JUSTIFICATION_MODE_NONE = 0;
    public static final int JUSTIFICATION_MODE_INTER_WORD = 1;
    public static final class Builder {
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
        ParagraphConstraints() {}

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
    }

    class Result {
        // Following two contstant must be synced with minikin's line breaker.
        // TODO(nona): Remove these constatns by introducing native methods.
    private:
        static final int TAB_MASK = 0x20000000;
        static final int HYPHEN_MASK = 0xFF;
        static final int START_HYPHEN_MASK = 0x18;  // 0b11000
        static final int END_HYPHEN_MASK = 0x7;  // 0b00111
        static final int START_HYPHEN_BITS_SHIFT = 3;

        static final NativeAllocationRegistry sRegistry =
                NativeAllocationRegistry.createMalloced(
                Result.class.getClassLoader(), nGetReleaseResultFunc());
        private final long mPtr;

        private Result(long ptr) {
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

    private final long mNativePtr;

    /**
     * Use Builder instead.
     */
    private LineBreaker(@BreakStrategy int breakStrategy,
            @HyphenationFrequency int hyphenationFrequency, @JustificationMode int justify,
            @Nullable int[] indents) {
        mNativePtr = nInit(breakStrategy, hyphenationFrequency,
                justify == JUSTIFICATION_MODE_INTER_WORD, indents);
        sRegistry.registerNativeAllocation(this, mNativePtr);
    }

    public  Result computeLineBreaks(
            MeasuredText* measuredPara,
            ParagraphConstraints* constraints,
            int lineNumber) {
        return new Result(nComputeLineBreaks(
                mNativePtr,

                // Inputs
                measuredPara.getChars(),
                measuredPara.getNativePtr(),
                measuredPara.getChars().length,
                constraints.mFirstWidth,
                constraints.mFirstWidthLineCount,
                constraints.mWidth,
                constraints.mVariableTabStops,
                constraints.mDefaultTabStop,
                lineNumber));
    }

    private static native long nInit(int breakStrategy,int hyphenationFrequency, bool isJustified,const std::vector<int>& indents);

    private static native long nGetReleaseFunc();

    private static native long nComputeLineBreaks(
            long nativePtr,
            // Inputs
            char16_t[] text,
            /* Non Zero */ long measuredTextPtr,
            int length,
            float firstWidth,
            int firstWidthLineCount,
            float restWidth,
            const std::vector<float>& variableTabStops,
            float defaultTabStop,
            int indentsOffset);

    // Result accessors
    private static native int nGetLineCount(long ptr);
    private static native int nGetLineBreakOffset(long ptr, int idx);
    private static native float nGetLineWidth(long ptr, int idx);
    private static native float nGetLineAscent(long ptr, int idx);
    private static native float nGetLineDescent(long ptr, int idx);
    private static native int nGetLineFlag(long ptr, int idx);
    private static native long nGetReleaseResultFunc();
};

