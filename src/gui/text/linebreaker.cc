#include <text/linebreaker.h>
#include <minikin/androidlinebreakerhelper.h>
namespace cdroid{

LineBreaker::LineBreaker(int breakStrategy, int hyphenationFrequency, int justify, const std::vector<int>& indents) {
    std::vector<float>fIndents;
    for(auto i:indents)fIndents.push_back(i);
    
    mNativePtr = new minikin::StaticLayoutNative(
            static_cast<minikin::BreakStrategy>(breakStrategy),
            static_cast<minikin::HyphenationFrequency>(hyphenationFrequency),
            justify, std::move(fIndents));
}

LineBreaker::Result LineBreaker::computeLineBreaks(MeasuredText* measuredPara,
        const ParagraphConstraints& constraints, int lineNumber) {
    const minikin::StaticLayoutNative* builder = mNativePtr;
    minikin::U16StringPiece u16Text;//(text.get(), length);
    minikin::MeasuredText*mt;
    minikin::LineBreakResult result=builder->computeBreaks(
                u16Text, *(minikin::MeasuredText*)measuredPara->getNativePtr(),
                constraints.getFirstWidth(),
                constraints.getFirstWidthLineCount(),
                constraints.getWidth(),0,//restWidth,indentsOffset,
                (float*)constraints.getTabStops().data(),
                constraints.getTabStops().size(),
                constraints.getDefaultTabStop()
                );
    return Result(new minikin::LineBreakResult(std::move(result)));
}


////////////////////////////////////////////////////////////////////////////////////////

int LineBreaker::Result::getLineCount() const{
    return mPtr->breakPoints.size();
}

int LineBreaker::Result::getLineBreakOffset( int lineIndex) const{
    return mPtr->breakPoints[lineIndex];
}

float LineBreaker::Result::getLineWidth(int lineIndex) const{
    return mPtr->widths[lineIndex];
}

float LineBreaker::Result::getLineAscent(int lineIndex) const{
    return mPtr->ascents[lineIndex];
}

float LineBreaker::Result::getLineDescent(int lineIndex) const{
    return mPtr->descents[lineIndex];
}

bool LineBreaker::Result::hasLineTab(int lineIndex) const{
    return (mPtr->flags[lineIndex] & TAB_MASK) != 0;
}

int LineBreaker::Result::getStartLineHyphenEdit(int lineIndex) const{
    return (mPtr->flags[lineIndex] & START_HYPHEN_MASK) >> START_HYPHEN_BITS_SHIFT;
}

int LineBreaker::Result::getEndLineHyphenEdit(int lineIndex) const{
    return (mPtr->flags[lineIndex]) & END_HYPHEN_MASK;
}

//////////////////////////////////////////////////////////////////////////////////////////
}/*endof namespace*/
