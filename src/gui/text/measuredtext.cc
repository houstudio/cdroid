#include <text/measuredtext.h>
#include <porting/cdlog.h>
#include <core/typeface.h>
#include <minikin/MeasuredText.h>
namespace cdroid{
// Use builder instead.
MeasuredText::MeasuredText(void* ptr,const std::vector<char16_t>& chars, bool computeHyphenation,
        bool computeLayout, bool computeBounds, int top, int bottom) {
    mNativePtr = ptr;
    mChars = chars;
    mComputeHyphenation = computeHyphenation;
    mComputeLayout = computeLayout;
    mComputeBounds = computeBounds;
    mTop = top;
    mBottom = bottom;
}

MeasuredText::~MeasuredText(){
    delete (minikin::MeasuredText*)mNativePtr;
}

void MeasuredText::rangeCheck(int start, int end) const{
    if (start < 0 || start > end || end > mChars.size()) {
        throwRangeError(start, end);
    }
}

void MeasuredText::throwRangeError(int start, int end) const{
    //throw new IllegalArgumentException(String.format(Locale.US,
    //    "start(%d) end(%d) length(%d) out of bounds", start, end, mChars.length));
}

void MeasuredText::offsetCheck(int offset) const{
    if (offset < 0 || offset >= mChars.size()) {
        throwOffsetError(offset);
    }
}

void MeasuredText::throwOffsetError(int offset) const{
    //throw new IllegalArgumentException(String.format(Locale.US,
    //"offset (%d) length(%d) out of bounds", offset, mChars.length));
}

float MeasuredText::getWidth(int start, int end) const{
    rangeCheck(start, end);
    //return nGetWidth(mNativePtr, start, end);
    auto ws=((minikin::MeasuredText*)mNativePtr)->widths;
    float width =0;
    for(auto w:ws)width+=w;
    return width;
}

int MeasuredText::getMemoryUsage() const{
    //nGetMemoryUsage(mNativePtr);
    return ((minikin::MeasuredText*)mNativePtr)->getMemoryUsage();
}

void MeasuredText::getBounds(int start, int end, Rect& rect) const{
    rangeCheck(start, end);
    //Preconditions.checkNotNull(rect);
    //nGetBounds(mNativePtr, mChars, start, end, rect);
    minikin::MinikinRect rc;
    const minikin::U16StringPiece usp((const uint16_t*)mChars.data(),mChars.size());
    const minikin::Range range((uint32_t)start,(uint32_t)end);
    ((minikin::MeasuredText*)mNativePtr)->getBounds(usp, range);
}

void MeasuredText::getFontMetricsInt(int start, int end, Paint::FontMetricsInt& outMetrics) const{
    rangeCheck(start, end);
    //Objects.requireNonNull(outMetrics);

    //long packed = nGetExtent(mNativePtr, mChars, start, end);
    const minikin::U16StringPiece usp((const uint16_t*)mChars.data(),mChars.size());
    const minikin::Range range((uint32_t)start,(uint32_t)end);
    const minikin::LayoutPieces lp;
    const auto ext=((minikin::MeasuredText*)mNativePtr)->getExtent(usp, range);
    outMetrics.ascent = ext.ascent;//(int) (packed >> 32);
    outMetrics.descent = ext.descent;//(int) (packed & 0xFFFFFFFF);
    outMetrics.top = std::min(outMetrics.ascent, mTop);
    outMetrics.bottom = std::max(outMetrics.descent, mBottom);
}

float MeasuredText::getCharWidthAt(int offset) const{
    offsetCheck(offset);
    //nGetCharWidthAt(mNativePtr, offset);
    return ((minikin::MeasuredText*)mNativePtr)->widths[offset];
}

////////////////////////////////////////////////////////////////////////////////////////////
//public static final class Builder {
MeasuredText::Builder::Builder(const std::vector<char16_t>& text) {
    //Preconditions.checkNotNull(text);
    mText = text;
    mNativePtr = new minikin::MeasuredTextBuilder();
}

MeasuredText::Builder::~Builder(){
    delete (minikin::MeasuredTextBuilder*)mNativePtr;
}

MeasuredText::Builder::Builder(const MeasuredText* text) {
    //Preconditions.checkNotNull(text);
    mText = text->mChars;
    mNativePtr = new minikin::MeasuredTextBuilder();
    if (!text->mComputeLayout) {
        //LOGE("The input MeasuredText must not be created with setComputeLayout(false).");
    }
    mComputeHyphenation = text->mComputeHyphenation;
    mComputeLayout = text->mComputeLayout;
    mHintMt = text;
}

MeasuredText::Builder& MeasuredText::Builder::appendStyleRun(Paint& paint, LineBreakConfig* lineBreakConfig, int length, bool isRtl) {
    //Preconditions.checkNotNull(paint);
    //Preconditions.checkArgument(length > 0, "length can not be negative");
    const int end = mCurrentOffset + length;
    //Preconditions.checkArgument(end <= mText.length, "Style exceeds the text length");
    const int lbStyle = LineBreakConfig::getResolvedLineBreakStyle(lineBreakConfig);
    const int lbWordStyle = LineBreakConfig::getResolvedLineBreakWordStyle(lineBreakConfig);
    const bool hyphenation = LineBreakConfig::getResolvedHyphenation(lineBreakConfig)
            == LineBreakConfig::HYPHENATION_ENABLED;

    minikin::MinikinPaint minikinPaint = *paint.getMinikinPaint();
    minikin::MeasuredTextBuilder*builder=(minikin::MeasuredTextBuilder*)mNativePtr;
    builder->addStyleRun(mCurrentOffset, mCurrentOffset+length, std::move(minikinPaint), isRtl);
    mCurrentOffset = end;
    paint.getFontMetricsInt(mCachedMetrics);
    mTop = std::min(mTop, mCachedMetrics.top);
    mBottom = std::max(mBottom, mCachedMetrics.bottom);
    return *this;
}

MeasuredText::Builder& MeasuredText::Builder::appendReplacementRun(Paint& paint,int length, float width) {
    //Preconditions.checkArgument(length > 0, "length can not be negative");
    const int end = mCurrentOffset + length;
    //Preconditions.checkArgument(end <= mText.length, "Replacement exceeds the text length");
    //nAddReplacementRun(mNativePtr, paint, mCurrentOffset, end, width);
    //addReplacementRun(int32_t start, int32_t end, float width, uint32_t localeListId
    ((minikin::MeasuredTextBuilder*)mNativePtr)->addReplacementRun(mCurrentOffset,end,width,0);
    mCurrentOffset = end;
    LOGD("");
    return *this;
}

/*MeasuredText::Builder& MeasuredText::Builder::setComputeHyphenation(bool computeHyphenation) {
    setComputeHyphenation(computeHyphenation ? HYPHENATION_MODE_NORMAL : HYPHENATION_MODE_NONE);
    return *this;
}*/

MeasuredText::Builder& MeasuredText::Builder::setComputeHyphenation(int mode) {
    switch (mode) {
        case HYPHENATION_MODE_NONE:
            mComputeHyphenation = false;
            mFastHyphenation = false;
            break;
        case HYPHENATION_MODE_NORMAL:
            mComputeHyphenation = true;
            mFastHyphenation = false;
            break;
        case HYPHENATION_MODE_FAST:
            mComputeHyphenation = true;
            mFastHyphenation = true;
            break;
        default:
            //LOGE("Unknown hyphenation mode:%d" ,mode);
            mComputeHyphenation = false;
            mFastHyphenation = false;
            break;
    }
    return *this;
}

/*MeasuredText::Builder& MeasuredText::Builder::setComputeLayout(bool computeLayout) {
    mComputeLayout = computeLayout;
    return *this;
}

MeasuredText::Builder& MeasuredText::Builder::setComputeBounds(bool computeBounds) {
    mComputeBounds = computeBounds;
    return *this;
}*/

MeasuredText* MeasuredText::Builder::build() {
    //ensureNativePtrNoReuse();
    if (mCurrentOffset != mText.size()) {
        //LOGE("Style info has not been provided for all text.");
    }
    if (mHintMt != nullptr && mHintMt->mComputeHyphenation != mComputeHyphenation) {
        //LOGE("The hyphenation configuration is different from given hint MeasuredText");
    }
    minikin::MeasuredText* hintPtr = (mHintMt == nullptr) ? nullptr : (minikin::MeasuredText*)mHintMt->getNativePtr();
    //long ptr = nBuildMeasuredText(mNativePtr, hintPtr, mText, mComputeHyphenation, mComputeLayout, mComputeBounds, mFastHyphenation);
    const minikin::U16StringPiece textBuffer((const uint16_t*)mText.data(), mText.size());
    // Pass the ownership to Java.
    auto ptr= ((minikin::MeasuredTextBuilder*)mNativePtr)
        ->build(textBuffer, mComputeHyphenation, mComputeLayout, hintPtr).release();
    return new MeasuredText(ptr, mText, mComputeHyphenation, mComputeLayout, mComputeBounds, mTop, mBottom);
}

}
