#ifndef __CDROID_LINE_BREAKER_H__
#define __CDROID_LINE_BREAKER_H__
namespace cdroid{

LineBreaker::LineBreaker(int breakStrategy, int hyphenationFrequency, int justify, const std::vector<int>& indents) {
    mNativePtr = nInit(breakStrategy, hyphenationFrequency,
            justify == JUSTIFICATION_MODE_INTER_WORD, indents);
}

Result LineBreaker::computeLineBreaks(MeasuredText* measuredPara,
        ParagraphConstraints* constraints,int lineNumber) {
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
    int nGetLineCount(long ptr);
    int nGetLineBreakOffset(long ptr, int idx);
    float nGetLineWidth(long ptr, int idx);
    float nGetLineAscent(long ptr, int idx);
    float nGetLineDescent(long ptr, int idx);
    int nGetLineFlag(long ptr, int idx);
    long nGetReleaseResultFunc();
};

