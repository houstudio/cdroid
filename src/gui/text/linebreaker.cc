#include <text/linebreaker.h>
#include <minikin/androidlinebreakerhelper.h>
namespace cdroid{

LineBreaker::LineBreaker(int breakStrategy, int hyphenationFrequency, int justify, const std::vector<int>& indents) {
    std::vector<float>fIndents;
    for(auto i:indents)fIndents.push_back(i);
    
    mNativePtr = new minikin::android::StaticLayoutNative(
            static_cast<minikin::BreakStrategy>(breakStrategy),
            static_cast<minikin::HyphenationFrequency>(hyphenationFrequency),
            justify, std::move(fIndents));
}

LineBreaker::~LineBreaker(){
    delete (minikin::android::StaticLayoutNative*)mNativePtr;
}

LineBreaker::Result LineBreaker::computeLineBreaks(MeasuredText* measuredPara,
        const ParagraphConstraints& constraints, int lineNumber) {
    const minikin::android::StaticLayoutNative* builder = (const minikin::android::StaticLayoutNative*)mNativePtr;
    auto& chars = measuredPara->getChars();
    minikin::U16StringPiece u16Text((const uint16_t*)chars.data(), chars.size());
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
LineBreaker::Result::~Result(){
    delete (minikin::LineBreakResult*)mPtr;
}
#define toResult(p) ((minikin::LineBreakResult*)(p))
int LineBreaker::Result::getLineCount() const{
    return toResult(mPtr)->breakPoints.size();
}

int LineBreaker::Result::getLineBreakOffset( int lineIndex) const{
    return toResult(mPtr)->breakPoints[lineIndex];
}

float LineBreaker::Result::getLineWidth(int lineIndex) const{
    return toResult(mPtr)->widths[lineIndex];
}

float LineBreaker::Result::getLineAscent(int lineIndex) const{
    return toResult(mPtr)->ascents[lineIndex];
}

float LineBreaker::Result::getLineDescent(int lineIndex) const{
    return toResult(mPtr)->descents[lineIndex];
}

bool LineBreaker::Result::hasLineTab(int lineIndex) const{
    return (toResult(mPtr)->flags[lineIndex] & TAB_MASK) != 0;
}

int LineBreaker::Result::getStartLineHyphenEdit(int lineIndex) const{
    return (toResult(mPtr)->flags[lineIndex] & START_HYPHEN_MASK) >> START_HYPHEN_BITS_SHIFT;
}

int LineBreaker::Result::getEndLineHyphenEdit(int lineIndex) const{
    return (toResult(mPtr)->flags[lineIndex]) & END_HYPHEN_MASK;
}

//////////////////////////////////////////////////////////////////////////////////////////
}/*endof namespace*/
