#ifndef __CDROID_LINE_BREAKER_H__
#define __CDROID_LINE_BREAKER_H__
#include <vector>
#include <text/measuredtext.h>
#include <text/linebreakconfig.h>
namespace minikin{
    class LineBreakResult;
    class StaticLayoutNative;
}

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
    };
public:
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

        void setTabStops(const std::vector<float>& tabStops,float defaultTabStop) {
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
        friend LineBreaker;
        static constexpr int TAB_MASK = 0x20000000;
        static constexpr int HYPHEN_MASK = 0xFF;
        static constexpr int START_HYPHEN_MASK = 0x18;  // 0b11000
        static constexpr int END_HYPHEN_MASK = 0x7;  // 0b00111
        static constexpr int START_HYPHEN_BITS_SHIFT = 3;
        minikin::LineBreakResult* mPtr;/*minikin's internal*/ 
        Result(minikin::LineBreakResult*p):mPtr(p){};
    public:
        int getLineCount() const;
        int getLineBreakOffset( int lineIndex)const;

        float getLineWidth(int lineIndex) const;
        float getLineAscent(int lineIndex) const;
        float getLineDescent(int lineIndex) const;
        bool hasLineTab(int lineIndex) const;
        int getStartLineHyphenEdit(int lineIndex)const;
        int getEndLineHyphenEdit(int lineIndex)const;
    };
private:
    minikin::StaticLayoutNative*mNativePtr;
public:
    LineBreaker(int breakStrategy, int hyphenationFrequency, int justify, const std::vector<int>& indents);
    Result computeLineBreaks( MeasuredText* measuredPara, const ParagraphConstraints& constraints, int lineNumber);
};
}/*endof namespace*/
#endif/*__CDROID_LINE_BREAKER_H__*/
