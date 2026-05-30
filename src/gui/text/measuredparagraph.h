#ifndef __MEASURED_PARAGRAPH_H__
#define __MEASURED_PARAGRAPH_H__
#include <core/pools.h>
#include <core/rect.h>
#include <core/spannablestring.h>
#include <text/textpaint.h>
#include <text/textdirectionheuristics.h>
namespace minikin{
    class MeasuredText;
    class MeasuredTextBuilder;
}
namespace cdroid{

class MeasuredParagraph {
private:
    static constexpr char16_t OBJECT_REPLACEMENT_CHARACTER = 0xFFFC;

    Spanned* mSpanned;
    int mTextStart;
    int mTextLength;
    std::vector<char16_t> mCopiedBuffer;
    int mParaDir;
    bool mLtrWithoutBidi;
    std::vector<uint8_t> mLevels;
    float mWholeWidth;
    std::vector<float> mWidths;
    std::array<int,4> mSpanEndCache;
    std::array<int,4*4> mFontMetrics;
    minikin::MeasuredText* mMeasuredText;/* The native MeasuredParagraph.*/
    TextPaint mCachedPaint;
    Paint::FontMetricsInt mCachedFm;
private:
    MeasuredParagraph()=default;

    static Pools::SynchronizedPool<MeasuredParagraph> sPool;

    static MeasuredParagraph* obtain();
    void reset();
    void resetAndAnalyzeBidi(CharSequence* text, int start, int end, const TextDirectionHeuristic* textDir);
    void applyReplacementRun(ReplacementSpan& replacement, int start, int end, minikin::MeasuredTextBuilder* builder);
    void applyStyleRun(int start, int end, minikin::MeasuredTextBuilder* builder);
    void applyMetricsAffectingSpan( TextPaint& paint, MetricAffectingSpan* spans,
            int start, int end, minikin::MeasuredTextBuilder* builder);
public:
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

    Directions getDirections( int start, int end);  // exclusive

    float getWholeWidth() const{
        return mWholeWidth;
    }

    const std::vector<float>& getWidths() const{
        return mWidths;
    }

    const std::array<int,4>& getSpanEndCache() const{
        return mSpanEndCache;
    }

    std::array<int,16> getFontMetrics() const{
        return mFontMetrics;
    }

    minikin::MeasuredText* getMeasuredText() const{
        return mMeasuredText;
    }

    float getWidth(int start, int end) const;

    void getBounds(int start, int end, Rect& bounds) const;

    float getCharWidthAt( int offset) const;

    static MeasuredParagraph* buildForBidi(CharSequence* text, int start, int end,
            const TextDirectionHeuristic* textDir, MeasuredParagraph* recycle);

    static MeasuredParagraph* buildForMeasurement(TextPaint* paint, CharSequence* text,
            int start, int end,const TextDirectionHeuristic* textDir, MeasuredParagraph* recycle);

    static MeasuredParagraph* buildForStaticLayout( TextPaint* paint, CharSequence* text, int start, int end,
            const TextDirectionHeuristic* textDir, bool computeHyphenation, bool computeLayout,
            MeasuredParagraph* hint, MeasuredParagraph* recycle);

    int breakText(int limit, bool forwards, float width);
    float measure(int start, int limit)const;

    int getMemoryUsage() const;
};
}/*end of namespace*/
#endif/*__MEASURED_PARAGRAPH_H__*/
