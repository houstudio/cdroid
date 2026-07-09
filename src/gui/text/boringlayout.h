#ifndef __BORING_LAYOUT_H__
#define __BORING_LAYOUT_H__
#include <text/layout.h>
#include <text/textpaint.h>
#include <string>
namespace cdroid{
class BoringLayout :public Layout {//TextUtils.EllipsizeCallback {
private:
    std::string/*String*/ mDirect;
    Paint mPaint;
    bool mUseFallbackLineSpacing;

    int mBottom, mDesc;// for Direct
    int mTopPadding, mBottomPadding;
    int mEllipsizedWidth, mEllipsizedStart, mEllipsizedCount;
    float mMax;
    RectF mDrawingBounds;

    static bool hasAnyInterestingChars(CharSequence* text, int textLength);
public:
    class Metrics :public Paint::FontMetricsInt {
    friend BoringLayout;
    private:
        RectF mDrawingBounds;
        void reset();
    public:
        int width;
        RectF getDrawingBoundingBox() const;
    };
public:
    static BoringLayout* make(CharSequence* source, TextPaint* paint, int outerWidth, Alignment align,
            float spacingMult, float spacingAdd, const Metrics& metrics, bool includePad);

    static BoringLayout* make(CharSequence* source, TextPaint* paint, int outerWidth, Alignment align,
            float spacingMult, float spacingAdd, const Metrics& metrics,
            bool includePad, TextUtils::TruncateAt ellipsize, int ellipsizedWidth);

    BoringLayout* replaceOrMake(CharSequence* source, TextPaint* paint, int outerwidth, Alignment align,
            float spacingMult, float spacingAdd, const Metrics& metrics, bool includePad);

    BoringLayout* replaceOrMake(CharSequence* source, TextPaint* paint, int outerWidth, Alignment align,
            float spacingMult, float spacingAdd, const Metrics& metrics,
            bool includePad, TextUtils::TruncateAt ellipsize, int ellipsizedWidth);

    BoringLayout(CharSequence* source, TextPaint* paint, int outerwidth, Alignment align,
            float spacingMult, float spacingAdd, const Metrics& metrics, bool includePad);

    BoringLayout(CharSequence* source, TextPaint* paint, int outerWidth, Alignment align,
            float spacingMult, float spacingAdd, const Metrics& metrics, bool includePad,
            TextUtils::TruncateAt ellipsize, int ellipsizedWidth);

    BoringLayout(CharSequence* source, TextPaint* paint, int outerWidth, Alignment align,
            float spacingMult, float spacingAdd, const Metrics& metrics, bool includePad,
            TextUtils::TruncateAt ellipsize, int ellipsizedWidth, bool useFallbackLineSpacing);

    BoringLayout(CharSequence* text, TextPaint* paint, int width, Alignment align,
            float spacingMult, float spacingAdd, bool includePad, bool fallbackLineSpacing,
            int ellipsizedWidth, TextUtils::TruncateAt ellipsize, const Metrics& metrics,
            bool useBoundsForWidth, bool shiftDrawingOffsetForStartOverhang,
            const Paint::FontMetrics* minimumFontMetrics);

    BoringLayout(CharSequence* text, TextPaint* paint, int width, Alignment align,
            const TextDirectionHeuristic* textDir, float spacingMult, float spacingAdd,
            bool includePad, bool fallbackLineSpacing, int ellipsizedWidth,
            TextUtils::TruncateAt ellipsize, int maxLines, int breakStrategy,
            int hyphenationFrequency, const std::vector<int>& leftIndents,
            const std::vector<int>& rightIndents, int justificationMode,
            const LineBreakConfig& lineBreakConfig, const Metrics& metrics,
            bool useBoundsForWidth, bool shiftDrawingOffsetForStartOverhang,
            const Paint::FontMetrics* minimumFontMetrics);

    /* package */ void init(CharSequence* source, TextPaint* paint, Alignment align,
            const BoringLayout::Metrics& metrics, bool includePad, bool trustWidth, bool useFallbackLineSpacing);

    static Metrics* isBoring(CharSequence* text, TextPaint* paint);
    static Metrics* isBoring(CharSequence* text, TextPaint* paint, Metrics* metrics);
    static Metrics* isBoring(CharSequence* text, TextPaint* paint,
            const TextDirectionHeuristic* textDir, Metrics* metrics);

    static Metrics* isBoring(CharSequence* text, TextPaint* paint,
            const TextDirectionHeuristic* textDir, bool useFallbackLineSpacing, Metrics* metrics);

    static Metrics* isBoring(CharSequence* text, TextPaint* paint,
            const TextDirectionHeuristic* textDir, bool useFallbackLineSpacing,
            Paint::FontMetrics* minimumFontMetrics, Metrics* metrics);

    int getHeight() const override;

    int getLineCount() const override;
    int getLineTop(int line) const override;
    int getLineDescent(int line) const override;
    int getLineStart(int line) const override;
    int getParagraphDirection(int line) const override;
    bool getLineContainsTab(int line) const override;
    float getLineMax(int line) const override;
    float getLineWidth(int line) const override;
    const Directions* getLineDirections(int line) const override;
    int getTopPadding() const override;
    int getBottomPadding() const override;
    int getEllipsisCount(int line) const override;
    int getEllipsisStart(int line) const override;
    int getEllipsizedWidth() const override;
    bool isFallbackLineSpacingEnabled() const override;
    RectF computeDrawingBoundingBox() const override;

    void draw(Canvas& c, Path* highlight,const Paint* highlightpaint,int cursorOffset)override;

    void ellipsized(int start, int end);
};
}/*endof namespace*/
#endif/*__BORING_LAYOUT_H__*/
