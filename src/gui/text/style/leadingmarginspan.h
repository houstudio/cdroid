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
            CharSequence* text, int start, int end, bool first, Layout* layout) const override {
        if (!first) return;
        const int color = mWantColor ? mColor : p.getColor();
        const float cy = (top + bottom) * 0.5f;
        const float cx = x + dir * (mBulletRadius + mGapWidth / 2);
        c.save();
        c.set_color(color);
        c.begin_new_sub_path();
        c.arc(cx, cy, static_cast<double>(mBulletRadius), 0.0, 2.0 * M_PI);
        c.fill();
        c.restore();
    }

    BulletSpan* clone() const override { return new BulletSpan(*this); }

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
            CharSequence* text, int start, int end, bool first, Layout* layout) const override {
        const int color = mColor != 0 ? mColor : p.getColor();
        c.save();
        c.set_color(color);
        if (dir == 1) {
            c.rectangle(x, top, mStripeWidth, bottom - top);
        } else {
            c.rectangle(x - mStripeWidth, top, mStripeWidth, bottom - top);
        }
        c.fill();
        c.restore();
    }

    QuoteSpan* clone() const override { return new QuoteSpan(*this); }

private:
    int mColor;
    int mStripeWidth;
    int mGapWidth;
};

} /* end namespace */
#endif /* __LEADING_MARGIN_SPAN_H__ */
