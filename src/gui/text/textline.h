#ifndef __CDROID_TEXT_LINE__
#define __CDROID_TEXT_LINE__
#include <text/textpaint.h>
#include <core/canvas.h>
#include <text/spannablestring.h>
namespace cdroid{
class Directions;
class TabStops;
class PrecomputedText;
class TextLine {
private:
    class DecorationInfo {
    public:
        bool isStrikeThruText;
        bool isUnderlineText;
        int underlineColor;
        float underlineThickness;
        int start = -1;
        int end = -1;
    public:
        bool hasDecoration() const{
            return isStrikeThruText || isUnderlineText || underlineColor != 0;
        }
        // Copies the info, but not the start and end range.
        DecorationInfo copyInfo() {
            DecorationInfo copy;
            copy.isStrikeThruText = isStrikeThruText;
            copy.isUnderlineText = isUnderlineText;
            copy.underlineColor = underlineColor;
            copy.underlineThickness = underlineThickness;
            return copy;
        }
    };
private:
    static constexpr char TAB_CHAR = '\t';
    static constexpr int TAB_INCREMENT = 20;

    const TextPaint* mPaint;
    CharSequence* mText;
    int mStart;
    int mLen;
    int mDir;
    const Directions* mDirections;
    bool mHasTabs;
    bool mCharsValid;
    bool mIsJustifying;
    bool mUseFallbackExtent;
    TabStops* mTabs;
    std::vector<char16_t> mChars;
    Spanned* mSpanned;
    PrecomputedText* mComputed;

    int mEllipsisStart;
    int mEllipsisEnd;

    float mAddedWidthForJustify;

    TextPaint mWorkPaint;
    TextPaint mActivePaint;
    SpanSet* mMetricAffectingSpanSpanSet;
    SpanSet* mCharacterStyleSpanSet;
    SpanSet* mReplacementSpanSpanSet;

    DecorationInfo mDecorationInfo;
    std::vector<DecorationInfo> mDecorations ;

    static TextLine* sCached[3];
private:
    int charAt(int i) const{
        return mCharsValid ? mChars[i] : mText->charAt(i + mStart);
    }
    float drawRun(Canvas& c, int start, int limit, bool runIsRtl, float x, int top, int y, int bottom, bool needWidth);
    float measureRun(int start, int offset, int limit, bool runIsRtl, Paint::FontMetricsInt* fmi) {
        return handleRun(start, offset, limit, runIsRtl, nullptr, 0, 0, 0, 0, fmi, true);
    }
    int getOffsetBeforeAfter(int runIndex, int runStart, int runLimit, bool runIsRtl, int offset, bool after);

    static void expandMetricsFromPaint(Paint::FontMetricsInt& fmi,const TextPaint& wp);
    static void drawStroke(TextPaint& wp, Canvas& c, int color, float position,
            float thickness, float xleft, float xright, float baseline);
    float getRunAdvance(TextPaint& wp, int start, int end, int contextStart, int contextEnd, bool runIsRtl, int offset);

    float handleText(TextPaint& wp, int start, int end, int contextStart, int contextEnd,
            bool runIsRtl, Canvas* c, float x, int top, int y, int bottom, Paint::FontMetricsInt* fmi,
            bool needWidth, int offset, const std::vector<DecorationInfo>* decorations);

    float handleReplacement(ReplacementSpan& replacement,const TextPaint& wp, int start, int limit, bool runIsRtl, Canvas* c,
           float x, int top, int y, int bottom,Paint::FontMetricsInt* fmi, bool needWidth);

    int adjustStartHyphenEdit(int start, int startHyphenEdit) {
        return start > 0 ? Paint::START_HYPHEN_EDIT_NO_EDIT : startHyphenEdit;
    }

    int adjustEndHyphenEdit(int limit, int endHyphenEdit) {
        return limit < mLen ? Paint::END_HYPHEN_EDIT_NO_EDIT : endHyphenEdit;
    }
    void extractDecorationInfo(TextPaint& paint, DecorationInfo& info);

    float handleRun(int start, int measureLimit, int limit, bool runIsRtl, Canvas*c, float x,
            int top, int y, int bottom, Paint::FontMetricsInt* fmi, bool needWidth);

    void drawTextRun(Canvas& c, TextPaint& wp, int start, int end,
            int contextStart, int contextEnd, bool runIsRtl, float x, int y);
    bool isStretchableWhitespace(int ch) const{
        return ch == 0x0020;
    }
    int countStretchableSpaces(int start, int end) const;
    static bool equalAttributes(const TextPaint& lp, const TextPaint& rp);
public:
    TextLine();
    virtual ~TextLine();
    static TextLine* obtain();
    static TextLine* recycle(TextLine* tl);
    void set(const TextPaint* paint, CharSequence* text, int start, int limit, int dir,const Directions* directions,
            bool hasTabs, TabStops* tabStops, int ellipsisStart, int ellipsisEnd,bool useFallbackLineSpacing);

    void justify(float justifyWidth);

    void draw(Canvas& c, float x, int top, int y, int bottom);
    float metrics(Paint::FontMetricsInt* fmi) {
        return measure(mLen, false, fmi);
    }

    float measure(int offset, bool trailing, Paint::FontMetricsInt* fmi);
    std::vector<float> measureAllOffsets(const std::vector<bool>& trailing, Paint::FontMetricsInt* fmi);

    int getOffsetToLeftRightOf(int cursor, bool toLeft);
    static void updateMetrics(Paint::FontMetricsInt& fmi, int previousTop, int previousAscent,
            int previousDescent, int previousBottom, int previousLeading);

    float nextTab(float h);

    static bool isLineEndSpace(char16_t ch);
};
}/*endof namespace*/
#endif /*__CDROID_TEXT_LINE__*/
