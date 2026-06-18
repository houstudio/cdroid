#ifndef __MEASURED_PARAGRAPH_H__
#define __MEASURED_PARAGRAPH_H__
#include <core/pools.h>
#include <core/rect.h>
#include <text/spannablestring.h>
#include <text/textpaint.h>
#include <text/measuredtext.h>
#include <text/textdirectionheuristics.h>

namespace cdroid{
class Directions;
class MeasuredParagraph {
private:
    static constexpr char16_t OBJECT_REPLACEMENT_CHARACTER = 0xFFFC;

    const Spanned* mSpanned;
    int mTextStart;
    int mTextLength;
    std::vector<char16_t> mCopiedBuffer;
    int mParaDir;
    bool mLtrWithoutBidi;
    std::vector<uint8_t> mLevels;
    float mWholeWidth;
    std::vector<float> mWidths;
    std::vector<int> mSpanEndCache;/*4*/
    std::vector<int> mFontMetrics;/*16*/
    MeasuredText* mMeasuredText;/* The native MeasuredParagraph.*/
    TextPaint mCachedPaint;
    Paint::FontMetricsInt mCachedFm;
private:
    MeasuredParagraph()=default;

    static Pools::SynchronizedPool<MeasuredParagraph> sPool;

    static MeasuredParagraph* obtain();
    void reset();
    void resetAndAnalyzeBidi(const CharSequence* text, int start, int end, const TextDirectionHeuristic* textDir);
    void applyReplacementRun(const ReplacementSpan& replacement, int start, int end, MeasuredText::Builder* builder);
    void applyStyleRun(int start, int end, MeasuredText::Builder* builder);
    void applyMetricsAffectingSpan(const TextPaint& paint,const std::vector<const ParcelableSpan*>& spans,
            int start, int end, MeasuredText::Builder* builder);
public:
    ~MeasuredParagraph(){
        delete mMeasuredText;
    }
    void recycle();
    void release();

    int getTextLength() const{
        return mTextLength;
    }

    const std::vector<char16_t>& getChars() const{
        return mCopiedBuffer;
    }

    int getParagraphDir() const{
        return mParaDir;
    }

    const Directions* getDirections( int start, int end)const;  // exclusive

    float getWholeWidth() const{
        return mWholeWidth;
    }

    const std::vector<float>& getWidths() const{
        return mWidths;
    }

    const std::vector<int>& getSpanEndCache() const{
        return mSpanEndCache;
    }

    std::vector<int> getFontMetrics() const{
        return mFontMetrics;
    }

    MeasuredText* getMeasuredText() const{
        return mMeasuredText;
    }

    float getWidth(int start, int end) const;

    void getBounds(int start, int end, Rect& bounds) const;

    float getCharWidthAt( int offset) const;

    static MeasuredParagraph* buildForBidi(CharSequence* text, int start, int end,
            const TextDirectionHeuristic* textDir, MeasuredParagraph* recycle);

    static MeasuredParagraph* buildForMeasurement(TextPaint* paint,const CharSequence* text,
            int start, int end,const TextDirectionHeuristic* textDir, MeasuredParagraph* recycle);

    static MeasuredParagraph* buildForStaticLayout(const TextPaint* paint,const CharSequence* text, int start, int end,
            const TextDirectionHeuristic* textDir, bool computeHyphenation, bool computeLayout,
            MeasuredParagraph* hint, MeasuredParagraph* recycle);

    int breakText(int limit, bool forwards, float width);
    float measure(int start, int limit)const;

    int getMemoryUsage() const;
};
}/*end of namespace*/
#endif/*__MEASURED_PARAGRAPH_H__*/
