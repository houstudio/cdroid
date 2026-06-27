#ifndef __LEADING_MARGIN_SPAN_H__
#define __LEADING_MARGIN_SPAN_H__
#include <text/parcelablespan.h>
namespace cdroid {

class Canvas;
class Layout;
class Paint;

class LeadingMarginSpan : public ParagraphStyle {
public:
    virtual int getLeadingMargin(bool first) const { return 0; };
    virtual void drawLeadingMargin(Canvas& c, Paint& p, int x, int dir, int top, int baseline, int bottom,
            CharSequence* text, int start, int end, bool first, Layout* layout) const {}
};

class LeadingMarginSpan2 : public LeadingMarginSpan {
public:
    virtual int getLeadingMarginLineCount() const { return 0; }
};

class BulletSpan : public LeadingMarginSpan {
public:
    BulletSpan() : mGapWidth(6), mBulletRadius(3), mColor(0xff000000), mWantColor(false) {}
    explicit BulletSpan(int gapWidth) : mGapWidth(gapWidth), mBulletRadius(3), mColor(0xff000000), mWantColor(false) {}
    BulletSpan(int gapWidth, int color) : mGapWidth(gapWidth), mBulletRadius(3), mColor(color), mWantColor(true) {}

    int getLeadingMargin(bool first) const override {
        return mGapWidth + mBulletRadius * 2;
    }

    void drawLeadingMargin(Canvas& c, Paint& p, int x, int dir, int top, int baseline, int bottom,
            CharSequence* text, int start, int end, bool first, Layout* layout) const override {}

private:
    int mGapWidth;
    int mBulletRadius;
    int mColor;
    bool mWantColor;
};

class QuoteSpan : public LeadingMarginSpan {
public:
    QuoteSpan() : mColor(0xff000000), mStripeWidth(6), mGapWidth(2) {}
    explicit QuoteSpan(int color) : mColor(color), mStripeWidth(6), mGapWidth(2) {}

    int getLeadingMargin(bool first) const override {
        return mStripeWidth + mGapWidth;
    }

    void drawLeadingMargin(Canvas& c, Paint& p, int x, int dir, int top, int baseline, int bottom,
            CharSequence* text, int start, int end, bool first, Layout* layout) const override {}

private:
    int mColor;
    int mStripeWidth;
    int mGapWidth;
};

} /* end namespace */
#endif /* __LEADING_MARGIN_SPAN_H__ */
