#include <text/parcelablespan.h>
#ifndef __PARAGRAPH_STYLES_H__
#define __PARAGRAPH_STYLES_H__
namespace cdroid{
class Layout;
class AlignmentSpan :public ParagraphStyle{
protected:
    int mAlignment;
public:
    int getAlignment()const{return mAlignment;}
};
class LineHeightSpan :public ParagraphStyle{
public:
    class WithDensity;
    virtual void chooseHeight(CharSequence* text, int start, int end,
            int spanstartv, int lineHeight,Paint::FontMetricsInt& fm)const{}
};
class LineHeightSpan::WithDensity :public LineHeightSpan{
public:
    virtual void chooseHeight(CharSequence* text, int start, int end,
                int spanstartv, int lineHeight,
                Paint::FontMetricsInt& fm, TextPaint* paint)const{};
};

class LineBackgroundSpan :public ParagraphStyle{
public:
    int getLineBackground()const{return 0;}
    void drawBackground(Canvas&,Paint& paint, int left, int right, int top, int baseline,
            int bottom, CharSequence* text, int start, int end, int lineNumber)const{};
};
class LeadingMarginSpan :public ParagraphStyle{
public:
    int getLeadingMargin(bool)const{return 0;};
    void drawLeadingMargin(Canvas& c, Paint& p,int x, int dir, int top, int baseline, int bottom,
            CharSequence* text, int start, int end, bool first, Layout* layout)const{}
};

class WrapTogetherSpan:public ParagraphStyle{
};

class LeadingMarginSpan2:public LeadingMarginSpan{
public:
    int getLeadingMarginLineCount()const{return 0;}
};
class TabStopSpan :public ParagraphStyle {
public:
    int getTabStop()const{return 0;}
};
class ForegroundColorSpan : public CharacterStyle {
public:
    explicit ForegroundColorSpan(int color) : mColor(color) {}
    int getForegroundColor() const { return mColor; }
private:
    int mColor;
};
}/*endof namespace*/
#endif/*__PARAGRAPH_STYLES_H__*/
