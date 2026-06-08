#ifndef __MEASUREDTEXT_H__
#define __MEASUREDTEXT_H__
#include <core/rect.h>
#include <text/textpaint.h>
#include <text/linebreakconfig.h>
namespace cdroid{

class MeasuredText {
private:
    void* mNativePtr;
    bool mComputeHyphenation;
    bool mComputeLayout;
    bool mComputeBounds;
    std::vector<char16_t> mChars;
    int mTop;
    int mBottom;
private:
    // Use builder instead.
    MeasuredText(void* ptr,const std::vector<char16_t>& chars, bool computeHyphenation,
            bool computeLayout, bool computeBounds, int top, int bottom);
    void rangeCheck(int start, int end)const;
    void throwRangeError(int start, int end)const;
    void offsetCheck(int offset)const;
    void throwOffsetError(int offset)const;
public:
    ~MeasuredText();
    const std::vector<char16_t>& getChars() const{
        return mChars;
    }

    float getWidth(int start, int end)const;
    int getMemoryUsage()const;

    void getBounds(int start, int end, Rect& rect)const;
    void getFontMetricsInt(int start, int end, Paint::FontMetricsInt& outMetrics)const;

    float getCharWidthAt(int offset)const;
    void* getNativePtr() const{
        return mNativePtr;
    }

    /*private static native float nGetWidth(long nativePtr, int start, int end);
    private static native long nGetReleaseFunc();
    private static native int nGetMemoryUsage( long nativePtr);
    private static native void nGetBounds(long nativePtr, char[] buf, int start, int end, Rect& rect);
    private static native float nGetCharWidthAt(long nativePtr, int offset);
    private static native long nGetExtent(long nativePtr, char[] buf, int start, int end);*/
    class Builder {
    private:
        void* mNativePtr;
        std::vector<char16_t> mText;
        bool mComputeHyphenation = false;
        bool mComputeLayout = true;
        bool mComputeBounds = true;
        bool mFastHyphenation = false;
        int mCurrentOffset = 0;
        const MeasuredText* mHintMt = nullptr;
        int mTop = 0;
        int mBottom = 0;
        Paint::FontMetricsInt mCachedMetrics;
    public:
        static constexpr int HYPHENATION_MODE_NONE = 0;
        static constexpr int HYPHENATION_MODE_NORMAL = 1;
        static constexpr int HYPHENATION_MODE_FAST = 2;
    public:
        Builder(const std::vector<char16_t>& text);
        Builder(const MeasuredText* text);
        ~Builder();

        Builder& appendStyleRun(Paint& paint, int length, bool isRtl) {
            return appendStyleRun(paint, nullptr, length, isRtl);
        }
        Builder& appendStyleRun(Paint& paint, LineBreakConfig* lineBreakConfig, int length, bool isRtl);
        Builder& appendReplacementRun(Paint& paint,int length, float width);

        Builder& setComputeHyphenation(bool computeHyphenation) {
            setComputeHyphenation( computeHyphenation ? HYPHENATION_MODE_NORMAL : HYPHENATION_MODE_NONE);
            return *this;
        }

        Builder& setComputeHyphenation(int mode) ;
        Builder& setComputeLayout(bool computeLayout) {
            mComputeLayout = computeLayout;
            return *this;
        }

        Builder& setComputeBounds(bool computeBounds) {
            mComputeBounds = computeBounds;
            return *this;
        }
        MeasuredText* build();
    };/*endof Builder*/
};
}/*endof namespace*/
#endif/* __MEASUREDTEXT_H__ */
